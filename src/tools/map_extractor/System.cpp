/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "AEVersion.hpp"

#include <stdio.h>
#include <deque>
#include <set>
#include <cstdlib>

#ifdef _WIN32
#include "direct.h"
#else
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "dbcfile.h"
#include "mpq_libmpq04.h"

#include "adt.h"
#include "wdt.h"
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>

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
    #define OPEN_FLAGS  (O_RDONLY | O_BINARY | O_LARGEFILE)
#else
    #define OPEN_FLAGS (O_RDONLY | O_BINARY)
#endif
extern ArchiveSet gOpenArchives;

#include <filesystem>
namespace fs = std::filesystem;

std::string getWowExeName()
{
    const std::string path("./");
    const std::string extension(".exe");
    for (const auto& files : fs::recursive_directory_iterator(path))
    {
        if (files.path().extension() == extension)
        {
            auto name = files.path().stem().string().substr(0, 6);
            std::transform(name.begin(), name.end(), name.begin(), [](unsigned char chars) { return std::tolower(chars); });
            if (name + extension == "wow.exe")
                return files.path().stem().string() + extension;
        }
    }

    return "";
}

//\ thanks to mangos for this function, cheers - Zyres
uint32_t getBuildNumber()
{
    // buffers used for working on the file's bytes
    unsigned char byteSearchBuffer[1];      // used for reading in a single character, ready to be
                                            // tested for the required text we are searching for: 1, 5, 6, or 8
    unsigned char jumpBytesBuffer[128];     // used for skipping past the bytes from the file's start
                                            // to the base # area, before we start searching for the base #, for faster processing

    unsigned char preWOTLKbuildNumber[3];   // will hold the last 3 digits of the build number
    unsigned char postTBCbuildNumber[4];    // will hold the last 4 digits of the build number

    // These do not include the first digit
    // as the first digit is used to locate the possible location of the build number
    // then the following bytes are grabbed, 3 for pre WOTLK, 4 for WOTLK and later.
    // Those grabbed bytes are then compared with the below variables in order to idenity the exe's build
    unsigned char vanillaBuild1[3] = { 0x38, 0x37, 0x35 };      // (5)875
    unsigned char vanillaBuild2[3] = { 0x30, 0x30, 0x35 };      // (6)005
    unsigned char vanillaBuild3[3] = { 0x31, 0x34, 0x31 };      // (6)141
    unsigned char tbcBuild[3] = { 0x36, 0x30, 0x36 };           // (8)606
    unsigned char wotlkBuild[4] = { 0x32, 0x33, 0x34, 0x30 };   // (1)2340
    unsigned char cataBuild[4] = { 0x35, 0x35, 0x39, 0x35 };    // (1)5595
    unsigned char mopBuild[4] = { 0x38, 0x34, 0x31, 0x34 };     // (1)8414

    const auto wowExeFile = fopen(getWowExeName().c_str(), "rb");

    /// jump over as much of the file as possible, before we start searching for the base #
    for (auto i = 0; i < 3300; ++i)
        fread(jumpBytesBuffer, sizeof(jumpBytesBuffer), 1, wowExeFile);

    // Search for the build #
    while (fread(byteSearchBuffer, 1, 1, wowExeFile))
    {
        // we are looking for 1, 5, 6, or 8
        // these values are the first digit of the build versions we are interested in
        // Vanilla and TBC
        if (byteSearchBuffer[0] == 0x35 || byteSearchBuffer[0] == 0x36 || byteSearchBuffer[0] == 0x38)
        {
            // grab the next 4 bytes
            fread(preWOTLKbuildNumber, sizeof(preWOTLKbuildNumber), 1, wowExeFile);

            if (!memcmp(preWOTLKbuildNumber, vanillaBuild1, sizeof(preWOTLKbuildNumber))) // build is Vanilla?
                return 5875;

            if (!memcmp(preWOTLKbuildNumber, vanillaBuild2, sizeof(preWOTLKbuildNumber))) // build is Vanilla?
                return 6005;

            if (!memcmp(preWOTLKbuildNumber, vanillaBuild3, sizeof(preWOTLKbuildNumber))) // build is Vanilla?
                return 6141;

            if (!memcmp(preWOTLKbuildNumber, tbcBuild, sizeof(preWOTLKbuildNumber))) // build is TBC?
                return 8606;
        }

        // WOTLK, CATA, MoP
        if (byteSearchBuffer[0] == 0x31)
        {
            // grab the next 4 bytes
            fread(postTBCbuildNumber, sizeof(postTBCbuildNumber), 1, wowExeFile);

            if (!memcmp(postTBCbuildNumber, wotlkBuild, sizeof(postTBCbuildNumber))) /// build is WOTLK?
                return 12340;

            if (!memcmp(postTBCbuildNumber, cataBuild, sizeof(postTBCbuildNumber))) /// build is CATA?
                return 15595;

            if (!memcmp(postTBCbuildNumber, mopBuild, sizeof(postTBCbuildNumber))) /// build is MoP?
                return 18414;
        }
    }

    return 0;
}

