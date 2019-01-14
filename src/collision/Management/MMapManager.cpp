/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

#include "MMapManager.h"
#include "MapDefines.h"
#include "StdAfx.h"
#include "Errors.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"

namespace MMAP
{
    // ######################## MMapManager ########################
    MMapManager::~MMapManager()
    {
        for (MMapDataSet::iterator i = loadedMMaps.begin(); i != loadedMMaps.end(); ++i)
            delete i->second;

        // by now we should not have maps loaded
        // if we had, tiles in MMapData->mmapLoadedTiles, their actual data is lost!
    }

    void MMapManager::InitializeThreadUnsafe(const std::vector<uint32>& mapIds)
    {
        // the caller must pass the list of all mapIds that will be used in the VMapManager2 lifetime
        for (const uint32& mapId : mapIds)
            loadedMMaps.insert(MMapDataSet::value_type(mapId, nullptr));

        thread_safe_environment = false;
    }

    MMapDataSet::const_iterator MMapManager::GetMMapData(uint32 mapId) const
    {
        // return the iterator if found or end() if not found/NULL
        MMapDataSet::const_iterator itr = loadedMMaps.find(mapId);
        if (itr != loadedMMaps.cend() && !itr->second)
            itr = loadedMMaps.cend();

        return itr;
    }

