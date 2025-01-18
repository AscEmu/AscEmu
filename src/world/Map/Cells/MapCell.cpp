/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "VMapFactory.h"
#include "VMapManager2.h"
#include "MMapManager.h"
#include "MMapFactory.h"
#include "Map/Cells/MapCell.hpp"

#include "Logging/Logger.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/GameObject.h"
#include "Management/ObjectMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObjectProperties.hpp"
#include "Server/EventMgr.h"
#include "Server/World.h"
#include "Storage/MySQLDataStore.hpp"

std::mutex m_cellloadLock;
uint32_t m_celltilesLoaded[MAX_NUM_MAPS][64][64];

extern bool bServerShutdown;

MapCell::~MapCell()
{
    removeObjects();
}

void MapCell::init(uint32_t x, uint32_t y, WorldMap* mapmgr)
{
    _map = mapmgr;
    _active = false;
    _loaded = false;
    _playerCount = 0;
    _corpses.clear();
    _x = static_cast<uint16_t>(x);
    _y = static_cast<uint16_t>(y);
    _unloadpending = false;
    _objects.clear();
    objects_iterator = _objects.begin();
}

void MapCell::addObject(Object* obj)
{
    if (obj->isPlayer())
    {
        ++_playerCount;
    }
    else if (obj->isTransporter())
    {
        ++_transportCount;
    }
    else if (obj->isCorpse())
    {
        _corpses.push_back(obj);
        if (_unloadpending)
            cancelPendingUnload();
    }

    _objects.insert(obj);
}

void MapCell::removeObject(Object* obj)
{
    if (obj->isPlayer())
        --_playerCount;
    else if (obj->isTransporter())
        --_transportCount;
    else if (obj->isCorpse())
        _corpses.remove(obj);

    if (objects_iterator != _objects.end() && (*objects_iterator) == obj)
        ++objects_iterator;

    _objects.erase(obj);
}

void MapCell::setActivity(bool state)
{
    uint32_t mapId = _map->getBaseMap()->getMapId();
    uint32_t tileX = _x / 8;
    uint32_t tileY = _y / 8;

    if (!state)
        _idlepending = false;

    if (!_active && state)
    {
        // Move all objects to active set.
        for (ObjectSet::iterator itr = _objects.begin(); itr != _objects.end(); ++itr)
        {
            if (!(*itr)->IsActive() && (*itr)->CanActivate())
                (*itr)->Activate(_map);
        }

        if (_unloadpending)
            cancelPendingUnload();

        if (worldConfig.terrainCollision.isCollisionEnabled)
        {
            const auto mgr = VMAP::VMapFactory::createOrGetVMapManager();
            MMAP::MMapManager* mmgr = MMAP::MMapFactory::createOrGetMMapManager();

            std::string vmapPath = worldConfig.server.dataDir + "vmaps";
            std::string mmapPath = worldConfig.server.dataDir + "mmaps";

            std::lock_guard lock(m_cellloadLock);

            if (m_celltilesLoaded[mapId][tileX][tileY] == 0)
            {
                mgr->loadMap(vmapPath.c_str(), mapId, tileX, tileY);
                mmgr->loadMap(mmapPath, mapId, tileX, tileY);
            }
            ++m_celltilesLoaded[mapId][tileX][tileY];
        }
    }
    else if (_active && !state)
    {
        // Move all objects from active set.
        for (ObjectSet::iterator itr = _objects.begin(); itr != _objects.end(); ++itr)
        {
            if ((*itr)->IsActive())
                (*itr)->deactivate(_map);
        }

        if (!_unloadpending && canUnload())
            queueUnloadPending();

        if (worldConfig.terrainCollision.isCollisionEnabled)
        {
            const auto mgr = VMAP::VMapFactory::createOrGetVMapManager();
            MMAP::MMapManager* mmgr = MMAP::MMapFactory::createOrGetMMapManager();
            std::lock_guard lock(m_cellloadLock);

            if (!(--m_celltilesLoaded[mapId][tileX][tileY]))
            {
                mgr->unloadMap(mapId, tileX, tileY);
                mmgr->unloadMap(mapId, tileX, tileY);
            }
        }
    }

    _active = state;
}

void MapCell::removeObjects()
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
                    _map->_reusable_guids_creature.push_back((*itr)->GetUIdFromGUID());
                    reinterpret_cast<Creature*>(*itr)->m_respawnCell = nullptr;
                    delete static_cast<Creature*>(*itr);
                }
                break;
            case TYPEID_GAMEOBJECT:
                _map->_reusable_guids_gameobject.push_back((*itr)->GetUIdFromGUID());
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
        if (!bServerShutdown && obj->isGameObject() && static_cast<GameObject*>(obj)->GetGameObjectProperties()->type == GAMEOBJECT_TYPE_MO_TRANSPORT)
            continue;

        if (obj->IsActive())
            obj->deactivate(_map);

        if (obj->IsInWorld())
            obj->RemoveFromWorld(true);

        delete obj;
    }
    _objects.clear();
    _corpses.clear();
    _playerCount = 0;
    _transportCount = 0;
    _loaded = false;
}

