/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#define _CRT_SECURE_NO_DEPRECATE

#include "ADTFile.hpp"
#include "WDTFile.hpp"
#include "mpqlib/ClientVersion.hpp"
#include "mpqlib/DBCFile.hpp"
#include "mpqlib/MPQFile.hpp"
#include "wmo.h"
#include "vmapexport.h"

#ifdef WIN32
    #include <sys/stat.h>
    #include <direct.h>
    #define mkdir _mkdir
#else
    #include <sys/stat.h>
#endif

#undef min
#undef max

#include <algorithm>
#include <array>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

#define MPQ_BLOCK_SIZE 0x1000

namespace fs = std::filesystem;
using mpqlib::ClientVersion;

// Single implicit archive chain (Classic/TBC/WotLK) covering everything this
// tool reads, or the world-data half of the two-chain split (Cata/Mop) - see
// LocaleMpq below and IsLegacyVmapArchiveLayout().
std::unique_ptr<mpqlib::MpqPatchChain> WorldMpq;
// Only populated for Cata/Mop: locale-specific data (LiquidType.dbc,
// Map.dbc, GameObjectDisplayInfo.dbc) live in a separate archive chain from
// WorldMpq's world/model/WMO binary data on those clients.
std::unique_ptr<mpqlib::MpqPatchChain> LocaleMpq;

ClientVersion gClientVersion = ClientVersion::WrathOfTheLichKing;

bool IsLegacyVmapArchiveLayout()
{
    return gClientVersion == ClientVersion::Vanilla || gClientVersion == ClientVersion::BurningCrusade
        || gClientVersion == ClientVersion::WrathOfTheLichKing;
}

bool IsPreWotLKModelFormat()
{
    return gClientVersion == ClientVersion::Vanilla || gClientVersion == ClientVersion::BurningCrusade;
}

namespace
{
    mpqlib::MpqPatchChain& DbcChain()
    {
        return IsLegacyVmapArchiveLayout() ? *WorldMpq : *LocaleMpq;
    }
}

struct map_id
{
    char name[64];
    uint32_t id;
};

std::vector<map_id> map_ids;
uint16_t* LiqType = nullptr;
char output_path[128] = ".";
char input_path[1024] = ".";
bool hasInputPathParam = false;
bool preciseVectorData = false;

const char* szWorkDirWmo = "./Buildings";
const char* szRawVMAPMagic = "VMAP041";

// Only meaningful for Cata+: which incremental wow-update patch build to
// extract up to, and the corresponding base-MPQ/patch-build list. Kept
// separate from map_extractor's own copies of these same-shaped constants -
// vmap4_extractor's own historically-tuned lists differ in both content
// (needs model.MPQ; doesn't need misc.MPQ pre-Mop) and order (world2.MPQ's
// position) from map_extractor's, and its Mop-era
// LAST_DBC_IN_DATA_BUILD/NEW_BASE_SET_BUILD values are genuinely different
// numbers from both map_extractor's and this file's own Cata branch.
uint32_t CONF_TargetBuild = 0;

std::vector<std::string> const CataMpqList = { "world.MPQ", "art.MPQ", "expansion1.MPQ", "expansion2.MPQ", "expansion3.MPQ", "world2.MPQ" };
std::vector<std::string> const MopMpqList = { "world.MPQ", "model.MPQ", "misc.MPQ", "expansion1.MPQ", "expansion2.MPQ", "expansion3.MPQ", "expansion4.MPQ" };
std::vector<uint32_t> const CataBuilds = { 13164, 13205, 13287, 13329, 13596, 13623, 13914, 14007, 14333, 14480, 14545, 15005, 15050, 15211, 15354, 15595 };
std::vector<uint32_t> const MopBuilds = { 16016, 16048, 16057, 16309, 16357, 16516, 16650, 16844, 16965, 17116, 17266, 17325, 17331, 17345, 17538, 17645, 17688, 17898, 18273 };

