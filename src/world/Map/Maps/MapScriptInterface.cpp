/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Storage/MySQLDataStore.hpp"
#include "MapScriptInterface.h"

MapScriptInterface::MapScriptInterface(WorldMap& mgr) : m_worldMap(mgr)
{}

MapScriptInterface::~MapScriptInterface()
{
    m_worldMap.ScriptInterface = nullptr;
}

uint32_t MapScriptInterface::getPlayerCountInRadius(float x, float y, float z /* = 0.0f */, float radius /* = 5.0f */)
{
    // use a cell radius of 2
    uint32_t PlayerCount = 0;
    uint32_t cellX = m_worldMap.getPosX(x);
    uint32_t cellY = m_worldMap.getPosY(y);

    uint32_t endX = cellX < Map::Cell::_sizeX ? cellX + 1 : Map::Cell::_sizeX;
    uint32_t endY = cellY < Map::Cell::_sizeY ? cellY + 1 : Map::Cell::_sizeY;
    uint32_t startX = cellX > 0 ? cellX - 1 : 0;
    uint32_t startY = cellY > 0 ? cellY - 1 : 0;
    MapCell* pCell;
    ObjectSet::iterator iter, iter_end;

    for (uint32_t cx = startX; cx < endX; ++cx)
    {
        for (uint32_t cy = startY; cy < endY; ++cy)
        {
            pCell = m_worldMap.getCell(cx, cy);
            if (pCell == nullptr || pCell->getPlayerCount() == 0)
                continue;

            iter = pCell->Begin();
            iter_end = pCell->End();

            for (; iter != iter_end; ++iter)
            {
                if ((*iter)->isPlayer() &&
                    (*iter)->CalcDistance(x, y, (z == 0.0f ? (*iter)->GetPositionZ() : z)) < radius)
                {
                    ++PlayerCount;
                }
            }
        }
    }

    return PlayerCount;
}

GameObject* MapScriptInterface::spawnGameObject(uint32_t Entry, LocationVector pos, bool AddToWorld, uint32_t /*Misc1*/, uint32_t /*Misc2*/, uint32_t phase)
{

    GameObject* pGameObject = m_worldMap.createGameObject(Entry);
    if (!pGameObject->create(Entry, &m_worldMap, phase, pos, QuaternionData(), GO_STATE_CLOSED))
    {
        delete pGameObject;
        return nullptr;
    }
    pGameObject->m_phase = phase;
    pGameObject->m_spawn = nullptr;

    if (AddToWorld)
        pGameObject->PushToWorld(&m_worldMap);

    return pGameObject;
}

GameObject* MapScriptInterface::spawnGameObject(uint32_t spawnId, bool AddToWorld)
{
    if (!spawnId)
        return nullptr;

    MySQLStructure::GameobjectSpawn const* data = sMySQLStore.getGameObjectSpawn(spawnId);

    GameObject* pGameObject = m_worldMap.createGameObject(data->entry);
    if (!pGameObject->loadFromDB(spawnId, &m_worldMap, false))
    {
        delete pGameObject;
        return nullptr;
    }

    if (AddToWorld)
        pGameObject->PushToWorld(&m_worldMap);

    return pGameObject;
}

// Zyres 11/06/2017 - bool tmplate not used!
Creature* MapScriptInterface::spawnCreature(uint32_t Entry, LocationVector pos, bool AddToWorld, bool /*tmplate*/, uint32_t /*Misc1*/, uint32_t /*Misc2*/, uint32_t phase)
{
    CreatureProperties const* creature_properties = sMySQLStore.getCreatureProperties(Entry);
    if (creature_properties == nullptr)
        return nullptr;

    MySQLStructure::CreatureSpawn* spawn = new MySQLStructure::CreatureSpawn;
    spawn->entry = Entry;
    uint32_t DisplayID = 0;
    uint8_t Gender = creature_properties->generateRandomDisplayIdAndReturnGender(&DisplayID);
    spawn->displayid = DisplayID;
    spawn->id = 0;
    spawn->movetype = 0;
    spawn->x = pos.x;
    spawn->y = pos.y;
    spawn->z = pos.z;
    spawn->o = pos.o;
    spawn->emote_state = 0;
    spawn->flags = 0;
    spawn->factionid = creature_properties->Faction;
    spawn->bytes0 = 0;
    spawn->bytes1 = 0;
    spawn->bytes2 = 0;
    spawn->stand_state = 0;
    spawn->death_state = 0;
    spawn->channel_target_creature = 0;
    spawn->channel_target_go = 0;
    spawn->channel_spell = 0;
    spawn->MountedDisplayID = 0;

    spawn->Item1SlotEntry = creature_properties->itemslot_1;
    spawn->Item2SlotEntry = creature_properties->itemslot_2;
    spawn->Item3SlotEntry = creature_properties->itemslot_3;

    spawn->Item1SlotDisplay = sMySQLStore.getItemDisplayIdForEntry(spawn->Item1SlotEntry);
    spawn->Item2SlotDisplay = sMySQLStore.getItemDisplayIdForEntry(spawn->Item2SlotEntry);
    spawn->Item3SlotDisplay = sMySQLStore.getItemDisplayIdForEntry(spawn->Item3SlotEntry);

    spawn->CanFly = 0;
    spawn->phase = phase;

    Creature* creature = this->m_worldMap.createCreature(Entry);
    if (creature)
    {
        creature->Load(spawn, 0, nullptr);
        creature->setGender(Gender);
        creature->spawnid = 0;
        creature->m_spawn = nullptr;

        delete spawn;

        if (AddToWorld)
            creature->PushToWorld(&m_worldMap);

        return creature;
    }

    sLogger.failure("MapScriptInterface::SpawnCreature tried to spawn invalid creature %u (nullptr), returning nullptr!", Entry);
    return nullptr;
}

Creature* MapScriptInterface::spawnCreature(MySQLStructure::CreatureSpawn* sp, bool AddToWorld)
{
    if (!sp)
        return nullptr;

    CreatureProperties const* creature_properties = sMySQLStore.getCreatureProperties(sp->entry);
    if (creature_properties == nullptr)
    {
        return nullptr;
    }

    uint8_t Gender = creature_properties->generateRandomDisplayIdAndReturnGender(&sp->displayid);
    Creature* p = this->m_worldMap.createCreature(sp->entry);
    if (p)
    {
        p->Load(sp, 0, nullptr);
        p->setGender(Gender);
        p->spawnid = 0;
        p->m_spawn = nullptr;
        if (AddToWorld)
            p->PushToWorld(&m_worldMap);
        return p;
    }

    sLogger.failure("MapScriptInterface::SpawnCreature tried to spawn invalid creature %u (nullptr), returning nullptr!", sp->entry);
    return nullptr;
}

void MapScriptInterface::deleteCreature(Creature* ptr)
{
    delete ptr;
}

void MapScriptInterface::deleteGameObject(GameObject* ptr)
{
    delete ptr;
}