struct map_id
{
    char name[64];
    uint32_t id;
};

map_id* map_ids;
uint16_t* areas;
uint16_t* LiqType;
#define MAX_PATH_LENGTH 128
char output_path[MAX_PATH_LENGTH] = ".";
char input_path[MAX_PATH_LENGTH] = ".";
uint32_t maxAreaId = 0;

uint32_t versionBuild = 0;

// **************************************************
// Extractor options
// **************************************************
enum Extract
{
    EXTRACT_MAP = 1,
    EXTRACT_DBC = 2
};

// Select data for extract
int   CONF_extract = EXTRACT_MAP | EXTRACT_DBC;
// This option allow limit minimum height to some value (Allow save some memory)
bool  CONF_allow_height_limit = true;
float CONF_use_minHeight = -500.0f;

// This option allow use float to int conversion
bool  CONF_allow_float_to_int   = true;
float CONF_float_to_int8_limit  = 2.0f;      // Max accuracy = val/256
float CONF_float_to_int16_limit = 2048.0f;   // Max accuracy = val/65536
float CONF_flat_height_delta_limit = 0.005f; // If max - min less this value - surface is flat
float CONF_flat_liquid_delta_limit = 0.001f; // If max - min less this value - liquid surface is flat

enum VersionMask : uint8_t
{
    MaskNone = 0x0,
    MaskClassic = 0x01,
    MaskBC = 0x02,
    MaskWotLK = 0x04,
    MaskCata = 0x08,
    MaskMop = 0x10,

    MaskBCWotLK = MaskBC | MaskWotLK,
    MaskClassicBCWotLK = MaskClassic | MaskBCWotLK,
    MaskCataMop = MaskCata | MaskMop
};

VersionMask getVersionMask()
{
    switch (versionBuild)
    {
        case 5875: return MaskClassic;
        case 8606: return MaskBC;
        case 12340: return MaskWotLK;
        case 15595: return MaskCata;
        case 18414: return MaskMop;
        default: return MaskNone;
    }
}

struct MpqList
{
    uint32_t versionMask;
    std::string fileName;
};

std::vector<MpqList> mpqList{
    {MaskClassic, "dbc.MPQ"},
    {MaskClassic, "terrain.MPQ"},

    {MaskClassicBCWotLK, "patch.MPQ"},
    {MaskClassicBCWotLK, "patch-2.MPQ"},

    {MaskBCWotLK, "common.MPQ"},
    {MaskWotLK, "common-2.MPQ"},
    {MaskWotLK, "lichking.MPQ"},
    {MaskBCWotLK, "expansion.MPQ"},
    {MaskWotLK, "patch-3.MPQ"},

    {MaskCataMop, "world.MPQ"},
    {MaskCata, "art.MPQ"},
    {MaskCata, "world2.MPQ"},
    {MaskMop, "misc.MPQ"},
    {MaskCataMop, "expansion1.MPQ"},
    {MaskCataMop, "expansion2.MPQ"},
    {MaskCataMop, "expansion3.MPQ"},
    {MaskMop, "expansion4.MPQ"},
};

static const char* const langs[] = {"enGB", "enUS", "deDE", "esES", "frFR", "koKR", "zhCN", "zhTW", "enCN", "enTW", "esMX", "ruRU", "ptBR", "ptPT", "itIT" };
#define LANG_COUNT 15

void CreateDir(const std::string& Path)
{
    if (fs::exists(Path))
    {
        std::cout << "NOTE: Directory " << Path << " already exists and gets now deleted" << std::endl;
        fs::remove_all(Path);
    }

    if (!fs::create_directory(Path))
    {
        std::cout << "Fatal Error: Could not create directory " << Path << " check your permissions" << std::endl;
        exit(1);
    }
}

bool FileExists(const std::string& FileName)
{
    return fs::exists(FileName);
}

