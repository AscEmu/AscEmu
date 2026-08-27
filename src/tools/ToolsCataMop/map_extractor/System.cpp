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

#include <array>
#include <bitset>
#include <cstdio>
#include <optional>
#include <deque>
#include <fstream>
#include <set>
#include <unordered_map>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <algorithm>
#include <string_view>
#include <cctype>

#include "AEVersion.hpp"

#ifdef _WIN32
#include "direct.h"
#include <tchar.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#define ERROR_PATH_NOT_FOUND ERROR_FILE_NOT_FOUND
#endif

#include "mpqlib/MpqPatchChain.hpp"
#include "mpqlib/DBCFile.hpp"
#include "mpqlib/ChunkTree.hpp"

#include "adt.h"
#include "wdt.h"
#include <fcntl.h>

#if defined( __GNUC__ )
#define _open   open
#define _close close
#ifndef O_BINARY
#define O_BINARY 0
#endif
#else
#include <io.h>
#endif

#ifdef O_LARGEFILE
#define OPEN_FLAGS (O_RDONLY | O_BINARY | O_LARGEFILE)
#else
#define OPEN_FLAGS (O_RDONLY | O_BINARY)
#endif

#include <cinttypes>
#include <filesystem>
#include <G3D/Plane.h>

std::unique_ptr<mpqlib::MpqPatchChain> WorldMpq;
std::unique_ptr<mpqlib::MpqPatchChain> LocaleMpq;

namespace
{
    // ADT/WDT chunk tags this tool ever looks at - anything else is skipped
    // over while scanning. MCNK nests MCVT/MCLQ inside its own payload.
    constexpr std::array<std::string_view, 8> kRecognizedChunkTags =
        { "MVER", "MAIN", "MH2O", "MCNK", "MCVT", "MWMO", "MCLQ", "MFBO" };

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

    // Loads and validates an ADT/WDT chunk tree the same way the old
    // ChunkedFile::loadFile() did: silently on a missing file when log is
    // false, but always noisily on a present-but-invalid (wrong/missing
    // MVER) one.
    std::optional<mpqlib::ChunkTree> loadChunkTree(std::string_view filename, bool log = true)
    {
        auto tree = mpqlib::ChunkTree::load(*WorldMpq, filename, kRecognizedChunkTags);
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
}

typedef struct
{
    char name[64];
    uint32_t id;
} map_id;

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
std::unordered_map<uint32_t, LiquidMaterialEntry> LiquidMaterials;
std::unordered_map<uint32_t, LiquidObjectEntry> LiquidObjects;
std::unordered_map<uint32_t, LiquidTypeEntry> LiquidTypes;
#define MAX_PATH_LENGTH 128
char output_path[MAX_PATH_LENGTH];
char input_path[MAX_PATH_LENGTH];

// **************************************************
// Extractor options
// **************************************************
enum Extract
{
    EXTRACT_MAP = 1,
    EXTRACT_DBC = 2,
    EXTRACT_CAMERA = 4
};

// Select data for extract
int   CONF_extract = EXTRACT_MAP | EXTRACT_DBC | EXTRACT_CAMERA;

// This option allow limit minimum height to some value (Allow save some memory)
bool  CONF_allow_height_limit = true;
float CONF_use_minHeight = -2000.0f;

// This option allow use float to int conversion
bool  CONF_allow_float_to_int = true;
float CONF_float_to_int8_limit = 2.0f;      // Max accuracy = val/256
float CONF_float_to_int16_limit = 2048.0f;   // Max accuracy = val/65536
float CONF_flat_height_delta_limit = 0.005f; // If max - min less this value - surface is flat
float CONF_flat_liquid_delta_limit = 0.001f; // If max - min less this value - liquid surface is flat

#if VERSION_STRING == Cata
uint32_t CONF_TargetBuild = 15595;              // 4.3.4.15595

// List MPQ for extract maps from
char const* CONF_mpq_list[] =
{
    "world.MPQ",
    "art.MPQ",
    "world2.MPQ",
    "expansion1.MPQ",
    "expansion2.MPQ",
    "expansion3.MPQ",
};

uint32_t const Builds[] = { 13164, 13205, 13287, 13329, 13596, 13623, 13914, 14007, 14333, 14480, 14545, 15005, 15050, 15211, 15354, 15595, 0 };
#else
uint32_t CONF_TargetBuild = 18273;              // 5.4.8 18273 -- current build is 18414, but Blizz did not rename the MPQ files

// List MPQ for extract maps from
char const* CONF_mpq_list[] =
{
    "world.MPQ",
    "misc.MPQ",
    "expansion1.MPQ",
    "expansion2.MPQ",
    "expansion3.MPQ",
    "expansion4.MPQ"
};

uint32_t const Builds[] = { 16016, 16048, 16057, 16309, 16357, 16516, 16650, 16844, 16965, 17116, 17266, 17325, 17331, 17345, 17538, 17645, 17688, 17898, 18273 };
#endif
#define LAST_DBC_IN_DATA_BUILD 13623    // after this build mpqs with dbc are back to locale folder
#define NEW_BASE_SET_BUILD  15211

#define LOCALES_COUNT 15

char const* Locales[LOCALES_COUNT] =
{
    "enGB", "enUS",
    "deDE", "esES",
    "frFR", "koKR",
    "zhCN", "zhTW",
    "enCN", "enTW",
    "esMX", "ruRU",
    "ptBR", "ptPT",
    "itIT"
};

enum LocaleConstant : uint8_t
{
    LOCALE_enUS = 0,
    LOCALE_koKR = 1,
    LOCALE_frFR = 2,
    LOCALE_deDE = 3,
    LOCALE_zhCN = 4,
    LOCALE_zhTW = 5,
    LOCALE_esES = 6,
    LOCALE_esMX = 7,
    LOCALE_ruRU = 8,
    LOCALE_NONE = 9,
    LOCALE_ptBR = 10,
    LOCALE_itIT = 11,

