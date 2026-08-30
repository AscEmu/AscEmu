/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by //-V1042
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

#include "MMapManager.h"
#include "MapDefines.h"
#include "DetourCommon.h"
#include "DetourNavMeshBuilder.h"
#include "Debugging/Errors.hpp"
#include "Logging/Logger.hpp"
#include "Server/World.h"

#include <cfloat>
#include <cstdio>

namespace MMAP
{
    // ######################## MMapManager ########################
    MMapManager::~MMapManager()
    {
        loadedMMaps.clear();

        // by now we should not have maps loaded
        // if we had, tiles in MMapData->mmapLoadedTiles, their actual data is lost!
    }

    void MMapManager::InitializeThreadUnsafe(const std::vector<uint32_t>& mapIds)
    {
        // the caller must pass the list of all mapIds that will be used in the VMapManager2 lifetime
        for (const uint32_t& mapId : mapIds)
            loadedMMaps.insert(MMapDataSet::value_type(mapId, nullptr));

        thread_safe_environment = false;
    }

    MMapDataSet::const_iterator MMapManager::GetMMapData(uint32_t mapId) const
    {
        // return the iterator if found or end() if not found/NULL
        MMapDataSet::const_iterator itr = loadedMMaps.find(mapId);
        if (itr != loadedMMaps.cend() && !itr->second)
            itr = loadedMMaps.cend();

        return itr;
    }

    bool MMapManager::loadMapData(uint32_t mapId)
    {
        // we already have this map loaded?
        MMapDataSet::iterator itr = loadedMMaps.find(mapId);
        if (itr != loadedMMaps.end())
        {
            if (itr->second)
                return true;
        }
        else
        {
            if (thread_safe_environment)
                itr = loadedMMaps.insert(MMapDataSet::value_type(mapId, nullptr)).first;
            else
            {
                sLogger.failure("Invalid mapId {} passed to MMapManager after startup in thread unsafe environment", mapId);
                ASSERT(false);
            }
        }

        // load and init dtNavMesh - read parameters from file
        std::string dataDir = worldConfig.server.dataDir + "mmaps/";
        uint32_t pathLen = static_cast<uint32_t>(dataDir.length() + strlen("%04i.mmap") + 1);
        auto fileName = std::make_unique<char[]>(pathLen);
        snprintf(fileName.get(), pathLen, (dataDir + "%04i.mmap").c_str(), mapId);

        FILE* file = fopen(fileName.get(), "rb");
        if (!file)
        {
            sLogger.debug("MMAP:loadMapData: Error: Could not open mmap file '{}'", fileName.get());
            return false;
        }

        dtNavMeshParams params;
        int count = static_cast<int>(fread(&params, sizeof(dtNavMeshParams), 1, file));
        fclose(file);
        if (count != 1)
        {
            sLogger.failure("Error: Could not read params from file '{}'", fileName.get());
            return false;
        }

        dtNavMesh* mesh = dtAllocNavMesh();
        ASSERT(mesh);
        if (dtStatusFailed(mesh->init(&params)))
        {
            dtFreeNavMesh(mesh);
            sLogger.failure("Failed to initialize dtNavMesh for mmap {:04} from file {}", mapId, fileName.get());
            return false;
        }

        sLogger.debug("MMAP:loadMapData: Loaded {:04}.mmap", mapId);

        // store inside our map list
        itr->second = std::make_unique<MMapData>(mesh);
        itr->second->mmapLoadedTiles.clear();
        return true;
    }

    uint32_t MMapManager::packTileID(int32_t x, int32_t y)
    {
        return uint32_t(x << 16 | y);
    }

