/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/WorldStatesHandler.hpp"
#include "DynamicTree.h"
#include "Server/EventableObject.h"

#include <queue>
#include <algorithm>
#include <deque>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <vector>
#include <mutex>

#include "InstanceDefines.hpp"
#include "Debugging/Errors.hpp"
#include "Map/SpawnGroups.hpp"
#include "Server/Packets/SmsgMessageChat.h"
#include "Map/Management/TerrainMgr.hpp"

#include "Map/Visibility/VisibilitySystem.hpp"
#include "Map/Management/ObjectFactory.hpp"
#include "Map/Management/WorldObjectRegistry.hpp"
#include "Map/Management/GuidAllocator.hpp"
#include "Map/Management/SpawnManager.hpp"

namespace AscEmu::Threading
{
    class AEThread;
}

namespace WDB::Structures
{
    struct SummonPropertiesEntry;
    struct MapDifficulty;
}

namespace visibility { class SpatialIndex; class VisibilitySystem; }
namespace world { class WorldObjectRegistry; }
class GuidAllocator;
class ObjectFactory;

class WorldSession;
class ByteBuffer;
class WorldPacket;
class DynamicObject;

class BaseMap;
class InstanceScript;
class MapScriptInterface;
class Object;
class GameObject;
class Unit;
class Creature;
class Player;
class Pet;
class Transporter;
class Corpse;
class Battleground;
class InstanceScript;
class Summon;
class InstanceMap;
class CreatureGroup;
enum LineOfSightChecks : uint8_t;
enum EnterState;
enum class UnitAwarenessSignal : uint16_t;

typedef std::set<Object*> UpdateQueue;
typedef std::set<Player*> PUpdateQueue;

typedef std::set<uint64_t> CombatProgressMap;

struct SpatialPerformanceSnapshot
{
    uint64_t windowMs = 0;
    uint32_t ticks = 0;

    uint64_t spatialQueries = 0;
    uint64_t spatialCandidates = 0;
    uint64_t spatialTryGets = 0;
    uint64_t spatialMoves = 0;
    uint64_t cellMoves = 0;

    uint64_t visibilityPairChecks = 0;
    uint64_t visibilityCreates = 0;
    uint64_t visibilityDestroys = 0;
    uint64_t ringSubscribes = 0;
    uint64_t ringUnsubscribes = 0;

    uint64_t awarenessQueries = 0;
    uint64_t awarenessCandidates = 0;

    uint64_t maxSpatialCandidatesPerTick = 0;
    uint64_t maxSpatialTryGetsPerTick = 0;
    uint64_t maxVisibilityPairChecksPerTick = 0;
    uint64_t maxAwarenessCandidatesPerTick = 0;
};

class SERVER_DECL WorldMap : public EventableObject, public WorldStatesHandler::WorldStatesObserver
{
    friend class MapScriptInterface;
    friend class ObjectFactory;

public:
    WorldMap(BaseMap* baseMap, uint32_t id, uint32_t expiryTime, uint32_t InstanceId, uint8_t SpawnMode);
    virtual ~WorldMap();

    virtual void initialize();
    virtual void update(uint32_t);
    virtual void delayedUpdate(std::chrono::milliseconds diff);
    virtual void unloadAll(bool onShutdown = false);

    void startMapThread();
    void runThread();
    void shutdownMapThread();
    void unsafeKillMapThread();
    bool isMapReadyForDelete() const;

    void Do();

    void setUnloadPending(bool value) { m_unloadPending = value; }
    bool isUnloadPending() { return m_unloadPending; }

    float getVisibilityDistance() const noexcept { return m_visibilityDistance; }
    float getVisibilityDistanceSq() const noexcept { return m_visibilityDistanceSq; }
    virtual void initVisibilityDistance();
    void syncVisibilitySubscriptionRadius();

    void outOfMapBoundariesTeleport(Object* object);

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Visibility System
    //////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    void updateAll(std::chrono::milliseconds diff);

    // Run cross-map handoffs on the target map thread.
    void queueMapTask(std::function<void()> task);

    void drainDeferredDestroy();

    void hookVisibilityEvents();
    bool onPlayerEnter(Player* plr);
    void onPlayerLeave(Player* plr);
    void addSession(WorldSession* session);
    void removeSession(WorldSession* session);
    void onObjectMoved(Object* obj);
    void refreshVisibilityForObject(Object* obj);

