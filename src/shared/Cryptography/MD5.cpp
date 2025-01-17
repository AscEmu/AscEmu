/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MD5.hpp"

MD5Hash::MD5Hash() noexcept
{
    m_ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(m_ctx, EVP_md5(), nullptr);
}

void MD5Hash::updateData(const std::string& _str) const
{
    updateData(reinterpret_cast<const uint8_t*>(_str.data()), static_cast<int>(_str.length()));
}

void MD5Hash::updateData(const uint8_t* _data, int _len) const
{
    EVP_DigestUpdate(m_ctx, _data, _len);
}

void MD5Hash::initialize() const
{
    EVP_DigestInit(m_ctx, EVP_md5());
}

void MD5Hash::finalize()
{
    uint32_t length = MD5_DIGEST_LENGTH;
    EVP_DigestFinal_ex(m_ctx, m_digest, &length);
}

uint8_t* MD5Hash::getDigest() { return m_digest; }
