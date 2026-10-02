/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CertificateBundle.hpp"

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

#include <algorithm>
#include <ctime>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string_view>

namespace cp
{
    namespace
    {
        // the client splits the bundle from its signature at this marker
        constexpr char SignatureMarker[] = "NGIS";

        // appended to the bundle before hashing it for the signature
        constexpr char SignatureSalt[] = "Blizzard Certificate Bundle";

        constexpr size_t SignatureLength = 256;

        std::string toHex(const uint8_t* _data, size_t _size)
        {
            static constexpr char digits[] = "0123456789ABCDEF";
            std::string text;
            text.reserve(_size * 2);
            for (size_t i = 0; i < _size; ++i)
            {
                text.push_back(digits[_data[i] >> 4]);
                text.push_back(digits[_data[i] & 0x0F]);
            }
            return text;
        }

        std::array<uint8_t, 32> sha256(const std::string& _first, const std::string& _second = {})
        {
            std::array<uint8_t, 32> digest{};
            unsigned int length = 0;

            std::unique_ptr<EVP_MD_CTX, void(*)(EVP_MD_CTX*)> context(EVP_MD_CTX_new(), EVP_MD_CTX_free);
            if (!context || EVP_DigestInit_ex(context.get(), EVP_sha256(), nullptr) != 1
                || EVP_DigestUpdate(context.get(), _first.data(), _first.size()) != 1
                || (!_second.empty() && EVP_DigestUpdate(context.get(), _second.data(), _second.size()) != 1)
                || EVP_DigestFinal_ex(context.get(), digest.data(), &length) != 1)
                throw std::runtime_error("SHA-256 failed");

            return digest;
        }
    }

    CertificateBundle::CertificateBundle(const std::filesystem::path& _serverCertificate) : m_signingKey(nullptr, EVP_PKEY_free)
    {
        std::ifstream in(_serverCertificate, std::ios::binary);
        if (!in)
            throw std::runtime_error("Cannot open the server certificate " + _serverCertificate.string());

        std::stringstream pem;
        pem << in.rdbuf();
        const std::string pemText = pem.str();

        std::unique_ptr<BIO, void(*)(BIO*)> bio(BIO_new_mem_buf(pemText.data(), static_cast<int>(pemText.size())), BIO_free_all);
        std::unique_ptr<X509, void(*)(X509*)> certificate(bio ? PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr) : nullptr, X509_free);
        if (!certificate)
            throw std::runtime_error("The server certificate is no PEM certificate: " + _serverCertificate.string());

        // pinned value: SHA-256 of the PKCS#1 RSAPublicKey (modulus and exponent) of the first certificate,
        // the one the server presents
        EVP_PKEY* publicKey = X509_get0_pubkey(certificate.get());
        if (publicKey == nullptr || EVP_PKEY_get_base_id(publicKey) != EVP_PKEY_RSA)
            throw std::runtime_error("The server certificate needs an RSA key");

        unsigned char* der = nullptr;
        const int derLength = i2d_PublicKey(publicKey, &der);
        if (derLength <= 0)
            throw std::runtime_error("Cannot encode the public key of the server certificate");

        const auto hash = sha256(std::string(reinterpret_cast<const char*>(der), static_cast<size_t>(derLength)));
        OPENSSL_free(der);
        m_publicKeyHash = toHex(hash.data(), hash.size());

        // trust anchor of the bundle: the root of the chain, which is the last certificate of the file
        // (the certificate itself when it is self signed); base64 without line breaks
        constexpr std::string_view beginMarker = "-----BEGIN CERTIFICATE-----";
        constexpr std::string_view endMarker = "-----END CERTIFICATE-----";

        const auto begin = pemText.rfind(beginMarker);
        const auto end = pemText.rfind(endMarker);
        if (begin == std::string::npos || end == std::string::npos || end < begin)
            throw std::runtime_error("The server certificate is no PEM certificate: " + _serverCertificate.string());