constexpr uint32_t kCataLastDbcInDataBuild = 13623;
constexpr uint32_t kCataNewBaseSetBuild = 15211;
constexpr uint32_t kMopLastDbcInDataBuild = 15595;
constexpr uint32_t kMopNewBaseSetBuild = 16016;

std::vector<std::string> const& GetTargetMpqList() { return gClientVersion == ClientVersion::MistsOfPandaria ? MopMpqList : CataMpqList; }
std::vector<uint32_t> const& GetTargetBuildList() { return gClientVersion == ClientVersion::MistsOfPandaria ? MopBuilds : CataBuilds; }
uint32_t GetLastDbcInDataBuild() { return gClientVersion == ClientVersion::MistsOfPandaria ? kMopLastDbcInDataBuild : kCataLastDbcInDataBuild; }
uint32_t GetNewBaseSetBuild() { return gClientVersion == ClientVersion::MistsOfPandaria ? kMopNewBaseSetBuild : kCataNewBaseSetBuild; }

#define LOCALES_COUNT 15

char const* const Locales[LOCALES_COUNT] =
{
    "enGB", "enUS", "deDE", "esES", "frFR", "koKR", "zhCN", "zhTW", "enCN", "enTW", "esMX", "ruRU", "ptBR", "ptPT", "itIT"
};

bool FileExists(const char* file)
{
    if (FILE* n = fopen(file, "rb"))
    {
        fclose(n);
        return true;
    }
    return false;
}

void strToLower(char* str)
{
    while (*str)
    {
        *str = static_cast<char>(std::tolower(static_cast<unsigned char>(*str)));
        ++str;
    }
}

void ReadLiquidTypeTableDBC()
{
    printf("Read LiquidType.dbc file...");

    // Mop's LiquidType.dbc read intentionally bypasses LocaleMpq (which
    // LoadLocaleMPQFile builds with the full locale+update+Cache patch
    // stack for everything else) in favor of a bare, unpatched misc.MPQ -
    // this is vmap4_extractor's own pre-existing quirk (map_extractor's
    // already-unified Mop path reads LiquidType.dbc through its LocaleMpq
    // equivalent instead), preserved exactly rather than "fixed" here.
    std::unique_ptr<mpqlib::MpqPatchChain> mopLocaleOnly;
    mpqlib::MpqPatchChain* chain = &DbcChain();
    if (gClientVersion == ClientVersion::MistsOfPandaria)
    {
        char localMpq[1024];
        snprintf(localMpq, sizeof(localMpq), "%smisc.MPQ", input_path);
        mopLocaleOnly = std::make_unique<mpqlib::MpqPatchChain>(localMpq);
        if (!mopLocaleOnly->isOpen())
            exit(1);
        chain = mopLocaleOnly.get();
    }

    DBCFile dbc(*chain, "DBFilesClient\\LiquidType.dbc");
    if (!dbc.open())
    {
        printf("Fatal error: Invalid LiquidType.dbc file format!\n");
        exit(1);
    }

    size_t liqTypeCount = dbc.getRowCount();
    size_t liqTypeMaxId = dbc.getMaxId();
    LiqType = new uint16_t[liqTypeMaxId + 1];
    memset(LiqType, 0xff, (liqTypeMaxId + 1) * sizeof(uint16_t));

    for (uint32_t x = 0; x < liqTypeCount; ++x)
        LiqType[dbc.getRow(x).getUInt(0)] = static_cast<uint16_t>(dbc.getRow(x).getUInt(3));

    printf("Done! (%u LiqTypes loaded)\n", static_cast<uint32_t>(liqTypeCount));
}

