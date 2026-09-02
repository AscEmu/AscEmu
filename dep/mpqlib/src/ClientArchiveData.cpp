/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/ClientArchiveData.hpp"

namespace mpqlib
{
    namespace
    {
        std::vector<uint32_t> const kCataBuilds = {
            13164, 13205, 13287, 13329, 13596, 13623, 13914, 14007, 14333,
            14480, 14545, 15005, 15050, 15211, 15354, 15595
        };
        std::vector<uint32_t> const kMopBuilds = {
            16016, 16048, 16057, 16309, 16357, 16516, 16650, 16844, 16965,
            17116, 17266, 17325, 17331, 17345, 17538, 17645, 17688, 17898, 18273
        };

        constexpr uint32_t kCataLastDbcInDataBuild = 13623;
        constexpr uint32_t kCataNewBaseSetBuild = 15211;
        constexpr uint32_t kMopLastDbcInDataBuild = 15595;
        constexpr uint32_t kMopNewBaseSetBuild = 16016;

        std::vector<std::string> const kDbcMapCataList = {
            "world.MPQ", "art.MPQ", "world2.MPQ", "expansion1.MPQ", "expansion2.MPQ", "expansion3.MPQ"
        };
        std::vector<std::string> const kDbcMapMopList = {
            "world.MPQ", "misc.MPQ", "expansion1.MPQ", "expansion2.MPQ", "expansion3.MPQ", "expansion4.MPQ"
        };

        std::vector<std::string> const kModelVmapCataList = {
            "world.MPQ", "art.MPQ", "expansion1.MPQ", "expansion2.MPQ", "expansion3.MPQ", "world2.MPQ"
        };
        std::vector<std::string> const kModelVmapMopList = {
            "world.MPQ", "model.MPQ", "misc.MPQ", "expansion1.MPQ", "expansion2.MPQ", "expansion3.MPQ", "expansion4.MPQ"
        };
    }

    std::vector<uint32_t> const& incrementalPatchBuilds(ClientVersion version)
    {
        return version == ClientVersion::MistsOfPandaria ? kMopBuilds : kCataBuilds;
    }

    uint32_t lastDbcInDataBuild(ClientVersion version)
    {
        return version == ClientVersion::MistsOfPandaria ? kMopLastDbcInDataBuild : kCataLastDbcInDataBuild;
    }

    uint32_t newBaseSetBuild(ClientVersion version)
    {
        return version == ClientVersion::MistsOfPandaria ? kMopNewBaseSetBuild : kCataNewBaseSetBuild;
    }

    std::vector<std::string> const& dbcAndMapArchiveList(ClientVersion version)
    {
        return version == ClientVersion::MistsOfPandaria ? kDbcMapMopList : kDbcMapCataList;
    }

    std::vector<std::string> const& modelAndVmapArchiveList(ClientVersion version)
    {
        return version == ClientVersion::MistsOfPandaria ? kModelVmapMopList : kModelVmapCataList;
    }
}
