/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "world/Server/ClientProtocol.hpp"

#include <array>
#include <cstdint>
#include <string_view>

// world connection of 12.1.5 clients (build 70391)
namespace AscEmu::Version::WorldProfile
{
    inline constexpr WoW::Expansion Expansion = WoW::Expansion::_Midnight;
    inline constexpr std::string_view Name = "Midnight";

    // plain strings closed by a line feed
    inline constexpr std::string_view ServerInitializer = "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT - V2\n";
    inline constexpr std::string_view ClientInitializer = "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER - V2\n";

    // size limit of a client packet with the opcode
    inline constexpr uint32_t MaxClientPacketSize = 0x10000;

    // the digest key is SHA512 of the key data and the auth key of the client build variant: Windows x64, Mac x64,
    // Mac arm64 and Windows arm64 of the retail client
    inline constexpr std::array<std::array<uint8_t, 16>, 4> AuthSeeds{ {
        { 0x19, 0xFB, 0xE9, 0xED, 0xFA, 0x14, 0x30, 0x11, 0x5B, 0xD5, 0x35, 0x29, 0xA0, 0x50, 0xEB, 0xD4 },
        { 0x44, 0x7B, 0xB8, 0xC2, 0x90, 0x2C, 0x87, 0x38, 0xE2, 0xD0, 0xEC, 0x16, 0x1F, 0x21, 0x60, 0x2E },
        { 0x53, 0x16, 0x90, 0xD6, 0x31, 0x64, 0x55, 0xDD, 0x00, 0x5D, 0xE3, 0x24, 0x7C, 0xE1, 0xDF, 0x81 },
        { 0xE1, 0x40, 0x64, 0x50, 0xBB, 0x7F, 0x5A, 0x7B, 0x24, 0xF8, 0x38, 0x18, 0x23, 0x45, 0x37, 0x93 }
    } };
}
