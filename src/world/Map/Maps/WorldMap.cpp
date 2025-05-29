/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WorldMap.hpp"
#include "Objects/DynamicObject.hpp"
#include "Objects/Units/Creatures/CreatureGroups.h"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Creatures/Summons/Summon.hpp"
#include "Objects/Units/Unit.hpp"
#include "VMapFactory.h"
#include "MMapFactory.h"
#include "Macros/MapsMacros.hpp"
#include "shared/WoWGuid.h"
#include "MapScriptInterface.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Map/Management/MapMgr.hpp"
#include "InstanceMap.hpp"
#include "Server/Packets/SmsgUpdateWorldState.h"
#include "Server/Packets/SmsgDefenseMessage.h"
#include "Map/Area/AreaManagementGlobals.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Debugging/CrashHandler.h"
#include "Objects/Transporter.hpp"
#include "Objects/Units/Creatures/Summons/SummonDefines.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Spell/Definitions/SummonControlTypes.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"

#include <ctime>
#include <cstdarg>

#include "Logging/Logger.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Objects/Item.hpp"
#include "Server/EventMgr.h"
#include "Storage/WDB/WDBStructures.hpp"

using namespace AscEmu::Packets;
using namespace AscEmu::Threading;

extern bool bServerShutdown;

WorldMap::WorldMap(BaseMap* baseMap, uint32_t id, uint32_t expiry, uint32_t InstanceId, uint8_t SpawnMode) : CellHandler<MapCell>(baseMap), eventHolder(InstanceId), worldstateshandler(id),
    _terrain(std::make_unique<TerrainHolder>(id)), m_unloadTimer(expiry), m_baseMap(baseMap)
{
    // Map
    setSpawnMode(SpawnMode);
    setInstanceId(InstanceId);

    m_holder = &eventHolder;
    m_event_Instanceid = eventHolder.GetInstanceID();

    // Create script interface
    ScriptInterface = std::make_unique<MapScriptInterface>(*this);

    // Set up storage arrays
    m_CreatureStorage.resize(getBaseMap()->CreatureSpawnCount, nullptr);
    m_GameObjectStorage.resize(getBaseMap()->GameObjectSpawnCount, nullptr);

    // Thread
    const std::string threadName("WorldMap - M" + std::to_string(getBaseMap()->getMapId()) + "|I" + std::to_string(getInstanceId()));
    m_thread = std::make_unique<AEThread>(threadName, [this](AEThread& /*thread*/) { this->runThread(); }, std::chrono::milliseconds(20), false);

    //lets initialize visibility distance for Continent
    WorldMap::initVisibilityDistance();
}

void WorldMap::initialize()
{
    // Create Instance script
    loadInstanceScript();

    // create Map Wide objects
    for (auto& GameobjectSpawn : _map->mapWideSpawns.GameobjectSpawns)
    {
        GameObject* obj = createGameObject(GameobjectSpawn->entry);
        obj->loadFromDB(GameobjectSpawn, this, false);
        PushStaticObject(obj);
    }

    for (auto& CreatureSpawn : _map->mapWideSpawns.CreatureSpawns)
    {
        Creature* obj = createCreature(CreatureSpawn->entry);
        obj->Load(CreatureSpawn, 0, getBaseMap()->getMapInfo());
        PushStaticObject(obj);
    }

    // Call script OnLoad virtual procedure
    if (getScript())
        getScript()->OnLoad();

    // load corpses
    sObjectMgr.loadCorpsesForInstance(this);
    worldstateshandler.InitWorldStates(sObjectMgr.getWorldStatesForMap(getBaseMap()->getMapId()));
    worldstateshandler.setObserver(this);
}

