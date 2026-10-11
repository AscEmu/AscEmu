/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "world/Server/ClientProtocol.hpp"

#include <array>
#include <cstdint>
#include <string_view>

// world connection of 11.2.7 clients (build 65299)
namespace AscEmu::Version::WorldProfile
{
    inline constexpr WoW::Expansion Expansion = WoW::Expansion::_TheWarWithin;
    inline constexpr std::string_view Name = "TheWarWithin";

    // plain strings closed by a line feed
    inline constexpr std::string_view ServerInitializer = "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT - V2\n";
    inline constexpr std::string_view ClientInitializer = "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER - V2\n";

    // size limit of a client packet with the opcode
    inline constexpr uint32_t MaxClientPacketSize = 0x10000;

    // the digest key is SHA512 of the key data and the seed of the client platform: Windows x64, Mac x64,
    // Mac arm64 and Windows arm64
    inline constexpr std::array<std::array<uint8_t, 16>, 4> AuthSeeds{ {
        { 0x11, 0x92, 0x25, 0x98, 0xFB, 0xB8, 0xB9, 0x2F, 0x69, 0x45, 0x11, 0xDD, 0xAE, 0x4B, 0x06, 0x5E },
        { 0x5C, 0x51, 0x3A, 0x31, 0x17, 0x4C, 0xD2, 0xE9, 0x1C, 0xE5, 0x92, 0x6B, 0xB2, 0x63, 0xC4, 0x59 },
        { 0x13, 0xD6, 0x7C, 0x8B, 0x31, 0xE6, 0xE0, 0x43, 0x8D, 0xDD, 0x54, 0x6E, 0x6B, 0x60, 0x89, 0xC3 },
        { 0xD0, 0x80, 0x2E, 0xFF, 0x53, 0x6A, 0x2E, 0x14, 0x11, 0x7D, 0xBF, 0xE0, 0xA5, 0x55, 0xEA, 0x47 }
    } };
}
