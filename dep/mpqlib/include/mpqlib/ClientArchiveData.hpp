/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// Data that is a property of a WoW client itself, not of what any particular
// tool needs from it - shared by every tool that opens a client's MPQ
// archives at runtime (map_extractor, vmap4_extractor, creature_data, ...).
// Different tools do genuinely need different Cata/Mop base-archive sets
// (map_extractor only needs DBC/map/terrain data; vmap4_extractor and
// creature_data both also read model/WMO geometry, so they need the archives
// that ship it) - "centralized" here means named per-need variants, not one
// forced-common list.

#pragma once

#include "mpqlib/ClientVersion.hpp"

#include <cstdint>
#include <vector>

namespace mpqlib
{
    // Every locale code a retail client could ship, matching a Data/<code>/
    // folder name. Complete list - some tools have historically carried
    // shorter, hand-typed copies of this that were quietly missing entries
    // (esMX/ptBR/ptPT/itIT).
    inline std::vector<char const*> const kLocales = {
        "enGB", "enUS", "deDE", "esES", "frFR", "koKR", "zhCN", "zhTW",
        "enCN", "enTW", "esMX", "ruRU", "ptBR", "ptPT", "itIT"
    };

    // Cata/Mop ship their DBC/model/map data across a base MPQ set plus a
    // long, version-specific chain of incremental "wow-update-*" patch
    // archives - one per client build that ever shipped for that expansion.
    // These build-number lists are exactly that patch history, independent
    // of which archives a given tool opens.
    std::vector<uint32_t> const& incrementalPatchBuilds(ClientVersion version);

    // Two points in a Cata/Mop client's patch history where the naming
    // convention for locating DBC/patch data changes:
    //  - lastDbcInDataBuild: patches at or below this build still keep DBC
    //    data directly under Data/, not the locale subfolder; above it,
    //    "wow-update-base-<build>.MPQ" replaces "wow-update-<build>.MPQ".
    //  - newBaseSetBuild: the build at which the archive base set itself
    //    changed (e.g. Cata's world2.MPQ only exists from this build on).
    // Cata and Mop have genuinely different values for both (Mop's are much
    // higher build numbers - it has its own, later patch history) - always
    // use the accessor for the client version actually in hand rather than
    // assuming one pair covers both expansions.
    uint32_t lastDbcInDataBuild(ClientVersion version);
    uint32_t newBaseSetBuild(ClientVersion version);

    // Cata/Mop base MPQ set for tools that only need DBC/map/terrain data
    // (map_extractor). No model/WMO-carrying archives (Mop's model.MPQ) -
    // adding those to a tool that doesn't read models would just be dead
    // weight on every open.
    std::vector<std::string> const& dbcAndMapArchiveList(ClientVersion version);

    // Cata/Mop base MPQ set for tools that also read model/WMO geometry
    // (vmap4_extractor, creature_data - both open individual .m2/WMO files
    // directly). Mop's set includes model.MPQ, which the DBC/map-only list
    // above doesn't need. Cata's set is the same *archives* as the list
    // above, only in a different order - patch-chain order can change which
    // file wins when the same path exists in more than one archive, so this
    // stays a distinct list rather than being reordered to match.
    std::vector<std::string> const& modelAndVmapArchiveList(ClientVersion version);
}
