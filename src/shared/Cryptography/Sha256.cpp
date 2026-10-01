/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Sha256.hpp"

#include <openssl/hmac.h>

Sha256Hash::Sha256Hash() noexcept
{
    m_ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(m_ctx, EVP_sha256(), nullptr);
}

Sha256Hash::~Sha256Hash()
{
    EVP_MD_CTX_free(m_ctx);
}

void Sha256Hash::updateData(const uint8_t* _data, size_t _len) const
{
    EVP_DigestUpdate(m_ctx, _data, _len);
}

void Sha256Hash::updateData(const std::string& _str) const
{
    updateData(reinterpret_cast<const uint8_t*>(_str.data()), _str.length());
}

void Sha256Hash::initialize() const
{
    EVP_DigestInit_ex(m_ctx, EVP_sha256(), nullptr);
}

void Sha256Hash::finalize()
{
    uint32_t length = SHA256_DIGEST_LENGTH;
    EVP_DigestFinal_ex(m_ctx, m_digest, &length);
}

void Sha256Hash::hash(const uint8_t* _data, size_t _len, uint8_t* _out)
{
    uint32_t length = SHA256_DIGEST_LENGTH;
    EVP_Digest(_data, _len, _out, &length, EVP_sha256(), nullptr);
}

void Sha256Hash::hmac(const uint8_t* _key, size_t _keyLen, const uint8_t* _data, size_t _len, uint8_t* _out)
{
    uint32_t length = SHA256_DIGEST_LENGTH;
    HMAC(EVP_sha256(), _key, static_cast<int>(_keyLen), _data, _len, _out, &length);
}