void Usage(char* prg)
{
    printf(
        "Usage:\n"\
        "%s -[var] [value]\n"\
        "-i set input path (max %d characters)\n"\
        "-o set output path (max %d characters)\n"\
        "-e extract only MAP(1)/DBC(2) - standard: both(3)\n"\
        "-f height stored as int (less map size but lost some accuracy) 1 by default\n"\
        "Example: %s -f 0 -i \"c:\\games\\game\"", prg, MAX_PATH_LENGTH - 1, MAX_PATH_LENGTH - 1, prg);
    exit(1);
}

void HandleArgs(int argc, char* arg[])
{
    for (int c = 1; c < argc; ++c)
    {
        // i - input path
        // o - output path
        // e - extract only MAP(1)/DBC(2) - standard both(3)
        // f - use float to int conversion
        // h - limit minimum height
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
                    CONF_allow_float_to_int = atoi(arg[(c++) + 1]) != 0;
                else
                    Usage(arg[0]);
                break;
            case 'e':
                if (c + 1 < argc)                            // all ok
                {
                    CONF_extract = atoi(arg[(c++) + 1]);
                    if (!(CONF_extract > 0 && CONF_extract < 4))
                        Usage(arg[0]);
                }
                else
                    Usage(arg[0]);
                break;
        }
    }
}

uint32_t ReadBuild(int locale)
{
    // include build info file also if available

    std::string filename = std::string("component.wow-") + langs[locale] + ".txt";
    printf("Read %s file... ", filename.c_str());

    MPQFile m(filename.c_str());
    if (m.isEof())
    {
        printf("Fatal error: Not found %s file!\n", filename.c_str());
        exit(1);
    }

    std::string text = std::string(m.getPointer(), m.getSize());
    m.close();

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

    std::cout << "Detected file build: " << build << std::endl;

    return build;
}

uint32_t ReadMapDBC()
{
    printf("Read Map.dbc file... ");
    DBCFile dbc("DBFilesClient\\Map.dbc");

    if (!dbc.open())
    {
        printf("Fatal error: Invalid Map.dbc file format!\n");
        exit(1);
    }

    size_t map_count = dbc.getRecordCount();
    map_ids = new map_id[map_count];
    for (uint32_t x = 0; x < map_count; ++x)
    {
        map_ids[x].id = dbc.getRecord(x).getUInt(0);

        const char* map_name = dbc.getRecord(x).getString(1);
        size_t max_map_name_length = sizeof(map_ids[x].name);
        if (strlen(map_name) >= max_map_name_length)
        {
            printf("Fatal error: Map name too long!\n");
            exit(1);
        }

        strncpy(map_ids[x].name, map_name, max_map_name_length);
        map_ids[x].name[max_map_name_length - 1] = '\0';
    }
    printf("Done! (%u maps loaded)\n", (uint32_t)map_count);
    return static_cast<uint32_t>(map_count);
}

void ReadAreaTableDBC()
{
    printf("Read AreaTable.dbc file...");
    DBCFile dbc("DBFilesClient\\AreaTable.dbc");

    if (!dbc.open())
    {
        printf("Fatal error: Invalid AreaTable.dbc file format!\n");
        exit(1);
    }

    size_t area_count = dbc.getRecordCount();
    size_t maxid = dbc.getMaxId();
    areas = new uint16_t[maxid + 1];
    memset(areas, 0xff, (maxid + 1) * sizeof(uint16_t));

    for (uint32_t x = 0; x < area_count; ++x)
        areas[dbc.getRecord(x).getUInt(0)] = dbc.getRecord(x).getUInt(3);

    maxAreaId = static_cast<uint32_t>(dbc.getMaxId());

    printf("Done! (%u areas loaded)\n", (uint32_t)area_count);
}

void ReadLiquidTypeTableDBC()
{
    printf("Read LiquidType.dbc file...");
    DBCFile dbc("DBFilesClient\\LiquidType.dbc");
    if (!dbc.open())
    {
        printf("Fatal error: Invalid LiquidType.dbc file format!\n");
        exit(1);
    }

    size_t liqTypeCount = dbc.getRecordCount();
    size_t liqTypeMaxId = dbc.getMaxId();
    LiqType = new uint16_t[liqTypeMaxId + 1];
    memset(LiqType, 0xff, (liqTypeMaxId + 1) * sizeof(uint16_t));

    for (uint32_t x = 0; x < liqTypeCount; ++x)
        LiqType[dbc.getRecord(x).getUInt(0)] = dbc.getRecord(x).getUInt(3);

    printf("Done! (%u LiqTypes loaded)\n", (uint32_t)liqTypeCount);
}

