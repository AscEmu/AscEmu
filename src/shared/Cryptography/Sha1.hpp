/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <openssl/sha.h>
#include "Cryptography/BigNumber.h"
#include <openssl/evp.h>

class Sha1Hash
{
public:
    Sha1Hash() noexcept;
    ~Sha1Hash() = default;

    void updateBigNumbers(BigNumber* _bn0, ...) const;

    void updateData(const uint8_t* _data, int _len) const;
    void updateData(const std::string& _str) const;

    void initialize() const;
    void finalize();

    uint8_t* getDigest();

private:
    EVP_MD_CTX* m_ctx;
    uint8_t m_digest[SHA_DIGEST_LENGTH] = {0};
};
