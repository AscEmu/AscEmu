/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MD5.h"

#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
#include <openssl/evp.h>
#endif

MD5Hash::MD5Hash() noexcept
{
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    mC = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mC, EVP_md5(), nullptr);
#else
    MD5_Init(&mC);    
#endif
}

void MD5Hash::UpdateData(const std::string& str)
{
    UpdateData((const uint8_t*)str.data(), (int)str.length());
}

void MD5Hash::UpdateData(const uint8_t* dta, int len)
{
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    EVP_DigestUpdate(mC, dta, len);
#else
    MD5_Update(&mC, dta, len);
#endif
}

void MD5Hash::Initialize()
{
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    EVP_DigestInit(mC, EVP_md5());
#else
    MD5_Init(&mC);
#endif
}

void MD5Hash::Finalize(void)
{
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    uint32_t length = MD5_DIGEST_LENGTH;
    EVP_DigestFinal_ex(mC, mDigest, &length);
#else
    MD5_Final(mDigest, &mC);
#endif
}
