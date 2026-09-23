/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

//\NOTE:    This file is part of an attempt to replace version specific code in files.
//          It defines the client version across all packet handling stages. Do not
//          use or work with this file unless you are able to understand what is
//          happening here ;)

#pragma once

#include "AEVersion.hpp"
#include "Platform/SymbolVisibility.hpp"

#include <cstdint>
#include <string_view>

namespace WoW::Build {
    // Suffix prevents collision with legacy bareword macros like '#define Classic 5875'
    inline constexpr uint32_t CLASSIC_BUILD = 5875;
    inline constexpr uint32_t TBC_BUILD = 8606;
    inline constexpr uint32_t WOTLK_BUILD = 12340;
    inline constexpr uint32_t CATA_BUILD = 15595;
    inline constexpr uint32_t MOP_BUILD = 18414;
    inline constexpr uint32_t WOD_BUILD = 20779;
    inline constexpr uint32_t LEGION_BUILD = 26972;
}

namespace WoW {
    enum class Expansion : uint8_t
    {
        _Classic = 0,
        _TBC,
        _WotLK,
        _Cata,
        _Mop,
        _WoD,
        _Legion,
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

    /// Protocol and expansion information for an active client connection
    struct ClientProtocol
    {
        Expansion expansion{Expansion::Unknown};

        [[nodiscard]] WoW::Expansion getExpansion() const { return expansion; }

        // index inside the version tables, the configured expansion when the client version is unknown
        [[nodiscard]] int32_t versionId() const noexcept;

        [[nodiscard]] bool isClassic() const { return expansion == WoW::Expansion::_Classic; }
        [[nodiscard]] bool isTbc() const { return expansion == WoW::Expansion::_TBC; }
        [[nodiscard]] bool isWotlk() const { return expansion == WoW::Expansion::_WotLK; }
        [[nodiscard]] bool isCata() const { return expansion == WoW::Expansion::_Cata; }
        [[nodiscard]] bool isMop() const { return expansion == WoW::Expansion::_Mop; }
        [[nodiscard]] bool isLegacy() const { return isClassic() || isTbc(); }
    };

    /// Global protocol state configured for this server instance
    struct ServerProtocol
    {
        Expansion expansion{Expansion::Unknown};

        [[nodiscard]] Expansion getExpansion() const noexcept { return expansion; }

        [[nodiscard]] constexpr bool isAtLeast(Expansion exp) const noexcept
        {
            if (expansion == Expansion::Unknown || exp == Expansion::Unknown)
            {
                return false;
            }

            return expansion >= exp;
        }

        [[nodiscard]] constexpr bool isLessThan(Expansion exp) const noexcept
        {
            if (expansion == Expansion::Unknown || exp == Expansion::Unknown)
            {
                return false;
            }

            return expansion < exp;
        }

        [[nodiscard]] constexpr bool isBetween(Expansion min, Expansion max) const noexcept
        {
            if (expansion == Expansion::Unknown || min == Expansion::Unknown || max == Expansion::Unknown)
            {
                return false;
            }

            return expansion >= min && expansion <= max;
        }
    };

    // Compile-Time Target Mapping
    // Represents the maximum expansion compiled into this binary via CMake definitions

#if defined(AE_CLASSIC)
    inline constexpr Expansion COMPILED_EXPANSION = Expansion::_Classic;
#elif defined(AE_TBC)
    inline constexpr Expansion COMPILED_EXPANSION = Expansion::_TBC;
#elif defined(AE_WOTLK)
    inline constexpr Expansion COMPILED_EXPANSION = Expansion::_WotLK;
#elif defined(AE_CATA)
    inline constexpr Expansion COMPILED_EXPANSION = Expansion::_Cata;
#elif defined(AE_MOP)
    inline constexpr Expansion COMPILED_EXPANSION = Expansion::_Mop;
#elif defined(AE_WOD)
    inline constexpr Expansion COMPILED_EXPANSION = Expansion::_WoD;
#elif defined(AE_LEGION)
    inline constexpr Expansion COMPILED_EXPANSION = Expansion::_Legion;
#else
    inline constexpr Expansion COMPILED_EXPANSION = Expansion::_WotLK; // Fallback
#endif

    [[nodiscard]] constexpr Expansion getCompiledExpansion() noexcept { return COMPILED_EXPANSION; }

    /// Checks whether the expansion is valid in the enum definition
    [[nodiscard]] constexpr bool isValidExpansion(Expansion const expansion) noexcept
    {
        return expansion != Expansion::Unknown && expansion <= Expansion::MN;
    }

    /// Checks whether THIS compiled server binary can run this expansion
    [[nodiscard]] constexpr bool isSupportedExpansion(Expansion const expansion) noexcept
    {
        return expansion != Expansion::Unknown && expansion <= COMPILED_EXPANSION;
    }

    [[nodiscard]] constexpr uint32_t getBuildForExpansion(Expansion const expansion) noexcept
    {
        switch (expansion)
        {
            case Expansion::_Classic: return Build::CLASSIC_BUILD;
            case Expansion::_TBC:     return Build::TBC_BUILD;
            case Expansion::_WotLK:   return Build::WOTLK_BUILD;
            case Expansion::_Cata:    return Build::CATA_BUILD;
            case Expansion::_Mop:     return Build::MOP_BUILD;
            case Expansion::_WoD:      return Build::WOD_BUILD;
            case Expansion::_Legion:   return Build::LEGION_BUILD;
            default:                  return 0;
        }
    }

