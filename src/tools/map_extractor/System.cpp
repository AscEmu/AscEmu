/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
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

#include "mpqlib/ChunkTree.hpp"
#include "mpqlib/ClientVersion.hpp"
#include "mpqlib/DBCFile.hpp"
#include "mpqlib/MpqPatchChain.hpp"

#include "adt.h"
#include "wdt.h"

#include <algorithm>
#include <array>
#include <bitset>
#include <cctype>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;
using mpqlib::ClientVersion;

// Two chains: on Classic/TBC/WotLK, map/DBC/locale data all lived in the same
// flat set of archives, so WorldMpq is the only one ever opened. On Cata/Mop,
// world data (maps) and locale data (DBC, component/build info) come from
// genuinely separate archive sets, mirroring the pre-mpqlib ToolsCataMop tool.
std::unique_ptr<mpqlib::MpqPatchChain> WorldMpq;
std::unique_ptr<mpqlib::MpqPatchChain> LocaleMpq;

ClientVersion gClientVersion = ClientVersion::WrathOfTheLichKing;

bool IsLegacyMapFormat()
{
    return gClientVersion == ClientVersion::Vanilla || gClientVersion == ClientVersion::BurningCrusade
        || gClientVersion == ClientVersion::WrathOfTheLichKing;
}

namespace
{
    // ChunkTree's byte-scan opportunistically matches any recognized tag it
    // happens to find, even one that was never really written by this file
    // (a false positive on 4 coincidental payload bytes - observed in
    // practice: recognizing "MFBO" while scanning a WotLK ADT matched
    // unrelated bytes deep in one file and grew its height section by a
    // spurious flight-box block). Keeping each file type/version's
    // recognized set as narrow as what can actually appear in it avoids
    // this entirely, rather than sharing one broad list across everything.
    constexpr std::array<std::string_view, 5> kLegacyAdtTags = { "MVER", "MH2O", "MCNK", "MCVT", "MCLQ" };
    constexpr std::array<std::string_view, 4> kLegacyWdtTags = { "MVER", "MPHD", "MAIN", "MWMO" };
    constexpr std::array<std::string_view, 6> kModernAdtTags = { "MVER", "MH2O", "MCNK", "MCVT", "MCLQ", "MFBO" };
    constexpr std::array<std::string_view, 2> kModernWdtTags = { "MVER", "MAIN" };

    constexpr uint32_t kAdtWdtFormatVersion = 18;

    struct VersionChunk
    {
        union
        {
            uint32_t fcc;
            char fcc_txt[4];
        };
        uint32_t size;
        uint32_t ver;
    };

    bool isValidVersionChunk(mpqlib::ChunkNode const* mver)
    {
        if (!mver)
            return false;

        auto const& version = mver->as<VersionChunk>();
        // fcc_txt holds the raw on-disk (reversed) tag, i.e. "REVM" for "MVER".
        return version.fcc_txt[0] == 'R' && version.fcc_txt[1] == 'E' && version.fcc_txt[2] == 'V' && version.fcc_txt[3] == 'M'
            && version.ver == kAdtWdtFormatVersion;
    }

    // Loads and validates an ADT/WDT chunk tree: silently on a missing file
    // when log is false, but always noisily on a present-but-invalid one.
    std::optional<mpqlib::ChunkTree> loadChunkTree(std::string_view filename, std::span<const std::string_view> recognizedTags, bool log = true)
    {
        auto tree = mpqlib::ChunkTree::load(*WorldMpq, filename, recognizedTags);
        if (!tree)
        {
            if (log)
                printf("No such file %.*s\n", static_cast<int>(filename.size()), filename.data());
            return std::nullopt;
        }

        if (!isValidVersionChunk(tree->find("MVER")))
        {
            printf("Error loading %.*s\n", static_cast<int>(filename.size()), filename.data());
            return std::nullopt;
        }

        return tree;
    }

    std::optional<mpqlib::ChunkTree> loadAdtChunkTree(std::string_view filename, bool log = true)
    {
        return loadChunkTree(filename, IsLegacyMapFormat() ? std::span<const std::string_view>(kLegacyAdtTags) : std::span<const std::string_view>(kModernAdtTags), log);
    }

    std::optional<mpqlib::ChunkTree> loadWdtChunkTree(std::string_view filename, bool log = true)
    {
        return loadChunkTree(filename, IsLegacyMapFormat() ? std::span<const std::string_view>(kLegacyWdtTags) : std::span<const std::string_view>(kModernWdtTags), log);
    }

    // Classic/TBC/WotLK WDTs additionally require MPHD/MWMO to be present -
    // carried over from the old WDT_file::prepareLoadedData() validation
    // chain. MPHD/MWMO carry no data this tool needs beyond their presence.
    bool hasLegacyWdtChunks(mpqlib::ChunkTree const& wdt)
    {
        return wdt.find("MPHD") != nullptr && wdt.find("MWMO") != nullptr;
    }
}

struct map_id
{
    char name[64];
    uint32_t id;
};

struct LiquidMaterialEntry
{
    int8_t LVF;
};

struct LiquidObjectEntry
{
    int16_t LiquidTypeID;
};

struct LiquidTypeEntry
{
    uint8_t SoundBank;
    uint8_t MaterialID;
};

std::vector<map_id> map_ids;

// Legacy (Classic/TBC/WotLK) area/liquid lookups: flat arrays indexed
// directly by DBC id, sized to the DBC's own max id, with an explicit
// "not found" sentinel - never throws on an unknown id.
std::vector<uint16_t> legacyAreaFlagById;
std::vector<uint16_t> legacyLiquidSoundBankById;
uint32_t legacyMaxAreaId = 0;

// Cata+ area/liquid lookups: populated directly from the DBCs, keyed by id.
std::unordered_map<uint32_t, LiquidMaterialEntry> LiquidMaterials;
std::unordered_map<uint32_t, LiquidObjectEntry> LiquidObjects;
std::unordered_map<uint32_t, LiquidTypeEntry> LiquidTypes;

#define MAX_PATH_LENGTH 128
char output_path[MAX_PATH_LENGTH] = ".";
char input_path[MAX_PATH_LENGTH] = ".";

// **************************************************
// Extractor options
// **************************************************
enum Extract
{
    EXTRACT_MAP = 1,
    EXTRACT_DBC = 2,
    EXTRACT_CAMERA = 4
};

int CONF_extract = EXTRACT_MAP | EXTRACT_DBC;

bool CONF_allow_height_limit = true;
float CONF_use_minHeight = -500.0f;

bool CONF_allow_float_to_int = true;
float CONF_float_to_int8_limit = 2.0f;
float CONF_float_to_int16_limit = 2048.0f;
float CONF_flat_height_delta_limit = 0.005f;
float CONF_flat_liquid_delta_limit = 0.001f;

// Only meaningful for Cata+: which incremental wow-update patch build to
// extract up to, and the corresponding base-MPQ/patch-build list. Mop's
// list intentionally has no trailing 0 sentinel (unlike Cata's) - the
// range-based loops below don't need one.
uint32_t CONF_TargetBuild = 0;

std::vector<std::string> const CataMpqList = { "world.MPQ", "art.MPQ", "world2.MPQ", "expansion1.MPQ", "expansion2.MPQ", "expansion3.MPQ" };
std::vector<std::string> const MopMpqList = { "world.MPQ", "misc.MPQ", "expansion1.MPQ", "expansion2.MPQ", "expansion3.MPQ", "expansion4.MPQ" };
std::vector<uint32_t> const CataBuilds = { 13164, 13205, 13287, 13329, 13596, 13623, 13914, 14007, 14333, 14480, 14545, 15005, 15050, 15211, 15354, 15595 };
std::vector<uint32_t> const MopBuilds = { 16016, 16048, 16057, 16309, 16357, 16516, 16650, 16844, 16965, 17116, 17266, 17325, 17331, 17345, 17538, 17645, 17688, 17898, 18273 };

#define LAST_DBC_IN_DATA_BUILD 13623    // after this build mpqs with dbc are back to locale folder
#define NEW_BASE_SET_BUILD  15211

std::vector<std::string> const& GetTargetMpqList() { return gClientVersion == ClientVersion::MistsOfPandaria ? MopMpqList : CataMpqList; }
std::vector<uint32_t> const& GetTargetBuildList() { return gClientVersion == ClientVersion::MistsOfPandaria ? MopBuilds : CataBuilds; }

#define LOCALES_COUNT 15

char const* const Locales[LOCALES_COUNT] =
{
    "enGB", "enUS", "deDE", "esES", "frFR", "koKR", "zhCN", "zhTW", "enCN", "enTW", "esMX", "ruRU", "ptBR", "ptPT", "itIT"
};

enum LocaleConstant : uint8_t
{
    LOCALE_enUS = 0, LOCALE_koKR = 1, LOCALE_frFR = 2, LOCALE_deDE = 3, LOCALE_zhCN = 4, LOCALE_zhTW = 5,
    LOCALE_esES = 6, LOCALE_esMX = 7, LOCALE_ruRU = 8, LOCALE_NONE = 9, LOCALE_ptBR = 10, LOCALE_itIT = 11,
    TOTAL_LOCALES
};

