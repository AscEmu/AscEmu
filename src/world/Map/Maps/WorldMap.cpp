/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WorldMap.hpp"
#include "Objects/DynamicObject.h"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Creatures/Summons/Summon.h"
#include "Objects/Units/Unit.h"
#include "VMapFactory.h"
#include "MMapFactory.h"
#include "Map/Cells/CellHandler.hpp"
#include "shared/WoWGuid.h"
#include "MapScriptInterface.h"
#include "Server/Script/ScriptMgr.h"
#include "Management/WorldStatesHandler.h"
#include "InstanceMap.hpp"
#include "Server/Packets/SmsgUpdateWorldState.h"
#include "Server/Packets/SmsgDefenseMessage.h"
#include "Map/Area/AreaManagementGlobals.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "TLSObject.h"
#include "CThreads.h"
#include "CrashHandler.h"

using namespace AscEmu::Packets;

Arcemu::Utility::TLSObject<WorldMap*> t_currentMapContext;

extern bool bServerShutdown;

WorldMap::WorldMap(BaseMap* baseMap, uint32_t id, time_t expiry, uint32_t InstanceId, uint8_t SpawnMode) : CellHandler<MapCell>(baseMap), eventHolder(InstanceId), worldstateshandler(id)
{
    // Thread
    thread_shutdown = false;
    thread_kill_only = false;
    thread_running = false;

    // Map
    _terrain = new TerrainHolder(id);
    m_baseMap = baseMap;
    pInstance = nullptr;
    m_unloadTimer = expiry;
    setSpawnMode(SpawnMode);
    setInstanceId(InstanceId);

    m_holder = &eventHolder;
    m_event_Instanceid = eventHolder.GetInstanceID();

    // Create script interface
    ScriptInterface = new MapScriptInterface(*this);

    // Set up storage arrays
    m_CreatureStorage.resize(getBaseMap()->CreatureSpawnCount, nullptr);
    m_GameObjectStorage.resize(getBaseMap()->GameObjectSpawnCount, nullptr);

    // Guids
    m_GOHighGuid = 0;
    m_CreatureHighGuid = 0;
    m_DynamicObjectHighGuid = 0;

    // Storage
    m_forcedcells.clear();
    m_PlayerStorage.clear();
    m_PetStorage.clear();
    m_DynamicObjectStorage.clear();
    Sessions.clear();
    m_TransportStorage.clear();
    _combatProgress.clear();

    // Active Set
    _mapWideStaticObjects.clear();
    activeGameObjects.clear();
    activeCreatures.clear();
    m_corpses.clear();

    _sqlids_creatures.clear();
    _sqlids_gameobjects.clear();
    _reusable_guids_gameobject.clear();
    _reusable_guids_creature.clear();

    // Timers
    _sessionUpdateTimer = 20;
    _respawnUpdateTimer = 1000;
    _dynamicUpdateTimer = 100;
    _gameObjectUpdateTimer = 200;

    // Updates
    _updates.clear();
    _processQueue.clear();
    Sessions.clear();

    // Script
    mInstanceScript = nullptr;

    //lets initialize visibility distance for Continent
    WorldMap::initVisibilityDistance();
}

void WorldMap::initialize()
{
    // Create Instance script
    loadInstanceScript();

    // create static objects
    for (auto& GameobjectSpawn : _map->staticSpawns.GameobjectSpawns)
    {
        GameObject* obj = createGameObject(GameobjectSpawn->entry);
        obj->Load(GameobjectSpawn);
        PushStaticObject(obj);
    }

    for (auto& CreatureSpawn : _map->staticSpawns.CreatureSpawns)
    {
        Creature* obj = createCreature(CreatureSpawn->entry);
        obj->Load(CreatureSpawn, 0, getBaseMap()->getMapInfo());
        PushStaticObject(obj);
    }

    // Call script OnLoad virtual procedure
    CALL_INSTANCE_SCRIPT_EVENT(this, OnLoad)();

    // load corpses
    sObjectMgr.LoadCorpses(this);
    worldstateshandler.InitWorldStates(sObjectMgr.GetWorldStatesForMap(getBaseMap()->getMapId()));
    worldstateshandler.setObserver(this);
}

WorldMap::~WorldMap()
{
    thread_shutdown = true;
    sEventMgr.RemoveEvents(this);

    if (ScriptInterface != nullptr)
    {
        delete ScriptInterface;
        ScriptInterface = nullptr;
    }

    delete _terrain;

    // Remove objects
    if (_cells)
    {
        for (uint32_t i = 0; i < Map::Cell::_sizeX; i++)
        {
            if (_cells[i] != 0)
            {
                for (uint32_t j = 0; j < Map::Cell::_sizeY; j++)
                {
                    if (_cells[i][j] != 0)
                    {
                        _cells[i][j]->_unloadpending = false;
                        _cells[i][j]->removeObjects();
                    }
                }
            }
        }
    }

    // Static Spawns
    for (auto _mapWideStaticObject : _mapWideStaticObjects)
    {
        if (_mapWideStaticObject->IsInWorld())
            _mapWideStaticObject->RemoveFromWorld(false);
        delete _mapWideStaticObject;
    }
    _mapWideStaticObjects.clear();

    m_GameObjectStorage.clear();
    m_CreatureStorage.clear();
    m_TransportStorage.clear();

    unloadAllRespawnInfos();

    for (auto itr = m_corpses.begin(); itr != m_corpses.end();)
    {
        Corpse* pCorpse = *itr;
        ++itr;

        if (pCorpse->IsInWorld())
            pCorpse->RemoveFromWorld(false);

        delete pCorpse;
    }
    m_corpses.clear();

    if (mInstanceScript != NULL)
        mInstanceScript->Destroy();

    // Empty remaining containers
    m_PlayerStorage.clear();
    m_PetStorage.clear();
    m_DynamicObjectStorage.clear();

    _updates.clear();
    _processQueue.clear();
    Sessions.clear();

    activeCreatures.clear();
    activeGameObjects.clear();
    _sqlids_creatures.clear();
    _sqlids_gameobjects.clear();
    _reusable_guids_creature.clear();
    _reusable_guids_gameobject.clear();

    MMAP::MMapFactory::createOrGetMMapManager()->unloadMapInstance(getBaseMap()->getMapId(), getInstanceId());

    sLogger.debug("WorldMap : Instance %u shut down. (%s)", getInstanceId(), getBaseMap()->getMapName().c_str());
}

bool WorldMap::runThread()
{
    bool rv = true;

    THREAD_TRY_EXECUTION
        rv = Do();
    THREAD_HANDLE_CRASH
        return rv;
}

bool WorldMap::Do()
{
#ifdef WIN32
    threadid = GetCurrentThreadId();
#endif

    t_currentMapContext.set(this);

    thread_running = true;
    ThreadState = THREADSTATE_BUSY;

    uint32_t id = getBaseMap()->getMapId();
    SetThreadName("WorldMap - M%u|I%u", getBaseMap()->getMapId(), getInstanceId());

    uint32_t last_exec = Util::getMSTime();

    Arcemu::Sleep(1000);

    while (GetThreadState() != THREADSTATE_TERMINATE && !thread_shutdown)
    {
        uint32_t exec_start = Util::getMSTime();
        uint32_t difftime = exec_start - last_exec;

        // Update Our Map
        update(difftime);

        last_exec = Util::getMSTime();

        // Sleep for 20 ms
        Arcemu::Sleep(20);
    }

    thread_running = false;
    if (thread_kill_only)
        return false;

    // delete ourselves
    delete this;

    // already deleted, so the threadpool doesn't have to.
    return false;
}

void WorldMap::instanceShutdown()
{
    pInstance = nullptr;
    SetThreadState(THREADSTATE_TERMINATE);
}

void WorldMap::killThread()
{
    pInstance = nullptr;
    thread_kill_only = true;
    SetThreadState(THREADSTATE_TERMINATE);
    while (thread_running)
    {
        Arcemu::Sleep(100);
    }
}

void WorldMap::update(uint32_t t_diff)
{
    //first push to world new objects
    {
        std::unique_lock<std::mutex> lock(m_objectinsertlock);
        if (m_objectinsertpool.size())
        {
            for (auto o : m_objectinsertpool)
                o->PushToWorld(this);

            m_objectinsertpool.clear();
        }
    }

    std::unique_lock<std::mutex> updateLock(m_Updatelock);

    // Time In Seconds
    const auto now = Util::getTimeNow();

    // Update any events.
    // we make update of events before objects so in case there are 0 timediff events they do not get deleted after update but on next server update loop
    eventHolder.Update(t_diff);

    // Update Dynamic Map
    _dynamicTree.update(t_diff);

    // Update Transporters
    {
        std::unique_lock<std::mutex> guard(m_transportsLock);
        for (auto itr = m_TransportStorage.cbegin(); itr != m_TransportStorage.cend();)
        {
            Transporter* trans = *itr;
            ++itr;

            if (!trans || !trans->IsInWorld())
                continue;

            trans->Update(t_diff);
        }
    }

    // Update Creatures
    {
        for (auto itr = activeCreatures.cbegin(); itr != activeCreatures.cend();)
        {
            Creature* ptr = *itr;
            ++itr;
            ptr->Update(t_diff);
        }
    }

    // Update Pets
    {
        for (auto itr = m_PetStorage.cbegin(); itr != m_PetStorage.cend();)
        {
            Pet* ptr = itr->second;
            ++itr;
            ptr->Update(t_diff);
        }
    }

    // Update Players
    {
        for (auto itr = m_PlayerStorage.cbegin(); itr != m_PlayerStorage.cend();)
        {
            Player* ptr = itr->second;
            ++itr;
            ptr->Update(t_diff);
        }
    }

    // Dynamic objects are updated every 100ms
    if (_dynamicUpdateTimer <= t_diff)
    {
        _dynamicUpdateTimer = 100;
        for (auto itr = m_DynamicObjectStorage.cbegin(); itr != m_DynamicObjectStorage.cend();)
        {
            DynamicObject* o = itr->second;
            ++itr;
            o->UpdateTargets();
        }
    }
    else
    {
        _dynamicUpdateTimer -= t_diff;
    }

    // Update gameobjects only every 200ms
    if (_gameObjectUpdateTimer <= t_diff)
    {
        _gameObjectUpdateTimer = 200;
        for (auto itr = activeGameObjects.cbegin(); itr != activeGameObjects.cend();)
        {
            GameObject* gameobject = *itr;
            ++itr;
            if (gameobject != nullptr)
                gameobject->Update(t_diff);
        }
    }
    else
    {
        _gameObjectUpdateTimer -= t_diff;
    }

    // Update Sessions
    if (_sessionUpdateTimer <= t_diff)
    {
        _sessionUpdateTimer = 1;
        for (auto itr = Sessions.cbegin(); itr != Sessions.cend();)
        {
            WorldSession* session = (*itr);
            auto it2 = itr;
            ++itr;

            if (session->GetInstance() != getInstanceId())
            {
                Sessions.erase(it2);
                continue;
            }

            // Don't update players not on our map.
            // If we abort in the handler, it means we will "lose" packets, or not process this.
            // .. and that could be disastrous to our client :P
            if (session->GetPlayer() && (session->GetPlayer()->getWorldMap() != this && session->GetPlayer()->getWorldMap() != nullptr))
                continue;

            uint8_t result;

            if ((result = session->Update(getInstanceId())) != 0)
            {
                if (result == 1)
                {
                    // complete deletion
                    sWorld.deleteSession(session);
                }
                Sessions.erase(it2);
            }
        }
    }
    else
    {
        _sessionUpdateTimer -= t_diff;
    }

    /// Update Respawns
    if (_respawnUpdateTimer <= t_diff)
    {
        processRespawns();

        while (!_corpseDespawnTimes.empty())
        {
            CorpseInfo next = _corpseDespawnTimes.top();
            if (now < next.time)
                break;

            _corpseDespawnTimes.pop();
            if (Corpse* pCorpse = sObjectMgr.GetCorpse(next.guid))
            {
                if (pCorpse->getWorldMap() != this)
                    break;

                pCorpse->Despawn();
            }
            break;
        }
        _respawnUpdateTimer = 1000;
    }
    else
    {
        _respawnUpdateTimer -= t_diff;
    }
    
    // Finally, A9 Building/Distribution
    updateObjects();
}

