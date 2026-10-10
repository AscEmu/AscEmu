/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "world/Server/ClientProtocol.hpp"

#include <array>
#include <cstdint>
#include <string_view>

// world connection of 9.2.7 clients (build 45745)
namespace AscEmu::Version::WorldProfile
{
    inline constexpr WoW::Expansion Expansion = WoW::Expansion::_Shadowlands;
    inline constexpr std::string_view Name = "Shadowlands";

    // plain strings closed by a line feed
    inline constexpr std::string_view ServerInitializer = "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT - V2\n";
    inline constexpr std::string_view ClientInitializer = "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER - V2\n";

    // size limit of a client packet with the opcode
    inline constexpr uint32_t MaxClientPacketSize = 0x40000;

    // the digest key is SHA256 of the key data and the seed of the client platform; only the Windows 64 bit
    // seed of this build is known
    inline constexpr std::array<std::array<uint8_t, 16>, 1> AuthSeeds{ {
        { 0x0F, 0x6D, 0xC9, 0x01, 0x61, 0x69, 0x4D, 0x76, 0x5A, 0x59, 0x5A, 0x3A, 0xF6, 0x03, 0x16, 0x6B }
    } };
}