    bool MMapManager::loadMap(const std::string& basePath, uint32_t mapId, int32_t x, int32_t y)
    {
        // make sure the mmap is loaded and ready to load tiles
        if (!loadMapData(mapId))
            return false;

        // get this mmap data
        MMapData* mmap = loadedMMaps[mapId].get();
        ASSERT(mmap->navMesh);

        // check if we already have this tile loaded
        uint32_t packedGridPos = packTileID(x, y);
        if (mmap->mmapLoadedTiles.find(packedGridPos) != mmap->mmapLoadedTiles.end())
            return false;

        // load this tile :: /MMMXXYY.mmtile
        uint32_t pathLen = static_cast<uint32_t>(basePath.length() + strlen("/%04i%02i%02i.mmtile") + 1);
        auto fileName = std::make_unique<char[]>(pathLen);

        snprintf(fileName.get(), pathLen, (basePath + "/%04i%02i%02i.mmtile").c_str(), mapId, x, y);

        FILE* file = fopen(fileName.get(), "rb");
        if (!file)
        {
            sLogger.debug("Could not open mmtile file '{}'", fileName.get());
            return false;
        }

        // read header
        MmapTileHeader fileHeader;
        if (fread(&fileHeader, sizeof(MmapTileHeader), 1, file) != 1 || fileHeader.mmapMagic != MMAP_MAGIC)
        {
            sLogger.failure("Bad header in mmap {:04}{:02}{:02}.mmtile", mapId, x, y);
            fclose(file);
            return false;
        }

        if (fileHeader.mmapVersion != MMAP_VERSION)
        {
            sLogger.failure("{:04}{:02}{:02}.mmtile was built with generator v{}, expected v{}",
                mapId, x, y, fileHeader.mmapVersion, MMAP_VERSION);
            fclose(file);
            return false;
        }

        unsigned char* data = (unsigned char*)dtAlloc(fileHeader.size, DT_ALLOC_PERM);
        ASSERT(data);

        size_t result = fread(data, fileHeader.size, 1, file);
        if (!result)
        {
            sLogger.failure("Bad header or data in mmap {:04}{:02}{:02}.mmtile", mapId, x, y);
            fclose(file);
            return false;
        }

        fclose(file);

        dtMeshHeader* header = (dtMeshHeader*)data;
        dtTileRef tileRef = 0;

        // memory allocated for data is now managed by detour, and will be deallocated when the tile is removed
        if (dtStatusSucceed(mmap->navMesh->addTile(data, fileHeader.size, DT_TILE_FREE_DATA, 0, &tileRef)))
        {
            mmap->mmapLoadedTiles.insert(std::pair<uint32_t, dtTileRef>(packedGridPos, tileRef));
            ++loadedTiles;
            sLogger.debug("MMAP:loadMap: Loaded mmtile {:04}[{:02}, {:02}] into {:04}[{:02}, {:02}]", mapId, x, y, mapId, header->x, header->y);
            return true;
        }
        else
        {
            sLogger.debug("MMAP:loadMap: Could not load {:04}{:02}{:02}.mmtile into navmesh", mapId, x, y);
            dtFree(data);
            return false;
        }
    }

    bool MMapManager::unloadMap(uint32_t mapId, int32_t x, int32_t y)
    {
        // check if we have this map loaded
        MMapDataSet::const_iterator itr = GetMMapData(mapId);
        if (itr == loadedMMaps.end())
        {
            // file may not exist, therefore not loaded
            sLogger.debug("MMAP:unloadMap: Asked to unload not loaded navmesh map. {:04}{:02}{:02}.mmtile", mapId, x, y);
            return false;
        }

        MMapData* mmap = itr->second.get();

        // check if we have this tile loaded
        uint32_t packedGridPos = packTileID(x, y);
        if (mmap->mmapLoadedTiles.find(packedGridPos) == mmap->mmapLoadedTiles.end())
        {
            // file may not exist, therefore not loaded
            sLogger.debug("MMAP:unloadMap: Asked to unload not loaded navmesh tile. {:04}{:02}{:02}.mmtile", mapId, x, y);
            return false;
        }

        dtTileRef tileRef = mmap->mmapLoadedTiles[packedGridPos];

        // unload, and mark as non loaded
        if (dtStatusFailed(mmap->navMesh->removeTile(tileRef, nullptr, nullptr)))
        {
            // this is technically a memory leak
            // if the grid is later reloaded, dtNavMesh::addTile will return error but no extra memory is used
            // we cannot recover from this error - assert out
            sLogger.failure("Could not unload {:04}{:02}{:02}.mmtile from navmesh", mapId, x, y);
        }
        else
        {
            mmap->mmapLoadedTiles.erase(packedGridPos);
            --loadedTiles;

            // the real ground tile it linked into is gone - drop our
            // off-mesh-only layer at the same position too, freeing the
            // reserved slot back up rather than leaving it orphaned
            {
                std::lock_guard<std::mutex> lock(m_offMeshMutex);
                auto offMeshIt = mmap->runtimeOffMeshTiles.find(packedGridPos);
                if (offMeshIt != mmap->runtimeOffMeshTiles.end())
                {
                    mmap->navMesh->removeTile(offMeshIt->second, nullptr, nullptr);
                    mmap->runtimeOffMeshTiles.erase(offMeshIt);
                }
            }

            sLogger.debug("MMAP:unloadMap: Unloaded mmtile {:04}[{:02}, {:02}] from {:04}", mapId, x, y, mapId);
            return true;
        }

        return false;
    }