uint8_t const MpqToWowLocale[LOCALES_COUNT] =
{
    LOCALE_enUS, LOCALE_enUS, LOCALE_deDE, LOCALE_esES, LOCALE_frFR, LOCALE_koKR, LOCALE_zhCN, LOCALE_zhTW,
    LOCALE_zhCN, LOCALE_zhTW, LOCALE_esMX, LOCALE_ruRU, LOCALE_ptBR, LOCALE_ptBR, LOCALE_itIT
};

char const* const localeNames[TOTAL_LOCALES] =
{
    "enUS", "koKR", "frFR", "deDE", "zhCN", "zhTW", "esES", "esMX", "ruRU", "none", "ptBR", "itIT"
};

bool FileExists(std::string const& fileName)
{
    return fs::exists(fileName);
}

void Usage(char const* prg)
{
    printf(
        "Usage:\n"
        "%s -[var] [value]\n"
        "-i set input path (max %d characters)\n"
        "-o set output path (max %d characters)\n"
        "-e extract only MAP(1)/DBC(2)/Camera(4, Cata+ only) - standard: MAP+DBC(3)\n"
        "-f height stored as int (less map size but lost some accuracy) 1 by default\n"
        "-b target build (Cata+ only)\n"
        "Example: %s -f 0 -i \"c:\\games\\game\"", prg, MAX_PATH_LENGTH - 1, MAX_PATH_LENGTH - 1, prg);
    exit(1);
}

void HandleArgs(int argc, char* arg[])
{
    for (int c = 1; c < argc; ++c)
    {
        if (arg[c][0] != '-')
            Usage(arg[0]);

        switch (arg[c][1])
        {
            case 'i':
                if (c + 1 < argc && strlen(arg[c + 1]) < MAX_PATH_LENGTH)
                {
                    strncpy(input_path, arg[c++ + 1], MAX_PATH_LENGTH);
                    input_path[MAX_PATH_LENGTH - 1] = '\0';
                }
                else
                    Usage(arg[0]);
                break;
            case 'o':
                if (c + 1 < argc && strlen(arg[c + 1]) < MAX_PATH_LENGTH)
                {
                    strncpy(output_path, arg[c++ + 1], MAX_PATH_LENGTH);
                    output_path[MAX_PATH_LENGTH - 1] = '\0';
                }
                else
                    Usage(arg[0]);
                break;
            case 'f':
                if (c + 1 < argc)
                    CONF_allow_float_to_int = atoi(arg[c++ + 1]) != 0;
                else
                    Usage(arg[0]);
                break;
            case 'e':
                if (c + 1 < argc)
                {
                    CONF_extract = atoi(arg[c++ + 1]);
                    if (!(CONF_extract > 0 && CONF_extract < 8))
                        Usage(arg[0]);
                }
                else
                    Usage(arg[0]);
                break;
            case 'b':
                if (c + 1 < argc)
                    CONF_TargetBuild = atoi(arg[c++ + 1]);
                else
                    Usage(arg[0]);
                break;
            default:
                break;
        }
    }
}

void CreateDir(std::string const& path)
{
    if (IsLegacyMapFormat())
    {
        // Classic/TBC/WotLK always started extraction from a clean directory.
        if (fs::exists(path))
        {
            printf("NOTE: Directory %s already exists and gets now deleted\n", path.c_str());
            fs::remove_all(path);
        }

        if (!fs::create_directory(path))
        {
            printf("Fatal Error: Could not create directory %s check your permissions\n", path.c_str());
            exit(1);
        }
        return;
    }

    // Cata+ never wiped existing output - re-running only fills in gaps
    // (ExtractDBCFiles/ExtractDB2Files/ExtractCameraFiles skip files that
    // already exist).
    if (!fs::exists(path))
        fs::create_directories(path);
}

uint32_t ReadBuild(int locale)
{
    std::string filename = std::string("component.wow-") + Locales[locale] + ".txt";

    std::vector<uint8_t> data;
    mpqlib::MpqPatchChain& chain = IsLegacyMapFormat() ? *WorldMpq : *LocaleMpq;
    if (!chain.readFile(filename, data) || data.empty())
    {
        printf("Fatal error: Not found %s file!\n", filename.c_str());
        exit(1);
    }

    size_t const textLen = std::min<size_t>(data.size(), 511);
    std::string text(reinterpret_cast<char const*>(data.data()), textLen);

    size_t pos = text.find("version=\"");
    size_t pos1 = pos + strlen("version=\"");
    size_t pos2 = text.find("\"", pos1);
    if (pos == text.npos || pos2 == text.npos || pos1 >= pos2)
    {
        printf("Fatal error: Invalid  %s file format!\n", filename.c_str());
        exit(1);
    }

    std::string build_str = text.substr(pos1, pos2 - pos1);

    int build = atoi(build_str.c_str());
    if (build <= 0)
    {
        printf("Fatal error: Invalid  %s file format!\n", filename.c_str());
        exit(1);
    }

    printf("Detected file build: %d\n", build);
    return static_cast<uint32_t>(build);
}

uint32_t ReadMapDBC()
{
    printf("Read Map.dbc file... ");

    DBCFile dbc(IsLegacyMapFormat() ? *WorldMpq : *LocaleMpq, "DBFilesClient\\Map.dbc");
    if (!dbc.open())
    {
        printf("Fatal error: Invalid Map.dbc file format!\n");
        exit(1);
    }

    size_t map_count = dbc.getRowCount();
    map_ids.resize(map_count);
    for (uint32_t x = 0; x < map_count; ++x)
    {
        map_ids[x].id = dbc.getRow(x).getUInt(0);

        char const* map_name = dbc.getRow(x).getString(1);
        size_t max_map_name_length = sizeof(map_ids[x].name);
        if (strlen(map_name) >= max_map_name_length)
        {
            printf("Fatal error: Map name too long!\n");
            exit(1);
        }

        strncpy(map_ids[x].name, map_name, max_map_name_length);
        map_ids[x].name[max_map_name_length - 1] = '\0';
    }
    printf("Done! (%u maps loaded)\n", static_cast<uint32_t>(map_count));
    return static_cast<uint32_t>(map_count);
}

// --- Legacy (Classic/TBC/WotLK) area/liquid DBC reads ---

void ReadAreaTableDBC()
{
    printf("Read AreaTable.dbc file...");
    DBCFile dbc(*WorldMpq, "DBFilesClient\\AreaTable.dbc");
    if (!dbc.open())
    {
        printf("Fatal error: Invalid AreaTable.dbc file format!\n");
        exit(1);
    }

    size_t area_count = dbc.getRowCount();
    legacyMaxAreaId = static_cast<uint32_t>(dbc.getMaxId());
    legacyAreaFlagById.assign(legacyMaxAreaId + 1, 0xffff);

    for (uint32_t x = 0; x < area_count; ++x)
        legacyAreaFlagById[dbc.getRow(x).getUInt(0)] = static_cast<uint16_t>(dbc.getRow(x).getUInt(3));

    printf("Done! (%u areas loaded)\n", static_cast<uint32_t>(area_count));
}

void ReadLegacyLiquidTypeTableDBC()
{
    printf("Read LiquidType.dbc file...");
    DBCFile dbc(*WorldMpq, "DBFilesClient\\LiquidType.dbc");
    if (!dbc.open())
    {
        printf("Fatal error: Invalid LiquidType.dbc file format!\n");
        exit(1);
    }

    size_t liqTypeCount = dbc.getRowCount();
    size_t liqTypeMaxId = dbc.getMaxId();
    legacyLiquidSoundBankById.assign(liqTypeMaxId + 1, 0xffff);

    for (uint32_t x = 0; x < liqTypeCount; ++x)
        legacyLiquidSoundBankById[dbc.getRow(x).getUInt(0)] = static_cast<uint16_t>(dbc.getRow(x).getUInt(3));

    printf("Done! (%u LiqTypes loaded)\n", static_cast<uint32_t>(liqTypeCount));
}

// --- Cata+ area/liquid DBC reads ---

#define SZFMTD "%" PRIuPTR

void ReadLiquidMaterialTable()
{
    printf("Read LiquidMaterial.dbc file...\n");
    DBCFile dbc(*LocaleMpq, "DBFilesClient\\LiquidMaterial.dbc");
    if (!dbc.open())
    {
        printf("Fatal error: Cannot find or parse LiquidMaterial.dbc in archive!\n");
        exit(1);
    }

    for (uint32_t x = 0; x < dbc.getRowCount(); ++x)
    {
        LiquidMaterialEntry& liquidType = LiquidMaterials[dbc.getRow(x).getUInt(0)];
        liquidType.LVF = static_cast<int8_t>(dbc.getRow(x).getUInt(1));
    }

    printf("Done! (" SZFMTD " LiquidMaterials loaded)\n", LiquidMaterials.size());
}