// Minimal single-'*' suffix matcher, replacing StormLib's SFileFindFirstFile/
// SFileFindNextFile - this tool only ever searches for a plain "*.wmo" suffix.
namespace
{
    bool CaseInsensitiveEquals(std::string_view a, std::string_view b)
    {
        if (a.size() != b.size())
            return false;

        return std::equal(a.begin(), a.end(), b.begin(), [](char x, char y)
        {
            return std::tolower(static_cast<unsigned char>(x)) == std::tolower(static_cast<unsigned char>(y));
        });
    }

    bool MatchesSimplePattern(std::string_view name, std::string_view pattern)
    {
        size_t const star = pattern.find('*');
        if (star == std::string_view::npos)
            return CaseInsensitiveEquals(name, pattern);

        std::string_view const prefix = pattern.substr(0, star);
        std::string_view const suffix = pattern.substr(star + 1);

        if (name.size() < prefix.size() + suffix.size())
            return false;

        return CaseInsensitiveEquals(name.substr(0, prefix.size()), prefix) &&
            CaseInsensitiveEquals(name.substr(name.size() - suffix.size()), suffix);
    }
}

bool ExtractWmo()
{
    bool success = true;

    for (std::string const& name : WorldMpq->listFiles())
    {
        if (!MatchesSimplePattern(name, "*.wmo"))
            continue;

        std::string str = name;
        success = ExtractSingleWmo(str);
        if (!success)
            break;
    }

    if (success)
        printf("\nExtract wmo complete (No (fatal) errors)\n");

    return success;
}

bool ExtractSingleWmo(std::string& fname)
{
    // Copy files from archive

    char szLocalFile[1024];
    const char* plain_name = getPlainName(fname.c_str());
    sprintf(szLocalFile, "%s/%s", szWorkDirWmo, plain_name);
    fixNameCase(szLocalFile, strlen(szLocalFile));

    if (FileExists(szLocalFile))
        return true;

    int p = 0;
    // Select root wmo files
    char const* rchr = strrchr(plain_name, '_');
    if (rchr != NULL)
    {
        char cpy[4];
        memcpy(cpy, rchr, 4);
        for (int i = 0; i < 4; ++i)
        {
            int m = cpy[i];
            if (isdigit(m))
                p++;
        }
    }

    if (p == 3)
        return true;

    bool file_ok = true;
    std::cout << "Extracting " << fname << std::endl;
    WMORoot froot(fname);
    if (!froot.open())
    {
        printf("Couldn't open RootWmo!!!\n");
        return true;
    }
    FILE* output = fopen(szLocalFile, "wb");
    if (!output)
    {
        printf("couldn't open %s for writing!\n", szLocalFile);
        return false;
    }
    froot.convertToVMapRootWmo(output);
    int Wmo_nVertices = 0;
    if (froot.m_numGroups != 0)
    {
        for (uint32_t i = 0; i < froot.m_numGroups; ++i)
        {
            char temp[1024];
            strncpy(temp, fname.c_str(), 1024);
            temp[fname.length() - 4] = 0;
            char groupFileName[1024];
            sprintf(groupFileName, "%s_%03u.wmo", temp, i);

            std::string s = groupFileName;
            WMOGroup fgroup(s);
            if (!fgroup.open())
            {
                printf("Could not open all Group file for: %s\n", plain_name);
                file_ok = false;
                break;
            }

            Wmo_nVertices += fgroup.convertToVMapGroupWmo(output, &froot, preciseVectorData);
        }
    }

    fseek(output, 8, SEEK_SET); // store the correct no of vertices
    fwrite(&Wmo_nVertices, sizeof(int), 1, output);
    fclose(output);

    // Delete the extracted file in the case of an error
    if (!file_ok)
        remove(szLocalFile);
    return true;
}

