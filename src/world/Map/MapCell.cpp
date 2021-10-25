/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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


#include "VMapFactory.h"
#include "MMapManager.h"
#include "MMapFactory.h"
#include "Map/MapCell.h"
#include "MapMgr.h"
#include "WorldCreator.h"
#include "Units/Creatures/Creature.h"
#include "Objects/GameObject.h"
#include "Objects/ObjectMgr.h"

Mutex m_cellloadLock;
uint32 m_celltilesLoaded[MAX_NUM_MAPS][64][64];

extern bool bServerShutdown;

MapCell::~MapCell()
{
    RemoveObjects();
}

void MapCell::Init(uint32 x, uint32 y, MapMgr* mapmgr)
{
    _mapmgr = mapmgr;
    _active = false;
    _loaded = false;
    _playerCount = 0;
    _corpses.clear();
    _x = static_cast<uint16>(x);
    _y = static_cast<uint16>(y);
    _unloadpending = false;
    _objects.clear();
    objects_iterator = _objects.begin();
}

void MapCell::AddObject(Object* obj)
{
    if (obj->isPlayer())
        ++_playerCount;
    else if (obj->isCorpse())
    {
        _corpses.push_back(obj);
        if (_unloadpending)
            CancelPendingUnload();
    }

    _objects.insert(obj);
}

void MapCell::RemoveObject(Object* obj)
{
    if (obj->isPlayer())
        --_playerCount;
    else if (obj->isCorpse())
        _corpses.remove(obj);

    if (objects_iterator != _objects.end() && (*objects_iterator) == obj)
        ++objects_iterator;

    _objects.erase(obj);
}

void MapCell::SetActivity(bool state)
{
    uint32 mapId = _mapmgr->GetMapId();
    uint32 tileX = _x / 8;
    uint32 tileY = _y / 8;

    if (!_active && state)
    {
        // Move all objects to active set.
        for (ObjectSet::iterator itr = _objects.begin(); itr != _objects.end(); ++itr)
        {
            if (!(*itr)->IsActive() && (*itr)->CanActivate())
                (*itr)->Activate(_mapmgr);
        }

        if (_unloadpending)
            CancelPendingUnload();

        if (worldConfig.terrainCollision.isCollisionEnabled)
        {
            VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
            MMAP::MMapManager* mmgr = MMAP::MMapFactory::createOrGetMMapManager();

            std::string vmapPath = worldConfig.server.dataDir + "vmaps";
            std::string mmapPath = worldConfig.server.dataDir + "mmaps";

            m_cellloadLock.Acquire();
            if (m_celltilesLoaded[mapId][tileX][tileY] == 0)
            {
                mgr->loadMap(vmapPath.c_str(), mapId, tileX, tileY);
                mmgr->loadMap(mmapPath, mapId, tileX, tileY);
            }
            ++m_celltilesLoaded[mapId][tileX][tileY];
            m_cellloadLock.Release();
        }
    }
    else if (_active && !state)
    {
        // Move all objects from active set.
        for (ObjectSet::iterator itr = _objects.begin(); itr != _objects.end(); ++itr)
        {
            if ((*itr)->IsActive())
                (*itr)->Deactivate(_mapmgr);
        }

        if (!_unloadpending && CanUnload())
            QueueUnloadPending();

        if (worldConfig.terrainCollision.isCollisionEnabled)
        {
            VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
            MMAP::MMapManager* mmgr = MMAP::MMapFactory::createOrGetMMapManager();
            m_cellloadLock.Acquire();
            if (!(--m_celltilesLoaded[mapId][tileX][tileY]))
            {
                mgr->unloadMap(mapId, tileX, tileY);
                mmgr->unloadMap(mapId, tileX, tileY);
            }

            m_cellloadLock.Release();
        }
    }

    _active = state;

}

