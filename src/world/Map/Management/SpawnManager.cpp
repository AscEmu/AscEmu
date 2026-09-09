/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SpawnManager.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "ObjectFactory.hpp"
#include "WorldObjectRegistry.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/GameObject.h"
#include "Server/DatabaseDefinition.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Map/SpawnGroups.hpp"
#include <algorithm>
#include <type_traits>

using visibility::worldToGrid;
using visibility::packGridId;

namespace
{
    int gridForPosition(LocationVector const& pos)
    {
        auto [gx, gy] = worldToGrid(pos);
        return packGridId(gx, gy);
    }
}

SpawnManager::SpawnManager(WorldMap& map, visibility::VisibilitySystem& visibilitySystem, ObjectFactory& factory)
    : m_worldMap(map), m_visibilitySystem(visibilitySystem), m_objectFactory(factory)
{
    loadSpawns(true);
}

uint32_t SpawnManager::allocEphemeralId()
{
    std::unique_lock lk(m_mutex);

    for (;;)
    {
        const uint32_t id = EPHEMERAL_MASK | (m_nextEphemeralId++);
        if (id == 0)
            continue;

        if (m_spawns.find({ SPAWN_TYPE_CREATURE, id }) == m_spawns.end() &&
            m_spawns.find({ SPAWN_TYPE_GAMEOBJECT, id }) == m_spawns.end())
            return id;
    }
}

void SpawnManager::indexHomeNoLock(const SpawnKey& key, int gid)
{
    if (gid >= 0)
        m_gridIndex[gid].insert(key);
}

void SpawnManager::unindexHomeNoLock(const SpawnKey& key, int gid)
{
    if (gid < 0)
        return;

    auto it = m_gridIndex.find(gid);
    if (it == m_gridIndex.end())
        return;

    it->second.erase(key);
    if (it->second.empty())
        m_gridIndex.erase(it);
}

void SpawnManager::indexCurrentNoLock(const SpawnKey& key, int gid)
{
    if (gid >= 0)
        m_currentGridIndex[gid].insert(key);
}