    // Queue creature awareness changes and process duplicates together.
    void queueUnitAwareness(Unit* unit, UnitAwarenessSignal reason);
    void processPendingUnitAwareness();

    // Refresh nearby area spells when units or DynamicObjects change.
    void refreshDynamicObjectTargets(DynamicObject* dynamicObject);

    void onObjectBecameVisible(const WoWGuid& viewer, const WoWGuid& obj);
    void onObjectBecameHidden(const WoWGuid& viewer, const WoWGuid& obj);

    void processPendingVisibilityChanges(std::size_t maxEvents = 2048, std::size_t maxCreatesPerPlayer = 32, std::size_t maxDestroysPerPlayer = 96);
    void processPendingVisibilityChangesForViewer(const WoWGuid& viewer, std::size_t maxCreates = 256, std::size_t maxDestroys = 256);
    void flushVisibilityRemovalForObject(const WoWGuid& object);
    void purgePendingVisibilityForGuid(const WoWGuid& guid);
    void clearVisibilitySourceForRecipient(const WoWGuid& viewer, Player* recipient);
    void resetVisibilityForPlayerRelocation(Player* player);
    void logVisibilityMemoryDiagnostics(const char* reason);
    const SpatialPerformanceSnapshot& getSpatialPerformanceSnapshot() const noexcept { return m_lastSpatialPerformance; }
    void collectVisibilityRecipientsForObject(const WoWGuid& object, std::vector<Player*>& out);
    void setVisibilityRecipientForViewer(const WoWGuid& viewer, Player* recipient);
    void clearVisibilityRecipientForViewer(const WoWGuid& viewer);

    // Temporary legacy map API while old scripts/commands are migrated to
    // ObjectRegistry/SpatialIndex. Keep these as thin registry views only.
    std::map<uint32_t, Player*> getPlayers() const;
    std::vector<Creature*> getCreatures() const;
    std::vector<GameObject*> getGameObjects() const;

    uint32_t getPlayerCount() const;
    bool hasPlayers() const { return getPlayerCount() != 0; }

    Creature* getSqlIdCreature(uint32_t spawnId) const;
    GameObject* getSqlIdGameObject(uint32_t spawnId) const;

    visibility::SpatialIndex& getSpatialIndex() const { assert(spatialIndex_); return *spatialIndex_; }
    visibility::VisibilitySystem& getVisibilitySystem() const { assert(visibilitySystem_); return *visibilitySystem_; }
    world::WorldObjectRegistry& getRegistry() const { assert(registry_); return *registry_; }
    ObjectFactory& getObjectFactory() const { assert(factory_);  return *factory_; }
    SpawnManager& getSpawnManager() const { assert(spawnMgr_); return *spawnMgr_; }

private:
    enum class PendingVisibilityAction : uint8_t
    {
        Visible,
        Hidden
    };

    struct PendingVisibilityEvent
    {
        WoWGuid viewer;
        WoWGuid object;
        PendingVisibilityAction action = PendingVisibilityAction::Visible;
    };

    void queueVisibilityChange(const WoWGuid& viewer, const WoWGuid& object, PendingVisibilityAction action);
    void finalizeSpatialPerformanceSample(std::chrono::milliseconds diff);
    Player* getVisibilityRecipientPlayer(const WoWGuid& viewer);
    bool hasOtherVisibilitySourceForRecipient(const WoWGuid& losingViewer, const WoWGuid& object, Player* recipient);
    bool applyQueuedVisibilityVisible(const WoWGuid& viewer, const WoWGuid& object);
    bool applyQueuedVisibilityHidden(const WoWGuid& viewer, const WoWGuid& object, bool hardDestroy = false);
    bool shouldSuppressInitialValueUpdateForViewer(uint64_t viewerRaw, uint64_t objectRaw) const;

    // Only ObjectFactory may schedule raw object deletion. All callers must go
    // through the object lifecycle so detach cleanup cannot be skipped.
    bool deferDestroy(Object* object);
    void processMapTasks();

    std::mutex mapTaskMutex_;
    std::deque<std::function<void()>> mapTasks_;


    std::unordered_set<Object*> deferred_destroy_;
    std::unordered_set<Object*> destroying_;
    std::mutex deferredDestroyMutex_;

    // Pending visibility is coalesced per viewer/object pair. Repeated
    // Visible/Hidden transitions before processing only keep the final state.
    // The viewer queue provides round-robin fairness without stale events.
    std::unordered_map<uint64_t, std::unordered_map<uint64_t, PendingVisibilityEvent>> pendingVisibilityByViewer_;
    std::deque<uint64_t> pendingVisibilityViewers_;
    std::unordered_set<uint64_t> pendingVisibilityQueuedViewers_;

