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

    // Legacy Builds (Final Patch)
    inline constexpr uint32_t CLASSIC_BUILD = 5875; // 1.12.1
    inline constexpr uint32_t TBC_BUILD = 8606; // 2.4.3
    inline constexpr uint32_t WOTLK_BUILD = 12340; // 3.3.5a
    inline constexpr uint32_t CATA_BUILD = 15595; // 4.4.0
    inline constexpr uint32_t MOP_BUILD = 18414; // 5.4.8

    // Mainline / Retail Builds (Final Patch)
    inline constexpr uint32_t WOD_BUILD = 21742; // 6.2.4
    inline constexpr uint32_t LEGION_BUILD = 26972; // 7.3.5
    inline constexpr uint32_t BFA_BUILD = 35662; // 8.3.7
    inline constexpr uint32_t SHADOWLANDS_BUILD = 45745; // 9.2.7
    inline constexpr uint32_t DRAGONFLIGHT_BUILD = 55664; // 10.2.7
    inline constexpr uint32_t TWW_BUILD = 65299; // 11.2.7
    inline constexpr uint32_t MIDNIGHT_BUILD = 70575; // 12.1.5

    // TODO: Check versions and builds for the following expansions, they are not final and may change in the future
    // Modern Classic Timeline
    inline constexpr uint32_t CLASSIC_ERA_BUILD = 70003;  // 1.15.9
    inline constexpr uint32_t TBC_CLASSIC_BUILD = 44833;  // 2.5.4
    inline constexpr uint32_t WOTLK_CLASSIC_BUILD = 58936;  // 3.4.3
    inline constexpr uint32_t CATA_CLASSIC_BUILD = 59069;  // 4.4.1
    inline constexpr uint32_t MOP_CLASSIC_BUILD = 70579; //  5.5.4

    // WoW Forever (1.60) is a special case, it is a modern client with a legacy major version (1.x) and a build higher than the last MoP build (5.4.8)
    inline constexpr uint32_t FOREVER_BUILD = 70532; // 1.60.1 (starts 2026-11-04)
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
        _BfA,
        _Shadowlands,
        _Dragonflight,
        _TheWarWithin,
        _Midnight,

        // Modern classic timeline (own space begin with 50)
        // (1.14.4 Classic Era, 2.5 TBC Classic, 3.4 WotLK Classic, 4.4 Cata Classic, 5.x MoP Classic, 1.60 WoW Forever
        _Classic_Era = 50,
        _TBC_Classic,
        _WotLK_Classic,
        _Cata_Classic,
        _MoP_Classic,
        _WoWForever,
        Unknown = 255
    };

    enum class ClientFlavor : uint8_t
    {
        LEGACY,         // Original 1.12, 2.4.3, 3.3.5a, 5.4.8
        MODERN_CLASSIC, // TBC Classic (2.5), WotLK Classic (3.4), WoW Forever (1.60)
        MAINLINE,       // WoD (6.x) to Midnight (12.x) and future expansions
        Unknown         // Unrecognized client flavor
    };

    struct ClientVersion
    {
        uint8_t major{0};
        uint8_t minor{0};
        uint8_t patch{0};
        uint32_t build{0};

        [[nodiscard]] constexpr bool isLegacy() const noexcept
        {
            // Original clients (2004–2014) had low builds
            return build <= Build::MOP_BUILD; // <= 5.4.8 (MoP)
        }

        [[nodiscard]] constexpr bool isModernClassic() const noexcept
        {
            // Modern Client-Core (Build > 20000) but Legacy-Major (Classic, TBC, WotLK, Cata, MoP)
            return !isLegacy() && major <= 5;
        }
    };

    inline Expansion resolveExpansion(const ClientVersion& clientVersion)
    {
        // Legacy Clients (Classic/Vanilla, TBC, WotLK, Cata, MoP)
        if (clientVersion.isLegacy())
        {
            switch (clientVersion.build)
            {
                case Build::CLASSIC_BUILD: return Expansion::_Classic;
                case Build::TBC_BUILD:     return Expansion::_TBC;
                case Build::WOTLK_BUILD:   return Expansion::_WotLK;
                case Build::CATA_BUILD:    return Expansion::_Cata;
                case Build::MOP_BUILD:     return Expansion::_Mop;
                default:                   return Expansion::Unknown;
            }
        }

        // Modern Classic Clients (overlapping builds, but Major <= 5)
        if (clientVersion.isModernClassic())
        {
            switch (clientVersion.major)
            {
                case 1: return (clientVersion.minor >= 60) ? Expansion::_WoWForever : Expansion::_Classic_Era;
                case 2: return Expansion::_TBC_Classic;   // 2.5.x
                case 3: return Expansion::_WotLK_Classic; // 3.4.x
                case 4: return Expansion::_Cata_Classic;  // 4.4.x
                case 5: return Expansion::_MoP_Classic;   // 5.x Classic
                default: return Expansion::Unknown;
            }
        }

        // Retail / Mainline (Major >= 6)
        switch (clientVersion.major)
        {
            case 6:  return Expansion::_WoD;
            case 7:  return Expansion::_Legion;
            case 8:  return Expansion::_BfA;
            case 9:  return Expansion::_Shadowlands;
            case 10: return Expansion::_Dragonflight;
            case 11: return Expansion::_TheWarWithin;
            case 12: return Expansion::_Midnight;
            default: return Expansion::Unknown;
        }
    }

    /// Protocol and expansion information for an active client connection
    struct ClientProtocol
    {
        Expansion expansion{Expansion::Unknown};
        uint32_t regionId{0};
        uint32_t battlegroupId{0};
        uint32_t realmId{0};

        [[nodiscard]] Expansion getExpansion() const { return expansion; }

        [[nodiscard]] uint32_t getVirtualRealmAddress() const noexcept
        {
            return ((regionId & 0xFFU) << 24U) | ((battlegroupId & 0xFFU) << 16U) | (realmId & 0xFFFFU);
        }

        // index inside the version tables, the configured expansion when the client version is unknown
        [[nodiscard]] int32_t versionId() const noexcept;

        [[nodiscard]] bool isClassic() const { return expansion == Expansion::_Classic; }
        [[nodiscard]] bool isTbc() const { return expansion == Expansion::_TBC; }
        [[nodiscard]] bool isWotlk() const { return expansion == Expansion::_WotLK; }
        [[nodiscard]] bool isCata() const { return expansion == Expansion::_Cata; }
        [[nodiscard]] bool isMop() const { return expansion == Expansion::_Mop; }
        [[nodiscard]] bool isWoD() const { return expansion == Expansion::_WoD; }
        [[nodiscard]] bool isLegion() const { return expansion == Expansion::_Legion; }
        [[nodiscard]] bool isBfA() const { return expansion == Expansion::_BfA; }
        [[nodiscard]] bool isShadowlands() const noexcept { return expansion == Expansion::_Shadowlands; }
        [[nodiscard]] bool isDragonflight() const { return expansion == Expansion::_Dragonflight; }
        [[nodiscard]] bool isTheWarWithin() const { return expansion == Expansion::_TheWarWithin; }
        [[nodiscard]] bool isMidnight() const { return expansion == Expansion::_Midnight; }
        [[nodiscard]] bool isForever() const { return expansion == Expansion::_WoWForever; }
        // the 11.2.7 layouts are the baseline of every later client, the branches of 11.x serve 12.x as well
        [[nodiscard]] bool isTheWarWithinOrLater() const
        {
            return expansion != Expansion::Unknown && expansion >= Expansion::_TheWarWithin && expansion <= Expansion::_Midnight;
        }

        [[nodiscard]] bool isValid() const noexcept
        {
            return expansion != Expansion::Unknown;
        }

        [[nodiscard]] bool isMainlineAtLeast(Expansion exp) const noexcept
        {
            return isValid() && expansion >= exp && expansion < Expansion::_Classic_Era;
        }

        [[nodiscard]] bool isMainlineAtMost(Expansion exp) const noexcept
        {
            return isValid() && expansion <= exp;
        }

        /// Content expansion is the expansion of the content the client can access
        [[nodiscard]] Expansion getContentExpansion() const noexcept
        {
            switch (expansion)
            {
                case Expansion::_Classic_Era:   return Expansion::_Classic;
                case Expansion::_TBC_Classic:   return Expansion::_TBC;
                case Expansion::_WotLK_Classic: return Expansion::_WotLK;
                case Expansion::_Cata_Classic:  return Expansion::_Cata;
                case Expansion::_MoP_Classic:   return Expansion::_Mop;
                default:                        return expansion; // Retail is content == expansion
            }
        }

        bool isContentAtLeast(Expansion exp) const noexcept
        {
            const auto base = getContentExpansion();
            return isValid() && base >= exp && base < Expansion::_Classic_Era;
        }
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
#elif defined(AE_BFA)
    inline constexpr Expansion COMPILED_EXPANSION = Expansion::_BfA;
#elif defined(AE_SHADOWLANDS)
    inline constexpr Expansion COMPILED_EXPANSION = Expansion::_Shadowlands;
#elif defined(AE_DRAGONFLIGHT)
    inline constexpr Expansion COMPILED_EXPANSION = Expansion::_Dragonflight;
#elif defined(AE_THEWARWITHIN)
    inline constexpr Expansion COMPILED_EXPANSION = Expansion::_TheWarWithin;
#elif defined(AE_MIDNIGHT)
    inline constexpr Expansion COMPILED_EXPANSION = Expansion::_Midnight;
#else
    inline constexpr Expansion COMPILED_EXPANSION = Expansion::_WotLK; // Fallback
#endif

    [[nodiscard]] constexpr Expansion getCompiledExpansion() noexcept { return COMPILED_EXPANSION; }

    /// Checks whether the expansion is valid in the enum definition
    [[nodiscard]] constexpr bool isValidExpansion(Expansion const expansion) noexcept
    {
        return expansion != Expansion::Unknown && expansion <= Expansion::_Midnight;
    }

    /// Checks whether THIS compiled server binary can run this expansion
    [[nodiscard]] constexpr bool isSupportedExpansion(Expansion const expansion) noexcept
    {
        return expansion != Expansion::Unknown && expansion <= COMPILED_EXPANSION;
    }

    [[nodiscard]] constexpr bool isModernClassic(Expansion expansion) noexcept
    {
        return expansion >= Expansion::_Classic_Era && expansion <= Expansion::_WoWForever;
    }

    [[nodiscard]] constexpr bool isMainline(Expansion expansion) noexcept
    {
        return expansion >= Expansion::_WoD && expansion <= Expansion::_Midnight;
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
            case Expansion::_WoD:     return Build::WOD_BUILD;
            case Expansion::_Legion:  return Build::LEGION_BUILD;
            case Expansion::_BfA:     return Build::BFA_BUILD;
            case Expansion::_Shadowlands:         return Build::SHADOWLANDS_BUILD;
            case Expansion::_Dragonflight:        return Build::DRAGONFLIGHT_BUILD;
            case Expansion::_TheWarWithin:        return Build::TWW_BUILD;
            case Expansion::_Midnight:            return Build::MIDNIGHT_BUILD;
            default:                  return 0;
        }
    }

    /// Expansion of the client data (dbc, maps) the server loads for an expansion.
    /// WoD, Legion, BfA, Shadowlands, Dragonflight, The War Within and Midnight run on the Mop client data until their own data is available.
    /// The database rows are filtered with the build of the expansion itself, see getDBDataBuildExpansion().
    [[nodiscard]] constexpr Expansion getDataExpansion(Expansion const expansion) noexcept
    {
        if (expansion >= Expansion::_WoD && expansion <= Expansion::_Midnight)
            return Expansion::_Mop;

        return expansion;
    }

    /// Expansions whose own DB2 tables the server loads (data_<expansion>/dbc) instead of the Mop dbc files.
    [[nodiscard]] constexpr bool hasOwnClientData(Expansion const expansion) noexcept
    {
        return expansion == Expansion::_Midnight;
    }

    /// Build the database rows (properties, spawns, quests, events, ...) are filtered with for an expansion:
    /// every expansion loads the rows of its own build, the tables carry the rows per build.
    [[nodiscard]] constexpr uint32_t getDBDataBuildExpansion(Expansion const expansion) noexcept
    {
        return getBuildForExpansion(expansion);
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
            case Build::BFA_BUILD:     return Expansion::_BfA;
            case Build::SHADOWLANDS_BUILD: return Expansion::_Shadowlands;
            case Build::DRAGONFLIGHT_BUILD: return Expansion::_Dragonflight;
            case Build::TWW_BUILD: return Expansion::_TheWarWithin;
            case Build::MIDNIGHT_BUILD: return Expansion::_Midnight;
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
        Expansion const serverExpansion = getServerExpansion();
        if (serverExpansion == Expansion::Unknown || expansion == Expansion::Unknown)
        {
            return false;
        }

        // Prevent false comparisons between modern classic and legacy expansions
        if (isModernClassic(serverExpansion) != isModernClassic(expansion))
            return false;

        return serverExpansion >= expansion;
    }

    [[nodiscard]] inline bool isServerExpansionBetween(Expansion const minExpansion, Expansion const maxExpansion) noexcept
    {
        Expansion const serverExpansion = getServerExpansion();
        if (serverExpansion == Expansion::Unknown || minExpansion == Expansion::Unknown || maxExpansion == Expansion::Unknown)
        {
            return false;
        }

        // Prevent false comparisons between modern classic and legacy expansions
        if (isModernClassic(serverExpansion) != isModernClassic(minExpansion))
            return false;

        return serverExpansion >= minExpansion && serverExpansion <= maxExpansion;
    }

    /// Converts world.conf integer setting (0 = Classic ... 4 = MoP) to enum
    [[nodiscard]] constexpr Expansion expansionFromVersionId(uint32_t const versionId) noexcept
    {
        if (versionId > static_cast<uint32_t>(Expansion::_Midnight))
            return Expansion::Unknown;

        return static_cast<Expansion>(versionId);
    }

    /// Determines if a data table or resource is required for the active server expansion
    [[nodiscard]] inline bool isDataLoadRequired(Expansion const minExpansion, Expansion const maxExpansion = Expansion::_Midnight) noexcept
    {
        return isServerExpansionBetween(minExpansion, maxExpansion);
    }

    /// Expansion whose dbc files and layouts the server loads
    [[nodiscard]] inline Expansion getServerDataExpansion() noexcept { return getDataExpansion(getServerExpansion()); }

    /// Legacy wrapper: Build number used for database build filters
    [[nodiscard]] inline uint32_t getConfigBuild() noexcept { return getDBDataBuildExpansion(getServerExpansion()); }

    /// Returns the array index for opcode/version tables (0 = Classic ... 11 = Midnight), or -1 if unsupported
    [[nodiscard]] constexpr int32_t getOpcodeTableIndex(Expansion const expansion) noexcept
    {
        if (expansion == Expansion::Unknown || expansion > Expansion::_Midnight)
        {
            return -1;
        }

        return static_cast<int32_t>(expansion);
    }

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
            case Expansion::_BfA: return "Battle for Azeroth";
            case Expansion::_Shadowlands: return "Shadowlands";
            case Expansion::_Dragonflight: return "Dragonflight";
            case Expansion::_TheWarWithin: return "The War Within";
            case Expansion::_Midnight: return "Midnight";
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
            case Expansion::_BfA:      return "BfA";
            case Expansion::_Shadowlands:       return "Shadowlands";
            case Expansion::_Dragonflight:      return "Dragonflight";
            case Expansion::_TheWarWithin:      return "TWW";
            case Expansion::_Midnight:          return "Midnight";
            case Expansion::Unknown:
            default:                  return "Unknown";
        }
    }
}