void WorldMap::processRespawns()
{
    const auto now = Util::getTimeNow();

    while (!_respawnTimes.empty())
    {
        RespawnInfo* next = _respawnTimes.top();
        if (now < next->time) // done for this tick
            break;

        if (checkRespawn(next)) // see if we're allowed to respawn
        {
            // ok, respawn
            _respawnTimes.pop();
            getRespawnMapForType(next->type).erase(next->spawnId);
            doRespawn(next->type, next->obj, next->spawnId, next->cellX, next->cellY);
            delete next;
        }
        else if (!next->time) // just remove respawn entry without rescheduling
        {
            _respawnTimes.pop();
            getRespawnMapForType(next->type).erase(next->spawnId);
            delete next;
        }
        else
        {
            saveRespawnDB(*next);
        }
    }
}

void WorldMap::unloadAll()
{
    if (getPlayerCount())
        return;

    sMapMgr.removeInstance(getInstanceId());
    instanceShutdown();
}

void WorldMap::initVisibilityDistance()
{
    //init visibility for continents
    m_VisibleDistance = 100 * 100;
}

void WorldMap::outOfMapBoundariesTeleport(Object* object)
{
    if (object->isPlayer())
    {
        Player* player = static_cast<Player*>(object);

        if (player->getBindMapId() != getBaseMap()->getMapId())
        {
            player->safeTeleport(player->getBindMapId(), 0, player->getBindPosition());
            player->getSession()->SystemMessage("Teleported you to your hearthstone location as you were out of the map boundaries.");
        }
        else
        {
            object->GetPositionV()->ChangeCoords(player->getBindPosition());
            player->getSession()->SystemMessage("Teleported you to your hearthstone location as you were out of the map boundaries.");
            player->sendTeleportAckPacket(player->getBindPosition());
        }
    }
    else
    {
        object->GetPositionV()->ChangeCoords({ 0, 0, 0, 0 });
    }
}

void WorldMap::removeAllPlayers()
{
    if (getPlayerCount())
    {
        for (PlayerStorageMap::iterator itr = m_PlayerStorage.begin(); itr != m_PlayerStorage.end(); ++itr)
        {
            Player* player = itr->second;

            if (!bServerShutdown)
            {
                player->ejectFromInstance();
            }
            else
            {
                if (player->getSession())
                    player->getSession()->LogoutPlayer(false);
                else
                    delete player;
            }
        }
    }
}

void WorldMap::AddObject(Object* obj)
{
    std::unique_lock<std::mutex> lock(m_objectinsertlock);
    m_objectinsertpool.insert(obj);

    if (obj->isPlayer())
        addPlayerToMap(obj->ToPlayer());
}

void WorldMap::PushObject(Object* obj)
{
    if (obj != nullptr)
    {
        //\todo That object types are not map objects. TODO: add AI groups here?
        if (obj->isItem() || obj->isContainer())
        {
            // mark object as updatable and exit
            return;
        }

        //Zyres: this was an old ASSERT MapMgr for map x is not allowed to push objects for mapId z
        if (obj->GetMapId() != getBaseMap()->getMapId())
        {
            sLogger.failure("MapMgr::PushObject manager for mapId %u tried to push object for mapId %u, return!", getBaseMap()->getMapId(), obj->GetMapId());
            return;
        }

        if (obj->GetPositionY() > Map::Terrain::_maxY || obj->GetPositionY() < Map::Terrain::_minY)
        {
            sLogger.failure("MapMgr::PushObject not allowed to push object to y: %f (max %f/min %f), return!", obj->GetPositionY(), Map::Terrain::_maxY, Map::Terrain::_minY);
            return;
        }

        if (_cells == nullptr)
        {
            sLogger.failure("MapMgr::PushObject not allowed to push object to invalid cell (nullptr), return!");
            return;
        }

        if (obj->isCorpse())
        {
            m_corpses.insert(static_cast<Corpse*>(obj));
        }

        obj->clearInRangeSets();

        // Check valid cell x/y values
        if (!(obj->GetPositionX() < Map::Terrain::_maxX && obj->GetPositionX() > Map::Terrain::_minX) || !(obj->GetPositionY() < Map::Terrain::_maxY && obj->GetPositionY() > Map::Terrain::_minY))
        {
            outOfMapBoundariesTeleport(obj);
        }

        // Get cell coordinates
        uint32_t x = getPosX(obj->GetPositionX());
        uint32_t y = getPosY(obj->GetPositionY());

        if (x >= Map::Cell::_sizeX || y >= Map::Cell::_sizeY)
        {
            outOfMapBoundariesTeleport(obj);

            x = getPosX(obj->GetPositionX());
            y = getPosY(obj->GetPositionY());
        }

        MapCell* objCell = getCell(x, y);
        if (objCell == nullptr)
        {
            objCell = create(x, y);
            if (objCell != nullptr)
            {
                objCell->init(x, y, this);
            }
            else
            {
                sLogger.fatal("MapCell for x f% and y f% seems to be invalid!", x, y);
                return;
            }
        }

        // Build update-block for player
        ByteBuffer* buf = 0;
        uint32_t count;
        Player* plObj = nullptr;

        if (obj->isPlayer())
        {
            plObj = static_cast<Player*>(obj);

            sLogger.debug("Creating player " I64FMT " for himself.", obj->getGuid());
            ByteBuffer pbuf(10000);
            count = plObj->buildCreateUpdateBlockForPlayer(&pbuf, plObj);
            plObj->getUpdateMgr().pushCreationData(&pbuf, count);
        }

        // Build in-range data
        uint8_t cellNumber = worldConfig.server.mapCellNumber;

        uint32_t endX = (x <= Map::Cell::_sizeX) ? x + cellNumber : (Map::Cell::_sizeX - cellNumber);
        uint32_t endY = (y <= Map::Cell::_sizeY) ? y + cellNumber : (Map::Cell::_sizeY - cellNumber);
        uint32_t startX = x > 0 ? x - cellNumber : 0;
        uint32_t startY = y > 0 ? y - cellNumber : 0;

        for (uint32_t posX = startX; posX <= endX; posX++)
        {
            for (uint32_t posY = startY; posY <= endY; posY++)
            {
                MapCell* cell = getCell(posX, posY);
                if (cell)
                {
                    updateInRangeSet(obj, plObj, cell, &buf);
                }
            }
        }

        // Forced Cells
        for (auto& cell : m_forcedcells)
            updateInRangeSet(obj, plObj, cell, &buf);

        //Add to the cell's object list
        objCell->addObject(obj);

        obj->SetMapCell(objCell);
        //Add to the mapmanager's object list
        if (plObj != nullptr)
        {
            m_PlayerStorage[plObj->getGuidLow()] = plObj;
            updateCellActivity(x, y, 2 + cellNumber);
        }
        else
        {
            switch (obj->GetTypeFromGUID())
            {
                case HIGHGUID_TYPE_PET:
                    m_PetStorage[obj->GetUIdFromGUID()] = static_cast<Pet*>(obj);
                    break;

                case HIGHGUID_TYPE_UNIT:
                case HIGHGUID_TYPE_VEHICLE:
                {
                    if (obj->GetUIdFromGUID() <= m_CreatureHighGuid)
                    {
                        m_CreatureStorage[obj->GetUIdFromGUID()] = static_cast<Creature*>(obj);
                        if (static_cast<Creature*>(obj)->m_spawn != nullptr)
                        {
                            _sqlids_creatures.insert(std::make_pair(static_cast<Creature*>(obj)->m_spawn->id, static_cast<Creature*>(obj)));
                        }
                    }
                } break;

                case HIGHGUID_TYPE_GAMEOBJECT:
                {
                    m_GameObjectStorage[obj->GetUIdFromGUID()] = static_cast<GameObject*>(obj);
                    if (static_cast<GameObject*>(obj)->m_spawn != nullptr)
                    {
                        _sqlids_gameobjects.insert(std::make_pair(static_cast<GameObject*>(obj)->m_spawn->id, static_cast<GameObject*>(obj)));
                    }
                } break;

                case HIGHGUID_TYPE_DYNAMICOBJECT:
                    m_DynamicObjectStorage[obj->getGuidLow()] = (DynamicObject*)obj;
                    break;
            }
        }

        // Handle activation of that object.
        if (objCell->isActive() && obj->CanActivate())
            obj->Activate(this);

        // Add the session to our set if it is a player.
        if (plObj)
        {
            Sessions.insert(plObj->getSession());

            // Change the instance ID, this will cause it to be removed from the world thread (return value 1)
            plObj->getSession()->SetInstance(getInstanceId());

            // Add the map wide objects
            if (_mapWideStaticObjects.size())
            {
                uint32 globalcount = 0;
                if (!buf)
                    buf = new ByteBuffer(300);

                for (auto _mapWideStaticObject : _mapWideStaticObjects)
                {
                    count = _mapWideStaticObject->buildCreateUpdateBlockForPlayer(buf, plObj);
                    globalcount += count;
                }
                /*VLack: It seems if we use the same buffer then it is a BAD idea to try and push created data one by one, add them at once!
                       If you try to add them one by one, then as the buffer already contains data, they'll end up repeating some object.
                       Like 6 object updates for Deeprun Tram, but the built package will contain these entries: 2AFD0, 2AFD0, 2AFD1, 2AFD0, 2AFD1, 2AFD2*/
                if (globalcount > 0)
                    plObj->getUpdateMgr().pushCreationData(buf, globalcount);
            }
        }

        delete buf;

        // todo aaron02
        /*
        if (plObj != nullptr && InactiveMoveTime && !forced_expire)
            InactiveMoveTime = 0;*/
    }
    else
    {
        sLogger.failure("MapMgr::PushObject tried to push invalid object (nullptr)!");
    }
}

void WorldMap::PushStaticObject(Object* obj)
{
    _mapWideStaticObjects.insert(obj);

    obj->SetInstanceID(getInstanceId());
    obj->SetMapId(getBaseMap()->getMapId());

    switch (obj->GetTypeFromGUID())
    {
    case HIGHGUID_TYPE_UNIT:
    case HIGHGUID_TYPE_VEHICLE:
        m_CreatureStorage[obj->GetUIdFromGUID()] = static_cast<Creature*>(obj);
        break;

    case HIGHGUID_TYPE_GAMEOBJECT:
        m_GameObjectStorage[obj->GetUIdFromGUID()] = static_cast<GameObject*>(obj);
        break;

    default:
        sLogger.debug("WorldMap::PushStaticObject called for invalid type %u.", obj->GetTypeFromGUID());
        break;
    }
}

