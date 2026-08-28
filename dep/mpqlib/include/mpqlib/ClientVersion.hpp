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
        Vanilla = 5875,
        BurningCrusade = 8606,
        WrathOfTheLichKing = 12340,
        Cataclysm = 15595,
        MistsOfPandaria = 18414,
    };

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
