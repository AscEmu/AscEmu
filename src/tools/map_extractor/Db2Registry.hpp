/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <string_view>
#include <span>

namespace MapExtractor::DB2
{
    struct Db2Entry
    {
        uint32_t fileDataId;
        uint32_t tableHash;
        std::string_view fileName;
    };

    struct BuildDefinition
    {
        std::string_view expansionName;
        uint32_t minBuild;
        uint32_t maxBuild;
        std::span<const Db2Entry> files;
    };

    // List of known build definitions by build number ranges. This list should be updated as new builds are released.
    const BuildDefinition* findBuildDefinition(uint32_t buildNumber);
}
