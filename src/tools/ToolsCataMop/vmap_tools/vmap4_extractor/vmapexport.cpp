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
#include "AEVersion.hpp"
#include <cstdio>
#include <iostream>
#include <vector>
#include <list>
#include <errno.h>
#include <memory>
#include <algorithm>
#include <string_view>
#include <cctype>

#ifdef WIN32
    #include <Windows.h>
    #include <tchar.h>
    #include <sys/stat.h>
    #include <direct.h>
    #define mkdir _mkdir
#else
    #include <sys/stat.h>
    #define ERROR_PATH_NOT_FOUND ERROR_FILE_NOT_FOUND
#endif

#undef min
#undef max

//#pragma warning(disable : 4505)
//#pragma comment(lib, "Winmm.lib")

#include <map>

//From Extractor
#include "ADTFile.hpp"
#include "WDTFile.hpp"
#include "mpqlib/DBCFile.hpp"
#include "wmo.h"
#include "mpqlib/MPQFile.hpp"

#include "vmapexport.h"

//------------------------------------------------------------------------------
// Defines

#define MPQ_BLOCK_SIZE 0x1000

//-----------------------------------------------------------------------------

std::unique_ptr<mpqlib::MpqPatchChain> WorldMpq;
std::unique_ptr<mpqlib::MpqPatchChain> LocaleMpq;

#if VERSION_STRING == Cata
uint32_t CONF_TargetBuild = 15595;              // 4.3.4.15595

// List MPQ for extract maps from
char const* CONF_mpq_list[]=
{
    "world.MPQ",
    "art.MPQ",
    "expansion1.MPQ",
    "expansion2.MPQ",
    "expansion3.MPQ",
    "world2.MPQ",
};

uint32_t const Builds[] = {13164, 13205, 13287, 13329, 13596, 13623, 13914, 14007, 14333, 14480, 14545, 15005, 15050, 15211, 15354, 15595, 0};
#define LAST_DBC_IN_DATA_BUILD 13623    // after this build mpqs with dbc are back to locale folder
#define NEW_BASE_SET_BUILD  15211
#else
uint32_t CONF_TargetBuild = 18273;              // 5.4.8.18273

// List MPQ for extract maps from
char const* CONF_mpq_list[] =
{
    "world.MPQ",
    "model.MPQ", // added in 5.x.x
    "misc.MPQ", // added in 5.x.x
    "expansion1.MPQ",
    "expansion2.MPQ",
    "expansion3.MPQ",
    "expansion4.MPQ", // added in 5.x.x
};

uint32_t const Builds[] = { 16016, 16048, 16057, 16309, 16357, 16516, 16650, 16844, 16965, 17116, 17266, 17325, 17331, 17345, 17538, 17645, 17688, 17898, 18273, 0 };
#define LAST_DBC_IN_DATA_BUILD 15595    // after this build mpqs with dbc are back to locale folder
#define NEW_BASE_SET_BUILD 16016 // 15211
#endif

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

typedef struct
{
    char name[64];
    unsigned int id;
}map_id;

map_id * map_ids;
uint16_t *LiqType = 0;
uint32_t map_count;
char output_path[128]=".";
char input_path[1024]=".";
bool preciseVectorData = false;

// Constants

//static const char * szWorkDirMaps = ".\\Maps";
const char* szWorkDirWmo = "./Buildings";
const char* szRawVMAPMagic = "VMAP041";

// Note: StormLib's SFileOpenPatchArchive() took a "path prefix" that remaps
// files stored under a locale subfolder inside a shared patch archive so they
// become addressable by their bare name. mpqlib has no equivalent - only
// relevant for archives at/below LAST_DBC_IN_DATA_BUILD, a build from years
// before Cata/Mop ever shipped, so in practice every archive this tool
// actually opens today never needs it.
bool LoadLocaleMPQFile(int locale)
{
    char buff[512];

#if VERSION_STRING < Mop
    snprintf(buff, sizeof(buff), "%s%s/locale-%s.MPQ", input_path, LocalesT[locale], LocalesT[locale]);
    LocaleMpq = std::make_unique<mpqlib::MpqPatchChain>(buff);
    if (!LocaleMpq->isOpen())
    {
        LocaleMpq.reset();
        return false;
    }

    printf("Loading %s locale MPQs\n", LocalesT[locale]);
    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        // Do not attempt to read older MPQ patch archives past this build, they were merged with base
        // and trying to read them together with new base will not end well
        if (CONF_TargetBuild >= NEW_BASE_SET_BUILD && Builds[i] < NEW_BASE_SET_BUILD)
            continue;

        if (Builds[i] > LAST_DBC_IN_DATA_BUILD)
            snprintf(buff, sizeof(buff), "%s%s/wow-update-%s-%u.MPQ", input_path, LocalesT[locale], LocalesT[locale], Builds[i]);
        else
            snprintf(buff, sizeof(buff), "%swow-update-%u.MPQ", input_path, Builds[i]);

        LocaleMpq->addPatch(buff);
    }

    printf("\n");
    return true;