void ParsMapFiles()
{
    char fn[512];
    char id[10];
    for (size_t i = 0; i < map_ids.size(); ++i)
    {
        sprintf(id, "%04u", map_ids[i].id);
        sprintf(fn, "World\\Maps\\%s\\%s.wdt", map_ids[i].name, map_ids[i].name);
        WDTFile WDT(fn, map_ids[i].name);
        if (WDT.init(id, map_ids[i].id))
        {
            printf("Processing Map %u\n[", map_ids[i].id);
            for (int x = 0; x < 64; ++x)
            {
                for (int y = 0; y < 64; ++y)
                {
                    if (ADTFile* ADT = WDT.getMap(x, y))
                    {
                        ADT->init(map_ids[i].id, x, y);
                        delete ADT;
                    }
                }
                printf("#");
                fflush(stdout);
            }
            printf("]\n");
        }
    }
}

// --- Legacy (Classic/TBC/WotLK) MPQ discovery: fixed per-version archive
// lists plus a disk probe for numbered patch levels ---

namespace
{
    void getGamePath()
    {
        strcpy(input_path, "Data/");
    }

    bool scan_patches(char* scanmatch, std::vector<std::string>& pArchiveNames)
    {
        char path[512];

        for (int i = 1; i <= 99; i++)
        {
            if (i != 1)
                sprintf(path, "%s-%d.MPQ", scanmatch, i);
            else
                sprintf(path, "%s.MPQ", scanmatch);

#ifdef __linux__
            if (FILE* h = fopen64(path, "rb"))
#else
            if (FILE* h = fopen(path, "rb"))
#endif
            {
                fclose(h);
                pArchiveNames.push_back(path);
            }
        }

        return true;
    }

