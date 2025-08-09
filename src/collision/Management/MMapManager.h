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

#ifndef _MMAP_MANAGER_H
#define _MMAP_MANAGER_H

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

//  move map related classes
namespace MMAP
{
    typedef std::unordered_map<uint32_t, dtTileRef> MMapTileSet;
    typedef std::unordered_map<uint32_t, dtNavMeshQuery*> NavMeshQuerySet;

    // dummy struct to hold map's mmap data
    struct MMapData
    {
        MMapData(dtNavMesh* mesh) : navMesh(mesh) { }
        ~MMapData()
        {
            for (NavMeshQuerySet::iterator i = navMeshQueries.begin(); i != navMeshQueries.end(); ++i)
                dtFreeNavMeshQuery(i->second);

            if (navMesh)
                dtFreeNavMesh(navMesh);
        }

        dtNavMesh* navMesh;

        // we have to use single dtNavMeshQuery for every instance, since those are not thread safe
        NavMeshQuerySet navMeshQueries;     // instanceId to query
        MMapTileSet mmapLoadedTiles;        // maps [map grid coords] to [dtTile]
    };


    typedef std::unordered_map<uint32_t, std::unique_ptr<MMapData>> MMapDataSet;

    // singleton class
    // holds all all access to mmap loading unloading and meshes
    class MMapManager
    {
        public:
            MMapManager() : loadedTiles(0), thread_safe_environment(true) {}
            ~MMapManager();

            void InitializeThreadUnsafe(const std::vector<uint32_t>& mapIds);
            bool loadMap(const std::string& basePath, uint32_t mapId, int32_t x, int32_t y);
            bool unloadMap(uint32_t mapId, int32_t x, int32_t y);
            bool unloadMap(uint32_t mapId);
            bool unloadMapInstance(uint32_t mapId, uint32_t instanceId);

            // the returned [dtNavMeshQuery const*] is NOT threadsafe
            dtNavMeshQuery const* GetNavMeshQuery(uint32_t mapId, uint32_t instanceId);
            dtNavMesh const* GetNavMesh(uint32_t mapId);

            uint32_t getLoadedTilesCount() const { return loadedTiles; }
            uint32_t getLoadedMapsCount() const { return uint32_t(loadedMMaps.size()); }
        private:
            bool loadMapData(uint32_t mapId);
            uint32_t packTileID(int32_t x, int32_t y);

            MMapDataSet::const_iterator GetMMapData(uint32_t mapId) const;
            MMapDataSet loadedMMaps;
            uint32_t loadedTiles;
            bool thread_safe_environment;
    };
}

#endif