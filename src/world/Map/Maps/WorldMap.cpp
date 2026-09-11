/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WorldMap.hpp"
#include "Objects/DynamicObject.hpp"
#include "Objects/Units/Creatures/CreatureGroups.h"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Creatures/Summons/Summon.hpp"
#include "Objects/Units/Unit.hpp"
#include "VMapFactory.h"
#include "MMapFactory.h"
#include "Macros/MapsMacros.hpp"
#include "shared/WoWGuid.hpp"
#include "MapScriptInterface.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Map/Management/MapMgr.hpp"
#include "InstanceMap.hpp"
#include "Server/Packets/SmsgUpdateWorldState.h"
#include "Server/Packets/SmsgDefenseMessage.h"
#include "Map/Area/AreaManagementGlobals.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Debugging/CrashHandler.hpp"
#include "Objects/Transporter.hpp"
#include "Objects/Units/Creatures/Summons/SummonDefines.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Spell/Definitions/SummonControlTypes.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"

#include <ctime>
#include <cstdarg>
#include <cmath>
#include <set>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "Logging/Logger.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Objects/Item.hpp"
#include "Server/EventMgr.h"
#include "Storage/WDB/WDBStructures.hpp"

using namespace AscEmu::Packets;
using namespace AscEmu::Threading;
using namespace visibility;

extern bool bServerShutdown;

WorldMap::WorldMap(BaseMap* baseMap, uint32_t id, uint32_t expiry, uint32_t InstanceId, uint8_t SpawnMode) : eventHolder(InstanceId), worldstateshandler(id),
    _terrain(std::make_unique<TerrainHolder>(id)), m_unloadTimer(expiry), m_baseMap(baseMap)
{
    // Map
    setSpawnMode(SpawnMode);
    setInstanceId(InstanceId);

    m_holder = &eventHolder;
    m_event_Instanceid = eventHolder.GetInstanceID();

    // Thread
    const std::string threadName("WorldMap - M" + std::to_string(getBaseMap()->getMapId()) + "|I" + std::to_string(getInstanceId()));
    m_thread = std::make_unique<AEThread>(threadName, [this](AEThread& /*thread*/) { this->runThread(); }, std::chrono::milliseconds(20), false);

    //lets initialize visibility distance for Continent
    WorldMap::initVisibilityDistance();

    // VisibilitySystem. Derive the spatial subscription radius from the actual
    // map visibility distance so candidate cells always cover the configured view.
    Config cfg;
    const int visibilityCells = visibility::SpatialIndex::cellsForRadius(getVisibilityDistance());
    cfg.defaultViewerRadius = visibilityCells;
    cfg.defaultActivatorRadius = cfg.defaultViewerRadius + 1;
    cfg.cellUnloadDelay = std::chrono::minutes(worldConfig.server.mapUnloadTime);

    spatialIndex_ = std::make_unique<visibility::SpatialIndex>();
    visibilitySystem_ = std::make_unique<visibility::VisibilitySystem>(*spatialIndex_, cfg);
    registry_ = std::make_unique<world::WorldObjectRegistry>();
    guids_ = std::make_unique<GuidAllocator>();

    factory_ = std::make_unique<ObjectFactory>(*this, *visibilitySystem_, *registry_, *guids_);
    spawnMgr_ = std::make_unique<SpawnManager>(*this, *visibilitySystem_, *factory_);

    hookVisibilityEvents();

    // Create script interface
    ScriptInterface = std::make_unique<MapScriptInterface>(*this, *spatialIndex_, *registry_, *factory_, *spawnMgr_);
}

void WorldMap::initialize()
{
    // Create Instance script
    loadInstanceScript();

    // Call script OnLoad virtual procedure
    if (getScript())
        getScript()->OnLoad();

    // Continent maps may contain autonomous DB spawns (for example long waypoint
    // creatures). Spawn only those definitions here without activating the rest of
    // their grid content. Instance maps are force-activated by MapMgr after their
    // persisted encounter state has been restored.
    if (!getBaseMap()->isInstanceableMap())
        spawnMgr_->spawnAutonomousSpawns();

    // load corpses
    sObjectMgr.loadCorpsesForInstance(this);
    worldstateshandler.InitWorldStates(sObjectMgr.getWorldStatesForMap(getBaseMap()->getMapId()));
    worldstateshandler.setObserver(this);
}

WorldMap::~WorldMap()
{
    m_thread->killAndJoin();
    sEventMgr.RemoveEvents(this);

    // Prevents a crash on map shutdown -Appled
    ScriptInterface = nullptr;

    if (mInstanceScript != nullptr)
        mInstanceScript->Destroy();

    _updates.clear();
    _processQueue.clear();
    Sessions.clear();

    MMAP::MMapFactory::createOrGetMMapManager()->unloadMapInstance(getBaseMap()->getMapId(), getInstanceId());

    sLogger.debug("WorldMap : Instance {} shut down. ({})", getInstanceId(), getBaseMap()->getMapName());
}

void WorldMap::startMapThread()
{
    m_terminateThread = false;
    m_lastUpdateTime = Util::getMSTime();
    m_thread->reboot();
}

void WorldMap::runThread()
{
    try
    {
        Do();
    }
    catch (const std::exception& e)
    {
        sLogger.failure(
            "WorldMap thread stopped: mapId={} instanceId={} map='{}' exception='{}'",
            getBaseMap() ? getBaseMap()->getMapId() : 0,
            getInstanceId(),
            getBaseMap() ? getBaseMap()->getMapName() : "<unknown>",
            e.what());
    }
    catch (...)
    {
        sLogger.failure(
            "WorldMap thread stopped: mapId={} instanceId={} map='{}' due to an unknown C++ exception.",
            getBaseMap() ? getBaseMap()->getMapId() : 0,
            getInstanceId(),
            getBaseMap() ? getBaseMap()->getMapName() : "<unknown>");
    }

    if (m_threadRunning)
    {
        m_threadRunning = false;
        m_thread->requestKill();
    }
}

void WorldMap::shutdownMapThread()
{
    pInstance = nullptr;
    m_terminateThread = true;
}

void WorldMap::unsafeKillMapThread()
{
    m_thread->killAndJoin();
}

bool WorldMap::isMapReadyForDelete() const
{
    return m_thread->isKilled() && m_thread->isDone();
}

void WorldMap::Do()
{
    using clock = std::chrono::steady_clock;
    using namespace std::chrono_literals;

    m_threadRunning = true;

    auto last = clock::now();

    while (!m_terminateThread)
    {
        const auto now = clock::now();
        const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last);
        last = now;

        try
        {
            update(static_cast<uint32_t>(diff.count()));
        }
        catch (const std::bad_alloc&)
        {
            sLogger.failure(
                "bad_alloc during WorldMap::update: mapId={}, instanceId={}",
                getBaseMap()->getMapId(),
                getInstanceId());

            throw;
        }

        try
        {
            delayedUpdate(diff);
        }
        catch (const std::bad_alloc&)
        {
            sLogger.failure(
                "bad_alloc during WorldMap::delayedUpdate: mapId={}, instanceId={}",
                getBaseMap()->getMapId(),
                getInstanceId());

            throw;
        }

        std::this_thread::sleep_for(10ms);
    }

    m_threadRunning = false;
    m_thread->requestKill();
}

void WorldMap::update(uint32_t t_diff)
{
    using namespace std::chrono_literals;

    processMapTasks();

    const auto dt = std::chrono::milliseconds{ t_diff };
    const auto instanceId = getInstanceId();

    eventHolder.Update(t_diff);
    _dynamicTree.update(t_diff);

    spawnMgr_->processQueuedAdds();

    m_visibilityAccum += dt;
    if (m_visibilityAccum >= 100ms)
    {
        visibilitySystem_->tick(m_visibilityAccum);
        m_visibilityAccum = 0ms;
    }

    m_respawnAccum += dt;
    if (m_respawnAccum >= 1000ms)
    {
        spawnMgr_->processRespawns();
        m_respawnAccum = 0ms;
    }

    updateAll(dt);

    m_sessionAccum += dt;
    if (m_sessionAccum >= 100ms)
    {
        std::vector<WorldSession*> sessions;
        {
            std::lock_guard<std::mutex> sessionGuard(m_sessionMutex);
            sessions.assign(Sessions.begin(), Sessions.end());
        }

        for (WorldSession* session : sessions)
        {
            if (session == nullptr)
                continue;

            if (session->GetInstance() != instanceId)
            {
                removeSession(session);
                continue;
            }

            Player* player = session->GetPlayer();
            if (player == nullptr || player->getWorldMap() != this)
            {
                removeSession(session);
                continue;
            }

            const uint8_t result = session->Update(instanceId);
            if (result != 0)
            {
                removeSession(session);

                if (result == 1)
                    sWorld.deleteSession(session);
            }
        }

        m_sessionAccum = 0ms;
    }

    processPendingUnitAwareness();
    processPendingVisibilityChanges();

    updateObjects();
    drainDeferredDestroy();

    finalizeSpatialPerformanceSample(dt);
}

void WorldMap::finalizeSpatialPerformanceSample(std::chrono::milliseconds diff)
{
    if (!spatialIndex_ || !visibilitySystem_)
        return;

    const auto spatial = spatialIndex_->takePerformanceCounters();
    const auto visibility = visibilitySystem_->takePerformanceCounters();

    auto& window = m_spatialPerformanceWindow;
    const uint64_t elapsedMs = diff.count() > 0 ? static_cast<uint64_t>(diff.count()) : 0ULL;
    window.windowMs += elapsedMs;
    ++window.ticks;

    window.spatialQueries += spatial.queries;
    window.spatialCandidates += spatial.candidates;
    window.spatialTryGets += spatial.tryGets;
    window.spatialMoves += spatial.moves;
    window.cellMoves += spatial.cellMoves;

    window.visibilityPairChecks += visibility.pairChecks;
    window.visibilityCreates += m_visibilityCreatesThisTick;
    window.visibilityDestroys += m_visibilityDestroysThisTick;
    window.ringSubscribes += visibility.ringSubscribes;
    window.ringUnsubscribes += visibility.ringUnsubscribes;

    window.awarenessQueries += m_awarenessQueriesThisTick;
    window.awarenessCandidates += m_awarenessCandidatesThisTick;


    window.maxSpatialCandidatesPerTick = std::max(window.maxSpatialCandidatesPerTick, spatial.candidates);
    window.maxSpatialTryGetsPerTick = std::max(window.maxSpatialTryGetsPerTick, spatial.tryGets);
    window.maxVisibilityPairChecksPerTick = std::max(window.maxVisibilityPairChecksPerTick, visibility.pairChecks);
    window.maxAwarenessCandidatesPerTick = std::max(window.maxAwarenessCandidatesPerTick, m_awarenessCandidatesThisTick);

    m_awarenessQueriesThisTick = 0;
    m_awarenessCandidatesThisTick = 0;
    m_visibilityCreatesThisTick = 0;
    m_visibilityDestroysThisTick = 0;


    m_spatialPerformanceAccum += diff;
    if (m_spatialPerformanceAccum >= std::chrono::milliseconds(1000))
    {
        m_lastSpatialPerformance = window;
        window = {};
        m_spatialPerformanceAccum = std::chrono::milliseconds(0);
    }
}

void WorldMap::delayedUpdate(std::chrono::milliseconds diff)
{
    thread_local std::vector<Transporter*> s_trans;

    registry_->snapshotTransporters(s_trans);
    registry_->forEachPinned(s_trans, [&](Transporter& t)
        {
        t.delayedUpdate(static_cast<unsigned long>(diff.count()));
        });

}

void WorldMap::updateAll(std::chrono::milliseconds diff)
{
    using namespace std::chrono_literals;
    using clock = std::chrono::steady_clock;

    thread_local std::vector<Player*>        s_players;
    thread_local std::vector<Creature*>      s_creatures;
    thread_local std::vector<GameObject*>    s_gos;
    thread_local std::vector<Transporter*>   s_trans;
    thread_local std::vector<DynamicObject*> s_dyns;
    thread_local std::vector<Pet*>           s_pets;

    // Transporters:
    registry_->snapshotTransporters(s_trans);
    registry_->forEachPinned(s_trans, [&](Transporter& t)
        {
            const auto last = t.getLastUpdate();
            const auto passed = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - last);

            if (passed >= 100ms)
            {
                t.Update(static_cast<uint32_t>(passed.count()));
                t.setLastUpdate(clock::now());
            }
        });

    // DynamicObjects
    m_dynamicObjectAccum += diff;
    if (m_dynamicObjectAccum >= 200ms)
    {
        const auto dynamicObjectDiff = static_cast<uint32_t>(m_dynamicObjectAccum.count());
        registry_->snapshotDynamicObjects(s_dyns);
        registry_->forEachPinned(s_dyns, [&](DynamicObject& d)
            {
                d.updateLifetime(dynamicObjectDiff);
            });
        m_dynamicObjectAccum = 0ms;
    }

    // Creatures
    m_creaturesAccum += diff;
    if (m_creaturesAccum >= 100ms)
    {
        registry_->snapshotCreatures(s_creatures);
        registry_->forEachPinned(s_creatures, [&](Creature& c)
            {
                if (c.IsInWorld())
                    c.Update(static_cast<uint32_t>(m_creaturesAccum.count()));
            });
        m_creaturesAccum = 0ms;
    }

    // GameObjects
    m_gameObjectAccum += diff;
    if (m_gameObjectAccum >= 100ms)
    {
        registry_->snapshotGameObjects(s_gos);
        registry_->forEachPinned(s_gos, [&](GameObject& g)
            {
                if (g.IsInWorld())
                    g.Update(static_cast<uint32_t>(m_gameObjectAccum.count()));
            });
        m_gameObjectAccum = 0ms;
    }

    // Players
    m_playersAccum += diff;
    if (m_playersAccum >= 50ms)
    {
        registry_->snapshotPlayers(s_players);
        registry_->forEachPinned(s_players, [&](Player& p)
            {
                p.Update(static_cast<uint32_t>(m_playersAccum.count()));
            });
        m_playersAccum = 0ms;
    }

    // Pets
    m_petsAccum += diff;
    if (m_petsAccum >= 100ms)
    {
        registry_->snapshotPets(s_pets);
        registry_->forEachPinned(s_pets, [&](Pet& pet)
            {
                pet.Update(static_cast<uint32_t>(m_petsAccum.count()));
            });
        m_petsAccum = 0ms;
    }

}