        for (size_t i = begin + beginMarker.size(); i < end; ++i)
        {
            if (pemText[i] != '\r' && pemText[i] != '\n')
                m_certificateBase64.push_back(pemText[i]);
        }
    }

    CertificateBundle::~CertificateBundle() = default;

    std::string CertificateBundle::buildJson() const
    {
        std::ostringstream json;
        json << "{\n"
             << "    \"Created\": " << static_cast<uint64_t>(std::time(nullptr)) << ",\n"
             << "    \"Certificates\": [\n"
             << "        { \"Uri\": \"*.*\", \"ShaHashPublicKeyInfo\": \"" << m_publicKeyHash << "\" }\n"
             << "    ],\n"
             << "    \"PublicKeys\": [\n"
             << "        { \"Uri\": \"*.*\", \"ShaHashPublicKeyInfo\": \"" << m_publicKeyHash << "\" }\n"
             << "    ],\n"
             << "    \"SigningCertificates\": [\n"
             << "        { \"RawData\": \"-----BEGIN CERTIFICATE-----" << m_certificateBase64 << "-----END CERTIFICATE-----\" }\n"
             << "    ]\n"
             << "}";
        return json.str();
    }

    void CertificateBundle::writeUnsigned(const std::filesystem::path& _destination) const
    {
        std::ofstream out(_destination, std::ios::binary | std::ios::trunc);
        if (!out)
            throw std::runtime_error("Cannot write " + _destination.string());

        // placeholder where the signature would be, 260 characters behind the json
        std::string placeholder = ";";
        for (int i = 0; i < 10; ++i)
            placeholder += "inserting_dummy_signature_";
        placeholder.pop_back();

        out << buildJson() << placeholder;
    }

    std::array<uint8_t, 256> CertificateBundle::createSigningKey()
    {
        m_signingKey.reset(EVP_RSA_gen(2048));
        if (!m_signingKey)
            throw std::runtime_error("Cannot generate the bundle signing key");

        BIGNUM* modulus = nullptr;
        if (EVP_PKEY_get_bn_param(m_signingKey.get(), OSSL_PKEY_PARAM_RSA_N, &modulus) != 1)
            throw std::runtime_error("Cannot read the modulus of the bundle signing key");

        // the client keeps the modulus little endian
        std::array<uint8_t, 256> bytes{};
        const bool converted = BN_bn2lebinpad(modulus, bytes.data(), static_cast<int>(bytes.size())) == static_cast<int>(bytes.size());
        BN_free(modulus);
        if (!converted)
            throw std::runtime_error("Unexpected size of the bundle signing modulus");

        return bytes;
    }

    void CertificateBundle::writeSigned(const std::filesystem::path& _destination) const
    {
        if (!m_signingKey)
            throw std::runtime_error("No bundle signing key");

        const std::string json = buildJson();
        const auto digest = sha256(json, SignatureSalt);

        // PKCS#1 v1.5 signature of the SHA-256 digest
        std::unique_ptr<EVP_PKEY_CTX, void(*)(EVP_PKEY_CTX*)> context(EVP_PKEY_CTX_new(m_signingKey.get(), nullptr), EVP_PKEY_CTX_free);
        std::array<uint8_t, SignatureLength> signature{};
        size_t signatureLength = signature.size();
        if (!context || EVP_PKEY_sign_init(context.get()) != 1
            || EVP_PKEY_CTX_set_rsa_padding(context.get(), RSA_PKCS1_PADDING) != 1
            || EVP_PKEY_CTX_set_signature_md(context.get(), EVP_sha256()) != 1
            || EVP_PKEY_sign(context.get(), signature.data(), &signatureLength, digest.data(), digest.size()) != 1
            || signatureLength != signature.size())
            throw std::runtime_error("Cannot sign the certificate bundle");

        // the client reads the signature little endian
        std::reverse(signature.begin(), signature.end());

        if (_destination.has_parent_path())
            std::filesystem::create_directories(_destination.parent_path());

        std::ofstream out(_destination, std::ios::binary | std::ios::trunc);
        if (!out)
            throw std::runtime_error("Cannot write " + _destination.string());

        out << json << SignatureMarker;
        out.write(reinterpret_cast<const char*>(signature.data()), static_cast<std::streamsize>(signature.size()));
    }
} // namespace cp