    bool fillLegacyArchiveNameVector(std::vector<std::string>& pArchiveNames)
    {
        printf("\nGame path: %s\n", input_path);

        char path[512];
        std::string in_path(input_path);

        std::vector<std::string> locales;

        if (gClientVersion != ClientVersion::Vanilla)
        {
            std::vector<std::string> searchLocales = {
                "enGB", "enUS", "deDE", "esES", "frFR", "koKR", "zhCN", "zhTW", "enCN", "enTW", "esMX", "ruRU"
            };

            for (std::string const& locale : searchLocales)
            {
                std::string localePath = in_path + locale;
                struct stat status;
                if (stat(localePath.c_str(), &status))
                    continue;
                if ((status.st_mode & S_IFDIR) == 0)
                    continue;
                printf("Found locale '%s'\n", locale.c_str());
                locales.push_back(locale);
            }
            printf("\n");

            printf("Adding data files from locale directories.\n");
            for (std::string const& locale : locales)
            {
                if (gClientVersion == ClientVersion::BurningCrusade)
                {
                    pArchiveNames.push_back(in_path + locale + "/patch-" + locale + ".MPQ");
                    pArchiveNames.push_back(in_path + locale + "/patch-" + locale + "-2.MPQ");
                    pArchiveNames.push_back(in_path + locale + "/locale-" + locale + ".MPQ");
                    pArchiveNames.push_back(in_path + locale + "/speech-" + locale + ".MPQ");
                    pArchiveNames.push_back(in_path + locale + "/expansion-locale-" + locale + ".MPQ");
                    pArchiveNames.push_back(in_path + locale + "/expansion-speech-" + locale + ".MPQ");
                }
                else // WrathOfTheLichKing
                {
                    pArchiveNames.push_back(in_path + locale + "/patch-" + locale + ".MPQ");
                    pArchiveNames.push_back(in_path + locale + "/patch-" + locale + "-2.MPQ");
                    pArchiveNames.push_back(in_path + locale + "/patch-" + locale + "-3.MPQ");
                    pArchiveNames.push_back(in_path + locale + "/locale-" + locale + ".MPQ");
                    pArchiveNames.push_back(in_path + locale + "/speech-" + locale + ".MPQ");
                    pArchiveNames.push_back(in_path + locale + "/expansion-locale-" + locale + ".MPQ");
                    pArchiveNames.push_back(in_path + locale + "/expansion-speech-" + locale + ".MPQ");
                    pArchiveNames.push_back(in_path + locale + "/lichking-locale-" + locale + ".MPQ");
                    pArchiveNames.push_back(in_path + locale + "/lichking-speech-" + locale + ".MPQ");
                }
            }
        }

        if (gClientVersion == ClientVersion::Vanilla)
        {
            pArchiveNames.push_back(input_path + std::string("patch.MPQ"));
            pArchiveNames.push_back(input_path + std::string("patch-2.MPQ"));
            pArchiveNames.push_back(input_path + std::string("wmo.MPQ"));
            pArchiveNames.push_back(input_path + std::string("texture.MPQ"));
            pArchiveNames.push_back(input_path + std::string("terrain.MPQ"));
            pArchiveNames.push_back(input_path + std::string("speech.MPQ"));
            pArchiveNames.push_back(input_path + std::string("sound.MPQ"));
            pArchiveNames.push_back(input_path + std::string("model.MPQ"));
            pArchiveNames.push_back(input_path + std::string("misc.MPQ"));
            pArchiveNames.push_back(input_path + std::string("dbc.MPQ"));
            pArchiveNames.push_back(input_path + std::string("base.MPQ"));
        }
        else if (gClientVersion == ClientVersion::BurningCrusade)
        {
            pArchiveNames.push_back(input_path + std::string("patch.MPQ"));
            pArchiveNames.push_back(input_path + std::string("patch-2.MPQ"));
            pArchiveNames.push_back(input_path + std::string("expansion.MPQ"));
            pArchiveNames.push_back(input_path + std::string("common.MPQ"));
        }
        else // WrathOfTheLichKing
        {
            pArchiveNames.push_back(input_path + std::string("patch.MPQ"));
            pArchiveNames.push_back(input_path + std::string("patch-2.MPQ"));
            pArchiveNames.push_back(input_path + std::string("patch-3.MPQ"));
            pArchiveNames.push_back(input_path + std::string("expansion.MPQ"));
            pArchiveNames.push_back(input_path + std::string("lichking.MPQ"));
            pArchiveNames.push_back(input_path + std::string("common.MPQ"));
            pArchiveNames.push_back(input_path + std::string("common-2.MPQ"));
        }

        printf("Scanning patch levels from data directory.\n");
        sprintf(path, "%spatch", input_path);
        if (!scan_patches(path, pArchiveNames))
            return false;

        if (gClientVersion == ClientVersion::Vanilla)
        {
            printf("\n");
        }
        else
        {
            printf("Scanning patch levels from locale directories.\n");
            bool foundOne = false;
            for (std::string const& locale : locales)
            {
                printf("Locale: %s\n", locale.c_str());
                sprintf(path, "%s%s/patch-%s", input_path, locale.c_str(), locale.c_str());
                if (scan_patches(path, pArchiveNames))
                    foundOne = true;
            }

            printf("\n");

            if (!foundOne)
            {
                printf("No locale found\n");
                return false;
            }
        }

        return true;
    }
}

// --- Cata+ MPQ discovery: incremental wow-update patch chains ---