    TOTAL_LOCALES
};

uint8_t const MpqToWowLocale[LOCALES_COUNT] =
{
    LOCALE_enUS,
    LOCALE_enUS,
    LOCALE_deDE,
    LOCALE_esES,
    LOCALE_frFR,
    LOCALE_koKR,
    LOCALE_zhCN,
    LOCALE_zhTW,
    LOCALE_zhCN,
    LOCALE_zhTW,
    LOCALE_esMX,
    LOCALE_ruRU,
    LOCALE_ptBR,
    LOCALE_ptBR,
    LOCALE_itIT
};

TCHAR const* LocalesT[LOCALES_COUNT] =
{
    _T("enGB"), _T("enUS"),
    _T("deDE"), _T("esES"),
    _T("frFR"), _T("koKR"),
    _T("zhCN"), _T("zhTW"),
    _T("enCN"), _T("enTW"),
    _T("esMX"), _T("ruRU"),
    _T("ptBR"), _T("ptPT"),
    _T("itIT"),
};

void CreateDir(std::string const& path)
{
    if (chdir(path.c_str()) == 0)
    {
        chdir("../");
        return;
    }

#ifdef _WIN32
    _mkdir(path.c_str());
#else
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO); // 0777
#endif
}

bool FileExists(TCHAR const* fileName)
{
    int fp = _open(fileName, OPEN_FLAGS);
    if (fp != -1)
    {
        _close(fp);
        return true;
    }

    return false;
}

void Usage(char const* prg)
{
    printf(
        "Usage:\n"\
        "%s -[var] [value]\n"\
        "-i set input path\n"\
        "-o set output path\n"\
        "-e extract only MAP(1)/DBC(2)/Camera(4) - standard: all(7)\n"\
        "-f height stored as int (less map size but lost some accuracy) 1 by default\n"\
        "-b target build (default %u)\n"\
        "Example: %s -f 0 -i \"c:\\games\\game\"", prg, CONF_TargetBuild, prg);
    exit(1);
}

void HandleArgs(int argc, char* arg[])
{
    for (int c = 1; c < argc; ++c)
    {
        // i - input path
        // o - output path
        // e - extract only MAP(1)/DBC(2)/Camera(4) - standard: all(7)
        // f - use float to int conversion
        // h - limit minimum height
        // b - target client build
        if (arg[c][0] != '-')
            Usage(arg[0]);

        switch (arg[c][1])
        {
        case 'i':
            if (c + 1 < argc && strlen(arg[c + 1]) < MAX_PATH_LENGTH) // all ok
            {
                strncpy(input_path, arg[c++ + 1], MAX_PATH_LENGTH);
                input_path[MAX_PATH_LENGTH - 1] = '\0';
            }
            else
                Usage(arg[0]);
            break;
        case 'o':
            if (c + 1 < argc && strlen(arg[c + 1]) < MAX_PATH_LENGTH) // all ok
            {
                strncpy(output_path, arg[c++ + 1], MAX_PATH_LENGTH);
                output_path[MAX_PATH_LENGTH - 1] = '\0';
            }
            else
                Usage(arg[0]);
            break;
        case 'f':
            if (c + 1 < argc)                            // all ok
                CONF_allow_float_to_int = atoi(arg[c++ + 1]) != 0;
            else
                Usage(arg[0]);
            break;
        case 'e':
            if (c + 1 < argc)                            // all ok
            {
                CONF_extract = atoi(arg[c++ + 1]);
                if (!(CONF_extract > 0 && CONF_extract < 8))
                    Usage(arg[0]);
            }
            else
                Usage(arg[0]);
            break;
        case 'b':
            if (c + 1 < argc)                            // all ok
                CONF_TargetBuild = atoi(arg[c++ + 1]);
            else
                Usage(arg[0]);
            break;
        default:
            break;
        }
    }
}

