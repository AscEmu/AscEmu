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

uint32 MapScriptInterface::GetPlayerCountInRadius(float x, float y, float z /* = 0.0f */, float radius /* = 5.0f */)
{
    // use a cell radius of 2
    uint32 PlayerCount = 0;
    uint32 cellX = m_worldMap.getPosX(x);
    uint32 cellY = m_worldMap.getPosY(y);

    uint32 endX = cellX < Map::Cell::_sizeX ? cellX + 1 : Map::Cell::_sizeX;
    uint32 endY = cellY < Map::Cell::_sizeY ? cellY + 1 : Map::Cell::_sizeY;
    uint32 startX = cellX > 0 ? cellX - 1 : 0;
    uint32 startY = cellY > 0 ? cellY - 1 : 0;
    MapCell* pCell;
    ObjectSet::iterator iter, iter_end;

    for (uint32 cx = startX; cx < endX; ++cx)
    {
        for (uint32 cy = startY; cy < endY; ++cy)
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

GameObject* MapScriptInterface::SpawnGameObject(uint32 Entry, float cX, float cY, float cZ, float cO, bool AddToWorld, uint32 /*Misc1*/, uint32 /*Misc2*/, uint32 phase)
{

    GameObject* pGameObject = m_worldMap.createGameObject(Entry);
    if (!pGameObject->CreateFromProto(Entry, m_worldMap.getBaseMap()->getMapId(), cX, cY, cZ, cO))
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

GameObject* MapScriptInterface::SpawnGameObject(MySQLStructure::GameobjectSpawn* gs, bool AddToWorld)
{
    if (!gs)
        return nullptr;

    GameObject* pGameObject = m_worldMap.createGameObject(gs->entry);
    if (!pGameObject->Load(gs))
    {
        delete pGameObject;
        return nullptr;
    }

    if (AddToWorld)
        pGameObject->PushToWorld(&m_worldMap);

    return pGameObject;
}

// Zyres 11/06/2017 - bool tmplate not used!
Creature* MapScriptInterface::SpawnCreature(uint32 Entry, float cX, float cY, float cZ, float cO, bool AddToWorld, bool /*tmplate*/, uint32 /*Misc1*/, uint32 /*Misc2*/, uint32 phase)
{
    CreatureProperties const* creature_properties = sMySQLStore.getCreatureProperties(Entry);
    if (creature_properties == nullptr)
        return nullptr;

    MySQLStructure::CreatureSpawn* spawn = new MySQLStructure::CreatureSpawn;
    spawn->entry = Entry;
    uint32 DisplayID = 0;
    uint8 Gender = creature_properties->GetGenderAndCreateRandomDisplayID(&DisplayID);
    spawn->displayid = DisplayID;
    spawn->id = 0;
    spawn->movetype = 0;
    spawn->x = cX;
    spawn->y = cY;
    spawn->z = cZ;
    spawn->o = cO;
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

Creature* MapScriptInterface::SpawnCreature(MySQLStructure::CreatureSpawn* sp, bool AddToWorld)
{
    if (!sp)
        return nullptr;

    CreatureProperties const* creature_properties = sMySQLStore.getCreatureProperties(sp->entry);
    if (creature_properties == nullptr)
    {
        return nullptr;
    }

    uint8 Gender = creature_properties->GetGenderAndCreateRandomDisplayID(&sp->displayid);
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

void MapScriptInterface::DeleteCreature(Creature* ptr)
{
    delete ptr;
}

void MapScriptInterface::DeleteGameObject(GameObject* ptr)
{
    delete ptr;
}