void WorldMap::RemoveObject(Object* obj, bool free_guid)
{
    // Assertions
    if (obj == nullptr)
    {
        sLogger.failure("MapMgr::RemoveObject tried to remove invalid object (nullptr)");
        return;
    }

    if (obj->GetMapId() != getBaseMap()->getMapId())
    {
        sLogger.failure("MapMgr::RemoveObject tried to remove object with map %u but mapMgr is for map %u!", obj->GetMapId(), getBaseMap()->getMapId());
        return;
    }

    if (_cells == nullptr)
    {
        sLogger.failure("MapMgr::RemoveObject tried to remove invalid cells (nullptr)");
        return;
    }

    if (obj->IsActive())
        obj->Deactivate(this);

    //there is a very small chance that on double player ports on same update player is added to multiple insertpools but not removed
    //one clear example was the double port proc when exploiting double resurrect
    {
        std::unique_lock<std::mutex> lock(m_objectinsertlock);
        m_objectinsertpool.erase(obj);
    }

    {
        std::unique_lock<std::mutex> lock(m_updateMutex);
        _updates.erase(obj);
        obj->ClearUpdateMask();
    }

    // Remove object from all needed places
    switch (obj->GetTypeFromGUID())
    {
        case HIGHGUID_TYPE_UNIT:
        case HIGHGUID_TYPE_VEHICLE:
        {
            if (obj->GetUIdFromGUID() <= m_CreatureHighGuid)
            {
                m_CreatureStorage[obj->GetUIdFromGUID()] = nullptr;

                if (static_cast<Creature*>(obj)->m_spawn != nullptr)
                    _sqlids_creatures.erase(static_cast<Creature*>(obj)->m_spawn->id);

                if (free_guid)
                    _reusable_guids_creature.push_back(obj->GetUIdFromGUID());
            }
        } break;
        case HIGHGUID_TYPE_PET:
        {
            m_PetStorage.erase(obj->GetUIdFromGUID());
        } break;
        case HIGHGUID_TYPE_DYNAMICOBJECT:
        {
            m_DynamicObjectStorage.erase(obj->getGuidLow());
        } break;
        case HIGHGUID_TYPE_GAMEOBJECT:
        {
            if (obj->GetUIdFromGUID() <= m_GOHighGuid)
            {
                m_GameObjectStorage[obj->GetUIdFromGUID()] = nullptr;
                if (static_cast<GameObject*>(obj)->m_spawn != nullptr)
                    _sqlids_gameobjects.erase(static_cast<GameObject*>(obj)->m_spawn->id);

                if (free_guid)
                    _reusable_guids_gameobject.push_back(obj->GetUIdFromGUID());
            }
        } break;
        case HIGHGUID_TYPE_TRANSPORTER:
            break;
        default:
        {
            sLogger.debug("MapMgr::RemoveObject called for invalid type %u.", obj->GetTypeFromGUID());
            break;
        }
    }

    //\todo That object types are not map objects. TODO: add AI groups here?
    if (obj->isItem() || obj->isContainer())
    {
        return;
    }

    if (obj->isCorpse())
    {
        m_corpses.erase(static_cast<Corpse*>(obj));
    }

    MapCell* cell = getCell(obj->GetMapCellX(), obj->GetMapCellY());
    if (cell == nullptr)
    {
        // set the map cell correctly
        if (obj->GetPositionX() < Map::Terrain::_maxX || obj->GetPositionX() > Map::Terrain::_minY || obj->GetPositionY() < Map::Terrain::_maxY || obj->GetPositionY() > Map::Terrain::_minY)
        {
            cell = this->getCellByCoords(obj->GetPositionX(), obj->GetPositionY());
            obj->SetMapCell(cell);
        }
    }

    if (cell != nullptr)
    {
        cell->removeObject(obj);        // Remove object from cell
        obj->SetMapCell(nullptr);          // Unset object's cell
    }

    Player* plObj = nullptr;
    if (obj->isPlayer())
    {
        plObj = static_cast<Player*>(obj);

        _processQueue.erase(plObj);     // Clear any updates pending
        plObj->getUpdateMgr().clearPendingUpdates();

        removePlayerFromMap(plObj);
    }

    obj->removeSelfFromInrangeSets();
    obj->clearInRangeSets();             // Clear object's in-range set

    uint8_t cellNumber = worldConfig.server.mapCellNumber;

    // If it's a player - update his nearby cells
    if (obj->isPlayer())
    {// get x/y
        if (obj->GetPositionX() < Map::Terrain::_maxX || obj->GetPositionX() > Map::Terrain::_minY || obj->GetPositionY() < Map::Terrain::_maxY || obj->GetPositionY() > Map::Terrain::_minY)
        {
            uint32_t x = getPosX(obj->GetPositionX());
            uint32_t y = getPosY(obj->GetPositionY());
            updateCellActivity(x, y, 2 + cellNumber);
        }
        m_PlayerStorage.erase(obj->getGuidLow());
    }
    else if (obj->isCreatureOrPlayer() && static_cast<Unit*>(obj)->mPlayerControler != nullptr)
    {
        if (obj->GetPositionX() < Map::Terrain::_maxX || obj->GetPositionX() > Map::Terrain::_minY || obj->GetPositionY() < Map::Terrain::_maxY || obj->GetPositionY() > Map::Terrain::_minY)
        {
            uint32_t x = getPosX(obj->GetPositionX());
            uint32_t y = getPosY(obj->GetPositionY());
            updateCellActivity(x, y, 2 + cellNumber);
        }
    }

    // Remove the session from our set if it is a player.
    if (obj->isPlayer() || obj->isCreatureOrPlayer() && static_cast<Unit*>(obj)->mPlayerControler != nullptr)
    {
        for (auto _mapWideStaticObject : _mapWideStaticObjects)
        {
            if (_mapWideStaticObject != nullptr && plObj)
                plObj->getUpdateMgr().pushOutOfRangeGuid(_mapWideStaticObject->GetNewGUID());
        }

        // Setting an instance ID here will trigger the session to be removed by MapMgr::run(). :)
        if (plObj && plObj->getSession())
        {
            plObj->getSession()->SetInstance(0);

            // Add it to the global session set. Don't "re-add" to session if it is being deleted.
            if (!plObj->getSession()->bDeleted)
                sWorld.addGlobalSession(plObj->getSession());
        }
    }
}

void WorldMap::addForcedCell(MapCell* c)
{
    uint8_t cellNumber = worldConfig.server.mapCellNumber;

    m_forcedcells.insert(c);
    updateCellActivity(c->getPositionX(), c->getPositionY(), cellNumber);
}

void WorldMap::removeForcedCell(MapCell* c)
{
    uint8_t cellNumber = worldConfig.server.mapCellNumber;

    m_forcedcells.erase(c);
    updateCellActivity(c->getPositionX(), c->getPositionY(), cellNumber);
}

void WorldMap::addForcedCell(MapCell* c, uint32_t range)
{
    m_forcedcells.insert(c);
    updateCellActivity(c->getPositionX(), c->getPositionY(), range);
}

void WorldMap::removeForcedCell(MapCell* c, uint32_t range)
{
    m_forcedcells.erase(c);
    updateCellActivity(c->getPositionX(), c->getPositionY(), range);
}

bool WorldMap::cellHasAreaID(uint32_t CellX, uint32_t CellY, uint16_t& AreaID)
{
    int32_t TileX = CellX / 8;
    int32_t TileY = CellY / 8;

    if (!getTerrain()->areTilesValid(TileX, TileY))
        return false;

    int32_t OffsetTileX = TileX - getTerrain()->TileStartX;
    int32_t OffsetTileY = TileY - getTerrain()->TileStartY;

    bool Required = false;
    bool Result = false;

    if (!getTerrain()->tileLoaded(OffsetTileX, OffsetTileY))
        Required = true;

    if (Required)
    {
        getTerrain()->loadTile(TileX, TileY);
        getTerrain()->loadTile(TileX, TileY);
        return Result;
    }

    for (uint32_t xc = (CellX % Map::Cell::CellsPerTile) * 16 / Map::Cell::CellsPerTile; xc < (CellX % Map::Cell::CellsPerTile) * 16 / Map::Cell::CellsPerTile + 16 / Map::Cell::CellsPerTile; xc++)
    {
        for (uint32_t yc = (CellY % Map::Cell::CellsPerTile) * 16 / Map::Cell::CellsPerTile; yc < (CellY % Map::Cell::CellsPerTile) * 16 / Map::Cell::CellsPerTile + 16 / Map::Cell::CellsPerTile; yc++)
        {
            const auto areaid = getTerrain()->getTile(OffsetTileX, OffsetTileY)->m_map.m_areaMap[yc * 16 + xc];
            if (areaid)
            {
                AreaID = areaid;
                Result = true;
                break;
            }
        }
    }

    if (Required)
        getTerrain()->unloadTile(TileX, TileY);

    return Result;
}

void WorldMap::updateAllCells(bool apply, uint32_t areamask)
{
    uint16_t AreaID = 0;
    MapCell* cellInfo;
    CellSpawns* spawns;
    uint32_t StartX = 0, EndX = 0, StartY = 0, EndY = 0;
    getTerrain()->getCellLimits(StartX, EndX, StartY, EndY);

    if (!areamask)
        sLogger.debugFlag(AscEmu::Logging::LF_MAP_CELL, "Updating all cells for map %03u, server might lag.", getBaseMap()->getMapId());

    for (uint32_t x = StartX; x < EndX; x++)
    {
        for (uint32_t y = StartY; y < EndY; y++)
        {
            if (areamask)
            {
                if (!cellHasAreaID(x, y, AreaID))
                    continue;

                auto at = sAreaStore.LookupEntry(AreaID);
                if (at == nullptr)
                    continue;
                if (at->zone != areamask)
                    if (at->id != areamask)
                        continue;
                AreaID = 0;
            }

            cellInfo = getCell(x, y);
            if (apply)
            {
                if (!cellInfo)
                {   // Cell doesn't exist, create it.
                    cellInfo = create(x, y);
                    cellInfo->init(x, y, this);
                    sLogger.debugFlag(AscEmu::Logging::LF_MAP_CELL, "Created cell [%u,%u] on map %u (instance %u).", x, y, getBaseMap()->getMapId(), getInstanceId());
                }

                spawns = _map->getSpawnsList(x, y);
                if (spawns)
                {
                    addForcedCell(cellInfo, 1);
                }
            }
            else
            {
                if (!cellInfo)
                    continue;

                removeForcedCell(cellInfo, 1);
            }
        }
    }

    if (!areamask)
        sLogger.debugFlag(AscEmu::Logging::LF_MAP_CELL, "Cell updating success for map %03u", getBaseMap()->getMapId());
}

void WorldMap::updateAllCells(bool apply)
{
    if (!getTerrain())
        return;

    MapCell* cellInfo;
    CellSpawns* spawns;
    uint32_t StartX = 0, EndX = 0, StartY = 0, EndY = 0;
    getTerrain()->getCellLimits(StartX, EndX, StartY, EndY);

    for (uint32_t x = StartX; x < EndX; x++)
    {
        for (uint32_t y = StartY; y < EndY; y++)
        {
            cellInfo = getCell(x, y);
            if (apply)
            {
                if (!cellInfo)
                {   // Cell doesn't exist, create it.
                    cellInfo = create(x, y);
                    cellInfo->init(x, y, this);
                    sLogger.debugFlag(AscEmu::Logging::LF_MAP_CELL, "Created cell [%u,%u] on map %u (instance %u).", x, y, getBaseMap()->getMapId(), getInstanceId());
                }

                spawns = _map->getSpawnsList(cellInfo->getPositionX(), cellInfo->getPositionY());
                if (spawns)
                {
                    addForcedCell(cellInfo, 1);
                }
            }
            else
            {
                if (!cellInfo)
                    continue;

                removeForcedCell(cellInfo, 1);
            }
        }
    }
    sLogger.debugFlag(AscEmu::Logging::LF_MAP_CELL, "Cell updating success for map %03u", getBaseMap()->getMapId());
}

