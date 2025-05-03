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
struct adt_MCVT
{
    union {
        uint32_t fcc;
        char   fcc_txt[4];
    };
    uint32_t size;
    float height_map[(ADT_CELL_SIZE + 1) * (ADT_CELL_SIZE + 1) + ADT_CELL_SIZE * ADT_CELL_SIZE];
};

//
// Adt file liquid map chunk (old)
//
struct adt_MCLQ
{
    union {
        uint32_t fcc;
        char   fcc_txt[4];
    };
    uint32_t size;
    float height1;
    float height2;
    struct liquid_data {
        uint32_t light;
        float  height;
    } liquid[ADT_CELL_SIZE + 1][ADT_CELL_SIZE + 1];

    // 1<<0 - ochen
    // 1<<1 - lava/slime
    // 1<<2 - water
    // 1<<6 - all water
    // 1<<7 - dark water
    // == 0x0F - not show liquid
    uint8_t flags[ADT_CELL_SIZE][ADT_CELL_SIZE];
    uint8_t data[84];
};

//
// Adt file cell chunk
//
struct adt_MCNK
{
    union {
        uint32_t fcc;
        char   fcc_txt[4];
    };
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
};


#define ADT_LIQUID_HEADER_FULL_LIGHT   0x01
#define ADT_LIQUID_HEADER_NO_HIGHT     0x02

enum class LiquidVertexFormatType : uint16_t
{
    HeightDepth = 0,
    HeightTextureCoord = 1,
    Depth = 2,
    HeightDepthTextureCoord = 3,
    Unk4 = 4,
    Unk5 = 5
};

struct adt_liquid_instance
{
    uint16_t LiquidType;              // Index from LiquidType.dbc
    uint16_t LiquidVertexFormat;      // Id from LiquidObject.dbc if >= 42
    float MinHeightLevel;
    float MaxHeightLevel;
    uint8_t OffsetX;
    uint8_t OffsetY;
    uint8_t Width;
    uint8_t Height;
    uint32_t OffsetExistsBitmap;
    uint32_t OffsetVertexData;

    uint8_t GetOffsetX() const { return LiquidVertexFormat < 42 ? OffsetX : 0; }
    uint8_t GetOffsetY() const { return LiquidVertexFormat < 42 ? OffsetY : 0; }
    uint8_t GetWidth() const { return LiquidVertexFormat < 42 ? Width : 8; }
    uint8_t GetHeight() const { return LiquidVertexFormat < 42 ? Height : 8; }
};

struct adt_liquid_attributes
{
    uint64_t Fishable;
    uint64_t Deep;
};

//
// Adt file liquid data chunk (new)
//
class adt_MH2O
{
public:
    union {
        uint32_t fcc;
        char   fcc_txt[4];
    };
    uint32_t size;

    struct adt_LIQUID {
        uint32_t OffsetInstances;
        uint32_t used;
        uint32_t OffsetAttributes;
    } liquid[ADT_CELLS_PER_GRID][ADT_CELLS_PER_GRID];

    bool   prepareLoadedData();

    adt_liquid_instance const* GetLiquidInstance(int32_t x, int32_t y) const
    {
        if (liquid[x][y].used && liquid[x][y].OffsetInstances)
            return (adt_liquid_instance*)((uint8_t*)this + 8 + liquid[x][y].OffsetInstances);
        return nullptr;
    }

    adt_liquid_attributes GetLiquidAttributes(int32_t x, int32_t y) const
    {
        if (liquid[x][y].used)
        {
            if (liquid[x][y].OffsetAttributes)
                return *((adt_liquid_attributes*)((uint8_t*)this + 8 + liquid[x][y].OffsetAttributes));
            return { 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF };
        }
        return { 0, 0 };
    }

    uint16_t GetLiquidType(adt_liquid_instance const* h) const
    {
        if (GetLiquidVertexFormat(h) == LiquidVertexFormatType::Depth)
            return 2;

        return h->LiquidType;
    }