void MapCell::loadObjects(CellSpawns* sp)
{
    //we still have mobs loaded on cell. There is no point of loading them again
    if (_loaded == true)
        return;

    _loaded = true;
    if (sp->CreatureSpawns.size())      //got creatures
    {
        for (CreatureSpawnList::iterator i = sp->CreatureSpawns.begin(); i != sp->CreatureSpawns.end(); ++i)
        {
            auto spawnGroupData = sMySQLStore.getSpawnGroupDataBySpawn((*i)->id);
            bool onRespawn = false;

            Creature* c = _map->createCreature((*i)->entry);

            c->m_loadedFromDB = true;

            if (c->Load(*i, _map->getDifficulty(), _map->getBaseMap()->getMapInfo()) && c->CanAddToWorld())
            {
                // Respawn Handling
                if (auto info = _map->getRespawnInfo(SPAWN_TYPE_CREATURE, c->spawnid))
                {
                    onRespawn = true;

                    // get the cell with our SPAWN location. if we've moved cell this might break :P
                    MapCell* pCell = _map->getCellByCoords(c->GetSpawnX(), c->GetSpawnY());
                    if (pCell == nullptr)
                        pCell = c->GetMapCell();

                    if (pCell != nullptr)
                    {
                        pCell->_respawnObjects.insert(c);

                        c->SetPosition(c->GetSpawnPosition(), true);
                        c->m_respawnCell = pCell;
                        sEventMgr.RemoveEvents(c);

                        RespawnInfo ri;
                        ri.type = SPAWN_TYPE_CREATURE;
                        ri.spawnId = c->getSpawnId();
                        ri.entry = c->getEntry();
                        ri.time = info->time;
                        ri.cellX = c->GetSpawnX();
                        ri.cellY = c->GetSpawnY();
                        ri.obj = c;

                        bool success = _map->addRespawn(ri);
                        if (success)
                            _map->saveRespawnDB(ri);
                    }
                }

                if (spawnGroupData)
                    spawnGroupData->spawns[(*i)->id] = c;

                // Creatures Spawning
                if (!onRespawn)
                    c->PushToWorld(_map);
            }
            else
            {
                MySQLStructure::CreatureSpawn* spawn = (*i);
                sLogger.failure("Failed spawning Creature {} with spawnId {} MapId {}", spawn->entry, spawn->id, _map->getBaseMap()->getMapId());
                delete c;       //missing proto or something of that kind
            }
        }
    }

    if (sp->GameobjectSpawns.size())    //got GOs
    {
        for (GameobjectSpawnList::iterator i = sp->GameobjectSpawns.begin(); i != sp->GameobjectSpawns.end(); ++i)
        {
            GameObject* go = _map->createGameObject((*i)->entry);
            bool onRespawn = false;
            
            if (go->loadFromDB(*i, _map, false))
            {
                go->m_loadedFromDB = true;

                // Respawn Handling
                if (auto info = _map->getRespawnInfo(SPAWN_TYPE_GAMEOBJECT, go->getSpawnId()))
                {
                    onRespawn = true;
                    RespawnInfo ri;
                    ri.type = SPAWN_TYPE_GAMEOBJECT;
                    ri.spawnId = go->getSpawnId();
                    ri.entry = go->getEntry();
                    ri.time = info->time;
                    ri.cellX = go->GetSpawnX();
                    ri.cellY = go->GetSpawnY();
                    ri.obj = nullptr;

                    bool success = _map->addRespawn(ri);
                    if (success)
                        _map->saveRespawnDB(ri);

                    go->expireAndDelete();
                }

                if (!onRespawn)
                    go->PushToWorld(_map);
            }
            else
            {
                MySQLStructure::GameobjectSpawn* spawn = (*i);
                sLogger.failure("Failed spawning GameObject {} with spawnId {} MapId {}", spawn->entry, spawn->id, _map->getBaseMap()->getMapId());
                delete go;          //missing proto or something of that kind
            }
        }
    }
}

bool MapCell::isIdlePending() const
{
    return _idlepending;
}

void MapCell::scheduleCellIdleState()
{
    if (isIdlePending() || _unloadpending)
        return;

    _idlepending = true;
    sLogger.debug("Queueing pending idle of cell {} {}", _x, _y);
    sEventMgr.AddEvent(_map, &WorldMap::setCellIdle, _x, _y, this, MAKE_CELL_EVENT(_x, _y), 30000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void MapCell::cancelPendingIdle()
{
    if (!isIdlePending() || _unloadpending)
        return;

    sLogger.debug("Cancelling pending idle of cell {} {}", _x, _y);
    sEventMgr.RemoveEvents(_map, MAKE_CELL_EVENT(_x, _y));
    _idlepending = false;
}

void MapCell::queueUnloadPending()
{
    if (_unloadpending)
        return;

    if (isIdlePending())
        cancelPendingIdle();

    _unloadpending = true;
    sLogger.debug("Queueing pending unload of cell {} {}", _x, _y);
    sEventMgr.AddEvent(_map, &WorldMap::unloadCell, (uint32_t)_x, (uint32_t)_y, MAKE_CELL_EVENT(_x, _y), worldConfig.server.mapUnloadTime * 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void MapCell::cancelPendingUnload()
{
    sLogger.debug("Cancelling pending unload of cell {} {}", _x, _y);
    if (!_unloadpending)
        return;

    if (isIdlePending())
        cancelPendingIdle();

    sEventMgr.RemoveEvents(_map, MAKE_CELL_EVENT(_x, _y));
    _unloadpending = false;
}

void MapCell::unload()
{
    if (_unloadpending)
    {
        sLogger.debug("Unloading cell {} {}", _x, _y);

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

        sLogger.debug("Unloading cell {} {}", _x, _y);

        removeObjects();
        _map->remove(_x, _y);
    }
    else
    {
        sLogger.failure("MapCell::Unload tried to unload MapCell which is not marked for unload");
    }
}

void MapCell::corpseGoneIdle(Object* corpse)
{
    _corpses.remove(corpse);
    checkUnload();
}

void MapCell::checkUnload()
{
    if (!_active && !_unloadpending && canUnload())
        queueUnloadPending();
}

bool MapCell::canUnload()
{
    if (_corpses.size() == 0  && !_map->getBaseMap()->isBattlegroundOrArena())
        return true;
    else
        return false;
}