// Note: StormLib's SFileOpenPatchArchive() took a "path prefix" that remaps
// files stored under a locale subfolder inside a shared patch archive so
// they become addressable by their bare name. mpqlib has no equivalent -
// only relevant for archives at/below LAST_DBC_IN_DATA_BUILD, a build from
// years before Cata/Mop ever shipped, so in practice every archive this
// tool actually opens today never needs it.
bool LoadModernLocaleMPQFile(int locale)
{
    char buff[512];

    if (gClientVersion != ClientVersion::MistsOfPandaria)
    {
        snprintf(buff, sizeof(buff), "%s%s/locale-%s.MPQ", input_path, Locales[locale], Locales[locale]);
        LocaleMpq = std::make_unique<mpqlib::MpqPatchChain>(buff);
        if (!LocaleMpq->isOpen())
        {
            LocaleMpq.reset();
            return false;
        }

        printf("Loading %s locale MPQs\n", Locales[locale]);
        for (uint32_t patchBuild : GetTargetBuildList())
        {
            if (patchBuild > CONF_TargetBuild)
                break;
            if (CONF_TargetBuild >= GetNewBaseSetBuild() && patchBuild < GetNewBaseSetBuild())
                continue;

            if (patchBuild > GetLastDbcInDataBuild())
                snprintf(buff, sizeof(buff), "%s%s/wow-update-%s-%u.MPQ", input_path, Locales[locale], Locales[locale], patchBuild);
            else
                snprintf(buff, sizeof(buff), "%swow-update-%u.MPQ", input_path, patchBuild);

            LocaleMpq->addPatch(buff);
        }

        printf("\n");
        return true;
    }

    snprintf(buff, sizeof(buff), "%smisc.MPQ", input_path);
    LocaleMpq = std::make_unique<mpqlib::MpqPatchChain>(buff);
    if (!LocaleMpq->isOpen())
    {
        LocaleMpq.reset();
        return false;
    }

    snprintf(buff, sizeof(buff), "%s%s/locale-%s.MPQ", input_path, Locales[locale], Locales[locale]);
    if (!LocaleMpq->addPatch(buff))
    {
        LocaleMpq.reset();
        return false;
    }

    printf("Loading %s locale MPQs\n", Locales[locale]);

    for (uint32_t patchBuild : GetTargetBuildList())
    {
        if (patchBuild > CONF_TargetBuild)
            break;
        snprintf(buff, sizeof(buff), "%swow-update-base-%u.MPQ", input_path, patchBuild);
        if (!LocaleMpq->addPatch(buff))
            printf("Not found %s\n", buff);
        else
            printf("Loaded %s\n", buff);
    }

    for (uint32_t patchBuild : GetTargetBuildList())
    {
        if (patchBuild > CONF_TargetBuild)
            break;
        snprintf(buff, sizeof(buff), "%s%s/wow-update-%s-%u.MPQ", input_path, Locales[locale], Locales[locale], patchBuild);
        if (LocaleMpq->addPatch(buff))
            printf("Loaded %s\n", buff);
    }

    for (uint32_t patchBuild : GetTargetBuildList())
    {
        if (patchBuild > CONF_TargetBuild)
            break;
        snprintf(buff, sizeof(buff), "%sCache\\patch-base-%u.MPQ", input_path, patchBuild);
        if (LocaleMpq->addPatch(buff))
            printf("Loaded %s\n", buff);
    }

    for (uint32_t patchBuild : GetTargetBuildList())
    {
        if (patchBuild > CONF_TargetBuild)
            break;
        snprintf(buff, sizeof(buff), "%sCache\\%s\\patch-%s-%u.MPQ", input_path, Locales[locale], Locales[locale], patchBuild);
        if (LocaleMpq->addPatch(buff))
            printf("Loaded %s\n", buff);
    }

    printf("\n");
    return true;
}