void WorldMap::updateCellActivity(uint32_t x, uint32_t y, uint32_t radius)
{
    CellSpawns* sp;
    uint32_t endX = (x + radius) <= Map::Cell::_sizeX ? x + radius : (Map::Cell::_sizeX - 1);
    uint32_t endY = (y + radius) <= Map::Cell::_sizeY ? y + radius : (Map::Cell::_sizeY - 1);
    uint32_t startX = x > radius ? x - radius : 0;
    uint32_t startY = y > radius ? y - radius : 0;

    for (uint32_t posX = startX; posX <= endX; posX++)
    {
        for (uint32_t posY = startY; posY <= endY; posY++)
        {
            MapCell* objCell = getCell(posX, posY);
            if (objCell == nullptr)
            {
                if (isCellActive(posX, posY))
                {
                    objCell = create(posX, posY);
                    objCell->init(posX, posY, this);

                    sLogger.debug("MapMgr : Cell [%u,%u] on map %u (instance %u) is now active.", posX, posY, getBaseMap()->getMapId(), getInstanceId());
                    objCell->setActivity(true);

                    getTerrain()->loadTile((int32)posX / 8, (int32)posY / 8);

                    if (!objCell->isLoaded())
                    {
                        sLogger.debug("MapMgr : Loading objects for Cell [%u][%u] on map %u (instance %u)...", posX, posY, getBaseMap()->getMapId(), getInstanceId());

                        sp = _map->getSpawnsList(posX, posY);
                        if (sp)
                            objCell->loadObjects(sp);
                    }
                }
            }
            else
            {
                //Cell is now active
                if (isCellActive(posX, posY) && (!objCell->isActive() || (objCell->isActive() && objCell->isIdlePending())))
                {
                    if (objCell->isIdlePending())
                        objCell->cancelPendingIdle();

                    sLogger.debug("Cell [%u,%u] on map %u (instance %u) is now active.", posX, posY, getBaseMap()->getMapId(), getInstanceId());

                    getTerrain()->loadTile((int32)posX / 8, (int32)posY / 8);
                    objCell->setActivity(true);

                    if (!objCell->isLoaded())
                    {
                        sLogger.debug("Loading objects for Cell [%u][%u] on map %u (instance %u)...", posX, posY, getBaseMap()->getMapId(), getInstanceId());
                        sp = _map->getSpawnsList(posX, posY);
                        if (sp)
                            objCell->loadObjects(sp);
                    }
                }
                //Cell is no longer active
                else if (!isCellActive(posX, posY) && objCell->isActive() && !objCell->isIdlePending())
                {
                    objCell->scheduleCellIdleState();
                }
            }
        }
    }
}

void WorldMap::setCellIdle(uint16_t x, uint16_t y, MapCell* cell)
{
    sLogger.debug("Cell [%u,%u] on map %u (instance %u) is now idle.", x, y, getBaseMap()->getMapId(), getInstanceId());
    cell->setActivity(false);

    _terrain->unloadTile(static_cast<int32_t>(x) / 8, static_cast<int32_t>(y) / 8);
}

void WorldMap::unloadCell(uint32_t x, uint32_t y)
{
    MapCell* c = getCell(x, y);
    if (c == nullptr || isCellActive(x, y) || !c->isUnloadPending())
        return;

    sLogger.debug("Unloading Cell [%u][%u] on map %u (instance %u)...", x, y, getBaseMap()->getMapId(), getInstanceId());

    c->unload();
}