    bool MMapManager::loadMapData(uint32 mapId)
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
                LOG_ERROR("Invalid mapId %u passed to MMapManager after startup in thread unsafe environment", mapId);
                ASSERT(false);
            }
        }

        // load and init dtNavMesh - read parameters from file
        std::string dataDir = worldConfig.server.dataDir + "mmaps/";
        uint32 pathLen = static_cast<uint32>(dataDir.length() + strlen("%04i.mmap") + 1);
        char *fileName = new char[pathLen];
        snprintf(fileName, pathLen, (dataDir + "%04i.mmap").c_str(), mapId);

        FILE* file = fopen(fileName, "rb");
        if (!file)
        {
            LogDebugFlag(LF_MMAP, "MMAP:loadMapData: Error: Could not open mmap file '%s'", fileName);
            delete [] fileName;
            return false;
        }

        dtNavMeshParams params;
        int count = static_cast<int>(fread(&params, sizeof(dtNavMeshParams), 1, file));
        fclose(file);
        if (count != 1)
        {
            LOG_ERROR("Error: Could not read params from file '%s'", fileName);
            delete [] fileName;
            return false;
        }

        dtNavMesh* mesh = dtAllocNavMesh();
        ASSERT(mesh);
        if (dtStatusFailed(mesh->init(&params)))
        {
            dtFreeNavMesh(mesh);
            LOG_ERROR("Failed to initialize dtNavMesh for mmap %04u from file %s", mapId, fileName);
            delete [] fileName;
            return false;
        }

        delete [] fileName;

        LogDebugFlag(LF_MMAP, "MMAP:loadMapData: Loaded %04i.mmap", mapId);

        // store inside our map list
        MMapData* mmap_data = new MMapData(mesh);
        mmap_data->mmapLoadedTiles.clear();

        itr->second = mmap_data;
        return true;
    }

    uint32 MMapManager::packTileID(int32 x, int32 y)
    {
        return uint32(x << 16 | y);
    }

    bool MMapManager::loadMap(const std::string& basePath, uint32 mapId, int32 x, int32 y)
    {
        // make sure the mmap is loaded and ready to load tiles
        if (!loadMapData(mapId))
            return false;

        // get this mmap data
        MMapData* mmap = loadedMMaps[mapId];
        ASSERT(mmap->navMesh);

        // check if we already have this tile loaded
        uint32 packedGridPos = packTileID(x, y);
        if (mmap->mmapLoadedTiles.find(packedGridPos) != mmap->mmapLoadedTiles.end())
            return false;

        // load this tile :: /MMMXXYY.mmtile
        uint32 pathLen = static_cast<uint32>(basePath.length() + strlen("/%04i%02i%02i.mmtile") + 1);
        char *fileName = new char[pathLen];

        snprintf(fileName, pathLen, (basePath + "/%04i%02i%02i.mmtile").c_str(), mapId, x, y);

        FILE* file = fopen(fileName, "rb");
        if (!file)
        {
            LOG_DEBUG("Could not open mmtile file '%s'", fileName);
            delete [] fileName;
            return false;
        }
        delete [] fileName;

        // read header
        MmapTileHeader fileHeader;
        if (fread(&fileHeader, sizeof(MmapTileHeader), 1, file) != 1 || fileHeader.mmapMagic != MMAP_MAGIC)
        {
            LOG_ERROR("Bad header in mmap %04u%02i%02i.mmtile", mapId, x, y);
            fclose(file);
            return false;
        }

        if (fileHeader.mmapVersion != MMAP_VERSION)
        {
            LOG_ERROR("%04u%02i%02i.mmtile was built with generator v%i, expected v%i",
                mapId, x, y, fileHeader.mmapVersion, MMAP_VERSION);
            fclose(file);
            return false;
        }

        unsigned char* data = (unsigned char*)dtAlloc(fileHeader.size, DT_ALLOC_PERM);
        ASSERT(data);

        size_t result = fread(data, fileHeader.size, 1, file);
        if (!result)
        {
            LOG_ERROR("Bad header or data in mmap %04u%02i%02i.mmtile", mapId, x, y);
            fclose(file);
            return false;
        }

        fclose(file);

        dtMeshHeader* header = (dtMeshHeader*)data;
        dtTileRef tileRef = 0;

        // memory allocated for data is now managed by detour, and will be deallocated when the tile is removed
        if (dtStatusSucceed(mmap->navMesh->addTile(data, fileHeader.size, DT_TILE_FREE_DATA, 0, &tileRef)))
        {
            mmap->mmapLoadedTiles.insert(std::pair<uint32, dtTileRef>(packedGridPos, tileRef));
            ++loadedTiles;
            LogDebugFlag(LF_MMAP, "MMAP:loadMap: Loaded mmtile %04i[%02i, %02i] into %04i[%02i, %02i]", mapId, x, y, mapId, header->x, header->y);
            return true;
        }
        else
        {
            LogDebugFlag(LF_MMAP, "MMAP:loadMap: Could not load %04u%02i%02i.mmtile into navmesh", mapId, x, y);
            dtFree(data);
            return false;
        }
    }

    bool MMapManager::unloadMap(uint32 mapId, int32 x, int32 y)
    {
        // check if we have this map loaded
        MMapDataSet::const_iterator itr = GetMMapData(mapId);
        if (itr == loadedMMaps.end())
        {
            // file may not exist, therefore not loaded
            LogDebugFlag(LF_MMAP, "MMAP:unloadMap: Asked to unload not loaded navmesh map. %04u%02i%02i.mmtile", mapId, x, y);
            return false;
        }

        MMapData* mmap = itr->second;

        // check if we have this tile loaded
        uint32 packedGridPos = packTileID(x, y);
        if (mmap->mmapLoadedTiles.find(packedGridPos) == mmap->mmapLoadedTiles.end())
        {
            // file may not exist, therefore not loaded
            LogDebugFlag(LF_MMAP, "MMAP:unloadMap: Asked to unload not loaded navmesh tile. %04u%02i%02i.mmtile", mapId, x, y);
            return false;
        }

        dtTileRef tileRef = mmap->mmapLoadedTiles[packedGridPos];

        // unload, and mark as non loaded
        if (dtStatusFailed(mmap->navMesh->removeTile(tileRef, nullptr, nullptr)))
        {
            // this is technically a memory leak
            // if the grid is later reloaded, dtNavMesh::addTile will return error but no extra memory is used
            // we cannot recover from this error - assert out
            LOG_ERROR("Could not unload %04u%02i%02i.mmtile from navmesh", mapId, x, y);
        }
        else
        {
            mmap->mmapLoadedTiles.erase(packedGridPos);
            --loadedTiles;
            LogDebugFlag(LF_MMAP, "MMAP:unloadMap: Unloaded mmtile %04i[%02i, %02i] from %04i", mapId, x, y, mapId);
            return true;
        }

        return false;
    }

    bool MMapManager::unloadMap(uint32 mapId)
    {
        MMapDataSet::iterator itr = loadedMMaps.find(mapId);
        if (itr == loadedMMaps.end() || !itr->second)
        {
            // file may not exist, therefore not loaded
            LogDebugFlag(LF_MMAP, "MMAP:unloadMap: Asked to unload not loaded navmesh map. %04u%02i%02i.mmtile", mapId);
            return false;
        }

        // unload all tiles from given map
        MMapData* mmap = itr->second;
        for (MMapTileSet::iterator i = mmap->mmapLoadedTiles.begin(); i != mmap->mmapLoadedTiles.end(); ++i)
        {
            uint32 x = (i->first >> 16);
            uint32 y = (i->first & 0x0000FFFF);
            if (dtStatusFailed(mmap->navMesh->removeTile(i->second, nullptr, nullptr)))
                LOG_ERROR("Could not unload %04u%02i%02i.mmtile from navmesh", mapId, x, y);
            else
            {
                --loadedTiles;
                LogDebugFlag(LF_MMAP, "MMAP:unloadMap: Unloaded mmtile %04i[%02i, %02i] from %04i", mapId, x, y, mapId);
            }
        }

        delete mmap;
        itr->second = nullptr;
        LogDebugFlag(LF_MMAP, "MMAP:unloadMap: Unloaded %04i.mmap", mapId);

        return true;
    }

    bool MMapManager::unloadMapInstance(uint32 mapId, uint32 instanceId)
    {
        // check if we have this map loaded
        MMapDataSet::const_iterator itr = GetMMapData(mapId);
        if (itr == loadedMMaps.end())
        {
            // file may not exist, therefore not loaded
            LogDebugFlag(LF_MMAP, "MMAP:unloadMapInstance: Asked to unload not loaded navmesh map %04u", mapId);
            return false;
        }

        MMapData* mmap = itr->second;
        if (mmap->navMeshQueries.find(instanceId) == mmap->navMeshQueries.end())
        {
            LogDebugFlag(LF_MMAP, "MMAP:unloadMapInstance: Asked to unload not loaded dtNavMeshQuery mapId %04u instanceId %u", mapId, instanceId);
            return false;
        }

        dtNavMeshQuery* query = mmap->navMeshQueries[instanceId];

        dtFreeNavMeshQuery(query);
        mmap->navMeshQueries.erase(instanceId);
        LogDebugFlag(LF_MMAP, "MMAP:unloadMapInstance: Unloaded mapId %04u instanceId %u", mapId, instanceId);

        return true;
    }

    dtNavMesh const* MMapManager::GetNavMesh(uint32 mapId)
    {
        MMapDataSet::const_iterator itr = GetMMapData(mapId);
        if (itr == loadedMMaps.end())
            return nullptr;

        return itr->second->navMesh;
    }

    dtNavMeshQuery const* MMapManager::GetNavMeshQuery(uint32 mapId, uint32 instanceId)
    {
        MMapDataSet::const_iterator itr = GetMMapData(mapId);
        if (itr == loadedMMaps.end())
            return nullptr;

        MMapData* mmap = itr->second;
        if (mmap->navMeshQueries.find(instanceId) == mmap->navMeshQueries.end())
        {
            // allocate mesh query
            dtNavMeshQuery* query = dtAllocNavMeshQuery();
            ASSERT(query);
            if (dtStatusFailed(query->init(mmap->navMesh, 1024)))
            {
                dtFreeNavMeshQuery(query);
                LOG_ERROR("Failed to initialize dtNavMeshQuery for mapId %04u instanceId %u", mapId, instanceId);
                return nullptr;
            }

            LogDebugFlag(LF_MMAP, "MMAP:GetNavMeshQuery: created dtNavMeshQuery for mapId %04u instanceId %u", mapId, instanceId);
            mmap->navMeshQueries.insert(std::pair<uint32, dtNavMeshQuery*>(instanceId, query));
        }

        return mmap->navMeshQueries[instanceId];
    }
}
