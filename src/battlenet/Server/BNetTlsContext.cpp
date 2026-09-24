/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "BNetTlsContext.hpp"

#include "BNetConfig.hpp"
#include "Logging/Logger.hpp"

#include <openssl/err.h>
#include <openssl/pkcs12.h>
#include <openssl/ssl.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>

namespace AscEmu::Battlenet
{
    namespace
    {
        std::string getOpenSslError()
        {
            const unsigned long error = ERR_get_error();
            if (error == 0)
                return "no OpenSSL error available";

            char buffer[256]{};
            ERR_error_string_n(error, buffer, sizeof(buffer));
            return std::string(buffer);
        }

        std::string getLowercaseExtension(const std::string& fileName)
        {
            std::string extension = std::filesystem::path(fileName).extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), [](const unsigned char value) { return static_cast<char>(std::tolower(value)); });
            return extension;
        }

        bool loadPkcs12(SSL_CTX* context, const std::string& fileName, const std::string& password)
        {
            BIO* input = BIO_new_file(fileName.c_str(), "rb");
            if (input == nullptr)
            {
                sLogger.failure("BNet TLS: failed to open PKCS#12 file '{}': {}", fileName, getOpenSslError());
                return false;
            }

            PKCS12* pkcs12 = d2i_PKCS12_bio(input, nullptr);
            BIO_free(input);

            if (pkcs12 == nullptr)
            {
                sLogger.failure("BNet TLS: failed to decode PKCS#12 file '{}': {}", fileName, getOpenSslError());
                return false;
            }

            EVP_PKEY* privateKey = nullptr;
            X509* certificate = nullptr;
            STACK_OF(X509)* certificateChain = nullptr;

            const int parseResult = PKCS12_parse(pkcs12, password.c_str(), &privateKey, &certificate, &certificateChain);
            PKCS12_free(pkcs12);

            if (parseResult != 1)
            {
                sLogger.failure("BNet TLS: failed to parse PKCS#12 file '{}': {}", fileName, getOpenSslError());
                return false;
            }

            bool success = true;

            if (SSL_CTX_use_certificate(context, certificate) != 1)
            {
                sLogger.failure("BNet TLS: failed to load certificate from '{}': {}", fileName, getOpenSslError());
                success = false;
            }

            if (success && SSL_CTX_use_PrivateKey(context, privateKey) != 1)
            {
                sLogger.failure("BNet TLS: failed to load private key from '{}': {}", fileName, getOpenSslError());
                success = false;
            }

            if (success && certificateChain != nullptr)
            {
                const int certificateCount = sk_X509_num(certificateChain);
                for (int index = 0; index < certificateCount; ++index)
                {
                    X509* chainCertificate = sk_X509_value(certificateChain, index);
                    if (chainCertificate == nullptr)
                        continue;

                    if (X509_up_ref(chainCertificate) != 1 || SSL_CTX_add_extra_chain_cert(context, chainCertificate) != 1)
                    {
                        if (chainCertificate != nullptr)
                            X509_free(chainCertificate);

                        sLogger.failure("BNet TLS: failed to add certificate chain from '{}': {}", fileName, getOpenSslError());
                        success = false;
                        break;
                    }
                }
            }

            sk_X509_pop_free(certificateChain, X509_free);
            X509_free(certificate);
            EVP_PKEY_free(privateKey);

            return success;
        }

        bool loadPem(SSL_CTX* context, const std::string& certificateFile, const std::string& privateKeyFile, const std::string& password)
        {
            if (privateKeyFile.empty())
            {
                sLogger.failure("BNet TLS: PrivateKeyFile must be set when using a PEM certificate");
                return false;
            }

            if (!password.empty())
            {
                SSL_CTX_set_default_passwd_cb_userdata(context, const_cast<char*>(password.c_str()));
            }

            if (SSL_CTX_use_certificate_chain_file(context, certificateFile.c_str()) != 1)
            {
                sLogger.failure("BNet TLS: failed to load certificate '{}': {}", certificateFile, getOpenSslError());
                return false;
            }

            if (SSL_CTX_use_PrivateKey_file(context, privateKeyFile.c_str(), SSL_FILETYPE_PEM) != 1)
            {
                sLogger.failure("BNet TLS: failed to load private key '{}': {}", privateKeyFile, getOpenSslError());
                return false;
            }

            return true;
        }
    }

    BNetTlsContext& BNetTlsContext::getInstance()
    {
        static BNetTlsContext instance;
        return instance;
    }

    bool BNetTlsContext::initialize()
    {
        finalize();

        m_context = SSL_CTX_new(TLS_server_method());
        if (m_context == nullptr)
        {
            sLogger.failure("BNet TLS: failed to create SSL context: {}", getOpenSslError());
            return false;
        }

        SSL_CTX_set_min_proto_version(m_context, TLS1_2_VERSION);

        const std::string extension = getLowercaseExtension(bnetConfig.tls.certificatesFile);
        bool loaded = false;

        if (extension == ".pfx" || extension == ".p12")
        {
            loaded = loadPkcs12(m_context, bnetConfig.tls.certificatesFile, bnetConfig.tls.privateKeyPassword);
        }
        else
        {
            loaded = loadPem(m_context, bnetConfig.tls.certificatesFile, bnetConfig.tls.privateKeyFile, bnetConfig.tls.privateKeyPassword);
        }

        if (!loaded)
        {
            finalize();
            return false;
        }

        if (SSL_CTX_check_private_key(m_context) != 1)
        {
            sLogger.failure("BNet TLS: certificate and private key do not match: {}", getOpenSslError());
            finalize();
            return false;
        }

        sLogger.info("BNet TLS: certificate loaded from '{}'", bnetConfig.tls.certificatesFile);
        return true;
    }

    void BNetTlsContext::finalize()
    {
        if (m_context == nullptr)
            return;

        SSL_CTX_free(m_context);
        m_context = nullptr;
    }
}