#else
    // Mirrors map_extractor's Mop-branch LoadLocaleMPQFile (System.cpp): misc.MPQ
    // as base, locale-XX.MPQ as the first patch, then wow-update-base-*, then the
    // locale wow-update-XX-*, then Data\Cache\patch-base-* and finally
    // Data\Cache\<locale>\patch-<locale>-*. Without the two Cache loops, DBC
    // reads through LocaleMpq (Map.dbc, GameObjectDisplayInfo.dbc, ...) silently
    // resolve to an older, incomplete archive - late-Mop map/gameobject records
    // added only via a Cache hotfix patch never show up.
    snprintf(buff, sizeof(buff), "%smisc.MPQ", input_path);
    LocaleMpq = std::make_unique<mpqlib::MpqPatchChain>(buff);
    if (!LocaleMpq->isOpen())
    {
        LocaleMpq.reset();
        return false;
    }

    snprintf(buff, sizeof(buff), "%s%s/locale-%s.MPQ", input_path, LocalesT[locale], LocalesT[locale]);
    if (!LocaleMpq->addPatch(buff))
    {
        LocaleMpq.reset();
        return false;
    }

    printf("Loading %s locale MPQs\n", LocalesT[locale]);

    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        snprintf(buff, sizeof(buff), "%swow-update-base-%u.MPQ", input_path, Builds[i]);

        if (!LocaleMpq->addPatch(buff))
            printf("Not found %s\n", buff);
        else
            printf("Loaded %s\n", buff);
    }

    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        snprintf(buff, sizeof(buff), "%s%s/wow-update-%s-%u.MPQ", input_path, LocalesT[locale], LocalesT[locale], Builds[i]);

        if (LocaleMpq->addPatch(buff))
            printf("Loaded %s\n", buff);
    }

    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        snprintf(buff, sizeof(buff), "%sCache\\patch-base-%u.MPQ", input_path, Builds[i]);

        if (LocaleMpq->addPatch(buff))
            printf("Loaded %s\n", buff);
    }

    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        snprintf(buff, sizeof(buff), "%sCache\\%s\\patch-%s-%u.MPQ", input_path, LocalesT[locale], LocalesT[locale], Builds[i]);

        if (LocaleMpq->addPatch(buff))
            printf("Loaded %s\n", buff);
    }

    printf("\n");
    return true;
#endif
}

void LoadCommonMPQFiles(uint32_t build)
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

    int count = sizeof(CONF_mpq_list) / sizeof(char*);
    for (int i = 1; i < count; ++i)
    {
        if (build < 15211 && !strcmp("world2.MPQ", CONF_mpq_list[i]))   // 4.3.2 and higher MPQ
            continue;

        snprintf(filename, sizeof(filename), "%s%s", input_path, CONF_mpq_list[i]);
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
            snprintf(filename, sizeof(filename), "%swow-update-base-%u.MPQ", input_path, Builds[i]);
        else
            snprintf(filename, sizeof(filename), "%swow-update-%u.MPQ", input_path, Builds[i]);

        if (!WorldMpq->addPatch(filename))
        {
            printf("Not found %s\n", filename);
            continue;
        }

        printf("Loaded %s\n", filename);
    }

    // Hotfix cache archives can update/add WDT, ADT, WMO and model data just
    // like they update DBC data (LoadLocaleMPQFile already loads these for
    // LocaleMpq) - without them, content delivered purely via a cache patch
    // (e.g. late-Mop zones such as HawaiiMainLand/Isle of Thunder) has no WDT
    // to convert at all.
    for (int i = 0; Builds[i] && Builds[i] <= CONF_TargetBuild; ++i)
    {
        snprintf(filename, sizeof(filename), "%sCache\\patch-base-%u.MPQ", input_path, Builds[i]);

        if (!WorldMpq->addPatch(filename))
        {
            printf("Not found %s\n", filename);
            continue;
        }

        printf("Loaded %s\n", filename);
    }

    printf("\n");
}


// Local testing functions

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
        // Cast input to unsigned char for safety, and the int result back to char
        *str = static_cast<char>(std::tolower(static_cast<unsigned char>(*str)));
        ++str;
    }
}

