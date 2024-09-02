/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MD5.h"
#include <openssl/evp.h>


MD5Hash::MD5Hash() noexcept
{
    mC = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mC, EVP_md5(), nullptr);
}

void MD5Hash::UpdateData(const std::string& str)
{
    UpdateData((const uint8_t*)str.data(), (int)str.length());
}

void MD5Hash::UpdateData(const uint8_t* dta, int len)
{
    EVP_DigestUpdate(mC, dta, len);
}

void MD5Hash::Initialize()
{
    EVP_DigestInit(mC, EVP_md5());
}

void MD5Hash::Finalize(void)
{
    uint32_t length = MD5_DIGEST_LENGTH;
    EVP_DigestFinal_ex(mC, mDigest, &length);
}
