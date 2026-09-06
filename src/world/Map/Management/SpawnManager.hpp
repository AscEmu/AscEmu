/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <ctime>
#include <array>
#include <algorithm>
#include <cstdint>
#include <variant>
#include <optional>
#include "Map/Visibility/VisibilitySystem.hpp"
#include "Map/SpawnTypes.hpp"
#include "Storage/MySQLStructures.h"

/// Forward declarations to avoid heavy includes in the header.
class Object;
class Creature;
class GameObject;
class ObjectFactory;

class LocationVector;
struct QuaternionData;

namespace visibility
{
    class VisibilitySystem;
}

class WoWGuid;

class WorldMap;

//////////////////////////////////////////////////////////////////////////////////////////
/// Hashable key for spawns (type + id).
//////////////////////////////////////////////////////////////////////////////////////////
struct SpawnKey
{
    SpawnObjectType type;
    uint32_t id;

    bool operator==(const SpawnKey& o) const noexcept
    {
        return type == o.type && id == o.id;
    }

    bool operator!=(SpawnKey const& rhs) const noexcept
    {
        return !(*this == rhs);
    }
};

struct SpawnKeyHash
{
    size_t operator()(const SpawnKey& k) const noexcept
    {
        return (static_cast<size_t>(k.type) << 32) ^ static_cast<size_t>(k.id);
    }
};


//////////////////////////////////////////////////////////////////////////////////////////
/// Authoritative in-memory definition/lifecycle state for one managed spawn.
/// No live object pointer is stored here; current instances are resolved through WorldObjectRegistry.
//////////////////////////////////////////////////////////////////////////////////////////
struct SpawnState
{
    SpawnKey key{ SPAWN_TYPE_INVALID, 0 };

    uint32_t entry{ 0 };
    int homeGrid{ -1 };

    bool persistent{ false };
    bool allowRespawn{ true };
    bool desiredInWorld{ true };

    bool respawnPending{ false };
    time_t respawnTime{ 0 };

    std::variant<
        MySQLStructure::CreatureSpawn,
        MySQLStructure::GameobjectSpawn
    > templateData;

    /// gameobject_spawns does not contain scale/faction/flags. Keep the optional
    /// override next to the GO template so runtime and DB-backed GOs recreate identically.
    std::optional<MySQLStructure::GameObjectSpawnOverrides> gameObjectOverrides;
    uint32_t gameObjectOverridesMask{ 0 };
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Read-only SpawnManager diagnostic counters.
//////////////////////////////////////////////////////////////////////////////////////////
struct SpawnManagerSnapshot
{
    size_t states{ 0 };
    size_t instances{ 0 };
    size_t inWorld{ 0 };
    size_t desiredInWorld{ 0 };
    size_t persistent{ 0 };
    size_t ephemeral{ 0 };
    size_t pendingRespawn{ 0 };

    size_t guidIndex{ 0 };
    size_t moveGuidIndex{ 0 };
    size_t homeGrids{ 0 };
    size_t currentGrids{ 0 };
    size_t activeGrids{ 0 };
    size_t unloadingGrids{ 0 };
    size_t pendingAdds{ 0 };

    size_t creatureRespawns{ 0 };
    size_t gameObjectRespawns{ 0 };
    size_t respawnQueue{ 0 };
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Read-only diagnostic state for one grid.
//////////////////////////////////////////////////////////////////////////////////////////
struct SpawnManagerGridSnapshot
{
    int gid{ 0 };
    bool active{ false };
    bool unloading{ false };

    size_t homeStates{ 0 };
    size_t currentStates{ 0 };