void LoadModernCommonMPQFiles(uint32_t build)
{
    char filename[512];
    snprintf(filename, sizeof(filename), "%sworld.MPQ", input_path);
    printf("Loading common MPQ files\n");
    WorldMpq = std::make_unique<mpqlib::MpqPatchChain>(filename);
    if (!WorldMpq->isOpen())
    {
        printf("Cannot open archive %s\n", filename);
        WorldMpq.reset();
        return;
    }

    std::vector<std::string> const& mpqList = GetTargetMpqList();
    for (size_t i = 1; i < mpqList.size(); ++i)
    {
        if (build < 15211 && mpqList[i] == "world2.MPQ")   // 4.3.2 and higher MPQ
            continue;

        snprintf(filename, sizeof(filename), "%s%s", input_path, mpqList[i].c_str());
        if (!WorldMpq->addPatch(filename))
            printf("Not found %s\n", filename);
        else
            printf("Loaded %s\n", filename);
    }

    for (uint32_t patchBuild : GetTargetBuildList())
    {
        if (patchBuild > CONF_TargetBuild)
            break;
        if (CONF_TargetBuild >= GetNewBaseSetBuild() && patchBuild < GetNewBaseSetBuild())
            continue;

        if (patchBuild > GetLastDbcInDataBuild())
            snprintf(filename, sizeof(filename), "%swow-update-base-%u.MPQ", input_path, patchBuild);
        else
            snprintf(filename, sizeof(filename), "%swow-update-%u.MPQ", input_path, patchBuild);

        if (!WorldMpq->addPatch(filename))
        {
            printf("Not found %s\n", filename);
            continue;
        }

        printf("Loaded %s\n", filename);
    }

    // Hotfix cache archives can update/add WDT, ADT, WMO and model data just
    // like they update DBC data (LoadModernLocaleMPQFile already loads these
    // for LocaleMpq) - without them, content delivered purely via a cache
    // patch (e.g. late-Mop zones such as HawaiiMainLand/Isle of Thunder) has
    // no WDT to convert at all.
    for (uint32_t patchBuild : GetTargetBuildList())
    {
        if (patchBuild > CONF_TargetBuild)
            break;

        snprintf(filename, sizeof(filename), "%sCache\\patch-base-%u.MPQ", input_path, patchBuild);

        if (!WorldMpq->addPatch(filename))
        {
            printf("Not found %s\n", filename);
            continue;
        }

        printf("Loaded %s\n", filename);
    }

    printf("\n");
}

bool processArgv(int argc, char** argv, const char* versionString)
{
    bool result = true;
    hasInputPathParam = false;
    preciseVectorData = false;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp("-s", argv[i]) == 0)
        {
            preciseVectorData = false;
        }
        else if (strcmp("-d", argv[i]) == 0)
        {
            if ((i + 1) < argc)
            {
                hasInputPathParam = true;
                strncpy(input_path, argv[i + 1], sizeof(input_path));
                input_path[sizeof(input_path) - 1] = '\0';

                if (input_path[strlen(input_path) - 1] != '\\' && input_path[strlen(input_path) - 1] != '/')
                    strcat(input_path, "/");
                ++i;
            }
            else
                result = false;
        }
        else if (strcmp("-?", argv[1]) == 0)
        {
            result = false;
        }
        else if (strcmp("-l", argv[i]) == 0)
        {
            preciseVectorData = true;
        }
        else if (strcmp("-b", argv[i]) == 0)
        {
            if (i + 1 < argc)
                CONF_TargetBuild = atoi(argv[i++ + 1]);
            else
                result = false;
        }
        else
        {
            result = false;
            break;
        }
    }

    if (!result)
    {
        printf("Extract %s.\n", versionString);
        printf("%s [-?][-s][-l][-d <path>][-b <build>]\n", argv[0]);
        printf("   -s : (default) small size (data size optimization), ~500MB less vmap data.\n");
        printf("   -l : large size, ~500MB more vmap data. (might contain more details)\n");
        printf("   -d <path>: Path to the vector data source folder.\n");
        printf("   -b <build>: target build (Cata+ only, default %u)\n", CONF_TargetBuild);
        printf("   -? : This message.\n");
    }

    if (!hasInputPathParam)
        getGamePath();

    return result;
}