    // Map-thread-owned AI awareness queue. The value is a bit-mask of
    // UnitAwarenessSignal values and naturally deduplicates movement/state events
    // generated multiple times before the next processing pass.
    std::unordered_map<uint64_t, uint16_t> pendingUnitAwareness_;
    std::unordered_map<uint64_t, uint16_t> processingUnitAwareness_;
    std::unordered_map<uint64_t, LocationVector> lastMovementAwarenessPosition_;
    std::mutex unitAwarenessMutex_;

    // High-water mark is intentionally not reduced when an area disappears. It
    // keeps unit-event lookup exact for any radius seen on this map without
    // requiring a global DynamicObject scan to recompute the maximum.
    float maxDynamicObjectTargetRadius_ = 0.0f;

    // Objects created for a viewer in the current visibility flush already include
    // their complete initial value state in the create block. If the object also
    // has a pending value update from before the viewer saw it, suppress that
    // one values update for this new viewer to avoid replaying GO animations.
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> initialCreateValueUpdateSuppress_;

    // Remote visibility sources (possessed units, Eyes of the Beast, farsight)
    // have their own spatial viewer GUID but deliver packets to a player. Keep
    // that relationship explicit so packet delivery and cleanup do not depend
    // on the remote object still being resolvable from the registry.
    std::unordered_map<uint64_t, uint64_t> visibilityRecipientByViewer_;

    // One-second rolling map-local performance window. Component counters are
    // map-thread owned and consumed once at the end of each WorldMap update.
    SpatialPerformanceSnapshot m_spatialPerformanceWindow{};
    SpatialPerformanceSnapshot m_lastSpatialPerformance{};
    std::chrono::milliseconds m_spatialPerformanceAccum{ 0 };
    uint64_t m_awarenessQueriesThisTick = 0;
    uint64_t m_awarenessCandidatesThisTick = 0;
    uint64_t m_visibilityCreatesThisTick = 0;
    uint64_t m_visibilityDestroysThisTick = 0;

protected:
    // Spatial/Visibility Systems
    std::unique_ptr<visibility::SpatialIndex> spatialIndex_;
    std::unique_ptr<visibility::VisibilitySystem> visibilitySystem_;
    std::unique_ptr<ObjectFactory>             factory_;
    std::unique_ptr<GuidAllocator>             guids_;
    std::unique_ptr<world::WorldObjectRegistry> registry_;
    std::unique_ptr<SpawnManager>               spawnMgr_;

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Object Registry
    //////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    Object* getObject(const WoWGuid& guid) const { return registry_ ? registry_->getAny(guid) : nullptr; }
    Creature* getCreature(const WoWGuid& guid) const { return registry_ ? registry_->getCreature(guid) : nullptr; }
    GameObject* getGameObject(const WoWGuid& guid) const { return registry_ ? registry_->getGameObject(guid) : nullptr; }
    DynamicObject* getDynamicObject(const WoWGuid& guid) const { return registry_ ? registry_->getDynamicObject(guid) : nullptr; }
    Player* getPlayer(const WoWGuid& guid) const { return registry_ ? registry_->getPlayer(guid) : nullptr; }
    Pet* getPet(const WoWGuid& guid) const { return registry_ ? registry_->getPet(guid) : nullptr; }
    Corpse* getCorpse(const WoWGuid& guid) const { return registry_ ? registry_->getCorpse(guid) : nullptr; }
    Unit* getUnit(const WoWGuid& guid) const;

    // Legacy migration helpers. These are O(n) and must not be used by new code.
    Creature* findCreatureByLow32(uint32_t lowGuid) const { return registry_ ? registry_->findCreatureByLow32(lowGuid) : nullptr; }
    GameObject* findGameObjectByLow32(uint32_t lowGuid) const { return registry_ ? registry_->findGameObjectByLow32(lowGuid) : nullptr; }

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Navigation System
    //////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    void navAcquireGrid(int gid);
    void navReleaseGrid(int gid);

private:
    std::mutex nav_mtx_;
    std::unordered_map<int, uint32_t> nav_gridRefs_;

    std::vector<int> collectTerrainGridsForArea(uint32_t id, bool matchZone);
    bool gridMatchesForcedRegion(int gid);
    bool shouldRuleForceGrid(int gid);
    bool ensureRuleGridPinned(int gid);
    void reevaluateRuleGridPins();