void ReadLiquidObjectTable()
{
    printf("Read LiquidObject.dbc file...\n");
    DBCFile dbc(*LocaleMpq, "DBFilesClient\\LiquidObject.dbc");
    if (!dbc.open())
    {
        printf("Fatal error: Cannot find or parse LiquidObject.dbc in archive!\n");
        exit(1);
    }

    for (uint32_t x = 0; x < dbc.getRowCount(); ++x)
    {
        LiquidObjectEntry& liquidType = LiquidObjects[dbc.getRow(x).getUInt(0)];
        liquidType.LiquidTypeID = static_cast<uint16_t>(dbc.getRow(x).getUInt(3));
    }

    printf("Done! (" SZFMTD " LiquidObjects loaded)\n", LiquidObjects.size());
}

void ReadLiquidTypeTable()
{
    printf("Read LiquidType.dbc file...");
    DBCFile dbc(*LocaleMpq, "DBFilesClient\\LiquidType.dbc");
    if (!dbc.open())
    {
        printf("Fatal error: Cannot find or parse LiquidType.dbc in archive!\n");
        exit(1);
    }

    for (uint32_t x = 0; x < dbc.getRowCount(); ++x)
    {
        LiquidTypeEntry& liquidType = LiquidTypes[dbc.getRow(x).getUInt(0)];
        liquidType.SoundBank = static_cast<uint8_t>(dbc.getRow(x).getUInt(3));
        liquidType.MaterialID = static_cast<uint8_t>(dbc.getRow(x).getUInt(14));
    }

    printf("Done! (" SZFMTD " LiquidTypes loaded)\n", LiquidTypes.size());
}

//
// Adt file convertor function and data
//

static char const* MAP_MAGIC = "MAPS";
static char const* MAP_VERSION_MAGIC = "v1.3";
static char const* MAP_AREA_MAGIC = "AREA";
static char const* MAP_HEIGHT_MAGIC = "MHGT";
static char const* MAP_LIQUID_MAGIC = "MLIQ";

struct map_fileheader
{
    uint32_t mapMagic;
    uint32_t versionMagic;
    uint32_t buildMagic;
    uint32_t areaMapOffset;
    uint32_t areaMapSize;
    uint32_t heightMapOffset;
    uint32_t heightMapSize;
    uint32_t liquidMapOffset;
    uint32_t liquidMapSize;
    uint32_t holesOffset;
    uint32_t holesSize;
};

#define MAP_AREA_NO_AREA      0x0001

struct map_areaHeader
{
    uint32_t fourcc;
    uint16_t flags;
    uint16_t gridArea;
};

#define MAP_HEIGHT_NO_HEIGHT            0x0001
#define MAP_HEIGHT_AS_INT16             0x0002
#define MAP_HEIGHT_AS_INT8              0x0004
#define MAP_HEIGHT_HAS_FLIGHT_BOUNDS    0x0008

struct map_heightHeader
{
    uint32_t fourcc;
    uint32_t flags;
    float  gridHeight;
    float  gridMaxHeight;
};

#define MAP_LIQUID_TYPE_NO_WATER    0x00
#define MAP_LIQUID_TYPE_WATER       0x01
#define MAP_LIQUID_TYPE_OCEAN       0x02
#define MAP_LIQUID_TYPE_MAGMA       0x04
#define MAP_LIQUID_TYPE_SLIME       0x08
#define MAP_LIQUID_TYPE_DARK_WATER  0x10

#define MAP_LIQUID_NO_TYPE    0x0001
#define MAP_LIQUID_NO_HEIGHT  0x0002

// On-disk liquid header layout genuinely differs by client family - kept as
// two distinct structs rather than one, each written only by its own branch
// in ConvertADT()'s liquid-packing section below.
struct LegacyLiquidHeader
{
    uint32_t fourcc;
    uint16_t flags;
    uint16_t liquidType;
    uint8_t  offsetX;
    uint8_t  offsetY;
    uint8_t  width;
    uint8_t  height;
    float  liquidLevel;
};

struct ModernLiquidHeader
{
    uint32_t fourcc;
    uint8_t flags;
    uint8_t liquidFlags;
    uint16_t liquidType;
    uint8_t  offsetX;
    uint8_t  offsetY;
    uint8_t  width;
    uint8_t  height;
    float  liquidLevel;
};

float selectUInt8StepStore(float maxDiff) { return 255 / maxDiff; }
float selectUInt16StepStore(float maxDiff) { return 65535 / maxDiff; }

// Temporary grid data store
uint16_t area_ids[ADT_CELLS_PER_GRID][ADT_CELLS_PER_GRID];

float V8[ADT_GRID_SIZE][ADT_GRID_SIZE];
float V9[ADT_GRID_SIZE + 1][ADT_GRID_SIZE + 1];
uint16_t uint16_V8[ADT_GRID_SIZE][ADT_GRID_SIZE];
uint16_t uint16_V9[ADT_GRID_SIZE + 1][ADT_GRID_SIZE + 1];
uint8_t  uint8_V8[ADT_GRID_SIZE][ADT_GRID_SIZE];
uint8_t  uint8_V9[ADT_GRID_SIZE + 1][ADT_GRID_SIZE + 1];

uint16_t liquid_entry[ADT_CELLS_PER_GRID][ADT_CELLS_PER_GRID];
uint8_t liquid_flags[ADT_CELLS_PER_GRID][ADT_CELLS_PER_GRID];
bool  liquid_show[ADT_GRID_SIZE][ADT_GRID_SIZE];
float liquid_height[ADT_GRID_SIZE + 1][ADT_GRID_SIZE + 1];
uint16_t holes[ADT_CELLS_PER_GRID][ADT_CELLS_PER_GRID];

int16_t flight_box_max[3][3];
int16_t flight_box_min[3][3];

LiquidVertexFormatType adt_MH2O::GetLiquidVertexFormat(adt_liquid_instance const* liquidInstance) const
{
    if (liquidInstance->LiquidVertexFormat < 42)
        return static_cast<LiquidVertexFormatType>(liquidInstance->LiquidVertexFormat);

    if (liquidInstance->LiquidType == 2)
        return LiquidVertexFormatType::Depth;

    auto liquidType = LiquidTypes.find(liquidInstance->LiquidType);
    if (liquidType != LiquidTypes.end())
    {
        auto liquidMaterial = LiquidMaterials.find(liquidType->second.MaterialID);
        if (liquidMaterial != LiquidMaterials.end())
            return static_cast<LiquidVertexFormatType>(liquidMaterial->second.LVF);
    }

    return static_cast<LiquidVertexFormatType>(-1);
}

// Cata-only: two zones whose water was reworked to no longer apply the
// classic "deep water" fatigue/dark-water treatment.
bool IsDeepWaterIgnored(uint32_t mapId, uint32_t x, uint32_t y)
{
    if (mapId == 0)
    {
        // Vashj'ir grids completely ignore fatigue
        return (x >= 39 && x <= 40 && y >= 24 && y <= 26) || (x >= 41 && x <= 46 && y >= 18 && y <= 26);
    }

    if (mapId == 1)
    {
        // Thousand Needles
        return x == 43 && (y == 39 || y == 40);
    }

    return false;
}

