/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AesGcmCrypt.hpp"

#include <openssl/evp.h>

#include <cstring>

namespace
{
    EVP_CIPHER_CTX* createContext(bool encrypting)
    {
        EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
        if (context == nullptr)
            return nullptr;

        if (EVP_CipherInit_ex(context, EVP_aes_128_gcm(), nullptr, nullptr, nullptr, encrypting ? 1 : 0) != 1)
        {
            EVP_CIPHER_CTX_free(context);
            return nullptr;
        }

        return context;
    }
}

AesGcmCrypt::AesGcmCrypt()
{
    m_encryptContext = createContext(true);
    m_decryptContext = createContext(false);
}

AesGcmCrypt::~AesGcmCrypt()
{
    EVP_CIPHER_CTX_free(static_cast<EVP_CIPHER_CTX*>(m_encryptContext));
    EVP_CIPHER_CTX_free(static_cast<EVP_CIPHER_CTX*>(m_decryptContext));
}

bool AesGcmCrypt::init(const Key& key)
{
    auto* encrypt = static_cast<EVP_CIPHER_CTX*>(m_encryptContext);
    auto* decrypt = static_cast<EVP_CIPHER_CTX*>(m_decryptContext);
    if (encrypt == nullptr || decrypt == nullptr)
        return false;

    if (EVP_CipherInit_ex(encrypt, nullptr, nullptr, key.data(), nullptr, -1) != 1)
        return false;

    if (EVP_CipherInit_ex(decrypt, nullptr, nullptr, key.data(), nullptr, -1) != 1)
        return false;

    m_initialized = true;
    return true;
}

bool AesGcmCrypt::process(void* context, bool encrypting, uint64_t counter, uint32_t magic, uint8_t* data, size_t length, uint8_t* tag)
{
    auto* cipher = static_cast<EVP_CIPHER_CTX*>(context);

    // nonce: packet counter of the direction, then the magic of the direction
    uint8_t nonce[12];
    std::memcpy(nonce, &counter, sizeof(counter));
    std::memcpy(nonce + sizeof(counter), &magic, sizeof(magic));

    if (EVP_CipherInit_ex(cipher, nullptr, nullptr, nullptr, nonce, -1) != 1)
        return false;

    int outLength = 0;
    if (length != 0 && EVP_CipherUpdate(cipher, data, &outLength, data, static_cast<int>(length)) != 1)
        return false;

    if (!encrypting && EVP_CIPHER_CTX_ctrl(cipher, EVP_CTRL_GCM_SET_TAG, static_cast<int>(TagSize), tag) != 1)
        return false;

    int finalLength = 0;
    if (EVP_CipherFinal_ex(cipher, data + outLength, &finalLength) != 1)
        return false;

    if (encrypting && EVP_CIPHER_CTX_ctrl(cipher, EVP_CTRL_GCM_GET_TAG, static_cast<int>(TagSize), tag) != 1)
        return false;

    return true;
}

bool AesGcmCrypt::encryptSend(uint8_t* data, size_t length, Tag& tag)
{
    bool result = true;
    if (m_initialized)
        result = process(m_encryptContext, true, m_serverCounter, ServerMagic, data, length, tag.data());
    else
        tag.fill(0);

    ++m_serverCounter;
    return result;
}

bool AesGcmCrypt::decryptReceive(uint8_t* data, size_t length, const Tag& tag)
{
    bool result = true;
    if (m_initialized)
    {
        Tag received = tag;
        result = process(m_decryptContext, false, m_clientCounter, ClientMagic, data, length, received.data());
    }

    ++m_clientCounter;
    return result;
}
