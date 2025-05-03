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

#ifndef ADT_H
#define ADT_H

#include "loadlib.h"

#define TILESIZE (533.33333f)
#define CHUNKSIZE ((TILESIZE) / 16.0f)
#define UNITSIZE (CHUNKSIZE / 8.0f)

enum LiquidType
{
    LIQUID_TYPE_WATER = 0,
    LIQUID_TYPE_OCEAN = 1,
    LIQUID_TYPE_MAGMA = 2,
    LIQUID_TYPE_SLIME = 3
};

//**************************************************************************************
// ADT file class
//**************************************************************************************
#define ADT_CELLS_PER_GRID    16
#define ADT_CELL_SIZE         8
#define ADT_GRID_SIZE         (ADT_CELLS_PER_GRID*ADT_CELL_SIZE)

#pragma pack(push, 1)

//
// Adt file height map chunk
//
class adt_MCVT
{
    union{
        uint32_t fcc;
        char   fcc_txt[4];
    };
    uint32_t size;
public:
    float height_map[(ADT_CELL_SIZE+1)*(ADT_CELL_SIZE+1)+ADT_CELL_SIZE*ADT_CELL_SIZE];

    bool  prepareLoadedData();
};

//
// Adt file liquid map chunk (old)
//
class adt_MCLQ
{
    union{
        uint32_t fcc;
        char   fcc_txt[4];
    };
public:
    uint32_t size;
    float height1;
    float height2;
    struct liquid_data{
        uint32_t light;
        float  height;
    } liquid[ADT_CELL_SIZE+1][ADT_CELL_SIZE+1];

    // 1<<0 - ochen
    // 1<<1 - lava/slime
    // 1<<2 - water
    // 1<<6 - all water
    // 1<<7 - dark water
    // == 0x0F - not show liquid
    uint8_t flags[ADT_CELL_SIZE][ADT_CELL_SIZE];
    uint8_t data[84];
    bool  prepareLoadedData();
};

//
// Adt file cell chunk
//
class adt_MCNK
{
    union{
        uint32_t fcc;
        char   fcc_txt[4];
    };
public:
    uint32_t size;
    uint32_t flags;
    uint32_t ix;
    uint32_t iy;
    uint32_t nLayers;
    uint32_t nDoodadRefs;
    uint32_t offsMCVT;        // height map
    uint32_t offsMCNR;        // Normal vectors for each vertex
    uint32_t offsMCLY;        // Texture layer definitions
    uint32_t offsMCRF;        // A list of indices into the parent file's MDDF chunk
    uint32_t offsMCAL;        // Alpha maps for additional texture layers
    uint32_t sizeMCAL;
    uint32_t offsMCSH;        // Shadow map for static shadows on the terrain
    uint32_t sizeMCSH;
    uint32_t areaid;
    uint32_t nMapObjRefs;
    uint32_t holes;
    uint16_t s[2];
    uint32_t data1;
    uint32_t data2;
    uint32_t data3;
    uint32_t predTex;
    uint32_t nEffectDoodad;
    uint32_t offsMCSE;
    uint32_t nSndEmitters;
    uint32_t offsMCLQ;         // Liqid level (old)
    uint32_t sizeMCLQ;         //
    float  zpos;
    float  xpos;
    float  ypos;
    uint32_t offsMCCV;         // offsColorValues in WotLK
    uint32_t props;
    uint32_t effectId;

    bool   prepareLoadedData();
    adt_MCVT *getMCVT()
    {
        if (offsMCVT)
            return (adt_MCVT *)((uint8_t *)this + offsMCVT);
        return 0;
    }
    adt_MCLQ *getMCLQ()
    {
        if (offsMCLQ)
            return (adt_MCLQ *)((uint8_t *)this + offsMCLQ);
        return 0;
    }
};

//
// Adt file grid chunk
//
class adt_MCIN
{
    union{
        uint32_t fcc;
        char   fcc_txt[4];
    };
public:
    uint32_t size;
    struct adt_CELLS{
        uint32_t offsMCNK;
        uint32_t size;
        uint32_t flags;
        uint32_t asyncId;
    } cells[ADT_CELLS_PER_GRID][ADT_CELLS_PER_GRID];

    bool   prepareLoadedData();
    // offset from begin file (used this-84)
    adt_MCNK *getMCNK(int x, int y)
    {
        if (cells[x][y].offsMCNK)
            return (adt_MCNK *)((uint8_t *)this + cells[x][y].offsMCNK - 84);
        return 0;
    }
};

