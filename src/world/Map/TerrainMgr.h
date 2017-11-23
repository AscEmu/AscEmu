/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

#include "Threading/Mutex.h"
#include "../world/Server/World.h"
#include <cstdio>

namespace VMAP
{
    class IVMapManager;
    class VMapFactory;
};

#define TERRAIN_INVALID_HEIGHT -50000
#define TERRAIN_TILE_SIZE 533.3333333f
#define TERRAIN_NUM_TILES 64
#define TERRAIN_MAP_RESOLUTION 128

class TerrainHolder;
class TerrainTile;

#define MAP_AREA_NO_AREA      0x0001

#define MAP_HEIGHT_NO_HEIGHT  0x0001
#define MAP_HEIGHT_AS_INT16   0x0002
#define MAP_HEIGHT_AS_INT8    0x0004

#define MAP_LIQUID_NO_TYPE    0x0001
#define MAP_LIQUID_NO_HEIGHT  0x0002


struct TileMapHeader
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

struct TileMapAreaHeader
{
    uint32_t fourcc;
    uint16_t flags;
    uint16_t gridArea;
};

struct TileMapHeightHeader
{
    uint32_t fourcc;
    uint32_t flags;
    float gridHeight;
    float gridMaxHeight;
};

struct TileMapLiquidHeader
{
    uint32_t fourcc;
    uint16_t flags;
    uint16_t liquidType;
    uint8_t offsetX;
    uint8_t offsetY;
    uint8_t width;
    uint8_t height;
    float liquidLevel;
};

class TileMap
{
    public:

        //Area Map
        uint16_t m_area;
        uint16_t* m_areaMap;

        //Height Map
        union
        {
            float* m_heightMap8F;
            uint16_t* m_heightMap8S;
            uint8_t* m_heightMap8B;
        };
        union
        {
            float* m_heightMap9F;
            uint16_t* m_heightMap9S;
            uint8_t* m_heightMap9B;
        };
        uint32_t m_heightMapFlags;
        float m_heightMapMult;
        float m_tileHeight;

        //Liquid Map
        uint8_t* m_liquidType;
        float* m_liquidMap;
        float m_liquidLevel;
        uint8_t m_liquidOffX;
        uint8_t m_liquidOffY;
        uint8_t m_liquidHeight;
        uint8_t m_liquidWidth;
        uint16_t m_defaultLiquidType;

        TileMap();
        ~TileMap();

        void Load(char* filename);

        void LoadLiquidData(FILE* f, TileMapHeader & header);
        void LoadHeightData(FILE* f, TileMapHeader & header);
        void LoadAreaData(FILE* f, TileMapHeader & header);

        float GetHeight(float x, float y);
        float GetHeightB(float x, float y, int x_int, int y_int);
        float GetHeightS(float x, float y, int x_int, int y_int);
        float GetHeightF(float x, float y, int x_int, int y_int);

        float GetTileLiquidHeight(float x, float y);
        uint8_t GetTileLiquidType(float x, float y);

        uint32_t GetTileArea(float x, float y);
};

class TerrainTile
{
    public:

        std::atomic<unsigned long> m_refs;

        TerrainHolder* m_parent;
        uint32_t m_mapid;
        int32_t m_tx;
        int32_t m_ty;

        //Children
        TileMap m_map;

        TerrainTile(TerrainHolder* parent, uint32_t mapid, int32_t x, int32_t y);
        ~TerrainTile();

        void AddRef() { ++m_refs; }
        void DecRef() { if (--m_refs == 0) delete this; }

        void Load()
        {
            char filename[1024];

            //Normal map stuff
            sprintf(filename, "%smaps/%03u%02u%02u.map", sWorld.settings.server.dataDir.c_str(), m_mapid, m_tx, m_ty);
            m_map.Load(filename);
        }
};

class TerrainHolder
{
    public:

        // This should be in AreaStorage.cpp
        const bool GetAreaInfo(float x, float y, float z, uint32_t &mogp_flags, int32_t &adt_id, int32_t &root_id, int32_t &group_id);

        uint32_t m_mapid;
        TerrainTile* m_tiles[TERRAIN_NUM_TILES][TERRAIN_NUM_TILES];
        FastMutex m_lock[TERRAIN_NUM_TILES][TERRAIN_NUM_TILES];
        std::atomic<unsigned long> m_tilerefs[TERRAIN_NUM_TILES][TERRAIN_NUM_TILES];

        TerrainHolder(uint32_t mapid);
        ~TerrainHolder();

        uint32_t GetAreaFlagWithoutAdtId(float x, float y);

        TerrainTile* GetTile(float x, float y);
        TerrainTile* GetTile(int32_t tx, int32_t ty);

        void LoadTile(float x, float y);
        void LoadTile(int32_t tx, int32_t ty);

        void UnloadTile(float x, float y);
        void UnloadTile(int32_t tx, int32_t ty);

        //test
        uint32_t GetAreaFlag(float x, float y);
};
