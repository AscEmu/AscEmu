/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Platform/SymbolVisibility.hpp"

#include <cstdint>
#include <list>

namespace MySQLStructure
{
    struct CreatureSpawn;
    struct GameobjectSpawn;
}

namespace visibility { class SpatialIndex; }
namespace world { class WorldObjectRegistry; }
class ObjectFactory;
class SpawnManager;

class LocationVector;
class Object;
class WorldMap;
class GameObject;
class Creature;
class Unit;
class Player;

//////////////////////////////////////////////////////////////////////////////////////////
/// Class MapScriptInterface
/// Provides an interface to WorldMap for scripts, to obtain objects, get players, etc.
//////////////////////////////////////////////////////////////////////////////////////////
class SERVER_DECL MapScriptInterface
{
public:
    MapScriptInterface(WorldMap& map, visibility::SpatialIndex& spatialIndex, world::WorldObjectRegistry& reg, ObjectFactory& factory, SpawnManager& manager);
    ~MapScriptInterface();

    GameObject* getGameObjectNearestCoords(float x, float y, float z = 0.0f, uint32_t entry = 0);
    Creature* getCreatureNearestCoords(float x, float y, float z = 0.0f, uint32_t entry = 0);

    GameObject* findNearestGoWithType(Object* o, uint32_t type);
    Creature* findNearestCreature(Object* obj, uint32_t entry, float maxRange = 250.0f) const;
    GameObject* findNearestGameObject(Object* obj, uint32_t entry, float maxRange = 250.0f) const;

    void getCreatureListWithEntryInRange(Creature* center, std::list<Creature*>& out, uint32_t entry, float maxRange = 250.0f) const;
    void getGameObjectListWithEntryInRange(Creature* center, std::list<GameObject*>& out, uint32_t entry, float maxRange = 250.0f) const;

    Player* getPlayerNearestCoords(float x, float y, float z = 0.0f);
    uint32_t  getPlayerCountInRadius(float x, float y, float z = 0.0f, float radius = 5.0f);

    GameObject* spawnGameObject(uint32_t entry, LocationVector pos, uint32_t phase = 0xFFFFFFF);
    GameObject* spawnGameObject(MySQLStructure::GameobjectSpawn* gs);
    Creature* spawnCreature(uint32_t entry, LocationVector pos, uint32_t phase = 0xFFFFFFF);
    Creature* spawnCreature(MySQLStructure::CreatureSpawn* sp);

    void deleteGameObject(GameObject* ptr);
    void deleteCreature(Creature* ptr);

private:
    WorldMap& m_worldMap;
    visibility::SpatialIndex& m_spatialIndex;
    world::WorldObjectRegistry& m_registry;
    ObjectFactory& m_factory;
    SpawnManager& m_spawnManager;
};