    [[nodiscard]] constexpr Expansion getExpansionFromBuild(uint32_t const build) noexcept
    {
        switch (build)
        {
            case Build::CLASSIC_BUILD: return Expansion::_Classic;
            case Build::TBC_BUILD:     return Expansion::_TBC;
            case Build::WOTLK_BUILD:   return Expansion::_WotLK;
            case Build::CATA_BUILD:    return Expansion::_Cata;
            case Build::MOP_BUILD:     return Expansion::_Mop;
            case Build::WOD_BUILD:     return Expansion::_WoD;
            case Build::LEGION_BUILD:  return Expansion::_Legion;
            default:                   return Expansion::Unknown;
        }
    }

    // Runtime Server State
    // Configured via world.conf at startup. Must satisfy: g_serverExpansion <= COMPILED_EXPANSION
    SERVER_DECL extern Expansion g_serverExpansion;

    [[nodiscard]] inline Expansion getServerExpansion() noexcept { return g_serverExpansion; }
    [[nodiscard]] inline uint32_t getServerBuild() noexcept { return getBuildForExpansion(getServerExpansion()); }

    // Checks for active server expansion
    [[nodiscard]] inline bool isServerExpansion(Expansion const expansion) noexcept
    {
        return getServerExpansion() == expansion;
    }

    [[nodiscard]] inline bool isServerExpansionAtLeast(Expansion const expansion) noexcept
    {
        Expansion const serverExp = getServerExpansion();
        if (serverExp == Expansion::Unknown || expansion == Expansion::Unknown)
        {
            return false;
        }

        return serverExp >= expansion;
    }

    [[nodiscard]] inline bool isServerExpansionBetween(Expansion const minExpansion, Expansion const maxExpansion) noexcept
    {
        Expansion const serverExp = getServerExpansion();
        if (serverExp == Expansion::Unknown || minExpansion == Expansion::Unknown || maxExpansion == Expansion::Unknown)
        {
            return false;
        }

        return serverExp >= minExpansion && serverExp <= maxExpansion;
    }

    /// Converts world.conf integer setting (0 = Classic ... 4 = MoP) to enum
    [[nodiscard]] constexpr Expansion expansionFromVersionId(uint32_t const versionId) noexcept
    {
        if (versionId > static_cast<uint32_t>(Expansion::MN))
            return Expansion::Unknown;

        return static_cast<Expansion>(versionId);
    }

    /// Determines if a data table or resource is required for the active server expansion
    [[nodiscard]] inline bool isDataLoadRequired(Expansion const minExpansion, Expansion const maxExpansion = Expansion::MN) noexcept
    {
        return isServerExpansionBetween(minExpansion, maxExpansion);
    }

    /// Legacy wrapper: Build number used for database build filters
    [[nodiscard]] inline uint32_t getConfigBuild() noexcept { return getServerBuild(); }

    /// Returns the array index for opcode/version tables (0 = Classic ... 4 = MoP), or -1 if unsupported
    [[nodiscard]] constexpr int32_t getOpcodeTableIndex(Expansion const expansion) noexcept
    {
        if (expansion == Expansion::Unknown || expansion > Expansion::_Mop)
        {
            return -1;
        }

        return static_cast<int32_t>(expansion);
    }

//
//    [[nodiscard]] constexpr ClientVersion getClientVersion(Expansion expansion) noexcept
//    {
//        switch (expansion)
//        {
//            case Expansion::_Classic: return ClientVersion{1, 12, 1};
//            case Expansion::_TBC: return ClientVersion{2, 4, 3};
//            case Expansion::_WotLK: return ClientVersion{3, 3, 5};
//            case Expansion::_Cata: return ClientVersion{4, 3, 4};
//            case Expansion::_Mop: return ClientVersion{5, 4, 8};
//
//            // Default placeholder for expansions beyond the core project scope
//            default: return ClientVersion{ 0, 0, 0 };
//        }
//    }
//

    /// Returns the full name of an expansion (useful for logs and messages)
    [[nodiscard]] constexpr std::string_view getExpansionName(Expansion const expansion) noexcept
    {
        switch (expansion)
        {
            case Expansion::_Classic: return "Classic";
            case Expansion::_TBC: return "The Burning Crusade";
            case Expansion::_WotLK: return "Wrath of the Lich King";
            case Expansion::_Cata: return "Cataclysm";
            case Expansion::_Mop: return "Mists of Pandaria";
            case Expansion::_WoD: return "Warlords of Draenor";
            case Expansion::_Legion: return "Legion";
            case Expansion::BfA: return "Battle for Azeroth";
            case Expansion::SL: return "Shadowlands";
            case Expansion::DF: return "Dragonflight";
            case Expansion::TWW: return "The War Within";
            case Expansion::MN: return "Midnight";
            case Expansion::Unknown:
            default: return "Unknown expansion";
        }
    }

    /// Returns the short abbreviation of an expansion (useful for compact logs and tags)
    [[nodiscard]] constexpr std::string_view getShortExpansionName(Expansion const expansion) noexcept
    {
        switch (expansion)
        {
            case Expansion::_Classic: return "Classic";
            case Expansion::_TBC:     return "TBC";
            case Expansion::_WotLK:   return "WotLK";
            case Expansion::_Cata:    return "Cata";
            case Expansion::_Mop:     return "MoP";
            case Expansion::_WoD:      return "WoD";
            case Expansion::_Legion:   return "Legion";
            case Expansion::BfA:      return "BfA";
            case Expansion::SL:       return "Shadowlands";
            case Expansion::DF:       return "Dragonflight";
            case Expansion::TWW:      return "TWW";
            case Expansion::MN:       return "Midnight";
            case Expansion::Unknown:
            default:                  return "Unknown";
        }
    }
}