//
// Adt file convertor function and data
//

// Map file format data
static char const* MAP_MAGIC         = "MAPS";
static char const* MAP_VERSION_MAGIC = "v1.3";
static char const* MAP_AREA_MAGIC    = "AREA";
static char const* MAP_HEIGHT_MAGIC  = "MHGT";
static char const* MAP_LIQUID_MAGIC  = "MLIQ";

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

#define MAP_HEIGHT_NO_HEIGHT  0x0001
#define MAP_HEIGHT_AS_INT16   0x0002
#define MAP_HEIGHT_AS_INT8    0x0004

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
#define MAP_LIQUID_TYPE_WMO_WATER   0x20


#define MAP_LIQUID_NO_TYPE    0x0001
#define MAP_LIQUID_NO_HEIGHT  0x0002

struct map_liquidHeader
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

float selectUInt8StepStore(float maxDiff)
{
    return 255 / maxDiff;
}

float selectUInt16StepStore(float maxDiff)
{
    return 65535 / maxDiff;
}
// Temporary grid data store
uint16_t area_flags[ADT_CELLS_PER_GRID][ADT_CELLS_PER_GRID];

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

bool ConvertADT(char* filename, char* filename2, int /*cell_y*/, int /*cell_x*/, uint32_t build)
{
    ADT_file adt;

    if (!adt.loadFile(filename))
        return false;

    adt_MCIN* cells = adt.a_grid->getMCIN();
    if (!cells)
    {
        printf("Can't find cells in '%s'\n", filename);
        return false;
    }

    memset(liquid_show, 0, sizeof(liquid_show));
    memset(liquid_flags, 0, sizeof(liquid_flags));
    memset(liquid_entry, 0, sizeof(liquid_entry));

    // Prepare map header
    map_fileheader map;
    map.mapMagic = *reinterpret_cast<uint32_t const*>(MAP_MAGIC);
    map.versionMagic = *reinterpret_cast<uint32_t const*>(MAP_VERSION_MAGIC);
    map.buildMagic = build;

    // Get area flags data
    for (int i = 0; i < ADT_CELLS_PER_GRID; i++)
    {
        for (int j = 0; j < ADT_CELLS_PER_GRID; j++)
        {
            adt_MCNK* cell = cells->getMCNK(i, j);
            uint32_t areaid = cell->areaid;
            if (areaid && areaid <= maxAreaId)
            {
                if (areas[areaid] != 0xffff)
                {
                    area_flags[i][j] = areas[areaid];
                    continue;
                }
                printf("File: %s\nCan't find area flag for areaid %u [%d, %d].\n", filename, areaid, cell->ix, cell->iy);
            }
            area_flags[i][j] = 0xffff;
        }
    }
    //============================================
    // Try pack area data
    //============================================
    bool fullAreaData = false;
    uint32_t areaflag = area_flags[0][0];
    for (int y = 0; y < ADT_CELLS_PER_GRID; y++)
    {
        for (int x = 0; x < ADT_CELLS_PER_GRID; x++)
        {
            if (area_flags[y][x] != areaflag)
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
        map.areaMapSize += sizeof(area_flags);
    }
    else
    {
        areaHeader.flags |= MAP_AREA_NO_AREA;
        areaHeader.gridArea = static_cast<uint16_t>(areaflag);
    }

    //
    // Get Height map from grid
    //
    for (int i = 0; i < ADT_CELLS_PER_GRID; i++)
    {
        for (int j = 0; j < ADT_CELLS_PER_GRID; j++)
        {
            adt_MCNK* cell = cells->getMCNK(i, j);
            if (!cell)
                continue;
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
                int cy = i * ADT_CELL_SIZE + y;
                for (int x = 0; x <= ADT_CELL_SIZE; x++)
                {
                    int cx = j * ADT_CELL_SIZE + x;
                    V9[cy][cx] = cell->ypos;
                }
            }
            for (int y = 0; y < ADT_CELL_SIZE; y++)
            {
                int cy = i * ADT_CELL_SIZE + y;
                for (int x = 0; x < ADT_CELL_SIZE; x++)
                {
                    int cx = j * ADT_CELL_SIZE + x;
                    V8[cy][cx] = cell->ypos;
                }
            }
            // Get custom height
            adt_MCVT* v = cell->getMCVT();
            if (!v)
                continue;
            // get V9 height map
            for (int y = 0; y <= ADT_CELL_SIZE; y++)
            {
                int cy = i * ADT_CELL_SIZE + y;
                for (int x = 0; x <= ADT_CELL_SIZE; x++)
                {
                    int cx = j * ADT_CELL_SIZE + x;
                    V9[cy][cx] += v->height_map[y * (ADT_CELL_SIZE * 2 + 1) + x];
                }
            }
            // get V8 height map
            for (int y = 0; y < ADT_CELL_SIZE; y++)
            {
                int cy = i * ADT_CELL_SIZE + y;
                for (int x = 0; x < ADT_CELL_SIZE; x++)
                {
                    int cx = j * ADT_CELL_SIZE + x;
                    V8[cy][cx] += v->height_map[y * (ADT_CELL_SIZE * 2 + 1) + ADT_CELL_SIZE + 1 + x];
                }
            }
        }
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

    // Get from MCLQ chunk (old)
    for (int i = 0; i < ADT_CELLS_PER_GRID; i++)
    {
        for (int j = 0; j < ADT_CELLS_PER_GRID; j++)
        {
            adt_MCNK* cell = cells->getMCNK(i, j);
            if (!cell)
                continue;

            adt_MCLQ* liquid = cell->getMCLQ();
            int count = 0;
            if (!liquid || cell->sizeMCLQ <= 8)
                continue;

            for (int y = 0; y < ADT_CELL_SIZE; y++)
            {
                int cy = i * ADT_CELL_SIZE + y;
                for (int x = 0; x < ADT_CELL_SIZE; x++)
                {
                    int cx = j * ADT_CELL_SIZE + x;
                    if (liquid->flags[y][x] != 0x0F)
                    {
                        liquid_show[cy][cx] = true;
                        if (liquid->flags[y][x] & (1 << 7))
                            liquid_flags[i][j] |= MAP_LIQUID_TYPE_DARK_WATER;
                        ++count;
                    }
                }
            }

            uint32_t c_flag = cell->flags;
            if (c_flag & (1 << 2))
            {
                liquid_entry[i][j] = 1;
                liquid_flags[i][j] |= MAP_LIQUID_TYPE_WATER;            // water
            }
            if (c_flag & (1 << 3))
            {
                liquid_entry[i][j] = 2;
                liquid_flags[i][j] |= MAP_LIQUID_TYPE_OCEAN;            // ocean
            }
            if (c_flag & (1 << 4))
            {
                liquid_entry[i][j] = 3;
                liquid_flags[i][j] |= MAP_LIQUID_TYPE_MAGMA;            // magma/slime
            }

            if (!count && liquid_flags[i][j])
                fprintf(stderr, "Wrong liquid detect in MCLQ chunk");

            for (int y = 0; y <= ADT_CELL_SIZE; y++)
            {
                int cy = i * ADT_CELL_SIZE + y;
                for (int x = 0; x <= ADT_CELL_SIZE; x++)
                {
                    int cx = j * ADT_CELL_SIZE + x;
                    liquid_height[cy][cx] = liquid->liquid[y][x].height;
                }
            }
        }
    }

    // Get liquid map for grid (in WOTLK used MH2O chunk)
    adt_MH2O* h2o = adt.a_grid->getMH2O();
    if (h2o)
    {
        for (int i = 0; i < ADT_CELLS_PER_GRID; i++)
        {
            for (int j = 0; j < ADT_CELLS_PER_GRID; j++)
            {
                adt_liquid_header* h = h2o->getLiquidData(i, j);
                if (!h)
                    continue;

                int count = 0;
                uint64_t show = h2o->getLiquidShowMap(h);
                for (int y = 0; y < h->height; y++)
                {
                    int cy = i * ADT_CELL_SIZE + y + h->yOffset;
                    for (int x = 0; x < h->width; x++)
                    {
                        int cx = j * ADT_CELL_SIZE + x + h->xOffset;
                        if (show & 1)
                        {
                            liquid_show[cy][cx] = true;
                            ++count;
                        }
                        show >>= 1;
                    }
                }

                liquid_entry[i][j] = h->liquidType;
                switch (LiqType[h->liquidType])
                {
                case LIQUID_TYPE_WATER: liquid_flags[i][j] |= MAP_LIQUID_TYPE_WATER; break;
                case LIQUID_TYPE_OCEAN: liquid_flags[i][j] |= MAP_LIQUID_TYPE_OCEAN; break;
                case LIQUID_TYPE_MAGMA: liquid_flags[i][j] |= MAP_LIQUID_TYPE_MAGMA; break;
                case LIQUID_TYPE_SLIME: liquid_flags[i][j] |= MAP_LIQUID_TYPE_SLIME; break;
                default:
                    printf("\nCan't find Liquid type %u for map %s\nchunk %d,%d\n", h->liquidType, filename, i, j);
                    break;
                }
                // Dark water detect
                if (LiqType[h->liquidType] == LIQUID_TYPE_OCEAN)
                {
                    uint8_t* lm = h2o->getLiquidLightMap(h);
                    if (!lm)
                        liquid_flags[i][j] |= MAP_LIQUID_TYPE_DARK_WATER;
                }

                if (!count && liquid_flags[i][j])
                    printf("Wrong liquid detect in MH2O chunk");

                float* height = h2o->getLiquidHeightMap(h);
                int pos = 0;
                for (int y = 0; y <= h->height; y++)
                {
                    int cy = i * ADT_CELL_SIZE + y + h->yOffset;
                    for (int x = 0; x <= h->width; x++)
                    {
                        int cx = j * ADT_CELL_SIZE + x + h->xOffset;
                        if (height)
                            liquid_height[cy][cx] = height[pos];
                        else
                            liquid_height[cy][cx] = h->heightLevel1;
                        pos++;
                    }
                }
            }
        }
    }
    //============================================
    // Pack liquid data
    //============================================
    uint8_t type = liquid_flags[0][0];
    bool fullType = false;
    for (int y = 0; y < ADT_CELLS_PER_GRID; y++)
    {
        for (int x = 0; x < ADT_CELLS_PER_GRID; x++)
        {
            if (liquid_flags[y][x] != type)
            {
                fullType = true;
                y = ADT_CELLS_PER_GRID;
                break;
            }
        }
    }

    map_liquidHeader liquidHeader;

    // no water data (if all grid have 0 liquid type)
    if (type == 0 && !fullType)
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
                    liquid_height[y][x] = CONF_use_minHeight;
            }
        }
        map.liquidMapOffset = map.heightMapOffset + map.heightMapSize;
        map.liquidMapSize = sizeof(map_liquidHeader);
        liquidHeader.fourcc = *reinterpret_cast<uint32_t const*>(MAP_LIQUID_MAGIC);
        liquidHeader.flags = 0;
        liquidHeader.liquidType = 0;
        liquidHeader.offsetX = minX;
        liquidHeader.offsetY = minY;
        liquidHeader.width = maxX - minX + 1 + 1;
        liquidHeader.height = maxY - minY + 1 + 1;
        liquidHeader.liquidLevel = minHeight;

        if (maxHeight == minHeight)
            liquidHeader.flags |= MAP_LIQUID_NO_HEIGHT;

        // Not need store if flat surface
        if (CONF_allow_float_to_int && (maxHeight - minHeight) < CONF_flat_liquid_delta_limit)
            liquidHeader.flags |= MAP_LIQUID_NO_HEIGHT;

        if (!fullType)
            liquidHeader.flags |= MAP_LIQUID_NO_TYPE;

        if (liquidHeader.flags & MAP_LIQUID_NO_TYPE)
            liquidHeader.liquidType = type;
        else
            map.liquidMapSize += sizeof(liquid_entry) + sizeof(liquid_flags);

        if (!(liquidHeader.flags & MAP_LIQUID_NO_HEIGHT))
            map.liquidMapSize += sizeof(float) * liquidHeader.width * liquidHeader.height;
    }

    // map hole info
    uint16_t holes[ADT_CELLS_PER_GRID][ADT_CELLS_PER_GRID];

    if (map.liquidMapOffset)
        map.holesOffset = map.liquidMapOffset + map.liquidMapSize;
    else
        map.holesOffset = map.heightMapOffset + map.heightMapSize;

    memset(holes, 0, sizeof(holes));
    bool hasHoles = false;

    for (int i = 0; i < ADT_CELLS_PER_GRID; ++i)
    {
        for (int j = 0; j < ADT_CELLS_PER_GRID; ++j)
        {
            adt_MCNK* cell = cells->getMCNK(i, j);
            if (!cell)
                continue;
            holes[i][j] = cell->holes;
            if (!hasHoles && cell->holes != 0)
                hasHoles = true;
        }
    }

    if (hasHoles)
        map.holesSize = sizeof(holes);
    else
        map.holesSize = 0;

    // Ok all data prepared - store it
    FILE* output = fopen(filename2, "wb");
    if (!output)
    {
        printf("Can't create the output file '%s'\n", filename2);
        return false;
    }
    fwrite(&map, sizeof(map), 1, output);
    // Store area data
    fwrite(&areaHeader, sizeof(areaHeader), 1, output);
    if (!(areaHeader.flags & MAP_AREA_NO_AREA))
        fwrite(area_flags, sizeof(area_flags), 1, output);

    // Store height data
    fwrite(&heightHeader, sizeof(heightHeader), 1, output);
    if (!(heightHeader.flags & MAP_HEIGHT_NO_HEIGHT))
    {
        if (heightHeader.flags & MAP_HEIGHT_AS_INT16)
        {
            fwrite(uint16_V9, sizeof(uint16_V9), 1, output);
            fwrite(uint16_V8, sizeof(uint16_V8), 1, output);
        }
        else if (heightHeader.flags & MAP_HEIGHT_AS_INT8)
        {
            fwrite(uint8_V9, sizeof(uint8_V9), 1, output);
            fwrite(uint8_V8, sizeof(uint8_V8), 1, output);
        }
        else
        {
            fwrite(V9, sizeof(V9), 1, output);
            fwrite(V8, sizeof(V8), 1, output);
        }
    }

    // Store liquid data if need
    if (map.liquidMapOffset)
    {
        fwrite(&liquidHeader, sizeof(liquidHeader), 1, output);
        if (!(liquidHeader.flags & MAP_LIQUID_NO_TYPE))
        {
            fwrite(liquid_entry, sizeof(liquid_entry), 1, output);
            fwrite(liquid_flags, sizeof(liquid_flags), 1, output);
        }
        if (!(liquidHeader.flags & MAP_LIQUID_NO_HEIGHT))
        {
            for (int y = 0; y < liquidHeader.height; y++)
                fwrite(&liquid_height[y + liquidHeader.offsetY][liquidHeader.offsetX], sizeof(float), liquidHeader.width, output);
        }
    }

    // store hole data
    if (hasHoles)
        fwrite(holes, map.holesSize, 1, output);

    fclose(output);

    return true;
}

