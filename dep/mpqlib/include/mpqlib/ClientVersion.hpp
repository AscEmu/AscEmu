/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// A runtime representation of "which WoW client are we looking at", for
// tools that need to steer behavior per client version without being
// compiled separately per version the way the rest of AscEmu is (see
// src/shared/AEVersion.hpp's VERSION_STRING). Deliberately not named
// Classic/TBC/WotLK/Cata/Mop: AEVersion.hpp #defines exactly those five
// identifiers to raw integers with no #undef, so using them here would
// make this enum unusable in any translation unit that also includes
// AEVersion.hpp (the preprocessor would rewrite e.g. ClientVersion::Cata
// into ClientVersion::15595 before the compiler ever sees it).

#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>

namespace mpqlib
{
    // Values are the same reference build numbers AscEmu already uses as its
    // per-branch VERSION_STRING constants (src/shared/AEVersion.hpp) - the
    // lowest build number of each expansion's lifetime, used as a range
    // threshold by clientVersionFromBuild() below rather than as the only
    // recognized value per expansion.
    enum class ClientVersion : uint32_t
    {
        Vanilla = 5875, // 1.12.1
        BurningCrusade = 8606, // 2.4.3
        WrathOfTheLichKing = 12340, // 3.3.5a
        Cataclysm = 15595, // 4.3.4
        MistsOfPandaria = 18414, // 5.4.8
        WarlordsOfDraenor = 21742, // 6.2.4
        Legion = 26972, // 7.3.5
        BattleForAzeroth = 35662, // 8.3.7
        Shadowlands = 45745, // 9.2.7
        Dragonflight = 55664, // 10.2.7
        TheWarWithin = 65000, // 11.x
        Midnight = 75000, // 12.x
        Forever = 69913 // WoW Forever Beta Client
    };

    enum class ClientSupportStatus
    {
        Unsupported, // Expansions like BfA, SL, DF, TWW, Midnight
        Db2Only,     // Verified DB2 extraction, Map/ADT pipeline pending (WoD, Legion, Forever)
        Full         // Full support: DB2, Maps, VMaps, MMaps (Vanilla to MoP)
    };

    [[nodiscard]] constexpr std::string_view getClientVersionName(ClientVersion version) noexcept
    {
        switch (version)
        {
            case ClientVersion::Vanilla:            return "Classic (Vanilla)";
            case ClientVersion::BurningCrusade:     return "The Burning Crusade";
            case ClientVersion::WrathOfTheLichKing: return "Wrath of the Lich King";
            case ClientVersion::Cataclysm:          return "Cataclysm";
            case ClientVersion::MistsOfPandaria:    return "Mists of Pandaria";
            case ClientVersion::WarlordsOfDraenor:  return "Warlords of Draenor";
            case ClientVersion::Legion:             return "Legion";
            case ClientVersion::BattleForAzeroth:   return "Battle for Azeroth";
            case ClientVersion::Shadowlands:        return "Shadowlands";
            case ClientVersion::Dragonflight:       return "Dragonflight";
            case ClientVersion::TheWarWithin:       return "The War Within";
            case ClientVersion::Midnight:           return "Midnight";
            case ClientVersion::Forever:            return "WoW Forever";
            default:                                return "Unknown";
        }
    }

    [[nodiscard]] uint32_t getDetectedBuildNumber();

    // Every expansion starting with WoD uses CASC instead of MPQ
    [[nodiscard]] constexpr bool isCascClient(ClientVersion version) noexcept
    {
        switch (version)
        {
            case ClientVersion::WarlordsOfDraenor:
            case ClientVersion::Legion:
            case ClientVersion::BattleForAzeroth:
            case ClientVersion::Shadowlands:
            case ClientVersion::Dragonflight:
            case ClientVersion::TheWarWithin:
            case ClientVersion::Midnight:
            case ClientVersion::Forever:
                return true;
            default:
                return false;
        }
    }

    // Flags which expansions have validated definitions and extraction support in AscEmu
    [[nodiscard]] constexpr bool isTestedClient(ClientVersion version) noexcept
    {
        switch (version)
        {
            case ClientVersion::Vanilla:
            case ClientVersion::BurningCrusade:
            case ClientVersion::WrathOfTheLichKing:
            case ClientVersion::Cataclysm:
            case ClientVersion::MistsOfPandaria:
            case ClientVersion::WarlordsOfDraenor:
            case ClientVersion::Legion:
            case ClientVersion::Forever:
                return true;
            default:
                return false; // BfA, SL, DF, TWW, Midnight, Forever are currently experimental / untested
        }
    }

    // Flags which expansions have full support for DB2, Maps, VMaps, and MMaps in AscEmu
    [[nodiscard]] constexpr ClientSupportStatus getClientSupportStatus(ClientVersion version) noexcept
    {
        switch (version)
        {
            case ClientVersion::Vanilla:
            case ClientVersion::BurningCrusade:
            case ClientVersion::WrathOfTheLichKing:
            case ClientVersion::Cataclysm:
            case ClientVersion::MistsOfPandaria:
                return ClientSupportStatus::Full;

            case ClientVersion::WarlordsOfDraenor:
            case ClientVersion::Legion:
            case ClientVersion::Forever:
                return ClientSupportStatus::Db2Only;

            default:
                return ClientSupportStatus::Unsupported;
        }
    }

    // Flags which expansions have full support for map extraction in AscEmu
    [[nodiscard]] constexpr bool supportsMapExtraction(ClientVersion version) noexcept
    {
        return getClientSupportStatus(version) == ClientSupportStatus::Full;
    }

    // Classifies a raw build number - obtained by whatever means a caller
    // has available (an MPQ/component patch manifest, this project's own
    // extracted .map file header, ...) - into a ClientVersion. A real
    // client's reported build is whatever patch level it happens to be, not
    // necessarily one of the five ClientVersion values themselves, so this
    // classifies by range (build numbers increase monotonically release-
    // over-release with no overlap between expansions) rather than requiring
    // an exact match. Returns std::nullopt for build == 0 (never detected).
    std::optional<ClientVersion> clientVersionFromBuild(uint32_t build);

    // Scans the Wow.exe found directly inside clientRoot for one of a fixed
    // set of known build-number byte patterns (the same approach map_extractor
    // has always used to tell client versions apart before opening any MPQ -
    // needed because which MPQs/patch chain layout to even look for already
    // depends on the version). Returns std::nullopt if no Wow.exe is found in
    // clientRoot, or its build doesn't match any known pattern.
    std::optional<ClientVersion> detectClientVersion(std::filesystem::path const& clientRoot);
}