    bool MMapManager::unloadMap(uint32_t mapId)
    {
        MMapDataSet::iterator itr = loadedMMaps.find(mapId);
        if (itr == loadedMMaps.end() || !itr->second)
        {
            // file may not exist, therefore not loaded
            sLogger.debug("MMAP:unloadMap: Asked to unload not loaded navmesh map. {:04}.mmtile", mapId);
            return false;
        }

        // unload all tiles from given map
        MMapData* mmap = itr->second.get();
        for (MMapTileSet::iterator i = mmap->mmapLoadedTiles.begin(); i != mmap->mmapLoadedTiles.end(); ++i)
        {
            uint32_t x = (i->first >> 16);
            uint32_t y = (i->first & 0x0000FFFF);
            if (dtStatusFailed(mmap->navMesh->removeTile(i->second, nullptr, nullptr)))
                sLogger.failure("Could not unload {:04}{:02}{:02}.mmtile from navmesh", mapId, x, y);
            else
            {
                --loadedTiles;
                sLogger.debug("MMAP:unloadMap: Unloaded mmtile {:04}[{:02}, {:02}] from {:04}", mapId, x, y, mapId);
            }
        }

        itr->second = nullptr;
        sLogger.debug("MMAP:unloadMap: Unloaded {:04}.mmap", mapId);

        return true;
    }

    bool MMapManager::unloadMapInstance(uint32_t mapId, uint32_t instanceId)
    {
        // check if we have this map loaded
        MMapDataSet::const_iterator itr = GetMMapData(mapId);
        if (itr == loadedMMaps.end())
        {
            // file may not exist, therefore not loaded
            sLogger.debug("MMAP:unloadMapInstance: Asked to unload not loaded navmesh map {:04}", mapId);
            return false;
        }

        MMapData* mmap = itr->second.get();
        if (mmap->navMeshQueries.find(instanceId) == mmap->navMeshQueries.end())
        {
            sLogger.debug("MMAP:unloadMapInstance: Asked to unload not loaded dtNavMeshQuery mapId {:04} instanceId {}", mapId, instanceId);
            return false;
        }

        dtNavMeshQuery* query = mmap->navMeshQueries[instanceId];

        dtFreeNavMeshQuery(query);
        mmap->navMeshQueries.erase(instanceId);
        sLogger.debug("MMAP:unloadMapInstance: Unloaded mapId {:04} instanceId {}", mapId, instanceId);

        return true;
    }

    dtNavMesh const* MMapManager::GetNavMesh(uint32_t mapId)
    {
        MMapDataSet::const_iterator itr = GetMMapData(mapId);
        if (itr == loadedMMaps.end())
            return nullptr;

        return itr->second->navMesh;
    }

