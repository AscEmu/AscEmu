/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MapScriptInterface.h"

#include "WorldMap.hpp"
#include "Map/Management/SpawnManager.hpp"
#include "Map/Management/ObjectFactory.hpp"
#include "Map/Visibility/SpatialIndex.hpp"
#include "Map/Management/WorldObjectRegistry.hpp"

#include "Objects/Object.hpp"
#include "Objects/GameObject.h"
#include "Objects/DynamicObject.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Storage/MySQLDataStore.hpp"
#include <limits>
#include <cmath>

MapScriptInterface::MapScriptInterface(WorldMap& map, visibility::SpatialIndex& spatialIndex, world::WorldObjectRegistry& reg, ObjectFactory& factory, SpawnManager& manager)
    : m_worldMap(map), m_spatialIndex(spatialIndex), m_registry(reg), m_factory(factory), m_spawnManager(manager)
{
}

MapScriptInterface::~MapScriptInterface() = default;

// nearest by coords
GameObject* MapScriptInterface::getGameObjectNearestCoords(float x, float y, float z, uint32_t entry)
{
    return m_spatialIndex.findNearestByEntry<GameObject>(LocationVector{ x,y,z,0.f }, entry, /*maxRange=*/250.f);
}

Creature* MapScriptInterface::getCreatureNearestCoords(float x, float y, float z, uint32_t entry)
{
    return m_spatialIndex.findNearestByEntry<Creature>(LocationVector{ x,y,z,0.f }, entry, /*maxRange=*/250.f);
}

// nearest by object
Creature* MapScriptInterface::findNearestCreature(Object* center, uint32_t entry, float maxSearchRange) const
{
    if (!center || maxSearchRange <= 0.f) return nullptr;
    return m_spatialIndex.findNearestByEntry<Creature>(center->GetPosition(), entry, maxSearchRange);
}

GameObject* MapScriptInterface::findNearestGameObject(Object* center, uint32_t entry, float maxSearchRange) const
{
    if (!center || maxSearchRange <= 0.f) return nullptr;
    return m_spatialIndex.findNearestByEntry<GameObject>(center->GetPosition(), entry, maxSearchRange);
}

// lists in range
void MapScriptInterface::getCreatureListWithEntryInRange(Creature* center, std::list<Creature*>& out, uint32_t entry, float maxRange) const
{
    out.clear();
    if (!center || maxRange <= 0.f) return;
    m_spatialIndex.collectByEntryInRange<Creature>(center->GetPosition(), out, entry, maxRange);
}

void MapScriptInterface::getGameObjectListWithEntryInRange(Creature* center, std::list<GameObject*>& out, uint32_t entry, float maxRange) const
{
    out.clear();
    if (!center || maxRange <= 0.f) return;
    m_spatialIndex.collectByEntryInRange<GameObject>(center->GetPosition(), out, entry, maxRange);
}

GameObject* MapScriptInterface::findNearestGoWithType(Object* o, uint32_t type)
{
    if (!o) return nullptr;

    thread_local std::vector<WoWGuid> ids;
    ids.clear(); ids.reserve(64);

    m_spatialIndex.collectGuidsInCellsAroundPos<GameObject>(o->GetPosition(), /*Rcells=*/2, 0, ids);

    GameObject* best = nullptr;
    float bestD2 = std::numeric_limits<float>::infinity();
    const uint32_t phase = o->GetPhase();

    for (const WoWGuid& g : ids)
    {
        GameObject* go = m_registry.getGameObject(g);
        if (!go || go->getWorldMap() != &m_worldMap) continue;
        if (go->getGoType() != type)         continue;
        if ((go->GetPhase() & phase) == 0)   continue;

        const float d2 = o->getDistanceSq(go);
        if (d2 < bestD2) { bestD2 = d2; best = go; }
    }
    return best;
}

Player* MapScriptInterface::getPlayerNearestCoords(float x, float y, float z)
{
    thread_local std::vector<WoWGuid> ids;
    ids.clear(); ids.reserve(64);
    m_spatialIndex.collectGuidsInCellsAroundPos<Player>(LocationVector{ x,y,z,0.f }, /*Rcells=*/4, 0, ids);

    Player* best = nullptr;
    float bestD2 = std::numeric_limits<float>::infinity();
    for (const WoWGuid& id : ids) {
        Player* p = m_registry.getPlayer(id);
        if (!p || p->getWorldMap() != &m_worldMap) continue;
        const float dx = p->GetPositionX() - x, dy = p->GetPositionY() - y;
        const float d2 = dx * dx + dy * dy;
        if (d2 < bestD2) { bestD2 = d2; best = p; }
    }
    return best;
}

uint32_t MapScriptInterface::getPlayerCountInRadius(float x, float y, float z, float radius)
{
    if (radius <= 0.f) return 0;

    thread_local std::vector<WoWGuid> ids;
    ids.clear(); ids.reserve(64);
    m_spatialIndex.collectGuidsInCellsAroundPos<Player>(LocationVector{ x,y,z,0.f }, /*Rcells=*/4, 0, ids);

    const float r2 = radius * radius;
    uint32_t count = 0;
    for (const WoWGuid& id : ids) {
        Player* p = m_registry.getPlayer(id);
        if (!p || p->getWorldMap() != &m_worldMap) continue;
        const float dx = p->GetPositionX() - x, dy = p->GetPositionY() - y;
        if (dx * dx + dy * dy < r2) ++count;
    }
    return count;
}

// ---------- Spawns/Deletes via Factory ----------
GameObject* MapScriptInterface::spawnGameObject(uint32_t entry, LocationVector pos, uint32_t phase)
{
    GameObject* go = m_spawnManager.createGameObject(entry, pos);
    if (!go)
        return nullptr;

    go->m_phase = phase;
    return m_spawnManager.pushToWorld(go) ? go : nullptr;
}

GameObject* MapScriptInterface::spawnGameObject(MySQLStructure::GameobjectSpawn* gs)
{
    if (!gs)
        return nullptr;

    GameObject* go = m_spawnManager.spawnGameObject(0, {}, {}, gs);
    if (!go)
        return nullptr;

    return go;
}

Creature* MapScriptInterface::spawnCreature(uint32_t entry, LocationVector pos, uint32_t phase)
{
    Creature* creature = m_spawnManager.createCreature(entry, pos);
    if (!creature)
        return nullptr;

    creature->setPhase(PHASE_SET, phase);
    return m_spawnManager.pushToWorld(creature) ? creature : nullptr;
}

Creature* MapScriptInterface::spawnCreature(MySQLStructure::CreatureSpawn* sp)
{
    if (!sp)
        return nullptr;

    Creature* creature = m_spawnManager.spawnCreature(0, {}, sp);
    if (!creature)
        return nullptr;

    return creature;
}

void MapScriptInterface::deleteCreature(Creature* ptr)
{
    if (!ptr)
        return;

    m_spawnManager.despawn(ptr);
}

void MapScriptInterface::deleteGameObject(GameObject* ptr)
{
    if (!ptr)
        return;

    m_spawnManager.despawn(ptr);
}