    bool forceAllGridsActive_ = false;
    std::unordered_set<int> forcedExplicitGrids_;
    std::unordered_set<int> rulePinnedGrids_;
    std::unordered_set<uint32_t> forcedZones_;
    std::unordered_set<uint32_t> forcedAreas_;

    //////////////////////////////////////////////////////////////////////////////////////////////////////

public:
    bool canUnload(uint32_t diff);

    virtual bool addPlayerToMap(Player*) { return true; }
    virtual void removePlayerFromMap(Player*) {};

    virtual EnterState cannotEnter(Player* /*player*/) { return EnterState::CAN_ENTER; }

    // Difficulty
    InstanceDifficulty::Difficulties getDifficulty() const { return InstanceDifficulty::Difficulties(getSpawnMode()); }
    bool isRegularDifficulty();
    WDB::Structures::MapDifficulty const* getMapDifficulty();

    // Area and Zone Management
    bool getAreaInfo(uint32_t phaseMask, LocationVector pos, uint32_t& mogpflags, int32_t& adtId, int32_t& rootId, int32_t& groupId);
    uint32_t getAreaId(uint32_t phaseMask, LocationVector const& pos);
    uint32_t getZoneId(uint32_t phaseMask, LocationVector const& pos);
    void getZoneAndAreaId(uint32_t phaseMask, uint32_t& zoneid, uint32_t& areaid, LocationVector const& pos);

    // Persistent grid activation. These functions are map-thread operations. Forced
    // grids stay spawned/loaded, while client visibility still uses normal distance.
    bool setGridForcedActive(int gid, bool active);
    size_t setAllGridsForcedActive(bool active);
    size_t setZoneGridsForcedActive(uint32_t zoneId, bool active);
    size_t setAreaGridsForcedActive(uint32_t areaId, bool active);
    bool isGridForcedActive(int gid) const;

    // Called whenever a grid starts being used by runtime objects or subscriptions.
    // Persistent map/zone/area rules are applied here so newly used grids stay active.
    void onGridMaterialized(int gid);

    void getFullTerrainStatusForPosition(uint32_t phaseId, float x, float y, float z, PositionFullTerrainStatus& data, uint8_t reqLiquidType, float collisionHeight) const;

    // Water
    ZLiquidStatus getLiquidStatus(uint32_t phaseMask, LocationVector pos, uint8_t ReqLiquidType, LiquidData* data = nullptr, float collisionHeight = 2.03128f);
    float getWaterLevel(float x, float y);
    bool isInWater(uint32_t phaseMask, LocationVector pos, LiquidData* data = nullptr);
    bool isUnderWater(uint32_t phaseMask, LocationVector pos);

    // Line of Sight
    bool isInLineOfSight(LocationVector pos1, LocationVector pos2, uint32_t phasemask, LineOfSightChecks checks);
    bool getObjectHitPos(uint32_t phasemask, LocationVector pos1, LocationVector pos2, float& rx, float& ry, float& rz, float modifyDist);

    // Dynamic Map
    DynamicMapTree const& getDynamicTree() const { return _dynamicTree; }
    void balance() { _dynamicTree.balance(); }
    void removeGameObjectModel(GameObjectModel const& model) { _dynamicTree.remove(model); }
    void insertGameObjectModel(GameObjectModel const& model) { _dynamicTree.insert(model); }
    bool containsGameObjectModel(GameObjectModel const& model) const { return _dynamicTree.contains(model); }
    float getGameObjectFloor(uint32_t phasemask, LocationVector pos, float maxSearchDist = 50.0f) const;

    // Terrain
    TerrainHolder* getTerrain() const { return _terrain.get(); }
    float getWaterOrGroundLevel(uint32_t phasemask, LocationVector const& pos, float* ground = nullptr, bool swim = false, float collisionHeight = 2.03128f);
    float getGridHeight(float x, float y) const;
    float getHeight(LocationVector const& pos, bool vmap = true, float maxSearchDist = 50.0f) const;
    // phasemask seems to be invalid when loading into a map                                                                                                                                                // phase
    float getHeight(uint32_t phasemask, LocationVector const& pos, bool vmap = true, float maxSearchDist = 50.0f) const;

    // Instance
    uint32_t getInstanceId() const { return _instanceId; }
    void setInstanceId(uint32_t instanceId) { _instanceId = instanceId; }
    uint8_t getSpawnMode() const { return (_instanceSpawnMode); }
    void setSpawnMode(uint8_t mode) { _instanceSpawnMode = mode; }
    bool isRaidOrHeroicDungeon();
    bool isHeroic();
    bool is25ManRaid();