void SpawnManager::unindexCurrentNoLock(const SpawnKey& key, int gid)
{
    if (gid < 0)
        return;

    auto it = m_currentGridIndex.find(gid);
    if (it == m_currentGridIndex.end())
        return;

    it->second.erase(key);
    if (it->second.empty())
        m_currentGridIndex.erase(it);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Load all DB-backed spawn definitions into the central SpawnState registry.
///
/// This is the only DB-spawn registration pass. Grid activation never re-imports DB rows,
/// which prevents a second SpawnState from being created for the same DB spawn.
//////////////////////////////////////////////////////////////////////////////////////////
void SpawnManager::loadSpawns(bool reload)
{
    const uint32_t mapId = m_worldMap.getBaseMap()->getMapId();

    std::unique_lock lk(m_mutex);

    if (reload)
    {
        /// loadSpawns() is currently used during map construction. Keep the semantics
        /// explicit: DB definitions are rebuilt from the datastore, runtime definitions
        /// would only exist after this point.
        m_spawns.clear();
        m_gridIndex.clear();
        m_currentGridIndex.clear();
        m_guidToSpawn.clear();
        m_spawnToGuid.clear();
    }

    uint32_t creatureCount = 0;
    uint32_t gameObjectCount = 0;

    if (!sMySQLStore.isTransportMap(mapId) && mapId < sMySQLStore._creatureSpawnsStore.size())
    {
        for (const auto* row : sMySQLStore._creatureSpawnsStore[mapId])
        {
            if (!row)
                continue;

            SpawnKey key{ SPAWN_TYPE_CREATURE, row->id };
            SpawnState state{};
            state.key = key;
            state.entry = row->entry;
            state.homeGrid = gridForPosition(row->spawnPoint);
            state.persistent = true;
            state.allowRespawn = true;
            state.desiredInWorld = true;
            state.templateData = *row;

            if (auto existing = m_spawns.find(key); existing != m_spawns.end())
            {
                unindexHomeNoLock(key, existing->second.homeGrid);
                sLogger.warning("SpawnMgr: duplicate creature spawnId {} in datastore for map {}; keeping the last definition.",
                    row->id, mapId);
            }

            m_spawns[key] = std::move(state);
            indexHomeNoLock(key, m_spawns[key].homeGrid);
            ++creatureCount;
        }
    }

    if (mapId < sMySQLStore._gameobjectSpawnsStore.size())
    {
        for (const auto* row : sMySQLStore._gameobjectSpawnsStore[mapId])
        {
            if (!row)
                continue;

            SpawnKey key{ SPAWN_TYPE_GAMEOBJECT, row->id };
            SpawnState state{};
            state.key = key;
            state.entry = row->entry;
            state.homeGrid = gridForPosition(row->spawnPoint);
            state.persistent = true;
            state.allowRespawn = true;
            state.desiredInWorld = true;
            state.templateData = *row;

            if (auto const* goOverride = sMySQLStore.getGameObjectOverride(row->id))
                state.gameObjectOverrides = *goOverride;

            if (auto existing = m_spawns.find(key); existing != m_spawns.end())
            {
                unindexHomeNoLock(key, existing->second.homeGrid);
                sLogger.warning("SpawnMgr: duplicate gameobject spawnId {} in datastore for map {}; keeping the last definition.",
                    row->id, mapId);
            }

            m_spawns[key] = std::move(state);
            indexHomeNoLock(key, m_spawns[key].homeGrid);
            ++gameObjectCount;
        }
    }

    lk.unlock();

    sLogger.info("SpawnMgr: Loaded {} creature spawns and {} gameobject spawns for map {}.",
        creatureCount, gameObjectCount, mapId);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Capture the current live configuration into the authoritative spawn definition.
///
/// Normal lifecycle calls leave persistent DB definitions unchanged; explicit spawn-edit
/// commands may opt in to synchronizing persistent in-memory state before writing the DB.
//////////////////////////////////////////////////////////////////////////////////////////
void SpawnManager::refreshTemplateFromObjectNoLock(SpawnState& state, Object* object, bool updateHomePosition, bool syncPersistent)
{
    if (!object || (state.persistent && !syncPersistent))
        return;

    state.entry = object->getEntry();

    if (state.key.type == SPAWN_TYPE_CREATURE)
    {
        Creature* creature = object->ToCreature();
        if (!creature)
            return;

        MySQLStructure::CreatureSpawn row{};
        if (auto const* existing = std::get_if<MySQLStructure::CreatureSpawn>(&state.templateData))
            row = *existing;

        row.id = state.key.id;
        row.entry = creature->getEntry();
        row.mapId = m_worldMap.getBaseMap()->getMapId();
        /// Normal movement must never change the home position. Only explicit spawn-edit
        /// operations opt in to updating it.
        if (updateHomePosition)
            row.spawnPoint = creature->GetPosition();
        else if (auto const* existing = std::get_if<MySQLStructure::CreatureSpawn>(&state.templateData))
            row.spawnPoint = existing->spawnPoint;
        else
            row.spawnPoint = creature->GetPosition();
        row.movetype = creature->getDefaultMovementType();
        row.displayid = creature->getDisplayId();
        row.factionid = creature->getFactionTemplate();
        row.flags = creature->getUnitFlags();
        row.pvp_flagged = creature->isPvpFlagSet() ? 1 : 0;
        row.bytes0 = creature->getBytes0();
        row.emote_state = creature->getEmoteState();
        row.stand_state = creature->getStandState();
        row.MountedDisplayID = creature->getMountDisplayId();
        row.sheath_state = creature->getSheathType();

#if VERSION_STRING < WotLK
        row.Item1SlotEntry = creature->getVirtualItemEntry(MELEE);
        row.Item2SlotEntry = creature->getVirtualItemEntry(OFFHAND);
        row.Item3SlotEntry = creature->getVirtualItemEntry(RANGED);
#else
        row.Item1SlotEntry = creature->getVirtualItemSlotId(MELEE);
        row.Item2SlotEntry = creature->getVirtualItemSlotId(OFFHAND);
        row.Item3SlotEntry = creature->getVirtualItemSlotId(RANGED);
#endif

        row.CanFly = creature->IsFlying() ? 1 : 0;
        row.phase = creature->GetPhase();

        state.templateData = std::move(row);
        return;
    }

    GameObject* go = object->ToGameObject();
    if (!go)
        return;

    MySQLStructure::GameobjectSpawn row{};
    if (auto const* existing = std::get_if<MySQLStructure::GameobjectSpawn>(&state.templateData))
        row = *existing;

    row.id = state.key.id;
    row.entry = go->getEntry();
    row.map = m_worldMap.getBaseMap()->getMapId();
    row.phase = go->GetPhase();
    /// Ordinary runtime movement must not redefine the home spawn. Explicit spawn-edit
    /// commands opt into updating the home position through syncGameObjectSpawn(..., true).
    if (updateHomePosition)
        row.spawnPoint = go->GetPosition();
    else if (auto const* existing = std::get_if<MySQLStructure::GameobjectSpawn>(&state.templateData))
        row.spawnPoint = existing->spawnPoint;
    else
        row.spawnPoint = go->GetPosition();
    row.rotation = QuaternionData(
        go->getParentRotation(0),
        go->getParentRotation(1),
        go->getParentRotation(2),
        go->getParentRotation(3));
    row.spawntimesecs = go->getRespawnDelay();
    row.state = GameObject_State(go->getState());
    row.origine.clear();

    state.templateData = std::move(row);

    MySQLStructure::GameObjectSpawnOverrides overrides{};
    overrides.id = state.key.id;
    overrides.scale = go->getScale();
    overrides.faction = go->getFactionTemplate();
    overrides.flags = go->getFlags();
    state.gameObjectOverrides = overrides;
    state.gameObjectOverridesMask = go->GetOverrides();
}

Object* SpawnManager::findSpawnObject(const SpawnKey& key) const
{
    uint64_t guidRaw = 0;
    {
        std::shared_lock lk(m_mutex);
        auto it = m_spawnToGuid.find(key);
        if (it == m_spawnToGuid.end())
            return nullptr;
        guidRaw = it->second;
    }

    return guidRaw ? m_worldMap.getObject(WoWGuid(guidRaw)) : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Register/update a DB-backed creature definition and migrate an old ephemeral key.
///
/// The current object remains the same live instance; only its spawn identity changes.
//////////////////////////////////////////////////////////////////////////////////////////
void SpawnManager::registerPersistentCreature(Creature* creature, const MySQLStructure::CreatureSpawn& row)
{
    if (!creature)
        return;

    const uint64_t guidRaw = creature->GetNewGUID().getRawGuid();
    const SpawnKey newKey{ SPAWN_TYPE_CREATURE, row.id };
    const int newHomeGrid = gridForPosition(row.spawnPoint);
    const int currentGrid = creature->IsInWorld() ? gridForPosition(creature->GetPosition()) : -1;

    std::unique_lock lk(m_mutex);

    if (auto existingGuid = m_spawnToGuid.find(newKey);
        existingGuid != m_spawnToGuid.end() && existingGuid->second != guidRaw)
    {
        sLogger.failure("SpawnMgr: refusing duplicate creature DB spawn {} on map {}.", row.id, m_worldMap.getBaseMap()->getMapId());
        return;
    }

    SpawnState carried{};
    bool haveCarried = false;

    if (auto oldIt = m_guidToSpawn.find(guidRaw); oldIt != m_guidToSpawn.end())
    {
        const SpawnKey oldKey = oldIt->second;
        if (auto stateIt = m_spawns.find(oldKey); stateIt != m_spawns.end())
        {
            carried = stateIt->second;
            haveCarried = true;
            unindexHomeNoLock(oldKey, stateIt->second.homeGrid);
            for (auto& [gid, keys] : m_currentGridIndex)
                keys.erase(oldKey);
            m_spawns.erase(stateIt);
        }

        m_spawnToGuid.erase(oldKey);
        m_guidToSpawn.erase(oldIt);
    }

    SpawnState state{};
    state.key = newKey;
    state.entry = row.entry;
    state.homeGrid = newHomeGrid;
    state.persistent = true;
    state.allowRespawn = haveCarried ? carried.allowRespawn : true;
    state.desiredInWorld = haveCarried ? carried.desiredInWorld : creature->IsInWorld();
    state.respawnPending = haveCarried ? carried.respawnPending : false;
    state.respawnTime = haveCarried ? carried.respawnTime : 0;
    state.templateData = row;

    m_spawns[newKey] = std::move(state);
    indexHomeNoLock(newKey, newHomeGrid);

    m_guidToSpawn[guidRaw] = newKey;
    m_spawnToGuid[newKey] = guidRaw;
    if (currentGrid >= 0)
        indexCurrentNoLock(newKey, currentGrid);

    creature->setSpawnId(row.id);
}

void SpawnManager::registerPersistentGameObject(GameObject* go, const MySQLStructure::GameobjectSpawn& row)
{
    if (!go)
        return;

    const uint64_t guidRaw = go->GetNewGUID().getRawGuid();
    const SpawnKey newKey{ SPAWN_TYPE_GAMEOBJECT, row.id };
    const int newHomeGrid = gridForPosition(row.spawnPoint);
    const int currentGrid = go->IsInWorld() ? gridForPosition(go->GetPosition()) : -1;

    std::unique_lock lk(m_mutex);

    if (auto existingGuid = m_spawnToGuid.find(newKey);
        existingGuid != m_spawnToGuid.end() && existingGuid->second != guidRaw)
    {
        sLogger.failure("SpawnMgr: refusing duplicate gameobject DB spawn {} on map {}.", row.id, m_worldMap.getBaseMap()->getMapId());
        return;
    }

    SpawnState carried{};
    bool haveCarried = false;

    if (auto oldIt = m_guidToSpawn.find(guidRaw); oldIt != m_guidToSpawn.end())
    {
        const SpawnKey oldKey = oldIt->second;
        if (auto stateIt = m_spawns.find(oldKey); stateIt != m_spawns.end())
        {
            carried = stateIt->second;
            haveCarried = true;
            unindexHomeNoLock(oldKey, stateIt->second.homeGrid);
            for (auto& [gid, keys] : m_currentGridIndex)
                keys.erase(oldKey);
            m_spawns.erase(stateIt);
        }

        m_spawnToGuid.erase(oldKey);
        m_guidToSpawn.erase(oldIt);
    }

    SpawnState state{};
    state.key = newKey;
    state.entry = row.entry;
    state.homeGrid = newHomeGrid;
    state.persistent = true;
    state.allowRespawn = haveCarried ? carried.allowRespawn : true;
    state.desiredInWorld = haveCarried ? carried.desiredInWorld : go->IsInWorld();
    state.respawnPending = haveCarried ? carried.respawnPending : false;
    state.respawnTime = haveCarried ? carried.respawnTime : 0;
    state.templateData = row;

    MySQLStructure::GameObjectSpawnOverrides overrides{};
    overrides.id = row.id;
    overrides.scale = go->getScale();
    overrides.faction = go->getFactionTemplate();
    overrides.flags = go->getFlags();
    state.gameObjectOverrides = overrides;

    m_spawns[newKey] = std::move(state);
    indexHomeNoLock(newKey, newHomeGrid);

    m_guidToSpawn[guidRaw] = newKey;
    m_spawnToGuid[newKey] = guidRaw;
    if (currentGrid >= 0)
        indexCurrentNoLock(newKey, currentGrid);

    go->setSpawnId(row.id);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Create managed runtime objects without attaching them to the world.
//////////////////////////////////////////////////////////////////////////////////////////
Creature* SpawnManager::createCreature(uint32_t entry, LocationVector const& pos)
{
    Creature* creature = m_objectFactory.createCreature(entry, pos);
    if (!creature)
        return nullptr;

    creature->setLifecycleMap(&m_worldMap);

    SpawnKey key{ SPAWN_TYPE_CREATURE, allocEphemeralId() };
    creature->setSpawnId(key.id);

    SpawnState state{};
    state.key = key;
    state.entry = entry;
    state.homeGrid = gridForPosition(pos);
    state.persistent = false;
    state.allowRespawn = true;
    state.desiredInWorld = false;

    MySQLStructure::CreatureSpawn row{};
    row.id = key.id;
    row.entry = entry;
    row.mapId = m_worldMap.getBaseMap()->getMapId();
    row.spawnPoint = pos;
    state.templateData = row;

    const uint64_t guidRaw = creature->GetNewGUID().getRawGuid();

    {
        std::unique_lock lk(m_mutex);
        m_spawns.emplace(key, std::move(state));
        indexHomeNoLock(key, gridForPosition(pos));
        m_guidToSpawn[guidRaw] = key;
        m_spawnToGuid[key] = guidRaw;

        auto it = m_spawns.find(key);
        if (it != m_spawns.end())
            refreshTemplateFromObjectNoLock(it->second, creature);
    }

    return creature;
}

GameObject* SpawnManager::createGameObject(uint32_t entry, LocationVector const& pos, QuaternionData const& rotation)
{
    GameObject* go = m_objectFactory.createGameObject(entry, pos, rotation);
    if (!go)
        return nullptr;

    go->setLifecycleMap(&m_worldMap);

    SpawnKey key{ SPAWN_TYPE_GAMEOBJECT, allocEphemeralId() };
    go->setSpawnId(key.id);

    SpawnState state{};
    state.key = key;
    state.entry = entry;
    state.homeGrid = gridForPosition(pos);
    state.persistent = false;
    state.allowRespawn = true;
    state.desiredInWorld = false;

    MySQLStructure::GameobjectSpawn row{};
    row.id = key.id;
    row.entry = entry;
    row.map = m_worldMap.getBaseMap()->getMapId();
    row.phase = go->GetPhase();
    row.spawnPoint = pos;
    row.rotation = rotation;
    row.state = GameObject_State(go->getState());
    state.templateData = row;

    const uint64_t guidRaw = go->GetNewGUID().getRawGuid();

    {
        std::unique_lock lk(m_mutex);
        m_spawns.emplace(key, std::move(state));
        indexHomeNoLock(key, gridForPosition(pos));
        m_guidToSpawn[guidRaw] = key;
        m_spawnToGuid[key] = guidRaw;

        auto it = m_spawns.find(key);
        if (it != m_spawns.end())
            refreshTemplateFromObjectNoLock(it->second, go);
    }

    return go;
}

GameObject* SpawnManager::createGameObject(uint32_t entry, LocationVector const& pos, float scale)
{
    GameObject* go = createGameObject(entry, pos, QuaternionData{});
    if (go)
        go->setScale(scale);
    return go;
}

bool SpawnManager::regenerateCreatureGuid(Creature* creature, uint32_t entry)
{
    if (!creature || creature->IsInWorld())
        return false;

    const WoWGuid oldWowGuid = creature->GetNewGUID();
    const uint64_t oldGuid = oldWowGuid.getRawGuid();
    const uint32_t guidEntry = entry != 0 ? entry : creature->getEntry();
    const uint64_t newGuid = m_objectFactory.generateCreatureGuid(guidEntry);
    const WoWGuid newWowGuid = newGuid;

    if (newGuid == oldGuid)
        return true;

    std::unique_lock lk(m_mutex);

    auto oldIt = m_guidToSpawn.find(oldGuid);
    if (oldIt == m_guidToSpawn.end())
        return false;

    const SpawnKey key = oldIt->second;
    if (key.type != SPAWN_TYPE_CREATURE)
        return false;

    auto stateIt = m_spawns.find(key);
    if (stateIt == m_spawns.end())
        return false;

    m_guidToSpawn.erase(oldIt);
    m_spawnToGuid.erase(key);

    /// Detached runtime objects remain registered. Re-key the registry together
    /// with the object GUID so no stale oldGuid -> creature entry survives.
    m_worldMap.getRegistry().rekey(creature, oldWowGuid, newWowGuid);
    creature->setGuid(newGuid);

    stateIt->second.entry = guidEntry;
    m_guidToSpawn[newGuid] = key;
    m_spawnToGuid[key] = newGuid;
    refreshTemplateFromObjectNoLock(stateIt->second, creature);
    return true;
}

bool SpawnManager::regenerateGameObjectGuid(GameObject* gameObject, uint32_t entry)
{
    if (!gameObject || gameObject->IsInWorld())
        return false;

    const WoWGuid oldWowGuid = gameObject->GetNewGUID();
    const uint64_t oldGuid = oldWowGuid.getRawGuid();
    const uint32_t guidEntry = entry != 0 ? entry : gameObject->getEntry();
    const uint64_t newGuid = m_objectFactory.generateGameObjectGuid(guidEntry);
    const WoWGuid newWowGuid = newGuid;

    if (newGuid == oldGuid)
        return true;

    std::unique_lock lk(m_mutex);

    auto oldIt = m_guidToSpawn.find(oldGuid);
    if (oldIt == m_guidToSpawn.end())
        return false;

    const SpawnKey key = oldIt->second;
    auto stateIt = m_spawns.find(key);
    if (stateIt == m_spawns.end())
        return false;

    m_guidToSpawn.erase(oldIt);
    m_spawnToGuid.erase(key);

    /// The registry intentionally retains detached runtime objects. Re-key it
    /// together with the runtime GUID so no stale oldGuid -> object entry survives.
    m_worldMap.getRegistry().rekey(gameObject, oldWowGuid, newWowGuid);
    gameObject->setGuid(newGuid);

    stateIt->second.entry = guidEntry;
    m_guidToSpawn[newGuid] = key;
    m_spawnToGuid[key] = newGuid;
    refreshTemplateFromObjectNoLock(stateIt->second, gameObject);
    return true;
}

bool SpawnManager::pushToWorld(Object* object)
{
    if (!object)
        return false;

    if (object->IsInWorld())
        return object->getWorldMap() == &m_worldMap;

    const uint64_t guidRaw = object->GetNewGUID().getRawGuid();
    SpawnKey key{};

    {
        std::unique_lock lk(m_mutex);

        auto guidIt = m_guidToSpawn.find(guidRaw);
        if (guidIt == m_guidToSpawn.end())
            return false;

        key = guidIt->second;
        auto stateIt = m_spawns.find(key);
        if (stateIt == m_spawns.end())
            return false;

        refreshTemplateFromObjectNoLock(stateIt->second, object);

        stateIt->second.desiredInWorld = true;
        m_spawnToGuid[key] = guidRaw;
    }

    m_objectFactory.attachToWorld(object);
    if (!object->IsInWorld())
        return false;

    const int currentGrid = gridForPosition(object->GetPosition());

    {
        std::unique_lock lk(m_mutex);
        auto stateIt = m_spawns.find(key);
        if (stateIt == m_spawns.end())
            return false;

        stateIt->second.desiredInWorld = true;
        stateIt->second.respawnPending = false;
        stateIt->second.respawnTime = 0;
        m_guidToSpawn[guidRaw] = key;
        m_spawnToGuid[key] = guidRaw;
        indexCurrentNoLock(key, currentGrid);
    }

    return true;
}

bool SpawnManager::addToWorld(Object* object)
{
    if (!object || object->IsInWorld())
        return false;

    std::lock_guard lk(m_pendingAddsMutex);
    if (std::find(m_pendingAdds.begin(), m_pendingAdds.end(), object) == m_pendingAdds.end())
        m_pendingAdds.push_back(object);
    return true;
}

void SpawnManager::processQueuedAdds()
{
    std::vector<Object*> pending;
    {
        std::lock_guard lk(m_pendingAddsMutex);
        pending.swap(m_pendingAdds);
    }

    for (Object* object : pending)
        pushToWorld(object);
}

bool SpawnManager::removeFromWorld(Object* object)
{
    if (!object)
        return false;

    const uint64_t guidRaw = object->GetNewGUID().getRawGuid();
    SpawnKey key{};

    {
        std::unique_lock lk(m_mutex);
        auto guidIt = m_guidToSpawn.find(guidRaw);
        if (guidIt == m_guidToSpawn.end())
            return false;

        key = guidIt->second;
        auto stateIt = m_spawns.find(key);
        if (stateIt == m_spawns.end())
            return false;

        stateIt->second.desiredInWorld = false;
    }

    const int currentGrid = object->IsInWorld() ? gridForPosition(object->GetPosition()) : -1;

    if (object->IsInWorld())
        m_objectFactory.detachFromWorld(object, /*keepRegistry=*/true, /*soft=*/true);

    {
        std::unique_lock lk(m_mutex);
        unindexCurrentNoLock(key, currentGrid);
    }

    return true;
}

Creature* SpawnManager::summonCreature(uint32_t entry, LocationVector const& pos, bool noRespawn)
{
    Creature* creature = createCreature(entry, pos);
    if (!creature)
        return nullptr;

    creature->m_noRespawn = noRespawn;

    {
        std::unique_lock lk(m_mutex);
        if (auto guidIt = m_guidToSpawn.find(creature->GetNewGUID().getRawGuid()); guidIt != m_guidToSpawn.end())
        {
            if (auto stateIt = m_spawns.find(guidIt->second); stateIt != m_spawns.end())
                stateIt->second.allowRespawn = !noRespawn;
        }
    }

    return pushToWorld(creature) ? creature : nullptr;
}

GameObject* SpawnManager::summonGameObject(uint32_t entry, LocationVector const& pos, QuaternionData const& rotation, bool noRespawn)
{
    GameObject* go = createGameObject(entry, pos, rotation);
    if (!go)
        return nullptr;

    go->setNoRespawn(noRespawn);

    {
        std::unique_lock lk(m_mutex);
        if (auto guidIt = m_guidToSpawn.find(go->GetNewGUID().getRawGuid()); guidIt != m_guidToSpawn.end())
        {
            if (auto stateIt = m_spawns.find(guidIt->second); stateIt != m_spawns.end())
                stateIt->second.allowRespawn = !noRespawn;
        }
    }

    return pushToWorld(go) ? go : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Explicit row-backed spawn. Existing live instance wins, so callers cannot create a
/// duplicate DB spawn by invoking this while the grid loader already spawned it.
//////////////////////////////////////////////////////////////////////////////////////////
Creature* SpawnManager::spawnCreature(uint32_t entry, LocationVector const& pos, const MySQLStructure::CreatureSpawn* row)
{
    if (!row)
    {
        Creature* creature = createCreature(entry, pos);
        return creature && pushToWorld(creature) ? creature : nullptr;
    }

    const SpawnKey key{ SPAWN_TYPE_CREATURE, row->id };
    uint64_t existingGuid = 0;

    {
        std::unique_lock lk(m_mutex);

        auto stateIt = m_spawns.find(key);
        if (stateIt == m_spawns.end())
        {
            SpawnState state{};
            state.key = key;
            state.entry = row->entry;
            state.homeGrid = gridForPosition(row->spawnPoint);
            state.persistent = true;
            state.allowRespawn = true;
            state.desiredInWorld = true;
            state.templateData = *row;
            m_spawns.emplace(key, std::move(state));
            indexHomeNoLock(key, gridForPosition(row->spawnPoint));
        }
        else
        {
            const int oldHome = stateIt->second.homeGrid;
            stateIt->second.entry = row->entry;
            stateIt->second.homeGrid = gridForPosition(row->spawnPoint);
            stateIt->second.templateData = *row;
            stateIt->second.persistent = true;
            stateIt->second.desiredInWorld = true;

            if (oldHome != stateIt->second.homeGrid)
            {
                unindexHomeNoLock(key, oldHome);
                indexHomeNoLock(key, stateIt->second.homeGrid);
            }
        }

        if (auto liveIt = m_spawnToGuid.find(key); liveIt != m_spawnToGuid.end())
            existingGuid = liveIt->second;
    }

    if (existingGuid)
    {
        if (Object* object = m_worldMap.getObject(WoWGuid(existingGuid)))
        {
            if (object->IsInWorld())
            {
                sLogger.failure("SpawnMgr: creature DB spawn {} on map {} is already in world as guid {}.", row->id, m_worldMap.getBaseMap()->getMapId(), existingGuid);
                return object->ToCreature();
            }
        }
        else
        {
            std::unique_lock lk(m_mutex);
            m_guidToSpawn.erase(existingGuid);
            m_spawnToGuid.erase(key);
        }
    }

    if (Object* object = spawnFromTemplate(key))
        return object->ToCreature();

    return nullptr;
}

GameObject* SpawnManager::spawnGameObject(uint32_t entry, LocationVector const& pos, QuaternionData const& rotation, const MySQLStructure::GameobjectSpawn* row)
{
    if (!row)
    {
        GameObject* go = createGameObject(entry, pos, rotation);
        return go && pushToWorld(go) ? go : nullptr;
    }

    const SpawnKey key{ SPAWN_TYPE_GAMEOBJECT, row->id };
    uint64_t existingGuid = 0;

    {
        std::unique_lock lk(m_mutex);

        auto stateIt = m_spawns.find(key);
        if (stateIt == m_spawns.end())
        {
            SpawnState state{};
            state.key = key;
            state.entry = row->entry;
            state.homeGrid = gridForPosition(row->spawnPoint);
            state.persistent = true;
            state.allowRespawn = true;
            state.desiredInWorld = true;
            state.templateData = *row;

            if (auto const* goOverride = sMySQLStore.getGameObjectOverride(row->id))
                state.gameObjectOverrides = *goOverride;

            m_spawns.emplace(key, std::move(state));
            indexHomeNoLock(key, gridForPosition(row->spawnPoint));
        }
        else
        {
            const int oldHome = stateIt->second.homeGrid;
            stateIt->second.entry = row->entry;
            stateIt->second.homeGrid = gridForPosition(row->spawnPoint);
            stateIt->second.templateData = *row;
            stateIt->second.persistent = true;
            stateIt->second.desiredInWorld = true;

            if (auto const* goOverride = sMySQLStore.getGameObjectOverride(row->id))
                stateIt->second.gameObjectOverrides = *goOverride;

            if (oldHome != stateIt->second.homeGrid)
            {
                unindexHomeNoLock(key, oldHome);
                indexHomeNoLock(key, stateIt->second.homeGrid);
            }
        }

        if (auto liveIt = m_spawnToGuid.find(key); liveIt != m_spawnToGuid.end())
            existingGuid = liveIt->second;
    }

    if (existingGuid)
    {
        if (Object* object = m_worldMap.getObject(WoWGuid(existingGuid)))
        {
            if (object->IsInWorld())
            {
                sLogger.failure("SpawnMgr: gameobject DB spawn {} on map {} is already in world as guid {}.", row->id, m_worldMap.getBaseMap()->getMapId(), existingGuid);
                return object->ToGameObject();
            }
        }
        else
        {
            std::unique_lock lk(m_mutex);
            m_guidToSpawn.erase(existingGuid);
            m_spawnToGuid.erase(key);
        }
    }

    if (Object* object = spawnFromTemplate(key))
        return object->ToGameObject();

    return nullptr;
}

Creature* SpawnManager::spawnPersistentCreature(uint32_t entry, LocationVector const& pos)
{
    Creature* creature = createCreature(entry, pos);
    if (!creature || !pushToWorld(creature))
        return nullptr;

    creature->SaveToDB();
    return creature;
}

GameObject* SpawnManager::spawnPersistentGameObject(uint32_t entry, LocationVector const& pos, QuaternionData const& rotation)
{
    GameObject* go = createGameObject(entry, pos, rotation);
    if (!go || !pushToWorld(go))
        return nullptr;

    go->saveToDB(true);
    return go;
}

GameObject* SpawnManager::spawnGameObject(uint32_t entry, LocationVector const& pos, float scale)
{
    GameObject* go = createGameObject(entry, pos, scale);
    return go && pushToWorld(go) ? go : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Grid activation is now a pure lookup into the single SpawnState registry.
//////////////////////////////////////////////////////////////////////////////////////////
void SpawnManager::onGridActivated(int gid)
{
    std::vector<SpawnKey> keys;

    {
        std::unique_lock lk(m_mutex);
        if (!m_activeGrids.insert(gid).second)
            return;

        m_unloadingGrids.erase(gid);

        if (auto it = m_gridIndex.find(gid); it != m_gridIndex.end())
            keys.assign(it->second.begin(), it->second.end());
    }

    const time_t now = std::time(nullptr);

    for (const SpawnKey& key : keys)
    {
        bool shouldSpawn = false;

        {
            std::unique_lock lk(m_mutex);
            auto stateIt = m_spawns.find(key);
            if (stateIt == m_spawns.end())
                continue;

            SpawnState& state = stateIt->second;
            if (!state.desiredInWorld)
                continue;

            /// Grid activation is not a respawn decision. noRespawn/allowRespawn only
            /// controls what happens after a real death/despawn. A living runtime spawn
            /// must survive a grid unload regardless of allowRespawn.
            if (state.respawnPending && state.respawnTime > now)
                continue;

            /// Keep the due respawn marker until spawnFromTemplate() has successfully
            /// restored the instance. It also tells the object to run its respawn reset.
            shouldSpawn = true;
        }

        if (shouldSpawn)
            spawnFromTemplate(key);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Grid unload destroys only current instances. Spawn definitions remain in m_gridIndex.
///
/// Moved objects are recreated at their home spawn if that grid is still active.
//////////////////////////////////////////////////////////////////////////////////////////
void SpawnManager::onGridUnload(int gid)
{
    std::vector<SpawnKey> currentKeys;

    {
        std::unique_lock lk(m_mutex);
        m_unloadingGrids.insert(gid);

        if (auto it = m_currentGridIndex.find(gid); it != m_currentGridIndex.end())
            currentKeys.assign(it->second.begin(), it->second.end());
    }

    std::vector<SpawnKey> relocateToHome;

    for (const SpawnKey& key : currentKeys)
    {
        int homeGrid = -1;
        bool desired = false;
        bool pending = false;
        bool persistent = false;

        {
            std::shared_lock lk(m_mutex);
            auto stateIt = m_spawns.find(key);
            if (stateIt == m_spawns.end())
                continue;

            homeGrid = stateIt->second.homeGrid;
            desired = stateIt->second.desiredInWorld;
            pending = stateIt->second.respawnPending;
            persistent = stateIt->second.persistent;
        }

        /// Grid unload is only a world-lifetime transition, never a gameplay despawn.
        const bool retainInstance = !persistent;
        deactivateSpawnInstance(key, retainInstance, /*preserveDesiredState=*/true);

        if (homeGrid != gid && desired && !pending)
        {
            std::shared_lock lk(m_mutex);
            if (isGridActive(homeGrid))
                relocateToHome.push_back(key);
        }
    }

    {
        std::unique_lock lk(m_mutex);
        m_currentGridIndex.erase(gid);
        m_activeGrids.erase(gid);
        m_unloadingGrids.erase(gid);
    }

    for (const SpawnKey& key : relocateToHome)
        spawnFromTemplate(key);
}

void SpawnManager::onGridChanged(const WoWGuid& guid, int oldGid, int newGid)
{
    if (oldGid == newGid)
        return;

    std::unique_lock lk(m_mutex);

    auto keyIt = m_guidToSpawn.find(guid.getRawGuid());
    if (keyIt == m_guidToSpawn.end())
        return;

    const SpawnKey key = keyIt->second;
    unindexCurrentNoLock(key, oldGid);
    indexCurrentNoLock(key, newGid);
}

void SpawnManager::addRespawnForCreature(Creature* creature)
{
    if (!creature)
        return;

    const uint32_t spawnId = creature->getSpawnId();
    const time_t respawnAt = creature->getRespawnTime();

    SpawnKey key{ SPAWN_TYPE_CREATURE, spawnId };
    uint32_t entry = 0;
    bool allowed = false;

    {
        std::shared_lock lk(m_mutex);
        auto it = m_spawns.find(key);
        if (it == m_spawns.end())
            return;

        entry = it->second.entry;
        allowed = it->second.allowRespawn;
    }

    if (!allowed)
    {
        cancelRespawn(SPAWN_TYPE_CREATURE, spawnId);
        return;
    }

    scheduleRespawn(SPAWN_TYPE_CREATURE, spawnId, entry, respawnAt);
}

RespawnMap& SpawnManager::getRespawnMapForType(SpawnObjectType type)
{
    return type == SPAWN_TYPE_CREATURE ? m_creatureRespawns : m_gameObjectRespawns;
}

RespawnMap const& SpawnManager::getRespawnMapForType(SpawnObjectType type) const
{
    return type == SPAWN_TYPE_CREATURE ? m_creatureRespawns : m_gameObjectRespawns;
}

time_t SpawnManager::getRespawnTime(SpawnObjectType type, uint32_t spawnId) const
{
    std::shared_lock lk(m_mutex);
    auto const& map = getRespawnMapForType(type);
    auto it = map.find(spawnId);
    return it == map.end() ? 0 : it->second->time;
}

uint32_t SpawnManager::spawnIdForGuid(uint64_t guidRaw) const
{
    std::shared_lock lk(m_mutex);
    auto it = m_guidToSpawn.find(guidRaw);
    return it == m_guidToSpawn.end() ? 0 : it->second.id;
}

Creature* SpawnManager::findLiveCreature(uint32_t spawnId) const
{
    Object* object = findSpawnObject({ SPAWN_TYPE_CREATURE, spawnId });
    return object && object->isCreature() ? static_cast<Creature*>(object) : nullptr;
}


bool SpawnManager::isPersistentSpawn(uint64_t guidRaw) const
{
    std::shared_lock lk(m_mutex);

    auto guidIt = m_guidToSpawn.find(guidRaw);
    if (guidIt == m_guidToSpawn.end())
        return false;

    auto stateIt = m_spawns.find(guidIt->second);
    return stateIt != m_spawns.end() && stateIt->second.persistent;
}

bool SpawnManager::getCreatureSpawnTemplate(uint64_t guidRaw, MySQLStructure::CreatureSpawn& out) const
{
    std::shared_lock lk(m_mutex);

    auto guidIt = m_guidToSpawn.find(guidRaw);
    if (guidIt == m_guidToSpawn.end() || guidIt->second.type != SPAWN_TYPE_CREATURE)
        return false;

    auto stateIt = m_spawns.find(guidIt->second);
    if (stateIt == m_spawns.end())
        return false;

    auto const* spawn = std::get_if<MySQLStructure::CreatureSpawn>(&stateIt->second.templateData);
    if (!spawn)
        return false;

    out = *spawn;
    return true;
}

bool SpawnManager::syncCreatureSpawn(Creature* creature, bool updateHomePosition)
{
    if (!creature)
        return false;

    const uint64_t guidRaw = creature->GetNewGUID().getRawGuid();
    std::unique_lock lk(m_mutex);

    auto guidIt = m_guidToSpawn.find(guidRaw);
    if (guidIt == m_guidToSpawn.end() || guidIt->second.type != SPAWN_TYPE_CREATURE)
        return false;

    auto stateIt = m_spawns.find(guidIt->second);
    if (stateIt == m_spawns.end())
        return false;

    const int oldHomeGrid = stateIt->second.homeGrid;
    refreshTemplateFromObjectNoLock(stateIt->second, creature, updateHomePosition, /*syncPersistent=*/true);

    if (updateHomePosition)
    {
        const int newHomeGrid = gridForPosition(creature->GetPosition());
        if (newHomeGrid != oldHomeGrid)
        {
            unindexHomeNoLock(stateIt->second.key, oldHomeGrid);
            stateIt->second.homeGrid = newHomeGrid;
            indexHomeNoLock(stateIt->second.key, newHomeGrid);
        }
    }

    return true;
}

bool SpawnManager::syncGameObjectSpawn(GameObject* gameObject, bool updateHomePosition)
{
    if (!gameObject)
        return false;

    const uint64_t guidRaw = gameObject->GetNewGUID().getRawGuid();
    std::unique_lock lk(m_mutex);

    auto guidIt = m_guidToSpawn.find(guidRaw);
    if (guidIt == m_guidToSpawn.end() || guidIt->second.type != SPAWN_TYPE_GAMEOBJECT)
        return false;

    auto stateIt = m_spawns.find(guidIt->second);
    if (stateIt == m_spawns.end())
        return false;

    const int oldHomeGrid = stateIt->second.homeGrid;
    refreshTemplateFromObjectNoLock(stateIt->second, gameObject, updateHomePosition, /*syncPersistent=*/true);

    if (updateHomePosition)
    {
        const int newHomeGrid = gridForPosition(gameObject->GetPosition());
        if (newHomeGrid != oldHomeGrid)
        {
            unindexHomeNoLock(stateIt->second.key, oldHomeGrid);
            stateIt->second.homeGrid = newHomeGrid;
            indexHomeNoLock(stateIt->second.key, newHomeGrid);
        }
    }

    return true;
}

bool SpawnManager::hasSavedRespawn(SpawnObjectType type, uint32_t spawnId, time_t& when_out) const
{
    std::shared_lock lk(m_mutex);
    auto const& mp = mapFor(type);
    auto it = mp.find(spawnId);
    if (it == mp.end())
        return false;

    when_out = it->second->time;
    return std::time(nullptr) < it->second->time;
}

void SpawnManager::ensureRespawnScheduled(SpawnObjectType type, uint32_t spawnId, uint32_t entry, time_t when)
{
    scheduleRespawn(type, spawnId, entry, when);
}

void SpawnManager::loadRespawnTimes()
{
    CharacterDatabase.execute(
        "DELETE FROM respawn "
        "WHERE mapId=%u AND instanceId=%u AND respawnTime <= UNIX_TIMESTAMP()",
        m_worldMap.getBaseMap()->getMapId(), m_worldMap.getInstanceId());

    auto result = CharacterDatabase.query(
        "SELECT type, spawnId, respawnTime FROM respawn WHERE mapId=%u AND instanceId=%u",
        m_worldMap.getBaseMap()->getMapId(), m_worldMap.getInstanceId());

    if (!result)
        return;

    do
    {
        Field* f = result->fetch();
        const auto type = static_cast<SpawnObjectType>(f[0].asUint16());
        const uint32_t spawnId = f[1].asUint32();
        const time_t when = static_cast<time_t>(f[2].asUint64());

        uint32_t entry = 0;
        {
            std::shared_lock lk(m_mutex);
            if (auto it = m_spawns.find({ type, spawnId }); it != m_spawns.end())
                entry = it->second.entry;
        }

        if (entry)
            scheduleRespawn(type, spawnId, entry, when);
    }
    while (result->nextRow());
}

void SpawnManager::processRespawns()
{
    const time_t now = std::time(nullptr);

    while (true)
    {
        SpawnKey dueKey{};
        bool shouldSpawn = false;
        bool persistent = false;

        {
            std::unique_lock lk(m_mutex);

            if (m_respawnTimes.empty())
                break;

            RespawnInfo* top = m_respawnTimes.top().get();
            if (now < top->time)
                break;

            const time_t dueTime = top->time;
            dueKey = { top->type, top->spawnId };

            RespawnMap& respawnMap = mapFor(top->type);
            respawnMap.erase(top->spawnId);
            m_respawnTimes.pop();

            auto stateIt = m_spawns.find(dueKey);
            if (stateIt == m_spawns.end())
                continue;

            SpawnState& state = stateIt->second;
            persistent = state.persistent;

            if (!state.allowRespawn || !state.desiredInWorld)
            {
                state.respawnPending = false;
                state.respawnTime = 0;
            }
            else if (!isGridActive(state.homeGrid))
            {
                /// Timer is due. Keep the due state in SpawnState; onGridActivated()
                /// will recreate it from templateData without needing a second queue entry.
                state.respawnPending = true;
                state.respawnTime = dueTime;
            }
            else
            {
                /// Keep respawnPending/respawnTime until spawnFromTemplate() succeeds.
                shouldSpawn = true;
            }
        }

        if (persistent)
            deleteRespawnFromDB(dueKey.type, dueKey.id);

        if (shouldSpawn)
            spawnFromTemplate(dueKey);
    }
}

bool SpawnManager::respawnNow(SpawnObjectType type, uint32_t spawnId)
{
    const SpawnKey key{ type, spawnId };
    bool persistent = false;
    int homeGrid = -1;

    {
        std::unique_lock lk(m_mutex);

        auto stateIt = m_spawns.find(key);
        if (stateIt == m_spawns.end() || !stateIt->second.allowRespawn)
            return false;

        if (auto respawnIt = mapFor(type).find(spawnId); respawnIt != mapFor(type).end())
        {
            m_respawnTimes.remove(respawnIt->second);
            mapFor(type).erase(respawnIt);
        }

        stateIt->second.respawnPending = true;
        stateIt->second.respawnTime = std::time(nullptr);
        stateIt->second.desiredInWorld = true;
        persistent = stateIt->second.persistent;
        homeGrid = stateIt->second.homeGrid;
    }

    if (persistent)
        deleteRespawnFromDB(type, spawnId);

    if (!isGridActiveForDebug(homeGrid))
        return true;

    if (Object* current = findSpawnObject(key))
        deactivateSpawnInstance(key, /*retainInstance=*/!persistent, /*preserveDesiredState=*/true);

    spawnFromTemplate(key);
    return true;
}

bool SpawnManager::respawnNow(SpawnObjectType type, uint64_t guidRaw)
{
    const uint32_t sid = spawnIdForGuid(guidRaw);
    return sid ? respawnNow(type, sid) : false;
}

void SpawnManager::deleteRespawnTimes()
{
    unloadAllRespawnInfos();
    deleteRespawnTimesInDB(m_worldMap.getBaseMap()->getMapId(), m_worldMap.getInstanceId());
}

void SpawnManager::unloadAllRespawnInfos()
{
    std::unique_lock lk(m_mutex);
    m_respawnTimes.clear();
    m_creatureRespawns.clear();
    m_gameObjectRespawns.clear();

    for (auto& [key, state] : m_spawns)
    {
        state.respawnPending = false;
        state.respawnTime = 0;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Recreate one instance from SpawnState::templateData.
///
/// No retained Object* exists in SpawnState. A fresh GUID is generated every time and
/// the live pointer becomes discoverable only through WorldObjectRegistry.
//////////////////////////////////////////////////////////////////////////////////////////
bool SpawnManager::isSpawnBlockedByBossState(const SpawnState& state) const
{
    /// Boss/encounter state only applies to DB-defined creature spawns. Runtime/ephemeral
    /// spawns must never become encounter-bound merely because an ephemeral id collides
    /// with a DB spawn id.
    if (state.key.type != SPAWN_TYPE_CREATURE || !state.persistent)
        return false;

    InstanceScript* script = m_worldMap.getScript();
    if (!script)
        return false;

    /// First block the encounter creature itself. This is required after a server restart
    /// and after grid unload/reload: a dead boss does not have a normal respawn timer, but
    /// its persistent spawn definition still exists and would otherwise be recreated from
    /// templateData. Encounter state is authoritative here; no runtime boss flag is needed.
    if (auto const* row = std::get_if<MySQLStructure::CreatureSpawn>(&state.templateData))
    {
        if (script->getBossStateByEntry(row->entry) == Performed)
            return true;
    }

    /// A spawn group with bossId belongs to that encounter independently of
    /// SPAWFLAG_FLAG_BOUNDTOBOSS. Once the linked boss is completed, its trash/group
    /// members must stay gone as well.
    const SpawnGroupTemplateData* group = sMySQLStore.getSpawnGroupDataBySpawn(state.key.id);
    if (!group || group->bossId == 0)
        return false;

    return script->getBossStateByEntry(group->bossId) == Performed;
}

Object* SpawnManager::spawnFromTemplate(const SpawnKey& key)
{
    SpawnState snapshot{};
    uint64_t retainedGuid = 0;

    {
        std::unique_lock lk(m_mutex);

        auto stateIt = m_spawns.find(key);
        if (stateIt == m_spawns.end())
            return nullptr;

        if (!stateIt->second.desiredInWorld)
            return nullptr;

        /// allowRespawn is intentionally not checked here. This function is also used
        /// to restore a still-living object after grid activation. A no-respawn summon
        /// remains desiredInWorld until its real death/despawn removes the SpawnState.
        if (!isGridActive(stateIt->second.homeGrid))
            return nullptr;

        if (stateIt->second.respawnPending && stateIt->second.respawnTime > std::time(nullptr))
            return nullptr;

        snapshot = stateIt->second;

        if (auto liveIt = m_spawnToGuid.find(key); liveIt != m_spawnToGuid.end())
            retainedGuid = liveIt->second;
    }

    /// A spawn group with bossId is tied to that encounter independently of
    /// SPAWFLAG_FLAG_BOUNDTOBOSS. Once the linked boss is completed, members of the
    /// group must stay gone. Keeping the gate at the common creation point applies
    /// the rule to initial/grid spawning and timer-driven respawns alike.
    if (isSpawnBlockedByBossState(snapshot))
    {
        bool deletePersistentRespawn = false;

        {
            std::unique_lock lk(m_mutex);
            auto stateIt = m_spawns.find(key);
            if (stateIt == m_spawns.end())
                return nullptr;

            stateIt->second.desiredInWorld = false;
            stateIt->second.respawnPending = false;
            stateIt->second.respawnTime = 0;
            deletePersistentRespawn = stateIt->second.persistent;

            if (auto respawnIt = mapFor(key.type).find(key.id); respawnIt != mapFor(key.type).end())
            {
                m_respawnTimes.remove(respawnIt->second);
                mapFor(key.type).erase(respawnIt);
            }
        }

        if (deletePersistentRespawn)
            deleteRespawnFromDB(key.type, key.id);

        return nullptr;
    }

    const bool wasRespawn = snapshot.respawnPending || snapshot.respawnTime != 0;

    /// Runtime objects may intentionally stay in WorldObjectRegistry while detached.
    /// Reattach that exact instance so script-held pointers remain valid.
    if (retainedGuid)
    {
        if (Object* retained = m_worldMap.getObject(WoWGuid(retainedGuid)))
        {
            if (retained->IsInWorld())
                return retained;

            /// Grid reloads and respawns always restore the object's home spawn position.
            if (key.type == SPAWN_TYPE_CREATURE)
            {
                if (auto const* row = std::get_if<MySQLStructure::CreatureSpawn>(&snapshot.templateData))
                    retained->SetPosition(row->spawnPoint);
            }
            else if (key.type == SPAWN_TYPE_GAMEOBJECT)
            {
                if (auto const* row = std::get_if<MySQLStructure::GameobjectSpawn>(&snapshot.templateData))
                    retained->SetPosition(row->spawnPoint);
            }

            if (wasRespawn && key.type == SPAWN_TYPE_GAMEOBJECT)
            {
                if (GameObject* go = retained->ToGameObject())
                    go->onRespawn();
            }

            m_objectFactory.attachToWorld(retained);
            if (!retained->IsInWorld())
                return nullptr;

            const int currentGrid = gridForPosition(retained->GetPosition());

            {
                std::unique_lock lk(m_mutex);
                auto stateIt = m_spawns.find(key);
                if (stateIt == m_spawns.end())
                    return nullptr;

                stateIt->second.respawnPending = false;
                stateIt->second.respawnTime = 0;
                indexCurrentNoLock(key, currentGrid);
            }

            if (wasRespawn && key.type == SPAWN_TYPE_CREATURE)
            {
                if (Creature* creature = retained->ToCreature())
                    creature->OnRespawn();
            }

            return retained;
        }

        /// Mapping survived but the registry no longer has the object: make it stale-free
        /// before creating a replacement.
        std::unique_lock lk(m_mutex);
        m_guidToSpawn.erase(retainedGuid);
        m_spawnToGuid.erase(key);
    }

    Object* created = nullptr;

    if (key.type == SPAWN_TYPE_CREATURE)
    {
        auto const* row = std::get_if<MySQLStructure::CreatureSpawn>(&snapshot.templateData);
        if (!row)
            return nullptr;

        Creature* creature = m_objectFactory.createCreatureFromSpawns(*row);
        if (!creature)
            return nullptr;

        creature->setLifecycleMap(&m_worldMap);
        creature->m_loadedFromDB = snapshot.persistent;
        creature->m_noRespawn = !snapshot.allowRespawn;
        created = creature;
    }
    else if (key.type == SPAWN_TYPE_GAMEOBJECT)
    {
        auto const* row = std::get_if<MySQLStructure::GameobjectSpawn>(&snapshot.templateData);
        if (!row)
            return nullptr;

        GameObject* go = m_objectFactory.createGameObjectFromSpawns(*row);
        if (!go)
            return nullptr;

        go->setLifecycleMap(&m_worldMap);
        go->m_loadedFromDB = snapshot.persistent;
        go->setNoRespawn(!snapshot.allowRespawn);

        if (snapshot.gameObjectOverrides)
        {
            go->setScale(snapshot.gameObjectOverrides->scale);
            go->SetFaction(snapshot.gameObjectOverrides->faction);
            go->setFlags(snapshot.gameObjectOverrides->flags);
        }

        if (snapshot.gameObjectOverridesMask != 0)
            go->SetOverrides(snapshot.gameObjectOverridesMask);

        if (wasRespawn)
            go->onRespawn();

        created = go;
    }

    if (!created)
        return nullptr;

    const uint64_t guidRaw = created->GetNewGUID().getRawGuid();

    {
        std::unique_lock lk(m_mutex);

        auto stateIt = m_spawns.find(key);
        if (stateIt == m_spawns.end())
        {
            m_objectFactory.recycleAndDestroy(created, true);
            return nullptr;
        }

        if (auto existingIt = m_spawnToGuid.find(key); existingIt != m_spawnToGuid.end())
        {
            /// Another path won the race. Never create a second instance for one SpawnKey.
            const uint64_t winnerGuid = existingIt->second;
            m_objectFactory.recycleAndDestroy(created, true);
            lk.unlock();

            return winnerGuid ? m_worldMap.getObject(WoWGuid(winnerGuid)) : nullptr;
        }

        m_guidToSpawn[guidRaw] = key;
        m_spawnToGuid[key] = guidRaw;
    }

    m_objectFactory.attachToWorld(created);
    if (!created->IsInWorld())
    {
        {
            std::unique_lock lk(m_mutex);
            m_guidToSpawn.erase(guidRaw);
            m_spawnToGuid.erase(key);
        }

        m_objectFactory.removeAndDestroy(created, true);
        return nullptr;
    }

    const int currentGrid = gridForPosition(created->GetPosition());

    {
        std::unique_lock lk(m_mutex);
        auto stateIt = m_spawns.find(key);
        if (stateIt == m_spawns.end())
            return nullptr;

        stateIt->second.respawnPending = false;
        stateIt->second.respawnTime = 0;
        indexCurrentNoLock(key, currentGrid);
    }

    if (wasRespawn && key.type == SPAWN_TYPE_CREATURE)
    {
        if (Creature* creature = created->ToCreature())
            creature->OnRespawn();
    }

    return created;
}

bool SpawnManager::deactivateSpawnInstance(const SpawnKey& key, bool retainInstance, bool preserveDesiredState)
{
    uint64_t guidRaw = 0;

    {
        std::shared_lock lk(m_mutex);
        auto guidIt = m_spawnToGuid.find(key);
        if (guidIt == m_spawnToGuid.end())
            return false;
        guidRaw = guidIt->second;
    }

    Object* object = m_worldMap.getObject(WoWGuid(guidRaw));
    if (!object)
    {
        std::unique_lock lk(m_mutex);
        m_spawnToGuid.erase(key);
        m_guidToSpawn.erase(guidRaw);
        return false;
    }

    const int currentGrid = object->IsInWorld() ? gridForPosition(object->GetPosition()) : -1;

    {
        std::unique_lock lk(m_mutex);
        auto stateIt = m_spawns.find(key);
        if (stateIt == m_spawns.end())
            return false;

        if (!preserveDesiredState)
            stateIt->second.desiredInWorld = false;
    }

    if (retainInstance && object->IsInWorld())
    {
        m_objectFactory.detachFromWorld(
            object,
            /*keepRegistry=*/true,
            /*soft=*/true);
    }

    {
        std::unique_lock lk(m_mutex);
        unindexCurrentNoLock(key, currentGrid);

        if (!retainInstance)
        {
            m_guidToSpawn.erase(guidRaw);
            m_spawnToGuid.erase(key);
        }
    }

    if (!retainInstance)
        m_objectFactory.removeAndDestroy(object, /*recycleGuid=*/true);

    return true;
}

bool SpawnManager::despawn(Object* object, uint32_t respawnDelayMs, bool keepRespawnSchedule)
{
    return object ? despawn(object->getGuid(), respawnDelayMs, keepRespawnSchedule) : false;
}

bool SpawnManager::despawn(uint64_t guidRaw, uint32_t respawnDelayMs, bool keepRespawnSchedule)
{
    if (respawnDelayMs == 0)
        return despawnAt(guidRaw, 0, keepRespawnSchedule);

    const time_t delaySeconds =
        static_cast<time_t>((static_cast<uint64_t>(respawnDelayMs) + 999ULL) / 1000ULL);

    return despawnAt(guidRaw, std::time(nullptr) + delaySeconds, keepRespawnSchedule);
}

bool SpawnManager::despawnAt(Object* object, time_t respawnAt, bool keepRespawnSchedule)
{
    return object ? despawnAt(object->getGuid(), respawnAt, keepRespawnSchedule) : false;
}

bool SpawnManager::despawnAt(uint64_t guidRaw, time_t respawnAt, bool keepRespawnSchedule)
{
    Object* currentObject = m_worldMap.getObject(WoWGuid(guidRaw));

    SpawnKey key{};
    bool managed = false;
    bool allowRespawn = false;
    bool persistent = false;
    uint32_t entry = 0;

    {
        std::unique_lock lk(m_mutex);

        auto guidIt = m_guidToSpawn.find(guidRaw);
        if (guidIt != m_guidToSpawn.end())
        {
            key = guidIt->second;
            auto stateIt = m_spawns.find(key);
            if (stateIt == m_spawns.end())
                return false;

            managed = true;
            allowRespawn = stateIt->second.allowRespawn;
            persistent = stateIt->second.persistent;
            entry = stateIt->second.entry;

            if (respawnAt > 0 && allowRespawn)
            {
                stateIt->second.desiredInWorld = true;
                stateIt->second.respawnPending = true;
                stateIt->second.respawnTime = respawnAt;
            }
            else if (!keepRespawnSchedule)
            {
                stateIt->second.desiredInWorld = false;
                stateIt->second.respawnPending = false;
                stateIt->second.respawnTime = 0;
            }
        }
    }

    if (!managed)
    {
        if (!currentObject)
            return false;

        m_objectFactory.removeAndDestroy(currentObject, true);
        return true;
    }

    /// DB-backed spawns are recreated from their stored template. Runtime respawnable
    /// objects are retained in WorldObjectRegistry so existing script pointers remain valid.
    const bool retainInstance = !persistent && allowRespawn && respawnAt > 0;
    deactivateSpawnInstance(key, retainInstance, /*preserveDesiredState=*/true);

    if (respawnAt > 0 && allowRespawn)
    {
        scheduleRespawn(key.type, key.id, entry, respawnAt);
    }
    else if (!keepRespawnSchedule)
    {
        cancelRespawn(key.type, key.id);

        if (!persistent)
        {
            std::unique_lock lk(m_mutex);
            auto stateIt = m_spawns.find(key);
            if (stateIt != m_spawns.end())
            {
                unindexHomeNoLock(key, stateIt->second.homeGrid);
                m_spawns.erase(stateIt);
            }
        }
    }

    return true;
}

void SpawnManager::scheduleRespawn(SpawnObjectType type, uint32_t spawnId, uint32_t entry, time_t when)
{
    if (!spawnId || !when)
        return;

    bool persistent = false;

    {
        std::unique_lock lk(m_mutex);

        RespawnMap& mp = mapFor(type);
        if (auto it = mp.find(spawnId); it != mp.end())
        {
            RespawnInfo const* existing = it->second;
            if (when >= existing->time)
            {
                if (auto stateIt = m_spawns.find({ type, spawnId }); stateIt != m_spawns.end())
                {
                    stateIt->second.respawnPending = true;
                    stateIt->second.respawnTime = existing->time;
                }
                return;
            }

            m_respawnTimes.remove(existing);
            mp.erase(it);
        }

        auto ri = std::make_unique<RespawnInfo>();
        ri->type = type;
        ri->spawnId = spawnId;
        ri->entry = entry;
        ri->time = when;

        RespawnInfo* ptr = ri.get();
        mp[spawnId] = ptr;
        m_respawnTimes.emplace(std::move(ri));

        if (auto stateIt = m_spawns.find({ type, spawnId }); stateIt != m_spawns.end())
        {
            stateIt->second.respawnPending = true;
            stateIt->second.respawnTime = when;
            stateIt->second.desiredInWorld = true;
            persistent = stateIt->second.persistent;
        }
    }

    if (persistent)
        saveRespawnDB(type, spawnId, when);
}

void SpawnManager::cancelRespawn(SpawnObjectType type, uint32_t spawnId)
{
    bool persistent = false;

    {
        std::unique_lock lk(m_mutex);

        RespawnMap& mp = mapFor(type);
        if (auto it = mp.find(spawnId); it != mp.end())
        {
            m_respawnTimes.remove(it->second);
            mp.erase(it);
        }

        if (auto stateIt = m_spawns.find({ type, spawnId }); stateIt != m_spawns.end())
        {
            stateIt->second.respawnPending = false;
            stateIt->second.respawnTime = 0;
            persistent = stateIt->second.persistent;
        }
    }

    if (persistent)
        deleteRespawnFromDB(type, spawnId);
}

bool SpawnManager::hasRespawnScheduledNoLock(SpawnObjectType type, uint32_t spawnId) const
{
    return mapFor(type).find(spawnId) != mapFor(type).end();
}

bool SpawnManager::hasRespawnScheduled(SpawnObjectType type, uint32_t spawnId) const
{
    std::shared_lock lk(m_mutex);
    return hasRespawnScheduledNoLock(type, spawnId);
}

void SpawnManager::saveRespawnDB(SpawnObjectType type, uint32_t spawnId, time_t when)
{
    CharacterDatabase.execute(
        "REPLACE INTO respawn (type, spawnId, respawnTime, mapId, instanceId) "
        "VALUES (%u, %u, %u, %u, %u)",
        static_cast<uint32_t>(type), spawnId, static_cast<uint64_t>(when),
        m_worldMap.getBaseMap()->getMapId(), m_worldMap.getInstanceId());
}

void SpawnManager::deleteRespawnFromDB(SpawnObjectType type, uint32_t spawnId)
{
    CharacterDatabase.execute(
        "DELETE FROM respawn WHERE type=%u AND spawnId=%u AND mapId=%u AND instanceId=%u",
        static_cast<uint32_t>(type), spawnId,
        m_worldMap.getBaseMap()->getMapId(), m_worldMap.getInstanceId());
}

void SpawnManager::deleteRespawnTimesInDB(uint32_t mapId, uint32_t instanceId)
{
    CharacterDatabase.execute(
        "DELETE FROM respawn WHERE mapId = %u AND instanceId = %u",
        mapId, instanceId);
}

bool SpawnManager::eraseGameObjectSpawnBySpawnID(uint32_t spawnID)
{
    const SpawnKey key{ SPAWN_TYPE_GAMEOBJECT, spawnID };

    cancelRespawn(key.type, key.id);

    std::unique_lock lk(m_mutex);
    auto stateIt = m_spawns.find(key);
    if (stateIt == m_spawns.end())
        return false;

    unindexHomeNoLock(key, stateIt->second.homeGrid);

    for (auto it = m_currentGridIndex.begin(); it != m_currentGridIndex.end(); )
    {
        it->second.erase(key);
        if (it->second.empty())
            it = m_currentGridIndex.erase(it);
        else
            ++it;
    }

    if (auto guidIt = m_spawnToGuid.find(key); guidIt != m_spawnToGuid.end())
    {
        m_guidToSpawn.erase(guidIt->second);
        m_spawnToGuid.erase(guidIt);
    }

    m_spawns.erase(stateIt);
    return true;
}

bool SpawnManager::eraseCreatureSpawnBySpawnID(uint32_t spawnID)
{
    const SpawnKey key{ SPAWN_TYPE_CREATURE, spawnID };

    cancelRespawn(key.type, key.id);

    std::unique_lock lk(m_mutex);
    auto stateIt = m_spawns.find(key);
    if (stateIt == m_spawns.end())
        return false;

    unindexHomeNoLock(key, stateIt->second.homeGrid);

    for (auto it = m_currentGridIndex.begin(); it != m_currentGridIndex.end(); )
    {
        it->second.erase(key);
        if (it->second.empty())
            it = m_currentGridIndex.erase(it);
        else
            ++it;
    }

    if (auto guidIt = m_spawnToGuid.find(key); guidIt != m_spawnToGuid.end())
    {
        m_guidToSpawn.erase(guidIt->second);
        m_spawnToGuid.erase(guidIt);
    }

    m_spawns.erase(stateIt);
    return true;
}

SpawnManagerSnapshot SpawnManager::snapshot() const
{
    SpawnManagerSnapshot out;

    {
        std::shared_lock lk(m_mutex);
        out.states = m_spawns.size();
        out.instances = m_spawnToGuid.size();
        out.guidIndex = m_guidToSpawn.size();
        out.moveGuidIndex = m_spawnToGuid.size();
        out.homeGrids = m_gridIndex.size();
        out.currentGrids = m_currentGridIndex.size();
        out.activeGrids = m_activeGrids.size();
        out.unloadingGrids = m_unloadingGrids.size();
        out.creatureRespawns = m_creatureRespawns.size();
        out.gameObjectRespawns = m_gameObjectRespawns.size();
        out.respawnQueue = m_respawnTimes.size();

        std::unordered_set<SpawnKey, SpawnKeyHash> inWorldKeys;
        for (auto const& [gid, keys] : m_currentGridIndex)
            inWorldKeys.insert(keys.begin(), keys.end());
        out.inWorld = inWorldKeys.size();

        for (auto const& [key, state] : m_spawns)
        {
            if (state.desiredInWorld)
                ++out.desiredInWorld;
            if (state.persistent)
                ++out.persistent;
            else
                ++out.ephemeral;
            if (state.respawnPending)
                ++out.pendingRespawn;
        }
    }

    {
        std::lock_guard lk(m_pendingAddsMutex);
        out.pendingAdds = m_pendingAdds.size();
    }

    return out;
}


std::vector<int> SpawnManager::definedGridIds() const
{
    std::vector<int> grids;
    std::shared_lock lk(m_mutex);
    grids.reserve(m_gridIndex.size());
    for (auto const& [gid, _] : m_gridIndex)
        grids.push_back(gid);
    return grids;
}

SpawnManagerGridSnapshot SpawnManager::gridSnapshot(int gid) const
{
    SpawnManagerGridSnapshot out;
    out.gid = gid;

    std::shared_lock lk(m_mutex);
    out.active = isGridActive(gid);
    out.unloading = m_unloadingGrids.count(gid) != 0;

    if (auto it = m_gridIndex.find(gid); it != m_gridIndex.end())
    {
        out.homeStates = it->second.size();

        for (auto const& key : it->second)
        {
            auto stateIt = m_spawns.find(key);
            if (stateIt == m_spawns.end())
                continue;

            auto const& state = stateIt->second;
            if (m_spawnToGuid.find(key) != m_spawnToGuid.end())
                ++out.homeInstances;

            bool inWorld = false;
            for (auto const& [currentGid, currentKeys] : m_currentGridIndex)
            {
                if (currentKeys.find(key) != currentKeys.end())
                {
                    inWorld = true;
                    break;
                }
            }

            if (inWorld)
                ++out.homeInWorld;
            if (state.desiredInWorld)
                ++out.homeDesiredInWorld;
            if (state.persistent)
                ++out.homePersistent;
            else
                ++out.homeEphemeral;
            if (state.respawnPending)
                ++out.homePendingRespawn;
        }
    }

    if (auto it = m_currentGridIndex.find(gid); it != m_currentGridIndex.end())
        out.currentStates = it->second.size();

    return out;
}

bool SpawnManager::isGridActiveForDebug(int gid) const
{
    std::shared_lock lk(m_mutex);
    return isGridActive(gid);
}

void SpawnManager::dumpInconsistencies() const
{
    std::vector<std::pair<uint64_t, SpawnKey>> guidSnapshot;
    size_t spawnCount = 0;
    size_t guidCount = 0;
    size_t reverseCount = 0;
    size_t missingStateForGuid = 0;
    size_t missingGuidForState = 0;

    {
        std::shared_lock lk(m_mutex);

        spawnCount = m_spawns.size();
        guidCount = m_guidToSpawn.size();
        reverseCount = m_spawnToGuid.size();
        guidSnapshot.reserve(m_guidToSpawn.size());

        for (auto const& [guid, key] : m_guidToSpawn)
        {
            if (m_spawns.find(key) == m_spawns.end())
                ++missingStateForGuid;

            auto reverse = m_spawnToGuid.find(key);
            if (reverse == m_spawnToGuid.end() || reverse->second != guid)
                ++missingGuidForState;

            guidSnapshot.emplace_back(guid, key);
        }
    }

    /// Never call into WorldObjectRegistry while holding SpawnManager::m_mutex.
    size_t staleLiveGuid = 0;
    for (auto const& [guid, key] : guidSnapshot)
    {
        (void)key;
        if (!m_worldMap.getObject(WoWGuid(guid)))
            ++staleLiveGuid;
    }

    sLogger.warning(
        "SM audit: spawns={} guidToSpawn={} spawnToGuid={} missingStateForGuid={} missingGuidForState={} staleRegistryGuid={}",
        spawnCount, guidCount, reverseCount, missingStateForGuid, missingGuidForState, staleLiveGuid);
}
