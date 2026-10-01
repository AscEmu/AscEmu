/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "world/Server/ClientProtocol.hpp"

#include <array>
#include <cstdint>
#include <string_view>

// world connection of 6.2.4 clients (build 21742)
namespace AscEmu::Version::WorldProfile
{
    inline constexpr WoW::Expansion Expansion = WoW::Expansion::_WoD;
    inline constexpr std::string_view Name = "WoD";

    // both initializers start with this magic and a length prefix
    inline constexpr bool InitializerHasMagic = true;
    inline constexpr uint32_t InitializerMagic = 0x0F5EB1CE;
    inline constexpr std::string_view ServerInitializer = "WORLD OF WARCRAFT CONNECTION - SERVER TO CLIENT";
    // the client sends its string with the terminating zero
    inline constexpr std::string_view ClientInitializer{ "WORLD OF WARCRAFT CONNECTION - CLIENT TO SERVER", 48 };

    // 4 byte header (uint16 size, uint16 opcode) until CMSG_AUTH_SESSION, 6 byte header afterwards
    inline constexpr bool SetupHeaderBeforeAuthSession = true;

    // size limit of a client packet without the opcode
    inline constexpr uint32_t MaxClientPacketSize = 10240;

    // the world encryption starts right after the authentication
    inline constexpr bool EnableEncryptionHandshake = false;

    // the digest key is SHA256 of the key data alone
    inline constexpr std::array<std::array<uint8_t, 16>, 0> AuthSeeds{};
}
