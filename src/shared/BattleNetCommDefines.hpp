/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace AscEmu::BattlenetComm
{
    enum Opcode : uint16_t
    {
        CMSG_REGISTER_REALM = 1,
        SMSG_REGISTER_REALM_RESULT = 2,
        SMSG_PENDING_WORLD_SESSION = 3
    };

    // BattleNetComm is an internal AscEmu protocol. Keep its wire format explicit so it
    // is independent of host endianness and cannot mistake unrelated TCP traffic for a
    // multi-gigabyte payload.
    constexpr std::array<uint8_t, 4> FrameMagic{ 'A', 'E', 'B', 'C' };
    constexpr uint16_t FrameVersion = 1;
    constexpr uint32_t ProtocolVersion = 2;
    constexpr size_t FrameHeaderSize = 12;
    constexpr uint32_t MaxPacketSize = 64 * 1024;

    struct FrameHeader
    {
        uint16_t version = 0;
        uint16_t opcode = 0;
        uint32_t payloadSize = 0;
    };

    inline void writeUInt16BE(uint8_t* destination, uint16_t value)
    {
        destination[0] = static_cast<uint8_t>((value >> 8) & 0xFFu);
        destination[1] = static_cast<uint8_t>(value & 0xFFu);
    }

    inline void writeUInt32BE(uint8_t* destination, uint32_t value)
    {
        destination[0] = static_cast<uint8_t>((value >> 24) & 0xFFu);
        destination[1] = static_cast<uint8_t>((value >> 16) & 0xFFu);
        destination[2] = static_cast<uint8_t>((value >> 8) & 0xFFu);
        destination[3] = static_cast<uint8_t>(value & 0xFFu);
    }

    [[nodiscard]] inline uint16_t readUInt16BE(const uint8_t* source)
    {
        return static_cast<uint16_t>((static_cast<uint16_t>(source[0]) << 8)
            | static_cast<uint16_t>(source[1]));
    }

    [[nodiscard]] inline uint32_t readUInt32BE(const uint8_t* source)
    {
        return (static_cast<uint32_t>(source[0]) << 24)
            | (static_cast<uint32_t>(source[1]) << 16)
            | (static_cast<uint32_t>(source[2]) << 8)
            | static_cast<uint32_t>(source[3]);
    }

    inline void encodeFrameHeader(std::array<uint8_t, FrameHeaderSize>& bytes, uint16_t opcode, uint32_t payloadSize)
    {
        bytes[0] = FrameMagic[0];
        bytes[1] = FrameMagic[1];
        bytes[2] = FrameMagic[2];
        bytes[3] = FrameMagic[3];
        writeUInt16BE(bytes.data() + 4, FrameVersion);
        writeUInt16BE(bytes.data() + 6, opcode);
        writeUInt32BE(bytes.data() + 8, payloadSize);
    }

    [[nodiscard]] inline bool decodeFrameHeader(const std::array<uint8_t, FrameHeaderSize>& bytes, FrameHeader& header)
    {
        if (bytes[0] != FrameMagic[0] || bytes[1] != FrameMagic[1]
            || bytes[2] != FrameMagic[2] || bytes[3] != FrameMagic[3])
        {
            return false;
        }

        header.version = readUInt16BE(bytes.data() + 4);
        header.opcode = readUInt16BE(bytes.data() + 6);
        header.payloadSize = readUInt32BE(bytes.data() + 8);
        return true;
    }
}