void MapCell::RemoveObjects()
{
    ObjectSet::iterator itr;

    //Zack : we are delaying cell removal so transports can see objects far away. We are waiting for the event to remove us
    if (_unloadpending == true)
        return;

    /* delete objects in pending respawn state */
    for (itr = _respawnObjects.begin(); itr != _respawnObjects.end(); ++itr)
    {
        switch ((*itr)->getObjectTypeId())
        {
            case TYPEID_UNIT:
                if (!(*itr)->isPet())
                {
                    _mapmgr->_reusable_guids_creature.push_back((*itr)->GetUIdFromGUID());
                    reinterpret_cast<Creature*>(*itr)->m_respawnCell = nullptr;
                    delete static_cast<Creature*>(*itr);
                }
                break;
            case TYPEID_GAMEOBJECT:
                _mapmgr->_reusable_guids_gameobject.push_back((*itr)->GetUIdFromGUID());
                reinterpret_cast<GameObject*>(*itr)->m_respawnCell = nullptr;
                delete static_cast<GameObject*>(*itr);
                break;
        }
    }
    _respawnObjects.clear();

    //This time it's simpler! We just remove everything :)
    for (objects_iterator = _objects.begin(); objects_iterator != _objects.end();)
    {
        Object* obj = (*objects_iterator);

        ++objects_iterator;

        //If MapUnloadTime is non-zero, a transport could get deleted here (when it arrives to a cell that's scheduled to be unloaded because players left from it), so don't delete it! - By: VLack aka. VLsoft
        if (!bServerShutdown && obj->isGameObject() && static_cast< GameObject* >(obj)->GetGameObjectProperties()->type == GAMEOBJECT_TYPE_MO_TRANSPORT)
            continue;

        if (obj->IsActive())
            obj->Deactivate(_mapmgr);

        if (obj->IsInWorld())
            obj->RemoveFromWorld(true);

        delete obj;
    }
    _objects.clear();
    _corpses.clear();
    _playerCount = 0;
    _loaded = false;
}


