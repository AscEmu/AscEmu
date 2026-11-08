/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "world/Server/ClientProtocol.hpp"

#include <array>
#include <cstdint>
#include <string_view>

// world connection of the WoW Forever 1.60.1 client (build 70862), the wide V2 transport of the 12.x engine
namespace AscEmu::Version::WorldProfile
{
    inline constexpr WoW::Expansion Expansion = WoW::Expansion::_WoWForever;
    inline constexpr std::string_view Name = "Forever";

    // plain strings closed by a line feed
    inline constexpr std::string_view ServerInitializer = "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT - V2\n";
    inline constexpr std::string_view ClientInitializer = "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER - V2\n";

    // size limit of a client packet with the opcode
    inline constexpr uint32_t MaxClientPacketSize = 0x10000;

    // no auth key of a Forever build is known; without seeds the session digest is not verified
    inline constexpr std::array<std::array<uint8_t, 16>, 0> AuthSeeds{};
}