bool ConvertADT(std::string const& inputPath, std::string const& outputPath, int /*cell_y*/, int /*cell_x*/, uint32_t build, bool ignoreDeepWater)
{
    auto adt = loadAdtChunkTree(inputPath);
    if (!adt)
        return false;

    bool const legacy = IsLegacyMapFormat();

    map_fileheader map{};
    map.mapMagic = *reinterpret_cast<uint32_t const*>(MAP_MAGIC);
    map.versionMagic = *reinterpret_cast<uint32_t const*>(MAP_VERSION_MAGIC);
    map.buildMagic = build;

    memset(area_ids, 0, sizeof(area_ids));
    memset(V9, 0, sizeof(V9));
    memset(V8, 0, sizeof(V8));
    memset(liquid_show, 0, sizeof(liquid_show));
    memset(liquid_flags, 0, sizeof(liquid_flags));
    memset(liquid_entry, 0, sizeof(liquid_entry));
    memset(holes, 0, sizeof(holes));

    bool hasHoles = false;
    bool hasFlightBox = false;
    bool foundAnyCell = false;

    for (mpqlib::ChunkNode const* mcnkNode : adt->findAll("MCNK"))
    {
        adt_MCNK const* mcnk = &mcnkNode->as<adt_MCNK>();

        if (mcnk->iy > ADT_CELLS_PER_GRID - 1 || mcnk->ix > ADT_CELLS_PER_GRID - 1)
        {
            printf("\n ADT_CELLS_PER_GRID %u max %u\n", mcnk->iy, ADT_CELLS_PER_GRID);
            continue;
        }

        foundAnyCell = true;

        // Area data. Classic/TBC/WotLK stored a pre-resolved AreaTable.dbc
        // flag; Cata+ stores the raw MCNK areaid and resolves it at runtime.
        if (legacy)
        {
            uint32_t areaid = mcnk->areaid;
            if (areaid && areaid <= legacyMaxAreaId && legacyAreaFlagById[areaid] != 0xffff)
                area_ids[mcnk->iy][mcnk->ix] = legacyAreaFlagById[areaid];
            else
            {
                if (areaid && areaid <= legacyMaxAreaId)
                    printf("File: %s\nCan't find area flag for areaid %u [%u, %u].\n", inputPath.c_str(), areaid, mcnk->ix, mcnk->iy);
                area_ids[mcnk->iy][mcnk->ix] = 0xffff;
            }
        }
        else
            area_ids[mcnk->iy][mcnk->ix] = static_cast<uint16_t>(mcnk->areaid);

        // Height values for triangles stored in order:
        // 1     2     3     4     5     6     7     8     9
        //    10    11    12    13    14    15    16    17
        // 18    19    20    21    22    23    24    25    26
        //    27    28    29    30    31    32    33    34
        // . . . . . . . .
        // For better get height values merge it to V9 and V8 map
        // V9 height map:
        // 1     2     3     4     5     6     7     8     9
        // 18    19    20    21    22    23    24    25    26
        // . . . . . . . .
        // V8 height map:
        //    10    11    12    13    14    15    16    17
        //    27    28    29    30    31    32    33    34
        // . . . . . . . .

        for (int y = 0; y <= ADT_CELL_SIZE; y++)
        {
            int cy = mcnk->iy * ADT_CELL_SIZE + y;
            for (int x = 0; x <= ADT_CELL_SIZE; x++)
            {
                int cx = mcnk->ix * ADT_CELL_SIZE + x;
                V9[cy][cx] = mcnk->ypos;
            }
        }
        for (int y = 0; y < ADT_CELL_SIZE; y++)
        {
            int cy = mcnk->iy * ADT_CELL_SIZE + y;
            for (int x = 0; x < ADT_CELL_SIZE; x++)
            {
                int cx = mcnk->ix * ADT_CELL_SIZE + x;
                V8[cy][cx] = mcnk->ypos;
            }
        }

        if (mpqlib::ChunkNode const* chunk = mcnkNode->find("MCVT"))
        {
            adt_MCVT const* mcvt = &chunk->as<adt_MCVT>();
            for (int y = 0; y <= ADT_CELL_SIZE; y++)
            {
                int cy = mcnk->iy * ADT_CELL_SIZE + y;
                for (int x = 0; x <= ADT_CELL_SIZE; x++)
                {
                    int cx = mcnk->ix * ADT_CELL_SIZE + x;
                    V9[cy][cx] += mcvt->height_map[y * (ADT_CELL_SIZE * 2 + 1) + x];
                }
            }
            for (int y = 0; y < ADT_CELL_SIZE; y++)
            {
                int cy = mcnk->iy * ADT_CELL_SIZE + y;
                for (int x = 0; x < ADT_CELL_SIZE; x++)
                {
                    int cx = mcnk->ix * ADT_CELL_SIZE + x;
                    V8[cy][cx] += mcvt->height_map[y * (ADT_CELL_SIZE * 2 + 1) + ADT_CELL_SIZE + 1 + x];
                }
            }
        }

        // Liquid data from the old, per-MCNK MCLQ chunk.
        if (mcnk->sizeMCLQ > 8)
        {
            if (mpqlib::ChunkNode const* chunk = mcnkNode->find("MCLQ"))
            {
                adt_MCLQ const* liquid = &chunk->as<adt_MCLQ>();
                int count = 0;
                for (int y = 0; y < ADT_CELL_SIZE; ++y)
                {
                    int cy = mcnk->iy * ADT_CELL_SIZE + y;
                    for (int x = 0; x < ADT_CELL_SIZE; ++x)
                    {
                        int cx = mcnk->ix * ADT_CELL_SIZE + x;
                        if (liquid->flags[y][x] != 0x0F)
                        {
                            liquid_show[cy][cx] = true;
                            if (!ignoreDeepWater && liquid->flags[y][x] & (1 << 7))
                                liquid_flags[mcnk->iy][mcnk->ix] |= MAP_LIQUID_TYPE_DARK_WATER;
                            ++count;
                        }
                    }
                }

                uint32_t c_flag = mcnk->flags;
                if (c_flag & (1 << 2))
                {
                    liquid_entry[mcnk->iy][mcnk->ix] = 1;
                    liquid_flags[mcnk->iy][mcnk->ix] |= MAP_LIQUID_TYPE_WATER;
                }
                if (c_flag & (1 << 3))
                {
                    liquid_entry[mcnk->iy][mcnk->ix] = 2;
                    liquid_flags[mcnk->iy][mcnk->ix] |= MAP_LIQUID_TYPE_OCEAN;
                }
                if (c_flag & (1 << 4))
                {
                    liquid_entry[mcnk->iy][mcnk->ix] = 3;
                    liquid_flags[mcnk->iy][mcnk->ix] |= MAP_LIQUID_TYPE_MAGMA;
                }

                if (!count && liquid_flags[mcnk->iy][mcnk->ix])
                    fprintf(stderr, "Wrong liquid detect in MCLQ chunk");

                for (int y = 0; y <= ADT_CELL_SIZE; ++y)
                {
                    int cy = mcnk->iy * ADT_CELL_SIZE + y;
                    for (int x = 0; x <= ADT_CELL_SIZE; ++x)
                    {
                        int cx = mcnk->ix * ADT_CELL_SIZE + x;
                        liquid_height[cy][cx] = liquid->liquid[y][x].height;
                    }
                }
            }
        }

        // Hole data
        holes[mcnk->iy][mcnk->ix] = static_cast<uint16_t>(mcnk->holes);
        if (!hasHoles && mcnk->holes != 0)
            hasHoles = true;
    }

    if (!foundAnyCell)
    {
        printf("Can't find cells in '%s'\n", inputPath.c_str());
        return false;
    }

    // Liquid map for the grid, from the MH2O chunk (introduced in WotLK).
    if (mpqlib::ChunkNode const* chunk = adt->find("MH2O"))
    {
        adt_MH2O const* h2o = &chunk->as<adt_MH2O>();
        for (int i = 0; i < ADT_CELLS_PER_GRID; i++)
        {
            for (int j = 0; j < ADT_CELLS_PER_GRID; j++)
            {
                adt_liquid_instance const* h = h2o->GetLiquidInstance(i, j);
                if (!h)
                    continue;

                if (legacy)
                {
                    int count = 0;
                    uint64_t show = h2o->GetLegacyLiquidShowMap(h);
                    for (int y = 0; y < h->Height; y++)
                    {
                        int cy = i * ADT_CELL_SIZE + y + h->OffsetY;
                        for (int x = 0; x < h->Width; x++)
                        {
                            int cx = j * ADT_CELL_SIZE + x + h->OffsetX;
                            if (show & 1)
                            {
                                liquid_show[cy][cx] = true;
                                ++count;
                            }
                            show >>= 1;
                        }
                    }

                    liquid_entry[i][j] = h->LiquidType;
                    uint16_t soundBank = h->LiquidType < legacyLiquidSoundBankById.size() ? legacyLiquidSoundBankById[h->LiquidType] : 0xffff;
                    switch (soundBank)
                    {
                        case LIQUID_TYPE_WATER: liquid_flags[i][j] |= MAP_LIQUID_TYPE_WATER; break;
                        case LIQUID_TYPE_OCEAN: liquid_flags[i][j] |= MAP_LIQUID_TYPE_OCEAN; break;
                        case LIQUID_TYPE_MAGMA: liquid_flags[i][j] |= MAP_LIQUID_TYPE_MAGMA; break;
                        case LIQUID_TYPE_SLIME: liquid_flags[i][j] |= MAP_LIQUID_TYPE_SLIME; break;
                        default:
                            printf("\nCan't find Liquid type %u for map %s\nchunk %d,%d\n", h->LiquidType, inputPath.c_str(), i, j);
                            break;
                    }

                    if (soundBank == LIQUID_TYPE_OCEAN && !h2o->GetLegacyLiquidLightMap(h))
                        liquid_flags[i][j] |= MAP_LIQUID_TYPE_DARK_WATER;

                    if (!count && liquid_flags[i][j])
                        printf("Wrong liquid detect in MH2O chunk");

                    float const* height = h2o->GetLegacyLiquidHeightMap(h);
                    int pos = 0;
                    for (int y = 0; y <= h->Height; y++)
                    {
                        int cy = i * ADT_CELL_SIZE + y + h->OffsetY;
                        for (int x = 0; x <= h->Width; x++)
                        {
                            int cx = j * ADT_CELL_SIZE + x + h->OffsetX;
                            liquid_height[cy][cx] = height ? height[pos] : h->MinHeightLevel;
                            pos++;
                        }
                    }
                }
                else
                {
                    adt_liquid_attributes attrs = h2o->GetLiquidAttributes(i, j);

                    int count = 0;
                    uint64_t existsMask = h2o->GetLiquidExistsBitmap(h);
                    for (int y = 0; y < h->GetHeight(); y++)
                    {
                        int cy = i * ADT_CELL_SIZE + y + h->GetOffsetY();
                        for (int x = 0; x < h->GetWidth(); x++)
                        {
                            int cx = j * ADT_CELL_SIZE + x + h->GetOffsetX();
                            if (existsMask & 1)
                            {
                                liquid_show[cy][cx] = true;
                                ++count;
                            }
                            existsMask >>= 1;
                        }
                    }

                    liquid_entry[i][j] = h2o->GetLiquidType(h);
                    switch (LiquidTypes.at(liquid_entry[i][j]).SoundBank)
                    {
                        case LIQUID_TYPE_WATER: liquid_flags[i][j] |= MAP_LIQUID_TYPE_WATER; break;
                        case LIQUID_TYPE_OCEAN: liquid_flags[i][j] |= MAP_LIQUID_TYPE_OCEAN; if (!ignoreDeepWater && attrs.Deep) liquid_flags[i][j] |= MAP_LIQUID_TYPE_DARK_WATER; break;
                        case LIQUID_TYPE_MAGMA: liquid_flags[i][j] |= MAP_LIQUID_TYPE_MAGMA; break;
                        case LIQUID_TYPE_SLIME: liquid_flags[i][j] |= MAP_LIQUID_TYPE_SLIME; break;
                        default:
                            printf("\nCan't find Liquid type %u for map %s\nchunk %d,%d\n", h->LiquidType, inputPath.c_str(), i, j);
                            break;
                    }

                    if (!count && liquid_flags[i][j])
                        printf("Wrong liquid detect in MH2O chunk");

                    int pos = 0;
                    for (int y = 0; y <= h->GetHeight(); y++)
                    {
                        int cy = i * ADT_CELL_SIZE + y + h->GetOffsetY();
                        for (int x = 0; x <= h->GetWidth(); x++)
                        {
                            int cx = j * ADT_CELL_SIZE + x + h->GetOffsetX();
                            liquid_height[cy][cx] = h2o->GetLiquidHeight(h, pos);
                            pos++;
                        }
                    }
                }
            }
        }
    }

    if (mpqlib::ChunkNode const* chunk = adt->find("MFBO"))
    {
        adt_MFBO const* mfbo = &chunk->as<adt_MFBO>();
        memcpy(flight_box_max, &mfbo->max, sizeof(flight_box_max));
        memcpy(flight_box_min, &mfbo->min, sizeof(flight_box_min));
        hasFlightBox = true;
    }

    //============================================
    // Try pack area data
    //============================================
    bool fullAreaData = false;
    uint32_t areaId = area_ids[0][0];
    for (int y = 0; y < ADT_CELLS_PER_GRID; ++y)
    {
        for (int x = 0; x < ADT_CELLS_PER_GRID; ++x)
        {
            if (area_ids[y][x] != areaId)
            {
                fullAreaData = true;
                break;
            }
        }
    }

    map.areaMapOffset = sizeof(map);
    map.areaMapSize = sizeof(map_areaHeader);

    map_areaHeader areaHeader;
    areaHeader.fourcc = *reinterpret_cast<uint32_t const*>(MAP_AREA_MAGIC);
    areaHeader.flags = 0;
    if (fullAreaData)
    {
        areaHeader.gridArea = 0;
        map.areaMapSize += sizeof(area_ids);
    }
    else
    {
        areaHeader.flags |= MAP_AREA_NO_AREA;
        areaHeader.gridArea = static_cast<uint16_t>(areaId);
    }

    //============================================
    // Try pack height data
    //============================================
    float maxHeight = -20000;
    float minHeight = 20000;
    for (int y = 0; y < ADT_GRID_SIZE; y++)
    {
        for (int x = 0; x < ADT_GRID_SIZE; x++)
        {
            float h = V8[y][x];
            if (maxHeight < h) maxHeight = h;
            if (minHeight > h) minHeight = h;
        }
    }
    for (int y = 0; y <= ADT_GRID_SIZE; y++)
    {
        for (int x = 0; x <= ADT_GRID_SIZE; x++)
        {
            float h = V9[y][x];
            if (maxHeight < h) maxHeight = h;
            if (minHeight > h) minHeight = h;
        }
    }

    if (CONF_allow_height_limit && minHeight < CONF_use_minHeight)
    {
        for (int y = 0; y < ADT_GRID_SIZE; y++)
            for (int x = 0; x < ADT_GRID_SIZE; x++)
                if (V8[y][x] < CONF_use_minHeight)
                    V8[y][x] = CONF_use_minHeight;
        for (int y = 0; y <= ADT_GRID_SIZE; y++)
            for (int x = 0; x <= ADT_GRID_SIZE; x++)
                if (V9[y][x] < CONF_use_minHeight)
                    V9[y][x] = CONF_use_minHeight;
        if (minHeight < CONF_use_minHeight)
            minHeight = CONF_use_minHeight;
        if (maxHeight < CONF_use_minHeight)
            maxHeight = CONF_use_minHeight;
    }

    map.heightMapOffset = map.areaMapOffset + map.areaMapSize;
    map.heightMapSize = sizeof(map_heightHeader);

    map_heightHeader heightHeader;
    heightHeader.fourcc = *reinterpret_cast<uint32_t const*>(MAP_HEIGHT_MAGIC);
    heightHeader.flags = 0;
    heightHeader.gridHeight = minHeight;
    heightHeader.gridMaxHeight = maxHeight;

    if (maxHeight == minHeight)
        heightHeader.flags |= MAP_HEIGHT_NO_HEIGHT;

    if (CONF_allow_float_to_int && (maxHeight - minHeight) < CONF_flat_height_delta_limit)
        heightHeader.flags |= MAP_HEIGHT_NO_HEIGHT;

    if (hasFlightBox)
    {
        heightHeader.flags |= MAP_HEIGHT_HAS_FLIGHT_BOUNDS;
        map.heightMapSize += sizeof(flight_box_max) + sizeof(flight_box_min);
    }

    if (!(heightHeader.flags & MAP_HEIGHT_NO_HEIGHT))
    {
        float step = 0;
        if (CONF_allow_float_to_int)
        {
            float diff = maxHeight - minHeight;
            if (diff < CONF_float_to_int8_limit)
            {
                heightHeader.flags |= MAP_HEIGHT_AS_INT8;
                step = selectUInt8StepStore(diff);
            }
            else if (diff < CONF_float_to_int16_limit)
            {
                heightHeader.flags |= MAP_HEIGHT_AS_INT16;
                step = selectUInt16StepStore(diff);
            }
        }

        if (heightHeader.flags & MAP_HEIGHT_AS_INT8)
        {
            for (int y = 0; y < ADT_GRID_SIZE; y++)
                for (int x = 0; x < ADT_GRID_SIZE; x++)
                    uint8_V8[y][x] = uint8_t((V8[y][x] - minHeight) * step + 0.5f);
            for (int y = 0; y <= ADT_GRID_SIZE; y++)
                for (int x = 0; x <= ADT_GRID_SIZE; x++)
                    uint8_V9[y][x] = uint8_t((V9[y][x] - minHeight) * step + 0.5f);
            map.heightMapSize += sizeof(uint8_V9) + sizeof(uint8_V8);
        }
        else if (heightHeader.flags & MAP_HEIGHT_AS_INT16)
        {
            for (int y = 0; y < ADT_GRID_SIZE; y++)
                for (int x = 0; x < ADT_GRID_SIZE; x++)
                    uint16_V8[y][x] = uint16_t((V8[y][x] - minHeight) * step + 0.5f);
            for (int y = 0; y <= ADT_GRID_SIZE; y++)
                for (int x = 0; x <= ADT_GRID_SIZE; x++)
                    uint16_V9[y][x] = uint16_t((V9[y][x] - minHeight) * step + 0.5f);
            map.heightMapSize += sizeof(uint16_V9) + sizeof(uint16_V8);
        }
        else
            map.heightMapSize += sizeof(V9) + sizeof(V8);
    }

    //============================================
    // Pack liquid data
    //============================================
    uint16_t firstLiquidType = liquid_entry[0][0];
    uint8_t firstLiquidFlag = liquid_flags[0][0];
    bool fullType = false;
    for (int y = 0; y < ADT_CELLS_PER_GRID; y++)
    {
        for (int x = 0; x < ADT_CELLS_PER_GRID; x++)
        {
            if (liquid_entry[y][x] != firstLiquidType || liquid_flags[y][x] != firstLiquidFlag)
            {
                fullType = true;
                y = ADT_CELLS_PER_GRID;
                break;
            }
        }
    }

    LegacyLiquidHeader legacyLiquidHeader{};
    ModernLiquidHeader modernLiquidHeader{};
    uint32_t liquidHeaderSize = legacy ? sizeof(legacyLiquidHeader) : sizeof(modernLiquidHeader);
    uint8_t liquidFlagsField = 0;
    uint8_t liquidWidth = 0, liquidHeightField = 0;
    uint8_t liquidOffsetX = 0, liquidOffsetY = 0;

    if (firstLiquidFlag == 0 && !fullType)
    {
        map.liquidMapOffset = 0;
        map.liquidMapSize = 0;
    }
    else
    {
        int minX = 255, minY = 255;
        int maxX = 0, maxY = 0;
        maxHeight = -20000;
        minHeight = 20000;
        for (int y = 0; y < ADT_GRID_SIZE; y++)
        {
            for (int x = 0; x < ADT_GRID_SIZE; x++)
            {
                if (liquid_show[y][x])
                {
                    if (minX > x) minX = x;
                    if (maxX < x) maxX = x;
                    if (minY > y) minY = y;
                    if (maxY < y) maxY = y;
                    float h = liquid_height[y][x];
                    if (maxHeight < h) maxHeight = h;
                    if (minHeight > h) minHeight = h;
                }
                else
                {
                    liquid_height[y][x] = CONF_use_minHeight;
                    if (!legacy && minHeight > CONF_use_minHeight)
                        minHeight = CONF_use_minHeight;
                }
            }
        }
        map.liquidMapOffset = map.heightMapOffset + map.heightMapSize;
        map.liquidMapSize = liquidHeaderSize;

        liquidOffsetX = static_cast<uint8_t>(minX);
        liquidOffsetY = static_cast<uint8_t>(minY);
        liquidWidth = static_cast<uint8_t>(maxX - minX + 1 + 1);
        liquidHeightField = static_cast<uint8_t>(maxY - minY + 1 + 1);

        if (maxHeight == minHeight)
            liquidFlagsField |= MAP_LIQUID_NO_HEIGHT;

        if (CONF_allow_float_to_int && (maxHeight - minHeight) < CONF_flat_liquid_delta_limit)
            liquidFlagsField |= MAP_LIQUID_NO_HEIGHT;

        if (!fullType)
            liquidFlagsField |= MAP_LIQUID_NO_TYPE;

        uint16_t liquidTypeField = 0;
        if (liquidFlagsField & MAP_LIQUID_NO_TYPE)
            liquidTypeField = legacy ? static_cast<uint16_t>(firstLiquidFlag) : firstLiquidType;
        else
            map.liquidMapSize += sizeof(liquid_entry) + sizeof(liquid_flags);

        if (!(liquidFlagsField & MAP_LIQUID_NO_HEIGHT))
            map.liquidMapSize += sizeof(float) * liquidWidth * liquidHeightField;

        if (legacy)
        {
            legacyLiquidHeader.fourcc = *reinterpret_cast<uint32_t const*>(MAP_LIQUID_MAGIC);
            legacyLiquidHeader.flags = liquidFlagsField;
            legacyLiquidHeader.liquidType = liquidTypeField;
            legacyLiquidHeader.offsetX = liquidOffsetX;
            legacyLiquidHeader.offsetY = liquidOffsetY;
            legacyLiquidHeader.width = liquidWidth;
            legacyLiquidHeader.height = liquidHeightField;
            legacyLiquidHeader.liquidLevel = minHeight;
        }
        else
        {
            modernLiquidHeader.fourcc = *reinterpret_cast<uint32_t const*>(MAP_LIQUID_MAGIC);
            modernLiquidHeader.flags = liquidFlagsField;
            modernLiquidHeader.liquidFlags = (liquidFlagsField & MAP_LIQUID_NO_TYPE) ? firstLiquidFlag : 0;
            modernLiquidHeader.liquidType = liquidTypeField;
            modernLiquidHeader.offsetX = liquidOffsetX;
            modernLiquidHeader.offsetY = liquidOffsetY;
            modernLiquidHeader.width = liquidWidth;
            modernLiquidHeader.height = liquidHeightField;
            modernLiquidHeader.liquidLevel = minHeight;
        }
    }

    if (legacy)
    {
        // WotLK always stamps holesOffset, even when there's no hole data
        // to go with it - only holesSize is conditional there.
        if (map.liquidMapOffset)
            map.holesOffset = map.liquidMapOffset + map.liquidMapSize;
        else
            map.holesOffset = map.heightMapOffset + map.heightMapSize;
        map.holesSize = hasHoles ? sizeof(holes) : 0;
    }
    else if (hasHoles)
    {
        if (map.liquidMapOffset)
            map.holesOffset = map.liquidMapOffset + map.liquidMapSize;
        else
            map.holesOffset = map.heightMapOffset + map.heightMapSize;
        map.holesSize = sizeof(holes);
    }
    else
    {
        map.holesOffset = 0;
        map.holesSize = 0;
    }

    std::ofstream outFile(outputPath, std::ofstream::out | std::ofstream::binary);
    if (!outFile)
    {
        printf("Can't create the output file '%s'\n", outputPath.c_str());
        return false;
    }

    outFile.write(reinterpret_cast<char const*>(&map), sizeof(map));
    outFile.write(reinterpret_cast<char const*>(&areaHeader), sizeof(areaHeader));
    if (!(areaHeader.flags & MAP_AREA_NO_AREA))
        outFile.write(reinterpret_cast<char const*>(area_ids), sizeof(area_ids));

    outFile.write(reinterpret_cast<char const*>(&heightHeader), sizeof(heightHeader));
    if (!(heightHeader.flags & MAP_HEIGHT_NO_HEIGHT))
    {
        if (heightHeader.flags & MAP_HEIGHT_AS_INT16)
        {
            outFile.write(reinterpret_cast<char const*>(uint16_V9), sizeof(uint16_V9));
            outFile.write(reinterpret_cast<char const*>(uint16_V8), sizeof(uint16_V8));
        }
        else if (heightHeader.flags & MAP_HEIGHT_AS_INT8)
        {
            outFile.write(reinterpret_cast<char const*>(uint8_V9), sizeof(uint8_V9));
            outFile.write(reinterpret_cast<char const*>(uint8_V8), sizeof(uint8_V8));
        }
        else
        {
            outFile.write(reinterpret_cast<char const*>(V9), sizeof(V9));
            outFile.write(reinterpret_cast<char const*>(V8), sizeof(V8));
        }
    }

    if (heightHeader.flags & MAP_HEIGHT_HAS_FLIGHT_BOUNDS)
    {
        outFile.write(reinterpret_cast<char const*>(flight_box_max), sizeof(flight_box_max));
        outFile.write(reinterpret_cast<char const*>(flight_box_min), sizeof(flight_box_min));
    }

    if (map.liquidMapOffset)
    {
        if (legacy)
            outFile.write(reinterpret_cast<char const*>(&legacyLiquidHeader), sizeof(legacyLiquidHeader));
        else
            outFile.write(reinterpret_cast<char const*>(&modernLiquidHeader), sizeof(modernLiquidHeader));

        if (!(liquidFlagsField & MAP_LIQUID_NO_TYPE))
        {
            outFile.write(reinterpret_cast<char const*>(liquid_entry), sizeof(liquid_entry));
            outFile.write(reinterpret_cast<char const*>(liquid_flags), sizeof(liquid_flags));
        }

        if (!(liquidFlagsField & MAP_LIQUID_NO_HEIGHT))
        {
            for (int y = 0; y < liquidHeightField; y++)
                outFile.write(reinterpret_cast<char const*>(&liquid_height[y + liquidOffsetY][liquidOffsetX]), sizeof(float) * liquidWidth);
        }
    }

    if (hasHoles)
        outFile.write(reinterpret_cast<char const*>(holes), map.holesSize);

    outFile.close();

    return true;
}

