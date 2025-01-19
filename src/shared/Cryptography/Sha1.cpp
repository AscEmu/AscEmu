/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Sha1.hpp"
#include <cstdarg>

Sha1Hash::Sha1Hash() noexcept
{
    m_ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(m_ctx, EVP_sha1(), nullptr);
}

void Sha1Hash::updateData(const uint8_t* _data, int _len) const
{
    EVP_DigestUpdate(m_ctx, _data, _len);
}

void Sha1Hash::updateData(const std::string& _str) const
{
    updateData(reinterpret_cast<const uint8_t*>(_str.data()), static_cast<int>(_str.length()));
}

void Sha1Hash::updateBigNumbers(BigNumber* _bn0, ...) const
{
    va_list v;

    va_start(v, _bn0);
    BigNumber* bigNumber = _bn0;
    while(bigNumber)
    {
        updateData(bigNumber->AsByteArray(), bigNumber->GetNumBytes());
        bigNumber = va_arg(v, BigNumber*);
    }
    va_end(v);
}

void Sha1Hash::initialize() const
{
    EVP_DigestInit(m_ctx, EVP_sha1());
}

void Sha1Hash::finalize()
{
    uint32_t length = SHA_DIGEST_LENGTH;
    EVP_DigestFinal_ex(m_ctx, m_digest, &length);
}

uint8_t* Sha1Hash::getDigest() { return m_digest; }

