/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <functional>
#include "Utilities/LocationVector.hpp"
#include "WoWGuid.hpp"

class GuidAllocator;
class WorldMap;
class Object;
class Creature;
class GameObject;
class Transporter;
class DynamicObject;
class Player;
class Pet;
class Corpse;

struct QuaternionData;

namespace MySQLStructure
{
    struct CreatureSpawn;
    struct GameobjectSpawn;
}

namespace world
{
    class WorldObjectRegistry;
}

namespace visibility
{
    class VisibilitySystem;
    struct ObjectMeta;
    enum class Container : uint8_t;
}

class ObjectFactory
{
public:
    ObjectFactory(WorldMap& map, visibility::VisibilitySystem& visibilitySystem, world::WorldObjectRegistry& reg, GuidAllocator& guids);

    void attachToWorld(Object* obj);
    void detachFromWorld(Object* obj, bool keepRegistry = false, bool soft = false);
    void transferWorld(Object* obj, WorldMap* newMap);
    void finishTransferWorld(Object* obj, const LocationVector& transferPosition);

    /// Remove an attached object and destroy it after the map is done with it.
    void removeAndDestroy(Object* obj, bool recycleGuid = true);

    /// Destroy an object that is already detached.
    void recycleAndDestroy(Object* obj, bool recycleGuid = true);

    Creature* createCreature(uint32_t entry);
    Creature* createCreature(uint32_t entry, const LocationVector& pos);
    Creature* createCreatureFromSpawns(const MySQLStructure::CreatureSpawn& row);
    Creature* createAndSpawnCreature(uint32_t entry, const LocationVector& pos);
    Creature* createAndSpawnCreatureFromSpawns(const MySQLStructure::CreatureSpawn& row);

    GameObject* createGameObject(uint32_t entry);
    GameObject* createGameObject(uint32_t entry, const LocationVector& pos, const QuaternionData& rotation = QuaternionData {});
    GameObject* createGameObjectFromSpawns(const MySQLStructure::GameobjectSpawn& row);
    GameObject* createAndSpawnGameObject(uint32_t entry, const LocationVector& pos, const QuaternionData& rotation = QuaternionData {});
    GameObject* createAndSpawnGameObjectFromSpawns(const MySQLStructure::GameobjectSpawn& row);

    Transporter* createTransporter(uint32_t entry, uint32_t mapId, const LocationVector& pos);
    Transporter* createAndSpawnTransporter(uint32_t entry, uint32_t mapId, const LocationVector& pos);

    DynamicObject* createDynamic();

    Corpse* createCorpse(uint64_t rawGuid = 0);

    uint64_t generateCreatureGuid(uint32_t entry, bool reuse = true) const;
    uint64_t generateGameObjectGuid(uint32_t entry, bool reuse = true) const;
    uint64_t generateDynamicGuid(bool reuse = true) const;
    uint64_t generateCorpseGuid(bool reuse = true) const;

private:

    visibility::ObjectMeta makeMeta(Object* obj, const WoWGuid& g) const;

    WorldMap& m_worldMap;
    visibility::VisibilitySystem& m_visibilitySystem;
    world::WorldObjectRegistry& m_objectRegistry;
    GuidAllocator& m_guidAllocator;
};