void ExtractMapsFromMpq(uint32_t build)
{
    char mpq_filename[1024];
    char output_filename[1024];
    char mpq_map_name[1024];

    printf("Extracting maps...\n");

    uint32_t map_count = ReadMapDBC();

    if (IsLegacyMapFormat())
    {
        ReadAreaTableDBC();
        ReadLegacyLiquidTypeTableDBC();
    }
    else
    {
        ReadLiquidMaterialTable();
        ReadLiquidObjectTable();
        ReadLiquidTypeTable();
    }

    std::string path = output_path;
    path += "/maps/";
    CreateDir(path);

    printf("Convert map files\n");
    for (uint32_t z = 0; z < map_count; ++z)
    {
        printf("Extract %s (%u/%u)                  \n", map_ids[z].name, z + 1, map_count);
        sprintf(mpq_map_name, "World\\Maps\\%s\\%s.wdt", map_ids[z].name, map_ids[z].name);

        auto wdt = loadWdtChunkTree(mpq_map_name, false);
        if (!wdt || (IsLegacyMapFormat() && !hasLegacyWdtChunks(*wdt)))
            continue;

        mpqlib::ChunkNode const* main = wdt->find("MAIN");
        if (!main)
            continue;

        wdt_MAIN const& mainData = main->as<wdt_MAIN>();
        for (uint32_t y = 0; y < WDT_MAP_SIZE; ++y)
        {
            for (uint32_t x = 0; x < WDT_MAP_SIZE; ++x)
            {
                if (!(mainData.adt_list[y][x].flag & 0x1))
                    continue;

                sprintf(mpq_filename, "World\\Maps\\%s\\%s_%u_%u.adt", map_ids[z].name, map_ids[z].name, x, y);
                sprintf(output_filename, "%s/maps/%04u_%02u_%02u.map", output_path, map_ids[z].id, y, x);
                bool ignoreDeepWater = !IsLegacyMapFormat() && IsDeepWaterIgnored(map_ids[z].id, y, x);
                ConvertADT(mpq_filename, output_filename, y, x, build, ignoreDeepWater);
            }
            printf("Processing........................%d%%\r", (100 * (y + 1)) / WDT_MAP_SIZE);
        }
    }
    printf("\n");
}

