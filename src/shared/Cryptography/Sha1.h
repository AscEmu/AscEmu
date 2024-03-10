/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdlib>
#include <openssl/sha.h>

#include "Cryptography/BigNumber.h"

#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
#include <openssl/evp.h>
#endif

class Sha1Hash
{
public:
    Sha1Hash() noexcept;
    ~Sha1Hash() = default;

    void UpdateFinalizeBigNumbers(BigNumber* bn0, ...);
    void UpdateBigNumbers(BigNumber* bn0, ...);

    void UpdateData(const uint8_t* dta, int len);
    void UpdateData(const std::string& str);

    void Initialize();
    void Finalize();

    uint8_t* GetDigest(void) { return mDigest; };
    int GetLength(void) { return SHA_DIGEST_LENGTH; };

    BigNumber GetBigNumber();

private:
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
    EVP_MD_CTX *mC;
    uint8_t mDigest[SHA_DIGEST_LENGTH];
#else
    SHA_CTX mC;
    uint8_t mDigest[SHA_DIGEST_LENGTH];
#endif
};