    dtNavMeshQuery const* MMapManager::GetNavMeshQuery(uint32_t mapId, uint32_t instanceId)
    {
        MMapDataSet::const_iterator itr = GetMMapData(mapId);
        if (itr == loadedMMaps.end())
            return nullptr;

        MMapData* mmap = itr->second.get();
        if (mmap->navMeshQueries.find(instanceId) == mmap->navMeshQueries.end())
        {
            // allocate mesh query
            dtNavMeshQuery* query = dtAllocNavMeshQuery();
            ASSERT(query);
            if (dtStatusFailed(query->init(mmap->navMesh, 1024)))
            {
                dtFreeNavMeshQuery(query);
                sLogger.failure("Failed to initialize dtNavMeshQuery for mapId {:04} instanceId {}", mapId, instanceId);
                return nullptr;
            }

            sLogger.debug("MMAP:GetNavMeshQuery: created dtNavMeshQuery for mapId {:04} instanceId {}", mapId, instanceId);
            mmap->navMeshQueries.insert(std::pair<uint32_t, dtNavMeshQuery*>(instanceId, query));
        }

        return mmap->navMeshQueries[instanceId];
    }

    // ######################## Runtime off-mesh connections ########################
    // See MMapManager.h for the design rationale. Coordinates entering this
    // section are in game (x, y, z) order and get swapped to Detour's
    // (y, z, x) convention wherever a raw Detour call needs it - mirroring
    // mmaps_generator's TerrainBuilder::loadOffMeshConnections(), which
    // does the exact same reorder for the on-disk --offMeshInput format
    // this feature reuses.

    uint64_t MMapManager::packMapTileID(uint32_t mapId, int32_t x, int32_t y)
    {
        return (uint64_t(mapId) << 32) | uint64_t(uint32_t(x) << 16 | uint32_t(y));
    }

    static std::string GetRuntimeOffMeshFilePath()
    {
        return worldConfig.server.dataDir + "mmaps/offmesh_runtime.txt";
    }

    void MMapManager::ensureRuntimeOffMeshFileCacheLoaded()
    {
        if (m_offMeshFileCacheLoaded)
            return;

        m_offMeshFileCacheLoaded = true;

        FILE* file = fopen(GetRuntimeOffMeshFilePath().c_str(), "r");
        if (!file)
            return; // nothing captured yet, in this session or a previous one - not an error

        char line[512];
        while (fgets(line, sizeof(line), file))
        {
            uint32_t mid, tx, ty;
            RuntimeOffMeshConnection conn;
            if (sscanf(line, "%u %u,%u (%f %f %f) (%f %f %f) %f", &mid, &tx, &ty,
                &conn.p0[0], &conn.p0[1], &conn.p0[2], &conn.p1[0], &conn.p1[1], &conn.p1[2], &conn.radius) != 10)
                continue;

            m_offMeshFileCache[packMapTileID(mid, int32_t(tx), int32_t(ty))].push_back(conn);
        }

        fclose(file);
    }

    void MMapManager::appendRuntimeOffMeshConnectionToFile(uint32_t mapId, int32_t x, int32_t y, const RuntimeOffMeshConnection& conn)
    {
        FILE* file = fopen(GetRuntimeOffMeshFilePath().c_str(), "a");
        if (!file)
        {
            sLogger.failure("MMAP:appendRuntimeOffMeshConnectionToFile: could not open '{}' for appending", GetRuntimeOffMeshFilePath());
            return;
        }

        fprintf(file, "%u %u,%u (%f %f %f) (%f %f %f) %f\n", mapId, uint32_t(x), uint32_t(y),
            conn.p0[0], conn.p0[1], conn.p0[2], conn.p1[0], conn.p1[1], conn.p1[2], conn.radius);
        fclose(file);
    }

