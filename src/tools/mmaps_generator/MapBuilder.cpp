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

#include "PathCommon.h"
#include "MapBuilder.h"
#include "MapTree.h"

#include "DetourNavMeshBuilder.h"
#include "DetourNavMesh.h"
#include "IntermediateValues.h"

#include <cstdio>
#include <execution>
#include <limits.h>
#include <numeric>


#define MMAP_MAGIC 0x4d4d4150   // 'MMAP'
#define MMAP_VERSION 5

struct MmapTileHeader
{
    uint32_t mmapMagic;
    uint32_t dtVersion;
    uint32_t mmapVersion;
    uint32_t size;
    bool usesLiquids : 1;

    MmapTileHeader() : mmapMagic(MMAP_MAGIC), dtVersion(DT_NAVMESH_VERSION),
        mmapVersion(MMAP_VERSION), size(0), usesLiquids(true) {}
};

// Mirrors map_extractor's map_fileheader layout (System.cpp) closely enough
// to read just the leading buildMagic field - the full struct isn't needed
// here since only these first three uint32_t fields matter for detection.
struct MapFileHeaderPrefix
{
    uint32_t mapMagic;
    uint32_t versionMagic;
    uint32_t buildMagic;
};

namespace MMAP
{
    std::optional<mpqlib::ClientVersion> detectClientVersion(const std::string& mapsDir)
    {
        std::vector<std::string> files;
        if (getDirContents(files, mapsDir, "*.map") != LISTFILE_OK || files.empty())
        {
            printf("detectClientVersion: no *.map files found in '%s'\n", mapsDir.c_str());
            return std::nullopt;
        }

        std::string const path = mapsDir + "/" + files.front();
        FILE* file = fopen(path.c_str(), "rb");
        if (!file)
        {
            printf("detectClientVersion: couldn't open '%s'\n", path.c_str());
            return std::nullopt;
        }

        MapFileHeaderPrefix header{};
        bool const readOk = fread(&header, sizeof(header), 1, file) == 1;
        fclose(file);

        if (!readOk)
        {
            printf("detectClientVersion: couldn't read header from '%s'\n", path.c_str());
            return std::nullopt;
        }

        auto const version = mpqlib::clientVersionFromBuild(header.buildMagic);
        if (!version)
            printf("detectClientVersion: '%s' has mapMagic=0x%08X versionMagic=0x%08X buildMagic=%u (unrecognized build)\n",
                path.c_str(), header.mapMagic, header.versionMagic, header.buildMagic);

        return version;
    }

    MmapTuning getDefaultMmapTuning(mpqlib::ClientVersion version)
    {
        switch (version)
        {
            case mpqlib::ClientVersion::Cataclysm:
            case mpqlib::ClientVersion::MistsOfPandaria:
                return { 70.0f, 4 };
            default:
                return { 70.0f, 2 };
        }
    }

    namespace
    {
        // RAII acquire/release for MapBuilder::_computeSlots - std::counting_semaphore
        // has no built-in lock_guard-style wrapper, and buildMoveMapTile()'s
        // per-sub-tile lambda (see below) has several early-return failure
        // paths that all need the permit released exactly once.
        class SemaphoreGuard
        {
        public:
            explicit SemaphoreGuard(std::counting_semaphore<>& sem) : m_sem(sem) { m_sem.acquire(); }
            ~SemaphoreGuard() { m_sem.release(); }
            SemaphoreGuard(SemaphoreGuard const&) = delete;
            SemaphoreGuard& operator=(SemaphoreGuard const&) = delete;
        private:
            std::counting_semaphore<>& m_sem;
        };
    }

    MapBuilder::MapBuilder(float maxWalkableAngle, bool skipLiquid,
        bool skipContinents, bool skipJunkMaps, bool skipBattlegrounds,
        bool debugOutput, bool bigBaseUnit, const char* offMeshFilePath,
        int smallWalkableHeight) :
        m_terrainBuilder     (NULL),
        m_debugOutput        (debugOutput),
        m_offMeshFilePath    (offMeshFilePath),
        m_skipContinents     (skipContinents),
        m_skipJunkMaps       (skipJunkMaps),
        m_skipBattlegrounds  (skipBattlegrounds),
        m_maxWalkableAngle   (maxWalkableAngle),
        m_bigBaseUnit        (bigBaseUnit),
        m_smallWalkableHeight(smallWalkableHeight),
        m_rcContext          (NULL),
        _cancelationToken    (false)
    {
        m_terrainBuilder = new TerrainBuilder(skipLiquid);

        m_rcContext = new rcContext(false);

        // Default sizing for the debug-only buildMap()/buildSingleTile()
        // paths, which never call buildAllMaps() (and so never re-size this
        // to a --threads value) but still want full inner parallelism.
        unsigned int hwThreads = std::thread::hardware_concurrency();
        _computeSlots.emplace(hwThreads > 0 ? hwThreads : 1);

        discoverTiles();
    }

