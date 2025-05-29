/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
#include "Debugging/Errors.h"
#include "Logging/Logger.hpp"
#include "Server/World.h"

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
}
