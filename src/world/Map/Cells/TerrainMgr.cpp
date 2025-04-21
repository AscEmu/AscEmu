/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AEVersion.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "G3D/Plane.h"
#include "TerrainMgr.hpp"

#include "LocationVector.h"
#include "Storage/WDB/WDBStores.hpp"
#include "Logging/Logger.hpp"
#include "Macros/MapsMacros.hpp"
#include "Server/World.h"
#include "Storage/WDB/WDBStructures.hpp"

static uint16_t const holetab_h[4] = { 0x1111, 0x2222, 0x4444, 0x8888 };
static uint16_t const holetab_v[4] = { 0x000F, 0x00F0, 0x0F00, 0xF000 };

TileMap::TileMap()
{
    // Height level data
    m_gridGetHeight = &TileMap::getHeightFromFlat;
}

TileMap::~TileMap()
{
    delete[] m_minHeightPlanes;
}

void TileMap::load(char* filename)
{
    sLogger.debug("Loading {}", filename);
    FILE* f = fopen(filename, "rb");

    if (f == NULL)
    {
        sLogger.failure("{} does not exist", filename);
        return;
    }

    TileMapHeader header;

    if (fread(&header, sizeof(header), 1, f) != 1)
    {
        fclose(f);
        return;
    }

#if VERSION_STRING < Mop
    if (header.buildMagic != BUILD_VERSION)  // wow version
    {
        sLogger.failure("{}: from incorrect client (you: {} us: {})", filename, header.buildMagic, BUILD_VERSION);
        fclose(f);
        return;
    }
#endif

    if (header.areaMapOffset != 0)
        loadAreaData(f, header);

    if (header.heightMapOffset != 0)
        loadHeightData(f, header);

    if (header.liquidMapOffset != 0)
        loadLiquidData(f, header);

    if (header.holesSize != 0)
        loadHolesData(f, header);

    fclose(f);
}

void TileMap::loadAreaData(FILE* f, TileMapHeader& header)
{
    TileMapAreaHeader areaHeader;

    if (fseek(f, header.areaMapOffset, SEEK_SET) != 0)
        return;

    if (fread(&areaHeader, sizeof(areaHeader), 1, f) != 1)
        return;

    m_tileArea = areaHeader.gridArea;
    if (!(areaHeader.flags & MAP_AREA_NO_AREA))
    {
        m_areaMap = std::make_unique<uint16_t[]>(16 * 16);
        if (fread(m_areaMap.get(), sizeof(uint16_t), 16 * 16, f) != 16 * 16)
            return;
    }
}

void TileMap::loadHeightData(FILE* f, TileMapHeader& header)
{
    TileMapHeightHeader mapHeader;

    if (fseek(f, header.heightMapOffset, SEEK_SET) != 0)
        return;

    if (fread(&mapHeader, sizeof(mapHeader), 1, f) != 1)
        return;

    m_tileHeight = mapHeader.gridHeight;
    m_heightMapFlags = mapHeader.flags;

    if (!(m_heightMapFlags & MAP_HEIGHT_NO_HEIGHT))
    {
        if (m_heightMapFlags & MAP_HEIGHT_AS_INT16)
        {
            m_tileHeightMultiplier = (mapHeader.gridMaxHeight - mapHeader.gridHeight) / 65535;
            m_gridGetHeight = &TileMap::getHeightFromUint16;

            m_heightMap9S = std::make_unique<uint16_t[]>(129 * 129);
            m_heightMap8S = std::make_unique<uint16_t[]>(128 * 128);
            if (fread(m_heightMap9S.get(), sizeof(uint16_t), 129 * 129, f) != 129 * 129 ||
                fread(m_heightMap8S.get(), sizeof(uint16_t), 128 * 128, f) != 128 * 128)
                return;
        }
        else if (m_heightMapFlags & MAP_HEIGHT_AS_INT8)
        {
            m_tileHeightMultiplier = (mapHeader.gridMaxHeight - mapHeader.gridHeight) / 255;
            m_gridGetHeight = &TileMap::getHeightFromUint8;

            m_heightMap9B = std::make_unique<uint8_t[]>(129 * 129);
            m_heightMap8B = std::make_unique<uint8_t[]>(128 * 128);
            if (fread(m_heightMap9B.get(), sizeof(uint8_t), 129 * 129, f) != 129 * 129 ||
                fread(m_heightMap8B.get(), sizeof(uint8_t), 128 * 128, f) != 128 * 128)
                return;
        }
        else
        {
            m_heightMap9F = std::make_unique<float[]>(129 * 129);
            m_heightMap8F = std::make_unique<float[]>(128 * 128);
            if (fread(m_heightMap9F.get(), sizeof(float), 129 * 129, f) != 129 * 129 ||
                fread(m_heightMap8F.get(), sizeof(float), 128 * 128, f) != 128 * 128)
                return;

            m_gridGetHeight = &TileMap::getHeightFromFloat;
        }
    }
    else
    {
        m_gridGetHeight = &TileMap::getHeightFromFlat;
    }
}

