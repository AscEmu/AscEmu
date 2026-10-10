/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "world/Server/ClientProtocol.hpp"

#include <array>
#include <cstdint>
#include <string_view>

// world connection of 10.2.7 clients (build 55664)
namespace AscEmu::Version::WorldProfile
{
    inline constexpr WoW::Expansion Expansion = WoW::Expansion::_Dragonflight;
    inline constexpr std::string_view Name = "Dragonflight";

    // plain strings closed by a line feed
    inline constexpr std::string_view ServerInitializer = "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT - V2\n";
    inline constexpr std::string_view ClientInitializer = "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER - V2\n";

    // size limit of a client packet with the opcode
    inline constexpr uint32_t MaxClientPacketSize = 0x40000;

    // the digest key is SHA256 of the key data and the seed of the client platform; only the Windows 64 bit
    // seed of this build is known
    inline constexpr std::array<std::array<uint8_t, 16>, 1> AuthSeeds{ {
        { 0xDB, 0xCA, 0x58, 0x48, 0x6F, 0xAA, 0xA0, 0xFE, 0x54, 0xEA, 0x28, 0x7A, 0x30, 0x47, 0xE9, 0x23 }
    } };
}
