/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Sha1.h"
#include <cstdarg>

Sha1Hash::Sha1Hash() noexcept
{
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    mC = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mC, EVP_sha1(), nullptr);
#else
    SHA1_Init(&mC);    
#endif
}

void Sha1Hash::UpdateData(const uint8_t* dta, int len)
{
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    EVP_DigestUpdate(mC, dta, len);
#else
    SHA1_Update(&mC, dta, len);
#endif
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
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    EVP_DigestInit(mC, EVP_sha1());
#else
    SHA1_Init(&mC);
#endif
}

void Sha1Hash::Finalize(void)
{
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    uint32_t length = SHA_DIGEST_LENGTH;
    EVP_DigestFinal_ex(mC, mDigest, &length);
#else
    SHA1_Final(mDigest, &mC);
#endif
}