    float GetLiquidHeight(adt_liquid_instance const* h, int32_t pos) const
    {
        if (!h->OffsetVertexData)
            return 0.0f;
        if (GetLiquidVertexFormat(h) == LiquidVertexFormatType::Depth)
            return 0.0f;

        switch (GetLiquidVertexFormat(h))
        {
        case LiquidVertexFormatType::HeightDepth:
        case LiquidVertexFormatType::HeightTextureCoord:
        case LiquidVertexFormatType::HeightDepthTextureCoord:
            return ((float const*)((uint8_t*)this + 8 + h->OffsetVertexData))[pos];
        case LiquidVertexFormatType::Depth:
            return 0.0f;
        case LiquidVertexFormatType::Unk4:
        case LiquidVertexFormatType::Unk5:
            return ((float const*)((uint8_t*)this + 8 + h->OffsetVertexData + 4))[pos * 2];
        default:
            break;
        }

        return 0.0f;
    }

    int8_t GetLiquidDepth(adt_liquid_instance const* h, int32_t pos) const
    {
        if (!h->OffsetVertexData)
            return -1;

        switch (GetLiquidVertexFormat(h))
        {
        case LiquidVertexFormatType::HeightDepth:
            return ((int8_t const*)((int8_t const*)this + 8 + h->OffsetVertexData + (h->GetWidth() + 1) * (h->GetHeight() + 1) * 4))[pos];
        case LiquidVertexFormatType::HeightTextureCoord:
            return 0;
        case LiquidVertexFormatType::Depth:
            return ((int8_t const*)((uint8_t*)this + 8 + h->OffsetVertexData))[pos];
        case LiquidVertexFormatType::HeightDepthTextureCoord:
            return ((int8_t const*)((int8_t const*)this + 8 + h->OffsetVertexData + (h->GetWidth() + 1) * (h->GetHeight() + 1) * 8))[pos];
        case LiquidVertexFormatType::Unk4:
            return ((int8_t const*)((uint8_t*)this + 8 + h->OffsetVertexData))[pos * 8];
        case LiquidVertexFormatType::Unk5:
            return 0;
        default:
            break;
        }
        return 0;
    }

    uint16_t const* GetLiquidTextureCoordMap(adt_liquid_instance const* h, int32_t pos) const
    {
        if (!h->OffsetVertexData)
            return nullptr;

        switch (GetLiquidVertexFormat(h))
        {
        case LiquidVertexFormatType::HeightDepth:
        case LiquidVertexFormatType::Depth:
        case LiquidVertexFormatType::Unk4:
            return nullptr;
        case LiquidVertexFormatType::HeightTextureCoord:
        case LiquidVertexFormatType::HeightDepthTextureCoord:
            return (uint16_t const*)((uint8_t const*)this + 8 + h->OffsetVertexData + 4 * ((h->GetWidth() + 1) * (h->GetHeight() + 1) + pos));
        case LiquidVertexFormatType::Unk5:
            return (uint16_t const*)((uint8_t const*)this + 8 + h->OffsetVertexData + 8 * ((h->GetWidth() + 1) * (h->GetHeight() + 1) + pos));
        default:
            break;
        }
        return nullptr;
    }

    uint64_t GetLiquidExistsBitmap(adt_liquid_instance const* h) const
    {
        if (h->OffsetExistsBitmap)
            return *((uint64_t*)((uint8_t*)this + 8 + h->OffsetExistsBitmap));
        else
            return 0xFFFFFFFFFFFFFFFFuLL;
    }

    LiquidVertexFormatType GetLiquidVertexFormat(adt_liquid_instance const* liquidInstance) const;
};

struct adt_MFBO
{
    union
    {
        uint32_t fcc;
        char   fcc_txt[4];
    };
    uint32_t size;
    struct plane
    {
        int16_t coords[9];
    };
    plane max;
    plane min;
};

#pragma pack(pop)

#endif  //ADT_H
