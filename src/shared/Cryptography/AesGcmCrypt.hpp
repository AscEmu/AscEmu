/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Platform/SymbolVisibility.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

// AES-128-GCM of the world packets of 8.x clients: one key for both directions, the nonce is the packet
// counter of the direction and a fixed magic. The counters run from the first packet of the connection on,
// packets before the key is known leave with an empty tag.
class SERVER_DECL AesGcmCrypt
{
public:
    static constexpr size_t KeySize = 16;
    static constexpr size_t TagSize = 12;

    using Key = std::array<uint8_t, KeySize>;
    using Tag = std::array<uint8_t, TagSize>;

    AesGcmCrypt();
    ~AesGcmCrypt();

    AesGcmCrypt(const AesGcmCrypt&) = delete;
    AesGcmCrypt& operator=(const AesGcmCrypt&) = delete;

    bool init(const Key& key);
    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    // in place, the tag of the packet is produced or verified
    bool encryptSend(uint8_t* data, size_t length, Tag& tag);
    bool decryptReceive(uint8_t* data, size_t length, const Tag& tag);

private:
    static constexpr uint32_t ClientMagic = 0x544E4C43;
    static constexpr uint32_t ServerMagic = 0x52565253;

    bool process(void* context, bool encrypting, uint64_t counter, uint32_t magic, uint8_t* data, size_t length, uint8_t* tag);

    void* m_encryptContext = nullptr;
    void* m_decryptContext = nullptr;
    uint64_t m_clientCounter = 0;
    uint64_t m_serverCounter = 0;
    bool m_initialized = false;
};