void ExtractMapsFromMpq(uint32_t build)
{
    char mpq_filename[1024];
    char output_filename[1024];
    char mpq_map_name[1024];

    printf("Extracting maps...\n");

    uint32_t map_count = ReadMapDBC();

    ReadAreaTableDBC();
    ReadLiquidTypeTableDBC();

    std::string path = output_path;
    path += "/maps/";
    CreateDir(path);

    printf("Convert map files\n");
    for (uint32_t z = 0; z < map_count; ++z)
    {
        printf("Extract %s (%d/%u)                  \n", map_ids[z].name, z + 1, map_count);
        // Loadup map grid data
        sprintf(mpq_map_name, "World\\Maps\\%s\\%s.wdt", map_ids[z].name, map_ids[z].name);
        WDT_file wdt;
        if (!wdt.loadFile(mpq_map_name, false))
        {
            //            printf("Error loading %s map wdt data\n", map_ids[z].name);
            continue;
        }

        for (uint32_t y = 0; y < WDT_MAP_SIZE; ++y)
        {
            for (uint32_t x = 0; x < WDT_MAP_SIZE; ++x)
            {
                if (!wdt.main->adt_list[y][x].exist)
                    continue;
                sprintf(mpq_filename, "World\\Maps\\%s\\%s_%u_%u.adt", map_ids[z].name, map_ids[z].name, x, y);
                sprintf(output_filename, "%s/maps/%04u_%02u_%02u.map", output_path, map_ids[z].id, y, x);
                ConvertADT(mpq_filename, output_filename, y, x, build);
            }
            // draw progress bar
            printf("Processing........................%d%%\r", (100 * (y + 1)) / WDT_MAP_SIZE);
        }
    }
    printf("\n");
    delete[] areas;
    delete[] map_ids;
}

