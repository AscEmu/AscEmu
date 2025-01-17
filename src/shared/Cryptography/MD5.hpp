/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>
#include <openssl/evp.h>
#include <openssl/md5.h>

class MD5Hash
{
public:
    MD5Hash() noexcept;
    ~MD5Hash() = default;

    void updateData(const uint8_t* _data, int _len) const;
    void updateData(const std::string& _str) const;

    void initialize() const;
    void finalize();

    uint8_t* getDigest();

private:
    EVP_MD_CTX* m_ctx;
    uint8_t m_digest[MD5_DIGEST_LENGTH] = {0};
};