    size_t homeInstances{ 0 };
    size_t homeInWorld{ 0 };
    size_t homeDesiredInWorld{ 0 };
    size_t homePersistent{ 0 };
    size_t homeEphemeral{ 0 };
    size_t homePendingRespawn{ 0 };
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Respawn scheduling payload.
//////////////////////////////////////////////////////////////////////////////////////////
struct RespawnInfo
{
    SpawnObjectType type;
    uint32_t spawnId;
    uint32_t entry;
    time_t   time;                  /// UNIX seconds when the spawn should respawn
};

struct CompareRespawnInfo
{
    bool operator()(std::unique_ptr<RespawnInfo> const& a, std::unique_ptr<RespawnInfo> const& b) const
    {
        if (a->time != b->time)
            return a->time > b->time; // min-heap via priority_queue invert
        if (a->spawnId != b->spawnId)
            return a->spawnId < b->spawnId;
        return a->type < b->type;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Priority queue with removal/clear helpers used for respawn timing.
//////////////////////////////////////////////////////////////////////////////////////////
class respawnQueue : public std::priority_queue<std::unique_ptr<RespawnInfo>, std::vector<std::unique_ptr<RespawnInfo>>, CompareRespawnInfo>

{
public:
    bool remove(RespawnInfo const* value)
    {
        auto it = std::find_if(this->c.begin(), this->c.end(), [value](std::unique_ptr<RespawnInfo> const& respawn) { return respawn.get() == value; });
        if (it != this->c.end())
        {
            this->c.erase(it);
            std::make_heap(this->c.begin(), this->c.end(), this->comp);
            return true;
        }

        return false;
    }

    void clear()
    {
        this->c.clear();
    }
};

using RespawnMap = std::unordered_map<uint32_t, RespawnInfo*>;

//////////////////////////////////////////////////////////////////////////////////////////
/// SpawnManager
///
/// Central manager responsible for:
///  - Creating/destroying live instances of creatures and gameobjects
///  - Maintaining template state per grid and per live object
///  - Scheduling and persisting respawns
///  - Reacting to grid activation/unload and object movement across grids
//////////////////////////////////////////////////////////////////////////////////////////
class SERVER_DECL SpawnManager
{
public:
    /// Construct a spawn manager for a given world map.
    /// \param map The owning world map (instance-aware).
    /// \param vis The visibility system for active grid checks.
    /// \param factory Factory responsible for creating/attaching/destroying objects.
    SpawnManager(WorldMap& map, visibility::VisibilitySystem& visibilitySystem, ObjectFactory& factory);


    //////////////////////////////////////////////////////////////////////////////////////////
    /// Spawn definitions
    //////////////////////////////////////////////////////////////////////////////////////////
    /// Load DB-backed spawn definitions into the central SpawnState registry.
    void loadSpawns(bool reload);

    /// Update/register a DB-backed definition. If the live object was ephemeral before
    /// save, its ephemeral SpawnKey is migrated instead of creating a duplicate state.
    void registerPersistentCreature(Creature* creature, const MySQLStructure::CreatureSpawn& row);
    void registerPersistentGameObject(GameObject* gameObject, const MySQLStructure::GameobjectSpawn& row);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Spawning API
    //////////////////////////////////////////////////////////////////////////////////////////
    static constexpr uint32_t EPHEMERAL_MASK = 0x80000000u;
    static constexpr bool isEphemeralSpawnId(uint32_t id) noexcept { return (id & EPHEMERAL_MASK) != 0; }

    /// Create managed non-persistent objects without attaching them to the world.
    Creature* createCreature(uint32_t entry, LocationVector const& pos);
    GameObject* createGameObject(uint32_t entry, LocationVector const& pos, QuaternionData const& rotation = QuaternionData {});
    GameObject* createGameObject(uint32_t entry, LocationVector const& pos, float scale);

    /// Spawn immediately. Supplying a DB row creates a persistent DB-backed instance;
    /// without a row this is an ephemeral managed spawn.
    Creature* spawnCreature(uint32_t entry, LocationVector const& pos, const MySQLStructure::CreatureSpawn* row = nullptr);
    GameObject* spawnGameObject(uint32_t entry, LocationVector const& pos, QuaternionData const& rotation = QuaternionData {}, const MySQLStructure::GameobjectSpawn* row = nullptr);
    GameObject* spawnGameObject(uint32_t entry, LocationVector const& pos, float scale);

    /// Explicit creation of a new persistent DB spawn.
    Creature* spawnPersistentCreature(uint32_t entry, LocationVector const& pos);
    GameObject* spawnPersistentGameObject(uint32_t entry, LocationVector const& pos, QuaternionData const& rotation = QuaternionData {});

    /// Convenience summons with explicit respawn policy.
    Creature* summonCreature(uint32_t entry, LocationVector const& pos, bool noRespawn = true);
    GameObject* summonGameObject(uint32_t entry, LocationVector const& pos, QuaternionData const& rotation = QuaternionData {}, bool noRespawn = true);

    /// Managed lifecycle.
    bool regenerateCreatureGuid(Creature* creature, uint32_t entry = 0);
    bool regenerateGameObjectGuid(GameObject* gameObject, uint32_t entry = 0);
    bool pushToWorld(Object* object);
    bool addToWorld(Object* object);
    bool removeFromWorld(Object* object);
    void processQueuedAdds();

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Despawn
    //////////////////////////////////////////////////////////////////////////////////////////
    /// Public despawn API: all delays are milliseconds.
    bool despawn(Object* object, uint32_t respawnDelayMs = 0, bool keepRespawnSchedule = false);
    bool despawn(uint64_t guidRaw, uint32_t respawnDelayMs = 0, bool keepRespawnSchedule = false);

    /// Absolute-time variant for lifecycle code that already owns a Unix respawn timestamp.
    bool despawnAt(Object* object, time_t respawnAt, bool keepRespawnSchedule = false);
    bool despawnAt(uint64_t guidRaw, time_t respawnAt, bool keepRespawnSchedule = false);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Grid lifecycle
    //////////////////////////////////////////////////////////////////////////////////////////
    /// Mark grid as active and spawn due instances.
    void onGridActivated(int gid);

    /// Grid unload handler: remove current instances while keeping restorable spawn definitions.
    void onGridUnload(int gid);

    /// Maintain live index when an object moves across grids.
    void onGridChanged(const WoWGuid& guid, int oldGid, int newGid);

    //////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////
    /// Record a creature death and schedule or defer its respawn.
    void addRespawnForCreature(Creature* c);

    RespawnMap& getRespawnMapForType(SpawnObjectType type);
    RespawnMap const& getRespawnMapForType(SpawnObjectType type) const;

    time_t getRespawnTime(SpawnObjectType type, uint32_t spawnId) const;
    time_t getCreatureRespawnTime(uint32_t spawnId) const { return getRespawnTime(SPAWN_TYPE_CREATURE, spawnId); }
    time_t getGORespawnTime(uint32_t spawnId) const { return getRespawnTime(SPAWN_TYPE_GAMEOBJECT, spawnId); }

    bool hasSavedRespawn(SpawnObjectType type, uint32_t spawnId, time_t& when_out) const;
    void ensureRespawnScheduled(SpawnObjectType type, uint32_t spawnId, uint32_t entry, time_t when);

    /// Load respawn times from DB and rebuild schedule.
    void loadRespawnTimes();

    /// Process due respawns (to be called from a periodic tick).
    void processRespawns();

    /// Force respawn now by spawn id or by live guid.
    bool respawnNow(SpawnObjectType type, uint32_t spawnId);
    bool respawnNow(SpawnObjectType type, uint64_t guidRaw);
    uint32_t spawnIdForGuid(uint64_t guidRaw) const;

    /// Resolve the current live creature instance for a DB/runtime spawn id.
    /// Spawn groups store stable spawn ids instead of raw Creature pointers.
    Creature* findLiveCreature(uint32_t spawnId) const;

    /// Query lifecycle metadata for the current live instance.
    bool isPersistentSpawn(uint64_t guidRaw) const;

    /// Copy the authoritative creature spawn template for the current live instance.
    /// Works for both persistent DB spawns and ephemeral/runtime spawns.
    bool getCreatureSpawnTemplate(uint64_t guidRaw, MySQLStructure::CreatureSpawn& out) const;

    /// Synchronize the authoritative Creature SpawnState from the current live object.
    /// updateHomePosition=true is reserved for explicit home/spawn-position edits.
    bool syncCreatureSpawn(Creature* creature, bool updateHomePosition = false);

    /// Synchronize the authoritative GameObject SpawnState from the current live object.
    /// updateHomePosition=true is used by explicit spawn-edit commands such as movehere.
    bool syncGameObjectSpawn(GameObject* gameObject, bool updateHomePosition = false);

    /// Clear all in-memory respawn info and DB rows for current map/instance.
    void deleteRespawnTimes();
    void unloadAllRespawnInfos();

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Despawn / DB helpers
    //////////////////////////////////////////////////////////////////////////////////////////
    void  scheduleRespawn(SpawnObjectType type, uint32_t spawnId, uint32_t entry, time_t when);
    void  cancelRespawn(SpawnObjectType type, uint32_t spawnId);
    bool  hasRespawnScheduled(SpawnObjectType type, uint32_t spawnId) const;

    void  saveRespawnDB(SpawnObjectType type, uint32_t spawnId, time_t when);
    void  deleteRespawnFromDB(SpawnObjectType type, uint32_t spawnId);
    static void deleteRespawnTimesInDB(uint32_t mapId, uint32_t instanceId);

    bool eraseGameObjectSpawnBySpawnID(uint32_t spawnID);
    bool eraseCreatureSpawnBySpawnID(uint32_t spawnID);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Debug / audit
    //////////////////////////////////////////////////////////////////////////////////////////
    /// Capture internal lifecycle/index counters for GM diagnostics.
    SpawnManagerSnapshot snapshot() const;
    /// Capture lifecycle/index counters for one grid.
    SpawnManagerGridSnapshot gridSnapshot(int gid) const;

    /// Snapshot of all grids that currently have spawn definitions.
    std::vector<int> definedGridIds() const;
    /// True when the manager currently considers this grid active.
    bool isGridActiveForDebug(int gid) const;
    /// Print internal consistency stats to the logger.
    void dumpInconsistencies() const;

private:
    /// Construct a new live instance from the stored template.
    void  spawnFromTemplate(const SpawnKey& key);

    /// Keep boss-linked spawns disabled after the encounter is completed.
    bool  isSpawnBlockedByBossState(const SpawnState& state) const;

    /// Save runtime changes before the live object is removed.
    void  refreshTemplateFromObjectNoLock(SpawnState& state, Object* object, bool updateHomePosition = false, bool syncPersistent = false);

    /// Resolve the current live object through the registry.
    Object* findLiveObject(const SpawnKey& key) const;

    /// Remove the current live instance and keep it only when runtime reuse is needed.
    bool deactivateLiveInstance(const SpawnKey& key, bool retainInstance, bool preserveDesiredState = true);

    /// Update the lightweight grid indices.
    void indexHomeNoLock(const SpawnKey& key, int gid);
    void unindexHomeNoLock(const SpawnKey& key, int gid);
    void indexCurrentNoLock(const SpawnKey& key, int gid);
    void unindexCurrentNoLock(const SpawnKey& key, int gid);

    /// Convenience access to respawn map for a given type (non-const/const).
    RespawnMap& mapFor(SpawnObjectType type)
    {
        return type == SPAWN_TYPE_CREATURE ? m_creatureRespawns : m_gameObjectRespawns;
    }

    const RespawnMap& mapFor(SpawnObjectType type) const
    {
        return type == SPAWN_TYPE_CREATURE ? m_creatureRespawns : m_gameObjectRespawns;
    }
    bool hasRespawnScheduledNoLock(SpawnObjectType type, uint32_t spawnId) const;

    /// True if grid is currently considered active.
    inline bool isGridActive(int gridId) const
    {
        return (m_activeGrids.find(gridId) != m_activeGrids.end()) &&
            (m_unloadingGrids.find(gridId) == m_unloadingGrids.end());
    }

    /// Allocate a temporary id for non-persistent/runtime spawns.
    uint32_t allocEphemeralId();

private:
    WorldMap& m_worldMap;
    visibility::VisibilitySystem& m_visibilitySystem;
    ObjectFactory& m_objectFactory;

    mutable std::shared_mutex m_mutex;

    /// Spawn definitions for DB and runtime spawns.
    std::unordered_map<SpawnKey, SpawnState, SpawnKeyHash> m_spawns;

    /// Grid indices point back to m_spawns.
    std::unordered_map<int, std::unordered_set<SpawnKey, SpawnKeyHash>> m_gridIndex;        // home grid -> spawn keys
    std::unordered_map<int, std::unordered_set<SpawnKey, SpawnKeyHash>> m_currentGridIndex; // current grid -> live spawn keys

    /// Link live GUIDs to their spawn state.
    std::unordered_map<uint64_t, SpawnKey> m_guidToSpawn; // live/current GUID -> SpawnKey
    std::unordered_map<SpawnKey, uint64_t, SpawnKeyHash> m_spawnToGuid; // SpawnKey -> live/current GUID

    mutable std::mutex                                       m_pendingAddsMutex;
    std::vector<Object*>                                     m_pendingAdds;

    std::unordered_set<int>                                 m_activeGrids;   // active grids
    std::unordered_set<int>                                 m_unloadingGrids;     // grids to be removed

    respawnQueue m_respawnTimes;
    RespawnMap m_creatureRespawns;
    RespawnMap m_gameObjectRespawns;

    uint32_t m_nextEphemeralId = 1;
};