void WorldMap::queueMapTask(std::function<void()> task)
{
    if (!task)
        return;

    std::lock_guard<std::mutex> guard(mapTaskMutex_);
    mapTasks_.push_back(std::move(task));
}

void WorldMap::processMapTasks()
{
    std::deque<std::function<void()>> tasks;

    {
        std::lock_guard<std::mutex> guard(mapTaskMutex_);
        if (mapTasks_.empty())
            return;

        tasks.swap(mapTasks_);
    }

    for (auto& task : tasks)
    {
        if (task)
            task();
    }
}

bool WorldMap::deferDestroy(Object* object)
{
    if (!object)
        return false;

    std::lock_guard<std::mutex> destroyGuard(deferredDestroyMutex_);

    // Objects currently being destroyed must never be queued again.
    // This can happen recursively when destructors clean up related
    // objects and trigger additional destruction requests.
    if (destroying_.contains(object))
        return false;

    // The set also prevents duplicate destroy requests while the object
    // is still waiting for the next deferred-destroy pass.
    return deferred_destroy_.insert(object).second;
}

void WorldMap::drainDeferredDestroy()
{
    std::vector<Object*> destroyNow;

    {
        // Always lock the update queues before the destroy state.
        // objectUpdated() already uses the update mutex, so keeping this
        // order avoids introducing an inverse lock order elsewhere.
        std::unique_lock<std::mutex> updateGuard(m_updateMutex);
        std::unique_lock<std::mutex> destroyGuard(deferredDestroyMutex_);

        if (deferred_destroy_.empty())
            return;

        destroyNow.reserve(deferred_destroy_.size());

        // Before any object is destroyed, remove all remaining raw-pointer
        // references from the legacy update queues. Move every pending object
        // into destroying_ so recursive or concurrent deferDestroy() calls
        // cannot queue the same object again while its destructor is running.
        //
        // The actual delete must happen after both mutexes are released:
        // object destructors may themselves schedule other objects for deferred
        // destruction (pets, summons, totems, etc..)
        for (Object* object : deferred_destroy_)
        {
            _updates.erase(object);

            if (object && object->isPlayer())
                _processQueue.erase(static_cast<Player*>(object));

            destroying_.insert(object);
            destroyNow.push_back(object);
        }

        deferred_destroy_.clear();
    }

    // Never execute Object destructors while holding deferredDestroyMutex_.
    // Destruction may recursively call deferDestroy() for related objects.
    // Those objects are queued normally and will be processed by a later pass.
    for (Object* object : destroyNow)
    {
        delete object;

        std::lock_guard<std::mutex> guard(deferredDestroyMutex_);
        destroying_.erase(object);
    }
}

std::map<uint32_t, Player*> WorldMap::getPlayers() const
{
    std::vector<Player*> players;
    if (registry_)
        registry_->snapshotPlayers(players);

    std::map<uint32_t, Player*> out;
    for (Player* player : players)
    {
        if (player)
            out.emplace(player->getGuidLow(), player);
    }
    return out;
}

std::vector<Creature*> WorldMap::getCreatures() const
{
    std::vector<Creature*> out;
    if (registry_)
        registry_->snapshotCreatures(out);
    return out;
}

std::vector<GameObject*> WorldMap::getGameObjects() const
{
    std::vector<GameObject*> out;
    if (registry_)
        registry_->snapshotGameObjects(out);
    return out;
}

uint32_t WorldMap::getPlayerCount() const
{
    return registry_ ? static_cast<uint32_t>(registry_->countPlayers()) : 0;
}

Creature* WorldMap::getSqlIdCreature(uint32_t spawnId) const
{
    std::vector<Creature*> creatures;
    if (registry_)
        registry_->snapshotCreatures(creatures);

    for (Creature* creature : creatures)
        if (creature && creature->getSpawnId() == spawnId)
            return creature;
    return nullptr;
}

GameObject* WorldMap::getSqlIdGameObject(uint32_t spawnId) const
{
    std::vector<GameObject*> gameObjects;
    if (registry_)
        registry_->snapshotGameObjects(gameObjects);

    for (GameObject* go : gameObjects)
        if (go && go->getSpawnId() == spawnId)
            return go;
    return nullptr;
}

Unit* WorldMap::getUnit(const WoWGuid& g) const
{
    switch (g.getHighType())
    {
        case HighGuid::Unit:
        case HighGuid::Vehicle:
            return getCreature(g);
        case HighGuid::Player:
            return getPlayer(g);
        case HighGuid::Pet:
            return getPet(g);
    }

    return nullptr;
}

inline int MinGridToNavIdx(int g_min)
{
    return Terrain::TilesCount - 1 - g_min;
}

void WorldMap::navAcquireGrid(int gid)
{
    auto [gx, gy] = unpackGridId(gid);

    if (!worldConfig.terrainCollision.isCollisionEnabled) return;

    std::lock_guard<std::mutex> g(nav_mtx_);
    auto& ref = nav_gridRefs_[gid];
    if (ref++ != 0) return;

    auto* vmap = VMAP::VMapFactory::createOrGetVMapManager();
    auto* mmap = MMAP::MMapFactory::createOrGetMMapManager();

    const int nx = MinGridToNavIdx(gx);
    const int ny = MinGridToNavIdx(gy);

    getTerrain()->loadTile(nx, ny);

    const std::string vpath = worldConfig.server.dataDir + "vmaps";
    const std::string mpath = worldConfig.server.dataDir + "mmaps";
    const int mapId = getBaseMap()->getMapId();

    vmap->loadMap(vpath.c_str(), mapId, nx, ny);
    mmap->loadMap(mpath, mapId, nx, ny);
}

void WorldMap::navReleaseGrid(int gid)
{
    auto [gx, gy] = unpackGridId(gid);

    if (!worldConfig.terrainCollision.isCollisionEnabled) return;

    std::lock_guard<std::mutex> g(nav_mtx_);
    auto it = nav_gridRefs_.find(gid);
    if (it == nav_gridRefs_.end()) return;
    if (--(it->second) != 0) return;

    const int nx = MinGridToNavIdx(gx);
    const int ny = MinGridToNavIdx(gy);

    getTerrain()->unloadTile(nx, ny);

    auto* vmap = VMAP::VMapFactory::createOrGetVMapManager();
    auto* mmap = MMAP::MMapFactory::createOrGetMMapManager();
    const int mapId = getBaseMap()->getMapId();

    vmap->unloadMap(mapId, nx, ny);
    mmap->unloadMap(mapId, nx, ny);

    nav_gridRefs_.erase(it);
}


std::vector<int> WorldMap::collectTerrainGridsForArea(uint32_t id, bool matchZone)
{
    std::vector<int> grids;
    if (!id || !getTerrain())
        return grids;

    grids.reserve(32);

    constexpr int samplesPerGrid = 16;
    constexpr float sampleStep = Terrain::TileSize / static_cast<float>(samplesPerGrid);

    for (int gy = 0; gy < Terrain::TilesCount; ++gy)
    {
        for (int gx = 0; gx < Terrain::TilesCount; ++gx)
        {
            const int tileX = MinGridToNavIdx(gx);
            const int tileY = MinGridToNavIdx(gy);
            if (!getTerrain()->areTilesValid(tileX, tileY))
                continue;

            getTerrain()->loadTile(tileX, tileY);
            TerrainTile* tile = getTerrain()->getTile(tileX, tileY);

            bool matches = false;
            if (tile)
            {
                const float minX = Terrain::MinX + static_cast<float>(gx) * Terrain::TileSize;
                const float minY = Terrain::MinY + static_cast<float>(gy) * Terrain::TileSize;

                for (int sy = 0; sy < samplesPerGrid && !matches; ++sy)
                {
                    const float y = minY + (static_cast<float>(sy) + 0.5f) * sampleStep;

                    for (int sx = 0; sx < samplesPerGrid; ++sx)
                    {
                        const float x = minX + (static_cast<float>(sx) + 0.5f) * sampleStep;
                        const uint16_t areaId = tile->m_map.getArea(x, y);
                        if (!areaId || areaId == 0xFFFF)
                            continue;

                        const auto* area = MapManagement::AreaManagement::AreaStorage::getAreaById(areaId);
                        if (!area)
                            continue;

                        const uint32_t value = matchZone ? (area->zone ? area->zone : area->id) : area->id;
                        if (value == id)
                        {
                            matches = true;
                            break;
                        }
                    }
                }
            }

            getTerrain()->unloadTile(tileX, tileY);

            if (matches)
                grids.push_back(visibility::packGridId(gx, gy));
        }
    }

    return grids;
}

bool WorldMap::gridMatchesForcedRegion(int gid)
{
    if ((forcedZones_.empty() && forcedAreas_.empty()) || !getTerrain())
        return false;

    const auto [gx, gy] = visibility::unpackGridId(gid);
    if (gx < 0 || gy < 0 || gx >= Terrain::TilesCount || gy >= Terrain::TilesCount)
        return false;

    const int tileX = MinGridToNavIdx(gx);
    const int tileY = MinGridToNavIdx(gy);
    if (!getTerrain()->areTilesValid(tileX, tileY))
        return false;

    getTerrain()->loadTile(tileX, tileY);
    TerrainTile* tile = getTerrain()->getTile(tileX, tileY);
    if (!tile)
    {
        getTerrain()->unloadTile(tileX, tileY);
        return false;
    }

    constexpr int samplesPerGrid = 16;
    constexpr float sampleStep = Terrain::TileSize / static_cast<float>(samplesPerGrid);
    const float minX = Terrain::MinX + static_cast<float>(gx) * Terrain::TileSize;
    const float minY = Terrain::MinY + static_cast<float>(gy) * Terrain::TileSize;

    bool matches = false;
    for (int sy = 0; sy < samplesPerGrid && !matches; ++sy)
    {
        const float y = minY + (static_cast<float>(sy) + 0.5f) * sampleStep;
        for (int sx = 0; sx < samplesPerGrid; ++sx)
        {
            const float x = minX + (static_cast<float>(sx) + 0.5f) * sampleStep;
            const uint16_t areaId = tile->m_map.getArea(x, y);
            if (!areaId || areaId == 0xFFFF)
                continue;

            const auto* area = MapManagement::AreaManagement::AreaStorage::getAreaById(areaId);
            if (!area)
                continue;

            const uint32_t zoneId = area->zone ? area->zone : area->id;
            if (forcedZones_.count(zoneId) != 0 || forcedAreas_.count(area->id) != 0)
            {
                matches = true;
                break;
            }
        }
    }

    getTerrain()->unloadTile(tileX, tileY);
    return matches;
}

bool WorldMap::shouldRuleForceGrid(int gid)
{
    if (gid < 0 || gid >= Terrain::TilesCount * Terrain::TilesCount)
        return false;

    if (forceAllGridsActive_)
        return true;

    return gridMatchesForcedRegion(gid);
}

bool WorldMap::ensureRuleGridPinned(int gid)
{
    if (!shouldRuleForceGrid(gid))
        return false;

    if (!rulePinnedGrids_.insert(gid).second)
        return true;

    visibilitySystem_->pinGrid(gid);
    return true;
}

void WorldMap::reevaluateRuleGridPins()
{
    for (auto it = rulePinnedGrids_.begin(); it != rulePinnedGrids_.end();)
    {
        if (shouldRuleForceGrid(*it))
        {
            ++it;
            continue;
        }

        visibilitySystem_->unpinGrid(*it);
        it = rulePinnedGrids_.erase(it);
    }
}

void WorldMap::onGridMaterialized(int gid)
{
    ensureRuleGridPinned(gid);
}

bool WorldMap::setGridForcedActive(int gid, bool active)
{
    if (gid < 0 || gid >= Terrain::TilesCount * Terrain::TilesCount)
        return false;

    if (active)
    {
        if (!forcedExplicitGrids_.insert(gid).second)
            return true;

        visibilitySystem_->pinGrid(gid);
        return true;
    }

    if (forcedExplicitGrids_.erase(gid) == 0)
        return false;

    visibilitySystem_->unpinGrid(gid);
    return true;
}

size_t WorldMap::setAllGridsForcedActive(bool active)
{
    if (forceAllGridsActive_ == active)
        return rulePinnedGrids_.size();

    forceAllGridsActive_ = active;

    if (active)
    {
        // Load every grid that already has spawn definitions. Empty grids stay sparse;
        // if one becomes used later, onGridMaterialized() pins it automatically.
        for (int gid : spawnMgr_->definedGridIds())
            ensureRuleGridPinned(gid);

        // Preserve grids that are already allocated/active even when they currently
        // have no static spawn definition.
        for (auto const& [gid, grid] : spatialIndex_->gridPointersSnapshot())
        {
            if (grid)
                ensureRuleGridPinned(gid);
        }
    }
    else
    {
        reevaluateRuleGridPins();
    }

    return rulePinnedGrids_.size();
}

size_t WorldMap::setZoneGridsForcedActive(uint32_t zoneId, bool active)
{
    if (!zoneId)
        return 0;

    if (active)
    {
        if (!forcedZones_.insert(zoneId).second)
            return rulePinnedGrids_.size();

        for (int gid : collectTerrainGridsForArea(zoneId, true))
            ensureRuleGridPinned(gid);
    }
    else
    {
        if (forcedZones_.erase(zoneId) == 0)
            return rulePinnedGrids_.size();

        reevaluateRuleGridPins();
    }

    return rulePinnedGrids_.size();
}

