/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Sha1.h"
#include <cstdarg>

Sha1Hash::Sha1Hash() noexcept
{
    mC = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mC, EVP_sha1(), nullptr);
}

void Sha1Hash::UpdateData(const uint8_t* dta, int len)
{
    EVP_DigestUpdate(mC, dta, len);
}

void Sha1Hash::UpdateData(const std::string& str)
{
    UpdateData((uint8_t*)str.c_str(), (int)str.length());
}

void Sha1Hash::UpdateBigNumbers(BigNumber* bn0, ...)
{
    va_list v;
    BigNumber* bn;

    va_start(v, bn0);
    bn = bn0;
    while(bn)
    {
        UpdateData(bn->AsByteArray(), bn->GetNumBytes());
        bn = va_arg(v, BigNumber*);
    }
    va_end(v);
}

void Sha1Hash::Initialize()
{
    EVP_DigestInit(mC, EVP_sha1());
}

void Sha1Hash::Finalize(void)
{
    uint32_t length = SHA_DIGEST_LENGTH;
    EVP_DigestFinal_ex(mC, mDigest, &length);
}
