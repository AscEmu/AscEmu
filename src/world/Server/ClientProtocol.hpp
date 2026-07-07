/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

//\NOTE:    This file is part of an attempt to replace version specific code in files.
//          It defines the client version across all packet handling stages. Do not
//          use or work with this file unless you are able to understand what is
//          happening here ;)

#pragma once

#include <cstdint>
#include <string_view>

namespace WoW {
    enum class Expansion : uint8_t
    {
        _Classic = 0,
        _TBC,
        _WotLK,
        _Cata,
        _Mop,
        WoD,
        Legion,
        BfA,
        SL,
        DF,
        TWW,
        MN,
        Unknown = 255
    };

    struct ClientVersion
    {
        uint8_t major;
        uint8_t minor;
        uint8_t patch;
    };

    struct ClientProtocolState
    {
        Expansion expansion{Expansion::Unknown};

        [[nodiscard]] constexpr int32_t versionId() const noexcept
        {
            if (expansion == Expansion::Unknown)
            {
                return -1;
            }

            return static_cast<int32_t>(expansion);
        }

        [[nodiscard]] WoW::Expansion getExpansion() const { return expansion; }

        [[nodiscard]] int32_t getVersionId() const { return versionId(); }

        [[nodiscard]] bool isClassic() const { return expansion == WoW::Expansion::_Classic; }
        [[nodiscard]] bool isTbc() const { return expansion == WoW::Expansion::_TBC; }
        [[nodiscard]] bool isWotlk() const { return expansion == WoW::Expansion::_WotLK; }
        [[nodiscard]] bool isCata() const { return expansion == WoW::Expansion::_Cata; }
        [[nodiscard]] bool isMop() const { return expansion == WoW::Expansion::_Mop; }
        [[nodiscard]] bool isLegacy() const { return isClassic() || isTbc(); }
    };

#ifndef ASC_DEFAULT_EXPANSION
#define ASC_DEFAULT_EXPANSION 4
#endif

    inline constexpr Expansion buildExpansion = static_cast<Expansion>(ASC_DEFAULT_EXPANSION);

    // Dynamic getter: Currently returns the fixed build value, later it will read from world.conf
    [[nodiscard]] inline Expansion getCurrentExpansion() noexcept
    {
        return buildExpansion; // TODO: Change to sWorld.getConfigExpansion() later
    }

    [[nodiscard]] constexpr ClientVersion getClientVersion(Expansion expansion) noexcept
    {
        switch (expansion)
        {
            case Expansion::_Classic: return ClientVersion{1, 12, 1};
            case Expansion::_TBC: return ClientVersion{2, 4, 3};
            case Expansion::_WotLK: return ClientVersion{3, 3, 5};
            case Expansion::_Cata: return ClientVersion{4, 3, 4};
            case Expansion::_Mop: return ClientVersion{5, 4, 8};

            // Default placeholder for expansions beyond the core project scope
            default: return ClientVersion{ 0, 0, 0 };
        }
    }

    [[nodiscard]] constexpr uint32_t getClientBuild(Expansion expansion) noexcept
    {
        switch (expansion)
        {
            case Expansion::_Classic: return 5875;
            case Expansion::_TBC: return 8606;
            case Expansion::_WotLK: return 12340;
            case Expansion::_Cata: return 15595;
            case Expansion::_Mop: return 18414;

            default: return 0;
        }
    }

    [[nodiscard]] constexpr std::string_view getExpansionName(Expansion expansion) noexcept
    {
        switch (expansion)
        {
            case Expansion::_Classic: return "Classic";
            case Expansion::_TBC: return "The Burning Crusade";
            case Expansion::_WotLK: return "Wrath of the Lich King";
            case Expansion::_Cata: return "Cataclysm";
            case Expansion::_Mop: return "Mists of Pandaria";
            case Expansion::WoD: return "Warlords of Draenor";
            case Expansion::Legion: return "Legion";
            case Expansion::BfA: return "Battle for Azeroth";
            case Expansion::SL: return "Shadowlands";
            case Expansion::DF: return "Dragonflight";
            case Expansion::TWW: return "The War Within";
            case Expansion::MN: return "Midnight";
            case Expansion::Unknown:
            default: return "Unknown expansion";
        }
    }
}
