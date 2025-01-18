/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#include <cstdint>
#include <list>

namespace MySQLStructure
{
    struct CreatureSpawn;
    struct GameobjectSpawn;
}

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
    MapScriptInterface(WorldMap& mgr);
    ~MapScriptInterface();

    GameObject* getGameObjectNearestCoords(float x, float y, float z = 0.0f, uint32_t Entry = 0);
    Creature* getCreatureNearestCoords(float x, float y, float z = 0.0f, uint32_t Entry = 0);

    GameObject* findNearestGoWithType(Object* o, uint32_t type);
    Creature* findNearestCreature(Object* pObject, uint32_t entry, float maxSearchRange /*= 250.0f*/) const;

    void getCreatureListWithEntryInRange(Creature* pCreature, std::list<Creature*>& container, uint32_t entry, float maxSearchRange /*= 250.0f*/) const;

    Creature* getNearestAssistCreatureInCell(Creature* pCreature, Unit* enemy, float range /*= 250.0f*/) const;

    void getGameObjectListWithEntryInRange(Creature* pCreature, std::list<GameObject*>& container, uint32_t entry, float maxSearchRange /*= 250.0f*/) const;

    GameObject* findNearestGameObject(Object* pObject, uint32_t entry, float maxSearchRange /*= 250.0f*/) const;

    Player* getPlayerNearestCoords(float x, float y, float z = 0.0f, uint32_t Entry = 0);
    uint32_t getPlayerCountInRadius(float x, float y, float z = 0.0f, float radius = 5.0f);

    GameObject* spawnGameObject(uint32_t Entry, LocationVector pos, bool AddToWorld, uint32_t Misc1, uint32_t Misc2, uint32_t phase = 0xFFFFFFF);
    GameObject* spawnGameObject(MySQLStructure::GameobjectSpawn* gs, bool AddToWorld);
    Creature* spawnCreature(uint32_t Entry, LocationVector pos, bool AddToWorld, bool tmplate, uint32_t Misc1, uint32_t Misc2, uint32_t phase = 0xFFFFFFF);
    Creature* spawnCreature(MySQLStructure::CreatureSpawn* sp, bool AddToWorld);

    void deleteGameObject(GameObject* ptr);
    void deleteCreature(Creature* ptr);

private:
    WorldMap& m_worldMap;
};
