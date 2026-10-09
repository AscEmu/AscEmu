/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

typedef struct evp_pkey_st EVP_PKEY;

namespace cp
{
    // certificate bundle of the 6.x/7.x Battle.net client: the client trusts a TLS server whose
    // public key hash (SHA-256 of the PKCS#1 RSAPublicKey) is listed in the bundle
    class CertificateBundle
    {
    public:
        // reads the PEM certificate of the Battle.net server the bundle has to trust
        explicit CertificateBundle(const std::filesystem::path& _serverCertificate);
        ~CertificateBundle();

        CertificateBundle(const CertificateBundle&) = delete;
        CertificateBundle& operator=(const CertificateBundle&) = delete;

        const std::string& publicKeyHash() const noexcept { return m_publicKeyHash; }

        // 6.2.4: the bundle with a placeholder signature, the patched client does not check it
        void writeUnsigned(const std::filesystem::path& _destination) const;

        // 7.3.5 and 8.3.7: the signing key whose modulus replaces the one the client verifies the bundle with;
        // the key is kept in _keyFile so every patched client of this server accepts the same cached bundle
        std::array<uint8_t, 256> createSigningKey(const std::filesystem::path& _keyFile);

        // 7.3.5: the bundle signed with the key of createSigningKey()
        void writeSigned(const std::filesystem::path& _destination) const;

    private:
        std::string buildJson() const;

        std::string m_publicKeyHash;
        std::string m_certificateBase64;
        std::unique_ptr<EVP_PKEY, void(*)(EVP_PKEY*)> m_signingKey;
    };
} // namespace cp