void TileMap::loadLiquidData(FILE* f, TileMapHeader& header)
{
    TileMapLiquidHeader liquidHeader;

    if (fseek(f, header.liquidMapOffset, SEEK_SET) != 0)
        return;

    if (fread(&liquidHeader, sizeof(liquidHeader), 1, f) != 1)
        return;

    m_liquidGlobalEntry = liquidHeader.liquidType;
    m_liquidGlobalFlags = liquidHeader.liquidFlags;
    m_liquidOffX = liquidHeader.offsetX;
    m_liquidOffY = liquidHeader.offsetY;
    m_liquidWidth = liquidHeader.width;
    m_liquidHeight = liquidHeader.height;
    m_liquidLevel = liquidHeader.liquidLevel;

    if (!(liquidHeader.flags & MAP_LIQUID_NO_TYPE))
    {
        m_liquidEntry = std::make_unique<uint16_t[]>(16 * 16);
        if (fread(m_liquidEntry.get(), sizeof(uint16_t), 16 * 16, f) != 16 * 16)
            return;

        m_liquidFlags = std::make_unique<uint8_t[]>(16 * 16);
        if (fread(m_liquidFlags.get(), sizeof(uint8_t), 16 * 16, f) != 16 * 16)
            return;
    }

    if (!(liquidHeader.flags & MAP_LIQUID_NO_HEIGHT))
    {
        m_liquidMap = std::make_unique<float[]>(m_liquidWidth * m_liquidHeight);
        if (fread(m_liquidMap.get(), sizeof(float), m_liquidWidth * m_liquidHeight, f) != 16 * 16)
            return;
    }
}

void TileMap::loadHolesData(FILE* in, TileMapHeader& header)
{
    if (fseek(in, header.holesOffset, SEEK_SET) != 0)
        return;

    m_holes = std::make_unique<uint16_t[]>(16 * 16);
    if (fread(m_holes.get(), sizeof(uint16_t), 16 * 16, in) != 16 * 16)
        return;
}

bool TileMap::isHole(int row, int col) const
{
    if (!m_holes)
        return false;

    int cellRow = row / 8;     // 8 squares per cell
    int cellCol = col / 8;
    int holeRow = row % 8 / 2;
    int holeCol = (col - (cellCol * 8)) / 2;

    uint16_t hole = m_holes[cellRow * 16 + cellCol];

    return (hole & holetab_h[holeCol] & holetab_v[holeRow]) != 0;
}

float TileMap::getHeightFromFlat(float /*x*/, float /*y*/) const
{
    return m_tileHeight;
}

