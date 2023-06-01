/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdio>

#include "Movement/MovementDefines.h"
#include "Server/World.h"
#include "Threading/Mutex.h"

namespace G3D { class Plane; }
namespace VMAP
{
    class IVMapManager;
    class VMapFactory;
};

class TerrainHolder;
class TerrainTile;

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
    uint8_t flags;
    uint8_t liquidFlags;
    uint16_t liquidType;
    uint8_t offsetX;
    uint8_t offsetY;
    uint8_t width;
    uint8_t height;
    float liquidLevel;
};

enum ZLiquidStatus : uint32_t
{
    LIQUID_MAP_NO_WATER         = 0x00000000,
    LIQUID_MAP_ABOVE_WATER      = 0x00000001,
    LIQUID_MAP_WATER_WALK       = 0x00000002,
    LIQUID_MAP_IN_WATER         = 0x00000004,
    LIQUID_MAP_UNDER_WATER      = 0x00000008,

    MAP_LIQUID_STATUS_SWIMMING  = (LIQUID_MAP_IN_WATER | LIQUID_MAP_UNDER_WATER),
    MAP_LIQUID_STATUS_IN_CONTACT = (MAP_LIQUID_STATUS_SWIMMING | LIQUID_MAP_WATER_WALK)
};

enum MapLiquidType
{
     MAP_LIQUID_TYPE_NO_WATER   = 0x00,
     MAP_LIQUID_TYPE_WATER      = 0x01,
     MAP_LIQUID_TYPE_OCEAN      = 0x02,
     MAP_LIQUID_TYPE_MAGMA      = 0x04,
     MAP_LIQUID_TYPE_SLIME      = 0x08,
     MAP_LIQUID_TYPE_DARK_WATER = 0x10,
     MAP_ALL_LIQUIDS            = (MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN | MAP_LIQUID_TYPE_MAGMA | MAP_LIQUID_TYPE_SLIME)
};

struct LiquidData
{
    uint32_t type_flags;
    uint32_t entry;
    float  level;
    float  depth_level;
};

class TileMap
{
public:
    uint32_t m_heightMapFlags = 0;

    // Height Map
    union
    {
        float* m_heightMap8F = nullptr;
        uint16_t* m_heightMap8S;
        uint8_t* m_heightMap8B;
    };
    union
    {
        float* m_heightMap9F = nullptr;
        uint16_t* m_heightMap9S;
        uint8_t* m_heightMap9B;
    };
    G3D::Plane * m_minHeightPlanes = nullptr;
    // Height Data
    float m_tileHeight = INVALID_HEIGHT;
    float m_tileHeightMultiplier = 0;

    // Area Data
    uint16_t* m_areaMap = nullptr;

    // Liquid Data
    float m_liquidLevel = INVALID_HEIGHT;
    uint16_t* m_liquidEntry = nullptr;
    uint8_t* m_liquidFlags = nullptr;
    float* m_liquidMap = nullptr;
    uint16_t m_tileArea = 0;
    uint16_t m_liquidGlobalEntry = 0;
    uint8_t m_liquidGlobalFlags = 0;
    uint8_t m_liquidOffX = 0;
    uint8_t m_liquidOffY = 0;
    uint8_t m_liquidWidth = 0;
    uint8_t m_liquidHeight = 0;

    uint16_t* m_holes = nullptr;

    TileMap();
    ~TileMap();

    void load(char* filename);

    void loadAreaData(FILE* f, TileMapHeader& header);
    void loadHeightData(FILE* f, TileMapHeader& header);
    void loadLiquidData(FILE* f, TileMapHeader& header);
    void loadHolesData(FILE* f, TileMapHeader& header);
    bool isHole(int row, int column) const;

    typedef float (TileMap::* GetHeightPtr) (float x, float y) const;
    GetHeightPtr m_gridGetHeight;
    float getHeightFromFloat(float x, float y) const;
    float getHeightFromUint16(float x, float y) const;
    float getHeightFromUint8(float x, float y) const;
    float getHeightFromFlat(float x, float y) const;

    uint16_t getArea(float x, float y) const;
    float getHeight(float x, float y) const { return (this->*m_gridGetHeight)(x, y); }
    float getLiquidLevel(float x, float y) const;
    ZLiquidStatus getLiquidStatus(LocationVector pos, uint8_t ReqLiquidType, LiquidData* data = 0, float collisionHeight = 2.03128f);
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

        // Normal map stuff
        sprintf(filename, "%smaps/%04u_%02u_%02u.map", sWorld.settings.server.dataDir.c_str(), m_mapid, m_tx, m_ty);
        m_map.load(filename);
    }
};

class TerrainHolder
{
public:
    uint32_t m_mapid;
    TerrainTile* m_tiles[Map::Terrain::TilesCount][Map::Terrain::TilesCount];
    FastMutex m_lock[Map::Terrain::TilesCount][Map::Terrain::TilesCount];
    std::atomic<unsigned long> m_tilerefs[Map::Terrain::TilesCount][Map::Terrain::TilesCount];

    /// Our memory saving system for small allocations
    uint32_t TileCountX, TileCountY;
    uint32_t TileStartX, TileEndX;
    uint32_t TileStartY, TileEndY;

    /// This holds the offsets of the tile information for each tile.
    uint32_t TileOffsets[Map::Terrain::TilesCount][Map::Terrain::TilesCount];

    TerrainHolder(uint32_t mapid);
    ~TerrainHolder();

    TerrainTile* getTile(float x, float y);
    TerrainTile* getTile(int32_t tx, int32_t ty);

    void loadTile(float x, float y);
    void loadTile(int32_t tx, int32_t ty);

    void unloadTile(float x, float y);
    void unloadTile(int32_t tx, int32_t ty);

    void getCellLimits(uint32_t& StartX, uint32_t& EndX, uint32_t& StartY, uint32_t& EndY);

    bool areTilesValid(uint32_t x, uint32_t y)
    {
        if (x < TileStartX || x > TileEndX)
            return false;
        if (y < TileStartY || y > TileEndY)
            return false;
        return true;
    }

    bool tileLoaded(int32_t x, int32_t y)
    {
        if (m_tiles[x][y] != 0)
            return true;

        return false;
    }
};