    bool MMapManager::rebuildRuntimeOffMeshTile(MMapData* mmap, uint32_t mapId, int32_t x, int32_t y)
    {
        uint32_t packedGridPos = packTileID(x, y);

        // drop the previous version of this tile's off-mesh layer, if any -
        // dtCreateNavMeshData bakes a fixed connection list, so adding one
        // more connection means rebuilding the whole thing, not patching it
        auto oldTileIt = mmap->runtimeOffMeshTiles.find(packedGridPos);
        if (oldTileIt != mmap->runtimeOffMeshTiles.end())
        {
            mmap->navMesh->removeTile(oldTileIt->second, nullptr, nullptr);
            mmap->runtimeOffMeshTiles.erase(oldTileIt);
        }

        auto connIt = mmap->runtimeOffMeshConnections.find(packedGridPos);
        if (connIt == mmap->runtimeOffMeshConnections.end() || connIt->second.empty())
            return true; // nothing to (re)build - fine, not an error

        std::vector<RuntimeOffMeshConnection> const& connections = connIt->second;
        size_t count = connections.size();

        // Off-mesh connection arrays, in Detour (y, z, x) order.
        std::vector<float> offMeshVerts(count * 6);
        std::vector<float> offMeshRad(count);
        std::vector<unsigned char> offMeshDir(count);
        std::vector<unsigned char> offMeshAreas(count);
        std::vector<unsigned short> offMeshFlags(count);

        float bmin[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
        float bmax[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

        for (size_t i = 0; i < count; ++i)
        {
            RuntimeOffMeshConnection const& c = connections[i];
            float dp0[3] = { c.p0[1], c.p0[2], c.p0[0] };
            float dp1[3] = { c.p1[1], c.p1[2], c.p1[0] };

            for (int axis = 0; axis < 3; ++axis)
            {
                offMeshVerts[i * 6 + axis] = dp0[axis];
                offMeshVerts[i * 6 + 3 + axis] = dp1[axis];

                bmin[axis] = dtMin(bmin[axis], dtMin(dp0[axis], dp1[axis]));
                bmax[axis] = dtMax(bmax[axis], dtMax(dp0[axis], dp1[axis]));
            }

            offMeshRad[i] = c.radius;
            offMeshDir[i] = 1;      // bidirectional, same as mmaps_generator's file-based connections
            offMeshAreas[i] = 0xFF; // "can be used same way as polygon flags" - mirrors loadOffMeshConnections()
            offMeshFlags[i] = 0xFF; // all movement masks can make this path
        }

        // Pad the bounds generously, then place one tiny, harmless "ground"
        // triangle near the (now-known) minimum corner - dtCreateNavMeshData
        // hard-requires at least one real polygon (see its early
        // `if (!params->polyCount || !params->polys) return false;`), but
        // this poly's own flags are 0, which never matches any real
        // dtQueryFilter's include flags (see PathGenerator::createFilter),
        // so it's never selected by any actual pathfinding/LoS query - it
        // exists purely to satisfy that requirement.
        for (int axis = 0; axis < 3; ++axis)
        {
            bmin[axis] -= 5.0f;
            bmax[axis] += 5.0f;
        }

        constexpr float kCellSize = 0.5f;
        constexpr float kCellHeight = 0.2f;

        unsigned short dummyVerts[9] = {
            2, 2, 2,
            6, 2, 2,
            2, 2, 6
        };
        constexpr unsigned short kMeshNullIdx = 0xffff; // matches DetourNavMeshBuilder.cpp's private MESH_NULL_IDX
        unsigned short dummyPoly[6] = { 0, 1, 2, kMeshNullIdx, kMeshNullIdx, kMeshNullIdx };
        unsigned short dummyFlags[1] = { 0 };
        unsigned char dummyAreas[1] = { 0 };

        dtNavMeshCreateParams params;
        memset(&params, 0, sizeof(params));
        params.verts = dummyVerts;
        params.vertCount = 3;
        params.polys = dummyPoly;
        params.polyFlags = dummyFlags;
        params.polyAreas = dummyAreas;
        params.polyCount = 1;
        params.nvp = 3;

        params.offMeshConVerts = offMeshVerts.data();
        params.offMeshConRad = offMeshRad.data();
        params.offMeshConDir = offMeshDir.data();
        params.offMeshConAreas = offMeshAreas.data();
        params.offMeshConFlags = offMeshFlags.data();
        params.offMeshConCount = int(count);

        params.walkableHeight = 2.0f;
        params.walkableRadius = 0.6f;
        params.walkableClimb = 1.0f;
        params.tileX = x;
        params.tileY = y;
        params.tileLayer = 1; // extra layer, alongside the real terrain tile at layer 0
        dtVcopy(params.bmin, bmin);
        dtVcopy(params.bmax, bmax);
        params.cs = kCellSize;
        params.ch = kCellHeight;
        params.buildBvTree = true;

        unsigned char* data = nullptr;
        int dataSize = 0;
        if (!dtCreateNavMeshData(&params, &data, &dataSize))
        {
            sLogger.failure("MMAP:rebuildRuntimeOffMeshTile: dtCreateNavMeshData failed for {:04}[{:02},{:02}]", mapId, x, y);
            return false;
        }

        dtTileRef tileRef = 0;
        if (dtStatusFailed(mmap->navMesh->addTile(data, dataSize, DT_TILE_FREE_DATA, 0, &tileRef)))
        {
            sLogger.failure("MMAP:rebuildRuntimeOffMeshTile: addTile failed for {:04}[{:02},{:02}] "
                "(is mmaps_generator's reserved tile headroom present in this map's .mmap file?)", mapId, x, y);
            dtFree(data);
            return false;
        }

        mmap->runtimeOffMeshTiles[packedGridPos] = tileRef;
        return true;
    }

    bool MMapManager::addRuntimeOffMeshConnection(uint32_t mapId, const float* p0, const float* p1, float radius, std::string& outError)
    {
        std::lock_guard<std::mutex> lock(m_offMeshMutex);

        MMapDataSet::const_iterator itr = GetMMapData(mapId);
        if (itr == loadedMMaps.end())
        {
            outError = "No navmesh loaded for this map (pathfinding disabled, or map has no mmaps).";
            return false;
        }

        MMapData* mmap = itr->second.get();

        float detourP0[3] = { p0[1], p0[2], p0[0] };
        int32_t x = -1, y = -1;
        mmap->navMesh->calcTileLoc(detourP0, &x, &y);
        if (x < 0 || y < 0 || mmap->navMesh->getTileAt(x, y, 0) == nullptr)
        {
            outError = "Your current tile isn't loaded - stand somewhere with pathfinding active.";
            return false;
        }

        ensureRuntimeOffMeshFileCacheLoaded();

        RuntimeOffMeshConnection conn;
        memcpy(conn.p0, p0, sizeof(conn.p0));
        memcpy(conn.p1, p1, sizeof(conn.p1));
        conn.radius = radius;

        mmap->runtimeOffMeshConnections[packTileID(x, y)].push_back(conn);
        m_offMeshFileCache[packMapTileID(mapId, x, y)].push_back(conn);
        appendRuntimeOffMeshConnectionToFile(mapId, x, y, conn);

        if (!rebuildRuntimeOffMeshTile(mmap, mapId, x, y))
        {
            outError = "Captured and saved, but injecting it into the live navmesh failed - see server log.";
            return false;
        }

        return true;
    }

    void MMapManager::loadRuntimeOffMeshConnections(uint32_t mapId, int32_t x, int32_t y)
    {
        std::lock_guard<std::mutex> lock(m_offMeshMutex);

        MMapDataSet::const_iterator itr = GetMMapData(mapId);
        if (itr == loadedMMaps.end())
            return;

        MMapData* mmap = itr->second.get();

        ensureRuntimeOffMeshFileCacheLoaded();

        auto cacheIt = m_offMeshFileCache.find(packMapTileID(mapId, x, y));
        if (cacheIt == m_offMeshFileCache.end() || cacheIt->second.empty())
            return;

        auto& liveList = mmap->runtimeOffMeshConnections[packTileID(x, y)];
        liveList.insert(liveList.end(), cacheIt->second.begin(), cacheIt->second.end());

        rebuildRuntimeOffMeshTile(mmap, mapId, x, y);
    }
}