WorldMap::~WorldMap()
{
    m_thread->killAndJoin();
    sEventMgr.RemoveEvents(this);

    // Prevents a crash on map shutdown -Appled
    ScriptInterface = nullptr;

    // Remove objects
    if (_cells != nullptr)
    {
        for (uint32_t i = 0; i < Map::Cell::_sizeX; i++)
        {
            if ((*_cells)[i] != nullptr)
            {
                for (uint32_t j = 0; j < Map::Cell::_sizeY; j++)
                {
                    if ((*(*_cells)[i])[j] != nullptr)
                    {
                        (*(*_cells)[i])[j]->_unloadpending = false;
                        (*(*_cells)[i])[j]->removeObjects();
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
    m_TransportDelayedRemoveStorage.clear();

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

    if (mInstanceScript != nullptr)
        mInstanceScript->Destroy();

    // Empty remaining containers
    m_PlayerStorage.clear();
    m_PetStorage.clear();
    m_DynamicObjectStorage.clear();

    _updates.clear();
    _processQueue.clear();
    Sessions.clear();

    activeCreatures.clear();
    creature_iterator = activeCreatures.begin();
    activeGameObjects.clear();
    gameObject_iterator = activeGameObjects.begin();
    _sqlids_creatures.clear();
    _sqlids_gameobjects.clear();
    _reusable_guids_creature.clear();
    _reusable_guids_gameobject.clear();

    MMAP::MMapFactory::createOrGetMMapManager()->unloadMapInstance(getBaseMap()->getMapId(), getInstanceId());

    sLogger.debug("WorldMap : Instance {} shut down. ({})", getInstanceId(), getBaseMap()->getMapName());
}

void WorldMap::startMapThread()
{
    m_terminateThread = false;
    m_lastUpdateTime = Util::getMSTime();
    m_thread->reboot();
}

void WorldMap::runThread()
{
    THREAD_TRY_EXECUTION
        Do();
    THREAD_HANDLE_CRASH
        return;
}

void WorldMap::shutdownMapThread()
{
    pInstance = nullptr;
    m_terminateThread = true;
}

void WorldMap::unsafeKillMapThread()
{
    m_thread->killAndJoin();
}

bool WorldMap::isMapReadyForDelete() const
{
    return m_thread->isKilled() && m_thread->isDone();
}

void WorldMap::Do()
{
    m_threadRunning = true;

    if (!m_terminateThread)
    {
        const auto exec_start = Util::getMSTime();

        // Time In Milliseconds ( exact Difftime Since last update Cycle )
        const uint32_t diffTime = exec_start - m_lastUpdateTime;

        //first push to world new objects
        {
            std::scoped_lock<std::mutex> lock(m_objectinsertlock);
            if (!m_objectinsertpool.empty())
            {
                for (const auto& o : m_objectinsertpool)
                    o->PushToWorld(this);

                m_objectinsertpool.clear();
            }
        }

        // Update Our Map
        update(diffTime);

        m_lastUpdateTime = Util::getMSTime();
        return;
    }

    m_threadRunning = false;

    // Map is added to MapMgr remove pool which deletes map when thread has finished all its work
    m_thread->requestKill();
}

void WorldMap::update(uint32_t t_diff)
{
    const auto msTime = Util::getMSTime();

    // Update any events.
    // we make update of events before objects so in case there are 0 timediff events they do not get deleted after update but on next server update loop
    eventHolder.Update(t_diff);

    // Update Dynamic Map
    _dynamicTree.update(t_diff);

    // Update Transporters
    auto diffTime = msTime - m_lastTransportUpdateTimer;
    if (diffTime >= 100)
    {
        {
            std::scoped_lock<std::mutex> guard(m_transportsLock);
            for (auto itr = m_TransportStorage.cbegin(); itr != m_TransportStorage.cend();)
            {
                Transporter* trans = *itr;
                ++itr;

                if (!trans || !trans->IsInWorld())
                    continue;

                trans->Update(diffTime);
            }
        }

        {
            std::scoped_lock<std::mutex> guard(m_delayedTransportLock);
            for (auto itr = m_TransportDelayedRemoveStorage.cbegin(); itr != m_TransportDelayedRemoveStorage.cend();)
            {
                auto* trans = itr->first;
                if (!trans || !trans->IsInWorld())
                {
                    itr = m_TransportDelayedRemoveStorage.erase(itr);
                    continue;
                }

                if (itr->second)
                {
                    removeFromMapMgr(trans);
                    trans->RemoveFromWorld(true);
                }
                else
                {
                    trans->delayedTeleportTransport();
                }

                itr = m_TransportDelayedRemoveStorage.erase(itr);
            }
        }

        m_lastTransportUpdateTimer = msTime;
    }

    // Update Creatures
    {
        auto diffTime = msTime - m_lastCreatureUpdateTimer;
        creature_iterator = activeCreatures.begin();
        for (; creature_iterator != activeCreatures.end();)
        {
            Creature* ptr = *creature_iterator;
            ++creature_iterator;
            ptr->Update(diffTime);
        }

        m_lastCreatureUpdateTimer = msTime;
    }

    // Update Pets
    {
        const auto diffTime = msTime - m_lastPetUpdateTimer;
        for (auto itr = m_PetStorage.cbegin(); itr != m_PetStorage.cend();)
        {
            Pet* ptr = itr->second;
            ++itr;
            ptr->Update(diffTime);
        }

        m_lastPetUpdateTimer = msTime;
    }

    // Update Players
    {
        auto diffTime = msTime - m_lastPlayerUpdateTimer;
        for (auto itr = m_PlayerStorage.cbegin(); itr != m_PlayerStorage.cend();)
        {
            Player* ptr = itr->second;
            ++itr;
            ptr->Update(diffTime);
        }

        m_lastPlayerUpdateTimer = msTime;
    }

    // Dynamic objects are updated every 100ms
    diffTime = msTime - m_lastDynamicUpdateTimer;
    if (diffTime >= 100)
    {
        for (auto itr = m_DynamicObjectStorage.cbegin(); itr != m_DynamicObjectStorage.cend();)
        {
            DynamicObject* o = itr->second;
            ++itr;
            o->updateTargets();
        }

        m_lastDynamicUpdateTimer = msTime;
    }

    // Update gameobjects only every 200ms
    diffTime = msTime - m_lastGameObjectUpdateTimer;
    if (diffTime >= 200)
    {
        gameObject_iterator = activeGameObjects.begin();
        for (; gameObject_iterator != activeGameObjects.end();)
        {
            GameObject* gameobject = *gameObject_iterator;
            ++gameObject_iterator;
            if (gameobject != nullptr)
                gameobject->Update(diffTime);
        }

        m_lastGameObjectUpdateTimer = msTime;
    }

    // Update Sessions every 100ms
    diffTime = msTime - m_lastSessionUpdateTimer;
    if (diffTime >= 100)
    {
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

        m_lastSessionUpdateTimer = msTime;
    }

    /// Update Respawns every 1000ms
    diffTime = msTime - m_lastRespawnUpdateTimer;
    if (diffTime >= 1000)
    {
        processRespawns();

        // Time In Seconds
        const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        while (!_corpseDespawnTimes.empty())
        {
            CorpseInfo next = _corpseDespawnTimes.top();
            if (now < next.time)
                break;

            _corpseDespawnTimes.pop();
            if (auto* const pCorpse = sObjectMgr.getCorpseByGuid(static_cast<uint32_t>(next.guid)))
            {
                if (pCorpse->getWorldMap() != this)
                    break;

                pCorpse->despawn();
            }
            break;
        }

        m_lastRespawnUpdateTimer = msTime;
    }
    
    // Finally, A9 Building/Distribution
    updateObjects();
}

void WorldMap::processRespawns()
{
    const auto now = Util::getTimeNow();

    while (!_respawnTimes.empty())
    {
        RespawnInfo* next = _respawnTimes.top().get();
        if (now < next->time) // done for this tick
            break;

        if (checkRespawn(next)) // see if we're allowed to respawn
        {
            // ok, respawn
            getRespawnMapForType(next->type).erase(next->spawnId);
            doRespawn(next->type, next->obj, next->spawnId, next->cellX, next->cellY);
            _respawnTimes.pop();
        }
        else if (!next->time) // just remove respawn entry without rescheduling
        {
            getRespawnMapForType(next->type).erase(next->spawnId);
            _respawnTimes.pop();
        }
        else
        {
            saveRespawnDB(*next);
        }
    }
}

bool WorldMap::canUnload(uint32_t diff)
{
    if (!m_unloadTimer)
        return false;

    if (m_unloadTimer <= diff)
        return true;

    m_unloadTimer -= diff;
    return false;
}

void WorldMap::unloadAll(bool onShutdown/* = false*/)
{
    if (getPlayerCount())
        return;

    if (onShutdown)
        return;

    if (getInstanceId() == 0)
        sMapMgr.addMapToRemovePool(this);
    else
        sMapMgr.removeInstance(getInstanceId());
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
        for (auto itr = m_PlayerStorage.begin(); itr != m_PlayerStorage.end();)
        {
            Player* player = itr->second;
            ++itr;

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
    std::scoped_lock<std::mutex> lock(m_objectinsertlock);
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
            sLogger.failure("WorldMap::PushObject manager for mapId {} tried to push object for mapId {}, return!", getBaseMap()->getMapId(), obj->GetMapId());
            return;
        }

        if (obj->GetPositionY() > Map::Terrain::_maxY || obj->GetPositionY() < Map::Terrain::_minY)
        {
            sLogger.failure("WorldMap::PushObject not allowed to push object to y: {} (max {}/min {}), return!", obj->GetPositionY(), Map::Terrain::_maxY, Map::Terrain::_minY);
            return;
        }

        if (_cells == nullptr)
        {
            sLogger.failure("WorldMap::PushObject not allowed to push object to invalid cell (nullptr), return!");
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
        std::unique_ptr<ByteBuffer> buf;
        uint32_t count;
        Player* plObj = nullptr;

        if (obj->isPlayer())
        {
            plObj = static_cast<Player*>(obj);

            sLogger.debug("Creating player {} for himself.", std::to_string(obj->getGuid()));
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
                    updateInRangeSet(obj, plObj, cell, buf);
                }
            }
        }

        // Forced Cells
        for (auto& cell : m_forcedcells)
            updateInRangeSet(obj, plObj, cell, buf);

        //Add to the cell's object list
        objCell->addObject(obj);

        // Add Object
        if (getScript())
            getScript()->addObject(obj);

        obj->SetMapCell(objCell);
        //Add to the mapmanager's object list
        if (plObj != nullptr)
        {
            m_PlayerStorage[plObj->getGuidLow()] = plObj;
            updateCellActivity(x, y, 2U + cellNumber);
        }
        else
        {
            switch (obj->GetTypeFromGUID())
            {
                case HIGHGUID_TYPE_PET:
                {
                    m_PetStorage[obj->GetUIdFromGUID()] = static_cast<Pet*>(obj);
                } break;
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
                {
                    m_DynamicObjectStorage[obj->getGuidLow()] = static_cast<DynamicObject*>(obj);
                } break;
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
                uint32_t globalcount = 0;
                if (buf == nullptr)
                    buf = std::make_unique<ByteBuffer>(300);

                for (auto _mapWideStaticObject : _mapWideStaticObjects)
                {
                    count = _mapWideStaticObject->buildCreateUpdateBlockForPlayer(buf.get(), plObj);
                    globalcount += count;
                }
                /*VLack: It seems if we use the same buffer then it is a BAD idea to try and push created data one by one, add them at once!
                       If you try to add them one by one, then as the buffer already contains data, they'll end up repeating some object.
                       Like 6 object updates for Deeprun Tram, but the built package will contain these entries: 2AFD0, 2AFD0, 2AFD1, 2AFD0, 2AFD1, 2AFD2*/
                if (globalcount > 0)
                    plObj->getUpdateMgr().pushCreationData(buf.get(), globalcount);
            }
        }
    }
    else
    {
        sLogger.failure("WorldMap::PushObject tried to push invalid object (nullptr)!");
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
            sLogger.debug("WorldMap::PushStaticObject called for invalid type {}.", obj->GetTypeFromGUID());
            break;
    }

    if (getScript())
        getScript()->addObject(obj);
}

void WorldMap::RemoveObject(Object* obj, bool free_guid)
{
    // Assertions
    if (obj == nullptr)
    {
        sLogger.failure("WorldMap::RemoveObject tried to remove invalid object (nullptr)");
        return;
    }

    if (obj->GetMapId() != getBaseMap()->getMapId())
    {
        sLogger.failure("WorldMap::RemoveObject tried to remove object with map {} but WorldMap is for map {}!", obj->GetMapId(), getBaseMap()->getMapId());
        return;
    }

    if (_cells == nullptr)
    {
        sLogger.failure("WorldMap::RemoveObject tried to remove invalid cells (nullptr)");
        return;
    }

    // Call Script Object Got Removed
    if (getScript())
        getScript()->removeObject(obj);

    if (obj->IsActive())
        obj->deactivate(this);

    //there is a very small chance that on double player ports on same update player is added to multiple insertpools but not removed
    //one clear example was the double port proc when exploiting double resurrect
    {
        std::scoped_lock<std::mutex> lock(m_objectinsertlock);
        m_objectinsertpool.erase(obj);
    }

    {
        std::scoped_lock<std::mutex> lock(m_updateMutex);
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
            if (!obj->isPlayer())
                sLogger.debug("WorldMap::RemoveObject called for invalid type {} (not player).", obj->GetTypeFromGUID());
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
            updateCellActivity(x, y, 2U + cellNumber);
        }
        m_PlayerStorage.erase(obj->getGuidLow());
    }
    else if (obj->isCreatureOrPlayer() && static_cast<Unit*>(obj)->m_playerControler != nullptr)
    {
        if (obj->GetPositionX() < Map::Terrain::_maxX || obj->GetPositionX() > Map::Terrain::_minY || obj->GetPositionY() < Map::Terrain::_maxY || obj->GetPositionY() > Map::Terrain::_minY)
        {
            uint32_t x = getPosX(obj->GetPositionX());
            uint32_t y = getPosY(obj->GetPositionY());
            updateCellActivity(x, y, 2U + cellNumber);
        }
    }

    // Remove the session from our set if it is a player.
    if (obj->isPlayer() || obj->isCreatureOrPlayer() && static_cast<Unit*>(obj)->m_playerControler != nullptr)
    {
        for (auto _mapWideStaticObject : _mapWideStaticObjects)
        {
            if (_mapWideStaticObject != nullptr && plObj)
                plObj->getUpdateMgr().pushOutOfRangeGuid(_mapWideStaticObject->GetNewGUID());
        }

        // Setting an instance ID here will trigger the session to be removed by WorldMap::run(). :)
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
        sLogger.debugFlag(AscEmu::Logging::LF_MAP_CELL, "Updating all cells for map {:03}, server might lag.", getBaseMap()->getMapId());

    for (uint32_t x = StartX; x < EndX; x++)
    {
        for (uint32_t y = StartY; y < EndY; y++)
        {
            if (areamask)
            {
                if (!cellHasAreaID(x, y, AreaID))
                    continue;

                auto at = MapManagement::AreaManagement::AreaStorage::GetAreaById(AreaID);
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
                    sLogger.debugFlag(AscEmu::Logging::LF_MAP_CELL, "Created cell [{},{}] on map {} (instance {}).", x, y, getBaseMap()->getMapId(), getInstanceId());
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
        sLogger.debugFlag(AscEmu::Logging::LF_MAP_CELL, "Cell updating success for map {:03}", getBaseMap()->getMapId());
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
                    sLogger.debugFlag(AscEmu::Logging::LF_MAP_CELL, "Created cell [{},{}] on map {} (instance {}).", x, y, getBaseMap()->getMapId(), getInstanceId());
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
    sLogger.debugFlag(AscEmu::Logging::LF_MAP_CELL, "Cell updating success for map {:03}", getBaseMap()->getMapId());
}

void WorldMap::updateCellActivity(uint32_t x, uint32_t y, uint32_t radius)
{
    std::scoped_lock<std::mutex> guard(m_cellActivityLock);

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

                    sLogger.debug("WorldMap : Cell [{},{}] on map {} (instance {}) is now active.", posX, posY, getBaseMap()->getMapId(), getInstanceId());
                    objCell->setActivity(true);

                    getTerrain()->loadTile(static_cast<int32_t>(posX) / 8, static_cast<int32_t>(posY) / 8);

                    if (!objCell->isLoaded())
                    {
                        sLogger.debug("WorldMap : Loading objects for Cell [{}][{}] on map {} (instance {})...", posX, posY, getBaseMap()->getMapId(), getInstanceId());

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

                    sLogger.debug("Cell [{},{}] on map {} (instance {}) is now active.", posX, posY, getBaseMap()->getMapId(), getInstanceId());

                    getTerrain()->loadTile(static_cast<int32_t>(posX) / 8, static_cast<int32_t>(posY) / 8);
                    objCell->setActivity(true);

                    if (!objCell->isLoaded())
                    {
                        sLogger.debug("Loading objects for Cell [{}][{}] on map {} (instance {})...", posX, posY, getBaseMap()->getMapId(), getInstanceId());
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
    sLogger.debug("Cell [{},{}] on map {} (instance {}) is now idle.", x, y, getBaseMap()->getMapId(), getInstanceId());
    cell->setActivity(false);

    _terrain->unloadTile(static_cast<int32_t>(x) / 8, static_cast<int32_t>(y) / 8);
}

void WorldMap::unloadCell(uint32_t x, uint32_t y)
{
    MapCell* c = getCell(x, y);
    if (c == nullptr || isCellActive(x, y) || !c->isUnloadPending())
        return;

    sLogger.debug("Unloading Cell [{}][{}] on map {} (instance {})...", x, y, getBaseMap()->getMapId(), getInstanceId());

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

void WorldMap::updateInRangeSet(Object* obj, Player* plObj, MapCell* cell, std::unique_ptr<ByteBuffer>& buf)
{
    if (cell == nullptr)
        return;

    Player* plObj2;
    uint32_t count;
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
                        if (buf == nullptr)
                            buf = std::make_unique<ByteBuffer>(2500);

                        count = obj->buildCreateUpdateBlockForPlayer(buf.get(), plObj2);
                        plObj2->getUpdateMgr().pushCreationData(buf.get(), count);
                        plObj2->addVisibleObject(obj->getGuid());
                        buf->clear();
                    }
                }
                else if (curObj->isCreatureOrPlayer() && static_cast<Unit*>(curObj)->m_playerControler != nullptr)
                {
                    plObj2 = static_cast<Unit*>(curObj)->m_playerControler;

                    if (plObj2->canSee(obj) && !plObj2->isVisibleObject(obj->getGuid()))
                    {
                        if (buf == nullptr)
                            buf = std::make_unique<ByteBuffer>(2500);

                        count = obj->buildCreateUpdateBlockForPlayer(buf.get(), plObj2);
                        plObj2->getUpdateMgr().pushCreationData(buf.get(), count);
                        plObj2->addVisibleObject(obj->getGuid());
                        buf->clear();
                    }
                }

                if (plObj != nullptr)
                {
                    if (plObj->canSee(curObj) && !plObj->isVisibleObject(curObj->getGuid()))
                    {
                        if (buf == nullptr)
                            buf = std::make_unique<ByteBuffer>(2500);

                        count = curObj->buildCreateUpdateBlockForPlayer(buf.get(), plObj);
                        plObj->getUpdateMgr().pushCreationData(buf.get(), count);
                        plObj->addVisibleObject(curObj->getGuid());
                        buf->clear();
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
                        if (buf == nullptr)
                            buf = std::make_unique<ByteBuffer>(2500);

                        count = obj->buildCreateUpdateBlockForPlayer(buf.get(), plObj2);
                        plObj2->getUpdateMgr().pushCreationData(buf.get(), count);
                        plObj2->addVisibleObject(obj->getGuid());
                        buf->clear();
                    }
                }
                else if (curObj->isCreatureOrPlayer() && static_cast<Unit*>(curObj)->m_playerControler != nullptr)
                {
                    plObj2 = static_cast<Unit*>(curObj)->m_playerControler;
                    cansee = plObj2->canSee(obj);
                    isvisible = plObj2->isVisibleObject(obj->getGuid());
                    if (!cansee && isvisible)
                    {
                        plObj2->getUpdateMgr().pushOutOfRangeGuid(obj->GetNewGUID());
                        plObj2->removeVisibleObject(obj->getGuid());
                    }
                    else if (cansee && !isvisible)
                    {
                        if (buf == nullptr)
                            buf = std::make_unique<ByteBuffer>(2500);

                        count = obj->buildCreateUpdateBlockForPlayer(buf.get(), plObj2);
                        plObj2->getUpdateMgr().pushCreationData(buf.get(), count);
                        plObj2->addVisibleObject(obj->getGuid());
                        buf->clear();
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
                        if (buf == nullptr)
                            buf = std::make_unique<ByteBuffer>(2500);

                        count = curObj->buildCreateUpdateBlockForPlayer(buf.get(), plObj);
                        plObj->getUpdateMgr().pushCreationData(buf.get(), count);
                        plObj->addVisibleObject(curObj->getGuid());
                        buf->clear();
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

    if (obj->isPlayer())
        plObj = static_cast<Player*>(obj);
    if (obj->isCreatureOrPlayer() && static_cast<Unit*>(obj)->m_playerControler != nullptr)
        plObj = static_cast<Unit*>(obj)->m_playerControler;

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

                    if (curObj->isCreatureOrPlayer() && static_cast<Unit*>(curObj)->m_playerControler != nullptr)
                        static_cast<Unit*>(curObj)->m_playerControler->removeIfVisiblePushOutOfRange(obj->getGuid());

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
            sLogger.failure("WorldMap::ChangeObjectLocation not able to create object cell (nullptr), return!");
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
            obj->deactivate(this);

        if (pOldCell != nullptr)
            pOldCell->removeObject(obj);

        objCell->addObject(obj);
        obj->SetMapCell(objCell);

        // if player we need to update cell activity radius = 2 is used in order to update
        // both old and new cells
        if (obj->isPlayer() || obj->isCreatureOrPlayer() && static_cast<Unit*>(obj)->m_playerControler != nullptr)
        {
            // have to unlock/lock here to avoid a deadlock situation.
            updateCellActivity(cellX, cellY, 2U + cellNumber);
            if (pOldCell != nullptr)
            {
                // only do the second check if there's -/+ 2 difference
                if (abs(static_cast<int>(cellX) - static_cast<int>(pOldCell->_x)) > 2 + cellNumber ||
                    abs(static_cast<int>(cellY) - static_cast<int>(pOldCell->_y)) > 2 + cellNumber)
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

    std::unique_ptr<ByteBuffer> buf;
    for (uint32_t posX = startX; posX <= endX; ++posX)
    {
        for (uint32_t posY = startY; posY <= endY; ++posY)
        {
            MapCell* cell = getCell(posX, posY);
            if (cell)
                updateInRangeSet(obj, plObj, cell, buf);
        }
    }
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

Player* WorldMap::getPlayer(uint64_t guid)
{
    return getPlayer(static_cast<uint32_t>(guid));
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
            m_CreatureStorage.resize(newsize, nullptr);
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

Creature* WorldMap::createAndSpawnCreature(uint32_t pEntry, LocationVector pos)
{
    auto* creature = createCreature(pEntry);
    const auto* cp = sMySQLStore.getCreatureProperties(pEntry);
    if (cp == nullptr)
    {
        delete creature;
        return nullptr;
    }

    creature->Load(cp, pos.x, pos.y, pos.z, pos.o);
    creature->AddToWorld(this);
    return creature;
}

Creature* WorldMap::getCreature(uint32_t guid)
{
    if (guid == 0 || guid > m_CreatureHighGuid)
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

Summon* WorldMap::summonCreature(uint32_t entry, LocationVector pos, WDB::Structures::SummonPropertiesEntry const* properties /*= nullptr*/, uint32_t duration /*= 0*/, Object* summoner /*= nullptr*/, uint32_t spellId /*= 0*/)
{
    // Generate always a new guid for totems, otherwise the totem bar will get messed up
    const auto isTotemSummon = properties != nullptr &&
        (properties->ControlType == SUMMON_CONTROL_TYPE_WILD ||
            properties->ControlType == SUMMON_CONTROL_TYPE_GUARDIAN ||
            properties->ControlType == SUMMON_CATEGORY_UNK) &&
        properties->Type == SUMMONTYPE_TOTEM;
    uint64_t guid = generateCreatureGuid(entry, !isTotemSummon);

    // Phase
    uint32_t phase = 1;
    if (summoner)
        phase = summoner->GetPhase();

    Unit* summonerUnit = summoner ? summoner->ToUnit() : nullptr;

    Summon* summon = nullptr;
    if (properties)
    {
        switch (properties->ControlType)
        {
            case SUMMON_CONTROL_TYPE_PET: // Guardians
            {
                summon = new WildSummon(guid, properties);
            } break;
            case SUMMON_CONTROL_TYPE_POSSESSED:
            {
                summon = new PossessedSummon(guid, properties);
            } break;
            case SUMMON_CONTROL_TYPE_VEHICLE:
            {
                summon = new CompanionSummon(guid, properties);
            } break;
            case SUMMON_CONTROL_TYPE_WILD:
            case SUMMON_CONTROL_TYPE_GUARDIAN:
            case SUMMON_CATEGORY_UNK:
            {
                switch (properties->Type)
                {
                    case SUMMONTYPE_MINION:
                    case SUMMONTYPE_GUARDIAN:
                    case SUMMONTYPE_GUARDIAN2:
                    {
                        summon = new WildSummon(guid, properties);
                    } break;
                    case SUMMONTYPE_TOTEM:
                    case SUMMONTYPE_LIGHTWELL:
                    {
                        summon = new TotemSummon(guid, properties);
                    } break;
                    case SUMMONTYPE_VEHICLE:
                    case SUMMONTYPE_VEHICLE2:
                    {
                        summon = new Summon(guid, properties);
                    } break;
                    case SUMMONTYPE_MINIPET:
                    {
                        summon = new CompanionSummon(guid, properties);
                    } break;
                    default:
                    {
                        if (properties->Flags & 512) // Mirror Image, Summon Gargoyle
                            summon = new WildSummon(guid, properties);
                    } break;
                }
            } break;
            default:
                summon = new Summon(guid, properties);
                break;
        }
    }
    else
    {
        summon = new Summon(guid, properties);
    }

    const auto* cp = sMySQLStore.getCreatureProperties(entry);
    if (cp == nullptr)
    {
        delete summon;
        return nullptr;
    }

    summon->load(cp, summonerUnit, pos, duration, spellId);
    summon->setPhase(PHASE_SET, phase);
    summon->PushToWorld(this);
    // This is needed to CastSpells or Move Right at Spawn
    updateObjects();

    // Delay this a bit to make sure its Spawned
    sEventMgr.AddEvent(static_cast<Creature*>(summon), &Creature::InitSummon, static_cast<Object*>(summonerUnit), EVENT_UNK, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    return summon;
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
            m_GameObjectStorage.resize(newsize, nullptr);
        }

        GUID = m_GOHighGuid;
    }

    GameObject* gameobject = sObjectMgr.createGameObjectByGuid(entry, GUID);
    if (gameobject == nullptr)
        return nullptr;

    return gameobject;
}

GameObject* WorldMap::createAndSpawnGameObject(uint32_t entryID, LocationVector pos, float scale, uint32_t respawnTime)
{
    auto gameobject_info = sMySQLStore.getGameObjectProperties(entryID);
    if (gameobject_info == nullptr)
    {
        sLogger.debug("Error looking up entry in CreateAndSpawnGameObject");
        return nullptr;
    }

    sLogger.debug("CreateAndSpawnGameObject: By Entry '{}'", entryID);

    GameObject* go = createGameObject(entryID);

    // Setup game object
    go->create(entryID, this, go->GetPhase(), pos, QuaternionData(), GO_STATE_CLOSED, sObjectMgr.generateGameObjectSpawnId());
    go->setScale(scale);
    go->InitAI();
    go->PushToWorld(this);

    // Create spawn instance
    auto go_spawn = new MySQLStructure::GameobjectSpawn;
    go_spawn->entry = go->getEntry();
    go_spawn->id = go->getSpawnId();
    go_spawn->map = go->GetMapId();
    go_spawn->spawnPoint = go->GetPosition();
    go_spawn->rotation = go->getLocalRotation();
    go_spawn->state = GameObject_State(go->getState());
    go_spawn->phase = go->GetPhase();
    go_spawn->spawntimesecs = respawnTime;
    go->m_spawn = go_spawn;

    uint32_t cx = getPosX(pos.x);
    uint32_t cy = getPosY(pos.y);

    getBaseMap()->getSpawnsListAndCreate(cx, cy)->GameobjectSpawns.push_back(go_spawn);

    if (respawnTime)
        go->setRespawnTime(respawnTime);

    MapCell* mCell = getCell(cx, cy);

    if (mCell != nullptr)
        mCell->setLoaded();

    return go;
}

GameObject* WorldMap::getGameObject(uint32_t guid)
{
    if (guid == 0 || guid > m_GOHighGuid)
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

        if ((ZoneMask != ZONE_MASK_ALL && plr->getZoneId() != static_cast<uint32_t>(ZoneMask)))
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

        if ((p->getSession() != nullptr) && (p->getZoneId() == zone))
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
    auto result = CharacterDatabase.Query("SELECT type, spawnId, respawnTime FROM respawn WHERE mapId = %u AND instanceId = %u", getBaseMap()->getMapId(), getInstanceId());
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        SpawnObjectType type = SpawnObjectType(fields[0].asUint16());
        uint32_t spawnId = fields[1].asUint32();
        uint64_t respawnTime = fields[2].asUint64();

        if (type == SPAWN_TYPE_CREATURE)
        {
            const auto& creature_spawns = sMySQLStore._creatureSpawnsStore[getBaseMap()->getMapId()];
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
            const auto& gameobject_spawns = sMySQLStore._gameobjectSpawnsStore[getBaseMap()->getMapId()];
            if (gameobject_spawns.size())
            {
                for (auto const& spawn : gameobject_spawns)
                {
                    if (spawn->id == spawnId)
                        saveRespawnTime(type, spawnId, spawn->entry, time_t(respawnTime), spawn->spawnPoint.x, spawn->spawnPoint.y, true);
                }
            }
        }
        else
        {
            sLogger.debug("Loading saved respawn time of %" PRIu64 " for spawnid ({},{}) - invalid spawn type, ignoring", respawnTime, uint32_t(type), spawnId);
        }

    } while (result->NextRow());
}

RespawnInfoMap& WorldMap::getRespawnMapForType(SpawnObjectType type)
{
    if (type == SPAWN_TYPE_CREATURE)
        return _creatureRespawnTimesBySpawnId;

    return _gameObjectRespawnTimesBySpawnId;
}

RespawnInfoMap const& WorldMap::getRespawnMapForType(SpawnObjectType type) const
{
    if (type == SPAWN_TYPE_CREATURE)
        return _creatureRespawnTimesBySpawnId;

    return _gameObjectRespawnTimesBySpawnId;
}

time_t WorldMap::getRespawnTime(SpawnObjectType type, uint32_t spawnId) const
{
    auto const& map = getRespawnMapForType(type);
    auto it = map.find(spawnId);
    return (it == map.end()) ? 0 : it->second->time;
}

void WorldMap::saveRespawnTime(SpawnObjectType type, uint32_t spawnId, uint32_t entry, time_t respawnTime, float cellX, float cellY, bool startup)
{
    if (!respawnTime)
    {
        // Delete only
        removeRespawnTime(type, spawnId);
        return;
    }

    RespawnInfo ri{};
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
            sLogger.failure("Attempt to load saved respawn %" PRIu64 " for ({},{}) failed - duplicate respawn? Skipped.", respawnTime, uint32_t(type), spawnId);
    }
    else if (success)
    {
        saveRespawnDB(ri);
    }
}

void WorldMap::saveRespawnDB(RespawnInfo const& info)
{
    CharacterDatabase.Execute("REPLACE INTO respawn (type, spawnId, respawnTime, mapId, instanceId) VALUES (%u, %u, %u, %u, %u)", info.type, info.spawnId, uint64_t(info.time), getBaseMap()->getMapId(), getInstanceId());
}

bool WorldMap::addRespawn(RespawnInfo const& info)
{
    if (!info.spawnId)
    {
        sLogger.failure("Attempt to insert respawn info for zero spawn id (type {})", uint32_t(info.type));
        return false;
    }

    RespawnInfoMap& bySpawnIdMap = getRespawnMapForType(info.type);

    // check if we already have the maximum possible number of respawns scheduled
    if (info.type == SPAWN_TYPE_CREATURE || info.type == SPAWN_TYPE_GAMEOBJECT)
    {
        auto it = bySpawnIdMap.find(info.spawnId);
        if (it != bySpawnIdMap.end()) // spawnid already has a respawn scheduled
        {
            RespawnInfo const* existing = it->second;
            if (info.time <= existing->time) // delete existing in this case
                deleteRespawn(existing);
            else
                return false;
        }
    }
    else
    {
        sLogger.failure("Invalid respawn info for spawn id ({},{}) being inserted", uint32_t(info.type), info.spawnId);
    }

    auto ri = std::make_unique<RespawnInfo>(info);
    bySpawnIdMap.emplace(ri->spawnId, ri.get());
    _respawnTimes.emplace(std::move(ri));
    return true;
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

void WorldMap::deleteRespawn(RespawnInfo const* info)
{
    // Delete from all relevant containers to ensure consistency
    ASSERT(info);

    // spawnid store
    auto& spawnMap = getRespawnMapForType(info->type);
    auto range = spawnMap.equal_range(info->spawnId);
    auto it = std::find_if(range.first, range.second, [info](RespawnInfoMap::value_type const& pair) { return (pair.second == info); });
    ASSERT(it != range.second);
    spawnMap.erase(it);

    // database
    deleteRespawnFromDB(info->type, info->spawnId);

    // respawn heap and cleanup the object
    _respawnTimes.remove(info);
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
        {
            sLogger.failure("Invalid spawn type {} with spawnId {} on map {}", uint32_t(info->type), info->spawnId, getBaseMap()->getMapId());
            return true;
        }
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
    if (cell == nullptr) //cell got deleted while waiting for respawn.
        return;

    switch (type)
    {
        case SPAWN_TYPE_CREATURE:
        {
            if (object)
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
            }
            break;
        }
        case SPAWN_TYPE_GAMEOBJECT:
        {
            if (object)
            {
                GameObject* obj = object->ToGameObject();
                if (obj)
                {
                    auto itr = cell->_respawnObjects.find(obj);
                    if (itr != cell->_respawnObjects.end())
                    {
                        obj->m_respawnCell = nullptr;
                        cell->_respawnObjects.erase(itr);
                        obj->PushToWorld(this);
                    }
                }
            }
            else
            {
                for (auto& GameobjectSpawn : sMySQLStore._gameobjectSpawnsStore[getBaseMap()->getMapId()])
                {
                    if (GameobjectSpawn && GameobjectSpawn->id == spawnId)
                    {
                        GameObject* obj = createGameObject(GameobjectSpawn->entry);
                        if (!obj->loadFromDB(GameobjectSpawn, this, true))
                            delete obj;
                    }
                }
            }
            break;
        }
        default:
        {
            sLogger.failure("Invalid spawn type {} (spawnid {}) on map {}", static_cast<uint32_t>(type), spawnId, getBaseMap()->getMapId());
        }
    }
}

void WorldMap::addCorpseDespawn(uint64_t guid, time_t time)
{
    const auto now = Util::getTimeNow();
    _corpseDespawnTimes.emplace(now + time, guid);
}

void WorldMap::updateObjects()
{
    {
        std::scoped_lock<std::mutex> lock(m_updateMutex);

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
                    else if (pObj->isCreatureOrPlayer() && static_cast<Unit*>(pObj)->m_playerControler != nullptr)
                    {
                        count = pObj->BuildValuesUpdateBlockForPlayer(&update, static_cast<Unit*>(pObj)->m_playerControler);
                        if (count)
                        {
                            static_cast<Unit*>(pObj)->m_playerControler->getUpdateMgr().pushUpdateData(&update, count);
                            update.clear();
                        }
                    }

                    // build the update
                    count = pObj->BuildValuesUpdateBlockForPlayer(&update, static_cast<Player*>(nullptr));
                    update.clear();

                    if (count)
                    {
                        for (const auto& itr : pObj->getInRangePlayersSet())
                        {
                            Player* lplr = static_cast<Player*>(itr);

                            // Make sure that the target player can see us.
                            if (lplr && lplr->isVisibleObject(pObj->getGuid()))
                            {
                                // Build correct update to each player
                                // Data may differ from player to player
                                pObj->BuildValuesUpdateBlockForPlayer(&update, lplr);
                                lplr->getUpdateMgr().pushUpdateData(&update, count);
                                update.clear();
                            }
                        }
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
    return ScriptInterface.get();
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
    std::scoped_lock<std::mutex> lock(m_transportsLock);

    m_TransportStorage.insert(obj);
    return true;
}

void WorldMap::removeFromMapMgr(Transporter* obj)
{
    std::scoped_lock<std::mutex> lock(m_transportsLock);

    m_TransportStorage.erase(obj);
    sTransportHandler.removeInstancedTransport(obj, getInstanceId());
}

void WorldMap::markDelayedRemoveFor(Transporter* transport, bool removeFromMap)
{
    std::scoped_lock<std::mutex> lock(m_delayedTransportLock);
    auto itr = m_TransportDelayedRemoveStorage.find(transport);
    if (itr != m_TransportDelayedRemoveStorage.end())
    {
        // If boolean is already set to true, do not set it to false
        if (!itr->second)
            itr->second = removeFromMap;

        return;
    }

    m_TransportDelayedRemoveStorage.insert(std::make_pair(transport, removeFromMap));
}

void WorldMap::removeDelayedRemoveFor(Transporter* transport)
{
    std::scoped_lock<std::mutex> lock(m_delayedTransportLock);
    m_TransportDelayedRemoveStorage.erase(transport);
}

void WorldMap::objectUpdated(Object* obj)
{
    // set our fields to dirty stupid fucked up code in places.. i hate doing this but i've got to :<- burlex
    std::scoped_lock<std::mutex> lock(m_updateMutex);
    _updates.insert(obj);
}

float WorldMap::getUpdateDistance(Object* curObj, Object* obj, Player* plObj)
{
    static float no_distance = 0.0f;

    // unlimited distance for people on same boat
#if VERSION_STRING < Cata
    if (curObj->isPlayer() && obj->isPlayer() && plObj != nullptr && plObj->obj_movement_info.hasMovementFlag(MOVEFLAG_TRANSPORT) && plObj->obj_movement_info.transport_guid == curObj->obj_movement_info.transport_guid)
        return no_distance;
#else
    if (curObj->isPlayer() && obj->isPlayer() && plObj != nullptr && plObj->obj_movement_info.transport_guid == curObj->obj_movement_info.transport_guid)
        return no_distance;
#endif
    // unlimited distance for transporters (only up to 2 cells +/- anyway.)
    if (curObj->GetTypeFromGUID() == HIGHGUID_TYPE_TRANSPORTER)
        return no_distance;

     // unlimited distance for Destructible Buildings (only up to 2 cells +/- anyway.)
    if (curObj->isGameObject() && (static_cast<GameObject*>(curObj)->getGoType() == GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING))
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

WDB::Structures::MapDifficulty const* WorldMap::getMapDifficulty()
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

bool WorldMap::getAreaInfo(uint32_t /*phaseMask*/, LocationVector pos, uint32_t& flags, int32_t& adtId, int32_t& rootId, int32_t& groupId)
{
    float vmap_z = pos.z;
    float dynamic_z = pos.z;
    float check_z = pos.z;
    const auto vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    uint32_t vflags;
    int32_t vadtId;
    int32_t vrootId;
    int32_t vgroupId;
    uint32_t dflags;
    int32_t dadtId;
    int32_t drootId;
    int32_t dgroupId;

    bool hasVmapAreaInfo = vmgr->getAreaInfo(getBaseMap()->getMapId(), pos.x, pos.y, vmap_z, vflags, vadtId, vrootId, vgroupId);
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
        if (TerrainTile* gmap = getTerrain()->getTile(pos.x, pos.y))
        {
            float mapHeight = gmap->m_map.getHeight(pos.x, pos.y);
            // z + 2.0f condition taken from getHeight(), not sure if it's such a great choice...
            if (pos.z + 2.0f > mapHeight && mapHeight > check_z)
                return false;
        }
        return true;
    }
    return false;
}

uint32_t WorldMap::getAreaId(uint32_t phaseMask, LocationVector const& pos)
{
    uint32_t mogpFlags;
    int32_t adtId, rootId, groupId;
    float vmapZ = pos.z;
    bool hasVmapArea = getAreaInfo(phaseMask, LocationVector(pos.x, pos.y, vmapZ), mogpFlags, adtId, rootId, groupId);

    uint32_t gridAreaId = 0;
    float gridMapHeight = INVALID_HEIGHT;
    if (TerrainTile* gmap = getTerrain()->getTile(pos.x, pos.y))
    {
        gridAreaId = gmap->m_map.getArea(pos.x, pos.y);
        gridMapHeight = gmap->m_map.getHeight(pos.x, pos.y);
    }

    uint32_t areaId = 0;

    // floor is the height we are closer to (but only if above)
    if (hasVmapArea && G3D::fuzzyGe(pos.z, vmapZ - GROUND_HEIGHT_TOLERANCE) && (G3D::fuzzyLt(pos.z, gridMapHeight - GROUND_HEIGHT_TOLERANCE) || vmapZ > gridMapHeight))
    {
        // wmo found
        if (WDB::Structures::WMOAreaTableEntry const* wmoEntry = GetWMOAreaTableEntryByTriple(rootId, adtId, groupId))
            areaId = wmoEntry->areaId;

        if (!areaId)
            areaId = gridAreaId;
    }
    else
    {
        areaId = gridAreaId;
    }

    if (!areaId)
        areaId = getBaseMap()->getMapEntry()->linked_zone;

    return areaId;
}

uint32_t WorldMap::getZoneId(uint32_t phaseMask, LocationVector const& pos)
{
    uint32_t areaId = 0;
    if (const auto* area = MapManagement::AreaManagement::AreaStorage::getExactArea(this, pos, phaseMask))
    {
        areaId = area->id;
        if (area->zone)
            return area->zone;
    }

    return areaId;
}

void WorldMap::getZoneAndAreaId(uint32_t phaseMask, uint32_t& zoneid, uint32_t& areaid, LocationVector const& pos)
{
    if (const auto* area = MapManagement::AreaManagement::AreaStorage::getExactArea(this, pos, phaseMask))
    {
        areaid = area->id;
        if (area->zone)
            zoneid = area->zone;
    }
}

static inline bool isInWMOInterior(uint32_t mogpFlags)
{
    return (mogpFlags & 0x2000) != 0;
}

ZLiquidStatus WorldMap::getLiquidStatus(uint32_t phaseMask, LocationVector pos, uint8_t ReqLiquidType, LiquidData* data, float collisionHeight)
{
    ZLiquidStatus result = LIQUID_MAP_NO_WATER;
    const auto vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    float liquid_level = INVALID_HEIGHT;
    float ground_level = INVALID_HEIGHT;
    uint32_t liquid_type = 0;
    uint32_t mogpFlags = 0;
    bool useGridLiquid = true;
    if (getBaseMap() && vmgr->getLiquidLevel(getBaseMap()->getMapId(), pos.x, pos.y, pos.z, ReqLiquidType, liquid_level, ground_level, liquid_type, mogpFlags))
    {
        useGridLiquid = !isInWMOInterior(mogpFlags);
        // Check water level and ground level
        if (liquid_level > ground_level && G3D::fuzzyGe(pos.z, ground_level - GROUND_HEIGHT_TOLERANCE))
        {
            // All ok in water -> store data
            if (data)
            {
                // hardcoded in client like this
                if (getBaseMap()->getMapId() == 530 && liquid_type == 2)
                    liquid_type = 15;

                uint32_t liquidFlagType = 0;
                if (WDB::Structures::LiquidTypeEntry const* liq = sLiquidTypeStore.lookupEntry(liquid_type))
                    liquidFlagType = liq->Type;

                if (liquid_type && liquid_type < 21)
                {
                    if (const auto* area = MapManagement::AreaManagement::AreaStorage::getExactArea(this, pos, phaseMask))
                    {
#if VERSION_STRING > Classic
                        uint32_t overrideLiquid = area->liquid_type_override[liquidFlagType];
                        if (!overrideLiquid && area->zone)
                        {
                            area = MapManagement::AreaManagement::AreaStorage::GetAreaById(area->zone);
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

                        if (WDB::Structures::LiquidTypeEntry const* liq = sLiquidTypeStore.lookupEntry(overrideLiquid))
                        {
                            liquid_type = overrideLiquid;
                            liquidFlagType = liq->Type;
                        }
                    }
                }

                data->level = liquid_level;
                data->depth_level = ground_level;

                data->entry = liquid_type;
                data->type_flags = 1U << liquidFlagType;
            }

            float delta = liquid_level - pos.z;

            // Get position delta
            if (delta > collisionHeight)        // Under water
                return LIQUID_MAP_UNDER_WATER;
            if (delta > 0.0f)                   // In water
                return LIQUID_MAP_IN_WATER;
            if (delta > -0.1f)                  // Walk on water
                return LIQUID_MAP_WATER_WALK;
            result = LIQUID_MAP_ABOVE_WATER;
        }
    }

    if (useGridLiquid)
    {
        if (TerrainTile* gmap = getTerrain()->getTile(pos.x, pos.y))
        {
            LiquidData map_data;
            ZLiquidStatus map_result = gmap->m_map.getLiquidStatus(pos, ReqLiquidType, &map_data, collisionHeight);
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

void WorldMap::getFullTerrainStatusForPosition(uint32_t phaseMask, float x, float y, float z, PositionFullTerrainStatus& data, uint8_t reqLiquidType, float collisionHeight) const
{
    if (!getTerrain())
        return;

    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    VMAP::AreaAndLiquidData vmapData;
    VMAP::AreaAndLiquidData dynData;
    VMAP::AreaAndLiquidData* wmoData = nullptr;

    TerrainTile* gmap = getTerrain()->getTile(x, y);

    vmgr->getAreaAndLiquidData(getBaseMap()->getMapId(), x, y, z, reqLiquidType, vmapData);
    _dynamicTree.getAreaAndLiquidData(x, y, z, phaseMask, reqLiquidType, dynData);

    uint32_t gridAreaId = 0;
    float gridMapHeight = INVALID_HEIGHT;
    if (gmap)
    {
        gridAreaId = gmap->m_map.getArea(x, y);
        gridMapHeight = gmap->m_map.getHeight(x, y);
    }

    bool useGridLiquid = true;

    // floorZ is the height we are closer to example we stand on an wmo
    data.floorZ = VMAP_INVALID_HEIGHT;
    if (gridMapHeight > INVALID_HEIGHT && G3D::fuzzyGe(z, gridMapHeight - GROUND_HEIGHT_TOLERANCE))
        data.floorZ = gridMapHeight;
    if (vmapData.floorZ > VMAP_INVALID_HEIGHT &&
        G3D::fuzzyGe(z, vmapData.floorZ - GROUND_HEIGHT_TOLERANCE) &&
        (G3D::fuzzyLt(z, gridMapHeight - GROUND_HEIGHT_TOLERANCE) || vmapData.floorZ > gridMapHeight))
    {
        data.floorZ = vmapData.floorZ;
        wmoData = &vmapData;
    }

    // When spawning and despawning Wmos the provided area/liquid from beneath them wont get detected properly
    // example: Lich King platform
    if (dynData.floorZ > VMAP_INVALID_HEIGHT &&
        G3D::fuzzyGe(z, dynData.floorZ - GROUND_HEIGHT_TOLERANCE) &&
        (G3D::fuzzyLt(z, gridMapHeight - GROUND_HEIGHT_TOLERANCE) || dynData.floorZ > gridMapHeight) &&
        (G3D::fuzzyLt(z, vmapData.floorZ - GROUND_HEIGHT_TOLERANCE) || dynData.floorZ > vmapData.floorZ))
    {
        data.floorZ = dynData.floorZ;
        wmoData = &dynData;
    }

    if (wmoData)
    {
        if (wmoData->areaInfo)
        {
            data.areaInfo.emplace(wmoData->areaInfo->adtId, wmoData->areaInfo->rootId, wmoData->areaInfo->groupId, wmoData->areaInfo->mogpFlags);
            // wmo found
            WDB::Structures::WMOAreaTableEntry const* wmoEntry = GetWMOAreaTableEntryByTriple(wmoData->areaInfo->rootId, wmoData->areaInfo->adtId, wmoData->areaInfo->groupId);
            data.outdoors = (wmoData->areaInfo->mogpFlags & 0x8) != 0;
            if (wmoEntry)
            {
                data.areaId = wmoEntry->areaId;
                if (wmoEntry->flags & 4)
                    data.outdoors = true;
                else if (wmoEntry->flags & 2)
                    data.outdoors = false;
            }

            if (!data.areaId)
                data.areaId = gridAreaId;

            useGridLiquid = !isInWMOInterior(wmoData->areaInfo->mogpFlags);
        }
    }
    else
    {
        data.outdoors = true;
        data.areaId = gridAreaId;
        if (WDB::Structures::AreaTableEntry const* areaEntry = sAreaStore.lookupEntry(data.areaId))
            data.outdoors = (areaEntry->flags & (MapManagement::AreaManagement::AreaFlags::AREA_FLAG_INSIDE | MapManagement::AreaManagement::AreaFlags::AREA_FLAG_OUTSIDE)) != MapManagement::AreaManagement::AreaFlags::AREA_FLAG_INSIDE;
    }

    if (!data.areaId)
        data.areaId = getBaseMap()->getMapEntry()->linked_zone;

    WDB::Structures::AreaTableEntry const* areaEntry = sAreaStore.lookupEntry(data.areaId);

    // liquid processing
    data.liquidStatus = LIQUID_MAP_NO_WATER;
    if (wmoData && wmoData->liquidInfo && wmoData->liquidInfo->level > wmoData->floorZ)
    {
        uint32_t liquidType = wmoData->liquidInfo->type;
        if (getBaseMap()->getMapId() == 530 && liquidType == 2) // gotta love blizzard hacks
            liquidType = 15;

        uint32_t liquidFlagType = 0;
        if (WDB::Structures::LiquidTypeEntry const* liquidData = sLiquidTypeStore.lookupEntry(liquidType))
            liquidFlagType = liquidData->Type;

        if (liquidType && liquidType < 21 && areaEntry)
        {
#if VERSION_STRING > Classic
            uint32_t overrideLiquid = areaEntry->liquid_type_override[liquidFlagType];
            if (!overrideLiquid && areaEntry->zone)
            {
                WDB::Structures::AreaTableEntry const* zoneEntry = sAreaStore.lookupEntry(areaEntry->zone);
                if (zoneEntry)
                    overrideLiquid = zoneEntry->liquid_type_override[liquidFlagType];
            }
#else
            uint32_t overrideLiquid = areaEntry->liquid_type_override;
            if(!overrideLiquid && areaEntry->zone)
            {
                WDB::Structures::AreaTableEntry const* zoneEntry = sAreaStore.lookupEntry(areaEntry->zone);
                if (zoneEntry)
                    overrideLiquid = zoneEntry->liquid_type_override;
            }
#endif
            if (WDB::Structures::LiquidTypeEntry const* overrideData = sLiquidTypeStore.lookupEntry(overrideLiquid))
            {
                liquidType = overrideLiquid;
                liquidFlagType = overrideData->Type;
            }
        }

        data.liquidInfo.emplace();
        data.liquidInfo->level = wmoData->liquidInfo->level;
        data.liquidInfo->depth_level = wmoData->floorZ;
        data.liquidInfo->entry = liquidType;
        data.liquidInfo->type_flags = 1 << liquidFlagType;

        float delta = wmoData->liquidInfo->level - z;
        if (delta > collisionHeight)
            data.liquidStatus = LIQUID_MAP_UNDER_WATER;
        else if (delta > 0.0f)
            data.liquidStatus = LIQUID_MAP_IN_WATER;
        else if (delta > -0.1f)
            data.liquidStatus = LIQUID_MAP_WATER_WALK;
        else
            data.liquidStatus = LIQUID_MAP_ABOVE_WATER;
    }
    // look up liquid data from grid map
    if (gmap && useGridLiquid)
    {
        LiquidData gridMapLiquid;
        ZLiquidStatus gridMapStatus = gmap->m_map.getLiquidStatus(LocationVector(x, y, z), reqLiquidType, &gridMapLiquid, collisionHeight);
        if (gridMapStatus != LIQUID_MAP_NO_WATER && (!wmoData || gridMapLiquid.level > wmoData->floorZ))
        {
            if (getBaseMap()->getMapId() == 530 && gridMapLiquid.entry == 2)
                gridMapLiquid.entry = 15;
            data.liquidInfo = gridMapLiquid;
            data.liquidStatus = gridMapStatus;
        }
    }
}

float WorldMap::getWaterLevel(float x, float y)
{
    if (TerrainTile* gmap = getTerrain()->getTile(x, y))
        return gmap->m_map.getLiquidLevel(x, y);
    else
        return 0;
}

bool WorldMap::isInWater(uint32_t phaseMask, LocationVector pos, LiquidData* data)
{
    LiquidData liquid_status{};
    LiquidData* liquid_ptr = data ? data : &liquid_status;
    return (getLiquidStatus(phaseMask, pos, MAP_ALL_LIQUIDS, liquid_ptr) & (LIQUID_MAP_IN_WATER | LIQUID_MAP_UNDER_WATER)) != 0;
}

bool WorldMap::isUnderWater(uint32_t phaseMask, LocationVector pos)
{
    return (getLiquidStatus(phaseMask, pos, MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN) & LIQUID_MAP_UNDER_WATER) != 0;
}

bool WorldMap::isInLineOfSight(LocationVector pos1, LocationVector pos2, uint32_t phasemask, LineOfSightChecks checks)
{
    if ((checks & LINEOFSIGHT_CHECK_VMAP)
        && !VMAP::VMapFactory::createOrGetVMapManager()->isInLineOfSight(getBaseMap()->getMapId(), pos1.x, pos1.y, pos1.z, pos2.x, pos2.y, pos2.z))
        return false;

    if (checks & LINEOFSIGHT_CHECK_GOBJECT && !_dynamicTree.isInLineOfSight(pos1.x, pos1.y, pos1.z, pos2.x, pos2.y, pos2.z, phasemask))
        return false;

    return true;
}

bool WorldMap::getObjectHitPos(uint32_t phasemask, LocationVector pos1, LocationVector pos2, float& rx, float& ry, float& rz, float modifyDist)
{
    G3D::Vector3 startPos(pos1.x, pos1.y, pos1.z);
    G3D::Vector3 dstPos(pos2.x, pos2.y, pos2.z);

    G3D::Vector3 resultPos;
    bool result = _dynamicTree.getObjectHitPos(phasemask, startPos, dstPos, resultPos, modifyDist);

    rx = resultPos.x;
    ry = resultPos.y;
    rz = resultPos.z;
    return result;
}

float WorldMap::getGameObjectFloor(uint32_t phasemask, LocationVector pos, float maxSearchDist /*= 50.0f*/) const
{
    return _dynamicTree.getHeight(pos.x, pos.y, pos.z, maxSearchDist, phasemask);
}

float WorldMap::getHeight(uint32_t phasemask, LocationVector const& pos, bool vmap /*= true*/, float maxSearchDist /*= 50.0f*/) const
{
    return std::max<float>(getHeight(pos, vmap, maxSearchDist), getGameObjectFloor(phasemask, pos, maxSearchDist));
}


float WorldMap::getWaterOrGroundLevel(uint32_t phasemask, LocationVector const& pos, float* ground /*= nullptr*/, bool /*swim = false*/, float collisionHeight /*= 2.03128f*/)
{
    if (getTerrain()->getTile(pos.x, pos.y))
    {
        // we need ground level (including grid height version) for proper return water level in point
        float ground_z = getHeight(phasemask, LocationVector(pos.x, pos.y, pos.z + collisionHeight), true, 50.0f);
        if (ground)
            *ground = ground_z;

        LiquidData liquid_status;

        ZLiquidStatus res = getLiquidStatus(phasemask, LocationVector(pos.x, pos.y, ground_z), MAP_ALL_LIQUIDS, &liquid_status, collisionHeight);
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

float WorldMap::getHeight(LocationVector const& pos, bool checkVMap /*= true*/, float maxSearchDist /*= 50.0f*/) const
{
    // find raw .map surface under Z coordinates
    float mapHeight = VMAP_INVALID_HEIGHT_VALUE;
    float gridHeight = getGridHeight(pos.x, pos.y);
    if (G3D::fuzzyGe(pos.z, gridHeight - GROUND_HEIGHT_TOLERANCE))
        mapHeight = gridHeight;

    float vmapHeight = VMAP_INVALID_HEIGHT_VALUE;
    if (checkVMap)
    {
        const auto vmgr = VMAP::VMapFactory::createOrGetVMapManager();
        if (vmgr->isHeightCalcEnabled())
            vmapHeight = vmgr->getHeight(getBaseMap()->getMapId(), pos.x, pos.y, pos.z, maxSearchDist);
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
            if (vmapHeight > mapHeight || std::fabs(mapHeight - pos.z) > std::fabs(vmapHeight - pos.z))
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
        {
            // Prevent Deletion of Current Update Iterator
            if (creature_iterator != activeCreatures.end() && (*creature_iterator)->getGuid() == obj->getGuid())
                ++creature_iterator;

            activeCreatures.erase(static_cast<Creature*>(obj));
        } break;
        case TYPEID_GAMEOBJECT:
        {
            // Prevent Deletion of Current Update Iterator
            if (gameObject_iterator != activeGameObjects.end() && (*gameObject_iterator)->getGuid() == obj->getGuid())
                ++gameObject_iterator;

            activeGameObjects.erase(static_cast<GameObject*>(obj));
        } break;
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
        for (auto& spawns : data->spawns)
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
