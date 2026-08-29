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

#ifndef _MAP_BUILDER_H
#define _MAP_BUILDER_H

#include "TerrainBuilder.h"

#include "Recast.h"
#include "DetourNavMesh.h"

#include "mpqlib/ClientVersion.hpp"

#include <vector>
#include <set>
#include <list>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <semaphore>
#include <type_traits>
#include <optional>
#include <string>
#include <unordered_map>

template <typename T>
class ProducerConsumerQueue
{
private:
    std::mutex _queueLock;
    std::queue<T> _queue;
    std::condition_variable _condition;
    std::atomic<bool> _shutdown;

public:
    ProducerConsumerQueue() : _shutdown(false) { }

    void Push(const T& value)
    {
        std::lock_guard<std::mutex> lock(_queueLock);
        _queue.push(std::move(value));

        _condition.notify_one();
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lock(_queueLock);

        return _queue.empty();
    }

    bool Pop(T& value)
    {
        std::lock_guard<std::mutex> lock(_queueLock);

        if (_queue.empty() || _shutdown)
            return false;

        value = _queue.front();

        _queue.pop();

        return true;
    }

    void WaitAndPop(T& value)
    {
        std::unique_lock<std::mutex> lock(_queueLock);

        // we could be using .wait(lock, predicate) overload here but it is broken
        // https://connect.microsoft.com/VisualStudio/feedback/details/1098841
        while (_queue.empty() && !_shutdown)
            _condition.wait(lock);

        if (_queue.empty() || _shutdown)
            return;

        value = _queue.front();

        _queue.pop();
    }

    void Cancel()
    {
        std::unique_lock<std::mutex> lock(_queueLock);

        while (!_queue.empty())
        {
            T& value = _queue.front();

            DeleteQueuedObject(value);

            _queue.pop();
        }

        _shutdown = true;

        _condition.notify_all();
    }

private:
    template<typename E = T>
    typename std::enable_if<std::is_pointer<E>::value>::type DeleteQueuedObject(E& obj) { delete obj; }

    template<typename E = T>
    typename std::enable_if<!std::is_pointer<E>::value>::type DeleteQueuedObject(E const& /*packet*/) { }
};


using namespace VMAP;

namespace MMAP
{
    struct MapTiles
    {
        MapTiles() : m_mapId(uint32_t(-1)), m_tiles(NULL) {}

        MapTiles(uint32_t id, std::set<uint32_t>* tiles) : m_mapId(id), m_tiles(tiles) {}
        ~MapTiles() {}

        uint32_t m_mapId;
        std::set<uint32_t>* m_tiles;

        bool operator==(uint32_t id)
        {
            return m_mapId == id;
        }
    };

    typedef std::list<MapTiles> TileList;

    // A single (map, tile) unit of work for buildAllMaps()'s worker pool -
    // queuing at this granularity (rather than one whole map per queue
    // entry) is what lets a handful of large continents' tiles spread
    // across every worker thread instead of monopolizing just one each
    // while threads working on small instance maps sit idle.
    struct TileWorkItem
    {
        uint32_t mapId = 0;
        uint32_t tileX = 0;
        uint32_t tileY = 0;
    };

    // Peeks at the buildMagic field of one already-extracted maps/*.map file
    // (map_extractor stamps every .map file's header with the client build
    // that produced it) to determine which client version this extraction
    // batch came from. Call only after checkDirectories() has confirmed
    // "maps" is non-empty. Returns std::nullopt if no .map file is found or
    // its buildMagic isn't a recognized build.
    std::optional<mpqlib::ClientVersion> detectClientVersion(const std::string& mapsDir = "maps");

    struct MmapTuning
    {
        float maxWalkableAngle;
        int smallWalkableHeight;
    };

    // Cata/MoP's client uses a slightly different pathing tolerance than
    // Classic/TBC/WotLK's; everything else about mmap generation is
    // identical across versions.
    MmapTuning getDefaultMmapTuning(mpqlib::ClientVersion version);

    struct Tile
    {
        Tile() : chf(NULL), solid(NULL), cset(NULL), pmesh(NULL), dmesh(NULL) {}
        ~Tile()
        {
            rcFreeCompactHeightfield(chf);
            rcFreeContourSet(cset);
            rcFreeHeightField(solid);
            rcFreePolyMesh(pmesh);
            rcFreePolyMeshDetail(dmesh);
        }
        rcCompactHeightfield* chf;
        rcHeightfield* solid;
        rcContourSet* cset;
        rcPolyMesh* pmesh;
        rcPolyMeshDetail* dmesh;
    };