bool ExtractFile(mpqlib::MpqPatchChain& mpq, char const* mpqFileName, char const* filename)
{
    std::vector<uint8_t> data;
    if (!mpq.readFile(mpqFileName, data))
        return false;

    FILE* output = fopen(filename, "wb");
    if (!output)
    {
        printf("Can't create the output file '%s'\n", filename);
        return false;
    }

    if (!data.empty())
        fwrite(data.data(), 1, data.size(), output);

    fclose(output);
    return true;
}

// --- Legacy (Classic/TBC/WotLK) DBC extraction: flat archive, no locale subfolder split beyond the top-level one ---

void ExtractLegacyDBCFiles(int locale, bool basicLocale)
{
    printf("Extracting dbc files...\n");

    std::set<std::string> dbcfiles;
    std::vector<std::string> files = WorldMpq->listFiles();
    for (auto const& file : files)
    {
        if (file.rfind(".dbc") == file.length() - strlen(".dbc"))
            dbcfiles.insert(file);
        if (file.rfind(".db2") == file.length() - strlen(".db2"))
            dbcfiles.insert(file);
    }

    std::string path = output_path;
    path += "/dbc/";
    CreateDir(path);
    if (!basicLocale)
    {
        path += Locales[locale];
        path += "/";
        CreateDir(path);
    }

    if (gClientVersion != ClientVersion::Vanilla)
    {
        std::string mpq_name = std::string("component.wow-") + Locales[locale] + ".txt";
        ExtractFile(*WorldMpq, mpq_name.c_str(), (path + mpq_name).c_str());
    }

    uint32_t count = 0;
    for (auto const& file : dbcfiles)
    {
        std::string filename = path + (file.c_str() + strlen("DBFilesClient\\"));
        if (ExtractFile(*WorldMpq, file.c_str(), filename.c_str()))
            ++count;
    }
    printf("Extracted %u DBC files\n\n", count);
}