size_t WorldMap::setAreaGridsForcedActive(uint32_t areaId, bool active)
{
    if (!areaId)
        return 0;

    if (active)
    {
        if (!forcedAreas_.insert(areaId).second)
            return rulePinnedGrids_.size();

        for (int gid : collectTerrainGridsForArea(areaId, false))
            ensureRuleGridPinned(gid);
    }
    else
    {
        if (forcedAreas_.erase(areaId) == 0)
            return rulePinnedGrids_.size();

        reevaluateRuleGridPins();
    }

    return rulePinnedGrids_.size();
}

bool WorldMap::isGridForcedActive(int gid) const
{
    return visibilitySystem_ && visibilitySystem_->isGridPinned(gid);
}

bool WorldMap::canUnload(uint32_t diff)
{
    if (getPlayerCount())
        return false;

    if (!m_unloadTimer)
        return false;

    if (m_unloadTimer <= diff)
        return true;

    m_unloadTimer -= diff;
    return false;
}

void WorldMap::unloadAll(bool onShutdown/* = false*/)
{
    if (registry_->countPlayers())
        return;

    if (onShutdown)
        return;

    if (getInstanceId() == 0)
        sMapMgr.addMapToRemovePool(this);
    else
        sMapMgr.removeInstance(getInstanceId());
}

void WorldMap::initVisibilityDistance()
{
    //init visibility for continents
    setVisibilityDistance(visibility::Cell::Size * worldConfig.server.mapCellNumber);
}

void WorldMap::syncVisibilitySubscriptionRadius()
{
    if (!visibilitySystem_)
        return;

    visibilitySystem_->setDefaultSubscriptionRadius(
        visibility::SpatialIndex::cellsForRadius(getVisibilityDistance()));
}

void WorldMap::outOfMapBoundariesTeleport(Object* object)
{
    if (object->isPlayer())
    {
        Player* player = static_cast<Player*>(object);

        if (player->getBindMapId() != getBaseMap()->getMapId())
        {
            player->safeTeleport(player->getBindMapId(), 0, player->getBindPosition());
            player->getSession()->systemMessage("Teleported you to your hearthstone location as you were out of the map boundaries.");
        }
        else
        {
            object->GetPositionV()->changeCoords(player->getBindPosition());
            player->getSession()->systemMessage("Teleported you to your hearthstone location as you were out of the map boundaries.");
            player->sendTeleportAckPacket(player->getBindPosition());
        }
    }
    else
    {
        object->GetPositionV()->changeCoords({ 0, 0, 0, 0 });
    }
}

void WorldMap::hookVisibilityEvents()
{
    visibilitySystem_->onGridActivated([this](int gid)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_MAP_CELL, "Grid Activated {} ", gid);

            // Apply persistent map/zone/area activation rules to grids that become
            // active through normal runtime subscriptions.
            onGridMaterialized(gid);

            // Navigation
            navAcquireGrid(gid);
            
            // Spawns
            spawnMgr_->onGridActivated(gid);
        });

    visibilitySystem_->onGridDeactivated([this](int gid)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_MAP_CELL, "Grid Deactivated {}", gid);
        });

    visibilitySystem_->onGridUnload([this](int gid)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_MAP_CELL, "Grid Unloaded {} ", gid);

            // Spawns
            spawnMgr_->onGridUnload(gid);

            // Navigation
            navReleaseGrid(gid);
        });

    visibilitySystem_->onGridChanged([this](const WoWGuid& g, int oldGid, int newGid)
        {
            spawnMgr_->onGridChanged(g, oldGid, newGid);
        });

    visibilitySystem_->onInterestProfileChanged([this](const WoWGuid& g, const visibility::InterestProfile& profile)
        {
            spawnMgr_->onInterestProfileChanged(g, profile.autonomous);
        });

    // A9 Packets...
    visibilitySystem_->onBecameVisible([this](WoWGuid viewer, WoWGuid object)
        {
            onObjectBecameVisible(viewer, object);
        });

    visibilitySystem_->onBecameHidden([this](WoWGuid viewer, WoWGuid object)
        {
            onObjectBecameHidden(viewer, object);
        });
}

void WorldMap::addSession(WorldSession* session)
{
    if (!session)
        return;

    std::lock_guard<std::mutex> sessionGuard(m_sessionMutex);
    Sessions.insert(session);
}

void WorldMap::removeSession(WorldSession* session)
{
    if (!session)
        return;

    std::lock_guard<std::mutex> sessionGuard(m_sessionMutex);
    Sessions.erase(session);
}

bool WorldMap::onPlayerEnter(Player* plr)
{
    if (!plr)
        return false;

    WorldSession* session = plr->getSession();
    if (!session)
        return false;

    // A player must be detached before entering another map.
    if (plr->IsInWorld() || plr->getWorldMap() != nullptr)
    {
        sLogger.failure("Refusing to attach player '{}' ({}) to map {} instance {} while it is still attached to another WorldMap",
            plr->getName(), plr->getGuidLow(), getBaseMap()->getMapId(), getInstanceId());
        return false;
    }

    // Check the map-specific entry rules before attaching the player.
    if (!addPlayerToMap(plr))
    {
        sLogger.failure("Failed to add player '{}' ({}) to map {} instance {}",
            plr->getName(), plr->getGuidLow(), getBaseMap()->getMapId(), getInstanceId());
        return false;
    }

    // Attach the player before the map starts updating the session.
    factory_->attachToWorld(plr);

    // From here on the session is updated by this map.
    session->SetMapUpdateOwner(getInstanceId());
    addSession(session);
    sWorld.removeGlobalSession(session);

    // Let nearby creatures react even if nothing is moving yet.
    queueUnitAwareness(plr, UnitAwarenessSignal::EnteredWorld);

    return true;
}

void WorldMap::onPlayerLeave(Player* plr)
{
    if (!plr || plr->getWorldMap() != this)
        return;

    // Use the same cleanup path for logout and map transfers.
    removeSession(plr->getSession());
    removePlayerFromMap(plr);
    factory_->detachFromWorld(plr);
}

void WorldMap::onObjectMoved(Object* obj)
{
    if (!obj || !visibilitySystem_) 
        return;

    if (obj->getWorldMap() != this)
        return;
    
    // Fix invalid positions before updating the spatial index.
    if (!std::isfinite(obj->GetPositionX()) || !std::isfinite(obj->GetPositionY()) ||
        obj->GetPositionX() < Terrain::MinX || obj->GetPositionX() > Terrain::MaxX ||
        obj->GetPositionY() < Terrain::MinY || obj->GetPositionY() > Terrain::MaxY)
    {
        outOfMapBoundariesTeleport(obj);
    }

    // Update Gameobject Collision
    if (obj->isGameObject())
        obj->ToGameObject()->updateModelPosition();

    // Retrieve the visibility-system handle for this object
    auto h = spatialIndex_->handleByGuid(obj->GetNewGUID());
    if (!h.id)
        return;

    Unit* controlledViewer = obj->ToUnit();
    if (controlledViewer && controlledViewer->m_playerControler)
    {
        Player* controller = controlledViewer->m_playerControler;
        if (controller->IsInWorld() && controller->getWorldMap() == this)
        {
            // Possession / Eyes of the Beast must never depend on a one-shot role
            // assignment done when control starts. Ensure the moving unit is still
            // a canonical viewer + activator before transferring its subscriptions
            // to the new cell/grid.
            setVisibilityRecipientForViewer(obj->GetNewGUID(), controller);
            const auto profile = visibilitySystem_->buildInterestProfile(obj);
            visibilitySystem_->applyInterestProfile(h, profile);
        }
    }

    const LocationVector now = obj->GetPosition();

    // Always update the spatial index. This keeps slot position, cell membership
    // and the object's own near-cache correct for scripts, spells, AI and area
    // helpers. Expensive visibility/interest refreshes are throttled inside
    // VisibilitySystem::onObjectMoved().
    auto move = spatialIndex_->moveObject(h, now);
    if (move.valid && move.gridChanged)
        onGridMaterialized(move.newGrid);

    visibilitySystem_->onObjectMoved(h, move);

    // Spatial position is updated for every relocation, but normal AI awareness
    // only needs a new evaluation after a meaningful displacement. DynamicObject
    // area membership remains exact: while area effects exist, movement still gets
    // an unthrottled area-only signal that never triggers nearby AI acquisition.
    if (Unit* movedUnit = obj->ToUnit(); movedUnit && move.valid)
    {
        if (maxDynamicObjectTargetRadius_ > 0.0f || !movedUnit->getDynamicObjectTargets().empty())
            queueUnitAwareness(movedUnit, UnitAwarenessSignal::AreaMovement);

        queueUnitAwareness(movedUnit, UnitAwarenessSignal::Movement);
    }

    if (move.valid && obj->getObjectTypeId() == TYPEID_DYNAMICOBJECT)
        refreshDynamicObjectTargets(static_cast<DynamicObject*>(obj));

    // A player-controlled remote viewpoint is driven directly by client movement.
    // Re-evaluate its complete remote view and flush it immediately; the controlling
    // player's physical body may remain several grids behind.
    if (controlledViewer && controlledViewer->m_playerControler)
    {
        visibilitySystem_->refreshObjectVisibility(h);
        processPendingVisibilityChangesForViewer(obj->GetNewGUID(), 512, 512);
    }
}

void WorldMap::queueUnitAwareness(Unit* unit, UnitAwarenessSignal reason)
{
    if (!unit || !unit->IsInWorld() || unit->getWorldMap() != this || reason == UnitAwarenessSignal::None)
        return;

    constexpr float movementRefreshDistanceSq = 2.0f * 2.0f;
    const uint64_t rawGuid = unit->GetNewGUID().getRawGuid();
    const LocationVector currentPosition = unit->GetPosition();
    const bool movementOnly = reason == UnitAwarenessSignal::Movement;
    const bool areaMovementOnly = reason == UnitAwarenessSignal::AreaMovement;

    std::lock_guard<std::mutex> guard(unitAwarenessMutex_);

    auto positionIt = lastMovementAwarenessPosition_.find(rawGuid);
    if (movementOnly && positionIt != lastMovementAwarenessPosition_.end())
    {
        const float dx = currentPosition.x - positionIt->second.x;
        const float dy = currentPosition.y - positionIt->second.y;
        if ((dx * dx + dy * dy) < movementRefreshDistanceSq)
            return;
    }

    // Accepted AI movement and explicit state changes become the new movement
    // baseline. AreaMovement is deliberately excluded because it exists only to
    // keep DynamicObject area boundaries exact and must not reset the AI threshold.
    if (!areaMovementOnly)
        lastMovementAwarenessPosition_[rawGuid] = currentPosition;

    auto& mask = pendingUnitAwareness_[rawGuid];
    mask |= static_cast<uint16_t>(reason);
}