uint32_t ReadBuild(int locale)
{
    // include build info file also
    std::string filename = std::string("component.wow-") + Locales[locale] + ".txt";
    //printf("Read %s file... ", filename.c_str());

    std::vector<uint8_t> fileData;
    if (!LocaleMpq->readFile(filename, fileData) || fileData.empty())
    {
        printf("Fatal error: Not found %s file!\n", filename.c_str());
        exit(1);
    }

    const size_t textLen = std::min<size_t>(fileData.size(), 511);
    std::string text(reinterpret_cast<char const*>(fileData.data()), textLen);

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

    return build;
}

uint32_t ReadMapDBC()
{
    printf("Read Map.dbc file... ");

    DBCFile dbc(*LocaleMpq, "DBFilesClient\\Map.dbc");
    if (!dbc.open())
    {
        printf("Fatal error: Cannot find or parse Map.dbc in archive!\n");
        exit(1);
    }

    size_t map_count = dbc.getRowCount();
    map_ids.resize(map_count);
    for (uint32_t x = 0; x < map_count; ++x)
    {
        map_ids[x].id = dbc.getRow(x).getUInt(0);
        strcpy(map_ids[x].name, dbc.getRow(x).getString(1));
    }

    printf("Done! (%u maps loaded)\n", uint32_t(map_count));
    return static_cast<uint32_t>(map_count);
}

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

// Map file format data
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

struct map_liquidHeader
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

float selectUInt8StepStore(float maxDiff)
{
    return 255 / maxDiff;
}

float selectUInt16StepStore(float maxDiff)
{
    return 65535 / maxDiff;
}
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

