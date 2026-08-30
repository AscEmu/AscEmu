/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
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
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

//  move map related classes
namespace MMAP
{
    typedef std::unordered_map<uint32_t, dtTileRef> MMapTileSet;
    typedef std::unordered_map<uint32_t, dtNavMeshQuery*> NavMeshQuerySet;

    // One GM-captured off-mesh connection (".debug offmesh"): a straight-
    // line traversable link between two points, used as-is by
    // dtNavMeshCreateParams::offMeshCon* regardless of walkable slope -
    // exactly the mechanism mmaps_generator's --offMeshInput file already
    // feeds into a real tile at build time. Coordinates are in game (x, y,
    // z) order; callers building Detour structures must still apply the
    // usual (y, z, x) axis swap.
    struct RuntimeOffMeshConnection
    {
        float p0[3];
        float p1[3];
        float radius;
    };

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

        // Runtime off-mesh connections (".debug offmesh"), keyed by the same
        // packed [x,y] grid position as mmapLoadedTiles - one small "off-mesh
        // only" tile per position, added as an extra layer alongside the
        // real terrain tile at layer 0, carrying every connection captured
        // (this session or a previous one) for that tile.
        std::unordered_map<uint32_t, std::vector<RuntimeOffMeshConnection>> runtimeOffMeshConnections;
        MMapTileSet runtimeOffMeshTiles;
    };


    typedef std::unordered_map<uint32_t, std::unique_ptr<MMapData>> MMapDataSet;

    // singleton class
    // holds all all access to mmap loading unloading and meshes
    class MMapManager
    {
        public:
            MMapManager() : loadedTiles(0), thread_safe_environment(true), m_offMeshFileCacheLoaded(false) {}
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

            // Captures one GM-authored off-mesh connection (".debug
            // offmesh"): rebuilds the small off-mesh-only tile for that
            // connection's home grid position from every connection
            // captured there so far (this one included), hot-swaps it into
            // the already-loaded navmesh as an extra layer so it's
            // immediately usable by pathfinding (no server restart), and
            // appends it to the runtime persistence file so it survives one
            // and can be fed into a real mmaps_generator --offMeshInput
            // rebuild later. p0/p1 are in game (x, y, z) order. Returns
            // false (with outError set) if mapId's navmesh isn't loaded, or
            // p0's tile isn't currently loaded (both required so the new
            // connection can be linked into real, currently-loaded ground).
            bool addRuntimeOffMeshConnection(uint32_t mapId, const float* p0, const float* p1, float radius, std::string& outError);

            // Called right after a real tile finishes loading (see
            // MapCell.cpp) to inject any off-mesh connections previously
            // captured for that exact tile position - this session (if the
            // tile was unloaded and reloaded) or a prior one (persisted to
            // disk).
            void loadRuntimeOffMeshConnections(uint32_t mapId, int32_t x, int32_t y);

        private:
            bool loadMapData(uint32_t mapId);
            uint32_t packTileID(int32_t x, int32_t y);

            MMapDataSet::const_iterator GetMMapData(uint32_t mapId) const;

            // Rebuilds and hot-swaps the off-mesh-only dummy tile for
            // (mapId, x, y) from mmap->runtimeOffMeshConnections[packed
            // grid pos] - called once after a new connection is appended to
            // that list, and once when a persisted connection is loaded for
            // a tile that just finished loading.
            bool rebuildRuntimeOffMeshTile(MMapData* mmap, uint32_t mapId, int32_t x, int32_t y);

            // Lazily reads the whole runtime persistence file (once) into
            // runtimeOffMeshFileCache, keyed by [mapId, x, y] packed into a
            // single 64-bit value (packTileID's 32 bits aren't enough once
            // mapId is folded in too).
            void ensureRuntimeOffMeshFileCacheLoaded();
            void appendRuntimeOffMeshConnectionToFile(uint32_t mapId, int32_t x, int32_t y, const RuntimeOffMeshConnection& conn);
            static uint64_t packMapTileID(uint32_t mapId, int32_t x, int32_t y);

            MMapDataSet loadedMMaps;
            uint32_t loadedTiles;
            bool thread_safe_environment;

            // Guards runtimeOffMesh* state on MMapData plus the persistence
            // file cache/writes below - the real-tile loading paths above
            // rely on their caller's own locking (see MapCell.cpp), but
            // ".debug offmesh" can run concurrently with that, so this
            // feature keeps its own lock rather than assuming it's covered.
            std::mutex m_offMeshMutex;
            bool m_offMeshFileCacheLoaded;
            std::unordered_map<uint64_t, std::vector<RuntimeOffMeshConnection>> m_offMeshFileCache;
    };
}

#endif