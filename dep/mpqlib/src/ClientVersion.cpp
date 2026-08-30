/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/ClientVersion.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>

namespace mpqlib
{
    namespace
    {
        std::string findWowExeName(std::filesystem::path const& clientRoot)
        {
            std::error_code ec;
            for (auto const& entry : std::filesystem::directory_iterator(clientRoot, ec))
            {
                if (entry.path().extension() != ".exe")
                    continue;

                std::string stem = entry.path().stem().string();
                std::transform(stem.begin(), stem.end(), stem.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (stem == "wow")
                    return entry.path().filename().string();
            }

            return {};
        }

        // Ported from map_extractor's original getBuildNumber() (credited
        // there to mangos) - jumps well past the exe's header/import tables,
        // then scans for one of the known build-number strings baked into
        // the binary. Every AscEmu-supported client build happens to start
        // with a digit unique enough (within this scan window) to key off:
        // 5/6/8 for Vanilla/TBC, 1 for everything WotLK and later.
        uint32_t scanBuildNumber(std::filesystem::path const& wowExePath)
        {
            unsigned char byteSearchBuffer[1];
            unsigned char jumpBytesBuffer[128];

            unsigned char preWOTLKbuildNumber[3];
            unsigned char postTBCbuildNumber[4];

            unsigned char vanillaBuild1[3] = { 0x38, 0x37, 0x35 };      // (5)875
            unsigned char vanillaBuild2[3] = { 0x30, 0x30, 0x35 };      // (6)005
            unsigned char vanillaBuild3[3] = { 0x31, 0x34, 0x31 };      // (6)141
            unsigned char tbcBuild[3] = { 0x36, 0x30, 0x36 };           // (8)606
            unsigned char wotlkBuild[4] = { 0x32, 0x33, 0x34, 0x30 };   // (1)2340
            unsigned char cataBuild[4] = { 0x35, 0x35, 0x39, 0x35 };    // (1)5595
            unsigned char mopBuild[4] = { 0x38, 0x34, 0x31, 0x34 };     // (1)8414

            FILE* wowExeFile = fopen(wowExePath.string().c_str(), "rb");
            if (!wowExeFile)
                return 0;

            for (auto i = 0; i < 3300; ++i)
                fread(jumpBytesBuffer, sizeof(jumpBytesBuffer), 1, wowExeFile);

            uint32_t build = 0;
            while (fread(byteSearchBuffer, 1, 1, wowExeFile))
            {
                if (byteSearchBuffer[0] == 0x35 || byteSearchBuffer[0] == 0x36 || byteSearchBuffer[0] == 0x38)
                {
                    fread(preWOTLKbuildNumber, sizeof(preWOTLKbuildNumber), 1, wowExeFile);

                    if (!memcmp(preWOTLKbuildNumber, vanillaBuild1, sizeof(preWOTLKbuildNumber))) { build = 5875; break; }
                    if (!memcmp(preWOTLKbuildNumber, vanillaBuild2, sizeof(preWOTLKbuildNumber))) { build = 6005; break; }
                    if (!memcmp(preWOTLKbuildNumber, vanillaBuild3, sizeof(preWOTLKbuildNumber))) { build = 6141; break; }
                    if (!memcmp(preWOTLKbuildNumber, tbcBuild, sizeof(preWOTLKbuildNumber))) { build = 8606; break; }
                }

                if (byteSearchBuffer[0] == 0x31)
                {
                    fread(postTBCbuildNumber, sizeof(postTBCbuildNumber), 1, wowExeFile);

                    if (!memcmp(postTBCbuildNumber, wotlkBuild, sizeof(postTBCbuildNumber))) { build = 12340; break; }
                    if (!memcmp(postTBCbuildNumber, cataBuild, sizeof(postTBCbuildNumber))) { build = 15595; break; }
                    if (!memcmp(postTBCbuildNumber, mopBuild, sizeof(postTBCbuildNumber))) { build = 18414; break; }
                }
            }

            fclose(wowExeFile);
            return build;
        }
    }

    std::optional<ClientVersion> clientVersionFromBuild(uint32_t build)
    {
        // The exact build number a real client reports (e.g. via
        // component.wow-<locale>.txt) is whatever patch level that client
        // happens to be - not necessarily one of the five reference builds
        // AEVersion.hpp uses per expansion. Build numbers increase
        // monotonically release-over-release with no overlap between
        // expansions, so classify by range against those same five
        // thresholds rather than requiring an exact match.
        if (build == 0)
            return std::nullopt;

        if (build < static_cast<uint32_t>(ClientVersion::BurningCrusade))
            return ClientVersion::Vanilla;
        if (build < static_cast<uint32_t>(ClientVersion::WrathOfTheLichKing))
            return ClientVersion::BurningCrusade;
        if (build < static_cast<uint32_t>(ClientVersion::Cataclysm))
            return ClientVersion::WrathOfTheLichKing;
        if (build < static_cast<uint32_t>(ClientVersion::MistsOfPandaria))
            return ClientVersion::Cataclysm;

        return ClientVersion::MistsOfPandaria;
    }

    std::optional<ClientVersion> detectClientVersion(std::filesystem::path const& clientRoot)
    {
        std::string exeName = findWowExeName(clientRoot);
        if (exeName.empty())
            return std::nullopt;

        return clientVersionFromBuild(scanBuildNumber(clientRoot / exeName));
    }
}