bool ExtractFile(char const* mpq_name, std::string const& filename)
{
    FILE* output = fopen(filename.c_str(), "wb");
    if (!output)
    {
        printf("Can't create the output file '%s'\n", filename.c_str());
        return false;
    }
    MPQFile m(mpq_name);
    if (!m.isEof())
        fwrite(m.getPointer(), 1, m.getSize(), output);

    fclose(output);
    return true;
}

void ExtractDBCFiles(int locale, bool basicLocale)
{
    printf("Extracting dbc files...\n");

    std::set<std::string> dbcfiles;

    // get DBC file list
    // this can be solved better with std::filesystem
    for (ArchiveSet::iterator i = gOpenArchives.begin(); i != gOpenArchives.end(); ++i)
    {
        std::vector<std::string> files;
        (*i)->GetFileListTo(files);
        for (std::vector<std::string>::iterator iter = files.begin(); iter != files.end(); ++iter)
        {
            if (iter->rfind(".dbc") == iter->length() - strlen(".dbc"))
                dbcfiles.insert(*iter);

            if (iter->rfind(".db2") == iter->length() - strlen(".db2"))
                dbcfiles.insert(*iter);
        }
    }

    std::string path = output_path;
    path += "/dbc/";
    CreateDir(path);
    if (!basicLocale)
    {
        path += langs[locale];
        path += "/";
        CreateDir(path);
    }

    // extract Build info file
    if (versionBuild > 5875)
    {
        std::string mpq_name = std::string("component.wow-") + langs[locale] + ".txt";
        std::string filename = path + mpq_name;

        ExtractFile(mpq_name.c_str(), filename);
    }

    // extract DBCs
    uint32_t count = 0;
    for (std::set<std::string>::iterator iter = dbcfiles.begin(); iter != dbcfiles.end(); ++iter)
    {
        std::string filename = path;
        filename += (iter->c_str() + strlen("DBFilesClient\\"));

        if (ExtractFile(iter->c_str(), filename))
            ++count;
    }
    printf("Extracted %u DBC files\n\n", count);
}