bool WorldMap::isCellActive(uint32_t x, uint32_t y)
{
    uint8_t cellNumber = worldConfig.server.mapCellNumber;

    uint32_t endX = ((x + cellNumber) <= Map::Cell::_sizeX) ? x + cellNumber : (Map::Cell::_sizeX - cellNumber);
    uint32_t endY = ((y + cellNumber) <= Map::Cell::_sizeY) ? y + cellNumber : (Map::Cell::_sizeY - cellNumber);
    uint32_t startX = x > 0 ? x - cellNumber : 0;
    uint32_t startY = y > 0 ? y - cellNumber : 0;

    for (uint32_t posX = startX; posX <= endX; posX++)
    {
        for (uint32_t posY = startY; posY <= endY; posY++)
        {
            MapCell* objCell = getCell(posX, posY);
            if (objCell != nullptr)
            {
                if (objCell->hasPlayers() || objCell->hasTransporters() || m_forcedcells.find(objCell) != m_forcedcells.end())
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void WorldMap::updateInRangeSet(Object* obj, Player* plObj, MapCell* cell, ByteBuffer** buf)
{
    if (cell == nullptr)
        return;

    Player* plObj2;
    int count;
    bool cansee, isvisible;

    auto iter = cell->Begin();
    while (iter != cell->End())
    {
        // Prevent undefined behaviour (related to transports) -Appled
        if (cell->_objects.empty())
            break;

        Object* curObj = *iter;
        ++iter;

        if (curObj == nullptr)
            continue;

        float fRange = getUpdateDistance(curObj, obj, plObj);

        if (curObj != obj && (curObj->GetDistance2dSq(obj) <= fRange || fRange == 0.0f))
        {
            if (!obj->isObjectInInRangeObjectsSet(curObj))
            {
                obj->addToInRangeObjects(curObj);          // Object in range, add to set
                curObj->addToInRangeObjects(obj);

                if (curObj->isPlayer())
                {
                    plObj2 = static_cast<Player*>(curObj);

                    if (plObj2->canSee(obj) && !plObj2->isVisibleObject(obj->getGuid()))
                    {
                        if (!*buf)
                            *buf = new ByteBuffer(2500);

                        count = obj->buildCreateUpdateBlockForPlayer(*buf, plObj2);
                        plObj2->getUpdateMgr().pushCreationData(*buf, count);
                        plObj2->addVisibleObject(obj->getGuid());
                        (*buf)->clear();
                    }
                }
                else if (curObj->isCreatureOrPlayer() && static_cast<Unit*>(curObj)->mPlayerControler != nullptr)
                {
                    plObj2 = static_cast<Unit*>(curObj)->mPlayerControler;

                    if (plObj2->canSee(obj) && !plObj2->isVisibleObject(obj->getGuid()))
                    {
                        if (!*buf)
                            *buf = new ByteBuffer(2500);

                        count = obj->buildCreateUpdateBlockForPlayer(*buf, plObj2);
                        plObj2->getUpdateMgr().pushCreationData(*buf, count);
                        plObj2->addVisibleObject(obj->getGuid());
                        (*buf)->clear();
                    }
                }

                if (plObj != nullptr)
                {
                    if (plObj->canSee(curObj) && !plObj->isVisibleObject(curObj->getGuid()))
                    {
                        if (!*buf)
                            *buf = new ByteBuffer(2500);

                        count = curObj->buildCreateUpdateBlockForPlayer(*buf, plObj);
                        plObj->getUpdateMgr().pushCreationData(*buf, count);
                        plObj->addVisibleObject(curObj->getGuid());
                        (*buf)->clear();
                    }
                }
            }
            else
            {
                // Check visibility
                if (curObj->isPlayer())
                {
                    plObj2 = static_cast<Player*>(curObj);
                    cansee = plObj2->canSee(obj);
                    isvisible = plObj2->isVisibleObject(obj->getGuid());
                    if (!cansee && isvisible)
                    {
                        plObj2->getUpdateMgr().pushOutOfRangeGuid(obj->GetNewGUID());
                        plObj2->removeVisibleObject(obj->getGuid());
                    }
                    else if (cansee && !isvisible)
                    {
                        if (!*buf)
                            *buf = new ByteBuffer(2500);

                        count = obj->buildCreateUpdateBlockForPlayer(*buf, plObj2);
                        plObj2->getUpdateMgr().pushCreationData(*buf, count);
                        plObj2->addVisibleObject(obj->getGuid());
                        (*buf)->clear();
                    }
                }
                else if (curObj->isCreatureOrPlayer() && static_cast<Unit*>(curObj)->mPlayerControler != nullptr)
                {
                    plObj2 = static_cast<Unit*>(curObj)->mPlayerControler;
                    cansee = plObj2->canSee(obj);
                    isvisible = plObj2->isVisibleObject(obj->getGuid());
                    if (!cansee && isvisible)
                    {
                        plObj2->getUpdateMgr().pushOutOfRangeGuid(obj->GetNewGUID());
                        plObj2->removeVisibleObject(obj->getGuid());
                    }
                    else if (cansee && !isvisible)
                    {
                        if (!*buf)
                            *buf = new ByteBuffer(2500);

                        count = obj->buildCreateUpdateBlockForPlayer(*buf, plObj2);
                        plObj2->getUpdateMgr().pushCreationData(*buf, count);
                        plObj2->addVisibleObject(obj->getGuid());
                        (*buf)->clear();
                    }
                }

                if (plObj != nullptr)
                {
                    cansee = plObj->canSee(curObj);
                    isvisible = plObj->isVisibleObject(curObj->getGuid());
                    if (!cansee && isvisible)
                    {
                        plObj->getUpdateMgr().pushOutOfRangeGuid(curObj->GetNewGUID());
                        plObj->removeVisibleObject(curObj->getGuid());
                    }
                    else if (cansee && !isvisible)
                    {
                        if (!*buf)
                            *buf = new ByteBuffer(2500);

                        count = curObj->buildCreateUpdateBlockForPlayer(*buf, plObj);
                        plObj->getUpdateMgr().pushCreationData(*buf, count);
                        plObj->addVisibleObject(curObj->getGuid());
                        (*buf)->clear();
                    }
                }
            }
        }
    }
}

void WorldMap::changeObjectLocation(Object* obj)
{
    if (obj == nullptr)
        return;

    // Items and containers are of no interest for us
    if (obj->isItem() || obj->isContainer() || obj->getWorldMap() != this)
        return;

    Player* plObj = nullptr;
    ByteBuffer* buf = nullptr;

    if (obj->isPlayer())
        plObj = static_cast<Player*>(obj);
    if (obj->isCreatureOrPlayer() && static_cast<Unit*>(obj)->mPlayerControler != nullptr)
        plObj = static_cast<Unit*>(obj)->mPlayerControler;

    float fRange = 0.0f;

    if (obj->isGameObject())
        obj->ToGameObject()->updateModelPosition();

    // Update in-range data for old objects
    if (obj->hasInRangeObjects())
    {
        for (const auto& iter : obj->getInRangeObjectsSet())
        {
            if (iter)
            {
                Object* curObj = iter;

                fRange = getUpdateDistance(curObj, obj, plObj);

                if (fRange > 0.0f && (curObj->GetDistance2dSq(obj) > fRange))
                {
                    if (plObj != nullptr)
                        plObj->removeIfVisiblePushOutOfRange(curObj->getGuid());

                    if (curObj->isPlayer())
                        static_cast<Player*>(curObj)->removeIfVisiblePushOutOfRange(obj->getGuid());

                    if (curObj->isCreatureOrPlayer() && static_cast<Unit*>(curObj)->mPlayerControler != nullptr)
                        static_cast<Unit*>(curObj)->mPlayerControler->removeIfVisiblePushOutOfRange(obj->getGuid());

                    curObj->removeObjectFromInRangeObjectsSet(obj);

                    if (obj->getWorldMap() != this)
                        return;             //Something removed us.

                    obj->removeObjectFromInRangeObjectsSet(curObj);
                }
            }
        }
    }

    // Get new cell coordinates
    if (obj->getWorldMap() != this)
    {
        return;                 //Something removed us.
    }

    if (obj->GetPositionX() >= Map::Terrain::_maxX || obj->GetPositionX() <= Map::Terrain::_minX || obj->GetPositionY() >= Map::Terrain::_maxY || obj->GetPositionY() <= Map::Terrain::_minY)
    {
        outOfMapBoundariesTeleport(obj);
    }

    uint32_t cellX = getPosX(obj->GetPositionX());
    uint32_t cellY = getPosY(obj->GetPositionY());

    if (cellX >= Map::Cell::_sizeX || cellY >= Map::Cell::_sizeY)
    {
        return;
    }

    MapCell* objCell = getCell(cellX, cellY);
    MapCell* pOldCell = obj->GetMapCell();
    if (objCell == nullptr)
    {
        objCell = create(cellX, cellY);
        if (objCell != nullptr)
        {
            objCell->init(cellX, cellY, this);
        }
        else
        {
            sLogger.failure("MapMgr::ChangeObjectLocation not able to create object cell (nullptr), return!");
            return;
        }
    }
    uint8_t cellNumber = worldConfig.server.mapCellNumber;

    // If object moved cell
    if (objCell != pOldCell)
    {
        // THIS IS A HACK!
        // Current code, if a creature on a long waypoint path moves from an active
        // cell into an inactive one, it will disable itself and will never return.
        // This is to prevent cpu leaks. I will think of a better solution very soon :P

        if (!objCell->isActive() && !plObj && obj->IsActive())
            obj->Deactivate(this);

        if (pOldCell != nullptr)
            pOldCell->removeObject(obj);

        objCell->addObject(obj);
        obj->SetMapCell(objCell);

        // if player we need to update cell activity radius = 2 is used in order to update
        // both old and new cells
        if (obj->isPlayer() || obj->isCreatureOrPlayer() && static_cast<Unit*>(obj)->mPlayerControler != nullptr)
        {
            // have to unlock/lock here to avoid a deadlock situation.
            updateCellActivity(cellX, cellY, 2 + cellNumber);
            if (pOldCell != NULL)
            {
                // only do the second check if there's -/+ 2 difference
                if (abs((int)cellX - (int)pOldCell->_x) > 2 + cellNumber ||
                    abs((int)cellY - (int)pOldCell->_y) > 2 + cellNumber)
                {
                    updateCellActivity(pOldCell->_x, pOldCell->_y, cellNumber);
                }
            }
        }
    }

    // Update in-range set for new objects
    uint32_t endX = cellX + cellNumber;
    uint32_t endY = cellY + cellNumber;
    uint32_t startX = cellX > 0 ? cellX - cellNumber : 0;
    uint32_t startY = cellY > 0 ? cellY - cellNumber : 0;

    //If the object announcing it's position is a special one, then it should do so in a much wider area - like the distance between the two transport towers in Orgrimmar, or more. - By: VLack
    if (obj->isGameObject() && (static_cast<GameObject*>(obj)->GetOverrides() & GAMEOBJECT_ONMOVEWIDE))
    {
        endX = cellX + 5 <= Map::Cell::_sizeX ? cellX + 6 : (Map::Cell::_sizeX - 1);
        endY = cellY + 5 <= Map::Cell::_sizeY ? cellY + 6 : (Map::Cell::_sizeY - 1);
        startX = cellX > 5 ? cellX - 6 : 0;
        startY = cellY > 5 ? cellY - 6 : 0;
    }

    for (uint32_t posX = startX; posX <= endX; ++posX)
    {
        for (uint32_t posY = startY; posY <= endY; ++posY)
        {
            MapCell* cell = getCell(posX, posY);
            if (cell)
                updateInRangeSet(obj, plObj, cell, &buf);
        }
    }

    if (buf)
        delete buf;
}

void WorldMap::changeFarsightLocation(Player* plr, DynamicObject* farsight)
{
    uint8_t cellNumber = worldConfig.server.mapCellNumber;

    if (farsight == 0)
    {
        // We're clearing.
        for (auto itr = plr->m_visibleFarsightObjects.begin(); itr != plr->m_visibleFarsightObjects.end(); ++itr)
        {
            if (plr->isVisibleObject((*itr)->getGuid()) && !plr->canSee((*itr)))
                plr->getUpdateMgr().pushOutOfRangeGuid((*itr)->GetNewGUID());      // Send destroy
        }

        plr->m_visibleFarsightObjects.clear();
    }
    else
    {
        uint32_t cellX = getPosX(farsight->GetPositionX());
        uint32_t cellY = getPosY(farsight->GetPositionY());
        uint32_t endX = (cellX <= Map::Cell::_sizeX) ? cellX + cellNumber : (Map::Cell::_sizeX - cellNumber);
        uint32_t endY = (cellY <= Map::Cell::_sizeY) ? cellY + cellNumber : (Map::Cell::_sizeY - cellNumber);
        uint32_t startX = cellX > 0 ? cellX - cellNumber : 0;
        uint32_t startY = cellY > 0 ? cellY - cellNumber : 0;

        for (uint32_t posX = startX; posX <= endX; ++posX)
        {
            for (uint32_t posY = startY; posY <= endY; ++posY)
            {
                MapCell* cell = getCell(posX, posY);
                if (cell != nullptr)
                {
                    for (auto iter = cell->Begin(); iter != cell->End(); ++iter)
                    {
                        Object* obj = (*iter);
                        if (obj == nullptr)
                            continue;

                        if (!plr->isVisibleObject(obj->getGuid()) && plr->canSee(obj) && farsight->GetDistance2dSq(obj) <= getVisibilityRange())
                        {
                            ByteBuffer buf;
                            uint32_t count = obj->buildCreateUpdateBlockForPlayer(&buf, plr);
                            plr->getUpdateMgr().pushCreationData(&buf, count);
                            plr->m_visibleFarsightObjects.insert(obj);
                        }
                    }
                }
            }
        }
    }
}

Player* WorldMap::getPlayer(uint32_t guid)
{
    auto itr = m_PlayerStorage.find(guid);
    return itr != m_PlayerStorage.end() ? itr->second : nullptr;
}

uint32_t WorldMap::getPlayerCount()
{
    return static_cast<uint32_t>(m_PlayerStorage.size());
}

bool WorldMap::hasPlayers()
{
    return (m_PlayerStorage.size() > 0);
}

uint64_t WorldMap::generateCreatureGuid(uint32_t entry, bool canUseOldGuid/* = true*/)
{
    uint64_t newguid = 0;

    CreatureProperties const* creature_properties = sMySQLStore.getCreatureProperties(entry);
    if (creature_properties == nullptr || creature_properties->vehicleid == 0)
        newguid = static_cast<uint64_t>(HIGHGUID_TYPE_UNIT) << 32;
    else
        newguid = static_cast<uint64_t>(HIGHGUID_TYPE_VEHICLE) << 32;

    char* pHighGuid = reinterpret_cast<char*>(&newguid);
    char* pEntry = reinterpret_cast<char*>(&entry);

    pHighGuid[3] |= pEntry[0];
    pHighGuid[4] |= pEntry[1];
    pHighGuid[5] |= pEntry[2];
    pHighGuid[6] |= pEntry[3];

    uint32_t guid = 0;

    if (!_reusable_guids_creature.empty() && canUseOldGuid)
    {
        guid = _reusable_guids_creature.front();
        _reusable_guids_creature.pop_front();

    }
    else
    {
        m_CreatureHighGuid++;

        if (m_CreatureHighGuid >= m_CreatureStorage.size())
        {
            // Reallocate array with larger size.
            size_t newsize = m_CreatureStorage.size() + RESERVE_EXPAND_SIZE;
            m_CreatureStorage.resize(newsize, NULL);
        }
        guid = m_CreatureHighGuid;
    }

    newguid |= guid;

    return newguid;
}

Creature* WorldMap::createCreature(uint32_t entry)
{
    uint64_t guid = generateCreatureGuid(entry);
    return new Creature(guid);
}

Creature* WorldMap::createAndSpawnCreature(uint32_t pEntry, float pX, float pY, float pZ, float pO)
{
    auto* creature = createCreature(pEntry);
    const auto* cp = sMySQLStore.getCreatureProperties(pEntry);
    if (cp == nullptr)
    {
        delete creature;
        return nullptr;
    }

    creature->Load(cp, pX, pY, pZ, pO);
    creature->AddToWorld(this);
    return creature;
}

Creature* WorldMap::getCreature(uint32_t guid)
{
    if (guid > m_CreatureHighGuid)
        return nullptr;

    return m_CreatureStorage[guid];
}

Creature* WorldMap::getSqlIdCreature(uint32_t sqlid)
{
    auto itr = _sqlids_creatures.find(sqlid);
    return itr == _sqlids_creatures.end() ? nullptr : itr->second;
}

Pet* WorldMap::getPet(uint32_t guid)
{
    auto itr = m_PetStorage.find(guid);
    return itr != m_PetStorage.end() ? itr->second : nullptr;
}

Summon* WorldMap::createSummon(uint32_t entry, SummonType type, uint32_t duration)
{
    // Generate always a new guid for totems, otherwise the totem timer bar will get messed up
    uint64_t guid = generateCreatureGuid(entry, type != SUMMONTYPE_TOTEM);

    return sObjectMgr.createSummonByGuid(guid, type, duration);
}

GameObject* WorldMap::createGameObject(uint32_t entry)
{
    uint32_t GUID = 0;

    if (_reusable_guids_gameobject.size() > GO_GUID_RECYCLE_INTERVAL)
    {
        uint32_t guid = _reusable_guids_gameobject.front();
        _reusable_guids_gameobject.pop_front();

        GUID = guid;
    }
    else
    {
        if (++m_GOHighGuid >= m_GameObjectStorage.size())
        {
            // Reallocate array with larger size.
            size_t newsize = m_GameObjectStorage.size() + RESERVE_EXPAND_SIZE;
            m_GameObjectStorage.resize(newsize, NULL);
        }

        GUID = m_GOHighGuid;
    }

    GameObject* gameobject = sObjectMgr.createGameObjectByGuid(entry, GUID);
    if (gameobject == nullptr)
        return nullptr;

    return gameobject;
}

GameObject* WorldMap::createAndSpawnGameObject(uint32_t entryID, float x, float y, float z, float o, float scale)
{
    auto gameobject_info = sMySQLStore.getGameObjectProperties(entryID);
    if (gameobject_info == nullptr)
    {
        sLogger.debug("Error looking up entry in CreateAndSpawnGameObject");
        return nullptr;
    }

    sLogger.debug("CreateAndSpawnGameObject: By Entry '%u'", entryID);

    GameObject* go = createGameObject(entryID);

    uint32_t mapid = getBaseMap()->getMapId();
    // Setup game object
    go->CreateFromProto(entryID, mapid, x, y, z, o);
    go->setScale(scale);
    go->InitAI();
    go->PushToWorld(this);

    // Create spawn instance
    auto go_spawn = new MySQLStructure::GameobjectSpawn;
    go_spawn->entry = go->getEntry();
    go_spawn->id = sObjectMgr.GenerateGameObjectSpawnID();
    go_spawn->map = go->GetMapId();
    go_spawn->position_x = go->GetPositionX();
    go_spawn->position_y = go->GetPositionY();
    go_spawn->position_z = go->GetPositionZ();
    go_spawn->orientation = go->GetOrientation();
    go_spawn->rotation_0 = go->getParentRotation(0);
    go_spawn->rotation_1 = go->getParentRotation(1);
    go_spawn->rotation_2 = go->getParentRotation(2);
    go_spawn->rotation_3 = go->getParentRotation(3);
    go_spawn->state = go->getState();
    go_spawn->flags = go->getFlags();
    go_spawn->faction = go->getFactionTemplate();
    go_spawn->scale = go->getScale();
    //go_spawn->stateNpcLink = 0;
    go_spawn->phase = go->GetPhase();
    go_spawn->overrides = go->GetOverrides();

    uint32_t cx = getPosX(x);
    uint32_t cy = getPosY(y);

    getBaseMap()->getSpawnsListAndCreate(cx, cy)->GameobjectSpawns.push_back(go_spawn);
    go->m_spawn = go_spawn;

    MapCell* mCell = getCell(cx, cy);

    if (mCell != nullptr)
        mCell->setLoaded();

    return go;
}

GameObject* WorldMap::getGameObject(uint32_t guid)
{
    if (guid > m_GOHighGuid)
        return nullptr;

    return m_GameObjectStorage[guid];
}

GameObject* WorldMap::getSqlIdGameObject(uint32_t sqlid)
{
    auto itr = _sqlids_gameobjects.find(sqlid);
    return itr == _sqlids_gameobjects.end() ? nullptr : itr->second;
}

DynamicObject* WorldMap::createDynamicObject()
{
    return new DynamicObject(HIGHGUID_TYPE_DYNAMICOBJECT, (++m_DynamicObjectHighGuid));
}

DynamicObject* WorldMap::getDynamicObject(uint32_t guid)
{
    auto itr = m_DynamicObjectStorage.find(guid);
    return itr != m_DynamicObjectStorage.end() ? itr->second : nullptr;
}

Unit* WorldMap::getUnit(const uint64_t& guid)
{
    if (guid == 0)
        return nullptr;

    WoWGuid wowGuid;
    wowGuid.Init(guid);

    switch (wowGuid.getHigh())
    {
    case HighGuid::Unit:
    case HighGuid::Vehicle:
        return getCreature(wowGuid.getGuidLowPart());
    case HighGuid::Player:
        return getPlayer(wowGuid.getGuidLowPart());
    case HighGuid::Pet:
        return getPet(wowGuid.getGuidLowPart());
    }

    return nullptr;
}

Object* WorldMap::getObject(const uint64_t& guid)
{
    if (!guid)
        return nullptr;

    WoWGuid wowGuid;
    wowGuid.Init(guid);

    switch (wowGuid.getHigh())
    {
    case HighGuid::GameObject:
        return getGameObject(wowGuid.getGuidLowPart());
    case HighGuid::Unit:
    case HighGuid::Vehicle:
        return getCreature(wowGuid.getGuidLowPart());
    case HighGuid::DynamicObject:
        return getDynamicObject(wowGuid.getGuidLowPart());
    case HighGuid::Transporter:
        return sTransportHandler.getTransporter(wowGuid.getGuidLowPart());
    default:
        return getUnit(guid);
    }
}

void WorldMap::sendChatMessageToCellPlayers(Object* obj, WorldPacket* packet, uint32_t cell_radius, uint32_t langpos, int32_t lang, WorldSession* originator)
{
    uint32_t cellX = getPosX(obj->GetPositionX());
    uint32_t cellY = getPosY(obj->GetPositionY());
    uint32_t endX = ((cellX + cell_radius) <= Map::Cell::_sizeX) ? cellX + cell_radius : (Map::Cell::_sizeX - 1);
    uint32_t endY = ((cellY + cell_radius) <= Map::Cell::_sizeY) ? cellY + cell_radius : (Map::Cell::_sizeY - 1);
    uint32_t startX = cellX > cell_radius ? cellX - cell_radius : 0;
    uint32_t startY = cellY > cell_radius ? cellY - cell_radius : 0;

    MapCell::ObjectSet::iterator iter, iend;
    for (uint32_t posX = startX; posX <= endX; ++posX)
    {
        for (uint32_t posY = startY; posY <= endY; ++posY)
        {
            MapCell* cell = getCell(posX, posY);
            if (cell && cell->hasPlayers())
            {
                iter = cell->Begin();
                iend = cell->End();
                for (; iter != iend; ++iter)
                {
                    if ((*iter)->isPlayer())
                    {
                        //TO< Player* >(*iter)->getSession()->SendPacket(packet);
                        if ((*iter)->GetPhase() & obj->GetPhase())
                            static_cast<Player*>(*iter)->getSession()->SendChatPacket(packet, langpos, lang, originator);
                    }
                }
            }
        }
    }
}

void WorldMap::sendPvPCaptureMessage(int32_t ZoneMask, uint32_t ZoneId, const char* Message, ...)
{
    va_list ap;
    va_start(ap, Message);

    char msgbuf[200];
    vsnprintf(msgbuf, 200, Message, ap);
    va_end(ap);

    for (auto itr = m_PlayerStorage.begin(); itr != m_PlayerStorage.end();)
    {
        Player* plr = itr->second;
        ++itr;

        if ((ZoneMask != ZONE_MASK_ALL && plr->GetZoneId() != (uint32_t)ZoneMask))
            continue;

        plr->getSession()->SendPacket(SmsgDefenseMessage(ZoneId, msgbuf).serialise().get());
    }
}

void WorldMap::sendPacketToAllPlayers(WorldPacket* packet) const
{
    for (const auto& itr : m_PlayerStorage)
    {
        Player* p = itr.second;

        if (p->getSession() != nullptr)
            p->getSession()->SendPacket(packet);
    }
}

void WorldMap::sendPacketToPlayersInZone(uint32_t zone, WorldPacket* packet) const
{
    for (const auto& itr : m_PlayerStorage)
    {
        Player* p = itr.second;

        if ((p->getSession() != nullptr) && (p->GetZoneId() == zone))
            p->getSession()->SendPacket(packet);
    }
}

InstanceScript* WorldMap::getScript()
{
    return mInstanceScript;
}

void WorldMap::loadInstanceScript()
{
    mInstanceScript = sScriptMgr.CreateScriptClassForInstance(getBaseMap()->getMapId(), this);
};

void WorldMap::callScriptUpdate()
{
    if (mInstanceScript != nullptr)
    {
        mInstanceScript->UpdateEvent();
        mInstanceScript->updateTimers();
    }
    else
    {
        sLogger.failure("WorldMap::callScriptUpdate tries to call without valid instance script (nullptr)");
    }
};

void WorldMap::loadRespawnTimes()
{
    // Load Saved Respawns
    QueryResult* result = CharacterDatabase.Query("SELECT type, spawnId, respawnTime FROM respawn WHERE mapId = %u AND instanceId = %u", getBaseMap()->getMapId(), getInstanceId());
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        SpawnObjectType type = SpawnObjectType(fields[0].GetUInt16());
        uint32_t spawnId = fields[1].GetUInt32();
        uint64_t respawnTime = fields[2].GetUInt64();

        if (type == SPAWN_TYPE_CREATURE)
        {
            auto creature_spawns = sMySQLStore._creatureSpawnsStore[getBaseMap()->getMapId()];
            if (creature_spawns.size())
            {
                for (auto const& spawn : creature_spawns)
                {
                    if (spawn->id == spawnId)
                        saveRespawnTime(type, spawnId, spawn->entry, time_t(respawnTime), spawn->x, spawn->y, true);
                }
            }
        }
        else if (type == SPAWN_TYPE_GAMEOBJECT)
        {
            auto gameobject_spawns = sMySQLStore._gameobjectSpawnsStore[getBaseMap()->getMapId()];
            if (gameobject_spawns.size())
            {
                for (auto const& spawn : gameobject_spawns)
                {
                    if (spawn->id == spawnId)
                        saveRespawnTime(type, spawnId, spawn->entry, time_t(respawnTime), spawn->position_x, spawn->position_y, true);
                }
            }
        }
        else
        {
            sLogger.debug("Loading saved respawn time of %" PRIu64 " for spawnid (%u,%u) - invalid spawn type, ignoring", respawnTime, uint32_t(type), spawnId);
        }

    } while (result->NextRow());
}

void WorldMap::saveRespawnTime(SpawnObjectType type, uint32_t spawnId, uint32_t entry, time_t respawnTime, float cellX, float cellY, bool startup)
{
    if (!respawnTime)
    {
        // Delete only
        removeRespawnTime(type, spawnId);
        return;
    }

    RespawnInfo ri;
    ri.type = type;
    ri.spawnId = spawnId;
    ri.entry = entry;
    ri.time = respawnTime;
    ri.obj = nullptr;
    ri.cellX = cellX;
    ri.cellY = cellY;
    bool success = addRespawn(ri);

    if (startup)
    {
        if (!success)
            sLogger.failure("Attempt to load saved respawn %" PRIu64 " for (%u,%u) failed - duplicate respawn? Skipped.", respawnTime, uint32_t(type), spawnId);
    }
    else if (success)
        saveRespawnDB(ri);
}

void WorldMap::saveRespawnDB(RespawnInfo const& info)
{
    CharacterDatabase.Execute("REPLACE INTO respawn (type, spawnId, respawnTime, mapId, instanceId) VALUES (%u, %u, %u, %u, %u)", info.type, info.spawnId, uint64_t(info.time), getBaseMap()->getMapId(), getInstanceId());
}

bool WorldMap::addRespawn(RespawnInfo const& info)
{
    if (!info.spawnId)
    {
        sLogger.failure("Attempt to insert respawn info for zero spawn id (type %u)", uint32_t(info.type));
        return false;
    }

    RespawnInfoMap& bySpawnIdMap = getRespawnMapForType(info.type);

    // check if we already have the maximum possible number of respawns scheduled
    if (info.type == SPAWN_TYPE_CREATURE || info.type == SPAWN_TYPE_GAMEOBJECT)
    {
        auto it = bySpawnIdMap.find(info.spawnId);
        if (it != bySpawnIdMap.end()) // spawnid already has a respawn scheduled
        {
            RespawnInfo* const existing = it->second;
            if (info.time <= existing->time) // delete existing in this case
                deleteRespawn(existing);
            else
                return false;
        }
    }
    else
    {
        sLogger.failure("Invalid respawn info for spawn id (%u,%u) being inserted", uint32(info.type), info.spawnId);
    }

    RespawnInfo* ri = new RespawnInfo(info);
    _respawnTimes.emplace(ri);
    bySpawnIdMap.emplace(ri->spawnId, ri);
    return true;
}

static void pushRespawnInfoFrom(std::vector<RespawnInfo*>& data, RespawnInfoMap const& map)
{
    data.reserve(data.size() + map.size());
    for (auto const& pair : map)
        data.push_back(pair.second);
}

void WorldMap::getRespawnInfo(std::vector<RespawnInfo*>& respawnData, SpawnObjectTypeMask types) const
{
    if (types & SPAWN_TYPEMASK_CREATURE)
        pushRespawnInfoFrom(respawnData, _creatureRespawnTimesBySpawnId);
    if (types & SPAWN_TYPEMASK_GAMEOBJECT)
        pushRespawnInfoFrom(respawnData, _gameObjectRespawnTimesBySpawnId);
}

RespawnInfo* WorldMap::getRespawnInfo(SpawnObjectType type, uint32_t spawnId) const
{
    RespawnInfoMap const& map = getRespawnMapForType(type);
    auto it = map.find(spawnId);
    if (it == map.end())
        return nullptr;
    return it->second;
}

void WorldMap::unloadAllRespawnInfos()
{
    _respawnTimes.clear();
    _creatureRespawnTimesBySpawnId.clear();
    _gameObjectRespawnTimesBySpawnId.clear();
}


void WorldMap::removeRespawnTime(SpawnObjectType type,uint32_t spawnId)
{
    if (RespawnInfo* info = getRespawnInfo(type, spawnId))
        deleteRespawn(info);
}

void WorldMap::deleteRespawn(RespawnInfo* info)
{
    // Delete from all relevant containers to ensure consistency
    ASSERT(info);

    // spawnid store
    auto& spawnMap = getRespawnMapForType(info->type);
    auto range = spawnMap.equal_range(info->spawnId);
    auto it = std::find_if(range.first, range.second, [info](RespawnInfoMap::value_type const& pair) { return (pair.second == info); });
    ASSERT(it != range.second);
    spawnMap.erase(it);

    // respawn heap
    _respawnTimes.remove(info);

    // database
    deleteRespawnFromDB(info->type, info->spawnId);

    // then cleanup the object
    delete info;
}

void WorldMap::deleteRespawnTimesInDB(uint32_t mapId, uint32_t instanceId)
{
    CharacterDatabase.Execute("DELETE FROM respawn WHERE mapId = %u AND instanceId = %u", mapId, instanceId);
}

void WorldMap::deleteRespawnFromDB(SpawnObjectType type, uint32_t spawnId)
{
    CharacterDatabase.Execute("DELETE FROM respawn WHERE type = %u AND spawnId = %u AND mapId = %u AND instanceId = %u", type, spawnId, getBaseMap()->getMapId(), getInstanceId());
}

bool WorldMap::checkRespawn(RespawnInfo* info)
{
    bool doDelete = false;
    switch (info->type)
    {
        case SPAWN_TYPE_CREATURE:
        {
            if (_sqlids_creatures.find(info->spawnId) != _sqlids_creatures.end())
                doDelete = true;
            break;
        }
        case SPAWN_TYPE_GAMEOBJECT:
        {
            if (_sqlids_gameobjects.find(info->spawnId) != _sqlids_gameobjects.end())
                doDelete = true;
            break;
        }
        default:
            sLogger.failure("Invalid spawn type %u with spawnId %u on map %u", uint32_t(info->type), info->spawnId, getBaseMap()->getMapId());
            return true;
    }

    if (doDelete)
    {
        info->time = 0;
        return false;
    }

    // everything ok, let's spawn
    return true;
}

void WorldMap::doRespawn(SpawnObjectType type, Object* object, uint32_t spawnId, float cellX, float cellY)
{
    deleteRespawnFromDB(type, spawnId);
    
    MapCell* cell = getCellByCoords(cellX, cellY);
    if (cell == nullptr)    //cell got deleted while waiting for respawn.
        return;

    switch (type)
    {
        case SPAWN_TYPE_CREATURE:
        {
            Creature* obj = object->ToCreature();
            if (obj)
            {
                auto itr = cell->_respawnObjects.find(obj);
                if (itr != cell->_respawnObjects.end())
                {
                    obj->m_respawnCell = nullptr;
                    cell->_respawnObjects.erase(itr);
                    obj->OnRespawn(this);
                }
            }
            break;
        }
        case SPAWN_TYPE_GAMEOBJECT:
        {
            GameObject* obj = object->ToGameObject();
            if (obj)
            {
                auto itr = cell->_respawnObjects.find(obj);
                if (itr != cell->_respawnObjects.end())
                {
                    obj->m_respawnCell = nullptr;
                    cell->_respawnObjects.erase(itr);
                    obj->Spawn(this);
                }
            }
            break;
        }
        default:
            sLogger.failure("Invalid spawn type %u (spawnid %u) on map %u", uint32(type), spawnId, getBaseMap()->getMapId());
    }
}

void WorldMap::addCorpseDespawn(uint64_t guid, time_t time)
{
    const auto now = Util::getTimeNow();

    CorpseInfo info(now + time, guid);
    _corpseDespawnTimes.emplace(info);
}

void WorldMap::updateObjects()
{
    {
        std::unique_lock<std::mutex> lock(m_updateMutex);

        if (!_updates.size() && !_processQueue.size())
            return;

        ByteBuffer update(2500);
        uint32_t count = 0;

        for (auto pObj : _updates)
        {
            if (pObj == nullptr)
                continue;

            if (pObj->isItem() || pObj->isContainer())
            {
                // our update is only sent to the owner here.
                Player* pOwner = static_cast<Item*>(pObj)->getOwner();
                if (pOwner != nullptr)
                {
                    count = pObj->BuildValuesUpdateBlockForPlayer(&update, pOwner);
                    // send update to owner
                    if (count)
                    {
                        pOwner->getUpdateMgr().pushUpdateData(&update, count);
                        update.clear();
                    }
                }
            }
            else
            {
                if (pObj->IsInWorld())
                {
                    // players have to receive their own updates ;)
                    if (pObj->isPlayer())
                    {
                        // need to be different! ;)
                        count = pObj->BuildValuesUpdateBlockForPlayer(&update, static_cast<Player*>(pObj));
                        if (count)
                        {
                            static_cast<Player*>(pObj)->getUpdateMgr().pushUpdateData(&update, count);
                            update.clear();
                        }
                    }
                    else if (pObj->isCreatureOrPlayer() && static_cast<Unit*>(pObj)->mPlayerControler != nullptr)
                    {
                        count = pObj->BuildValuesUpdateBlockForPlayer(&update, static_cast<Unit*>(pObj)->mPlayerControler);
                        if (count)
                        {
                            static_cast<Unit*>(pObj)->mPlayerControler->getUpdateMgr().pushUpdateData(&update, count);
                            update.clear();
                        }
                    }

                    // build the update
                    count = pObj->BuildValuesUpdateBlockForPlayer(&update, static_cast<Player*>(NULL));

                    if (count)
                    {
                        for (const auto& itr : pObj->getInRangePlayersSet())
                        {
                            Player* lplr = static_cast<Player*>(itr);

                            // Make sure that the target player can see us.
                            if (lplr && lplr->isVisibleObject(pObj->getGuid()))
                                lplr->getUpdateMgr().pushUpdateData(&update, count);
                        }
                        update.clear();
                    }
                }
            }
            pObj->ClearUpdateMask();
        }
        _updates.clear();

        // generate pending a9packets and send to clients.
        for (auto it = _processQueue.begin(); it != _processQueue.end();)
        {
            Player* player = *it;

            auto it2 = it;
            ++it;

            _processQueue.erase(it2);
            if (player->getWorldMap() == this)
                player->processPendingUpdates();
        }
    }
}

void WorldMap::pushToProcessed(Player* plr)
{
    _processQueue.insert(plr);
}

MapScriptInterface* WorldMap::getInterface()
{
    return ScriptInterface;
}

WorldStatesHandler& WorldMap::getWorldStatesHandler()
{
    return worldstateshandler;
}

void WorldMap::onWorldStateUpdate(uint32_t zone, uint32_t field, uint32_t value)
{
    sendPacketToPlayersInZone(zone, SmsgUpdateWorldState(field, value).serialise().get());
}

bool WorldMap::isCombatInProgress()
{
    return (_combatProgress.size() > 0);
}

void WorldMap::addCombatInProgress(uint64_t guid)
{
    _combatProgress.insert(guid);
}

void WorldMap::removeCombatInProgress(uint64_t guid)
{
    _combatProgress.erase(guid);
}

bool WorldMap::addToMapMgr(Transporter* obj)
{
    std::unique_lock<std::mutex> lock(m_transportsLock);

    m_TransportStorage.insert(obj);
    return true;
}

void WorldMap::removeFromMapMgr(Transporter* obj)
{
    std::unique_lock<std::mutex> lock(m_transportsLock);

    m_TransportStorage.erase(obj);
    sTransportHandler.removeInstancedTransport(obj, getInstanceId());
}

void WorldMap::objectUpdated(Object* obj)
{
    // set our fields to dirty stupid fucked up code in places.. i hate doing this but i've got to :<- burlex
    std::unique_lock<std::mutex> lock(m_updateMutex);
    _updates.insert(obj);
}

float WorldMap::getUpdateDistance(Object* curObj, Object* obj, Player* plObj)
{
    static float no_distance = 0.0f;

    // unlimited distance for people on same boat
    if (curObj->isPlayer() && obj->isPlayer() && plObj != nullptr && plObj->obj_movement_info.hasMovementFlag(MOVEFLAG_TRANSPORT) && plObj->obj_movement_info.transport_guid == curObj->obj_movement_info.transport_guid)
        return no_distance;
    // unlimited distance for transporters (only up to 2 cells +/- anyway.)
    if (curObj->GetTypeFromGUID() == HIGHGUID_TYPE_TRANSPORTER)
        return no_distance;

    // unlimited distance in Instances/Raids
    if (getBaseMap()->getMapInfo()->isInstanceMap())
        return no_distance;

    //If the object announcing its position is a transport, or other special object, then deleting it from visible objects should be avoided. - By: VLack
    if (obj->isGameObject() && (static_cast<GameObject*>(obj)->GetOverrides() & GAMEOBJECT_INFVIS) && obj->GetMapId() == curObj->GetMapId())
        return no_distance;

    //If the object we're checking for possible removal is a transport or other special object, and we are players on the same map, don't remove it, and add it whenever possible...
    if (plObj && curObj->isGameObject() && (static_cast<GameObject*>(curObj)->GetOverrides() & GAMEOBJECT_INFVIS) && obj->GetMapId() == curObj->GetMapId())
        return no_distance;

    // normal distance
    return m_VisibleDistance;
}

bool WorldMap::isRegularDifficulty()
{
    return getDifficulty() == InstanceDifficulty::Difficulties::DUNGEON_NORMAL;
}

DBC::Structures::MapDifficulty const* WorldMap::getMapDifficulty()
{
    return getMapDifficultyData(getBaseMap()->getMapId(), getDifficulty());
}

bool WorldMap::isRaidOrHeroicDungeon()
{
    return getBaseMap()->isRaid() || getSpawnMode() > InstanceDifficulty::Difficulties::DUNGEON_NORMAL;
}

bool WorldMap::isHeroic()
{
    return getBaseMap()->isRaid() ? getSpawnMode() >= InstanceDifficulty::Difficulties::RAID_10MAN_HEROIC : getSpawnMode() >= InstanceDifficulty::Difficulties::DUNGEON_HEROIC;
}

bool WorldMap::is25ManRaid()
{
    return getBaseMap()->isRaid() && getSpawnMode() & 1;
}

bool WorldMap::getAreaInfo(uint32_t /*phaseMask*/, float x, float y, float z, uint32_t& flags, int32_t& adtId, int32_t& rootId, int32_t& groupId)
{
    float vmap_z = z;
    float dynamic_z = z;
    float check_z = z;
    const auto vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    uint32_t vflags;
    int32_t vadtId;
    int32_t vrootId;
    int32_t vgroupId;
    uint32_t dflags;
    int32_t dadtId;
    int32_t drootId;
    int32_t dgroupId;

    bool hasVmapAreaInfo = vmgr->getAreaInfo(getBaseMap()->getMapId(), x, y, vmap_z, vflags, vadtId, vrootId, vgroupId);
    bool hasDynamicAreaInfo = false;/*_dynamicTree.getAreaInfo(x, y, dynamic_z, phaseMask, dflags, dadtId, drootId, dgroupId);*/
    auto useVmap = [&]() { check_z = vmap_z; flags = vflags; adtId = vadtId; rootId = vrootId; groupId = vgroupId; };
    auto useDyn = [&]() { check_z = dynamic_z; flags = dflags; adtId = dadtId; rootId = drootId; groupId = dgroupId; };

    if (hasVmapAreaInfo)
    {
        if (hasDynamicAreaInfo && dynamic_z > vmap_z)
            useDyn();
        else
            useVmap();
    }
    else if (hasDynamicAreaInfo)
    {
        useDyn();
    }

    if (hasVmapAreaInfo || hasDynamicAreaInfo)
    {
        // check if there's terrain between player height and object height
        if (TerrainTile* gmap = getTerrain()->getTile(x, y))
        {
            float mapHeight = gmap->m_map.getHeight(x, y);
            // z + 2.0f condition taken from getHeight(), not sure if it's such a great choice...
            if (z + 2.0f > mapHeight && mapHeight > check_z)
                return false;
        }
        return true;
    }
    return false;
}

uint32_t WorldMap::getAreaId(uint32_t phaseMask, float x, float y, float z)
{
    uint32_t mogpFlags;
    int32_t adtId, rootId, groupId;
    float vmapZ = z;
    bool hasVmapArea = getAreaInfo(phaseMask, x, y, vmapZ, mogpFlags, adtId, rootId, groupId);

    uint32_t gridAreaId = 0;
    float gridMapHeight = INVALID_HEIGHT;
    if (TerrainTile* gmap = getTerrain()->getTile(x, y))
    {
        gridAreaId = gmap->m_map.getArea(x, y);
        gridMapHeight = gmap->m_map.getHeight(x, y);
    }

    uint32_t areaId = 0;

    // floor is the height we are closer to (but only if above)
    if (hasVmapArea && G3D::fuzzyGe(z, vmapZ - GROUND_HEIGHT_TOLERANCE) && (G3D::fuzzyLt(z, gridMapHeight - GROUND_HEIGHT_TOLERANCE) || vmapZ > gridMapHeight))
    {
        // wmo found
        if (DBC::Structures::WMOAreaTableEntry const* wmoEntry = GetWMOAreaTableEntryByTriple(rootId, adtId, groupId))
            areaId = wmoEntry->areaId;

        if (!areaId)
            areaId = gridAreaId;
    }
    else
        areaId = gridAreaId;

    if (!areaId)
        areaId = getBaseMap()->getMapEntry()->linked_zone;

    return areaId;
}

uint32_t WorldMap::getZoneId(uint32_t phaseMask, float x, float y, float z)
{
    uint32_t areaId = getAreaId(phaseMask, x, y, z);
    if (DBC::Structures::AreaTableEntry const* area = sAreaStore.LookupEntry(areaId))
        if (area->zone)
            return area->zone;

    return areaId;
}

void WorldMap::getZoneAndAreaId(uint32_t phaseMask, uint32_t& zoneid, uint32_t& areaid, float x, float y, float z)
{
    areaid = zoneid = getAreaId(phaseMask, x, y, z);
    if (DBC::Structures::AreaTableEntry const* area = sAreaStore.LookupEntry(areaid))
        if (area->zone)
            zoneid = area->zone;
}

static inline bool isInWMOInterior(uint32_t mogpFlags)
{
    return (mogpFlags & 0x2000) != 0;
}

ZLiquidStatus WorldMap::getLiquidStatus(uint32_t phaseMask, float x, float y, float z, uint8_t ReqLiquidType, LiquidData* data, float collisionHeight)
{
    ZLiquidStatus result = LIQUID_MAP_NO_WATER;
    const auto vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    float liquid_level = INVALID_HEIGHT;
    float ground_level = INVALID_HEIGHT;
    uint32_t liquid_type = 0;
    uint32_t mogpFlags = 0;
    bool useGridLiquid = true;
    if (vmgr->getLiquidLevel(getBaseMap()->getMapId(), x, y, z, ReqLiquidType, liquid_level, ground_level, liquid_type, mogpFlags))
    {
        useGridLiquid = !isInWMOInterior(mogpFlags);
        // Check water level and ground level
        if (liquid_level > ground_level && G3D::fuzzyGe(z, ground_level - GROUND_HEIGHT_TOLERANCE))
        {
            // All ok in water -> store data
            if (data)
            {
                // hardcoded in client like this
                if (getBaseMap()->getMapId() == 530 && liquid_type == 2)
                    liquid_type = 15;

                uint32_t liquidFlagType = 0;
                if (DBC::Structures::LiquidTypeEntry const* liq = sLiquidTypeStore.LookupEntry(liquid_type))
                    liquidFlagType = liq->Type;

                if (liquid_type && liquid_type < 21)
                {
                    if (DBC::Structures::AreaTableEntry const* area = sAreaStore.LookupEntry(getAreaId(phaseMask, x, y, z)))
                    {
#if VERSION_STRING > Classic
                        uint32_t overrideLiquid = area->liquid_type_override[liquidFlagType];
                        if (!overrideLiquid && area->zone)
                        {
                            area = sAreaStore.LookupEntry(area->zone);
                            if (area)
                                overrideLiquid = area->liquid_type_override[liquidFlagType];
                        }
#else
                        uint32_t overrideLiquid = area->liquid_type_override;
                        if (!overrideLiquid && area->zone)
                        {
                            area = MapManagement::AreaManagement::AreaStorage::GetAreaById(area->zone);
                            if (area)
                                overrideLiquid = area->liquid_type_override;
                        }
#endif

                        if (DBC::Structures::LiquidTypeEntry const* liq = sLiquidTypeStore.LookupEntry(overrideLiquid))
                        {
                            liquid_type = overrideLiquid;
                            liquidFlagType = liq->Type;
                        }
                    }
                }

            data->level = liquid_level;
            data->depth_level = ground_level;

            data->entry = liquid_type;
            data->type_flags = 1 << liquidFlagType;
            }

        float delta = liquid_level - z;

        // Get position delta
        if (delta > collisionHeight)                   // Under water
            return LIQUID_MAP_UNDER_WATER;
        if (delta > 0.0f)                   // In water
            return LIQUID_MAP_IN_WATER;
        if (delta > -0.1f)                   // Walk on water
            return LIQUID_MAP_WATER_WALK;
        result = LIQUID_MAP_ABOVE_WATER;
        }
    }

if (useGridLiquid)
{
    if (TerrainTile* gmap = getTerrain()->getTile(x, y))
    {
        LiquidData map_data;
        ZLiquidStatus map_result = gmap->m_map.getLiquidStatus(x, y, z, ReqLiquidType, &map_data, collisionHeight);
        // Not override LIQUID_MAP_ABOVE_WATER with LIQUID_MAP_NO_WATER:
        if (map_result != LIQUID_MAP_NO_WATER && (map_data.level > ground_level))
        {
            if (data)
            {
                // hardcoded in client like this
                if (getBaseMap()->getMapId() == 530 && map_data.entry == 2)
                    map_data.entry = 15;

                *data = map_data;
            }
            return map_result;
        }
    }
}
return result;
}

float WorldMap::getWaterLevel(float x, float y)
{
    if (TerrainTile* gmap = getTerrain()->getTile(x, y))
        return gmap->m_map.getLiquidLevel(x, y);
    else
        return 0;
}

bool WorldMap::isInWater(uint32_t phaseMask, float x, float y, float pZ, LiquidData* data)
{
    LiquidData liquid_status;
    LiquidData* liquid_ptr = data ? data : &liquid_status;
    return (getLiquidStatus(phaseMask, x, y, pZ, MAP_ALL_LIQUIDS, liquid_ptr) & (LIQUID_MAP_IN_WATER | LIQUID_MAP_UNDER_WATER)) != 0;
}

bool WorldMap::isUnderWater(uint32_t phaseMask, float x, float y, float z)
{
    return (getLiquidStatus(phaseMask, x, y, z, MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN) & LIQUID_MAP_UNDER_WATER) != 0;
}

bool WorldMap::isInLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2, uint32_t phasemask, LineOfSightChecks checks)
{
    if ((checks & LINEOFSIGHT_CHECK_VMAP)
        && !VMAP::VMapFactory::createOrGetVMapManager()->isInLineOfSight(getBaseMap()->getMapId(), x1, y1, z1, x2, y2, z2))
        return false;

    if (checks & LINEOFSIGHT_CHECK_GOBJECT && !_dynamicTree.isInLineOfSight(x1, y1, z1, x2, y2, z2, phasemask))
        return false;

    return true;
}

bool WorldMap::getObjectHitPos(uint32_t phasemask, float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float& ry, float& rz, float modifyDist)
{
    G3D::Vector3 startPos(x1, y1, z1);
    G3D::Vector3 dstPos(x2, y2, z2);

    G3D::Vector3 resultPos;
    bool result = _dynamicTree.getObjectHitPos(phasemask, startPos, dstPos, resultPos, modifyDist);

    rx = resultPos.x;
    ry = resultPos.y;
    rz = resultPos.z;
    return result;
}

float WorldMap::getWaterOrGroundLevel(uint32_t phasemask, float x, float y, float z, float* ground /*= nullptr*/, bool /*swim = false*/, float collisionHeight /*= 2.03128f*/)
{
    if (getTerrain()->getTile(x, y))
    {
        // we need ground level (including grid height version) for proper return water level in point
        float ground_z = getHeight(phasemask, x, y, z + collisionHeight, true, 50.0f);
        if (ground)
            *ground = ground_z;

        LiquidData liquid_status;

        ZLiquidStatus res = getLiquidStatus(phasemask, x, y, ground_z, MAP_ALL_LIQUIDS, &liquid_status, collisionHeight);
        switch (res)
        {
        case LIQUID_MAP_ABOVE_WATER:
            return std::max<float>(liquid_status.level, ground_z);
        case LIQUID_MAP_NO_WATER:
            return ground_z;
        default:
            return liquid_status.level;
        }
    }

    return VMAP_INVALID_HEIGHT_VALUE;
}

float WorldMap::getHeight(float x, float y, float z, bool checkVMap /*= true*/, float maxSearchDist /*= 50.0f*/) const
{
    // find raw .map surface under Z coordinates
    float mapHeight = VMAP_INVALID_HEIGHT_VALUE;
    float gridHeight = getGridHeight(x, y);
    if (G3D::fuzzyGe(z, gridHeight - GROUND_HEIGHT_TOLERANCE))
        mapHeight = gridHeight;

    float vmapHeight = VMAP_INVALID_HEIGHT_VALUE;
    if (checkVMap)
    {
        const auto vmgr = VMAP::VMapFactory::createOrGetVMapManager();
        if (vmgr->isHeightCalcEnabled())
            vmapHeight = vmgr->getHeight(getBaseMap()->getMapId(), x, y, z, maxSearchDist);
    }

    // mapHeight set for any above raw ground Z or <= INVALID_HEIGHT
    // vmapheight set for any under Z value or <= INVALID_HEIGHT
    if (vmapHeight > INVALID_HEIGHT)
    {
        if (mapHeight > INVALID_HEIGHT)
        {
            // we have mapheight and vmapheight and must select more appropriate

            // vmap height above map height
            // or if the distance of the vmap height is less the land height distance
            if (vmapHeight > mapHeight || std::fabs(mapHeight - z) > std::fabs(vmapHeight - z))
                return vmapHeight;

            return mapHeight;                           // better use .map surface height
        }

        return vmapHeight;                              // we have only vmapHeight (if have)
    }

    return mapHeight;                               // explicitly use map data
}

float WorldMap::getGridHeight(float x, float y) const
{
    if (TerrainTile* gmap = getTerrain()->getTile(x, y))
        return gmap->m_map.getHeight(x, y);

    return VMAP_INVALID_HEIGHT_VALUE;
}

void WorldMap::addObjectToActiveSet(Object* obj)
{
    switch (obj->getObjectTypeId())
    {
        case TYPEID_UNIT:
            activeCreatures.insert(static_cast<Creature*>(obj));
            break;
        case TYPEID_GAMEOBJECT:
            activeGameObjects.insert(static_cast<GameObject*>(obj));
            break;
    }
}

void WorldMap::removeObjectFromActiveSet(Object* obj)
{
    switch (obj->getObjectTypeId())
    {
        case TYPEID_UNIT:
            activeCreatures.erase(static_cast<Creature*>(obj));
            break;
        case TYPEID_GAMEOBJECT:
            activeGameObjects.erase(static_cast<GameObject*>(obj));
            break;
    }
}

PlayerStorageMap const& WorldMap::getPlayers() const { return m_PlayerStorage; }
CreaturesStorageMap const& WorldMap::getCreatures() const { return m_CreatureStorage; }
PetStorageMap const& WorldMap::getPets() const { return m_PetStorage; }
GameObjectStorageMap const& WorldMap::getGameObjects() const { return m_GameObjectStorage; }
DynamicObjectStorageMap const& WorldMap::getDynamicObjects() const { return m_DynamicObjectStorage; }
TransportsContainer const& WorldMap::getTransports() const { return m_TransportStorage; }

void WorldMap::respawnBossLinkedGroups(uint32_t bossId)
{
    // Get all Killed npcs out of Killed npc Store
    for (Creature* spawn : sMySQLStore.getSpawnGroupDataByBoss(bossId))
    {
        if (spawn && spawn->m_spawn && spawn->getSpawnId())
        {
            // Get the Group Data and see if we have to Respawn the npcs
            auto data = sMySQLStore.getSpawnGroupDataBySpawn(spawn->getSpawnId());

            if (data && data->spawnFlags & SPAWFLAG_FLAG_BOUNDTOBOSS)
            {
                // Respawn the Npc
                if (spawn->IsInWorld() && !spawn->isAlive())
                {
                    spawn->Despawn(0, 1000);
                }
                else if (!spawn->isAlive())
                {
                    // get the cell with our SPAWN location. if we've moved cell this might break :P
                    MapCell* pCell = getCellByCoords(spawn->GetSpawnX(), spawn->GetSpawnY());
                    if (pCell == nullptr)
                        pCell = spawn->GetMapCell();

                    if (pCell != nullptr)
                    {
                        pCell->_respawnObjects.insert(spawn);

                        sEventMgr.RemoveEvents(spawn);
                        doRespawn(SPAWN_TYPE_CREATURE, spawn, spawn->getSpawnId(), pCell->getPositionX(), pCell->getPositionY());

                        spawn->SetPosition(spawn->GetSpawnPosition(), true);
                        spawn->m_respawnCell = pCell;
                    }
                }
            }
        }
    }
}

void WorldMap::spawnManualGroup(uint32_t groupId)
{
    auto data = sMySQLStore.getSpawnGroupDataByGroup(groupId);

    if (data)
    {
        for (auto spawns : data->spawns)
        {
            if (auto creature = spawns.second)
            {
                creature->PrepareForRemove();

                // get the cell with our SPAWN location. if we've moved cell this might break :P
                MapCell* pCell = getCellByCoords(creature->GetSpawnX(), creature->GetSpawnY());
                if (pCell == nullptr)
                    pCell = creature->GetMapCell();

                if (pCell != nullptr)
                {
                    pCell->_respawnObjects.insert(creature);

                    sEventMgr.RemoveEvents(creature);
                    doRespawn(SPAWN_TYPE_CREATURE, creature, creature->getSpawnId(), pCell->getPositionX(), pCell->getPositionY());
                    creature->SetPosition(creature->GetSpawnPosition(), true);
                    creature->m_respawnCell = pCell;
                }
            }
        }
    }
}