bool ConvertADT(std::string const& inputPath, std::string const& outputPath, int /*cell_y*/, int /*cell_x*/, uint32_t build, bool ignoreDeepWater)
{
    auto adt = loadChunkTree(inputPath);
    if (!adt)
        return false;

    // Prepare map header
    map_fileheader map;
    map.mapMagic = *reinterpret_cast<uint32_t const*>(MAP_MAGIC);
    map.versionMagic = *(uint32_t const*)MAP_VERSION_MAGIC;
    map.buildMagic = build;

    // Get area flags data
    memset(area_ids, 0, sizeof(area_ids));
    memset(V9, 0, sizeof(V9));
    memset(V8, 0, sizeof(V8));

    memset(liquid_show, 0, sizeof(liquid_show));
    memset(liquid_flags, 0, sizeof(liquid_flags));
    memset(liquid_entry, 0, sizeof(liquid_entry));

    memset(holes, 0, sizeof(holes));

    bool hasHoles = false;
    bool hasFlightBox = false;

    for (mpqlib::ChunkNode const* mcnkNode : adt->findAll("MCNK"))
    {
        adt_MCNK const* mcnk = &mcnkNode->as<adt_MCNK>();

        if (mcnk->iy > ADT_CELLS_PER_GRID - 1 || mcnk->ix > ADT_CELLS_PER_GRID - 1)
        {
            printf("\n ADT_CELLS_PER_GRID %u max %u\n", mcnk->iy, ADT_CELLS_PER_GRID);
            continue;
        }

        // Area data
        area_ids[mcnk->iy][mcnk->ix] = static_cast<uint16_t>(mcnk->areaid);

        // Height
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

        // Set map height as grid height
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

        // Get custom height
        if (mpqlib::ChunkNode const* chunk = mcnkNode->find("MCVT"))
        {
            adt_MCVT const* mcvt = &chunk->as<adt_MCVT>();
            // get V9 height map
            for (int y = 0; y <= ADT_CELL_SIZE; y++)
            {
                int cy = mcnk->iy * ADT_CELL_SIZE + y;
                for (int x = 0; x <= ADT_CELL_SIZE; x++)
                {
                    int cx = mcnk->ix * ADT_CELL_SIZE + x;
                    V9[cy][cx] += mcvt->height_map[y * (ADT_CELL_SIZE * 2 + 1) + x];
                }
            }
            // get V8 height map
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

        // Liquid data
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
                    liquid_flags[mcnk->iy][mcnk->ix] |= MAP_LIQUID_TYPE_WATER;            // water
                }
                if (c_flag & (1 << 3))
                {
                    liquid_entry[mcnk->iy][mcnk->ix] = 2;
                    liquid_flags[mcnk->iy][mcnk->ix] |= MAP_LIQUID_TYPE_OCEAN;            // ocean
                }
                if (c_flag & (1 << 4))
                {
                    liquid_entry[mcnk->iy][mcnk->ix] = 3;
                    liquid_flags[mcnk->iy][mcnk->ix] |= MAP_LIQUID_TYPE_MAGMA;            // magma/slime
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

    // Get liquid map for grid (in WOTLK used MH2O chunk)
    if (mpqlib::ChunkNode const* chunk = adt->find("MH2O"))
    {
        adt_MH2O const* h2o = &chunk->as<adt_MH2O>();
        for (int32_t i = 0; i < ADT_CELLS_PER_GRID; i++)
        {
            for (int32_t j = 0; j < ADT_CELLS_PER_GRID; j++)
            {
                adt_liquid_instance const* h = h2o->GetLiquidInstance(i, j);
                if (!h)
                    continue;

                adt_liquid_attributes attrs = h2o->GetLiquidAttributes(i, j);

                int32_t count = 0;
                uint64_t existsMask = h2o->GetLiquidExistsBitmap(h);
                for (int32_t y = 0; y < h->GetHeight(); y++)
                {
                    int32_t cy = i * ADT_CELL_SIZE + y + h->GetOffsetY();
                    for (int32_t x = 0; x < h->GetWidth(); x++)
                    {
                        int32_t cx = j * ADT_CELL_SIZE + x + h->GetOffsetX();
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

                int32_t pos = 0;
                for (int32_t y = 0; y <= h->GetHeight(); y++)
                {
                    int32_t cy = i * ADT_CELL_SIZE + y + h->GetOffsetY();
                    for (int32_t x = 0; x <= h->GetWidth(); x++)
                    {
                        int32_t cx = j * ADT_CELL_SIZE + x + h->GetOffsetX();
                        liquid_height[cy][cx] = h2o->GetLiquidHeight(h, pos);
                        pos++;
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

    // Check for allow limit minimum height (not store height in deep ochean - allow save some memory)
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

    // Not need store if flat surface
    if (CONF_allow_float_to_int && (maxHeight - minHeight) < CONF_flat_height_delta_limit)
        heightHeader.flags |= MAP_HEIGHT_NO_HEIGHT;

    if (hasFlightBox)
    {
        heightHeader.flags |= MAP_HEIGHT_HAS_FLIGHT_BOUNDS;
        map.heightMapSize += sizeof(flight_box_max) + sizeof(flight_box_min);
    }

    // Try store as packed in uint16_t or uint8_t values
    if (!(heightHeader.flags & MAP_HEIGHT_NO_HEIGHT))
    {
        float step = 0;
        // Try Store as uint values
        if (CONF_allow_float_to_int)
        {
            float diff = maxHeight - minHeight;
            if (diff < CONF_float_to_int8_limit)      // As uint8_t (max accuracy = CONF_float_to_int8_limit/256)
            {
                heightHeader.flags |= MAP_HEIGHT_AS_INT8;
                step = selectUInt8StepStore(diff);
            }
            else if (diff < CONF_float_to_int16_limit)  // As uint16_t (max accuracy = CONF_float_to_int16_limit/65536)
            {
                heightHeader.flags |= MAP_HEIGHT_AS_INT16;
                step = selectUInt16StepStore(diff);
            }
        }

        // Pack it to int values if need
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

    map_liquidHeader liquidHeader{};

    // no water data (if all grid have 0 liquid type)
    if (firstLiquidFlag == 0 && !fullType)
    {
        // No liquid data
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
                    if (minHeight > CONF_use_minHeight) minHeight = CONF_use_minHeight;
                }
            }
        }
        map.liquidMapOffset = map.heightMapOffset + map.heightMapSize;
        map.liquidMapSize = sizeof(map_liquidHeader);
        liquidHeader.fourcc = *reinterpret_cast<uint32_t const*>(MAP_LIQUID_MAGIC);
        liquidHeader.flags = 0;
        liquidHeader.liquidType = 0;
        liquidHeader.offsetX = static_cast<uint8_t>(minX);
        liquidHeader.offsetY = static_cast<uint8_t>(minY);
        liquidHeader.width = static_cast<uint8_t>(maxX - minX + 1 + 1);
        liquidHeader.height = static_cast<uint8_t>(maxY - minY + 1 + 1);
        liquidHeader.liquidLevel = minHeight;

        if (maxHeight == minHeight)
            liquidHeader.flags |= MAP_LIQUID_NO_HEIGHT;

        // Not need store if flat surface
        if (CONF_allow_float_to_int && (maxHeight - minHeight) < CONF_flat_liquid_delta_limit)
            liquidHeader.flags |= MAP_LIQUID_NO_HEIGHT;

        if (!fullType)
            liquidHeader.flags |= MAP_LIQUID_NO_TYPE;

        if (liquidHeader.flags & MAP_LIQUID_NO_TYPE)
        {
            liquidHeader.liquidFlags = firstLiquidFlag;
            liquidHeader.liquidType = firstLiquidType;
        }
        else
            map.liquidMapSize += sizeof(liquid_entry) + sizeof(liquid_flags);

        if (!(liquidHeader.flags & MAP_LIQUID_NO_HEIGHT))
            map.liquidMapSize += sizeof(float) * liquidHeader.width * liquidHeader.height;
    }

    if (hasHoles)
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

    // Ok all data prepared - store it
    std::ofstream outFile(outputPath, std::ofstream::out | std::ofstream::binary);
    if (!outFile)
    {
        printf("Can't create the output file '%s'\n", outputPath.c_str());
        return false;
    }

    outFile.write(reinterpret_cast<const char*>(&map), sizeof(map));
    // Store area data
    outFile.write(reinterpret_cast<const char*>(&areaHeader), sizeof(areaHeader));
    if (!(areaHeader.flags & MAP_AREA_NO_AREA))
        outFile.write(reinterpret_cast<const char*>(area_ids), sizeof(area_ids));

    // Store height data
    outFile.write(reinterpret_cast<const char*>(&heightHeader), sizeof(heightHeader));
    if (!(heightHeader.flags & MAP_HEIGHT_NO_HEIGHT))
    {
        if (heightHeader.flags & MAP_HEIGHT_AS_INT16)
        {
            outFile.write(reinterpret_cast<const char*>(uint16_V9), sizeof(uint16_V9));
            outFile.write(reinterpret_cast<const char*>(uint16_V8), sizeof(uint16_V8));
        }
        else if (heightHeader.flags & MAP_HEIGHT_AS_INT8)
        {
            outFile.write(reinterpret_cast<const char*>(uint8_V9), sizeof(uint8_V9));
            outFile.write(reinterpret_cast<const char*>(uint8_V8), sizeof(uint8_V8));
        }
        else
        {
            outFile.write(reinterpret_cast<const char*>(V9), sizeof(V9));
            outFile.write(reinterpret_cast<const char*>(V8), sizeof(V8));
        }
    }

    if (heightHeader.flags & MAP_HEIGHT_HAS_FLIGHT_BOUNDS)
    {
        outFile.write(reinterpret_cast<char*>(flight_box_max), sizeof(flight_box_max));
        outFile.write(reinterpret_cast<char*>(flight_box_min), sizeof(flight_box_min));
    }

    // Store liquid data if need
    if (map.liquidMapOffset)
    {
        outFile.write(reinterpret_cast<const char*>(&liquidHeader), sizeof(liquidHeader));
        if (!(liquidHeader.flags & MAP_LIQUID_NO_TYPE))
        {
            outFile.write(reinterpret_cast<const char*>(liquid_entry), sizeof(liquid_entry));
            outFile.write(reinterpret_cast<const char*>(liquid_flags), sizeof(liquid_flags));
        }

        if (!(liquidHeader.flags & MAP_LIQUID_NO_HEIGHT))
        {
            for (int y = 0; y < liquidHeader.height; y++)
                outFile.write(reinterpret_cast<const char*>(&liquid_height[y + liquidHeader.offsetY][liquidHeader.offsetX]), sizeof(float) * liquidHeader.width);
        }
    }

    // store hole data
    if (hasHoles)
        outFile.write(reinterpret_cast<const char*>(holes), map.holesSize);

    outFile.close();

    return true;
}

bool IsDeepWaterIgnored(uint32_t mapId, uint32_t x, uint32_t y)
{
    if (mapId == 0)
    {
        //                                                                                                GRID(39, 24) || GRID(39, 25) || GRID(39, 26) ||
        //                                                                                                GRID(40, 24) || GRID(40, 25) || GRID(40, 26) ||
        //GRID(41, 18) || GRID(41, 19) || GRID(41, 20) || GRID(41, 21) || GRID(41, 22) || GRID(41, 23) || GRID(41, 24) || GRID(41, 25) || GRID(41, 26) ||
        //GRID(42, 18) || GRID(42, 19) || GRID(42, 20) || GRID(42, 21) || GRID(42, 22) || GRID(42, 23) || GRID(42, 24) || GRID(42, 25) || GRID(42, 26) ||
        //GRID(43, 18) || GRID(43, 19) || GRID(43, 20) || GRID(43, 21) || GRID(43, 22) || GRID(43, 23) || GRID(43, 24) || GRID(43, 25) || GRID(43, 26) ||
        //GRID(44, 18) || GRID(44, 19) || GRID(44, 20) || GRID(44, 21) || GRID(44, 22) || GRID(44, 23) || GRID(44, 24) || GRID(44, 25) || GRID(44, 26) ||
        //GRID(45, 18) || GRID(45, 19) || GRID(45, 20) || GRID(45, 21) || GRID(45, 22) || GRID(45, 23) || GRID(45, 24) || GRID(45, 25) || GRID(45, 26) ||
        //GRID(46, 18) || GRID(46, 19) || GRID(46, 20) || GRID(46, 21) || GRID(46, 22) || GRID(46, 23) || GRID(46, 24) || GRID(46, 25) || GRID(46, 26)

        // Vashj'ir grids completely ignore fatigue
        return (x >= 39 && x <= 40 && y >= 24 && y <= 26) || (x >= 41 && x <= 46 && y >= 18 && y <= 26);
    }

    if (mapId == 1)
    {
        // GRID(43, 39) || GRID(43, 40)
        // Thousand Needles
        return x == 43 && (y == 39 || y == 40);
    }

    return false;
}

void ExtractMapsFromMpq(uint32_t build)
{
    char mpq_filename[1024];
    char output_filename[1024];
    char mpq_map_name[1024];

    printf("Extracting maps...\n");

    uint32_t map_count = ReadMapDBC();

    ReadLiquidMaterialTable();
    ReadLiquidObjectTable();
    ReadLiquidTypeTable();

    std::string path = output_path;
    path += "/maps/";
    CreateDir(path);

    printf("Convert map files\n");
    for (uint32_t z = 0; z < map_count; ++z)
    {
        printf("Extract %s (%d/%u)                  \n", map_ids[z].name, z + 1, map_count);
        // Loadup map grid data
        sprintf(mpq_map_name, "World\\Maps\\%s\\%s.wdt", map_ids[z].name, map_ids[z].name);
        std::bitset<(WDT_MAP_SIZE)* (WDT_MAP_SIZE)> existingTiles;
        if (auto wdt = loadChunkTree(mpq_map_name, false))
        {
            mpqlib::ChunkNode const* main = wdt->find("MAIN");
            for (uint32_t y = 0; y < WDT_MAP_SIZE; ++y)
            {
                for (uint32_t x = 0; x < WDT_MAP_SIZE; ++x)
                {
                    if (!(main->as<wdt_MAIN>().adt_list[y][x].flag & 0x1))
                        continue;

                    sprintf(mpq_filename, "World\\Maps\\%s\\%s_%u_%u.adt", map_ids[z].name, map_ids[z].name, x, y);
                    sprintf(output_filename, "%s/maps/%04u_%02u_%02u.map", output_path, map_ids[z].id, y, x);
                    bool ignoreDeepWater = IsDeepWaterIgnored(map_ids[z].id, y, x);
                    existingTiles[y * WDT_MAP_SIZE + x] = ConvertADT(mpq_filename, output_filename, y, x, build, ignoreDeepWater);
                }

                // draw progress bar
                printf("Processing........................%d%%\r", (100 * (y + 1)) / WDT_MAP_SIZE);
            }
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

// Minimal single-'*' wildcard matcher, replacing StormLib's SFileFindFirstFile/
// SFileFindNextFile - the only two patterns this tool ever searches for
// ("DBFilesClient\\*dbc" / "DBFilesClient\\*db2") are simple prefix+suffix matches.
static bool CaseInsensitiveEquals(std::string_view a, std::string_view b)
{
    if (a.size() != b.size())
        return false;

    return std::equal(a.begin(), a.end(), b.begin(), [](char x, char y)
    {
        return std::tolower(static_cast<unsigned char>(x)) == std::tolower(static_cast<unsigned char>(y));
    });
}

static bool MatchesSimplePattern(std::string_view name, std::string_view pattern)
{
    const size_t star = pattern.find('*');
    if (star == std::string_view::npos)
        return CaseInsensitiveEquals(name, pattern);

    const std::string_view prefix = pattern.substr(0, star);
    const std::string_view suffix = pattern.substr(star + 1);

    if (name.size() < prefix.size() + suffix.size())
        return false;

    return CaseInsensitiveEquals(name.substr(0, prefix.size()), prefix) &&
        CaseInsensitiveEquals(name.substr(name.size() - suffix.size()), suffix);
}

char const* localeNames[TOTAL_LOCALES] =
{
    "enUS",
    "koKR",
    "frFR",
    "deDE",
    "zhCN",
    "zhTW",
    "esES",
    "esMX",
    "ruRU",
    "none",
    "ptBR",
    "itIT"
};

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

        if (FileExists(filename.c_str()))
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

    // get camera file list from DBC
    std::vector<std::string> camerafiles;
    size_t cam_count = camdbc.getRowCount();

    for (size_t i = 0; i < cam_count; ++i)
    {
        std::string camFile(camdbc.getRow(i).getString(1));
        size_t loc = camFile.find(".mdx");
        if (loc != std::string::npos)
            camFile.replace(loc, 4, ".m2");
        camerafiles.push_back(std::string(camFile));
    }

    std::string path = output_path;
    path += "/Cameras/";
    CreateDir(path);

    // extract M2s
    uint32_t count = 0;
    for (std::string thisFile : camerafiles)
    {
        std::string filename = path;
        filename += (thisFile.c_str() + strlen("Cameras\\"));

        if (FileExists(filename.c_str()))
            continue;

        if (ExtractFile(*WorldMpq, thisFile.c_str(), filename.c_str()))
            ++count;
    }
    printf("Extracted %u camera files\n", count);
}

// Note: StormLib's SFileOpenPatchArchive() took a "path prefix" that remaps
// files stored under a locale subfolder inside a shared patch archive so they
// become addressable by their bare name. mpqlib has no equivalent - only
// relevant for archives at/below LAST_DBC_IN_DATA_BUILD (13623), a build from
// years before Cata/Mop ever shipped, so in practice every archive this tool
// actually opens today never needs it.
bool LoadLocaleMPQFile(int locale)
{
#if VERSION_STRING < Mop
    char buff[512];
    snprintf(buff, sizeof(buff), "%s/Data/%s/locale-%s.MPQ", input_path, LocalesT[locale], LocalesT[locale]);
    LocaleMpq = std::make_unique<mpqlib::MpqPatchChain>(buff);
    if (!LocaleMpq->isOpen())
    {
        LocaleMpq.reset();
        return false;
    }

    printf("\nLoading %s locale MPQs\n", LocalesT[locale]);
    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        // Do not attempt to read older MPQ patch archives past this build, they were merged with base
        // and trying to read them together with new base will not end well
        if (CONF_TargetBuild >= NEW_BASE_SET_BUILD && Builds[i] < NEW_BASE_SET_BUILD)
            continue;

        if (Builds[i] > LAST_DBC_IN_DATA_BUILD)
            snprintf(buff, sizeof(buff), "%s/Data/%s/wow-update-%s-%u.MPQ", input_path, LocalesT[locale], LocalesT[locale], Builds[i]);
        else
            snprintf(buff, sizeof(buff), "%s/Data/wow-update-%u.MPQ", input_path, Builds[i]);

        if (!LocaleMpq->addPatch(buff))
            continue;

        printf("Loaded %s\n", buff);
    }

    printf("\n");
    return true;
#else
    char buff[512];

    snprintf(buff, sizeof(buff), "%s/Data/misc.MPQ", input_path);
    LocaleMpq = std::make_unique<mpqlib::MpqPatchChain>(buff);
    if (!LocaleMpq->isOpen())
    {
        LocaleMpq.reset();
        return false;
    }

    snprintf(buff, sizeof(buff), "%s/Data/%s/locale-%s.MPQ", input_path, LocalesT[locale], LocalesT[locale]);
    if (!LocaleMpq->addPatch(buff))
    {
        LocaleMpq.reset();
        return false;
    }

    printf("\nLoading %s locale MPQs\n", LocalesT[locale]);

    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        snprintf(buff, sizeof(buff), "%s/Data/wow-update-base-%u.MPQ", input_path, Builds[i]);

        if (!LocaleMpq->addPatch(buff))
        {
            printf("Not found %s\n", buff);
            continue;
        }

        printf("Loaded %s\n", buff);
    }

    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        snprintf(buff, sizeof(buff), "%s/Data/%s/wow-update-%s-%u.MPQ", input_path, LocalesT[locale], LocalesT[locale], Builds[i]);

        if (!LocaleMpq->addPatch(buff))
            continue;

        printf("Loaded %s\n", buff);
    }

    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        snprintf(buff, sizeof(buff), "%s/Data/Cache/patch-base-%u.MPQ", input_path, Builds[i]);

        if (!LocaleMpq->addPatch(buff))
            continue;

        printf("Loaded %s\n", buff);
    }

    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        // Load cached locales
        snprintf(buff, sizeof(buff), "%s/Data/Cache/%s/patch-%s-%u.MPQ", input_path, LocalesT[locale], LocalesT[locale], Builds[i]);

        if (!LocaleMpq->addPatch(buff))
            continue;

        printf("Loaded %s\n", buff);
    }

    printf("\n");
    return true;
#endif
}

void LoadCommonMPQFiles(uint32_t build)
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

    int count = sizeof(CONF_mpq_list) / sizeof(char*);
    for (int i = 1; i < count; ++i)
    {
        if (build < NEW_BASE_SET_BUILD && !strcmp("world2.MPQ", CONF_mpq_list[i]))   // 4.3.2 and higher MPQ
            continue;

        snprintf(filename, sizeof(filename), "%s/Data/%s", input_path, CONF_mpq_list[i]);
        if (!WorldMpq->addPatch(filename))
            printf("Not found %s\n", filename);
        else
            printf("Loaded %s\n", filename);
    }

    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        // Do not attempt to read older MPQ patch archives past this build, they were merged with base
        // and trying to read them together with new base will not end well
        if (CONF_TargetBuild >= NEW_BASE_SET_BUILD && Builds[i] < NEW_BASE_SET_BUILD)
            continue;

        if (Builds[i] > LAST_DBC_IN_DATA_BUILD)
            snprintf(filename, sizeof(filename), "%s/Data/wow-update-base-%u.MPQ", input_path, Builds[i]);
        else
            snprintf(filename, sizeof(filename), "%s/Data/wow-update-%u.MPQ", input_path, Builds[i]);

        if (!WorldMpq->addPatch(filename))
        {
            printf("Not found %s\n", filename);
            continue;
        }

        printf("Loaded %s\n", filename);
    }

    printf("\n");
}

int main(int argc, char* arg[])
{
    printf("Map & DBC Extractor\n");
    printf("===================\n");

    std::filesystem::path current(std::filesystem::current_path());
    strcpy(input_path, current.string().c_str());
    strcpy(output_path, current.string().c_str());

    HandleArgs(argc, arg);

    int FirstLocale = -1;
    uint32_t build = 0;

    for (int i = 0; i < LOCALES_COUNT; ++i)
    {
        //Open MPQs
        if (!LoadLocaleMPQFile(i))
            continue;

        printf("Detected locale: %s\n", Locales[i]);
        if ((CONF_extract & EXTRACT_DBC) == 0)
        {
            FirstLocale = i;
            build = ReadBuild(i);
            if (build > CONF_TargetBuild)
            {
                printf("Base locale-%s.MPQ has build higher than target build (%u > %u), nothing extracted!\n", Locales[i], build, CONF_TargetBuild);
                return 0;
            }

            printf("Detected client build: %u\n", build);
            printf("\n");
            break;
        }

        //Extract DBC files
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

        //Close MPQs
        LocaleMpq.reset();
    }

    if (FirstLocale < 0)
    {
        printf("No locales detected\n");
        return 0;
    }

    if (CONF_extract & EXTRACT_CAMERA)
    {
        printf("Using locale: %s\n", Locales[FirstLocale]);

        // Open MPQs
        LoadLocaleMPQFile(FirstLocale);
        LoadCommonMPQFiles(build);

        // Extract cameras
        ExtractCameraFiles();

        // Close MPQs
        WorldMpq.reset();
        LocaleMpq.reset();
    }

    if (CONF_extract & EXTRACT_MAP)
    {
        printf("Using locale: %s\n", Locales[FirstLocale]);

        // Open MPQs
        LoadLocaleMPQFile(FirstLocale);
        LoadCommonMPQFiles(build);

        // Extract maps
        ExtractMapsFromMpq(build);

        // Close MPQs
        WorldMpq.reset();
        LocaleMpq.reset();
    }

    return 0;
}