void WorldMap::processPendingUnitAwareness()
{
    // Keep the cross-thread critical section tiny. Most producers run on the map
    // thread, but login/world-attach and a few legacy event paths can enqueue from
    // elsewhere. The expensive spatial/script work always happens after unlock.
    {
        std::lock_guard<std::mutex> guard(unitAwarenessMutex_);
        if (pendingUnitAwareness_.empty())
            return;

        // Reuse both maps' bucket capacity across ticks.
        processingUnitAwareness_.clear();
        processingUnitAwareness_.swap(pendingUnitAwareness_);
    }

    thread_local std::vector<Creature*> observerCreatures;
    thread_local std::vector<Unit*> nearbyUnits;
    thread_local std::vector<DynamicObject*> nearbyDynamicObjects;
    thread_local std::vector<uint64_t> currentDynamicObjects;
    thread_local std::unordered_set<uint64_t> processedDynamicObjects;

    // Automatic creature aggro is capped at 45 yards; the shared awareness
    // range keeps a 5-yard safety margin while avoiding unnecessary wider scans.
    const float awarenessSearchRange = std::min<float>(
        getVisibilityDistance(), AIConstants::AutomaticAwarenessSearchRange);

    for (const auto& [rawGuid, rawMask] : processingUnitAwareness_)
    {
        Unit* source = getUnit(WoWGuid(rawGuid));
        if (!source || !source->IsInWorld() || source->getWorldMap() != this)
            continue;

        const auto reason = static_cast<UnitAwarenessSignal>(rawMask);

        const bool refreshDynamicAreaTargets =
            hasAwarenessSignal(reason, UnitAwarenessSignal::Movement) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::AreaMovement) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::EnteredWorld) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::Respawned) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::PhaseChanged) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::StealthChanged) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::InvisibilityChanged) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::DetectionChanged) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::VisibilityChanged) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::ReactionChanged) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::ControlStateChanged) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::CombatEligibilityChanged);

        if (refreshDynamicAreaTargets)
        {
            processedDynamicObjects.clear();

            // First re-evaluate areas that currently own an aura on this unit.
            // They must be processed even if the unit moved completely outside
            // their radius in a single relocation.
            currentDynamicObjects.assign(source->getDynamicObjectTargets().begin(), source->getDynamicObjectTargets().end());
            for (uint64_t dynamicGuid : currentDynamicObjects)
            {
                if (DynamicObject* dynamicObject = getDynamicObject(WoWGuid(dynamicGuid)))
                {
                    processedDynamicObjects.insert(dynamicGuid);
                    dynamicObject->considerTarget(source);
                }
                else
                {
                    source->removeDynamicObjectTarget(dynamicGuid);
                }
            }

            // Then discover areas the unit may just have entered. The high-water
            // radius guarantees that every active DynamicObject capable of
            // containing the current position is included in this spatial query.
            if (maxDynamicObjectTargetRadius_ > 0.0f)
            {
                nearbyDynamicObjects.clear();
                spatialIndex_->collectObjectsInRange<DynamicObject>(
                    source->GetPosition(), maxDynamicObjectTargetRadius_, nearbyDynamicObjects);
                ++m_awarenessQueriesThisTick;
                m_awarenessCandidatesThisTick += nearbyDynamicObjects.size();

                for (DynamicObject* dynamicObject : nearbyDynamicObjects)
                {
                    if (!dynamicObject || !dynamicObject->IsInWorld() || dynamicObject->getWorldMap() != this)
                        continue;

                    const uint64_t dynamicGuid = dynamicObject->getGuid();
                    if (!processedDynamicObjects.insert(dynamicGuid).second)
                        continue;

                    dynamicObject->considerTarget(source);
                }
            }
        }

        const bool notifyNearbyObservers =
            hasAwarenessSignal(reason, UnitAwarenessSignal::Movement) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::EnteredWorld) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::Respawned) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::PhaseChanged) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::StealthChanged) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::InvisibilityChanged) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::VisibilityChanged) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::ReactionChanged) ||
            hasAwarenessSignal(reason, UnitAwarenessSignal::CombatEligibilityChanged);

        const bool refreshSourceCreature =
            source->isCreature() &&
            (hasAwarenessSignal(reason, UnitAwarenessSignal::Movement) ||
             hasAwarenessSignal(reason, UnitAwarenessSignal::EnteredWorld) ||
             hasAwarenessSignal(reason, UnitAwarenessSignal::Respawned) ||
             hasAwarenessSignal(reason, UnitAwarenessSignal::PhaseChanged) ||
             hasAwarenessSignal(reason, UnitAwarenessSignal::DetectionChanged) ||
             hasAwarenessSignal(reason, UnitAwarenessSignal::ReactionChanged) ||
             hasAwarenessSignal(reason, UnitAwarenessSignal::ControlStateChanged) ||
             hasAwarenessSignal(reason, UnitAwarenessSignal::CombatEligibilityChanged) ||
             hasAwarenessSignal(reason, UnitAwarenessSignal::WarmupComplete));

        // A creature movement/state change is naturally bidirectional: nearby
        // creatures may notice the source and the source may notice nearby units.
        // Collect the unit neighborhood only once and reuse it for both directions.
        if (source->isCreature() && (notifyNearbyObservers || refreshSourceCreature))
        {
            Creature* sourceCreature = source->ToCreature();
            AIInterface* sourceAI = sourceCreature ? sourceCreature->getAIInterface() : nullptr;
            const bool sourceCanObserve = refreshSourceCreature && sourceAI && sourceAI->isAutomaticAwarenessActive();

            nearbyUnits.clear();
            spatialIndex_->collectUnitsInRange(source->GetPosition(), awarenessSearchRange, nearbyUnits);
            ++m_awarenessQueriesThisTick;
            m_awarenessCandidatesThisTick += nearbyUnits.size();

            for (Unit* nearby : nearbyUnits)
            {
                if (!nearby || nearby == source || !nearby->IsInWorld() || nearby->getWorldMap() != this)
                    continue;

                if (notifyNearbyObservers && nearby->isCreature())
                {
                    Creature* observer = nearby->ToCreature();
                    AIInterface* observerAI = observer ? observer->getAIInterface() : nullptr;
                    if (observerAI && observer->isAlive())
                        observerAI->considerObservedUnit(source, reason);
                }

                if (sourceCanObserve)
                    sourceAI->considerObservedUnit(nearby, reason);

                // A script may despawn/transfer the source while handling the hook.
                if (!source->IsInWorld() || source->getWorldMap() != this)
                    break;
            }

            continue;
        }

        // Players/pets/non-creature units only need to tell nearby creatures that
        // their observable state changed; they do not own Creature AI awareness.
        if (notifyNearbyObservers)
        {
            observerCreatures.clear();
            spatialIndex_->collectObjectsInRange<Creature>(source->GetPosition(), awarenessSearchRange, observerCreatures);
            ++m_awarenessQueriesThisTick;
            m_awarenessCandidatesThisTick += observerCreatures.size();

            for (Creature* observer : observerCreatures)
            {
                if (!observer || !observer->IsInWorld() || observer->getWorldMap() != this || !observer->isAlive())
                    continue;

                AIInterface* ai = observer->getAIInterface();
                if (ai)
                    ai->considerObservedUnit(source, reason);
            }
        }
    }

    processingUnitAwareness_.clear();
}

void WorldMap::refreshDynamicObjectTargets(DynamicObject* dynamicObject)
{
    if (!dynamicObject || !dynamicObject->IsInWorld() || dynamicObject->getWorldMap() != this)
        return;

    const float radius = std::max(0.0f, dynamicObject->getRadius());
    maxDynamicObjectTargetRadius_ = std::max(maxDynamicObjectTargetRadius_, radius);

    // Movement or radius changes must also evict existing targets that are no
    // longer valid, including targets now outside the current search radius.
    dynamicObject->refreshCurrentTargets();

    if (radius <= 0.0f)
        return;

    thread_local std::vector<Unit*> nearbyUnits;
    nearbyUnits.clear();
    spatialIndex_->collectUnitsInRange(dynamicObject->GetPosition(), radius, nearbyUnits);
    ++m_awarenessQueriesThisTick;
    m_awarenessCandidatesThisTick += nearbyUnits.size();

    for (Unit* target : nearbyUnits)
    {
        if (!target || !target->IsInWorld() || target->getWorldMap() != this)
            continue;

        dynamicObject->considerTarget(target);

        // Aura/script application may despawn the DynamicObject.
        if (!dynamicObject->IsInWorld() || dynamicObject->getWorldMap() != this)
            break;
    }
}

void WorldMap::refreshVisibilityForObject(Object* obj)
{
    if (!obj || !visibilitySystem_)
        return;

    if (obj->getWorldMap() != this)
        return;

    auto h = spatialIndex_->handleByGuid(obj->GetNewGUID());
    if (!h.id)
        return;


    // Keep default interest roles in sync for objects whose role can change while
    // already attached. applyInterestProfile is idempotent for already enabled
    // roles and is the single place that knows object-type defaults.
    auto profile = visibilitySystem_->buildInterestProfile(obj);
    visibilitySystem_->applyInterestProfile(h, profile);

    visibilitySystem_->refreshObjectVisibility(h);

    if (profile.viewer)
        processPendingVisibilityChangesForViewer(obj->GetNewGUID(), 512, 512);
    else
        processPendingVisibilityChanges(2048, 64, 256);
}


void WorldMap::resetVisibilityForPlayerRelocation(Player* player)
{
    if (!player)
        return;

    const WoWGuid viewerGuid = player->GetNewGUID();
    const uint64_t playerRaw = viewerGuid.getRawGuid();

    // Same-map relocation/repop must be a hard client-visibility boundary.
    // Do not only clear Player::visible_: the VisibilitySystem can still have
    // the old player viewer subscribed to the death-location cells. If that
    // stale subscription remains, the old creatures are recreated or stay
    // visible until the player moves enough to trigger normal reconciliation.
    //
    // Also drop pending create/update data from the old location before queuing
    // the destroy list. Player::die()/updateVisibility() can enqueue create
    // blocks for corpse-area creatures immediately before Release Spirit. If
    // those pending creates survive this relocation reset, the outgoing packet
    // can contain: OutOfRange(old NPCs) followed by Create(old NPCs), which
    // makes the client keep the death-location creatures visible at the graveyard.
    player->getUpdateMgr().clearPendingUpdates();


    std::vector<WoWGuid> oldVisible;
    player->collectVisibleObjectGuidsForRelocation(oldVisible);

    if (auto h = getSpatialIndex().handleByGuid(viewerGuid); h.id)
        getVisibilitySystem().resetViewerForRelocation(h, oldVisible);
    else
        getVisibilitySystem().clearVisibleObjectsForViewer(viewerGuid, oldVisible);

    std::set<uint64_t> unique;
    for (const WoWGuid& objectGuid : oldVisible)
    {
        const uint64_t raw = objectGuid.getRawGuid();
        if (!raw || raw == playerRaw)
            continue;

        if (unique.insert(raw).second)
            player->getUpdateMgr().pushOutOfRangeGuid(objectGuid);
    }

    player->clearVisibleObjectCachesForRelocation();

    initialCreateValueUpdateSuppress_.erase(playerRaw);
    purgePendingVisibilityForGuid(viewerGuid);

    // Send the destroy list before new creates for the destination are queued.
    player->processPendingUpdates();
}

void WorldMap::logVisibilityMemoryDiagnostics(const char* reason)
{
    if (!visibilitySystem_ || !registry_)
        return;

    const auto vis = visibilitySystem_->snapshot();
    const auto reg = registry_->counts();

    std::size_t pendingPairs = 0;
    for (const auto& [viewerRaw, objects] : pendingVisibilityByViewer_)
        pendingPairs += objects.size();

    std::size_t suppressPairs = 0;
    for (const auto& [viewerRaw, objects] : initialCreateValueUpdateSuppress_)
        suppressPairs += objects.size();

    sLogger.warning(
        "mapmem: reason={} map={} inst={} registry[any={},total={},players={},creatures={},gos={},dyn={},pets={},corpses={}] "
        "pending[queuedViewers={},viewers={},pairs={}] suppress[viewers={},pairs={}] deferredDestroy={} "
        "vis[grids={},activeGrids={},cells={},activeCells={},poolLive={},guidIndex={},visiblePairs={},seenPairs={},gridWidePub={},cellRadiusPub={},nearCached={},nearGuidCap={},nearStampCap={}]",
        reason ? reason : "",
        getBaseMap() ? getBaseMap()->getMapId() : 0,
        getInstanceId(),
        reg.any,
        reg.total,
        reg.players,
        reg.creatures,
        reg.gameObjects,
        reg.dynamics,
        reg.pets,
        reg.corpses,
        pendingVisibilityViewers_.size(),
        pendingVisibilityByViewer_.size(),
        pendingPairs,
        initialCreateValueUpdateSuppress_.size(),
        suppressPairs,
        deferred_destroy_.size(),
        vis.grids,
        vis.activeGrids,
        vis.cells,
        vis.activeCells,
        vis.poolLive,
        vis.guidIndex,
        vis.visiblePairs,
        vis.seenPairs,
        vis.gridWidePublishers,
        vis.cellRadiusPublishers,
        vis.nearCacheCachedGuids,
        vis.nearCacheGuidsCapacity,
        vis.nearCacheStampsCapacity);

    visibilitySystem_->logMemoryDiagnostics(reason);
}

void WorldMap::queueVisibilityChange(const WoWGuid& viewer, const WoWGuid& object, PendingVisibilityAction action)
{
    const uint64_t viewerRaw = viewer.getRawGuid();
    const uint64_t objectRaw = object.getRawGuid();

    if (!viewerRaw || !objectRaw)
        return;

    auto& pending = pendingVisibilityByViewer_[viewerRaw];
    pending[objectRaw] = { viewer, object, action };

    if (pendingVisibilityQueuedViewers_.insert(viewerRaw).second)
        pendingVisibilityViewers_.push_back(viewerRaw);
}

void WorldMap::purgePendingVisibilityForGuid(const WoWGuid& guid)
{
    const uint64_t raw = guid.getRawGuid();
    if (!raw)
        return;

    // This function is called during detach. It must only purge state where the
    // removed guid acts as a viewer. Hidden states for this guid as an object
    // must remain queued so recipients can still receive the removal.
    pendingVisibilityByViewer_.erase(raw);
    pendingVisibilityQueuedViewers_.erase(raw);
    initialCreateValueUpdateSuppress_.erase(raw);

    // The same detach point is also the lifetime boundary for movement-awareness
    // throttling. Purge queued state and the last evaluated position so a reused
    // GUID can never inherit the previous object's movement baseline.
    {
        std::lock_guard<std::mutex> guard(unitAwarenessMutex_);
        pendingUnitAwareness_.erase(raw);
        lastMovementAwarenessPosition_.erase(raw);
    }
}

void WorldMap::setVisibilityRecipientForViewer(const WoWGuid& viewerGuid, Player* recipient)
{
    const uint64_t viewerRaw = viewerGuid.getRawGuid();
    if (!viewerRaw || !recipient || !recipient->IsInWorld() || recipient->getWorldMap() != this)
        return;

    visibilityRecipientByViewer_[viewerRaw] = recipient->GetNewGUID().getRawGuid();
}

void WorldMap::clearVisibilityRecipientForViewer(const WoWGuid& viewerGuid)
{
    const uint64_t viewerRaw = viewerGuid.getRawGuid();
    if (viewerRaw)
        visibilityRecipientByViewer_.erase(viewerRaw);
}

Player* WorldMap::getVisibilityRecipientPlayer(const WoWGuid& viewerGuid)
{
    if (Player* player = getPlayer(viewerGuid))
    {
        if (!player->IsInWorld() || player->getWorldMap() != this)
            return nullptr;

        return player;
    }

    const uint64_t viewerRaw = viewerGuid.getRawGuid();
    if (auto it = visibilityRecipientByViewer_.find(viewerRaw); it != visibilityRecipientByViewer_.end())
    {
        Player* recipient = getPlayer(WoWGuid(it->second));
        if (!recipient || !recipient->IsInWorld() || recipient->getWorldMap() != this)
        {
            visibilityRecipientByViewer_.erase(it);
            return nullptr;
        }

        return recipient;
    }

    Object* viewerObject = getObject(viewerGuid);
    if (!viewerObject)
        return nullptr;

    if (Unit* viewerUnit = viewerObject->ToUnit())
    {
        Player* controller = viewerUnit->m_playerControler;
        if (!controller || !controller->IsInWorld() || controller->getWorldMap() != this)
            return nullptr;

        return controller;
    }

    if (auto* dyn = dynamic_cast<DynamicObject*>(viewerObject))
    {
        if (dyn->getDynamicType() != DYNAMIC_OBJECT_FARSIGHT_FOCUS)
            return nullptr;

        Player* caster = getPlayer(WoWGuid(dyn->getCasterGuid()));
        if (!caster || !caster->IsInWorld() || caster->getWorldMap() != this)
            return nullptr;

        return caster;
    }

    return nullptr;
}