void MapCell::LoadObjects(CellSpawns* sp)
{
    //we still have mobs loaded on cell. There is no point of loading them again
    if (_loaded == true)
        return;

    _loaded = true;
    Instance* pInstance = _mapmgr->pInstance;
    if (sp->CreatureSpawns.size())      //got creatures
    {
        for (CreatureSpawnList::iterator i = sp->CreatureSpawns.begin(); i != sp->CreatureSpawns.end(); ++i)
        {
            auto spawnGroupData = sMySQLStore.getSpawnGroupDataBySpawn((*i)->id);
            bool skip = false;
            bool onRespawn = false;
            if (pInstance)
            {
                auto encounters = sObjectMgr.GetDungeonEncounterList(_mapmgr->GetMapId(), pInstance->m_difficulty);

                // Spawn Group Handling
                if (spawnGroupData && spawnGroupData->groupFlags & SPAWNGROUP_FLAG_MANUAL_SPAWN)
                    skip = true;

                if (encounters != NULL && !skip)
                {
                    for (auto killedNpc : pInstance->m_killedNpcs)
                    {
                        // is Boss Killed but trash untouched ( on Retail all Npcs not Killed but linked to a Boss thats already Killed dont Spawn )
                        if (spawnGroupData && spawnGroupData->bossId)
                        {
                            if (pInstance->m_killedNpcs.find(spawnGroupData->bossId) != pInstance->m_killedNpcs.end())
                            {
                                skip = true;
                                break;
                            }
                            else
                            {
                                skip = false;
                                break;
                            }
                        }

                        // is Killed add ?
                        if (killedNpc == (*i)->id)
                        {
                            auto data = sMySQLStore.getSpawnGroupDataBySpawn(killedNpc);

                            // When Our Add is bound to a Boss thats not killed Respawn it
                            if (data && data->spawnFlags & SPAWFLAG_FLAG_BOUNDTOBOSS && data->bossId)
                            {
                                if (pInstance->m_killedNpcs.find(data->bossId) != pInstance->m_killedNpcs.end())
                                {
                                    skip = true;
                                    break;
                                }
                                else
                                {
                                    skip = false;
                                    break;
                                }
                            }

                            onRespawn = true;
                            break;
                        }

                        // Is killed boss?
                        if (killedNpc == (*i)->entry)
                        {
                            for (DungeonEncounterList::const_iterator itr = encounters->begin(); itr != encounters->end(); ++itr)
                            {
                                DungeonEncounter const* encounter = *itr;
                                if (encounter->creditType == ENCOUNTER_CREDIT_KILL_CREATURE && encounter->creditEntry == killedNpc)
                                {
                                    skip = true;
                                    break;
                                }

                            }
                        }
                    }

                    if (!skip && !onRespawn)
                    {
                        // remove from Killed Npcs
                        if (pInstance->m_killedNpcs.find((*i)->id) != pInstance->m_killedNpcs.end())
                            pInstance->m_killedNpcs.erase((*i)->id);
                    }
                }
                else
                {
                    // No boss information available ... fallback ...
                    if (pInstance->m_killedNpcs.find((*i)->id) != pInstance->m_killedNpcs.end())
                        continue;
                }
            }

            sInstanceMgr.SaveInstanceToDB(pInstance);

            Creature* c = _mapmgr->CreateCreature((*i)->entry);

            c->m_loadedFromDB = true;


            if (c->Load(*i, _mapmgr->iInstanceMode, _mapmgr->GetMapInfo()) && c->CanAddToWorld())
            {
                if (spawnGroupData)
                    spawnGroupData->spawns[(*i)->id] = c;

                if (!skip && !onRespawn)
                    c->PushToWorld(_mapmgr);

                // Creatures in Instances are always Respawning after 2 hours
                if (onRespawn)
                {
                    // get the cell with our SPAWN location. if we've moved cell this might break :P
                    MapCell* pCell = _mapmgr->GetCellByCoords(c->GetSpawnX(), c->GetSpawnY());
                    if (pCell == nullptr)
                        pCell = c->GetMapCell();

                    if (pCell != nullptr)
                    {
                        pCell->_respawnObjects.insert(c);

                        sEventMgr.RemoveEvents(c);
                        sEventMgr.AddEvent(_mapmgr, &MapMgr::EventRespawnCreature, c, pCell->GetPositionX(), pCell->GetPositionY(), EVENT_CREATURE_RESPAWN, (1000 * 60 * 60 * 2), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

                        c->SetPosition(c->GetSpawnPosition(), true);
                        c->m_respawnCell = pCell;
                    }
                }
            }
            else
            {
                MySQLStructure::CreatureSpawn* spawn = (*i);
                sLogger.failure("Failed spawning Creature %u with spawnId %u MapId %u", spawn->entry, spawn->id, _mapmgr->GetMapId());
                delete c;       //missing proto or something of that kind
            }
        }
    }

    if (sp->GameobjectSpawns.size())    //got GOs
    {
        for (GameobjectSpawnList::iterator i = sp->GameobjectSpawns.begin(); i != sp->GameobjectSpawns.end(); ++i)
        {
            GameObject* go = _mapmgr->CreateGameObject((*i)->entry);

            if (go->Load(*i))
            {
                go->m_loadedFromDB = true;
                go->PushToWorld(_mapmgr);
            }
            else
            {
                MySQLStructure::GameobjectSpawn* spawn = (*i);
                sLogger.failure("Failed spawning GameObject %u with spawnId %u MapId %u", spawn->entry, spawn->id, _mapmgr->GetMapId());
                delete go;          //missing proto or something of that kind
            }
        }
    }
}


void MapCell::QueueUnloadPending()
{
    if (_unloadpending)
        return;

    _unloadpending = true;
    sLogger.debug("Queueing pending unload of cell %u %u", _x, _y);
    sEventMgr.AddEvent(_mapmgr, &MapMgr::UnloadCell, (uint32)_x, (uint32)_y, MAKE_CELL_EVENT(_x, _y), worldConfig.server.mapUnloadTime * 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void MapCell::CancelPendingUnload()
{
    sLogger.debug("Cancelling pending unload of cell %u %u", _x, _y);
    if (!_unloadpending)
        return;

    sEventMgr.RemoveEvents(_mapmgr, MAKE_CELL_EVENT(_x, _y));
    _unloadpending = false;
}

void MapCell::Unload()
{
    if (_unloadpending)
    {
        sLogger.debug("Unloading cell %u %u", _x, _y);

        if (_active)
        {
            sLogger.failure("MapCell::Unload tried to unload an active MapCell, return!");
            return;
        }

        _unloadpending = false;

        /*in ~MapCell RemoveObjects() can delete an Object without removing it from the MapCell.cpp
        Example:
        Creature A has guardian B. MapCell is unloaded, _mapmgr->Remove(_x, _y) is called, nullifying the reference to the cell
        in CellHandler. ~MapCell is called, RemoveObjects() is called and despawns A which despawns B, calling Object::RemoveFromWorld()
        which calls MapMgr::RemoveObject(B) which calls cell->RemoveObject(obj) ONLY if cell is not NULL, but in this case is NULL, leaving
        a reference to a deleted Object in MapCell::_objects, iterated in RemoveObjects(). Calling it here fixes this issue.
        Note: RemoveObjects() is still called in ~MapCell, due to fancy ArcEmu behaviors, like the in-game command ".mapcell delete <x> <y>*/

        sLogger.debug("Unloading cell %u %u", _x, _y);

        RemoveObjects();
        _mapmgr->Remove(_x, _y);
    }
    else
    {
        sLogger.failure("MapCell::Unload tried to unload MapCell which is not marked for unload");
    }
}

void MapCell::CorpseGoneIdle(Object* corpse)
{
    _corpses.remove(corpse);
    CheckUnload();
}

void MapCell::CheckUnload()
{
    if (!_active && !_unloadpending && CanUnload())
        QueueUnloadPending();
}

bool MapCell::CanUnload()
{
    if (_corpses.size() == 0 && _mapmgr->m_battleground == NULL)
        return true;
    else
        return false;
}