void LoadLocaleMPQFiles(int const locale)
{
    char filename[512];

    sprintf(filename, "%s/Data/%s/locale-%s.MPQ", input_path, langs[locale], langs[locale]);
    new MPQArchive(filename);

    for (int i = 1; i < 5; ++i)
    {
        char ext[3] = "";
        if (i > 1)
            sprintf(ext, "-%i", i);

        sprintf(filename, "%s/Data/%s/patch-%s%s.MPQ", input_path, langs[locale], langs[locale], ext);
        if (FileExists(filename))
            new MPQArchive(filename);
    }
}

void LoadCommonMPQFiles()
{
    for (auto mpq : mpqList)
    {
        if (mpq.versionMask & getVersionMask())
        {
            std::string fileName (std::string(input_path) + "/Data/" + mpq.fileName);
            if (FileExists(fileName))
                new MPQArchive(fileName.c_str());
        }
    }
}

inline void CloseMPQFiles()
{
    for (ArchiveSet::iterator j = gOpenArchives.begin(); j != gOpenArchives.end(); ++j)
        (*j)->close();
    gOpenArchives.clear();
}

int getFindLanguageIndex()
{
    int langIndex = -1;
    bool foundLanguage = false;

    const std::string path(std::string(input_path) + "/Data/");
    for (const auto* lang : langs)
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
        std::cout << "Detected locale: " << langs[langIndex] << std::endl;
        return langIndex;
    }

    std::cout << "No locale found!"<< std::endl;
    return -1;
}