void WorldMap::collectVisibilityRecipientsForObject(const WoWGuid& objectGuid, std::vector<Player*>& out)
{
    out.clear();

    if (!visibilitySystem_)
        return;

    thread_local std::vector<WoWGuid> s_viewers;
    s_viewers.clear();
    visibilitySystem_->collectViewersOf(objectGuid, s_viewers);

    std::unordered_set<uint64_t> seenRecipients;
    seenRecipients.reserve(s_viewers.size() + 1);

    auto addRecipient = [&](Player* player)
    {
        if (!player || !player->IsInWorld() || player->getWorldMap() != this)
            return;

        const uint64_t raw = player->GetNewGUID().getRawGuid();
        if (!raw || !seenRecipients.insert(raw).second)
            return;

        // Only forward ordinary world updates to clients for which the object has
        // already completed its visibility create. This keeps packet ordering
        // correct for normal, possessed and farsight viewer sources alike.
        if (player->seesGuid(objectGuid))
            out.push_back(player);
    };

    for (const WoWGuid& viewerGuid : s_viewers)
        addRecipient(getVisibilityRecipientPlayer(viewerGuid));

    // A controlled unit must always be able to send controller-specific movement/
    // state packets to its player. Normally it is already present through the
    // possessed viewer relation above; this is a defensive bridge during the short
    // possess/unpossess transition while visibility events are still being drained.
    if (Object* object = getObject(objectGuid))
    {
        if (Unit* unit = object->ToUnit())
            addRecipient(unit->m_playerControler);
    }
}

bool WorldMap::hasOtherVisibilitySourceForRecipient(const WoWGuid& losingViewerGuid, const WoWGuid& objectGuid, Player* recipient)
{
    if (!recipient)
        return false;

    thread_local std::vector<WoWGuid> s_viewers;
    s_viewers.clear();

    getVisibilitySystem().collectViewersOf(objectGuid, s_viewers);

    const uint64_t losingRaw = losingViewerGuid.getRawGuid();
    for (const WoWGuid& otherViewerGuid : s_viewers)
    {
        if (otherViewerGuid.getRawGuid() == losingRaw)
            continue;

        if (getVisibilityRecipientPlayer(otherViewerGuid) == recipient)
            return true;
    }

    return false;
}

void WorldMap::clearVisibilitySourceForRecipient(const WoWGuid& viewerGuid, Player* recipient)
{
    if (!recipient)
        return;

    thread_local std::vector<WoWGuid> s_objects;
    s_objects.clear();

    // Tear down the source inside VisibilitySystem first. Merely reading the
    // viewer's visible set leaves visibleNow_/seenBy_ intact, which means the
    // source can still be treated as an active owner of those client objects
    // while farsight/possession is ending.
    getVisibilitySystem().clearVisibleObjectsForViewer(viewerGuid, s_objects);

    const uint64_t recipientRaw = recipient->GetNewGUID().getRawGuid();

    for (const WoWGuid& objectGuid : s_objects)
    {
        if (!recipient->seesGuid(objectGuid))
            continue;

        // Keep objects that the same client still sees through another viewer source,
        // for example the player's own body after possession ends near the body.
        if (hasOtherVisibilitySourceForRecipient(viewerGuid, objectGuid, recipient))
            continue;

        // Important: this helper is used by Unit::unPossess() while the
        // possessor still has charm/farsight state pointing at the controlled unit.
        // Do not run legacy onRemoveInRangeObject side effects here: if objectGuid
        // is the possessed unit, Player/Unit cleanup can interrupt the possess aura
        // and re-enter unPossess while it is already executing. Normal visibility
        // hidden events still run these callbacks through applyQueuedVisibilityHidden().
        // Here we only remove the client-side visibility introduced by the temporary
        // viewer source and queue the matching OutOfRange update.
        recipient->_visRemove(objectGuid);

        if (auto it = initialCreateValueUpdateSuppress_.find(recipientRaw); it != initialCreateValueUpdateSuppress_.end())
        {
            it->second.erase(objectGuid.getRawGuid());
            if (it->second.empty())
                initialCreateValueUpdateSuppress_.erase(it);
        }

        recipient->getUpdateMgr().pushOutOfRangeGuid(objectGuid);
    }
}


bool WorldMap::applyQueuedVisibilityVisible(const WoWGuid& viewerGuid, const WoWGuid& objectGuid)
{
    Player* player = getVisibilityRecipientPlayer(viewerGuid);
    if (!player)
        return false;

    Object* object = getObject(objectGuid);
    if (!object)
        return false;

    // If the object already left the recipient player's visibility again while queued, do not create it.
    // This also deduplicates cases where the player sees the same object through both their body and
    // a possessed/farsight-controlled unit.
    if (player->seesGuid(objectGuid))
        return true;

    player->_visAdd(objectGuid);
    ++m_visibilityCreatesThisTick;

    object->prepareInitialCreateForPlayer(player);

    ByteBuffer buf(2500);
    const uint32_t cnt = object->buildCreateUpdateBlockForPlayer(&buf, player);
    if (cnt)
        player->getUpdateMgr().pushCreationData(&buf, cnt);

    object->queueInitialVisiblePacketsForPlayer(player);

    initialCreateValueUpdateSuppress_[player->GetNewGUID().getRawGuid()].insert(objectGuid.getRawGuid());

    // The create block already contains the current values for this recipient.
    // Scheduling a normal value update here is redundant and can replay
    // GameObject state/animation updates immediately after spawn.
    return true;
}

bool WorldMap::applyQueuedVisibilityHidden(const WoWGuid& viewerGuid, const WoWGuid& objectGuid, bool hardDestroy)
{
    Player* player = getVisibilityRecipientPlayer(viewerGuid);
    if (!player)
        return false;

    if (!player->seesGuid(objectGuid))
        return true;

    // Do not destroy the object on the client if the same player still sees it
    // through another visibility source, e.g. both their own body and a possessed NPC.
    if (hasOtherVisibilitySourceForRecipient(viewerGuid, objectGuid, player))
        return true;

    // Cache stable ids before invoking any legacy callbacks. Those callbacks can
    // re-enter visibility/gameplay cleanup and may invalidate native Object/Player
    // pointers or mutate visibility bookkeeping. Never keep an STL iterator or a
    // native object pointer alive across them.
    const WoWGuid recipientGuid = player->GetNewGUID();
    const uint64_t recipientRaw = recipientGuid.getRawGuid();
    const uint64_t objectRaw = objectGuid.getRawGuid();

    // Once the final visibility state is Hidden, an initial-create suppression for
    // this pair is no longer useful. Remove it before legacy callbacks so re-entrant
    // cleanup cannot invalidate an iterator/reference that this function still owns.
    if (auto suppressIt = initialCreateValueUpdateSuppress_.find(recipientRaw); suppressIt != initialCreateValueUpdateSuppress_.end())
    {
        suppressIt->second.erase(objectRaw);
        if (suppressIt->second.empty())
            initialCreateValueUpdateSuppress_.erase(recipientRaw);
    }

    // Bridge old in-range removal side effects into the new visibility system.
    // Important: only run these callbacks for the player's own normal viewer.
    // A possessed NPC is a temporary viewer whose packet recipient is the
    // possessor player. When that temporary viewer is removed during unPossess(),
    // running Player::onRemoveInRangeObject(possessedNpc) can interrupt the
    // possess aura and re-enter unPossess(), causing recursive cleanup/crashes.
    // The player's real viewer source will still run normal gameplay side effects.
    const bool normalPlayerViewer = viewerGuid.getRawGuid() == recipientRaw;
    if (normalPlayerViewer)
    {
        if (Object* object = getObject(objectGuid))
        {
            player->onRemoveInRangeObject(object);

            // The player callback is legacy code and can synchronously remove the
            // object or relocate/remove the player. Resolve both sides again before
            // invoking the reverse callback.
            player = getPlayer(recipientGuid);
            object = getObject(objectGuid);
            if (player && player->IsInWorld() && player->getWorldMap() == this && object)
                object->onRemoveInRangeObject(player);
        }
    }

    // Legacy callbacks above are allowed to synchronously alter world membership.
    // Re-resolve the recipient before touching client visibility/update state.
    player = getPlayer(recipientGuid);
    if (!player || !player->IsInWorld() || player->getWorldMap() != this)
        return true;

    // Another re-entrant visibility path may already have consumed this removal.
    if (!player->seesGuid(objectGuid))
        return true;

    player->_visRemove(objectGuid);
    ++m_visibilityDestroysThisTick;

    // Normal visibility leave stays batched through SMSG_UPDATE_OBJECT.
    // Explicit editor refreshes need a hard destroy so the client fully drops
    // the cached GameObject movement/create state before the same GUID is
    // created again. Position and rotation are part of the create/movement
    // block and are otherwise retained by some clients until relog.
    if (hardDestroy)
        player->sendDestroyObjectPacket(objectRaw);
    else
        player->getUpdateMgr().pushOutOfRangeGuid(objectGuid);

    return true;
}


bool WorldMap::shouldSuppressInitialValueUpdateForViewer(uint64_t viewerRaw, uint64_t objectRaw) const
{
    auto viewerIt = initialCreateValueUpdateSuppress_.find(viewerRaw);
    if (viewerIt == initialCreateValueUpdateSuppress_.end())
        return false;

    return viewerIt->second.find(objectRaw) != viewerIt->second.end();
}

void WorldMap::processPendingVisibilityChanges(std::size_t maxEvents, std::size_t maxCreatesPerPlayer, std::size_t maxDestroysPerPlayer)
{
    if (pendingVisibilityViewers_.empty() || maxEvents == 0)
        return;

    std::size_t attempted = 0;
    const std::size_t viewersToVisit = pendingVisibilityViewers_.size();

    for (std::size_t viewerIndex = 0; viewerIndex < viewersToVisit && attempted < maxEvents && !pendingVisibilityViewers_.empty(); ++viewerIndex)
    {
        const uint64_t viewerRaw = pendingVisibilityViewers_.front();
        pendingVisibilityViewers_.pop_front();
        pendingVisibilityQueuedViewers_.erase(viewerRaw);

        std::size_t creates = 0;
        std::size_t destroys = 0;

        while (attempted < maxEvents)
        {
            auto viewerIt = pendingVisibilityByViewer_.find(viewerRaw);
            if (viewerIt == pendingVisibilityByViewer_.end())
                break;

            auto& pending = viewerIt->second;
            auto eventIt = pending.end();

            for (auto it = pending.begin(); it != pending.end(); ++it)
            {
                const bool isCreate = it->second.action == PendingVisibilityAction::Visible;
                if ((isCreate && creates >= maxCreatesPerPlayer) || (!isCreate && destroys >= maxDestroysPerPlayer))
                    continue;

                eventIt = it;
                break;
            }

            if (eventIt == pending.end())
                break;

            // Visibility callbacks can synchronously re-enter the visibility system.
            // Never keep an iterator/reference into pendingVisibilityByViewer_ alive
            // across those callbacks. Consume the queued state first, then apply it.
            const PendingVisibilityEvent event = eventIt->second;
            pending.erase(eventIt);

            if (pending.empty())
                pendingVisibilityByViewer_.erase(viewerIt);

            const bool isCreate = event.action == PendingVisibilityAction::Visible;
            if (isCreate)
            {
                if (applyQueuedVisibilityVisible(event.viewer, event.object))
                    ++creates;
            }
            else
            {
                if (applyQueuedVisibilityHidden(event.viewer, event.object))
                    ++destroys;
            }

            ++attempted;
        }

        auto viewerIt = pendingVisibilityByViewer_.find(viewerRaw);
        if (viewerIt != pendingVisibilityByViewer_.end() && !viewerIt->second.empty())
        {
            if (pendingVisibilityQueuedViewers_.insert(viewerRaw).second)
                pendingVisibilityViewers_.push_back(viewerRaw);
        }
    }
}

void WorldMap::processPendingVisibilityChangesForViewer(const WoWGuid& viewer, std::size_t maxCreates, std::size_t maxDestroys)
{
    const uint64_t viewerRaw = viewer.getRawGuid();
    if (!viewerRaw)
        return;

    std::size_t creates = 0;
    std::size_t destroys = 0;

    while (true)
    {
        auto viewerIt = pendingVisibilityByViewer_.find(viewerRaw);
        if (viewerIt == pendingVisibilityByViewer_.end())
            break;

        auto& pending = viewerIt->second;
        auto eventIt = pending.end();

        for (auto it = pending.begin(); it != pending.end(); ++it)
        {
            const bool isCreate = it->second.action == PendingVisibilityAction::Visible;
            if ((isCreate && creates >= maxCreates) || (!isCreate && destroys >= maxDestroys))
                continue;

            eventIt = it;
            break;
        }

        if (eventIt == pending.end())
            break;

        // applyQueuedVisibilityVisible/Hidden may synchronously trigger gameplay
        // callbacks which mutate visibility again. Remove the current state before
        // entering those callbacks and reacquire the viewer map on every iteration.
        const PendingVisibilityEvent event = eventIt->second;
        pending.erase(eventIt);

        if (pending.empty())
            pendingVisibilityByViewer_.erase(viewerIt);

        const bool isCreate = event.action == PendingVisibilityAction::Visible;
        if (isCreate)
        {
            if (applyQueuedVisibilityVisible(event.viewer, event.object))
                ++creates;
        }
        else
        {
            if (applyQueuedVisibilityHidden(event.viewer, event.object))
                ++destroys;
        }
    }

    auto viewerIt = pendingVisibilityByViewer_.find(viewerRaw);
    if (viewerIt != pendingVisibilityByViewer_.end() && !viewerIt->second.empty())
    {
        if (pendingVisibilityQueuedViewers_.insert(viewerRaw).second)
            pendingVisibilityViewers_.push_back(viewerRaw);
    }
}