// copied from contrib/extractor/System.cpp
void ReadLiquidTypeTableDBC()
{
#if VERSION_STRING == Cata
    printf("Read LiquidType.dbc file...");

    DBCFile dbc(*LocaleMpq, "DBFilesClient\\LiquidType.dbc");
    if(!dbc.open())
    {
        printf("Fatal error: Invalid LiquidType.dbc file format!\n");
        exit(1);
    }

    size_t LiqType_count = dbc.getRowCount();
    size_t LiqType_maxid = dbc.getRow(LiqType_count - 1).getUInt(0);
    LiqType = new uint16_t[LiqType_maxid + 1];
    memset(LiqType, 0xff, (LiqType_maxid + 1) * sizeof(uint16_t));

    for(uint32_t x = 0; x < LiqType_count; ++x)
        LiqType[dbc.getRow(x).getUInt(0)] = static_cast<uint16_t>(dbc.getRow(x).getUInt(3));

    printf("Done! (%u LiqTypes loaded)\n", (unsigned int)LiqType_count);
#else
    char localMPQ[1024];
    snprintf(localMPQ, sizeof(localMPQ), "%smisc.MPQ", input_path);

    mpqlib::MpqPatchChain localeChain(localMPQ);
    if (!localeChain.isOpen())
        exit(1);

    printf("Read LiquidType.dbc file...");

    DBCFile dbc(localeChain, "DBFilesClient\\LiquidType.dbc");
    if (!dbc.open())
    {
        printf("Fatal error: Invalid LiquidType.dbc file format!\n");
        exit(1);
    }

    size_t LiqType_count = dbc.getRowCount();
    size_t LiqType_maxid = dbc.getMaxId();
    LiqType = new uint16_t[LiqType_maxid + 1];
    memset(LiqType, 0xff, (LiqType_maxid + 1) * sizeof(uint16_t));

    for (size_t x = 0; x < LiqType_count; ++x)
        LiqType[dbc.getRow(x).getUInt(0)] = static_cast<uint16_t>(dbc.getRow(x).getUInt(3));

    printf("Done! (%zu LiqTypes loaded)\n", LiqType_count);
#endif
}

// Minimal single-'*' suffix matcher, replacing StormLib's SFileFindFirstFile/
// SFileFindNextFile - this tool only ever searches for a plain "*.wmo" suffix.
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

bool ExtractWmo()
{
    bool success = false;

    for (std::string const& name : WorldMpq->listFiles())
    {
        if (!MatchesSimplePattern(name, "*.wmo"))
            continue;

        std::string str = name;
        //printf("Extracting wmo %s\n", str.c_str());
        success |= ExtractSingleWmo(str);
    }

    if (success)
        printf("\nExtract wmo complete (No (fatal) errors)\n");

    return success;
}

bool ExtractSingleWmo(std::string& fname)
{
    // Copy files from archive

    char szLocalFile[1024];
    const char * plain_name = getPlainName(fname.c_str());
    sprintf(szLocalFile, "%s/%s", szWorkDirWmo, plain_name);
    fixNameCase(szLocalFile,strlen(szLocalFile));

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
    if(!froot.open())
    {
        printf("Couldn't open RootWmo!!!\n");
        return true;
    }
    FILE *output = fopen(szLocalFile,"wb");
    if(!output)
    {
        printf("couldn't open %s for writing!\n", szLocalFile);
        return false;
    }
    froot.ConvertToVMAPRootWmo(output);
    int Wmo_nVertices = 0;
    //printf("root has %d groups\n", froot->nGroups);
    if (froot.nGroups !=0)
    {
        for (uint32_t i = 0; i < froot.nGroups; ++i)
        {
            char temp[1024];
            strcpy(temp, fname.c_str());
            temp[fname.length()-4] = 0;
            char groupFileName[1024];
            sprintf(groupFileName, "%s_%03u.wmo", temp, i);
            //printf("Trying to open groupfile %s\n",groupFileName);

            std::string s = groupFileName;
            WMOGroup fgroup(s);
            if(!fgroup.open())
            {
                printf("Could not open all Group file for: %s\n", plain_name);
                file_ok = false;
                break;
            }

            Wmo_nVertices += fgroup.ConvertToVMAPGroupWmo(output, &froot, preciseVectorData);
        }
    }

    fseek(output, 8, SEEK_SET); // store the correct no of vertices
    fwrite(&Wmo_nVertices,sizeof(int),1,output);
    fclose(output);

    // Delete the extracted file in the case of an error
    if (!file_ok)
        remove(szLocalFile);
    return true;
}