float TileMap::getHeightFromFloat(float x, float y) const
{
    if (!m_heightMap8F || !m_heightMap9F)
        return m_tileHeight;

    x = Map::Terrain::MapResoloution * (Map::Terrain::MapCenter - x / Map::Terrain::TileSize);
    y = Map::Terrain::MapResoloution * (Map::Terrain::MapCenter - y / Map::Terrain::TileSize);

    int x_int = static_cast<int>(x);
    int y_int = static_cast<int>(y);
    x -= x_int;
    y -= y_int;
    x_int &= (Map::Terrain::MapResoloution - 1);
    y_int &= (Map::Terrain::MapResoloution - 1);

    if (isHole(x_int, y_int))
        return INVALID_HEIGHT;

    // Height stored as: h5 - its v8 grid, h1-h4 - its v9 grid
    // +--------------> X
    // | h1-------h2     Coordinates is:
    // | | \  1  / |     h1 0, 0
    // | |  \   /  |     h2 0, 1
    // | | 2  h5 3 |     h3 1, 0
    // | |  /   \  |     h4 1, 1
    // | | /  4  \ |     h5 1/2, 1/2
    // | h3-------h4
    // V Y
    // For find height need
    // 1 - detect triangle
    // 2 - solve linear equation from triangle points
    // Calculate coefficients for solve h = a*x + b*y + c

    float a, b, c;
    // Select triangle:
    if (x + y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            float h1 = m_heightMap9F[(x_int) * 129 + y_int];
            float h2 = m_heightMap9F[(x_int + 1) * 129 + y_int];
            float h5 = 2 * m_heightMap8F[x_int * 128 + y_int];
            a = h2 - h1;
            b = h5 - h1 - h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            float h1 = m_heightMap9F[x_int * 129 + y_int];
            float h3 = m_heightMap9F[x_int * 129 + y_int + 1];
            float h5 = 2 * m_heightMap8F[x_int * 128 + y_int];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if (x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            float h2 = m_heightMap9F[(x_int + 1) * 129 + y_int];
            float h4 = m_heightMap9F[(x_int + 1) * 129 + y_int + 1];
            float h5 = 2 * m_heightMap8F[x_int * 128 + y_int];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            float h3 = m_heightMap9F[(x_int) * 129 + y_int + 1];
            float h4 = m_heightMap9F[(x_int + 1) * 129 + y_int + 1];
            float h5 = 2 * m_heightMap8F[x_int * 128 + y_int];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }
    // Calculate height
    return a * x + b * y + c;
}

float TileMap::getHeightFromUint8(float x, float y) const
{
    if (!m_heightMap8B || !m_heightMap9B)
        return m_tileHeight;

    x = Map::Terrain::MapResoloution * (Map::Terrain::MapCenter - x / Map::Terrain::TileSize);
    y = Map::Terrain::MapResoloution * (Map::Terrain::MapCenter - y / Map::Terrain::TileSize);

    int x_int = static_cast<int>(x);
    int y_int = static_cast<int>(y);
    x -= x_int;
    y -= y_int;
    x_int &= (Map::Terrain::MapResoloution - 1);
    y_int &= (Map::Terrain::MapResoloution - 1);

    if (isHole(x_int, y_int))
        return INVALID_HEIGHT;

    int32_t a, b, c;
    uint8_t* V9_h1_ptr = &m_heightMap9B[x_int * 128 + x_int + y_int];
    if (x + y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            int32_t h1 = V9_h1_ptr[0];
            int32_t h2 = V9_h1_ptr[129];
            int32_t h5 = 2 * m_heightMap8B[x_int * 128 + y_int];
            a = h2 - h1;
            b = h5 - h1 - h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            int32_t h1 = V9_h1_ptr[0];
            int32_t h3 = V9_h1_ptr[1];
            int32_t h5 = 2 * m_heightMap8B[x_int * 128 + y_int];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if (x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            int32_t h2 = V9_h1_ptr[129];
            int32_t h4 = V9_h1_ptr[130];
            int32_t h5 = 2 * m_heightMap8B[x_int * 128 + y_int];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            int32_t h3 = V9_h1_ptr[1];
            int32_t h4 = V9_h1_ptr[130];
            int32_t h5 = 2 * m_heightMap8B[x_int * 128 + y_int];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }
    // Calculate height
    return static_cast<float>((a * x) + (b * y) + c) * m_tileHeightMultiplier + m_tileHeight;
}

float TileMap::getHeightFromUint16(float x, float y) const
{
    if (!m_heightMap8S || !m_heightMap9S)
        return m_tileHeight;

    x = Map::Terrain::MapResoloution * (Map::Terrain::MapCenter - x / Map::Terrain::TileSize);
    y = Map::Terrain::MapResoloution * (Map::Terrain::MapCenter - y / Map::Terrain::TileSize);

    int x_int = static_cast<int>(x);
    int y_int = static_cast<int>(y);
    x -= x_int;
    y -= y_int;
    x_int &= (Map::Terrain::MapResoloution - 1);
    y_int &= (Map::Terrain::MapResoloution - 1);

    if (isHole(x_int, y_int))
        return INVALID_HEIGHT;

    int32_t a, b, c;
    uint16_t* V9_h1_ptr = &m_heightMap9S[x_int * 128 + x_int + y_int];
    if (x + y < 1)
    {
        if (x > y)
        {
            // 1 triangle (h1, h2, h5 points)
            int32_t h1 = V9_h1_ptr[0];
            int32_t h2 = V9_h1_ptr[129];
            int32_t h5 = 2 * m_heightMap8S[x_int * 128 + y_int];
            a = h2 - h1;
            b = h5 - h1 - h2;
            c = h1;
        }
        else
        {
            // 2 triangle (h1, h3, h5 points)
            int32_t h1 = V9_h1_ptr[0];
            int32_t h3 = V9_h1_ptr[1];
            int32_t h5 = 2 * m_heightMap8S[x_int * 128 + y_int];
            a = h5 - h1 - h3;
            b = h3 - h1;
            c = h1;
        }
    }
    else
    {
        if (x > y)
        {
            // 3 triangle (h2, h4, h5 points)
            int32_t h2 = V9_h1_ptr[129];
            int32_t h4 = V9_h1_ptr[130];
            int32_t h5 = 2 * m_heightMap8S[x_int * 128 + y_int];
            a = h2 + h4 - h5;
            b = h4 - h2;
            c = h5 - h4;
        }
        else
        {
            // 4 triangle (h3, h4, h5 points)
            int32_t h3 = V9_h1_ptr[1];
            int32_t h4 = V9_h1_ptr[130];
            int32_t h5 = 2 * m_heightMap8S[x_int * 128 + y_int];
            a = h4 - h3;
            b = h3 + h4 - h5;
            c = h5 - h4;
        }
    }
    // Calculate height
    return static_cast<float>((a * x) + (b * y) + c) * m_tileHeightMultiplier + m_tileHeight;
}

uint16_t TileMap::getArea(float x, float y) const
{
    if (!m_areaMap)
        return m_tileArea;

    x = 16 * (Map::Terrain::MapCenter - x / Map::Terrain::TileSize);
    y = 16 * (Map::Terrain::MapCenter - y / Map::Terrain::TileSize);
    int lx = static_cast<int>(x) & 15;
    int ly = static_cast<int>(y) & 15;
    return m_areaMap[lx * 16 + ly];
}

float TileMap::getLiquidLevel(float x, float y) const
{
    if (!m_liquidMap)
        return m_liquidLevel;

    x = Map::Terrain::MapResoloution * (Map::Terrain::MapCenter - x / Map::Terrain::TileSize);
    y = Map::Terrain::MapResoloution * (Map::Terrain::MapCenter - y / Map::Terrain::TileSize);

    int cx_int = (static_cast<int>(x) & (Map::Terrain::MapResoloution - 1)) - m_liquidOffY;
    int cy_int = (static_cast<int>(y) & (Map::Terrain::MapResoloution - 1)) - m_liquidOffX;

    if (cx_int < 0 || cx_int >= m_liquidHeight)
        return INVALID_HEIGHT;
    if (cy_int < 0 || cy_int >= m_liquidWidth)
        return INVALID_HEIGHT;

    return m_liquidMap[cx_int * m_liquidWidth + cy_int];
}

// Get water state on map
ZLiquidStatus TileMap::getLiquidStatus(LocationVector pos, uint8_t ReqLiquidType, LiquidData* data, float collisionHeight)
{
    // Check water type (if no water return)
    if (!m_liquidGlobalFlags && !m_liquidFlags)
        return LIQUID_MAP_NO_WATER;

    // Get cell
    float cx = Map::Terrain::MapResoloution * (Map::Terrain::MapCenter - pos.x / Map::Terrain::TileSize);
    float cy = Map::Terrain::MapResoloution * (Map::Terrain::MapCenter - pos.y / Map::Terrain::TileSize);

    int x_int = static_cast<int>(cx) & (Map::Terrain::MapResoloution - 1);
    int y_int = static_cast<int>(cy) & (Map::Terrain::MapResoloution - 1);

    // Check water type in cell
    int idx = (x_int >> 3) * 16 + (y_int >> 3);
    uint8_t type = m_liquidFlags ? m_liquidFlags[idx] : m_liquidGlobalFlags;
    uint32_t entry = m_liquidEntry ? m_liquidEntry[idx] : m_liquidGlobalEntry;
    if (WDB::Structures::LiquidTypeEntry const* liquidEntry = sLiquidTypeStore.lookupEntry(entry))
    {
        type &= MAP_LIQUID_TYPE_DARK_WATER;
        uint32_t liqTypeIdx = liquidEntry->Type;
        if (entry < 21)
        {
            if (WDB::Structures::AreaTableEntry const* area = sAreaStore.lookupEntry(getArea(pos.x, pos.y)))
            {
#if VERSION_STRING > Classic
                uint32_t overrideLiquid = area->liquid_type_override[liquidEntry->Type];
                if (!overrideLiquid && area->zone)
                {
                    area = MapManagement::AreaManagement::AreaStorage::GetAreaById(area->zone);
                    if (area)
                        overrideLiquid = area->liquid_type_override[liquidEntry->Type];
                }
#else
                uint32_t overrideLiquid = 0;
#endif

                if (WDB::Structures::LiquidTypeEntry const* liq = sLiquidTypeStore.lookupEntry(overrideLiquid))
                {
                    entry = overrideLiquid;
                    liqTypeIdx = liq->Type;
                }
            }
        }

        type |= 1 << liqTypeIdx;
    }

    if (type == 0)
        return LIQUID_MAP_NO_WATER;

    // Check req liquid type mask
    if (ReqLiquidType && !(ReqLiquidType & type))
        return LIQUID_MAP_NO_WATER;

    // Check water level:
    // Check water height map
    int lx_int = x_int - m_liquidOffY;
    int ly_int = y_int - m_liquidOffX;
    if (lx_int < 0 || lx_int >= m_liquidHeight)
        return LIQUID_MAP_NO_WATER;
    if (ly_int < 0 || ly_int >= m_liquidWidth)
        return LIQUID_MAP_NO_WATER;

    // Get water level
    float liquid_level = m_liquidMap ? m_liquidMap[lx_int * m_liquidWidth + ly_int] : m_liquidLevel;
    // Get ground level (sub 0.2 for fix some errors)
    float ground_level = getHeight(pos.x, pos.y);

    // Check water level and ground level
    if (liquid_level < ground_level || pos.z < ground_level)
        return LIQUID_MAP_NO_WATER;

    // All ok in water -> store data
    if (data)
    {
        data->entry = entry;
        data->type_flags = type;
        data->level = liquid_level;
        data->depth_level = ground_level;
    }

    // For speed check as int values
    float delta = liquid_level - pos.z;

    if (delta > collisionHeight)        // Under water
        return LIQUID_MAP_UNDER_WATER;
    if (delta > 0.0f)                   // In water
        return LIQUID_MAP_IN_WATER;
    if (delta > -0.1f)                  // Walk on water
        return LIQUID_MAP_WATER_WALK;
    // Above water
    return LIQUID_MAP_ABOVE_WATER;
}

TerrainTile::~TerrainTile() = default;

TerrainTile::TerrainTile(TerrainHolder* parent, uint32_t mapid, int32_t x, int32_t y)
{
    m_parent = parent;
    m_mapid = mapid;
    m_tx = x;
    m_ty = y;
}

void TerrainTile::Load()
{
    char filename[1024];

    // Normal map stuff
    sprintf(filename, "%smaps/%04u_%02u_%02u.map", sWorld.settings.server.dataDir.c_str(), m_mapid, m_tx, m_ty);
    m_map.load(filename);
}

TerrainHolder::TerrainHolder(uint32_t mapid)
{
    TileCountX = TileCountY = 0;
    TileStartX = TileEndX = 0;
    TileStartY = TileEndY = 0;

    for (uint8_t i = 0; i < Map::Terrain::TilesCount; ++i)
    {
        for (uint8_t j = 0; j < Map::Terrain::TilesCount; ++j)
        {
            m_tiles[i][j] = nullptr;
            if (TileOffsets[i][j])
            {
                if (!TileStartX || TileStartX > i)
                    TileStartX = i;
                if (!TileStartY || TileStartY > j)
                    TileStartY = j;
                if (i > TileEndX)
                    TileEndX = i;
                if (j > TileEndY)
                    TileEndY = j;
            }
        }
    }

    m_mapid = mapid;
    TileCountX = (TileEndX - TileStartX) + 1;
    TileCountY = (TileEndY - TileStartY) + 1;
}

TerrainHolder::~TerrainHolder()
{
    for (uint8_t i = 0; i < Map::Terrain::TilesCount; ++i)
        for (uint8_t j = 0; j < Map::Terrain::TilesCount; ++j)
            unloadTile(i, j);
}

TerrainTile* TerrainHolder::getTile(float x, float y)
{
    int32_t tx = static_cast<int32_t>(Map::Terrain::MapCenter - (x / Map::Terrain::TileSize));
    int32_t ty = static_cast<int32_t>(Map::Terrain::MapCenter - (y / Map::Terrain::TileSize));

    return getTile(tx, ty);
}

TerrainTile* TerrainHolder::getTile(int32_t tx, int32_t ty)
{
    std::lock_guard lock(m_lock[tx][ty]);
    return m_tiles[tx][ty].get();
}

void TerrainHolder::loadTile(float x, float y)
{
    int32_t tx = static_cast<int32_t>(Map::Terrain::MapCenter - (x / Map::Terrain::TileSize));
    int32_t ty = static_cast<int32_t>(Map::Terrain::MapCenter - (y / Map::Terrain::TileSize));
    loadTile(tx, ty);
}

void TerrainHolder::loadTile(int32_t tx, int32_t ty)
{
    std::lock_guard lock(m_lock[tx][ty]);

    ++m_tilerefs[tx][ty];
    if (m_tiles[tx][ty] == nullptr)
    {
        m_tiles[tx][ty] = std::make_unique<TerrainTile>(this, m_mapid, tx, ty);
        m_tiles[tx][ty]->Load();
    }
}

void TerrainHolder::unloadTile(float x, float y)
{
    int32_t tx = static_cast<int32_t>(Map::Terrain::MapCenter - (x / Map::Terrain::TileSize));
    int32_t ty = static_cast<int32_t>(Map::Terrain::MapCenter - (y / Map::Terrain::TileSize));
    unloadTile(tx, ty);
}

void TerrainHolder::unloadTile(int32_t tx, int32_t ty)
{
    std::lock_guard lock(m_lock[tx][ty]);

    if (m_tiles[tx][ty] == nullptr)
        return;

    if (--m_tilerefs[tx][ty] == 0)
        m_tiles[tx][ty] = nullptr;
}

void TerrainHolder::getCellLimits(uint32_t& StartX, uint32_t& EndX, uint32_t& StartY, uint32_t& EndY)
{
    StartX = TileStartX * 8;
    StartY = TileStartY * 8;
    EndX = TileEndX * 8;
    EndY = TileEndY * 8;
}