void WorldMap::flushVisibilityRemovalForObject(const WoWGuid& objectGuid)
{
    const uint64_t wantedObjectRaw = objectGuid.getRawGuid();
    if (!wantedObjectRaw)
        return;

    std::vector<PendingVisibilityEvent> removals;

    // First detach the matching Hidden events from the pending containers.
    // Applying them can re-enter visibility and must not happen while iterating
    // pendingVisibilityByViewer_.
    for (auto viewerIt = pendingVisibilityByViewer_.begin(); viewerIt != pendingVisibilityByViewer_.end();)
    {
        auto& pending = viewerIt->second;
        auto objectIt = pending.find(wantedObjectRaw);

        if (objectIt != pending.end() && objectIt->second.action == PendingVisibilityAction::Hidden)
        {
            removals.push_back(objectIt->second);
            pending.erase(objectIt);
        }

        if (pending.empty())
            viewerIt = pendingVisibilityByViewer_.erase(viewerIt);
        else
            ++viewerIt;
    }

    for (const PendingVisibilityEvent& event : removals)
        applyQueuedVisibilityHidden(event.viewer, event.object, true);
}

void WorldMap::onObjectBecameVisible(const WoWGuid& viewer, const WoWGuid& obj)
{
    queueVisibilityChange(viewer, obj, PendingVisibilityAction::Visible);
}

void WorldMap::onObjectBecameHidden(const WoWGuid& viewer, const WoWGuid& obj)
{
    queueVisibilityChange(viewer, obj, PendingVisibilityAction::Hidden);
}

inline int homeGridFromWorld(float x, float y) 
{
    LocationVector p{ x, y, 0.f, 0.f };
    auto [gx, gy] = visibility::worldToGrid(p);
    return visibility::packGridId(gx, gy);
}

void WorldMap::removeAllPlayers()
{
    thread_local std::vector<Player*> s_players;
    registry_->snapshotPlayers(s_players);

    thread_local std::vector<WoWGuid> s_playerGuids;
    s_playerGuids.clear();
    s_playerGuids.reserve(s_players.size());
    for (Player* p : s_players)
    {
        if (p && p->getWorldMap() == this)
            s_playerGuids.push_back(p->GetNewGUID());
    }

    for (const WoWGuid& g : s_playerGuids)
    {
        Player* player = registry_->getPlayer(g);
        if (!player || player->getWorldMap() != this)
            continue;

        if (WorldSession* session = player->getSession())
        {
            session->LogoutPlayer(false);
        }
        else
        {
            onPlayerLeave(player);
            factory_->recycleAndDestroy(player, true);
        }
    }

    drainDeferredDestroy();
}

bool WorldMap::cellHasAreaID(uint32_t CellX, uint32_t CellY, uint16_t& AreaID)
{
    int32_t TileX = CellX / 8;
    int32_t TileY = CellY / 8;

    if (!getTerrain()->areTilesValid(TileX, TileY))
        return false;

    int32_t OffsetTileX = TileX - getTerrain()->TileStartX;
    int32_t OffsetTileY = TileY - getTerrain()->TileStartY;

    bool Required = false;
    bool Result = false;

    if (!getTerrain()->tileLoaded(OffsetTileX, OffsetTileY))
        Required = true;

    if (Required)
    {
        getTerrain()->loadTile(TileX, TileY);
        getTerrain()->loadTile(TileX, TileY);
        return Result;
    }

    for (uint32_t xc = (CellX % Cell::CellsPerTile) * 16 / Cell::CellsPerTile; xc < (CellX % Cell::CellsPerTile) * 16 / Cell::CellsPerTile + 16 / Cell::CellsPerTile; xc++)
    {
        for (uint32_t yc = (CellY % Cell::CellsPerTile) * 16 / Cell::CellsPerTile; yc < (CellY % Cell::CellsPerTile) * 16 / Cell::CellsPerTile + 16 / Cell::CellsPerTile; yc++)
        {
            const auto areaid = getTerrain()->getTile(OffsetTileX, OffsetTileY)->m_map.m_areaMap[yc * 16 + xc];
            if (areaid)
            {
                AreaID = areaid;
                Result = true;
                break;
            }
        }
    }

    if (Required)
        getTerrain()->unloadTile(TileX, TileY);

    return Result;
}

void WorldMap::changeFarsightLocation(Player* plr, DynamicObject* farsight)
{
    if (!plr)
        return;

    // Farsight is a real VisibilitySystem viewer/activator. Its physical position
    // drives subscriptions while the caster player remains the packet recipient
    // and supplies player-specific visibility rules (GM invisibility, stealth, etc.).
    // ObjectFactory already applies the DynamicObject interest profile on attach;
    // refresh here so the initial remote view is available synchronously to the spell.
    if (farsight)
    {
        setVisibilityRecipientForViewer(farsight->GetNewGUID(), plr);

        auto h = spatialIndex_->handleByGuid(farsight->GetNewGUID());
        if (!h.id)
            return;

        auto profile = visibilitySystem_->buildInterestProfile(farsight);
        visibilitySystem_->applyInterestProfile(h, profile);
        visibilitySystem_->refreshObjectVisibility(h);
        processPendingVisibilityChangesForViewer(farsight->GetNewGUID(), 512, 512);
        return;
    }

    // Farsight ends when the aura clears the player's farsight guid, which can
    // happen before the DynamicObject itself reaches its lifetime/destruction path.
    // Tear down the remote viewer synchronously here so the client does not keep
    // objects that were visible only from the remote camera.
    const WoWGuid viewerGuid(plr->getFarsightGuid());
    if (viewerGuid.getRawGuid())
    {
        if (DynamicObject* viewer = getDynamicObject(viewerGuid);
            viewer && viewer->getDynamicType() == DYNAMIC_OBJECT_FARSIGHT_FOCUS)
        {
            clearVisibilitySourceForRecipient(viewerGuid, plr);

            // Send the OutOfRange batch before this source can be removed or
            // refreshed again. Waiting for the normal map process queue is too late
            // for aura teardown because the camera has already returned to the player.
            plr->processPendingUpdates();

            if (auto h = spatialIndex_->handleByGuid(viewerGuid); h.id)
            {
                visibilitySystem_->setViewerRole(h, false, 0);
                visibilitySystem_->setActivatorRole(h, false, 0);
                processPendingVisibilityChangesForViewer(viewerGuid, 4096, 4096);
            }

            clearVisibilityRecipientForViewer(viewerGuid);
            purgePendingVisibilityForGuid(viewerGuid);

            // changeFarsightLocation() is called while PLAYER_FIELD_FARSIGHT still
            // contains this guid. Remove the focus here, not later in the aura-effect
            // callback where the guid may already have been cleared (interrupt path).
            viewer->remove();
        }
    }

    plr->m_visibleFarsightObjects.clear();
}

