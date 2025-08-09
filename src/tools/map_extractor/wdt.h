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

#ifndef WDT_H
#define WDT_H

#include "loadlib.h"

//**************************************************************************************
// WDT file class and structures
//**************************************************************************************
#define WDT_MAP_SIZE 64

#pragma pack(push, 1)

class wdt_MWMO{
    union{
        uint32_t fcc;
        char   fcc_txt[4];
    };
public:
    uint32_t size;
    bool prepareLoadedData();
};

class wdt_MPHD{
    union{
        uint32_t fcc;
        char   fcc_txt[4];
    };
public:
    uint32_t size;

    uint32_t data1;
    uint32_t data2;
    uint32_t data3;
    uint32_t data4;
    uint32_t data5;
    uint32_t data6;
    uint32_t data7;
    uint32_t data8;
    bool   prepareLoadedData();
};

class wdt_MAIN{
    union{
        uint32_t fcc;
        char   fcc_txt[4];
    };
public:
    uint32_t size;

    struct adtData{
        uint32_t exist;
        uint32_t data1;
    } adt_list[64][64];

    bool   prepareLoadedData();
};

class WDT_file : public FileLoader{
public:
    bool   prepareLoadedData();

    WDT_file();
    ~WDT_file();
    void free();

    wdt_MPHD *mphd;
    wdt_MAIN *main;
    wdt_MWMO *wmo;
};

#pragma pack(pop)

#endif