    // Player
    virtual void removeAllPlayers();

    // Creatures
    std::unordered_map<uint32_t /*leaderSpawnId*/, std::unique_ptr<CreatureGroup>> CreatureGroupHolder;

    // Summons
    Summon* summonCreature(uint32_t entry, LocationVector pos, WDB::Structures::SummonPropertiesEntry const* = nullptr, uint32_t duration = 0, Object* summoner = nullptr, uint32_t spellId = 0);
    GameObject* summonGameObject(uint32_t entryID, LocationVector pos, QuaternionData const& rot, uint32_t duration = 0, Object* summoner = nullptr);

    // Base Template
    BaseMap* getBaseMap() const { return m_baseMap; }

    bool cellHasAreaID(uint32_t x, uint32_t y, uint16_t& AreaID);
   
    void changeFarsightLocation(Player* plr, DynamicObject* farsight);

    // Packts
    void sendChatMessageToCellPlayers(Object* obj, AscEmu::Packets::SmsgMessageChat& packet, uint32_t cell_radius, uint32_t lang, WorldSession* originator);
    void sendPvPCaptureMessage(int32_t ZoneMask, uint32_t ZoneId, const char* Message, ...);
    void sendPacketToAllPlayers(WorldPacket* packet) const;
    void sendPacketToPlayersInZone(uint32_t zone, WorldPacket* packet) const;

    EventableObjectHolder eventHolder;

    void respawnBossLinkedGroups(uint32_t bossId);

    // Update Timers
    std::chrono::milliseconds m_visibilityAccum{ 0 };
    std::chrono::milliseconds m_respawnAccum{ 0 };
    std::chrono::milliseconds m_sessionAccum{ 0 };
    std::chrono::milliseconds m_dynamicObjectAccum{ 0 };
    std::chrono::milliseconds m_transporterAccum{ 0 };
    std::chrono::milliseconds m_gameObjectAccum{ 0 };
    std::chrono::milliseconds m_playersAccum{ 0 };
    std::chrono::milliseconds m_petsAccum{ 0 };
    std::chrono::milliseconds m_creaturesAccum{ 0 };
    uint32_t m_lastUpdateTime = 0;

    // Worldstates
    WorldStatesHandler& getWorldStatesHandler();
    void onWorldStateUpdate(uint32_t zone, uint32_t field, uint32_t value) override;

    // Update System
    std::mutex m_updateMutex;
    UpdateQueue _updates;
    PUpdateQueue _processQueue;
    void updateObjects();
    void flushPendingObjectUpdate(Object* obj);
    void pushToProcessed(Player* plr);
    // Mark object as updated
    void objectUpdated(Object* obj);

    // Combat Progress
    CombatProgressMap _combatProgress;
    bool isCombatInProgress();
    void addCombatInProgress(uint64_t guid);
    void removeCombatInProgress(uint64_t guid);

    // Script related
    InstanceScript* getScript();
    void loadInstanceScript();
    void callScriptUpdate();

    MapScriptInterface* getInterface();
    InstanceMap* getInstance() { return pInstance; }

private:
    std::unique_ptr<AscEmu::Threading::AEThread> m_thread;
    bool m_threadRunning = false;
    bool m_terminateThread = false;

    WorldStatesHandler worldstateshandler;
    std::unique_ptr<MapScriptInterface> ScriptInterface;
    bool m_unloadPending = false;

    std::unique_ptr<TerrainHolder> _terrain;
    uint32_t _instanceId;
    uint8_t _instanceSpawnMode = InstanceDifficulty::Difficulties::DUNGEON_NORMAL;

    // Sessions
    std::set<WorldSession*> Sessions;
    std::mutex m_sessionMutex;

protected:
    InstanceScript* mInstanceScript = nullptr;
    DynamicMapTree _dynamicTree;
    uint32_t m_unloadTimer = 0;

    void setVisibilityDistance(float distance) noexcept
    {
        m_visibilityDistance = distance > 0.0f ? distance : 0.0f;
        m_visibilityDistanceSq = m_visibilityDistance * m_visibilityDistance;
    }

    float m_visibilityDistance = 0.0f;
    float m_visibilityDistanceSq = 0.0f;
    BaseMap* m_baseMap = nullptr;
    InstanceMap* pInstance = nullptr;
};