#define ADT_LIQUID_HEADER_FULL_LIGHT   0x01
#define ADT_LIQUID_HEADER_NO_HIGHT     0x02

struct adt_liquid_header{
    uint16_t liquidType;             // Index from LiquidType.dbc
    uint16_t formatFlags;
    float  heightLevel1;
    float  heightLevel2;
    uint8_t  xOffset;
    uint8_t  yOffset;
    uint8_t  width;
    uint8_t  height;
    uint32_t offsData2a;
    uint32_t offsData2b;
};

//
// Adt file liquid data chunk (new)
//
class adt_MH2O
{
public:
    union{
        uint32_t fcc;
        char   fcc_txt[4];
    };
    uint32_t size;

    struct adt_LIQUID{
        uint32_t offsData1;
        uint32_t used;
        uint32_t offsData2;
    } liquid[ADT_CELLS_PER_GRID][ADT_CELLS_PER_GRID];

    bool   prepareLoadedData();

    adt_liquid_header *getLiquidData(int x, int y)
    {
        if (liquid[x][y].used && liquid[x][y].offsData1)
            return (adt_liquid_header *)((uint8_t*)this + 8 + liquid[x][y].offsData1);
        return 0;
    }

    float *getLiquidHeightMap(adt_liquid_header *h)
    {
        if (h->formatFlags & ADT_LIQUID_HEADER_NO_HIGHT)
            return 0;
        if (h->offsData2b)
            return (float *)((uint8_t*)this + 8 + h->offsData2b);
        return 0;
    }

    uint8_t *getLiquidLightMap(adt_liquid_header *h)
    {
        if (h->formatFlags&ADT_LIQUID_HEADER_FULL_LIGHT)
            return 0;
        if (h->offsData2b)
        {
            if (h->formatFlags & ADT_LIQUID_HEADER_NO_HIGHT)
                return (uint8_t *)((uint8_t*)this + 8 + h->offsData2b);
            return (uint8_t *)((uint8_t*)this + 8 + h->offsData2b + (h->width+1)*(h->height+1)*4);
        }
        return 0;
    }

    uint32_t *getLiquidFullLightMap(adt_liquid_header *h)
    {
        if (!(h->formatFlags&ADT_LIQUID_HEADER_FULL_LIGHT))
            return 0;
        if (h->offsData2b)
        {
            if (h->formatFlags & ADT_LIQUID_HEADER_NO_HIGHT)
                return (uint32_t *)((uint8_t*)this + 8 + h->offsData2b);
            return (uint32_t *)((uint8_t*)this + 8 + h->offsData2b + (h->width+1)*(h->height+1)*4);
        }
        return 0;
    }

    uint64_t getLiquidShowMap(adt_liquid_header *h)
    {
        if (h->offsData2a)
            return *((uint64_t *)((uint8_t*)this + 8 + h->offsData2a));
        else
            return 0xFFFFFFFFFFFFFFFFuLL;
    }

};

//
// Adt file header chunk
//
class adt_MHDR
{
    union{
        uint32_t fcc;
        char   fcc_txt[4];
    };
public:
    uint32_t size;

    uint32_t pad;
    uint32_t offsMCIN;           // MCIN
    uint32_t offsTex;               // MTEX
    uint32_t offsModels;           // MMDX
    uint32_t offsModelsIds;       // MMID
    uint32_t offsMapObejcts;       // MWMO
    uint32_t offsMapObejctsIds;  // MWID
    uint32_t offsDoodsDef;       // MDDF
    uint32_t offsObjectsDef;     // MODF
    uint32_t offsMFBO;           // MFBO
    uint32_t offsMH2O;           // MH2O
    uint32_t data1;
    uint32_t data2;
    uint32_t data3;
    uint32_t data4;
    uint32_t data5;
    bool prepareLoadedData();
    adt_MCIN *getMCIN(){ return (adt_MCIN *)((uint8_t *)&pad+offsMCIN);}
    adt_MH2O *getMH2O(){ return offsMH2O ? (adt_MH2O *)((uint8_t *)&pad+offsMH2O) : 0;}

};

class ADT_file : public FileLoader{
public:
    bool prepareLoadedData();
    ADT_file();
    ~ADT_file();
    void free();

    adt_MHDR *a_grid;
};

#pragma pack(pop)

#endif
