/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <vector>
#include <cstdint>
#include "RC4.hpp"

class WowCrypt
{
public:
    WowCrypt();
    ~WowCrypt() = default;

    bool isInitialized();

    const static int seedLenght = 16;

private:
    bool m_isInitialized;

//////////////////////////////////////////////////////////////////////////////////////////
// WotLK
public:
    void initWotlkCrypt(uint8_t* key);
    void initMopCrypt(uint8_t* key);
    void decryptWotlkReceive(uint8_t* data, size_t length);
    void encryptWotlkSend(uint8_t* data, size_t length);

private:
    AscEmu::RC4StreamCipher::KeyStruct m_clientWotlkDecryptKey;
    AscEmu::RC4StreamCipher::KeyStruct m_serverWotlkEncryptKey;

    AscEmu::RC4Engine m_clientWotlkDecrypt;
    AscEmu::RC4Engine m_servertWotlkEncrypt;

//////////////////////////////////////////////////////////////////////////////////////////
// Legacy
public:
    const static size_t cryptedSendLength = 4;
    const static size_t cryptedReceiveLength = 6;

    void initLegacyCrypt();
    void decryptLegacyReceive(uint8_t* data, size_t length);
    void encryptLegacySend(uint8_t* data, size_t length);
    void setLegacyKey(uint8_t* key, size_t length);

    static void generateTbcKey(uint8_t* key, uint8_t* sessionkey);

private:
    std::vector<uint8_t> crypKeyVector;
    uint8_t m_sendI;
    uint8_t m_sendJ;
    uint8_t m_recvI;
    uint8_t m_recvJ;
};