    class MapBuilder
    {
        public:
            MapBuilder(float maxWalkableAngle   = 55.f,
                bool skipLiquid          = false,
                bool skipContinents      = false,
                bool skipJunkMaps        = true,
                bool skipBattlegrounds   = false,
                bool debugOutput         = false,
                bool bigBaseUnit         = false,
                const char* offMeshFilePath = NULL,
                int smallWalkableHeight = 2);

            ~MapBuilder();

            // builds all mmap tiles for the specified map id (ignores skip settings)
            void buildMap(uint32_t mapID);
            void buildMeshFromFile(char* name);

            // builds an mmap tile for the specified map and its mesh
            void buildSingleTile(uint32_t mapID, uint32_t tileX, uint32_t tileY);

            // builds list of maps, then builds all of mmap tiles (based on the skip settings)
            void buildAllMaps(unsigned int threads);

            void WorkerThread();

        private:
            // detect maps and tiles
            void discoverTiles();
            std::set<uint32_t>* getTileList(uint32_t mapID);

            void buildNavMesh(uint32_t mapID, dtNavMesh* &navMesh);

            // Shared setup step for both buildMap() (single-map debug path,
            // sequential) and buildAllMaps() (batch path, queues tiles
            // individually instead): resolves/discovers the map's tile
            // list and creates its navmesh. Returns nullptr on failure.
            dtNavMesh* prepareMapForBuild(uint32_t mapID, std::set<uint32_t>*& tilesOut);

            void buildTile(uint32_t mapID, uint32_t tileX, uint32_t tileY, dtNavMesh* navMesh);

            // move map building
            void buildMoveMapTile(uint32_t mapID,
                uint32_t tileX,
                uint32_t tileY,
                MeshData &meshData,
                float bmin[3],
                float bmax[3],
                dtNavMesh* navMesh);

            void getTileBounds(uint32_t tileX, uint32_t tileY,
                float* verts, int vertCount,
                float* bmin, float* bmax);
            void getGridBounds(uint32_t mapID, uint32_t &minX, uint32_t &minY, uint32_t &maxX, uint32_t &maxY) const;

            bool shouldSkipMap(uint32_t mapID);
            bool isTransportMap(uint32_t mapID);
            bool shouldSkipTile(uint32_t mapID, uint32_t tileX, uint32_t tileY);

            TerrainBuilder* m_terrainBuilder;
            TileList m_tiles;

            bool m_debugOutput;

            const char* m_offMeshFilePath;
            bool m_skipContinents;
            bool m_skipJunkMaps;
            bool m_skipBattlegrounds;

            float m_maxWalkableAngle;
            bool m_bigBaseUnit;
            int m_smallWalkableHeight;

            // build performance - not really used for now
            rcContext* m_rcContext;

            std::vector<std::thread> _workerThreads;
            ProducerConsumerQueue<TileWorkItem> _queue;
            std::atomic<bool> _cancelationToken;

            // Populated once, up front, by buildAllMaps() before any worker
            // thread starts - one navmesh per map being built, looked up
            // (read-only, via .at()) by worker threads processing that
            // map's tiles. Freed once the queue has fully drained.
            std::unordered_map<uint32_t, dtNavMesh*> _navMeshes;

            // dtNavMesh::addTile()/removeTile() mutate the navmesh's
            // internal tile pool and are not safe to call concurrently on
            // the same instance - guards just that short registration+write
            // sequence in buildMoveMapTile(), not the expensive Recast
            // rasterization/contour work that happens before it, so it
            // doesn't become a bottleneck.
            std::mutex _tileRegistrationMutex;

            // Bounds the *total* number of concurrently-running Recast
            // compute tasks across both levels of parallelism this class
            // has: buildAllMaps()'s outer worker threads (one whole tile
            // each) and buildMoveMapTile()'s inner per-sub-tile loop (one
            // map-tile is subdivided into up to TILES_PER_MAP^2 independent
            // sub-tiles, each its own Recast pipeline run). Both levels
            // acquire a permit only around the actual rc*() work, so the
            // inner loop only picks up extra parallelism when there's slack
            // (e.g. near the end of a batch run, when fewer outer tiles
            // remain than there are threads) rather than oversubscribing
            // the machine on top of the outer pool. Sized to hardware
            // concurrency by default (so the debug-only buildMap()/
            // buildSingleTile() paths, which have no outer pool at all,
            // still get full inner parallelism); buildAllMaps() re-sizes it
            // to match its own --threads value so an explicit user-chosen
            // thread cap is honored for inner parallelism too.
            std::optional<std::counting_semaphore<>> _computeSlots;
    };
}

#endif