// --- Cata+ DBC/DB2/Camera extraction: pattern-matched listing into a per-locale subfolder ---

// Minimal single-'*' wildcard matcher, replacing StormLib's SFileFindFirstFile/
// SFileFindNextFile - the only two patterns this tool ever searches for
// ("DBFilesClient\\*dbc" / "DBFilesClient\\*db2") are simple prefix+suffix matches.
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

void ExtractDBCFiles(int l)
{
    printf("Extracting dbc files...\n");

    uint32_t count = 0;

    std::string outputPath = output_path;
    outputPath += "/dbc/";
    CreateDir(outputPath);
    outputPath += localeNames[MpqToWowLocale[l]];
    outputPath += "/";
    CreateDir(outputPath);

    for (std::string const& mpqFileName : LocaleMpq->listFiles())
    {
        if (!MatchesSimplePattern(mpqFileName, "DBFilesClient\\*dbc"))
            continue;

        std::string filename = outputPath + mpqFileName.substr(mpqFileName.rfind('\\') + 1);
        if (FileExists(filename))
            continue;

        if (ExtractFile(*LocaleMpq, mpqFileName.c_str(), filename.c_str()))
            ++count;
    }

    printf("Extracted %u DBC files\n\n", count);
}

void ExtractDB2Files(int l)
{
    printf("Extracting db2 files...\n");

    uint32_t count = 0;

    std::string outputPath = output_path;
    outputPath += "/dbc/";
    outputPath += localeNames[MpqToWowLocale[l]];
    outputPath += "/";

    for (std::string const& mpqFileName : LocaleMpq->listFiles())
    {
        if (!MatchesSimplePattern(mpqFileName, "DBFilesClient\\*db2"))
            continue;

        std::string filename = outputPath + mpqFileName.substr(mpqFileName.rfind('\\') + 1);
        if (ExtractFile(*LocaleMpq, mpqFileName.c_str(), filename.c_str()))
            ++count;
    }

    printf("Extracted %u DB2 files\n\n", count);
}

void ExtractCameraFiles()
{
    printf("Extracting camera files...\n");

    DBCFile camdbc(*LocaleMpq, "DBFilesClient\\CinematicCamera.dbc");
    if (!camdbc.open())
    {
        printf("Unable to open CinematicCamera.dbc. Camera extract aborted.\n");
        return;
    }

    std::vector<std::string> camerafiles;
    size_t cam_count = camdbc.getRowCount();
    for (size_t i = 0; i < cam_count; ++i)
    {
        std::string camFile(camdbc.getRow(i).getString(1));
        size_t loc = camFile.find(".mdx");
        if (loc != std::string::npos)
            camFile.replace(loc, 4, ".m2");
        camerafiles.push_back(camFile);
    }

    std::string path = output_path;
    path += "/Cameras/";
    CreateDir(path);

    uint32_t count = 0;
    for (std::string const& thisFile : camerafiles)
    {
        std::string filename = path + (thisFile.c_str() + strlen("Cameras\\"));
        if (FileExists(filename))
            continue;

        if (ExtractFile(*WorldMpq, thisFile.c_str(), filename.c_str()))
            ++count;
    }
    printf("Extracted %u camera files\n", count);
}

// --- Legacy MPQ loading: flat, priority-ordered archive list ---

enum VersionMask : uint8_t
{
    MaskNone = 0x0,
    MaskClassic = 0x01,
    MaskBC = 0x02,
    MaskWotLK = 0x04,

    MaskBCWotLK = MaskBC | MaskWotLK,
    MaskClassicBCWotLK = MaskClassic | MaskBCWotLK,
};

VersionMask getLegacyVersionMask()
{
    switch (gClientVersion)
    {
        case ClientVersion::Vanilla: return MaskClassic;
        case ClientVersion::BurningCrusade: return MaskBC;
        case ClientVersion::WrathOfTheLichKing: return MaskWotLK;
        default: return MaskNone;
    }
}

struct MpqList
{
    uint32_t versionMask;
    std::string fileName;
};

std::vector<MpqList> const legacyMpqList{
    {MaskClassic, "dbc.MPQ"},
    {MaskClassic, "terrain.MPQ"},

    {MaskClassicBCWotLK, "patch.MPQ"},
    {MaskClassicBCWotLK, "patch-2.MPQ"},

    {MaskBCWotLK, "common.MPQ"},
    {MaskWotLK, "common-2.MPQ"},
    {MaskWotLK, "lichking.MPQ"},
    {MaskBCWotLK, "expansion.MPQ"},
    {MaskWotLK, "patch-3.MPQ"},
};

// mpqlib::MpqPatchChain has an explicit base + patches, unlike the old flat
// priority list of independently-opened archives - the first archive opened
// here becomes the base and every one after it is added as a patch, and if
// the base fails to open the chain is torn down so the next candidate can
// retry as a fresh base, reproducing "just skip archives that don't exist".
bool OpenLegacyMpqArchive(char const* filename)
{
    if (!WorldMpq)
    {
        WorldMpq = std::make_unique<mpqlib::MpqPatchChain>(filename);
        if (WorldMpq->isOpen())
            return true;

        WorldMpq.reset();
        return false;
    }

    return WorldMpq->addPatch(filename);
}

void LoadLegacyLocaleMPQFiles(int locale)
{
    char filename[512];

    sprintf(filename, "%s/Data/%s/locale-%s.MPQ", input_path, Locales[locale], Locales[locale]);
    OpenLegacyMpqArchive(filename);

    for (int i = 1; i < 5; ++i)
    {
        char ext[3] = "";
        if (i > 1)
            sprintf(ext, "-%i", i);

        sprintf(filename, "%s/Data/%s/patch-%s%s.MPQ", input_path, Locales[locale], Locales[locale], ext);
        if (FileExists(filename))
            OpenLegacyMpqArchive(filename);
    }
}

void LoadLegacyCommonMPQFiles()
{
    for (auto const& mpq : legacyMpqList)
    {
        if (mpq.versionMask & getLegacyVersionMask())
        {
            std::string fileName(std::string(input_path) + "/Data/" + mpq.fileName);
            if (FileExists(fileName))
                OpenLegacyMpqArchive(fileName.c_str());
        }
    }
}

int getFindLanguageIndex()
{
    int langIndex = -1;
    bool foundLanguage = false;

    std::string const path(std::string(input_path) + "/Data/");
    for (auto const* lang : Locales)
    {
        langIndex++;
        if (FileExists(path + lang + "/locale-" + lang + ".MPQ"))
        {
            foundLanguage = true;
            break;
        }
    }

    if (foundLanguage)
    {
        printf("Detected locale: %s\n", Locales[langIndex]);
        return langIndex;
    }

    printf("No locale found!\n");
    return -1;
}

// --- Cata+ MPQ loading: incremental wow-update patch chains ---