int main(int argc, char * arg[])
{
    std::cout << "Map & DBC Extractor" << std::endl;
    std::cout << "===================" << std::endl;

    if (!getWowExeName().empty())
    {
        std::cout << "Found " << getWowExeName() << " file." << std::endl;
    }
    else
    {
        std::cout << "Fatal Error: No wow.exe found!" << std::endl;
        std::cin.get();
        return 0;
    }

    versionBuild = getBuildNumber();
    std::cout << "== WoWExe build Version: " << versionBuild << std::endl;

    HandleArgs(argc, arg);

    const auto langIndex = getFindLanguageIndex();
    if (versionBuild > 5875 && langIndex < 0)
    {
        std::cout << "Fatal Error: no langIndex found for client: " << versionBuild << std::endl;
        std::cin.get();
        return 0;
    }

    uint32_t build = 5875;

    if (CONF_extract & EXTRACT_DBC)
    {
        LoadCommonMPQFiles();

        // version > classic
        if (versionBuild > 5875)
        {
            LoadLocaleMPQFiles(langIndex);

            build = ReadBuild(langIndex);

            ExtractDBCFiles(langIndex, langIndex < 0 ? true : false);

            CloseMPQFiles();
        }
        else
        {
            ExtractDBCFiles(langIndex, true);
        }

        CloseMPQFiles();
    }

    if (CONF_extract & EXTRACT_MAP)
    {
        LoadCommonMPQFiles();

        // version > classic
        if (versionBuild > 5875)
            LoadLocaleMPQFiles(langIndex);

        ExtractMapsFromMpq(build);

        CloseMPQFiles();
    }

    std::cout << "Finished - Press any key to close map_extractor.exe" << std::endl;
    std::cin.get();
    return 0;
}