    /**************************************************************************/
    MapBuilder::~MapBuilder()
    {
        for (TileList::iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
        {
            (*it).m_tiles->clear();
            delete (*it).m_tiles;
        }

        delete m_terrainBuilder;
        delete m_rcContext;
    }

    /**************************************************************************/
    void MapBuilder::discoverTiles()
    {
        std::vector<std::string> files;
        uint32_t mapID, tileX, tileY, tileID, count = 0;
        char filter[255];

        printf("Discovering maps... ");
        getDirContents(files, "maps");
        for (uint32_t i = 0; i < files.size(); ++i)
        {
            mapID = uint32_t(atoi(files[i].substr(0, 4).c_str()));
            if (std::find(m_tiles.begin(), m_tiles.end(), mapID) == m_tiles.end())
            {
                m_tiles.emplace_back(MapTiles(mapID, new std::set<uint32_t>));
                count++;
            }
        }

        files.clear();
        getDirContents(files, "vmaps", "*.vmtree");
        for (uint32_t i = 0; i < files.size(); ++i)
        {
            mapID = uint32_t(atoi(files[i].substr(0, 4).c_str()));
            if (std::find(m_tiles.begin(), m_tiles.end(), mapID) == m_tiles.end())
            {
                m_tiles.emplace_back(MapTiles(mapID, new std::set<uint32_t>));
                count++;
            }
        }
        printf("found %u.\n", count);

        count = 0;
        printf("Discovering tiles... ");
        for (TileList::iterator itr = m_tiles.begin(); itr != m_tiles.end(); ++itr)
        {
            std::set<uint32_t>* tiles = (*itr).m_tiles;
            mapID = (*itr).m_mapId;

            sprintf(filter, "%04u*.vmtile", mapID);
            files.clear();
            getDirContents(files, "vmaps", filter);
            for (uint32_t i = 0; i < files.size(); ++i)
            {
                tileX = uint32_t(atoi(files[i].substr(8, 2).c_str()));
                tileY = uint32_t(atoi(files[i].substr(5, 2).c_str()));
                tileID = StaticMapTree::packTileID(tileY, tileX);

                tiles->insert(tileID);
                count++;
            }

            sprintf(filter, "%04u*", mapID);
            files.clear();
            getDirContents(files, "maps", filter);
            for (uint32_t i = 0; i < files.size(); ++i)
            {
                tileY = uint32_t(atoi(files[i].substr(5, 2).c_str()));
                tileX = uint32_t(atoi(files[i].substr(8, 2).c_str()));
                tileID = StaticMapTree::packTileID(tileX, tileY);

                if (tiles->insert(tileID).second)
                    count++;
            }
        }
        printf("found %u.\n\n", count);
    }

    /**************************************************************************/
    std::set<uint32_t>* MapBuilder::getTileList(uint32_t mapID)
    {
        TileList::iterator itr = std::find(m_tiles.begin(), m_tiles.end(), mapID);
        if (itr != m_tiles.end())
            return (*itr).m_tiles;

        std::set<uint32_t>* tiles = new std::set<uint32_t>();
        m_tiles.emplace_back(MapTiles(mapID, tiles));
        return tiles;
    }

    /**************************************************************************/

    void MapBuilder::WorkerThread()
    {
        while (1)
        {
            TileWorkItem item;

            _queue.WaitAndPop(item);

            if (_cancelationToken)
                return;

            buildTile(item.mapId, item.tileX, item.tileY, _navMeshes.at(item.mapId));
        }
    }

    void MapBuilder::buildAllMaps(unsigned int threads)
    {
        printf("Using %u theads for mmaps", threads);

        // Match the inner per-sub-tile semaphore to this run's own thread
        // cap (an explicit --threads 0 means "don't parallelize at all",
        // which should hold for the inner loop too, not just the outer
        // queue).
        _computeSlots.emplace(threads > 0 ? threads : 1);

        m_tiles.sort([](MapTiles a, MapTiles b)
        {
            return a.m_tiles->size() > b.m_tiles->size();
        });

        // Set up every map's navmesh and queue its tiles individually
        // (rather than queuing one whole map per work item) before any
        // worker thread starts, so a handful of large continents don't end
        // up monopolizing one thread each while other threads run dry.
        for (TileList::iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
        {
            uint32_t mapId = it->m_mapId;
            if (shouldSkipMap(mapId))
                continue;

            std::set<uint32_t>* tiles = nullptr;
            dtNavMesh* navMesh = prepareMapForBuild(mapId, tiles);
            if (!navMesh)
                continue;

            _navMeshes[mapId] = navMesh;

            printf("[Map %04i] We have %u tiles.\n", mapId, (unsigned int)tiles->size());
            for (uint32_t tileId : *tiles)
            {
                uint32_t tileX, tileY;
                StaticMapTree::unpackTileID(tileId, tileX, tileY);

                if (shouldSkipTile(mapId, tileX, tileY))
                    continue;

                if (threads > 0)
                    _queue.Push(TileWorkItem{ mapId, tileX, tileY });
                else
                    buildTile(mapId, tileX, tileY, navMesh);
            }
        }

        if (threads > 0)
        {
            for (unsigned int i = 0; i < threads; ++i)
            {
                _workerThreads.push_back(std::thread(&MapBuilder::WorkerThread, this));
            }

            while (!_queue.Empty())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }

            _cancelationToken = true;

            _queue.Cancel();

            for (auto& thread : _workerThreads)
            {
                thread.join();
            }
        }

        for (auto& [mapId, navMesh] : _navMeshes)
            dtFreeNavMesh(navMesh);
        _navMeshes.clear();
    }

    /**************************************************************************/
    void MapBuilder::getGridBounds(uint32_t mapID, uint32_t &minX, uint32_t &minY, uint32_t &maxX, uint32_t &maxY) const
    {
        maxX = 0;
        maxY = 0;
        minX = std::numeric_limits<uint32_t>::max();
        minY = std::numeric_limits<uint32_t>::max();

        float bmin[3] = { 0, 0, 0 };
        float bmax[3] = { 0, 0, 0 };
        float lmin[3] = { 0, 0, 0 };
        float lmax[3] = { 0, 0, 0 };
        MeshData meshData;

        // make sure we process maps which don't have tiles
        // initialize the static tree, which loads WDT models
        if (!m_terrainBuilder->loadVMap(mapID, 64, 64, meshData))
            return;

        // get the coord bounds of the model data
        if (meshData.solidVerts.size() + meshData.liquidVerts.size() == 0)
            return;

        // get the coord bounds of the model data
        if (meshData.solidVerts.size() && meshData.liquidVerts.size())
        {
            rcCalcBounds(meshData.solidVerts.getCArray(), meshData.solidVerts.size() / 3, bmin, bmax);
            rcCalcBounds(meshData.liquidVerts.getCArray(), meshData.liquidVerts.size() / 3, lmin, lmax);
            rcVmin(bmin, lmin);
            rcVmax(bmax, lmax);
        }
        else if (meshData.solidVerts.size())
            rcCalcBounds(meshData.solidVerts.getCArray(), meshData.solidVerts.size() / 3, bmin, bmax);
        else
            rcCalcBounds(meshData.liquidVerts.getCArray(), meshData.liquidVerts.size() / 3, lmin, lmax);

        // convert coord bounds to grid bounds
        maxX = static_cast<uint32_t>(32 - bmin[0] / GRID_SIZE);
        maxY = static_cast<uint32_t>(32 - bmin[2] / GRID_SIZE);
        minX = static_cast<uint32_t>(32 - bmax[0] / GRID_SIZE);
        minY = static_cast<uint32_t>(32 - bmax[2] / GRID_SIZE);
    }

    void MapBuilder::buildMeshFromFile(char* name)
    {
        FILE* file = fopen(name, "rb");
        if (!file)
            return;

        printf("Building mesh from file\n");
        int tileX, tileY, mapId;
        if (fread(&mapId, sizeof(int), 1, file) != 1)
        {
            fclose(file);
            return;
        }
        if (fread(&tileX, sizeof(int), 1, file) != 1)
        {
            fclose(file);
            return;
        }
        if (fread(&tileY, sizeof(int), 1, file) != 1)
        {
            fclose(file);
            return;
        }

        dtNavMesh* navMesh = NULL;
        buildNavMesh(mapId, navMesh);
        if (!navMesh)
        {
            printf("Failed creating navmesh!\n");
            fclose(file);
            return;
        }

        uint32_t verticesCount, indicesCount;
        if (fread(&verticesCount, sizeof(uint32_t), 1, file) != 1)
        {
            fclose(file);
            return;
        }

        if (fread(&indicesCount, sizeof(uint32_t), 1, file) != 1)
        {
            fclose(file);
            return;
        }

        float* verts = new float[verticesCount];
        int* inds = new int[indicesCount];

        if (fread(verts, sizeof(float), verticesCount, file) != verticesCount)
        {
            fclose(file);
            delete[] verts;
            delete[] inds;
            return;
        }

        if (fread(inds, sizeof(int), indicesCount, file) != indicesCount)
        {
            fclose(file);
            delete[] verts;
            delete[] inds;
            return;
        }

        MeshData data;

        for (uint32_t i = 0; i < verticesCount; ++i)
            data.solidVerts.append(verts[i]);
        delete[] verts;

        for (uint32_t i = 0; i < indicesCount; ++i)
            data.solidTris.append(inds[i]);
        delete[] inds;

        TerrainBuilder::cleanVertices(data.solidVerts, data.solidTris);
        // get bounds of current tile
        float bmin[3], bmax[3];
        getTileBounds(tileX, tileY, data.solidVerts.getCArray(), data.solidVerts.size() / 3, bmin, bmax);

        // build navmesh tile
        buildMoveMapTile(mapId, tileX, tileY, data, bmin, bmax, navMesh);
        fclose(file);
    }

    /**************************************************************************/
    void MapBuilder::buildSingleTile(uint32_t mapID, uint32_t tileX, uint32_t tileY)
    {
        dtNavMesh* navMesh = NULL;
        buildNavMesh(mapID, navMesh);
        if (!navMesh)
        {
            printf("Failed creating navmesh!\n");
            return;
        }

        buildTile(mapID, tileX, tileY, navMesh);
        dtFreeNavMesh(navMesh);
    }

    /**************************************************************************/
    dtNavMesh* MapBuilder::prepareMapForBuild(uint32_t mapID, std::set<uint32_t>*& tilesOut)
    {
        std::set<uint32_t>* tiles = getTileList(mapID);

        // make sure we process maps which don't have tiles
        if (!tiles->size())
        {
            // convert coord bounds to grid bounds
            uint32_t minX, minY, maxX, maxY;
            getGridBounds(mapID, minX, minY, maxX, maxY);

            // add all tiles within bounds to tile list.
            for (uint32_t i = minX; i <= maxX; ++i)
                for (uint32_t j = minY; j <= maxY; ++j)
                    tiles->insert(StaticMapTree::packTileID(i, j));
        }

        tilesOut = tiles;

        if (tiles->empty())
            return nullptr;

        dtNavMesh* navMesh = NULL;
        buildNavMesh(mapID, navMesh);
        if (!navMesh)
            printf("[Map %04i] Failed creating navmesh!\n", mapID);

        return navMesh;
    }

    void MapBuilder::buildMap(uint32_t mapID)
    {
#ifndef __APPLE__
        //printf("[Thread %u] Building map %04u:\n", uint32_t(ACE_Thread::self()), mapID);
#endif

        std::set<uint32_t>* tiles = nullptr;
        dtNavMesh* navMesh = prepareMapForBuild(mapID, tiles);

        if (!navMesh)
        {
            // prepareMapForBuild() already printed the failure reason when
            // there were tiles to build but navmesh creation itself failed;
            // an empty tile list is not an error, just nothing to do.
            if (!tiles->empty())
                return;
        }
        else
        {
            // now start building mmtiles for each tile
            printf("[Map %04i] We have %u tiles.\n", mapID, (unsigned int)tiles->size());
            for (std::set<uint32_t>::iterator it = tiles->begin(); it != tiles->end(); ++it)
            {
                uint32_t tileX, tileY;

                // unpack tile coords
                StaticMapTree::unpackTileID((*it), tileX, tileY);

                if (shouldSkipTile(mapID, tileX, tileY))
                    continue;

                buildTile(mapID, tileX, tileY, navMesh);
            }

            dtFreeNavMesh(navMesh);
        }

        printf("[Map %04u] Complete!\n", mapID);
    }

    /**************************************************************************/
    void MapBuilder::buildTile(uint32_t mapID, uint32_t tileX, uint32_t tileY, dtNavMesh* navMesh)
    {
        //printf("[Map %04i] Building tile [%02u,%02u]\n", mapID, tileX, tileY);

        MeshData meshData;

        // get heightmap data
        m_terrainBuilder->loadMap(mapID, tileX, tileY, meshData);

        // get model data
        m_terrainBuilder->loadVMap(mapID, tileY, tileX, meshData);

        // if there is no data, give up now
        if (!meshData.solidVerts.size() && !meshData.liquidVerts.size())
            return;

        // remove unused vertices
        TerrainBuilder::cleanVertices(meshData.solidVerts, meshData.solidTris);
        TerrainBuilder::cleanVertices(meshData.liquidVerts, meshData.liquidTris);

        // gather all mesh data for final data check, and bounds calculation
        G3D::Array<float> allVerts;
        allVerts.append(meshData.liquidVerts);
        allVerts.append(meshData.solidVerts);

        if (!allVerts.size())
            return;

        // get bounds of current tile
        float bmin[3], bmax[3];
        getTileBounds(tileX, tileY, allVerts.getCArray(), allVerts.size() / 3, bmin, bmax);

        m_terrainBuilder->loadOffMeshConnections(mapID, tileX, tileY, meshData, m_offMeshFilePath);

        // build navmesh tile
        buildMoveMapTile(mapID, tileX, tileY, meshData, bmin, bmax, navMesh);
    }

    /**************************************************************************/
    void MapBuilder::buildNavMesh(uint32_t mapID, dtNavMesh* &navMesh)
    {
        std::set<uint32_t>* tiles = getTileList(mapID);

        // old code for non-statically assigned bitmask sizes:
        ///*** calculate number of bits needed to store tiles & polys ***/
        //int tileBits = dtIlog2(dtNextPow2(tiles->size()));
        //if (tileBits < 1) tileBits = 1;                                     // need at least one bit!
        //int polyBits = sizeof(dtPolyRef)*8 - SALT_MIN_BITS - tileBits;

        int polyBits = DT_POLY_BITS;

        // A handful of spare tile slots beyond what this map's real terrain
        // needs, reserved so the live server can inject small, extra
        // "off-mesh connection only" tiles (see MMapManager's runtime
        // off-mesh support) as additional layers at an already-used (x,y)
        // grid position, without needing to rebuild this .mmap file first.
        // DT_TILE_BITS (21) gives ~2M addressable tile slots regardless of
        // maxTiles, so this reserve never risks exhausting that budget even
        // for the smallest instance maps.
        constexpr int kRuntimeOffMeshTileReserve = 64;
        int maxTiles = static_cast<int>(tiles->size()) + kRuntimeOffMeshTileReserve;
        int maxPolysPerTile = 1 << polyBits;

        /***          calculate bounds of map         ***/

        uint32_t tileXMin = 64, tileYMin = 64, tileXMax = 0, tileYMax = 0, tileX, tileY;
        for (std::set<uint32_t>::iterator it = tiles->begin(); it != tiles->end(); ++it)
        {
            StaticMapTree::unpackTileID(*it, tileX, tileY);

            if (tileX > tileXMax)
                tileXMax = tileX;
            else if (tileX < tileXMin)
                tileXMin = tileX;

            if (tileY > tileYMax)
                tileYMax = tileY;
            else if (tileY < tileYMin)
                tileYMin = tileY;
        }

        // use Max because '32 - tileX' is negative for values over 32
        float bmin[3], bmax[3];
        getTileBounds(tileXMax, tileYMax, NULL, 0, bmin, bmax);

        /***       now create the navmesh       ***/

        // navmesh creation params
        dtNavMeshParams navMeshParams;
        memset(&navMeshParams, 0, sizeof(dtNavMeshParams));
        navMeshParams.tileWidth = GRID_SIZE;
        navMeshParams.tileHeight = GRID_SIZE;
        rcVcopy(navMeshParams.orig, bmin);
        navMeshParams.maxTiles = maxTiles;
        navMeshParams.maxPolys = maxPolysPerTile;

        navMesh = dtAllocNavMesh();
        printf("[Map %04u] Creating navMesh...\n", mapID);
        if (!navMesh->init(&navMeshParams))
        {
            printf("[Map %04u] Failed creating navmesh!\n", mapID);
            return;
        }

        char fileName[255];
        sprintf(fileName, "mmaps/%04u.mmap", mapID);

        FILE* file = fopen(fileName, "wb");
        if (!file)
        {
            dtFreeNavMesh(navMesh);
            char message[255];
            sprintf(message, "[Map %04u] Failed to open %s for writing!\n", mapID, fileName);
            perror(message);
            return;
        }

        // now that we know navMesh params are valid, we can write them to file
        fwrite(&navMeshParams, sizeof(dtNavMeshParams), 1, file);
        fclose(file);
    }

    /**************************************************************************/
    void MapBuilder::buildMoveMapTile(uint32_t mapID, uint32_t tileX, uint32_t tileY,
        MeshData &meshData, float bmin[3], float bmax[3],
        dtNavMesh* navMesh)
    {
        // console output
        char tileString[255];
        sprintf(tileString, "[Map %04u] [%02i,%02i]: ", mapID, tileX, tileY);
        //printf("%s Building movemap tiles...\n", tileString);

        IntermediateValues iv;

        float* tVerts = meshData.solidVerts.getCArray();
        int tVertCount = meshData.solidVerts.size() / 3;
        int* tTris = meshData.solidTris.getCArray();
        int tTriCount = meshData.solidTris.size() / 3;

        float* lVerts = meshData.liquidVerts.getCArray();
        int lVertCount = meshData.liquidVerts.size() / 3;
        int* lTris = meshData.liquidTris.getCArray();
        int lTriCount = meshData.liquidTris.size() / 3;
        uint8_t* lTriFlags = meshData.liquidType.getCArray();

        // these are WORLD UNIT based metrics
        // this are basic unit dimentions
        // value have to divide GRID_SIZE(533.3333f) ( aka: 0.5333, 0.2666, 0.3333, 0.1333, etc )
        const static float BASE_UNIT_DIM = m_bigBaseUnit ? 0.5333333f : 0.2666666f;

        // All are in UNIT metrics!
        const static int VERTEX_PER_MAP = int(GRID_SIZE/BASE_UNIT_DIM + 0.5f);
        const static int VERTEX_PER_TILE = m_bigBaseUnit ? 40 : 80; // must divide VERTEX_PER_MAP
        const static int TILES_PER_MAP = VERTEX_PER_MAP/VERTEX_PER_TILE;

        rcConfig config;
        memset(&config, 0, sizeof(rcConfig));

        rcVcopy(config.bmin, bmin);
        rcVcopy(config.bmax, bmax);

        config.maxVertsPerPoly = DT_VERTS_PER_POLYGON;
        config.cs = BASE_UNIT_DIM;
        config.ch = BASE_UNIT_DIM;
        config.walkableSlopeAngle = m_maxWalkableAngle;
        config.tileSize = VERTEX_PER_TILE;
        config.walkableRadius = m_bigBaseUnit ? 1 : 2;
        config.borderSize = config.walkableRadius + 3;
        // ~6 world units regardless of base unit size (was VERTEX_PER_TILE+1,
        // i.e. deliberately larger than any real edge so this never fired at
        // all) - Recast only inserts extra vertices along a contour edge
        // longer than this, so a long/steep ramp edge previously got none,
        // leaving getSteerTarget()'s path-smoothing funnel nothing to steer
        // through except the edge's two endpoints - it "sees" the far end
        // directly and cuts a straight line across the slope instead of
        // following it. A denser vertex chain along long edges gives it
        // intermediate points to follow the actual terrain shape instead.
        config.maxEdgeLen = static_cast<int>(6.0f / config.cs);
        // walkableHeight/walkableClimb used to be 3|6 and 4|8 (world units
        // ~1.6 and ~2.13) - climb bigger than height. That climb value was a
        // deliberate workaround to let npcs walk over fences (see the WMO
        // MOPY material-flag fix in vmap4_extractor/wmo.cpp/.h), but Recast
        // requires walkableClimb < walkableHeight for its region/connectivity
        // logic to treat vertically stacked walkable surfaces as separate -
        // violating it let the open floor beneath an elevated surface (e.g. a
        // ramp) get merged into the same region as the surface above it,
        // producing navmesh points stuck at the lower floor's height the
        // entire way up a ramp. Now that the MOPY fix gives fences real
        // collision geometry, the fence-hopping climb value isn't needed
        // anymore, so both constants move to realistic values (walkableHeight
        // ~2.13 world units, close to the actual player capsule height) with
        // climb strictly below height.
        config.walkableHeight = m_bigBaseUnit ? m_smallWalkableHeight : 8;
        config.walkableClimb = m_bigBaseUnit ? 3 : 6;
        config.minRegionArea = rcSqr(60);
        config.mergeRegionArea = rcSqr(50);
        config.maxSimplificationError = 1.8f;
        // Was cs*64 (~17 world units between height samples) / ch*2 - finer
        // detail-mesh sampling (inaccurate Z from Detour on slopes).
        // Unlike maxEdgeLen above, this only affects the height-detail mesh
        // used by getPolyHeight()/getClosestPointOnPoly(), not the polygon
        // graph itself - more accurate height without adding any A* nodes
        // or changing navmesh connectivity, just extra generation time and
        // .mmtile size for the extra detail vertices.
        config.detailSampleDist = config.cs * 16;
        config.detailSampleMaxError = config.ch * 1;

        // this sets the dimensions of the heightfield - should maybe happen before border padding
        rcCalcGridSize(config.bmin, config.bmax, config.cs, &config.width, &config.height);

        // allocate subregions : tiles
        Tile* tiles = new Tile[TILES_PER_MAP * TILES_PER_MAP];

        // Each sub-tile below writes into its own fixed slot (indexed by
        // its own x+y*TILES_PER_MAP, matching tiles[]) rather than
        // compacting into pmmerge/dmmerge as it goes - the compaction pass
        // further down runs afterward, sequentially, in the same y-major
        // scan order the original single-threaded loop used, so
        // rcMergePolyMeshes()/rcMergePolyMeshDetails() see identical input
        // ordering no matter which sub-tile task happens to finish first.
        std::vector<rcPolyMesh*> pmeshSlots(TILES_PER_MAP * TILES_PER_MAP, nullptr);
        std::vector<rcPolyMeshDetail*> dmeshSlots(TILES_PER_MAP * TILES_PER_MAP, nullptr);

        std::vector<int> subTileIndices(TILES_PER_MAP * TILES_PER_MAP);
        std::iota(subTileIndices.begin(), subTileIndices.end(), 0);

        // build all sub-tiles - see _computeSlots's declaration in
        // MapBuilder.h for why this can safely run alongside
        // buildAllMaps()'s outer worker-thread pool without oversubscribing.
        std::for_each(std::execution::par, subTileIndices.begin(), subTileIndices.end(), [&](int idx)
        {
            int x = idx % TILES_PER_MAP;
            int y = idx / TILES_PER_MAP;

            SemaphoreGuard computeSlot(*_computeSlots);

            Tile& tile = tiles[idx];

            // Per-sub-tile config, computed fresh from the shared, never-
            // mutated `config` above - the original single-threaded loop
            // reused one shared tileCfg instance across iterations, which
            // would be a data race if run concurrently.
            rcConfig tileCfg = config;
            tileCfg.width = config.tileSize + config.borderSize*2;
            tileCfg.height = config.tileSize + config.borderSize*2;

            // Calculate the per tile bounding box.
            tileCfg.bmin[0] = config.bmin[0] + float(x*config.tileSize - config.borderSize)*config.cs;
            tileCfg.bmin[2] = config.bmin[2] + float(y*config.tileSize - config.borderSize)*config.cs;
            tileCfg.bmax[0] = config.bmin[0] + float((x+1)*config.tileSize + config.borderSize)*config.cs;
            tileCfg.bmax[2] = config.bmin[2] + float((y+1)*config.tileSize + config.borderSize)*config.cs;

            // build heightfield
            tile.solid = rcAllocHeightfield();
            if (!tile.solid || !rcCreateHeightfield(m_rcContext, *tile.solid, tileCfg.width, tileCfg.height, tileCfg.bmin, tileCfg.bmax, tileCfg.cs, tileCfg.ch))
            {
                printf("%s Failed building heightfield!\n", tileString);
                return;
            }

            // mark all walkable tiles, both liquids and solids
            unsigned char* triFlags = new unsigned char[tTriCount];
            memset(triFlags, NAV_GROUND, tTriCount*sizeof(unsigned char));
            rcClearUnwalkableTriangles(m_rcContext, tileCfg.walkableSlopeAngle, tVerts, tVertCount, tTris, tTriCount, triFlags);
            rcRasterizeTriangles(m_rcContext, tVerts, tVertCount, tTris, triFlags, tTriCount, *tile.solid, config.walkableClimb);
            delete[] triFlags;

            rcFilterLowHangingWalkableObstacles(m_rcContext, config.walkableClimb, *tile.solid);
            rcFilterLedgeSpans(m_rcContext, tileCfg.walkableHeight, tileCfg.walkableClimb, *tile.solid);
            rcFilterWalkableLowHeightSpans(m_rcContext, tileCfg.walkableHeight, *tile.solid);

            rcRasterizeTriangles(m_rcContext, lVerts, lVertCount, lTris, lTriFlags, lTriCount, *tile.solid, config.walkableClimb);

            // compact heightfield spans
            tile.chf = rcAllocCompactHeightfield();
            if (!tile.chf || !rcBuildCompactHeightfield(m_rcContext, tileCfg.walkableHeight, tileCfg.walkableClimb, *tile.solid, *tile.chf))
            {
                printf("%s Failed compacting heightfield!\n", tileString);
                return;
            }

            // build polymesh intermediates
            if (!rcErodeWalkableArea(m_rcContext, config.walkableRadius, *tile.chf))
            {
                printf("%s Failed eroding area!\n", tileString);
                return;
            }

            if (!rcBuildDistanceField(m_rcContext, *tile.chf))
            {
                printf("%s Failed building distance field!\n", tileString);
                return;
            }

            if (!rcBuildRegions(m_rcContext, *tile.chf, tileCfg.borderSize, tileCfg.minRegionArea, tileCfg.mergeRegionArea))
            {
                printf("%s Failed building regions!\n", tileString);
                return;
            }

            tile.cset = rcAllocContourSet();
            if (!tile.cset || !rcBuildContours(m_rcContext, *tile.chf, tileCfg.maxSimplificationError, tileCfg.maxEdgeLen, *tile.cset))
            {
                printf("%s Failed building contours!\n", tileString);
                return;
            }

            // build polymesh
            tile.pmesh = rcAllocPolyMesh();
            if (!tile.pmesh || !rcBuildPolyMesh(m_rcContext, *tile.cset, tileCfg.maxVertsPerPoly, *tile.pmesh))
            {
                printf("%s Failed building polymesh!\n", tileString);
                return;
            }

            tile.dmesh = rcAllocPolyMeshDetail();
            if (!tile.dmesh || !rcBuildPolyMeshDetail(m_rcContext, *tile.pmesh, *tile.chf, tileCfg.detailSampleDist, tileCfg.detailSampleMaxError, *tile.dmesh))
            {
                printf("%s Failed building polymesh detail!\n", tileString);
                return;
            }

            // free those up
            // we may want to keep them in the future for debug
            // but right now, we don't have the code to merge them
            rcFreeHeightField(tile.solid);
            tile.solid = NULL;
            rcFreeCompactHeightfield(tile.chf);
            tile.chf = NULL;
            rcFreeContourSet(tile.cset);
            tile.cset = NULL;

            pmeshSlots[idx] = tile.pmesh;
            dmeshSlots[idx] = tile.dmesh;
        });

        // merge per tile poly and detail meshes - compacted here in the
        // same order the sub-tiles were originally built in sequentially.
        rcPolyMesh** pmmerge = new rcPolyMesh*[TILES_PER_MAP * TILES_PER_MAP];
        rcPolyMeshDetail** dmmerge = new rcPolyMeshDetail*[TILES_PER_MAP * TILES_PER_MAP];
        int nmerge = 0;
        for (int idx = 0; idx < TILES_PER_MAP * TILES_PER_MAP; ++idx)
        {
            if (pmeshSlots[idx])
            {
                pmmerge[nmerge] = pmeshSlots[idx];
                dmmerge[nmerge] = dmeshSlots[idx];
                nmerge++;
            }
        }

        iv.polyMesh = rcAllocPolyMesh();
        if (!iv.polyMesh)
        {
            printf("%s alloc iv.polyMesh FAILED!\n", tileString);
            delete[] pmmerge;
            delete[] dmmerge;
            delete[] tiles;
            return;
        }
        rcMergePolyMeshes(m_rcContext, pmmerge, nmerge, *iv.polyMesh);

        iv.polyMeshDetail = rcAllocPolyMeshDetail();
        if (!iv.polyMeshDetail)
        {
            printf("%s alloc m_dmesh FAILED!\n", tileString);
            delete[] pmmerge;
            delete[] dmmerge;
            delete[] tiles;
            return;
        }
        rcMergePolyMeshDetails(m_rcContext, dmmerge, nmerge, *iv.polyMeshDetail);

        // free things up
        delete[] pmmerge;
        delete[] dmmerge;
        delete[] tiles;

        // set polygons as walkable
        // TODO: special flags for DYNAMIC polygons, ie surfaces that can be turned on and off
        for (int i = 0; i < iv.polyMesh->npolys; ++i)
            if (iv.polyMesh->areas[i] & RC_WALKABLE_AREA)
                iv.polyMesh->flags[i] = iv.polyMesh->areas[i];

        // setup mesh parameters
        dtNavMeshCreateParams params;
        memset(&params, 0, sizeof(params));
        params.verts = iv.polyMesh->verts;
        params.vertCount = iv.polyMesh->nverts;
        params.polys = iv.polyMesh->polys;
        params.polyAreas = iv.polyMesh->areas;
        params.polyFlags = iv.polyMesh->flags;
        params.polyCount = iv.polyMesh->npolys;
        params.nvp = iv.polyMesh->nvp;
        params.detailMeshes = iv.polyMeshDetail->meshes;
        params.detailVerts = iv.polyMeshDetail->verts;
        params.detailVertsCount = iv.polyMeshDetail->nverts;
        params.detailTris = iv.polyMeshDetail->tris;
        params.detailTriCount = iv.polyMeshDetail->ntris;

        params.offMeshConVerts = meshData.offMeshConnections.getCArray();
        params.offMeshConCount = meshData.offMeshConnections.size()/6;
        params.offMeshConRad = meshData.offMeshConnectionRads.getCArray();
        params.offMeshConDir = meshData.offMeshConnectionDirs.getCArray();
        params.offMeshConAreas = meshData.offMeshConnectionsAreas.getCArray();
        params.offMeshConFlags = meshData.offMeshConnectionsFlags.getCArray();

        params.walkableHeight = BASE_UNIT_DIM*config.walkableHeight;    // agent height
        params.walkableRadius = BASE_UNIT_DIM*config.walkableRadius;    // agent radius
        params.walkableClimb = BASE_UNIT_DIM*config.walkableClimb;      // keep less that walkableHeight (aka agent height)!
        params.tileX = static_cast<int>((((bmin[0] + bmax[0]) / 2) - navMesh->getParams()->orig[0]) / GRID_SIZE);
        params.tileY = static_cast<int>((((bmin[2] + bmax[2]) / 2) - navMesh->getParams()->orig[2]) / GRID_SIZE);
        rcVcopy(params.bmin, bmin);
        rcVcopy(params.bmax, bmax);
        params.cs = config.cs;
        params.ch = config.ch;
        params.tileLayer = 0;
        params.buildBvTree = true;

        // will hold final navmesh
        unsigned char* navData = NULL;
        int navDataSize = 0;

        do
        {
            // these values are checked within dtCreateNavMeshData - handle them here
            // so we have a clear error message
            if (params.nvp > DT_VERTS_PER_POLYGON)
            {
                printf("%s Invalid verts-per-polygon value! \n", tileString);
                break;
            }
            if (params.vertCount >= 0xffff)
            {
                printf("%s Too many vertices! \n", tileString);
                break;
            }
            if (!params.vertCount || !params.verts)
            {
                // occurs mostly when adjacent tiles have models
                // loaded but those models don't span into this tile

                // message is an annoyance
                //printf("%sNo vertices to build tile!\n", tileString);
                break;
            }
            if (!params.polyCount || !params.polys ||
                TILES_PER_MAP*TILES_PER_MAP == params.polyCount)
            {
                // we have flat tiles with no actual geometry - don't build those, its useless
                // keep in mind that we do output those into debug info
                // drop tiles with only exact count - some tiles may have geometry while having less tiles
                printf("%s No polygons to build on tile! \n", tileString);
                break;
            }
            if (!params.detailMeshes || !params.detailVerts || !params.detailTris)
            {
                printf("%s No detail mesh to build tile!\n", tileString);
                break;
            }

            //printf("%s Building navmesh tile...\n", tileString);
            if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
            {
                printf("%s Failed building navmesh tile!\n", tileString);
                break;
            }

            // buildAllMaps() runs many worker threads against tiles that
            // belong to the same map (and therefore share one dtNavMesh*) -
            // addTile()/removeTile() mutate that navmesh's internal tile
            // pool and aren't safe to call concurrently on the same
            // instance, so serialize just this registration+write+removal
            // sequence. The expensive rasterization/contour work above
            // already finished by this point and stays fully parallel.
            std::lock_guard<std::mutex> tileRegistrationLock(_tileRegistrationMutex);

            dtTileRef tileRef = 0;
            //printf("%s Adding tile to navmesh...\n", tileString);
            // DT_TILE_FREE_DATA tells detour to unallocate memory when the tile
            // is removed via removeTile()
            dtStatus dtResult = navMesh->addTile(navData, navDataSize, DT_TILE_FREE_DATA, 0, &tileRef);
            if (!tileRef || dtResult != DT_SUCCESS)
            {
                printf("%s Failed adding tile to navmesh!\n", tileString);
                break;
            }

            // file output
            char fileName[255];
            sprintf(fileName, "mmaps/%04u%02i%02i.mmtile", mapID, tileY, tileX);
            FILE* file = fopen(fileName, "wb");
            if (!file)
            {
                char message[1024];
                sprintf(message, "[Map %04u] Failed to open %s for writing!\n", mapID, fileName);
                perror(message);
                navMesh->removeTile(tileRef, NULL, NULL);
                break;
            }

            //printf("%s Writing to file...\n", tileString);

            // write header
            MmapTileHeader header;
            header.usesLiquids = m_terrainBuilder->usesLiquids() ? 1 : 0;
            header.size = uint32_t(navDataSize);
            fwrite(&header, sizeof(MmapTileHeader), 1, file);

            // write data
            fwrite(navData, sizeof(unsigned char), navDataSize, file);
            fclose(file);

            // now that tile is written to disk, we can unload it
            navMesh->removeTile(tileRef, NULL, NULL);
        }
        while (0);

        if (m_debugOutput)
        {
            // restore padding so that the debug visualization is correct
            for (int i = 0; i < iv.polyMesh->nverts; ++i)
            {
                unsigned short* v = &iv.polyMesh->verts[i*3];
                v[0] += (unsigned short)config.borderSize;
                v[2] += (unsigned short)config.borderSize;
            }

            iv.generateObjFile(mapID, tileX, tileY, meshData);
            iv.writeIV(mapID, tileX, tileY);
        }
    }

    /**************************************************************************/
    void MapBuilder::getTileBounds(uint32_t tileX, uint32_t tileY, float* verts, int vertCount, float* bmin, float* bmax)
    {
        // this is for elevation
        if (verts && vertCount)
            rcCalcBounds(verts, vertCount, bmin, bmax);
        else
        {
            bmin[1] = FLT_MIN;
            bmax[1] = FLT_MAX;
        }

        // this is for width and depth
        bmax[0] = (32 - int(tileX)) * GRID_SIZE;
        bmax[2] = (32 - int(tileY)) * GRID_SIZE;
        bmin[0] = bmax[0] - GRID_SIZE;
        bmin[2] = bmax[2] - GRID_SIZE;
    }

    /**************************************************************************/
    bool MapBuilder::shouldSkipMap(uint32_t mapID)
    {
        if (m_skipContinents)
            switch (mapID)
            {
                case 0:
                case 1:
                case 530:
                case 571:
                case 870:
                case 1116:
                    return true;
                default:
                    break;
            }

        if (m_skipJunkMaps)
            switch (mapID)
            {
                case 13:    // test.wdt
                case 25:    // ScottTest.wdt
                case 29:    // Test.wdt
                case 42:    // Colin.wdt
                case 169:   // EmeraldDream.wdt (unused, and very large)
                case 451:   // development.wdt
                case 573:   // ExteriorTest.wdt
                case 597:   // CraigTest.wdt
                case 605:   // development_nonweighted.wdt
                case 606:   // QA_DVD.wdt
                case 651:   // ElevatorSpawnTest.wdt
                case 1060:  // LevelDesignLand-DevOnly.wdt
                case 1181:  // PattyMackTestGarrisonBldgMap.wdt
                case 1264:  // Propland-DevOnly.wdt
                case 1270:  // devland3.wdt
                case 1427:  // PattyMackTestGarrisonBldgMap2.wdt
                    return true;
                default:
                    if (isTransportMap(mapID))
                        return true;
                    break;
            }

        if (m_skipBattlegrounds)
            switch (mapID)
            {
                case 30:    // Alterac Valley
                case 37:    // ?
                case 489:   // Warsong Gulch
                case 529:   // Arathi Basin
                case 566:   // Eye of the Storm
                case 607:   // Strand of the Ancients
                case 628:   // Isle of Conquest
                case 726:   // Twin Peaks
                case 727:   // Silvershard Mines
                case 761:   // The Battle for Gilneas
                case 968:   // Rated Eye of the Storm
                case 998:   // Temple of Kotmogu
                case 1010:  // CTF3
                case 1105:  // Deepwind Gorge
                case 1280:  // Southshore vs. Tarren Mill
                    return true;
                default:
                    break;
            }

        return false;
    }

    /**************************************************************************/
    bool MapBuilder::isTransportMap(uint32_t mapID)
    {
        switch (mapID)
        {
            // transport maps
            case 582:
            case 584:
            case 586:
            case 587:
            case 588:
            case 589:
            case 590:
            case 591:
            case 592:
            case 593:
            case 594:
            case 596:
            case 610:
            case 612:
            case 613:
            case 614:
            case 620:
            case 621:
            case 622:
            case 623:
            case 641:
            case 642:
            case 647:
            case 662:
            case 672:
            case 673:
            case 674:
            case 712:
            case 713:
            case 718:
            case 738:
            case 739:
            case 740:
            case 741:
            case 742:
            case 743:
            case 747:
            case 748:
            case 749:
            case 750:
            case 762:
            case 763:
            case 765:
            case 766:
            case 767:
            case 1113:
            case 1132:
            case 1133:
            case 1172:
            case 1173:
            case 1192:
            case 1231:
                return true;
            default:
                return false;
        }
    }

    /**************************************************************************/
    bool MapBuilder::shouldSkipTile(uint32_t mapID, uint32_t tileX, uint32_t tileY)
    {
        char fileName[255];
        sprintf(fileName, "mmaps/%04u%02i%02i.mmtile", mapID, tileY, tileX);
        FILE* file = fopen(fileName, "rb");
        if (!file)
            return false;

        MmapTileHeader header;
        int count = static_cast<int>(fread(&header, sizeof(MmapTileHeader), 1, file));
        fclose(file);
        if (count != 1)
            return false;

        if (header.mmapMagic != MMAP_MAGIC || header.dtVersion != uint32_t(DT_NAVMESH_VERSION))
            return false;

        if (header.mmapVersion != MMAP_VERSION)
            return false;

        return true;
    }
}