Summon* WorldMap::summonCreature(uint32_t entry, LocationVector pos, WDB::Structures::SummonPropertiesEntry const* properties /*= nullptr*/, uint32_t duration /*= 0*/, Object* summoner /*= nullptr*/, uint32_t spellId /*= 0*/)
{
    // Generate always a new guid for totems, otherwise the totem bar will get messed up
    const auto isTotemSummon = properties != nullptr &&
        (properties->ControlType == SUMMON_CONTROL_TYPE_WILD ||
            properties->ControlType == SUMMON_CONTROL_TYPE_GUARDIAN ||
            properties->ControlType == SUMMON_CATEGORY_UNK) &&
        properties->Type == SUMMONTYPE_TOTEM;

    uint64_t guid = factory_->generateCreatureGuid(entry, !isTotemSummon);

    // Phase
    uint32_t phase = 1;
    if (summoner)
        phase = summoner->GetPhase();

    Unit* summonerUnit = summoner ? summoner->ToUnit() : nullptr;

    Summon* summon = nullptr;
    if (properties)
    {
        switch (properties->ControlType)
        {
            case SUMMON_CONTROL_TYPE_PET: // Guardians
            {
                summon = new WildSummon(guid, properties);
            } break;
            case SUMMON_CONTROL_TYPE_POSSESSED:
            {
                summon = new PossessedSummon(guid, properties);
            } break;
            case SUMMON_CONTROL_TYPE_VEHICLE:
            {
                summon = new CompanionSummon(guid, properties);
            } break;
            case SUMMON_CONTROL_TYPE_WILD:
            case SUMMON_CONTROL_TYPE_GUARDIAN:
            case SUMMON_CATEGORY_UNK:
            {
                switch (properties->Type)
                {
                    case SUMMONTYPE_MINION:
                    case SUMMONTYPE_GUARDIAN:
                    case SUMMONTYPE_GUARDIAN2:
                    {
                        summon = new WildSummon(guid, properties);
                    } break;
                    case SUMMONTYPE_TOTEM:
                    case SUMMONTYPE_LIGHTWELL:
                    {
                        summon = new TotemSummon(guid, properties);
                    } break;
                    case SUMMONTYPE_VEHICLE:
                    case SUMMONTYPE_VEHICLE2:
                    {
                        summon = new Summon(guid, properties);
                    } break;
                    case SUMMONTYPE_MINIPET:
                    {
                        summon = new CompanionSummon(guid, properties);
                    } break;
                    default:
                    {
                        if (properties->Flags & 512) // Mirror Image, Summon Gargoyle
                            summon = new WildSummon(guid, properties);
                    } break;
                }
            } break;
            default:
                summon = new Summon(guid, properties);
                break;
        }
    }
    else
    {
        summon = new Summon(guid, properties);
    }

    const auto* cp = sMySQLStore.getCreatureProperties(entry);
    if (cp == nullptr)
    {
        delete summon;
        return nullptr;
    }

    summon->load(cp, summonerUnit, pos, duration, spellId);
    summon->setPhase(PHASE_SET, phase);
    factory_->attachToWorld(summon);

    // This is needed to CastSpells or Move Right at Spawn
    updateObjects();

    // Delay this a bit to make sure its Spawned
    sEventMgr.AddEvent(static_cast<Creature*>(summon), &Creature::InitSummon, static_cast<Object*>(summonerUnit), EVENT_UNK, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    return summon;
}

GameObject* WorldMap::summonGameObject(uint32_t entryID, LocationVector pos, QuaternionData const& rot, uint32_t durationMs, Object* summoner)
{
    auto gameobject_info = sMySQLStore.getGameObjectProperties(entryID);
    if (gameobject_info == nullptr)
    {
        sLogger.debug("Error looking up entry in CreateAndSpawnGameObject");
        return nullptr;
    }

    sLogger.debug("CreateAndSpawnGameObject: By Entry '{}'", entryID);

    GameObject* go = getSpawnManager().summonGameObject(entryID, pos, rot);
    if (!go)
        return nullptr;

    if (summoner)
        go->m_phase = summoner->GetPhase();

    if (durationMs > 0)
        go->despawn(durationMs, 0);

    return go;
}

void WorldMap::sendChatMessageToCellPlayers(Object* obj, SmsgMessageChat& packet, uint32_t cell_radius, uint32_t lang, WorldSession* originator)
{
    if (!obj)
        return;

    thread_local std::vector<WoWGuid> s_guids;
    s_guids.clear();
    s_guids.reserve(128);

    spatialIndex_->collectGuidsInCellsAroundPos<Player>(obj->GetPosition(), static_cast<int>(cell_radius), 0, s_guids);

    const uint32_t senderPhase = obj->GetPhase();

    for (const WoWGuid& guid : s_guids)
    {
        Player* player = registry_->getPlayer(guid);
        if (!player)
            continue;

        if (player->getWorldMap() != this)
            continue;

        if ((player->GetPhase() & senderPhase) == 0)
            continue;

        Object::UpdatePin pin(player);

        if (WorldSession* session = player->getSession())
        {
            session->sendChatPacket(packet, lang, originator);
        }
        else
        {
            sLogger.failure("sendChatMessageToCellPlayers: invalid session for player {}", guid.getCounter());
        }
    }
}

void WorldMap::sendPvPCaptureMessage(int32_t ZoneMask, uint32_t ZoneId, const char* Message, ...)
{
    va_list ap;
    va_start(ap, Message);

    char msgbuf[200];
    vsnprintf(msgbuf, 200, Message, ap);
    va_end(ap);

    thread_local std::vector<Player*> s_players;
    registry_->snapshotPlayers(s_players);
    registry_->forEachPinned(s_players, [&](Player& player)
        {
            if (player.getWorldMap() != this)
                return;

            if ((ZoneMask != ZONE_MASK_ALL && player.getZoneId() != static_cast<uint32_t>(ZoneMask)))
                return;

            SmsgDefenseMessage managedPacket(ZoneId, msgbuf);
            player.getSession()->sendManagedPacket(managedPacket);
        });
}

void WorldMap::sendPacketToAllPlayers(WorldPacket* packet) const
{
    if (!packet)
        return;

    thread_local std::vector<Player*> s_players;
    registry_->snapshotPlayers(s_players);

    registry_->forEachPinned(s_players, [&](Player& player)
        {
            if (player.getWorldMap() != this) return;

            if (WorldSession* session = player.getSession())
                session->SendPacket(packet);
            else
                sLogger.failure("sendPacketToAllPlayers: invalid session for player {}", player.getGuidLow());
        });
}

void WorldMap::sendPacketToPlayersInZone(uint32_t zone, WorldPacket* packet) const
{
    if (!packet)
        return;

    thread_local std::vector<Player*> s_players;
    registry_->snapshotPlayers(s_players);

    registry_->forEachPinned(s_players, [&](Player& player)
        {
            if (player.getWorldMap() != this)
                return;

            if (player.getZoneId() != zone)
                return;

            if (WorldSession* session = player.getSession())
            {
                session->SendPacket(packet);
            }
            else
            {
                sLogger.failure("sendPacketToPlayersInZone: invalid session for player {}", player.getGuidLow());
            }
        });
}

InstanceScript* WorldMap::getScript()
{
    return mInstanceScript;
}

void WorldMap::loadInstanceScript()
{
    mInstanceScript = sScriptMgr.CreateScriptClassForInstance(getBaseMap()->getMapId(), this);
};

void WorldMap::callScriptUpdate()
{
    if (mInstanceScript != nullptr)
    {
        mInstanceScript->UpdateEvent();
        mInstanceScript->updateTimers();
    }
    else
    {
        sLogger.failure("WorldMap::callScriptUpdate tries to call without valid instance script (nullptr)");
    }
};

void WorldMap::updateObjects()
{
    std::scoped_lock<std::mutex> lock(m_updateMutex);

    if (!_updates.size() && !_processQueue.size())
    {
        initialCreateValueUpdateSuppress_.clear();
        return;
    }

    ByteBuffer update(2500);
    uint32_t count = 0;

    for (auto pObj : _updates)
    {
        if (pObj == nullptr)
            continue;

        if (pObj->isItem() || pObj->isContainer())
        {
            // our update is only sent to the owner here.
            Player* pOwner = static_cast<Item*>(pObj)->getOwner();
            if (pOwner != nullptr)
            {
                count = pObj->BuildValuesUpdateBlockForPlayer(&update, pOwner);
                // send update to owner
                if (count)
                {
                    pOwner->getUpdateMgr().pushUpdateData(&update, count);
                    update.clear();
                }
            }
        }
        else
        {
            if (pObj->IsInWorld())
            {
                // players have to receive their own updates ;)
                if (pObj->isPlayer())
                {
                    // need to be different! ;)
                    count = pObj->BuildValuesUpdateBlockForPlayer(&update, static_cast<Player*>(pObj));
                    if (count)
                    {
                        static_cast<Player*>(pObj)->getUpdateMgr().pushUpdateData(&update, count);
                        update.clear();
                    }
                }

                // build the update
                count = pObj->BuildValuesUpdateBlockForPlayer(&update, static_cast<Player*>(nullptr));
                update.clear();

                if (count)
                {
                    thread_local std::vector<Player*> s_visibilityRecipients;
                    collectVisibilityRecipientsForObject(pObj->GetNewGUID(), s_visibilityRecipients);

                    for (Player* lplr : s_visibilityRecipients)
                    {
                        if (!lplr)
                            continue;

                        // Players already receive their own update in the dedicated branch above.
                        if (pObj->isPlayer() && lplr == pObj)
                            continue;

                        const uint64_t viewerRaw = lplr->GetNewGUID().getRawGuid();
                        const uint64_t objectRaw = pObj->GetNewGUID().getRawGuid();

                        // The queued create packet already contains the initial values for
                        // this recipient. Suppress exactly one normal value-update pass so
                        // delayed visibility creates do not immediately replay stale/duplicate
                        // GameObject, animation, dynamic flag, or combat-state values.
                        if (shouldSuppressInitialValueUpdateForViewer(viewerRaw, objectRaw))
                            continue;

                        // Build the recipient-specific values. Remote visibility sources use
                        // the same player recipient as normal visibility, so field filtering
                        // remains player-correct.
                        const uint32_t recipientCount = pObj->BuildValuesUpdateBlockForPlayer(&update, lplr);
                        if (recipientCount)
                            lplr->getUpdateMgr().pushUpdateData(&update, recipientCount);
                        update.clear();
                    }
                }
            }
        }
        pObj->ClearUpdateMask();
    }
    _updates.clear();
    initialCreateValueUpdateSuppress_.clear();

    // generate pending a9packets and send to clients.
    for (auto it = _processQueue.begin(); it != _processQueue.end();)
    {
        Player* player = *it;

        auto it2 = it;
        ++it;

        _processQueue.erase(it2);
        if (player->getWorldMap() == this)
            player->processPendingUpdates();
    }
}

void WorldMap::pushToProcessed(Player* plr)
{
    _processQueue.insert(plr);
}

MapScriptInterface* WorldMap::getInterface()
{
    return ScriptInterface.get();
}

WorldStatesHandler& WorldMap::getWorldStatesHandler()
{
    return worldstateshandler;
}

void WorldMap::onWorldStateUpdate(uint32_t zone, uint32_t field, uint32_t value)
{
    sendPacketToPlayersInZone(zone, SmsgUpdateWorldState(field, value).serialise().get());
}

bool WorldMap::isCombatInProgress()
{
    return (_combatProgress.size() > 0);
}

void WorldMap::addCombatInProgress(uint64_t guid)
{
    _combatProgress.insert(guid);
}

void WorldMap::removeCombatInProgress(uint64_t guid)
{
    _combatProgress.erase(guid);
}

void WorldMap::objectUpdated(Object* obj)
{
    // set our fields to dirty stupid fucked up code in places.. i hate doing this but i've got to :<- burlex
    std::scoped_lock<std::mutex> lock(m_updateMutex);
    _updates.insert(obj);
}

bool WorldMap::isRegularDifficulty()
{
    return getDifficulty() == InstanceDifficulty::Difficulties::DUNGEON_NORMAL;
}

WDB::Structures::MapDifficulty const* WorldMap::getMapDifficulty()
{
    return getMapDifficultyData(getBaseMap()->getMapId(), getDifficulty());
}

bool WorldMap::isRaidOrHeroicDungeon()
{
    return getBaseMap()->isRaid() || getSpawnMode() > InstanceDifficulty::Difficulties::DUNGEON_NORMAL;
}

bool WorldMap::isHeroic()
{
    return getBaseMap()->isRaid() ? getSpawnMode() >= InstanceDifficulty::Difficulties::RAID_10MAN_HEROIC : getSpawnMode() >= InstanceDifficulty::Difficulties::DUNGEON_HEROIC;
}

bool WorldMap::is25ManRaid()
{
    return getBaseMap()->isRaid() && getSpawnMode() & 1;
}

bool WorldMap::getAreaInfo(uint32_t /*phaseMask*/, LocationVector pos, uint32_t& flags, int32_t& adtId, int32_t& rootId, int32_t& groupId)
{
    float vmap_z = pos.z;
    float dynamic_z = pos.z;
    float check_z = pos.z;
    const auto vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    uint32_t vflags;
    int32_t vadtId;
    int32_t vrootId;
    int32_t vgroupId;
    uint32_t dflags;
    int32_t dadtId;
    int32_t drootId;
    int32_t dgroupId;

    bool hasVmapAreaInfo = vmgr->getAreaInfo(getBaseMap()->getMapId(), pos.x, pos.y, vmap_z, vflags, vadtId, vrootId, vgroupId);
    bool hasDynamicAreaInfo = false;/*_dynamicTree.getAreaInfo(x, y, dynamic_z, phaseMask, dflags, dadtId, drootId, dgroupId);*/
    auto useVmap = [&]() { check_z = vmap_z; flags = vflags; adtId = vadtId; rootId = vrootId; groupId = vgroupId; };
    auto useDyn = [&]() { check_z = dynamic_z; flags = dflags; adtId = dadtId; rootId = drootId; groupId = dgroupId; };

    if (hasVmapAreaInfo)
    {
        if (hasDynamicAreaInfo && dynamic_z > vmap_z)
            useDyn();
        else
            useVmap();
    }
    else if (hasDynamicAreaInfo)
    {
        useDyn();
    }

    if (hasVmapAreaInfo || hasDynamicAreaInfo)
    {
        // check if there's terrain between player height and object height
        if (TerrainTile* gmap = getTerrain()->getTile(pos.x, pos.y))
        {
            float mapHeight = gmap->m_map.getHeight(pos.x, pos.y);
            // z + 2.0f condition taken from getHeight(), not sure if it's such a great choice...
            if (pos.z + 2.0f > mapHeight && mapHeight > check_z)
                return false;
        }
        return true;
    }
    return false;
}

uint32_t WorldMap::getAreaId(uint32_t phaseMask, LocationVector const& pos)
{
    uint32_t mogpFlags;
    int32_t adtId, rootId, groupId;
    float vmapZ = pos.z;
    bool hasVmapArea = getAreaInfo(phaseMask, LocationVector(pos.x, pos.y, vmapZ), mogpFlags, adtId, rootId, groupId);

    uint32_t gridAreaId = 0;
    float gridMapHeight = INVALID_HEIGHT;
    if (TerrainTile* gmap = getTerrain()->getTile(pos.x, pos.y))
    {
        gridAreaId = gmap->m_map.getArea(pos.x, pos.y);
        gridMapHeight = gmap->m_map.getHeight(pos.x, pos.y);
    }

    uint32_t areaId = 0;

    // floor is the height we are closer to (but only if above)
    if (hasVmapArea && G3D::fuzzyGe(pos.z, vmapZ - GROUND_HEIGHT_TOLERANCE) && (G3D::fuzzyLt(pos.z, gridMapHeight - GROUND_HEIGHT_TOLERANCE) || vmapZ > gridMapHeight))
    {
        // wmo found
        if (WDB::Structures::WMOAreaTableEntry const* wmoEntry = GetWMOAreaTableEntryByTriple(rootId, adtId, groupId))
            areaId = wmoEntry->areaId;

        if (!areaId)
            areaId = gridAreaId;
    }
    else
    {
        areaId = gridAreaId;
    }

    if (!areaId)
        areaId = getBaseMap()->getMapEntry()->linkedZone;

    return areaId;
}

uint32_t WorldMap::getZoneId(uint32_t phaseMask, LocationVector const& pos)
{
    uint32_t areaId = 0;
    if (const auto* area = MapManagement::AreaManagement::AreaStorage::getExactArea(this, pos, phaseMask))
    {
        areaId = area->id;
        if (area->zone)
            return area->zone;
    }

    return areaId;
}

void WorldMap::getZoneAndAreaId(uint32_t phaseMask, uint32_t& zoneid, uint32_t& areaid, LocationVector const& pos)
{
    if (const auto* area = MapManagement::AreaManagement::AreaStorage::getExactArea(this, pos, phaseMask))
    {
        areaid = area->id;
        if (area->zone)
            zoneid = area->zone;
    }
}

static inline bool isInWMOInterior(uint32_t mogpFlags)
{
    return (mogpFlags & 0x2000) != 0;
}

ZLiquidStatus WorldMap::getLiquidStatus(uint32_t phaseMask, LocationVector pos, uint8_t ReqLiquidType, LiquidData* data, float collisionHeight)
{
    ZLiquidStatus result = LIQUID_MAP_NO_WATER;
    const auto vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    float liquid_level = INVALID_HEIGHT;
    float ground_level = INVALID_HEIGHT;
    uint32_t liquid_type = 0;
    uint32_t mogpFlags = 0;
    bool useGridLiquid = true;
    if (getBaseMap() && vmgr->getLiquidLevel(getBaseMap()->getMapId(), pos.x, pos.y, pos.z, ReqLiquidType, liquid_level, ground_level, liquid_type, mogpFlags))
    {
        useGridLiquid = !isInWMOInterior(mogpFlags);
        // Check water level and ground level
        if (liquid_level > ground_level && G3D::fuzzyGe(pos.z, ground_level - GROUND_HEIGHT_TOLERANCE))
        {
            // All ok in water -> store data
            if (data)
            {
                // hardcoded in client like this
                if (getBaseMap()->getMapId() == 530 && liquid_type == 2)
                    liquid_type = 15;

                uint32_t liquidFlagType = 0;
                if (WDB::Structures::LiquidTypeEntry const* liq = sLiquidTypeStore.lookupEntry(liquid_type))
                    liquidFlagType = liq->Type;

                if (liquid_type && liquid_type < 21)
                {
                    if (const auto* area = MapManagement::AreaManagement::AreaStorage::getExactArea(this, pos, phaseMask))
                    {
                        uint32_t const index = (WoW::getCurrentExpansion() == WoW::Expansion::_Classic) ? 0 : liquidFlagType;
                        uint32_t overrideLiquid = area->liquid_type_override[index];

                        if (!overrideLiquid && area->zone)
                        {
                            area = MapManagement::AreaManagement::AreaStorage::getAreaById(area->zone);
                            if (area)
                            {
                                overrideLiquid = area->liquid_type_override[index];
                            }
                        }

                        if (WDB::Structures::LiquidTypeEntry const* liq = sLiquidTypeStore.lookupEntry(overrideLiquid))
                        {
                            liquid_type = overrideLiquid;
                            liquidFlagType = liq->Type;
                        }
                    }
                }

                data->level = liquid_level;
                data->depth_level = ground_level;

                data->entry = liquid_type;
                data->type_flags = 1U << liquidFlagType;
            }

            float delta = liquid_level - pos.z;

            // Get position delta
            if (delta > collisionHeight)        // Under water
                return LIQUID_MAP_UNDER_WATER;
            if (delta > 0.0f)                   // In water
                return LIQUID_MAP_IN_WATER;
            if (delta > -0.1f)                  // Walk on water
                return LIQUID_MAP_WATER_WALK;
            result = LIQUID_MAP_ABOVE_WATER;
        }
    }

    if (useGridLiquid)
    {
        if (TerrainTile* gmap = getTerrain()->getTile(pos.x, pos.y))
        {
            LiquidData map_data;
            ZLiquidStatus map_result = gmap->m_map.getLiquidStatus(pos, ReqLiquidType, &map_data, collisionHeight);
            // Not override LIQUID_MAP_ABOVE_WATER with LIQUID_MAP_NO_WATER:
            if (map_result != LIQUID_MAP_NO_WATER && (map_data.level > ground_level))
            {
                if (data)
                {
                    // hardcoded in client like this
                    if (getBaseMap()->getMapId() == 530 && map_data.entry == 2)
                        map_data.entry = 15;

                    *data = map_data;
                }
                return map_result;
            }
        }
    }
    return result;
}

void WorldMap::getFullTerrainStatusForPosition(uint32_t phaseMask, float x, float y, float z, PositionFullTerrainStatus& data, uint8_t reqLiquidType, float collisionHeight) const
{
    if (!getTerrain())
        return;

    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    VMAP::AreaAndLiquidData vmapData;
    VMAP::AreaAndLiquidData dynData;
    VMAP::AreaAndLiquidData* wmoData = nullptr;

    TerrainTile* gmap = getTerrain()->getTile(x, y);
    if (!gmap)
        return;

    vmgr->getAreaAndLiquidData(getBaseMap()->getMapId(), x, y, z, reqLiquidType, vmapData);
    _dynamicTree.getAreaAndLiquidData(x, y, z, phaseMask, reqLiquidType, dynData);

    uint32_t gridAreaId = 0;
    float gridMapHeight = INVALID_HEIGHT;
    if (gmap)
    {
        gridAreaId = gmap->m_map.getArea(x, y);
        gridMapHeight = gmap->m_map.getHeight(x, y);
    }

    bool useGridLiquid = true;

    // floorZ is the height we are closer to example we stand on an wmo
    data.floorZ = VMAP_INVALID_HEIGHT;
    if (gridMapHeight > INVALID_HEIGHT && G3D::fuzzyGe(z, gridMapHeight - GROUND_HEIGHT_TOLERANCE))
        data.floorZ = gridMapHeight;
    if (vmapData.floorZ > VMAP_INVALID_HEIGHT &&
        G3D::fuzzyGe(z, vmapData.floorZ - GROUND_HEIGHT_TOLERANCE) &&
        (G3D::fuzzyLt(z, gridMapHeight - GROUND_HEIGHT_TOLERANCE) || vmapData.floorZ > gridMapHeight))
    {
        data.floorZ = vmapData.floorZ;
        wmoData = &vmapData;
    }

    // When spawning and despawning Wmos the provided area/liquid from beneath them wont get detected properly
    // example: Lich King platform
    if (dynData.floorZ > VMAP_INVALID_HEIGHT &&
        G3D::fuzzyGe(z, dynData.floorZ - GROUND_HEIGHT_TOLERANCE) &&
        (G3D::fuzzyLt(z, gridMapHeight - GROUND_HEIGHT_TOLERANCE) || dynData.floorZ > gridMapHeight) &&
        (G3D::fuzzyLt(z, vmapData.floorZ - GROUND_HEIGHT_TOLERANCE) || dynData.floorZ > vmapData.floorZ))
    {
        data.floorZ = dynData.floorZ;
        wmoData = &dynData;
    }

    if (wmoData)
    {
        if (wmoData->areaInfo)
        {
            data.areaInfo.emplace(wmoData->areaInfo->adtId, wmoData->areaInfo->rootId, wmoData->areaInfo->groupId, wmoData->areaInfo->mogpFlags);
            // wmo found
            WDB::Structures::WMOAreaTableEntry const* wmoEntry = GetWMOAreaTableEntryByTriple(wmoData->areaInfo->rootId, wmoData->areaInfo->adtId, wmoData->areaInfo->groupId);
            data.outdoors = (wmoData->areaInfo->mogpFlags & 0x8) != 0;
            if (wmoEntry)
            {
                data.areaId = wmoEntry->areaId;
                if (wmoEntry->flags & 4)
                    data.outdoors = true;
                else if (wmoEntry->flags & 2)
                    data.outdoors = false;
            }

            if (!data.areaId)
                data.areaId = gridAreaId;

            useGridLiquid = !isInWMOInterior(wmoData->areaInfo->mogpFlags);
        }
    }
    else
    {
        data.outdoors = true;
        data.areaId = gridAreaId;
        if (WDB::Structures::AreaTableEntry const* areaEntry = sAreaStore.lookupEntry(data.areaId))
            data.outdoors = (areaEntry->flags & (MapManagement::AreaManagement::AreaFlags::AREA_FLAG_INSIDE | MapManagement::AreaManagement::AreaFlags::AREA_FLAG_OUTSIDE)) != MapManagement::AreaManagement::AreaFlags::AREA_FLAG_INSIDE;
    }

    if (!data.areaId)
        data.areaId = getBaseMap()->getMapEntry()->linkedZone;

    WDB::Structures::AreaTableEntry const* areaEntry = sAreaStore.lookupEntry(data.areaId);

    // liquid processing
    data.liquidStatus = LIQUID_MAP_NO_WATER;
    if (wmoData && wmoData->liquidInfo && wmoData->liquidInfo->level > wmoData->floorZ)
    {
        uint32_t liquidType = wmoData->liquidInfo->type;
        if (getBaseMap()->getMapId() == 530 && liquidType == 2) // gotta love blizzard hacks
            liquidType = 15;

        uint32_t liquidFlagType = 0;
        if (WDB::Structures::LiquidTypeEntry const* liquidData = sLiquidTypeStore.lookupEntry(liquidType))
            liquidFlagType = liquidData->Type;

        if (liquidType && liquidType < 21 && areaEntry)
        {
            uint32_t const liquidIndex = (WoW::getCurrentExpansion() == WoW::Expansion::_Classic) ? 0 : liquidFlagType;
            uint32_t overrideLiquid = areaEntry->liquid_type_override[liquidIndex];

            if (!overrideLiquid && areaEntry->zone)
            {
                if (WDB::Structures::AreaTableEntry const* zoneEntry = sAreaStore.lookupEntry(areaEntry->zone))
                {
                    overrideLiquid = zoneEntry->liquid_type_override[liquidIndex];
                }
            }

            if (WDB::Structures::LiquidTypeEntry const* overrideData = sLiquidTypeStore.lookupEntry(overrideLiquid))
            {
                liquidType = overrideLiquid;
                liquidFlagType = overrideData->Type;
            }
        }

        data.liquidInfo.emplace();
        data.liquidInfo->level = wmoData->liquidInfo->level;
        data.liquidInfo->depth_level = wmoData->floorZ;
        data.liquidInfo->entry = liquidType;
        data.liquidInfo->type_flags = 1 << liquidFlagType;

        float delta = wmoData->liquidInfo->level - z;
        if (delta > collisionHeight)
            data.liquidStatus = LIQUID_MAP_UNDER_WATER;
        else if (delta > 0.0f)
            data.liquidStatus = LIQUID_MAP_IN_WATER;
        else if (delta > -0.1f)
            data.liquidStatus = LIQUID_MAP_WATER_WALK;
        else
            data.liquidStatus = LIQUID_MAP_ABOVE_WATER;
    }
    // look up liquid data from grid map
    if (gmap && useGridLiquid)
    {
        LiquidData gridMapLiquid;
        ZLiquidStatus gridMapStatus = gmap->m_map.getLiquidStatus(LocationVector(x, y, z), reqLiquidType, &gridMapLiquid, collisionHeight);
        if (gridMapStatus != LIQUID_MAP_NO_WATER && (!wmoData || gridMapLiquid.level > wmoData->floorZ))
        {
            if (getBaseMap()->getMapId() == 530 && gridMapLiquid.entry == 2)
                gridMapLiquid.entry = 15;
            data.liquidInfo = gridMapLiquid;
            data.liquidStatus = gridMapStatus;
        }
    }
}

float WorldMap::getWaterLevel(float x, float y)
{
    if (TerrainTile* gmap = getTerrain()->getTile(x, y))
        return gmap->m_map.getLiquidLevel(x, y);
    else
        return 0;
}

bool WorldMap::isInWater(uint32_t phaseMask, LocationVector pos, LiquidData* data)
{
    LiquidData liquid_status{};
    LiquidData* liquid_ptr = data ? data : &liquid_status;
    return (getLiquidStatus(phaseMask, pos, MAP_ALL_LIQUIDS, liquid_ptr) & (LIQUID_MAP_IN_WATER | LIQUID_MAP_UNDER_WATER)) != 0;
}

bool WorldMap::isUnderWater(uint32_t phaseMask, LocationVector pos)
{
    return (getLiquidStatus(phaseMask, pos, MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN) & LIQUID_MAP_UNDER_WATER) != 0;
}

bool WorldMap::isInLineOfSight(LocationVector pos1, LocationVector pos2, uint32_t phasemask, LineOfSightChecks checks)
{
    if ((checks & LINEOFSIGHT_CHECK_VMAP)
        && !VMAP::VMapFactory::createOrGetVMapManager()->isInLineOfSight(getBaseMap()->getMapId(), pos1.x, pos1.y, pos1.z, pos2.x, pos2.y, pos2.z))
        return false;

    if (checks & LINEOFSIGHT_CHECK_GOBJECT && !_dynamicTree.isInLineOfSight(pos1.x, pos1.y, pos1.z, pos2.x, pos2.y, pos2.z, phasemask))
        return false;

    return true;
}

bool WorldMap::getObjectHitPos(uint32_t phasemask, LocationVector pos1, LocationVector pos2, float& rx, float& ry, float& rz, float modifyDist)
{
    G3D::Vector3 startPos(pos1.x, pos1.y, pos1.z);
    G3D::Vector3 dstPos(pos2.x, pos2.y, pos2.z);

    G3D::Vector3 resultPos;
    bool result = _dynamicTree.getObjectHitPos(phasemask, startPos, dstPos, resultPos, modifyDist);

    rx = resultPos.x;
    ry = resultPos.y;
    rz = resultPos.z;
    return result;
}

float WorldMap::getGameObjectFloor(uint32_t phasemask, LocationVector pos, float maxSearchDist /*= 50.0f*/) const
{
    return _dynamicTree.getHeight(pos.x, pos.y, pos.z, maxSearchDist, phasemask);
}

float WorldMap::getHeight(uint32_t phasemask, LocationVector const& pos, bool vmap /*= true*/, float maxSearchDist /*= 50.0f*/) const
{
    return std::max<float>(getHeight(pos, vmap, maxSearchDist), getGameObjectFloor(phasemask, pos, maxSearchDist));
}


float WorldMap::getWaterOrGroundLevel(uint32_t phasemask, LocationVector const& pos, float* ground /*= nullptr*/, bool /*swim = false*/, float collisionHeight /*= 2.03128f*/)
{
    if (getTerrain()->getTile(pos.x, pos.y))
    {
        // we need ground level (including grid height version) for proper return water level in point
        float ground_z = getHeight(phasemask, LocationVector(pos.x, pos.y, pos.z + collisionHeight), true, 50.0f);
        if (ground)
            *ground = ground_z;

        LiquidData liquid_status;

        ZLiquidStatus res = getLiquidStatus(phasemask, LocationVector(pos.x, pos.y, ground_z), MAP_ALL_LIQUIDS, &liquid_status, collisionHeight);
        switch (res)
        {
            case LIQUID_MAP_ABOVE_WATER:
                return std::max<float>(liquid_status.level, ground_z);
            case LIQUID_MAP_NO_WATER:
                return ground_z;
            default:
                return liquid_status.level;
        }
    }

    return VMAP_INVALID_HEIGHT_VALUE;
}

float WorldMap::getHeight(LocationVector const& pos, bool checkVMap /*= true*/, float maxSearchDist /*= 50.0f*/) const
{
    // find raw .map surface under Z coordinates
    float mapHeight = VMAP_INVALID_HEIGHT_VALUE;
    float gridHeight = getGridHeight(pos.x, pos.y);
    if (G3D::fuzzyGe(pos.z, gridHeight - GROUND_HEIGHT_TOLERANCE))
        mapHeight = gridHeight;

    float vmapHeight = VMAP_INVALID_HEIGHT_VALUE;
    if (checkVMap)
    {
        const auto vmgr = VMAP::VMapFactory::createOrGetVMapManager();
        if (vmgr->isHeightCalcEnabled())
            vmapHeight = vmgr->getHeight(getBaseMap()->getMapId(), pos.x, pos.y, pos.z, maxSearchDist);
    }

    // mapHeight set for any above raw ground Z or <= INVALID_HEIGHT
    // vmapheight set for any under Z value or <= INVALID_HEIGHT
    if (vmapHeight > INVALID_HEIGHT)
    {
        if (mapHeight > INVALID_HEIGHT)
        {
            // we have mapheight and vmapheight and must select more appropriate

            // vmap height above map height
            // or if the distance of the vmap height is less the land height distance
            if (vmapHeight > mapHeight || std::fabs(mapHeight - pos.z) > std::fabs(vmapHeight - pos.z))
                return vmapHeight;

            return mapHeight;                           // better use .map surface height
        }

        return vmapHeight;                              // we have only vmapHeight (if have)
    }

    return mapHeight;                               // explicitly use map data
}

float WorldMap::getGridHeight(float x, float y) const
{
    if (TerrainTile* gmap = getTerrain()->getTile(x, y))
        return gmap->m_map.getHeight(x, y);

    return VMAP_INVALID_HEIGHT_VALUE;
}

void WorldMap::respawnBossLinkedGroups(uint32_t bossId)
{
    for (uint32_t spawnId : sMySQLStore.getSpawnGroupDataByBoss(bossId))
    {
        const SpawnGroupTemplateData* group = sMySQLStore.getSpawnGroupDataBySpawn(spawnId);
        if (!group || !(group->spawnFlags & SPAWFLAG_FLAG_BOUNDTOBOSS))
            continue;

        if (Creature* creature = getSpawnManager().findLiveCreature(spawnId);
            creature && creature->isAlive())
        {
            continue;
        }

        getSpawnManager().respawnNow(SPAWN_TYPE_CREATURE, spawnId);
    }
}
