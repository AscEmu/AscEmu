/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstddef>
#include <cstdint>

// Ed25519 signatures with a context (Ed25519ctx of RFC 8032) on the big number arithmetic of OpenSSL, so the
// signature does not depend on the Ed25519 context support that OpenSSL offers from 3.2 on
class Ed25519
{
public:
    static constexpr size_t KeyLength = 32;
    static constexpr size_t SignatureLength = 64;

    // signature of a message with the 32 byte private key and a context of up to 255 bytes
    static bool signWithContext(const uint8_t* _privateKey, const uint8_t* _context, size_t _contextLength, const uint8_t* _message, size_t _messageLength, uint8_t* _signature);

    // the 32 byte public key of a private key
    static bool publicKey(const uint8_t* _privateKey, uint8_t* _publicKey);
};
