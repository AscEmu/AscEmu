/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */


/* * Class MapScriptInterface
   * Provides an interface to mapmgr for scripts, to obtain objects,
   * get players, etc.
   */

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "MapScriptInterface.h"

createFileSingleton(StructFactory);

MapScriptInterface::MapScriptInterface(MapMgr & mgr) : mapMgr(mgr)
{}

MapScriptInterface::~MapScriptInterface()
{
    mapMgr.ScriptInterface = nullptr;
}

uint32 MapScriptInterface::GetPlayerCountInRadius(float x, float y, float z /* = 0.0f */, float radius /* = 5.0f */)
{
    // use a cell radius of 2
    uint32 PlayerCount = 0;
    uint32 cellX = mapMgr.GetPosX(x);
    uint32 cellY = mapMgr.GetPosY(y);

    uint32 endX = cellX < _sizeX ? cellX + 1 : _sizeX;
    uint32 endY = cellY < _sizeY ? cellY + 1 : _sizeY;
    uint32 startX = cellX > 0 ? cellX - 1 : 0;
    uint32 startY = cellY > 0 ? cellY - 1 : 0;
    MapCell* pCell;
    ObjectSet::iterator iter, iter_end;

    for (uint32 cx = startX; cx < endX; ++cx)
    {
        for (uint32 cy = startY; cy < endY; ++cy)
        {
            pCell = mapMgr.GetCell(cx, cy);
            if (pCell == nullptr || pCell->GetPlayerCount() == 0)
                continue;

            iter = pCell->Begin();
            iter_end = pCell->End();

            for (; iter != iter_end; ++iter)
            {
                if ((*iter)->IsPlayer() &&
                    (*iter)->CalcDistance(x, y, (z == 0.0f ? (*iter)->GetPositionZ() : z)) < radius)
                {
                    ++PlayerCount;
                }
            }
        }
    }

    return PlayerCount;
}

GameObject* MapScriptInterface::SpawnGameObject(uint32 Entry, float cX, float cY, float cZ, float cO, bool AddToWorld, uint32 Misc1, uint32 Misc2, uint32 phase)
{

    GameObject* pGameObject = mapMgr.CreateGameObject(Entry);
    if (!pGameObject->CreateFromProto(Entry, mapMgr.GetMapId(), cX, cY, cZ, cO))
    {
        delete pGameObject;
        return nullptr;
    }
    pGameObject->m_phase = phase;
    pGameObject->m_spawn = nullptr;

    if (AddToWorld)
        pGameObject->PushToWorld(&mapMgr);

    return pGameObject;
}

GameObject* MapScriptInterface::SpawnGameObject(GameobjectSpawn* gs, bool AddToWorld)
{
    if (!gs)
        return nullptr;

    GameObject* pGameObject = mapMgr.CreateGameObject(gs->entry);
    if (!pGameObject->Load(gs))
    {
        delete pGameObject;
        return nullptr;
    }

    if (AddToWorld)
        pGameObject->PushToWorld(&mapMgr);

    return pGameObject;
}

// Zyres 11/06/2017 - bool tmplate not used!
Creature* MapScriptInterface::SpawnCreature(uint32 Entry, float cX, float cY, float cZ, float cO, bool AddToWorld, bool /*tmplate*/, uint32 /*Misc1*/, uint32 /*Misc2*/, uint32 phase)
{
    CreatureProperties const* creature_properties = sMySQLStore.getCreatureProperties(Entry);
    if (creature_properties == nullptr)
        return nullptr;

    CreatureSpawn* spawn = new CreatureSpawn;
    spawn->entry = Entry;
    uint32 DisplayID = 0;
    uint8 Gender = creature_properties->GetGenderAndCreateRandomDisplayID(&DisplayID);
    spawn->displayid = DisplayID;
    spawn->form = nullptr;
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
    spawn->Item1SlotDisplay = creature_properties->itemslot_1;
    spawn->Item2SlotDisplay = creature_properties->itemslot_2;
    spawn->Item3SlotDisplay = creature_properties->itemslot_3;
    spawn->CanFly = 0;
    spawn->phase = phase;

    Creature* creature = this->mapMgr.CreateCreature(Entry);
    ARCEMU_ASSERT(creature != nullptr);

    creature->Load(spawn, 0, nullptr);
    creature->setGender(Gender);
    creature->spawnid = 0;
    creature->m_spawn = nullptr;

    delete spawn;

    if (AddToWorld)
        creature->PushToWorld(&mapMgr);

    return creature;
}

Creature* MapScriptInterface::SpawnCreature(CreatureSpawn* sp, bool AddToWorld)
{
    if (!sp)
        return nullptr;

    CreatureProperties const* creature_properties = sMySQLStore.getCreatureProperties(sp->entry);
    if (creature_properties == nullptr)
    {
        return nullptr;
    }

    uint8 Gender = creature_properties->GetGenderAndCreateRandomDisplayID(&sp->displayid);
    Creature* p = this->mapMgr.CreateCreature(sp->entry);
    ARCEMU_ASSERT(p != NULL);
    p->Load(sp, (uint32)NULL, nullptr);
    p->setGender(Gender);
    p->spawnid = 0;
    p->m_spawn = nullptr;
    if (AddToWorld)
        p->PushToWorld(&mapMgr);
    return p;
}

void MapScriptInterface::DeleteCreature(Creature* ptr)
{
    delete ptr;
}

void MapScriptInterface::DeleteGameObject(GameObject* ptr)
{
    delete ptr;
}

Movement::WayPoint* StructFactory::CreateWaypoint()
{
    return new Movement::WayPoint;
}