void ParsMapFiles()
{
    char fn[512];
    //char id_filename[64];
    char id[10];
    for (unsigned int i=0; i<map_count; ++i)
    {
        sprintf(id, "%04u", map_ids[i].id);
        sprintf(fn,"World\\Maps\\%s\\%s.wdt", map_ids[i].name, map_ids[i].name);
        WDTFile WDT(fn,map_ids[i].name);
        if(WDT.init(id, map_ids[i].id))
        {
            printf("Processing Map %u\n[", map_ids[i].id);
            for (int x=0; x<64; ++x)
            {
                for (int y=0; y<64; ++y)
                {
                    if (ADTFile *ADT = WDT.getMap(x,y))
                    {
                        //sprintf(id_filename,"%02u %02u %03u",x,y,map_ids[i].id);//!!!!!!!!!
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

void getGamePath()
{
#ifdef _WIN32
    strcpy(input_path,"Data\\");
#else
    strcpy(input_path,"Data/");
#endif
}

bool processArgv(int argc, char ** argv, const char *versionString)
{
    bool result = true;
    bool hasInputPathParam = false;
    preciseVectorData = false;

    for(int i = 1; i < argc; ++i)
    {
        if(strcmp("-s",argv[i]) == 0)
        {
            preciseVectorData = false;
        }
        else if(strcmp("-d",argv[i]) == 0)
        {
            if((i+1)<argc)
            {
                hasInputPathParam = true;
                strcpy(input_path, argv[i+1]);
                if (input_path[strlen(input_path) - 1] != '\\' && input_path[strlen(input_path) - 1] != '/')
                    strcat(input_path, "/");
                ++i;
            }
            else
            {
                result = false;
            }
        }
        else if(strcmp("-?",argv[1]) == 0)
        {
            result = false;
        }
        else if(strcmp("-l",argv[i]) == 0)
        {
            preciseVectorData = true;
        }
        else if(strcmp("-b",argv[i]) == 0)
        {
            if (i + 1 < argc)                            // all ok
                CONF_TargetBuild = atoi(argv[i++ + 1]);
        }
        else
        {
            result = false;
            break;
        }
    }

    if(!result)
    {
        printf("Extract %s.\n",versionString);
        printf("%s [-?][-s][-l][-d <path>]\n", argv[0]);
        printf("   -s : (default) small size (data size optimization), ~500MB less vmap data.\n");
        printf("   -l : large size, ~500MB more vmap data. (might contain more details)\n");
        printf("   -d <path>: Path to the vector data source folder.\n");
        printf("   -b : target build (default %u)\n", CONF_TargetBuild);
        printf("   -? : This message.\n");
    }

    if(!hasInputPathParam)
        getGamePath();

    return result;
}


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Main
//
// The program must be run with two command line arguments
//
// Arg1 - The source MPQ name (for testing reading and file find)
// Arg2 - Listfile name
//

int main(int argc, char ** argv)
{
    bool success=true;
    const char *versionString = "V4.00 2012_02";

    // Use command line arguments, when some
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

    printf("Extract %s. Beginning work ....\n\n",versionString);
    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    // Create the working directory
    if (mkdir(szWorkDirWmo
#if defined(__linux__) || defined(__APPLE__)
                    , 0711
#endif
                    ))
            success = (errno == EEXIST);

    LoadCommonMPQFiles(CONF_TargetBuild);

    for (int i = 0; i < LOCALES_COUNT; ++i)
    {
        //Open MPQs
        if (!LoadLocaleMPQFile(i))
            continue;

        printf("Detected and using locale: %s\n", Locales[i]);
        break;
    }

    ReadLiquidTypeTableDBC();

    // extract data
    if (success)
        success = ExtractWmo();

    //xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    //map.dbc
    if (success)
    {
        DBCFile * dbc = new DBCFile(*LocaleMpq, "DBFilesClient\\Map.dbc");
        if (!dbc->open())
        {
            delete dbc;
            printf("FATAL ERROR: Map.dbc not found in data file.\n");
            return 1;
        }
        map_count=static_cast<uint32_t>(dbc->getRowCount());
        map_ids=new map_id[map_count];
        for (unsigned int x=0;x<map_count;++x)
        {
            map_ids[x].id=dbc->getRow(x).getUInt(0);
            strcpy(map_ids[x].name,dbc->getRow(x).getString(1));
            printf("Map - %s\n",map_ids[x].name);
        }


        delete dbc;
        ParsMapFiles();
        delete [] map_ids;
        //nError = ERROR_SUCCESS;
        // Extract models, listed in GameObjectDisplayInfo.dbc
        ExtractGameobjectModels();
    }

    LocaleMpq.reset();
    WorldMpq.reset();

    printf("\n");
    if (!success)
    {
        printf("ERROR: Extract %s. Work NOT complete.\n   Precise vector data=%d.\nPress any key.\n",versionString, preciseVectorData);
        getchar();
    }

    printf("Extract %s. Work complete. No errors.\n",versionString);
    delete [] LiqType;
    return 0;
}
