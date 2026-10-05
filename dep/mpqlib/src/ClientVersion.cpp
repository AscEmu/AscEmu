/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "mpqlib/ClientVersion.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>

namespace mpqlib
{
    namespace
    {
        uint32_t gLastDetectedBuild = 0;

        std::vector<std::string> splitPipe(const std::string& line)
        {
            std::vector<std::string> tokens;
            std::stringstream ss(line);
            std::string item;
            while (std::getline(ss, item, '|'))
                tokens.push_back(item);
            return tokens;
        }

        uint32_t scanCascBuildNumber(std::filesystem::path const& clientRoot)
        {
            std::filesystem::path buildInfoPath = clientRoot / ".build.info";
            std::ifstream file(buildInfoPath);
            if (!file.is_open())
                return 0;

            std::string headerLine;
            if (!std::getline(file, headerLine))
                return 0;

            auto headers = splitPipe(headerLine);
            int versionIndex = -1;
            for (size_t i = 0; i < headers.size(); ++i)
            {
                if (headers[i].rfind("Version", 0) == 0)
                {
                    versionIndex = static_cast<int>(i);
                    break;
                }
            }

            if (versionIndex == -1)
                return 0;

            std::string dataLine;
            while (std::getline(file, dataLine))
            {
                if (dataLine.empty())
                    continue;

                auto values = splitPipe(dataLine);
                if (static_cast<size_t>(versionIndex) >= values.size())
                    continue;

                std::string verStr = values[versionIndex];
                size_t lastDot = verStr.find_last_of('.');
                if (lastDot == std::string::npos || lastDot + 1 >= verStr.size())
                    continue;

                std::string buildStr = verStr.substr(lastDot + 1);
                uint32_t build = 0;
                auto [ptr, ec] = std::from_chars(buildStr.data(), buildStr.data() + buildStr.size(), build);
                if (ec == std::errc())
                    return build;
            }

            return 0;
        }

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

    uint32_t getDetectedBuildNumber()
    {
        return gLastDetectedBuild;
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

        // Special client branches
        if (build >= 69000 && build <= 71000)
            return ClientVersion::Forever;

        if (build < static_cast<uint32_t>(ClientVersion::BurningCrusade))
            return ClientVersion::Vanilla;
        if (build < static_cast<uint32_t>(ClientVersion::WrathOfTheLichKing))
            return ClientVersion::BurningCrusade;
        if (build < static_cast<uint32_t>(ClientVersion::Cataclysm))
            return ClientVersion::WrathOfTheLichKing;
        if (build < static_cast<uint32_t>(ClientVersion::MistsOfPandaria))
            return ClientVersion::Cataclysm;
        if (build < static_cast<uint32_t>(ClientVersion::WarlordsOfDraenor))
            return ClientVersion::MistsOfPandaria;
        if (build < static_cast<uint32_t>(ClientVersion::Legion))
            return ClientVersion::WarlordsOfDraenor;
        if (build < static_cast<uint32_t>(ClientVersion::BattleForAzeroth))
            return ClientVersion::Legion;
        if (build < static_cast<uint32_t>(ClientVersion::Shadowlands))
            return ClientVersion::BattleForAzeroth;
        if (build < static_cast<uint32_t>(ClientVersion::Dragonflight))
            return ClientVersion::Shadowlands;
        if (build < static_cast<uint32_t>(ClientVersion::TheWarWithin))
            return ClientVersion::Dragonflight;
        if (build < static_cast<uint32_t>(ClientVersion::Midnight))
            return ClientVersion::TheWarWithin;

        return ClientVersion::Midnight;
    }

    std::optional<ClientVersion> detectClientVersion(std::filesystem::path const& clientRoot)
    {
        // Check for CASC build first, since it has a .build.info file that can be read without opening Wow.exe.
        // If that fails, fall back to scanning the Wow.exe for known build-number patterns.
        uint32_t build = scanCascBuildNumber(clientRoot);
        if (build > 0)
        {
            gLastDetectedBuild = build;
            return clientVersionFromBuild(build);
        }

        // Fallback: Legacy MPQ (wow.exe scans) - this is slower and less reliable, but still works for older clients.
        std::string exeName = findWowExeName(clientRoot);
        if (exeName.empty())
            return std::nullopt;

        build = scanBuildNumber(clientRoot / exeName);
        gLastDetectedBuild = build;
        return clientVersionFromBuild(build);
    }
}