int main(int argc, char** argv)
{
    bool success = true;
    const char* versionString = "V4.00 2012_02";

    auto detected = mpqlib::detectClientVersion(fs::current_path());
    if (!detected)
    {
        printf("Fatal Error: No wow.exe found in current directory!\n");
        return 1;
    }
    gClientVersion = *detected;
    CONF_TargetBuild = gClientVersion == ClientVersion::MistsOfPandaria ? 18273 : 15595;

    if (!processArgv(argc, argv, versionString))
        return 1;

    // some simple check if working dir is dirty
    else
    {
        std::string sdir = std::string(szWorkDirWmo) + "/dir";
        std::string sdir_bin = std::string(szWorkDirWmo) + "/dir_bin";
        struct stat status;
        if (!stat(sdir.c_str(), &status) || !stat(sdir_bin.c_str(), &status))
        {
            printf("Your output directory seems to be polluted, please use an empty directory!\n");
            printf("<press return to exit>");
            char garbage[2];
            return scanf("%c", garbage);
        }
    }

    printf("Extract %s. Beginning work ....\n", versionString);
    // Create the working directory
    if (mkdir(szWorkDirWmo
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
                    , 0711
#endif
                    ))
            success = (errno == EEXIST);

    if (IsLegacyVmapArchiveLayout())
    {
        // prepare archive name list. The first archive that successfully
        // opens becomes the chain's base, every one after it is added as a
        // patch - mirrors the old flat ArchiveSet's "just skip archives
        // that don't exist" behavior, since a failed base open tears the
        // chain back down so the next candidate can retry as a fresh base.
        std::vector<std::string> archiveNames;
        fillLegacyArchiveNameVector(archiveNames);
        for (std::string const& archiveName : archiveNames)
        {
            if (!WorldMpq)
            {
                WorldMpq = std::make_unique<mpqlib::MpqPatchChain>(archiveName);
                if (!WorldMpq->isOpen())
                    WorldMpq.reset();
            }
            else
            {
                WorldMpq->addPatch(archiveName);
            }
        }

        if (!WorldMpq)
        {
            printf("FATAL ERROR: None MPQ archive found by path '%s'. Use -d option with proper path.\n", input_path);
            return 1;
        }
    }
    else
    {
        LoadModernCommonMPQFiles(CONF_TargetBuild);

        for (int i = 0; i < LOCALES_COUNT; ++i)
        {
            if (!LoadModernLocaleMPQFile(i))
                continue;

            printf("Detected and using locale: %s\n", Locales[i]);
            break;
        }

        if (!WorldMpq || !LocaleMpq)
        {
            printf("FATAL ERROR: None MPQ archive found by path '%s'. Use -d option with proper path.\n", input_path);
            return 1;
        }
    }

    ReadLiquidTypeTableDBC();

    // extract data
    if (success)
        success = ExtractWmo();

    // map.dbc
    if (success)
    {
        DBCFile dbc(DbcChain(), "DBFilesClient\\Map.dbc");
        if (!dbc.open())
        {
            printf("FATAL ERROR: Map.dbc not found in data file.\n");
            return 1;
        }

        size_t map_count = dbc.getRowCount();
        map_ids.resize(map_count);
        for (size_t x = 0; x < map_count; ++x)
        {
            map_ids[x].id = dbc.getRow(x).getUInt(0);

            char const* map_name = dbc.getRow(x).getString(1);
            size_t max_map_name_length = sizeof(map_ids[x].name);
            if (strlen(map_name) >= max_map_name_length)
            {
                printf("FATAL ERROR: Map name too long.\n");
                return 1;
            }

            strncpy(map_ids[x].name, map_name, max_map_name_length);
            map_ids[x].name[max_map_name_length - 1] = '\0';
            printf("Map - %s\n", map_ids[x].name);
        }

        ParsMapFiles();
        map_ids.clear();
        // Extract models, listed in GameObjectDisplayInfo.dbc
        ExtractGameobjectModels();
    }

    LocaleMpq.reset();
    WorldMpq.reset();

    printf("\n");
    if (!success)
    {
        printf("ERROR: Extract %s. Work NOT complete.\n   Precise vector data=%d.\nPress any key.\n", versionString, preciseVectorData);
        getchar();
    }

    printf("Extract %s. Work complete. No errors.\n", versionString);
    delete[] LiqType;
    return 0;
}
