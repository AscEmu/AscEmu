/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdlib>
#include <string>
#include <openssl/md5.h>

#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
#include <openssl/types.h>
#endif

class MD5Hash
{
public:
    MD5Hash() noexcept;
    ~MD5Hash() = default;

    void UpdateData(const uint8_t* dta, int len);
    void UpdateData(const std::string& str);

    void Initialize();
    void Finalize();

    uint8_t* GetDigest(void) { return mDigest; };
    int GetLength(void) { return MD5_DIGEST_LENGTH; };

private:
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    EVP_MD_CTX *mC;
    uint8_t mDigest[MD5_DIGEST_LENGTH];
#else
    MD5_CTX mC;
    uint8_t mDigest[MD5_DIGEST_LENGTH];
#endif
};
