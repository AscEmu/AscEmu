/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "BNetCertificateGenerator.hpp"

#include "Logging/Logger.hpp"

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <system_error>

namespace AscEmu::Battlenet
{
    namespace
    {
        constexpr int KeyBits = 2048;
        constexpr long ValidSeconds = 20L * 365L * 24L * 60L * 60L;

        using KeyPtr = std::unique_ptr<EVP_PKEY, void(*)(EVP_PKEY*)>;
        using CertificatePtr = std::unique_ptr<X509, void(*)(X509*)>;
        using BioPtr = std::unique_ptr<BIO, void(*)(BIO*)>;

        std::string lastOpenSslError()
        {
            const unsigned long error = ERR_get_error();
            if (error == 0)
                return "no OpenSSL error available";

            char buffer[256]{};
            ERR_error_string_n(error, buffer, sizeof(buffer));
            return std::string(buffer);
        }

        bool addExtension(X509* certificate, X509* issuer, int nid, const char* value)
        {
            X509V3_CTX context;
            X509V3_set_ctx_nodb(&context);
            X509V3_set_ctx(&context, issuer, certificate, nullptr, nullptr, 0);

            X509_EXTENSION* extension = X509V3_EXT_conf_nid(nullptr, &context, nid, value);
            if (extension == nullptr)
                return false;

            const bool added = X509_add_ext(certificate, extension, -1) == 1;
            X509_EXTENSION_free(extension);
            return added;
        }

        // certificate with a random serial, valid from yesterday (clocks of clients may be behind)
        CertificatePtr createCertificate(EVP_PKEY* publicKey, const char* commonName)
        {
            CertificatePtr certificate(X509_new(), X509_free);
            if (!certificate)
                return certificate;

            uint64_t serial = 0;
            if (RAND_bytes(reinterpret_cast<unsigned char*>(&serial), sizeof(serial)) != 1)
                return CertificatePtr(nullptr, X509_free);
            serial &= 0x7FFFFFFFFFFFFFFFULL;

            bool ok = X509_set_version(certificate.get(), X509_VERSION_3) == 1
                && ASN1_INTEGER_set_uint64(X509_get_serialNumber(certificate.get()), serial) == 1
                && X509_gmtime_adj(X509_getm_notBefore(certificate.get()), -24L * 60L * 60L) != nullptr
                && X509_gmtime_adj(X509_getm_notAfter(certificate.get()), ValidSeconds) != nullptr
                && X509_set_pubkey(certificate.get(), publicKey) == 1;

            std::unique_ptr<X509_NAME, void(*)(X509_NAME*)> name(X509_NAME_new(), X509_NAME_free);
            ok = ok && name
                && X509_NAME_add_entry_by_txt(name.get(), "O", MBSTRING_ASC, reinterpret_cast<const unsigned char*>("AscEmu"), -1, -1, 0) == 1
                && X509_NAME_add_entry_by_txt(name.get(), "CN", MBSTRING_ASC, reinterpret_cast<const unsigned char*>(commonName), -1, -1, 0) == 1
                && X509_set_subject_name(certificate.get(), name.get()) == 1;

            if (!ok)
                return CertificatePtr(nullptr, X509_free);

            return certificate;
        }

        bool writeCertificates(const std::string& fileName, X509* server, X509* root)
        {
            BioPtr output(BIO_new_file(fileName.c_str(), "wb"), BIO_free_all);
            return output
                && PEM_write_bio_X509(output.get(), server) == 1
                && PEM_write_bio_X509(output.get(), root) == 1
                && BIO_flush(output.get()) == 1;
        }

        bool writePrivateKey(const std::string& fileName, EVP_PKEY* key)
        {
            BioPtr output(BIO_new_file(fileName.c_str(), "wb"), BIO_free_all);
            return output
                && PEM_write_bio_PrivateKey(output.get(), key, nullptr, nullptr, 0, nullptr, nullptr) == 1
                && BIO_flush(output.get()) == 1;
        }

        void createParentDirectory(const std::string& fileName)
        {
            const std::filesystem::path parent = std::filesystem::path(fileName).parent_path();
            if (parent.empty())
                return;

            std::error_code error;
            std::filesystem::create_directories(parent, error);
        }
    }

    bool generateServerCertificate(const std::string& certificateFile, const std::string& privateKeyFile)
    {
        KeyPtr rootKey(EVP_RSA_gen(KeyBits), EVP_PKEY_free);
        KeyPtr serverKey(EVP_RSA_gen(KeyBits), EVP_PKEY_free);
        if (!rootKey || !serverKey)
        {
            sLogger.failure("BNet TLS: cannot generate the RSA keys of the server certificate: {}", lastOpenSslError());
            return false;
        }

        // root: signs the server certificate and is the trust anchor the clients get
        CertificatePtr root = createCertificate(rootKey.get(), "AscEmu Battle.net Root CA");
        if (!root
            || X509_set_issuer_name(root.get(), X509_get_subject_name(root.get())) != 1
            || !addExtension(root.get(), root.get(), NID_basic_constraints, "critical,CA:TRUE")
            || !addExtension(root.get(), root.get(), NID_key_usage, "critical,digitalSignature,keyCertSign,cRLSign")
            || !addExtension(root.get(), root.get(), NID_subject_key_identifier, "hash")
            || !addExtension(root.get(), root.get(), NID_authority_key_identifier, "keyid:always")
            || X509_sign(root.get(), rootKey.get(), EVP_sha256()) == 0)
        {
            sLogger.failure("BNet TLS: cannot create the root certificate: {}", lastOpenSslError());
            return false;
        }

        // server: the name matches every host the clients connect to
        CertificatePtr server = createCertificate(serverKey.get(), "*.*");
        if (!server
            || X509_set_issuer_name(server.get(), X509_get_subject_name(root.get())) != 1
            || !addExtension(server.get(), root.get(), NID_basic_constraints, "CA:FALSE")
            || !addExtension(server.get(), root.get(), NID_key_usage, "critical,digitalSignature,keyEncipherment")
            || !addExtension(server.get(), root.get(), NID_ext_key_usage, "serverAuth")
            || !addExtension(server.get(), root.get(), NID_subject_key_identifier, "hash")
            || !addExtension(server.get(), root.get(), NID_authority_key_identifier, "keyid:always")
            || X509_sign(server.get(), rootKey.get(), EVP_sha256()) == 0)
        {
            sLogger.failure("BNet TLS: cannot create the server certificate: {}", lastOpenSslError());
            return false;
        }

        createParentDirectory(certificateFile);
        createParentDirectory(privateKeyFile);

        if (!writePrivateKey(privateKeyFile, serverKey.get()))
        {
            sLogger.failure("BNet TLS: cannot write the private key '{}': {}", privateKeyFile, lastOpenSslError());
            return false;
        }

        if (!writeCertificates(certificateFile, server.get(), root.get()))
        {
            sLogger.failure("BNet TLS: cannot write the certificate '{}': {}", certificateFile, lastOpenSslError());
            return false;
        }

        return true;
    }
}