// Note: StormLib's SFileOpenPatchArchive() took a "path prefix" that remaps
// files stored under a locale subfolder inside a shared patch archive so they
// become addressable by their bare name. mpqlib has no equivalent - only
// relevant for archives at/below LAST_DBC_IN_DATA_BUILD (13623), a build from
// years before Cata/Mop ever shipped, so in practice every archive this tool
// actually opens today never needs it.
bool LoadModernLocaleMPQFile(int locale)
{
    char buff[512];

    if (gClientVersion != ClientVersion::MistsOfPandaria)
    {
        snprintf(buff, sizeof(buff), "%s/Data/%s/locale-%s.MPQ", input_path, Locales[locale], Locales[locale]);
        LocaleMpq = std::make_unique<mpqlib::MpqPatchChain>(buff);
        if (!LocaleMpq->isOpen())
        {
            LocaleMpq.reset();
            return false;
        }

        printf("\nLoading %s locale MPQs\n", Locales[locale]);
        for (uint32_t patchBuild : GetTargetBuildList())
        {
            if (patchBuild > CONF_TargetBuild)
                break;
            if (CONF_TargetBuild >= NEW_BASE_SET_BUILD && patchBuild < NEW_BASE_SET_BUILD)
                continue;

            if (patchBuild > LAST_DBC_IN_DATA_BUILD)
                snprintf(buff, sizeof(buff), "%s/Data/%s/wow-update-%s-%u.MPQ", input_path, Locales[locale], Locales[locale], patchBuild);
            else
                snprintf(buff, sizeof(buff), "%s/Data/wow-update-%u.MPQ", input_path, patchBuild);

            if (!LocaleMpq->addPatch(buff))
                continue;

            printf("Loaded %s\n", buff);
        }

        printf("\n");
        return true;
    }

    snprintf(buff, sizeof(buff), "%s/Data/misc.MPQ", input_path);
    LocaleMpq = std::make_unique<mpqlib::MpqPatchChain>(buff);
    if (!LocaleMpq->isOpen())
    {
        LocaleMpq.reset();
        return false;
    }

    snprintf(buff, sizeof(buff), "%s/Data/%s/locale-%s.MPQ", input_path, Locales[locale], Locales[locale]);
    if (!LocaleMpq->addPatch(buff))
    {
        LocaleMpq.reset();
        return false;
    }

    printf("\nLoading %s locale MPQs\n", Locales[locale]);

    for (uint32_t patchBuild : GetTargetBuildList())
    {
        if (patchBuild > CONF_TargetBuild)
            break;
        snprintf(buff, sizeof(buff), "%s/Data/wow-update-base-%u.MPQ", input_path, patchBuild);
        if (!LocaleMpq->addPatch(buff))
        {
            printf("Not found %s\n", buff);
            continue;
        }
        printf("Loaded %s\n", buff);
    }

    for (uint32_t patchBuild : GetTargetBuildList())
    {
        if (patchBuild > CONF_TargetBuild)
            break;
        snprintf(buff, sizeof(buff), "%s/Data/%s/wow-update-%s-%u.MPQ", input_path, Locales[locale], Locales[locale], patchBuild);
        if (!LocaleMpq->addPatch(buff))
            continue;
        printf("Loaded %s\n", buff);
    }

    for (uint32_t patchBuild : GetTargetBuildList())
    {
        if (patchBuild > CONF_TargetBuild)
            break;
        snprintf(buff, sizeof(buff), "%s/Data/Cache/patch-base-%u.MPQ", input_path, patchBuild);
        if (!LocaleMpq->addPatch(buff))
            continue;
        printf("Loaded %s\n", buff);
    }

    for (uint32_t patchBuild : GetTargetBuildList())
    {
        if (patchBuild > CONF_TargetBuild)
            break;
        snprintf(buff, sizeof(buff), "%s/Data/Cache/%s/patch-%s-%u.MPQ", input_path, Locales[locale], Locales[locale], patchBuild);
        if (!LocaleMpq->addPatch(buff))
            continue;
        printf("Loaded %s\n", buff);
    }

    printf("\n");
    return true;
}

void LoadModernCommonMPQFiles(uint32_t build)
{
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/Data/world.MPQ", input_path);
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
        if (build < NEW_BASE_SET_BUILD && mpqList[i] == "world2.MPQ")   // 4.3.2 and higher MPQ
            continue;

        snprintf(filename, sizeof(filename), "%s/Data/%s", input_path, mpqList[i].c_str());
        if (!WorldMpq->addPatch(filename))
            printf("Not found %s\n", filename);
        else
            printf("Loaded %s\n", filename);
    }

    for (uint32_t patchBuild : GetTargetBuildList())
    {
        if (patchBuild > CONF_TargetBuild)
            break;
        if (CONF_TargetBuild >= NEW_BASE_SET_BUILD && patchBuild < NEW_BASE_SET_BUILD)
            continue;

        if (patchBuild > LAST_DBC_IN_DATA_BUILD)
            snprintf(filename, sizeof(filename), "%s/Data/wow-update-base-%u.MPQ", input_path, patchBuild);
        else
            snprintf(filename, sizeof(filename), "%s/Data/wow-update-%u.MPQ", input_path, patchBuild);

        if (!WorldMpq->addPatch(filename))
        {
            printf("Not found %s\n", filename);
            continue;
        }
        printf("Loaded %s\n", filename);
    }

    printf("\n");
}

void CloseMPQFiles()
{
    WorldMpq.reset();
    LocaleMpq.reset();
}

void RunLegacyExtraction()
{
    int const langIndex = getFindLanguageIndex();
    if (gClientVersion != ClientVersion::Vanilla && langIndex < 0)
    {
        printf("Fatal Error: no langIndex found for this client\n");
        return;
    }

    uint32_t build = 5875;

    if (CONF_extract & EXTRACT_DBC)
    {
        LoadLegacyCommonMPQFiles();

        if (gClientVersion != ClientVersion::Vanilla)
        {
            LoadLegacyLocaleMPQFiles(langIndex);
            build = ReadBuild(langIndex);
            ExtractLegacyDBCFiles(langIndex, langIndex < 0);
            CloseMPQFiles();
        }
        else
            ExtractLegacyDBCFiles(langIndex, true);

        CloseMPQFiles();
    }

    if (CONF_extract & EXTRACT_MAP)
    {
        LoadLegacyCommonMPQFiles();
        if (gClientVersion != ClientVersion::Vanilla)
            LoadLegacyLocaleMPQFiles(langIndex);

        ExtractMapsFromMpq(build);
        CloseMPQFiles();
    }
}

void RunModernExtraction()
{
    int FirstLocale = -1;
    uint32_t build = 0;

    for (int i = 0; i < LOCALES_COUNT; ++i)
    {
        if (!LoadModernLocaleMPQFile(i))
            continue;

        printf("Detected locale: %s\n", Locales[i]);
        if ((CONF_extract & EXTRACT_DBC) == 0)
        {
            FirstLocale = i;
            build = ReadBuild(i);
            if (build > CONF_TargetBuild)
            {
                printf("Base locale-%s.MPQ has build higher than target build (%u > %u), nothing extracted!\n", Locales[i], build, CONF_TargetBuild);
                return;
            }

            printf("Detected client build: %u\n\n", build);
            break;
        }

        uint32_t tempBuild = ReadBuild(i);
        printf("Detected client build %u for locale %s\n", tempBuild, Locales[i]);
        if (tempBuild > CONF_TargetBuild)
        {
            LocaleMpq.reset();
            printf("Base locale-%s.MPQ has build higher than target build (%u > %u), nothing extracted!\n", Locales[i], tempBuild, CONF_TargetBuild);
            continue;
        }

        printf("\n");
        ExtractDBCFiles(i);
        ExtractDB2Files(i);

        if (FirstLocale < 0)
        {
            FirstLocale = i;
            build = tempBuild;
        }

        LocaleMpq.reset();
    }

    if (FirstLocale < 0)
    {
        printf("No locales detected\n");
        return;
    }

    if (CONF_extract & EXTRACT_CAMERA)
    {
        printf("Using locale: %s\n", Locales[FirstLocale]);
        LoadModernLocaleMPQFile(FirstLocale);
        LoadModernCommonMPQFiles(build);
        ExtractCameraFiles();
        CloseMPQFiles();
    }

    if (CONF_extract & EXTRACT_MAP)
    {
        printf("Using locale: %s\n", Locales[FirstLocale]);
        LoadModernLocaleMPQFile(FirstLocale);
        LoadModernCommonMPQFiles(build);
        ExtractMapsFromMpq(build);
        CloseMPQFiles();
    }
}

int main(int argc, char* arg[])
{
    printf("Map & DBC Extractor\n");
    printf("===================\n");

    std::string cwd = fs::current_path().string();
    strncpy(input_path, cwd.c_str(), MAX_PATH_LENGTH - 1);
    strncpy(output_path, cwd.c_str(), MAX_PATH_LENGTH - 1);

    auto detected = mpqlib::detectClientVersion(input_path);
    if (!detected)
    {
        printf("Fatal Error: No wow.exe found in %s!\n", input_path);
        std::cin.get();
        return 0;
    }

    gClientVersion = *detected;
    printf("Detected client version build family: %u\n", static_cast<uint32_t>(gClientVersion));

    if (!IsLegacyMapFormat())
        CONF_extract |= EXTRACT_CAMERA;

    CONF_use_minHeight = IsLegacyMapFormat() ? -500.0f : -2000.0f;
    CONF_TargetBuild = gClientVersion == ClientVersion::MistsOfPandaria ? 18273 : 15595;

    HandleArgs(argc, arg);

    if (IsLegacyMapFormat())
        RunLegacyExtraction();
    else
        RunModernExtraction();

    printf("Finished - Press any key to close map_extractor.exe\n");
    std::cin.get();
    return 0;
}
