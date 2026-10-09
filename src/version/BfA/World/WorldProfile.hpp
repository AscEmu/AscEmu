/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "world/Server/ClientProtocol.hpp"

#include <array>
#include <cstdint>
#include <string_view>

// world connection of 8.3.7 clients (build 35662)
namespace AscEmu::Version::WorldProfile
{
    inline constexpr WoW::Expansion Expansion = WoW::Expansion::_BfA;
    inline constexpr std::string_view Name = "BfA";

    // plain strings closed by a line feed
    inline constexpr std::string_view ServerInitializer = "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT - V2\n";
    inline constexpr std::string_view ClientInitializer = "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER - V2\n";

    // size limit of a client packet with the opcode
    inline constexpr uint32_t MaxClientPacketSize = 0x40000;

    // the digest key is SHA256 of the key data and the seed of the client platform (Wn64, Mc64)
    inline constexpr std::array<std::array<uint8_t, 16>, 2> AuthSeeds{ {
        { 0x57, 0x8B, 0xC9, 0x48, 0x70, 0xC2, 0x78, 0xCB, 0x69, 0x62, 0xF3, 0x0E, 0x6D, 0xC2, 0x03, 0xBB },
        { 0x59, 0x66, 0x01, 0x6C, 0x36, 0x8E, 0xD9, 0xF7, 0xAA, 0xB6, 0x03, 0xEE, 0x67, 0x03, 0x08, 0x1C }
    } };
}
