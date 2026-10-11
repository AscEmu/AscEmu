/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <openssl/evp.h>
#include <openssl/sha.h>

class Sha512Hash
{
public:
    static constexpr size_t DigestLength = SHA512_DIGEST_LENGTH;

    Sha512Hash() noexcept;
    ~Sha512Hash();

    Sha512Hash(Sha512Hash const&) = delete;
    Sha512Hash& operator=(Sha512Hash const&) = delete;

    void updateData(const uint8_t* _data, size_t _len) const;
    void updateData(const std::string& _str) const;

    void initialize() const;
    void finalize();

    const uint8_t* getDigest() const { return m_digest; }

    // one-shot helpers
    static void hash(const uint8_t* _data, size_t _len, uint8_t* _out);
    static void hmac(const uint8_t* _key, size_t _keyLen, const uint8_t* _data, size_t _len, uint8_t* _out);

private:
    EVP_MD_CTX* m_ctx;
    uint8_t m_digest[SHA512_DIGEST_LENGTH] = { 0 };
};
