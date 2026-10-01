/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "world/Server/ClientProtocol.hpp"

#include <array>
#include <cstdint>
#include <string_view>

// world connection of 7.3.5 clients (build 26972)
namespace AscEmu::Version::WorldProfile
{
    inline constexpr WoW::Expansion Expansion = WoW::Expansion::_Legion;
    inline constexpr std::string_view Name = "Legion";

    // plain strings closed by a line feed
    inline constexpr bool InitializerHasMagic = false;
    inline constexpr uint32_t InitializerMagic = 0;
    inline constexpr std::string_view ServerInitializer = "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT\n";
    inline constexpr std::string_view ClientInitializer = "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER\n";

    // always the 6 byte header (uint32 size, uint16 opcode)
    inline constexpr bool SetupHeaderBeforeAuthSession = false;

    // size limit of a client packet without the opcode
    inline constexpr uint32_t MaxClientPacketSize = 0x10000;

    // SMSG_ENABLE_ENCRYPTION / CMSG_ENABLE_ENCRYPTION_ACK before the world encryption starts
    inline constexpr bool EnableEncryptionHandshake = true;

    // the digest key is SHA256 of the key data and the seed of the client platform (Win, Wn64, Mc64)
    inline constexpr std::array<std::array<uint8_t, 16>, 3> AuthSeeds{ {
        { 0x79, 0x7E, 0xCC, 0x19, 0x66, 0x2D, 0xCB, 0xD5, 0x09, 0x0A, 0x44, 0x81, 0x17, 0x3F, 0x1D, 0x26 },
        { 0x6E, 0x21, 0x2D, 0xEF, 0x6A, 0x01, 0x24, 0xA3, 0xD9, 0xAD, 0x07, 0xF5, 0xE3, 0x22, 0xF7, 0xAE },
        { 0x34, 0x1C, 0xFE, 0xFE, 0x3D, 0x72, 0xAC, 0xA9, 0xA4, 0x40, 0x7D, 0xC5, 0x35, 0xDE, 0xD6, 0x6A }
    } };
}
