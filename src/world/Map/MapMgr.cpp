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

#include "StdAfx.h"

#include "TLSObject.h"
#include "Objects/DynamicObject.h"
#include "CellHandler.h"
#include "CThreads.h"
#include "Management/WorldStatesHandler.h"
#include "Management/Item.h"
#include "CrashHandler.h"
#include "Units/Summons/Summon.h"
#include "Units/Summons/GuardianSummon.h"
#include "Units/Summons/WildSummon.h"
#include "Units/Summons/TotemSummon.h"
#include "Units/Summons/PossessedSummon.h"
#include "Units/Summons/CompanionSummon.h"
#include "Units/Unit.h"
#include "VMapFactory.h"
#include "MMapFactory.h"
#include "Storage/MySQLDataStore.hpp"
#include "MapMgr.h"
#include "MapScriptInterface.h"
#include "WorldCreatorDefines.hpp"
#include "WorldCreator.h"
#include "Units/Creatures/Pet.h"


Arcemu::Utility::TLSObject<MapMgr*> t_currentMapContext;

#define MAP_MGR_UPDATE_PERIOD 100
#define MAPMGR_INACTIVE_MOVE_TIME 30

#define Z_SEARCH_RANGE 2

extern bool bServerShutdown;

MapMgr::MapMgr(Map* map, uint32 mapId, uint32 instanceid) : CellHandler<MapCell>(map), _mapId(mapId), eventHolder(instanceid), worldstateshandler(mapId)
{
    _terrain = new TerrainHolder(mapId);
    _shutdown = false;
    m_instanceID = instanceid;
    pMapInfo = sMySQLStore.getWorldMapInfo(mapId);
    m_UpdateDistance = pMapInfo->update_distance * pMapInfo->update_distance;
    iInstanceMode = 0;

    // Create script interface
    ScriptInterface = new MapScriptInterface(*this);

    // Set up storage arrays
    CreatureStorage.resize(map->CreatureSpawnCount, NULL);
    GOStorage.resize(map->GameObjectSpawnCount, NULL);

    m_GOHighGuid = m_CreatureHighGuid = 0;
    m_DynamicObjectHighGuid = 0;
    lastUnitUpdate = Util::getMSTime();
    lastGameobjectUpdate = Util::getMSTime();
    m_battleground = NULL;

    m_holder = &eventHolder;
    m_event_Instanceid = eventHolder.GetInstanceID();
    forced_expire = false;
    InactiveMoveTime = 0;
    mLoopCounter = 0;
    pInstance = nullptr;
    thread_kill_only = false;
    thread_running = false;

    m_forcedcells.clear();
    m_PlayerStorage.clear();
    m_PetStorage.clear();
    m_DynamicObjectStorage.clear();

    _combatProgress.clear();
    _mapWideStaticObjects.clear();
    //_worldStateSet.clear();
    _updates.clear();
    _processQueue.clear();
    Sessions.clear();

    activeGameObjects.clear();
    activeCreatures.clear();
    creature_iterator = activeCreatures.begin();
    pet_iterator = m_PetStorage.begin();
    m_corpses.clear();
    _sqlids_creatures.clear();
    _sqlids_gameobjects.clear();
    _reusable_guids_gameobject.clear();
    _reusable_guids_creature.clear();

    mInstanceScript = NULL;
}

MapMgr::~MapMgr()
{
    _shutdown = true;
    sEventMgr.RemoveEvents(this);
    if (ScriptInterface != NULL)
    {
        delete ScriptInterface;
        ScriptInterface = NULL;
    }

    delete _terrain;

    // Remove objects
    if (_cells)
    {
        for (uint32 i = 0; i < _sizeX; i++)
        {
            if (_cells[i] != 0)
            {
                for (uint32 j = 0; j < _sizeY; j++)
                {
                    if (_cells[i][j] != 0)
                    {
                        _cells[i][j]->_unloadpending = false;
                        _cells[i][j]->RemoveObjects();
                    }
                }
            }
        }
    }

    for (std::set<Object*>::iterator itr = _mapWideStaticObjects.begin(); itr != _mapWideStaticObjects.end(); ++itr)
    {
        if ((*itr)->IsInWorld())
            (*itr)->RemoveFromWorld(false);
        delete(*itr);
    }
    _mapWideStaticObjects.clear();

    GOStorage.clear();
    CreatureStorage.clear();

    Corpse* pCorpse;
    for (std::set<Corpse*>::iterator itr = m_corpses.begin(); itr != m_corpses.end();)
    {
        pCorpse = *itr;
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

    _combatProgress.clear();
    _updates.clear();
    _processQueue.clear();
    Sessions.clear();

    activeCreatures.clear();
    activeGameObjects.clear();
    _sqlids_creatures.clear();
    _sqlids_gameobjects.clear();
    _reusable_guids_creature.clear();
    _reusable_guids_gameobject.clear();

    if (m_battleground)
    {
        m_battleground = NULL;
    }

    MMAP::MMapFactory::createOrGetMMapManager()->unloadMapInstance(GetMapId(), m_instanceID);

    LogDebugFlag(LF_MAP, "MapMgr : Instance %u shut down. (%s)", m_instanceID, GetBaseMap()->GetMapName().c_str());
}

void MapMgr::PushObject(Object* obj)
{
    // Assertions
    ARCEMU_ASSERT(obj != nullptr);

    ///\todo That object types are not map objects. TODO: add AI groups here?
    if (obj->IsItem() || obj->IsContainer())
    {
        // mark object as updatable and exit
        return;
    }

    if (obj->IsCorpse())
    {
        m_corpses.insert(static_cast< Corpse* >(obj));
    }

    obj->ClearInRangeSet();

    // Check valid cell x/y values
    ARCEMU_ASSERT(obj->GetMapId() == _mapId);
    if (!(obj->GetPositionX() < _maxX && obj->GetPositionX() > _minX) || !(obj->GetPositionY() < _maxY && obj->GetPositionY() > _minY))
    {
        OutOfMapBoundariesTeleport(obj);
    }

    ARCEMU_ASSERT(obj->GetPositionY() < _maxY && obj->GetPositionY() > _minY);
    ARCEMU_ASSERT(_cells != nullptr);

    // Get cell coordinates
    uint32 x = GetPosX(obj->GetPositionX());
    uint32 y = GetPosY(obj->GetPositionY());

    if (x >= _sizeX || y >= _sizeY)
    {
        OutOfMapBoundariesTeleport(obj);

        x = GetPosX(obj->GetPositionX());
        y = GetPosY(obj->GetPositionY());
    }

    MapCell* objCell = GetCell(x, y);
    if (objCell == nullptr)
    {
        objCell = Create(x, y);
        objCell->Init(x, y, this);
    }
    ARCEMU_ASSERT(objCell != nullptr);

    // Build update-block for player
    ByteBuffer* buf = 0;
    uint32 count;
    Player* plObj = nullptr;

    if (obj->IsPlayer())
    {
        plObj = static_cast<Player*>(obj);

        LogDebugFlag(LF_MAP, "Creating player " I64FMT " for himself.", obj->GetGUID());
        ByteBuffer pbuf(10000);
        count = plObj->BuildCreateUpdateBlockForPlayer(&pbuf, plObj);
        plObj->PushCreationData(&pbuf, count);
    }

    // Build in-range data
    uint8 cellNumber = worldConfig.server.mapCellNumber;

    uint32 endX = (x <= _sizeX) ? x + cellNumber : (_sizeX - cellNumber);
    uint32 endY = (y <= _sizeY) ? y + cellNumber : (_sizeY - cellNumber);
    uint32 startX = x > 0 ? x - cellNumber : 0;
    uint32 startY = y > 0 ? y - cellNumber : 0;

    for (uint32 posX = startX; posX <= endX; posX++)
    {
        for (uint32 posY = startY; posY <= endY; posY++)
        {
            MapCell* cell = GetCell(posX, posY);
            if (cell)
            {
                UpdateInRangeSet(obj, plObj, cell, &buf);
            }
        }
    }

    //Add to the cell's object list
    objCell->AddObject(obj);

    obj->SetMapCell(objCell);
    //Add to the mapmanager's object list
    if (plObj != nullptr)
    {
        m_PlayerStorage[plObj->GetLowGUID()] = plObj;
        UpdateCellActivity(x, y, 2 + cellNumber);
    }
    else
    {
        switch (obj->GetTypeFromGUID())
        {
            case HIGHGUID_TYPE_PET:
                m_PetStorage[obj->GetUIdFromGUID()] = static_cast< Pet* >(obj);
                break;

            case HIGHGUID_TYPE_UNIT:
            case HIGHGUID_TYPE_VEHICLE:
            {
                ARCEMU_ASSERT(obj->GetUIdFromGUID() <= m_CreatureHighGuid);
                CreatureStorage[obj->GetUIdFromGUID()] = static_cast< Creature* >(obj);
                if (static_cast<Creature*>(obj)->m_spawn != NULL)
                {
                    _sqlids_creatures.insert(std::make_pair(static_cast<Creature*>(obj)->m_spawn->id, static_cast<Creature*>(obj)));
                }
            }
            break;

            case HIGHGUID_TYPE_GAMEOBJECT:
            {
                GOStorage[obj->GetUIdFromGUID()] = static_cast< GameObject* >(obj);
                if (static_cast<GameObject*>(obj)->m_spawn != NULL)
                {
                    _sqlids_gameobjects.insert(std::make_pair(static_cast<GameObject*>(obj)->m_spawn->id, static_cast<GameObject*>(obj)));
                }
            }
            break;

            case HIGHGUID_TYPE_DYNAMICOBJECT:
                m_DynamicObjectStorage[obj->GetLowGUID()] = (DynamicObject*)obj;
                break;
        }
    }

    // Handle activation of that object.
    if (objCell->IsActive() && obj->CanActivate())
        obj->Activate(this);

    // Add the session to our set if it is a player.
    if (plObj != nullptr)
    {
        Sessions.insert(plObj->GetSession());

        // Change the instance ID, this will cause it to be removed from the world thread (return value 1)
        plObj->GetSession()->SetInstance(GetInstanceID());

        // Add the map wide objects
        if (_mapWideStaticObjects.size())
        {
            uint32 globalcount = 0;
            if (!buf)
                buf = new ByteBuffer(300);

            for (std::set<Object*>::iterator itr = _mapWideStaticObjects.begin(); itr != _mapWideStaticObjects.end(); ++itr)
            {
                count = (*itr)->BuildCreateUpdateBlockForPlayer(buf, plObj);
                globalcount += count;
            }
            /*VLack: It seems if we use the same buffer then it is a BAD idea to try and push created data one by one, add them at once!
                   If you try to add them one by one, then as the buffer already contains data, they'll end up repeating some object.
                   Like 6 object updates for Deeprun Tram, but the built package will contain these entries: 2AFD0, 2AFD0, 2AFD1, 2AFD0, 2AFD1, 2AFD2*/
            if (globalcount > 0)
                plObj->PushCreationData(buf, globalcount);
        }
    }

    if (buf)
        delete buf;

    if (plObj != nullptr && InactiveMoveTime && !forced_expire)
        InactiveMoveTime = 0;
}

void MapMgr::PushStaticObject(Object* obj)
{
    _mapWideStaticObjects.insert(obj);

    obj->SetInstanceID(GetInstanceID());
    obj->SetMapId(GetMapId());

    switch (obj->GetTypeFromGUID())
    {
        case HIGHGUID_TYPE_UNIT:
        case HIGHGUID_TYPE_VEHICLE:
            CreatureStorage[obj->GetUIdFromGUID()] = static_cast<Creature*>(obj);
            break;

        case HIGHGUID_TYPE_GAMEOBJECT:
            GOStorage[obj->GetUIdFromGUID()] = static_cast<GameObject*>(obj);
            break;

        default:
            LogDebugFlag(LF_MAP, "MapMgr::PushStaticObject", "called for invalid type %u.", obj->GetTypeFromGUID());
            break;
    }
}

void MapMgr::RemoveObject(Object* obj, bool free_guid)
{
    // Assertions
    ARCEMU_ASSERT(obj != nullptr);
    ARCEMU_ASSERT(obj->GetMapId() == _mapId);
    ARCEMU_ASSERT(_cells != nullptr);

    if (obj->IsActive())
        obj->Deactivate(this);

    //there is a very small chance that on double player ports on same update player is added to multiple insertpools but not removed
    //one clear example was the double port proc when exploiting double resurrect
    m_objectinsertlock.Acquire();
    m_objectinsertpool.erase(obj);
    m_objectinsertlock.Release();

    _updates.erase(obj);
    obj->ClearUpdateMask();

    // Remove object from all needed places
    switch (obj->GetTypeFromGUID())
    {
        case HIGHGUID_TYPE_UNIT:
        case HIGHGUID_TYPE_VEHICLE:
        {
            ARCEMU_ASSERT(obj->GetUIdFromGUID() <= m_CreatureHighGuid);
            CreatureStorage[obj->GetUIdFromGUID()] = NULL;

            if (static_cast<Creature*>(obj)->m_spawn != NULL)
                _sqlids_creatures.erase(static_cast<Creature*>(obj)->m_spawn->id);

            if (free_guid)
                _reusable_guids_creature.push_back(obj->GetUIdFromGUID());

            break;
        }
        case HIGHGUID_TYPE_PET:
        {
            if (pet_iterator != m_PetStorage.end() && pet_iterator->second->GetGUID() == obj->GetGUID())
                ++pet_iterator;
            m_PetStorage.erase(obj->GetUIdFromGUID());

            break;
        }
        case HIGHGUID_TYPE_DYNAMICOBJECT:
        {
            m_DynamicObjectStorage.erase(obj->GetLowGUID());

            break;
        }
        case HIGHGUID_TYPE_GAMEOBJECT:
        {
            ARCEMU_ASSERT(obj->GetUIdFromGUID() <= m_GOHighGuid);
            GOStorage[obj->GetUIdFromGUID()] = NULL;
            if (static_cast<GameObject*>(obj)->m_spawn != NULL)
                _sqlids_gameobjects.erase(static_cast<GameObject*>(obj)->m_spawn->id);

            if (free_guid)
                _reusable_guids_gameobject.push_back(obj->GetUIdFromGUID());

            break;
        }
        case HIGHGUID_TYPE_TRANSPORTER:
        {
            break;
        }
        default:
        {
            LogDebugFlag(LF_MAP, "MapMgr::RemoveObject", "called for invalid type %u.", obj->GetTypeFromGUID());
            break;
        }
    }

    ///\todo That object types are not map objects. TODO: add AI groups here?
    if (obj->IsItem() || obj->IsContainer())
    {
        return;
    }

    if (obj->IsCorpse())
    {
        m_corpses.erase(static_cast< Corpse* >(obj));
    }

    MapCell* cell = GetCell(obj->GetMapCellX(), obj->GetMapCellY());
    if (cell == nullptr)
    {
        // set the map cell correctly
        if (obj->GetPositionX() < _maxX || obj->GetPositionX() > _minY || obj->GetPositionY() < _maxY || obj->GetPositionY() > _minY)
        {
            cell = this->GetCellByCoords(obj->GetPositionX(), obj->GetPositionY());
            obj->SetMapCell(cell);
        }
    }

    if (cell != nullptr)
    {
        cell->RemoveObject(obj);        // Remove object from cell
        obj->SetMapCell(nullptr);          // Unset object's cell
    }

    Player* plObj = nullptr;
    if (obj->IsPlayer())
    {
        plObj = static_cast<Player*>(obj);
        _processQueue.erase(plObj);     // Clear any updates pending
        plObj->ClearAllPendingUpdates();
    }

    obj->RemoveSelfFromInrangeSets();
    obj->ClearInRangeSet();             // Clear object's in-range set

    uint8 cellNumber = worldConfig.server.mapCellNumber;

    // If it's a player - update his nearby cells
    if (!_shutdown && obj->IsPlayer())
    {
        // get x/y
        if (obj->GetPositionX() < _maxX || obj->GetPositionX() > _minY || obj->GetPositionY() < _maxY || obj->GetPositionY() > _minY)
        {
            uint32 x = GetPosX(obj->GetPositionX());
            uint32 y = GetPosY(obj->GetPositionY());
            UpdateCellActivity(x, y, 2 + cellNumber);
        }
        m_PlayerStorage.erase(static_cast< Player* >(obj)->GetLowGUID());
    }

    // Remove the session from our set if it is a player.
    if (obj->IsPlayer())
    {
        for (std::set<Object*>::iterator itr = _mapWideStaticObjects.begin(); itr != _mapWideStaticObjects.end(); ++itr)
        {
            plObj->PushOutOfRange((*itr)->GetNewGUID());
        }

        // Setting an instance ID here will trigger the session to be removed by MapMgr::run(). :)
        plObj->GetSession()->SetInstance(0);

        // Add it to the global session set. Don't "re-add" to session if it is being deleted.
        if (!plObj->GetSession()->bDeleted)
            sWorld.addGlobalSession(plObj->GetSession());
    }

    if (!HasPlayers())
    {
        if (this->pInstance != nullptr && this->pInstance->m_persistent)
            this->pInstance->m_creatorGroup = 0;

        if (!InactiveMoveTime && !forced_expire && GetMapInfo()->type != INSTANCE_NULL)
        {
            InactiveMoveTime = UNIXTIME + (MAPMGR_INACTIVE_MOVE_TIME * 60);
            LogDebugFlag(LF_MAP, "MapMgr", "Instance %u is now idle. (%s)", m_instanceID, GetBaseMap()->GetMapName().c_str());
        }
    }
}

void MapMgr::ChangeObjectLocation(Object* obj)
{
    ARCEMU_ASSERT(obj != nullptr);

    // Items and containers are of no interest for us
    if (obj->IsItem() || obj->IsContainer() || obj->GetMapMgr() != this)
    {
        return;
    }

    Player* plObj = nullptr;
    ByteBuffer* buf = 0;

    if (obj->IsPlayer())
    {
        plObj = static_cast<Player*>(obj);
    }

    Object* curObj;
    float fRange = 0.0f;

    // Update in-range data for old objects
    if (obj->HasInRangeObjects())
    {
        for (Object::InRangeSet::iterator iter = obj->GetInRangeSetBegin(); iter != obj->GetInRangeSetEnd();)
        {
            curObj = *iter;
            ++iter;

            fRange = GetUpdateDistance(curObj, obj, plObj);

            if (fRange > 0.0f && (curObj->GetDistance2dSq(obj) > fRange))
            {
                if (plObj != nullptr)
                    plObj->RemoveIfVisible(curObj->GetGUID());

                if (curObj->IsPlayer())
                    static_cast< Player* >(curObj)->RemoveIfVisible(obj->GetGUID());

                curObj->RemoveInRangeObject(obj);

                if (obj->GetMapMgr() != this)
                {
                    return;             //Something removed us.
                }
                obj->RemoveInRangeObject(curObj);
            }
        }
    }

    // Get new cell coordinates
    if (obj->GetMapMgr() != this)
    {
        return;                 //Something removed us.
    }

    if (obj->GetPositionX() >= _maxX || obj->GetPositionX() <= _minX || obj->GetPositionY() >= _maxY || obj->GetPositionY() <= _minY)
    {
        OutOfMapBoundariesTeleport(obj);
    }

    uint32 cellX = GetPosX(obj->GetPositionX());
    uint32 cellY = GetPosY(obj->GetPositionY());

    if (cellX >= _sizeX || cellY >= _sizeY)
    {
        return;
    }

    MapCell* objCell = GetCell(cellX, cellY);
    MapCell* pOldCell = obj->GetMapCell();
    if (objCell == nullptr)
    {
        objCell = Create(cellX, cellY);
        objCell->Init(cellX, cellY, this);
    }

    ARCEMU_ASSERT(objCell != nullptr);

    uint8 cellNumber = worldConfig.server.mapCellNumber;

    // If object moved cell
    if (objCell != pOldCell)
    {
        // THIS IS A HACK!
        // Current code, if a creature on a long waypoint path moves from an active
        // cell into an inactive one, it will disable itself and will never return.
        // This is to prevent cpu leaks. I will think of a better solution very soon :P

        if (!objCell->IsActive() && !plObj && obj->IsActive())
            obj->Deactivate(this);

        if (pOldCell != nullptr)
            pOldCell->RemoveObject(obj);

        objCell->AddObject(obj);
        obj->SetMapCell(objCell);

        // if player we need to update cell activity radius = 2 is used in order to update
        // both old and new cells
        if (obj->IsPlayer())
        {
            // have to unlock/lock here to avoid a deadlock situation.
            UpdateCellActivity(cellX, cellY, 2 + cellNumber);
            if (pOldCell != NULL)
            {
                // only do the second check if there's -/+ 2 difference
                if (abs((int)cellX - (int)pOldCell->_x) > 2 + cellNumber ||
                    abs((int)cellY - (int)pOldCell->_y) > 2 + cellNumber)
                {
                    UpdateCellActivity(pOldCell->_x, pOldCell->_y, cellNumber);
                }
            }
        }
    }

    // Update in-range set for new objects
    uint32 endX = cellX + cellNumber;
    uint32 endY = cellY + cellNumber;
    uint32 startX = cellX > 0 ? cellX - cellNumber : 0;
    uint32 startY = cellY > 0 ? cellY - cellNumber : 0;

    //If the object announcing it's position is a special one, then it should do so in a much wider area - like the distance between the two transport towers in Orgrimmar, or more. - By: VLack
    if (obj->IsGameObject() && (static_cast< GameObject* >(obj)->GetOverrides() & GAMEOBJECT_ONMOVEWIDE) || plObj && plObj->camControle)
    {
        endX = cellX + 5 <= _sizeX ? cellX + 6 : (_sizeX - 1);
        endY = cellY + 5 <= _sizeY ? cellY + 6 : (_sizeY - 1);
        startX = cellX > 5 ? cellX - 6 : 0;
        startY = cellY > 5 ? cellY - 6 : 0;
    }

    for (uint32 posX = startX; posX <= endX; ++posX)
    {
        for (uint32 posY = startY; posY <= endY; ++posY)
        {
            MapCell* cell = GetCell(posX, posY);
            if (cell)
                UpdateInRangeSet(obj, plObj, cell, &buf);
        }
    }

    if (buf)
        delete buf;
}

void MapMgr::OutOfMapBoundariesTeleport(Object* object)
{
    if (object->IsPlayer())
    {
        Player* player = static_cast<Player*>(object);

        if (player->GetBindMapId() != GetMapId())
        {
            player->SafeTeleport(player->GetBindMapId(), 0, player->GetBindPositionX(), player->GetBindPositionY(), player->GetBindPositionZ(), 0);
            player->GetSession()->SystemMessage("Teleported you to your hearthstone location as you were out of the map boundaries.");
            return;
        }
        else
        {
            object->GetPositionV()->ChangeCoords(player->GetBindPositionX(), player->GetBindPositionY(), player->GetBindPositionZ(), 0);
            player->GetSession()->SystemMessage("Teleported you to your hearthstone location as you were out of the map boundaries.");
            player->SendTeleportAckPacket(player->GetBindPositionX(), player->GetBindPositionY(), player->GetBindPositionZ(), 0);
        }
    }
    else
    {
        object->GetPositionV()->ChangeCoords(0, 0, 0, 0);
    }
}

void MapMgr::UpdateInRangeSet(Object* obj, Player* plObj, MapCell* cell, ByteBuffer** buf)
{
#define CHECK_BUF if (!*buf) *buf = new ByteBuffer(2500)

    if (cell == nullptr)
        return;

    Player* plObj2;
    int count;
    bool cansee, isvisible;

    ObjectSet::iterator iter = cell->Begin();
    while (iter != cell->End())
    {
        Object* curObj = *iter;
        ++iter;

        if (curObj == nullptr)
            continue;

        float fRange = GetUpdateDistance(curObj, obj, plObj);

        if (curObj != obj && (curObj->GetDistance2dSq(obj) <= fRange || fRange == 0.0f))
        {
            if (!obj->IsInRangeSet(curObj))
            {
                obj->AddInRangeObject(curObj);          // Object in range, add to set
                curObj->AddInRangeObject(obj);

                if (curObj->IsPlayer())
                {
                    plObj2 = static_cast<Player*>(curObj);

                    if (plObj2->CanSee(obj) && !plObj2->IsVisible(obj->GetGUID()))
                    {
                        CHECK_BUF;
                        count = obj->BuildCreateUpdateBlockForPlayer(*buf, plObj2);
                        plObj2->PushCreationData(*buf, count);
                        plObj2->AddVisibleObject(obj->GetGUID());
                        (*buf)->clear();
                    }
                }

                if (plObj != nullptr)
                {
                    if (plObj->CanSee(curObj) && !plObj->IsVisible(curObj->GetGUID()))
                    {
                        CHECK_BUF;
                        count = curObj->BuildCreateUpdateBlockForPlayer(*buf, plObj);
                        plObj->PushCreationData(*buf, count);
                        plObj->AddVisibleObject(curObj->GetGUID());
                        (*buf)->clear();
                    }
                }
            }
            else
            {
                // Check visibility
                if (curObj->IsPlayer())
                {
                    plObj2 = static_cast<Player*>(curObj);
                    cansee = plObj2->CanSee(obj);
                    isvisible = plObj2->IsVisible(obj->GetGUID());
                    if (!cansee && isvisible)
                    {
                        plObj2->PushOutOfRange(obj->GetNewGUID());
                        plObj2->RemoveVisibleObject(obj->GetGUID());
                    }
                    else if (cansee && !isvisible)
                    {
                        CHECK_BUF;
                        count = obj->BuildCreateUpdateBlockForPlayer(*buf, plObj2);
                        plObj2->PushCreationData(*buf, count);
                        plObj2->AddVisibleObject(obj->GetGUID());
                        (*buf)->clear();
                    }
                }

                if (plObj != nullptr)
                {
                    cansee = plObj->CanSee(curObj);
                    isvisible = plObj->IsVisible(curObj->GetGUID());
                    if (!cansee && isvisible)
                    {
                        plObj->PushOutOfRange(curObj->GetNewGUID());
                        plObj->RemoveVisibleObject(curObj->GetGUID());
                    }
                    else if (cansee && !isvisible)
                    {
                        CHECK_BUF;
                        count = curObj->BuildCreateUpdateBlockForPlayer(*buf, plObj);
                        plObj->PushCreationData(*buf, count);
                        plObj->AddVisibleObject(curObj->GetGUID());
                        (*buf)->clear();
                    }
                }
            }
        }
    }
}

float MapMgr::GetUpdateDistance(Object* curObj, Object* obj, Player* plObj)
{
    static float no_distance = 0.0f;

    // unlimited distance for people on same boat
#if VERSION_STRING != Cata
    if (curObj->IsPlayer() && obj->IsPlayer() && plObj != nullptr && plObj->obj_movement_info.transporter_info.guid && plObj->obj_movement_info.transporter_info.guid == static_cast< Player* >(curObj)->obj_movement_info.transporter_info.guid)
#else
    if (curObj->IsPlayer() && obj->IsPlayer() && plObj != nullptr && !plObj->obj_movement_info.getTransportGuid().IsEmpty() && plObj->obj_movement_info.getTransportGuid() == static_cast< Player* >(curObj)->obj_movement_info.getTransportGuid())
#endif
        return no_distance;
    // unlimited distance for transporters (only up to 2 cells +/- anyway.)
    else if (curObj->GetTypeFromGUID() == HIGHGUID_TYPE_TRANSPORTER)
        return no_distance;
    //If the object announcing its position is a transport, or other special object, then deleting it from visible objects should be avoided. - By: VLack
    else if (obj->IsGameObject() && (static_cast<GameObject*>(obj)->GetOverrides() & GAMEOBJECT_INFVIS) && obj->GetMapId() == curObj->GetMapId())
        return no_distance;
    //If the object we're checking for possible removal is a transport or other special object, and we are players on the same map, don't remove it, and add it whenever possible...
    else if (plObj && curObj->IsGameObject() && (static_cast<GameObject*>(curObj)->GetOverrides() & GAMEOBJECT_INFVIS) && obj->GetMapId() == curObj->GetMapId())
        return no_distance;
    else if (plObj != nullptr && plObj->camControle)
        return no_distance;
    else
        return m_UpdateDistance;                  // normal distance
}

void MapMgr::_UpdateObjects()
{
    if (!_updates.size() && !_processQueue.size())
        return;

    ByteBuffer update(2500);
    uint32 count = 0;

    m_updateMutex.Acquire();

    for (UpdateQueue::iterator iter = _updates.begin(); iter != _updates.end(); ++iter)
    {
        Object* pObj = *iter;
        if (pObj == nullptr)
            continue;

        if (pObj->IsItem() || pObj->IsContainer())
        {
            // our update is only sent to the owner here.
            Player* pOwner = static_cast<Item*>(pObj)->GetOwner();
            if (pOwner != nullptr)
            {
                count = static_cast<Item*>(pObj)->BuildValuesUpdateBlockForPlayer(&update, pOwner);
                // send update to owner
                if (count)
                {
                    pOwner->PushUpdateData(&update, count);
                    update.clear();
                }
            }
        }
        else
        {
            if (pObj->IsInWorld())
            {
                // players have to receive their own updates ;)
                if (pObj->IsPlayer())
                {
                    // need to be different! ;)
                    count = pObj->BuildValuesUpdateBlockForPlayer(&update, static_cast<Player*>(pObj));
                    if (count)
                    {
                        static_cast<Player*>(pObj)->PushUpdateData(&update, count);
                        update.clear();
                    }
                }

                if (pObj->IsUnit() && pObj->HasUpdateField(UNIT_FIELD_HEALTH))
                    static_cast<Unit*>(pObj)->EventHealthChangeSinceLastUpdate();

                // build the update
                count = pObj->BuildValuesUpdateBlockForPlayer(&update, static_cast<Player*>(NULL));

                if (count)
                {
                    for (std::set<Object*>::iterator itr = pObj->GetInRangePlayerSetBegin(); itr != pObj->GetInRangePlayerSetEnd(); ++itr)
                    {
                        Player* lplr = static_cast<Player*>(*itr);

                        // Make sure that the target player can see us.
                        if (lplr->IsVisible(pObj->GetGUID()))
                            lplr->PushUpdateData(&update, count);
                    }
                    update.clear();
                }
            }
        }
        pObj->ClearUpdateMask();
    }
    _updates.clear();
    m_updateMutex.Release();

    // generate pending a9packets and send to clients.
    for (PUpdateQueue::iterator it = _processQueue.begin(); it != _processQueue.end();)
    {
        Player* player = *it;

        PUpdateQueue::iterator it2 = it;
        ++it;

        _processQueue.erase(it2);
        if (player->GetMapMgr() == this)
            player->ProcessPendingUpdates();
    }
}


uint32 MapMgr::GetPlayerCount()
{
    return static_cast<uint32>(m_PlayerStorage.size());
}

void MapMgr::UpdateCellActivity(uint32 x, uint32 y, uint32 radius)
{
    CellSpawns* sp;
    uint32 endX = (x + radius) <= _sizeX ? x + radius : (_sizeX - 1);
    uint32 endY = (y + radius) <= _sizeY ? y + radius : (_sizeY - 1);
    uint32 startX = x > radius ? x - radius : 0;
    uint32 startY = y > radius ? y - radius : 0;

    for (uint32 posX = startX; posX <= endX; posX++)
    {
        for (uint32 posY = startY; posY <= endY; posY++)
        {
            MapCell* objCell = GetCell(posX, posY);
            if (objCell == nullptr)
            {
                if (_CellActive(posX, posY))
                {
                    objCell = Create(posX, posY);
                    objCell->Init(posX, posY, this);

                    LogDebugFlag(LF_MAP_CELL, "MapMgr : Cell [%u,%u] on map %u (instance %u) is now active.", posX, posY, this->_mapId, m_instanceID);
                    objCell->SetActivity(true);

                    _terrain->LoadTile((int32)posX / 8, (int32)posY / 8);

                    ARCEMU_ASSERT(!objCell->IsLoaded());

                    LogDebugFlag(LF_MAP_CELL, "MapMgr : Loading objects for Cell [%u][%u] on map %u (instance %u)...", posX, posY, this->_mapId, m_instanceID);

                    sp = _map->GetSpawnsList(posX, posY);
                    if (sp)
                        objCell->LoadObjects(sp);
                }
            }
            else
            {
                //Cell is now active
                if (_CellActive(posX, posY) && !objCell->IsActive())
                {
                    LogDebugFlag(LF_MAP_CELL, "Cell [%u,%u] on map %u (instance %u) is now active.", posX, posY, this->_mapId, m_instanceID);

                    _terrain->LoadTile((int32)posX / 8, (int32)posY / 8);
                    objCell->SetActivity(true);

                    if (!objCell->IsLoaded())
                    {
                        LogDebugFlag(LF_MAP_CELL, "Loading objects for Cell [%u][%u] on map %u (instance %u)...", posX, posY, this->_mapId, m_instanceID);
                        sp = _map->GetSpawnsList(posX, posY);
                        if (sp)
                            objCell->LoadObjects(sp);
                    }
                }
                //Cell is no longer active
                else if (!_CellActive(posX, posY) && objCell->IsActive())
                {
                    LogDebugFlag(LF_MAP_CELL, "Cell [%u,%u] on map %u (instance %u) is now idle.", posX, posY, _mapId, m_instanceID);
                    objCell->SetActivity(false);

                    _terrain->UnloadTile((int32)posX / 8, (int32)posY / 8);
                }
            }
        }
    }
}

float MapMgr::GetLandHeight(float x, float y, float z)
{
    float adtheight = GetADTLandHeight(x, y);

    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();
    float vmapheight = vmgr->getHeight(_mapId, x, y, z + 0.5f, 10000.0f);

    if (adtheight > z && vmapheight > -1000)
        return vmapheight; //underground

    return std::max(vmapheight, adtheight);
}

float MapMgr::GetADTLandHeight(float x, float y)
{
    TerrainTile* tile = _terrain->GetTile(x, y);
    if (tile == nullptr)
        return TERRAIN_INVALID_HEIGHT;

    float rv = tile->m_map.GetHeight(x, y);
    tile->DecRef();

    return rv;
}

bool MapMgr::IsUnderground(float x, float y, float z)
{
    return GetADTLandHeight(x, y) > (z + 0.5f);
}

bool MapMgr::GetLiquidInfo(float x, float y, float z, float& liquidlevel, uint32& liquidtype)
{
    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();

    float flr;
    if (vmgr->GetLiquidLevel(_mapId, x, y, z, 0xFF, liquidlevel, flr, liquidtype))
        return true;

    liquidlevel = GetLiquidHeight(x, y);
    liquidtype = GetLiquidType(x, y);

    if (liquidtype == 0)
        return false;

    return true;
}

float MapMgr::GetLiquidHeight(float x, float y)
{
    TerrainTile* tile = _terrain->GetTile(x, y);
    if (tile == nullptr)
        return TERRAIN_INVALID_HEIGHT;

    float rv = tile->m_map.GetTileLiquidHeight(x, y);
    tile->DecRef();

    return rv;
}

uint8 MapMgr::GetLiquidType(float x, float y)
{
    TerrainTile* tile = _terrain->GetTile(x, y);
    if (tile == nullptr)
        return 0;

    uint8 rv = tile->m_map.GetTileLiquidType(x, y);
    tile->DecRef();

    return rv;
}

const ::DBC::Structures::AreaTableEntry* MapMgr::GetArea(float x, float y, float z)
{
    uint32 mogp_flags;
    int32 adt_id;
    int32 root_id;
    int32 group_id;

    bool have_area_info = _terrain->GetAreaInfo(x, y, z, mogp_flags, adt_id, root_id, group_id);
    auto area_flag_without_adt_id = _terrain->GetAreaFlagWithoutAdtId(x, y);
    auto area_flag = MapManagement::AreaManagement::AreaStorage::GetFlagByPosition(area_flag_without_adt_id, have_area_info, mogp_flags, adt_id, root_id, group_id, _mapId, x, y, z, nullptr);

    if (area_flag)
        return MapManagement::AreaManagement::AreaStorage::GetAreaByFlag(area_flag);
    else
        return MapManagement::AreaManagement::AreaStorage::GetAreaByMapId(_mapId);
}

bool MapMgr::isInLineOfSight(float x, float y, float z, float x2, float y2, float z2)
{
    VMAP::IVMapManager* vmgr = VMAP::VMapFactory::createOrGetVMapManager();

    return vmgr->isInLineOfSight(GetMapId(), x, y, z, x2, y2, z2);
}

uint32 MapMgr::GetMapId()
{
    return _mapId;
}

bool MapMgr::_CellActive(uint32 x, uint32 y)
{
    uint8 cellNumber = worldConfig.server.mapCellNumber;

    uint32 endX = ((x + cellNumber) <= _sizeX) ? x + cellNumber : (_sizeX - cellNumber);
    uint32 endY = ((y + cellNumber) <= _sizeY) ? y + cellNumber : (_sizeY - cellNumber);
    uint32 startX = x > 0 ? x - cellNumber : 0;
    uint32 startY = y > 0 ? y - cellNumber : 0;

    for (uint32 posX = startX; posX <= endX; posX++)
    {
        for (uint32 posY = startY; posY <= endY; posY++)
        {
            MapCell* objCell = GetCell(posX, posY);
            if (objCell != nullptr)
            {
                if (objCell->HasPlayers() || m_forcedcells.find(objCell) != m_forcedcells.end())
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void MapMgr::ObjectUpdated(Object* obj)
{
    // set our fields to dirty stupid fucked up code in places.. i hate doing this but i've got to :<- burlex
    m_updateMutex.Acquire();
    _updates.insert(obj);
    m_updateMutex.Release();
}

void MapMgr::PushToProcessed(Player* plr)
{
    _processQueue.insert(plr);
}

bool MapMgr::HasPlayers()
{
    return (m_PlayerStorage.size() > 0);
}

bool MapMgr::IsCombatInProgress()
{
    return (_combatProgress.size() > 0);
}

void MapMgr::ChangeFarsightLocation(Player* plr, DynamicObject* farsight)
{
    uint8 cellNumber = worldConfig.server.mapCellNumber;

    if (farsight == 0)
    {
        // We're clearing.
        for (ObjectSet::iterator itr = plr->m_visibleFarsightObjects.begin(); itr != plr->m_visibleFarsightObjects.end(); ++itr)
        {
            if (plr->IsVisible((*itr)->GetGUID()) && !plr->CanSee((*itr)))
            {
                plr->PushOutOfRange((*itr)->GetNewGUID());      // Send destroy
            }
        }
        plr->m_visibleFarsightObjects.clear();
    }
    else
    {
        uint32 cellX = GetPosX(farsight->GetPositionX());
        uint32 cellY = GetPosY(farsight->GetPositionY());
        uint32 endX = (cellX <= _sizeX) ? cellX + cellNumber : (_sizeX - cellNumber);
        uint32 endY = (cellY <= _sizeY) ? cellY + cellNumber : (_sizeY - cellNumber);
        uint32 startX = cellX > 0 ? cellX - cellNumber : 0;
        uint32 startY = cellY > 0 ? cellY - cellNumber : 0;

        for (uint32 posX = startX; posX <= endX; ++posX)
        {
            for (uint32 posY = startY; posY <= endY; ++posY)
            {
                MapCell* cell = GetCell(posX, posY);
                if (cell != nullptr)
                {
                    for (MapCell::ObjectSet::iterator iter = cell->Begin(); iter != cell->End(); ++iter)
                    {
                        Object* obj = (*iter);
                        if (obj == nullptr)
                            continue;

                        if (!plr->IsVisible(obj->GetGUID()) && plr->CanSee(obj) && farsight->GetDistance2dSq(obj) <= m_UpdateDistance)
                        {
                            ByteBuffer buf;
                            uint32 count = obj->BuildCreateUpdateBlockForPlayer(&buf, plr);
                            plr->PushCreationData(&buf, count);
                            plr->m_visibleFarsightObjects.insert(obj);
                        }
                    }
                }
            }
        }
    }
}

bool MapMgr::runThread()
{
    bool rv = true;

    THREAD_TRY_EXECUTION
        rv = Do();
    THREAD_HANDLE_CRASH

        return rv;
}

bool MapMgr::Do()
{
#ifdef WIN32
    threadid = GetCurrentThreadId();
#endif

    t_currentMapContext.set(this);

    thread_running = true;
    ThreadState = THREADSTATE_BUSY;
    SetThreadName("Map mgr - M%u|I%u", this->_mapId, this->m_instanceID);

    uint32 last_exec = Util::getMSTime();

    // Create Instance script
    LoadInstanceScript();

    // create static objects
    for (GameobjectSpawnList::iterator itr = _map->staticSpawns.GameobjectSpawns.begin(); itr != _map->staticSpawns.GameobjectSpawns.end(); ++itr)
    {
        GameObject* obj = CreateGameObject((*itr)->entry);
        obj->Load((*itr));
        PushStaticObject(obj);
    }

    // Call script OnLoad virtual procedure
    CALL_INSTANCE_SCRIPT_EVENT(this, OnLoad)();

    for (CreatureSpawnList::iterator itr = _map->staticSpawns.CreatureSpawns.begin(); itr != _map->staticSpawns.CreatureSpawns.end(); ++itr)
    {
        Creature* obj = CreateCreature((*itr)->entry);
        obj->Load(*itr, 0, pMapInfo);
        PushStaticObject(obj);
    }

    // load corpses
    objmgr.LoadCorpses(this);
    worldstateshandler.InitWorldStates(objmgr.GetWorldStatesForMap(_mapId));
    worldstateshandler.setObserver(this);

    // always declare local variables outside of the loop!
    // otherwise there's a lot of sub esp; going on.

    uint32 exec_time, exec_start;

    while ((GetThreadState() != THREADSTATE_TERMINATE) && !_shutdown)
    {
        exec_start = Util::getMSTime();

        //////////////////////////////////////////////////////////////////////////////////////////
        //first push to world new objects
        m_objectinsertlock.Acquire();

        if (m_objectinsertpool.size())
        {
            for (ObjectSet::iterator i = m_objectinsertpool.begin(); i != m_objectinsertpool.end(); ++i)
            {
                Object* o = *i;

                o->PushToWorld(this);
            }

            m_objectinsertpool.clear();
        }

        m_objectinsertlock.Release();
        //////////////////////////////////////////////////////////////////////////////////////////

        //Now update sessions of this map + objects
        _PerformObjectDuties();

        last_exec = Util::getMSTime();
        exec_time = last_exec - exec_start;
        if (exec_time < MAP_MGR_UPDATE_PERIOD)
        {
            Arcemu::Sleep(MAP_MGR_UPDATE_PERIOD - exec_time);
        }

        // Check if we have to die :P
        if (InactiveMoveTime && UNIXTIME >= InactiveMoveTime)
            break;
    }

    // Teleport any left-over players out.
    TeleportPlayers();

    // Clear the instance's reference to us.
    if (m_battleground)
    {
        BattlegroundManager.DeleteBattleground(m_battleground);
        sInstanceMgr.DeleteBattlegroundInstance(GetMapId(), GetInstanceID());
    }

    if (pInstance != nullptr)
    {
        // check for a non-raid instance, these expire after 10 minutes.
        if (GetMapInfo()->type == INSTANCE_NONRAID || pInstance->m_isBattleground)
        {
            pInstance->m_mapMgr = nullptr;
            sInstanceMgr._DeleteInstance(pInstance, true);
            pInstance = nullptr;
        }
        else
        {
            // just null out the pointer
            pInstance->m_mapMgr = nullptr;
        }
    }
    else if (GetMapInfo()->type == INSTANCE_NULL)
    {
        sInstanceMgr.m_singleMaps[GetMapId()] = NULL;
    }

    thread_running = false;
    if (thread_kill_only)
        return false;

    // delete ourselves
    delete this;

    // already deleted, so the threadpool doesn't have to.
    return false;
}

void MapMgr::BeginInstanceExpireCountdown()
{
    WorldPacket data(SMSG_RAID_GROUP_ONLY, 8);
    // so players getting removed don't overwrite us
    forced_expire = true;

    // send our sexy packet
    data << uint32(60000);
    data << uint32(1);
    for (PlayerStorageMap::iterator itr = m_PlayerStorage.begin(); itr != m_PlayerStorage.end(); ++itr)
    {
        if (!itr->second->raidgrouponlysent)
            itr->second->GetSession()->SendPacket(&data);
    }

    // set our expire time to 60 seconds.
    InactiveMoveTime = UNIXTIME + 60;
}

void MapMgr::InstanceShutdown()
{
    pInstance = nullptr;
    SetThreadState(THREADSTATE_TERMINATE);
}

void MapMgr::KillThread()
{
    pInstance = nullptr;
    thread_kill_only = true;
    SetThreadState(THREADSTATE_TERMINATE);
    while(thread_running)
    {
        Arcemu::Sleep(100);
    }
}

void MapMgr::AddObject(Object* obj)
{
    m_objectinsertlock.Acquire();
    m_objectinsertpool.insert(obj);
    m_objectinsertlock.Release();
}

Unit* MapMgr::GetUnit(const uint64 & guid)
{
    if (guid == 0)
        return nullptr;

    switch (GET_TYPE_FROM_GUID(guid))
    {
        case HIGHGUID_TYPE_UNIT:
        case HIGHGUID_TYPE_VEHICLE:
            return GetCreature(GET_LOWGUID_PART(guid));
            break;

        case HIGHGUID_TYPE_PLAYER:
            return GetPlayer(Arcemu::Util::GUID_LOPART(guid));
            break;

        case HIGHGUID_TYPE_PET:
            return GetPet(GET_LOWGUID_PART(guid));
            break;
    }

    return nullptr;
}

Object* MapMgr::_GetObject(const uint64 & guid)
{
    if (!guid)
        return nullptr;

    switch (GET_TYPE_FROM_GUID(guid))
    {
        case HIGHGUID_TYPE_GAMEOBJECT:
            return GetGameObject(GET_LOWGUID_PART(guid));
            break;
        case HIGHGUID_TYPE_UNIT:
        case HIGHGUID_TYPE_VEHICLE:
            return GetCreature(GET_LOWGUID_PART(guid));
            break;
        case HIGHGUID_TYPE_DYNAMICOBJECT:
            return GetDynamicObject((uint32)guid);
            break;
        case HIGHGUID_TYPE_TRANSPORTER:
            return objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(guid));
            break;
        default:
            return GetUnit(guid);
            break;
    }
}

void MapMgr::_PerformObjectDuties()
{
    ++mLoopCounter;

    uint32 mstime = Util::getMSTime();
    uint32 difftime = mstime - lastUnitUpdate;

    if (difftime > 500)
        difftime = 500;

    // Update any events.
    // we make update of events before objects so in case there are 0 timediff events they do not get deleted after update but on next server update loop
    eventHolder.Update(difftime);

    // Update creatures.
    {
        for (creature_iterator = activeCreatures.begin(); creature_iterator != activeCreatures.end();)
        {
            Creature* ptr = *creature_iterator;
            ++creature_iterator;
            ptr->Update(difftime);
        }

        for (pet_iterator = m_PetStorage.begin(); pet_iterator != m_PetStorage.end();)
        {
            Pet* ptr2 = pet_iterator->second;
            ++pet_iterator;
            ptr2->Update(difftime);
        }
    }

    // Update players.
    {
        for (PlayerStorageMap::iterator itr = m_PlayerStorage.begin(); itr != m_PlayerStorage.end();)
        {
            Player* ptr = itr->second;
            ++itr;
            ptr->Update(difftime);
        }

        lastUnitUpdate = mstime;
    }

    // Dynamic objects
    // We take the pointer, increment, and update in this order because during the update the DynamicObject might get deleted,
    // rendering the iterator unincrementable. Which causes a crash!
    {
        for (DynamicObjectStorageMap::iterator itr = m_DynamicObjectStorage.begin(); itr != m_DynamicObjectStorage.end();)
        {
            DynamicObject* o = itr->second;
            ++itr;

            o->UpdateTargets();
        }
    }

    // Update gameobjects (not on every loop, however)
    if (mLoopCounter % 2)
    {
        difftime = mstime - lastGameobjectUpdate;

        for (std::vector<GameObject*>::iterator itr = GOStorage.begin(); itr != GOStorage.end(); )
        {
            GameObject* gameobject = *itr;
            ++itr;
            if (gameobject != nullptr)
                gameobject->Update(difftime);
        }

        lastGameobjectUpdate = mstime;
    }

    // Sessions are updated every loop.
    {
        for (SessionSet::iterator itr = Sessions.begin(); itr != Sessions.end();)
        {
            WorldSession* session = (*itr);
            SessionSet::iterator it2 = itr;

            ++itr;

            if (session->GetInstance() != m_instanceID)
            {
                Sessions.erase(it2);
                continue;
            }

            // Don't update players not on our map.
            // If we abort in the handler, it means we will "lose" packets, or not process this.
            // .. and that could be disastrous to our client :P
            if (session->GetPlayer() && (session->GetPlayer()->GetMapMgr() != this && session->GetPlayer()->GetMapMgr() != 0))
            {
                continue;
            }

            uint8 result;

            if ((result = session->Update(m_instanceID)) != 0)
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

    // Finally, A9 Building/Distribution
    _UpdateObjects();
}

void MapMgr::EventCorpseDespawn(uint64 guid)
{
    Corpse* pCorpse = objmgr.GetCorpse((uint32)guid);
    if (pCorpse == nullptr)     // Already Deleted
        return;

    if (pCorpse->GetMapMgr() != this)
        return;

    pCorpse->Despawn();
    delete pCorpse;
}

void MapMgr::TeleportPlayers()
{
    for (PlayerStorageMap::iterator itr = m_PlayerStorage.begin(); itr != m_PlayerStorage.end();)
    {
        Player* p = itr->second;
        ++itr;

        if (!bServerShutdown)
        {
            p->EjectFromInstance();
        }
        else
        {
            if (p->GetSession())
                p->GetSession()->LogoutPlayer(false);
            else
                delete p;
        }
    }
}

uint32 MapMgr::GetInstanceID()
{
    return m_instanceID;
}

MySQLStructure::MapInfo const* MapMgr::GetMapInfo()
{
    return pMapInfo;
}

MapScriptInterface* MapMgr::GetInterface()
{
    return ScriptInterface;
}

int32 MapMgr::event_GetInstanceID()
{
    return m_instanceID;
}

void MapMgr::UnloadCell(uint32 x, uint32 y)
{
    MapCell* c = GetCell(x, y);
    if (c == nullptr || c->HasPlayers() || _CellActive(x, y) || !c->IsUnloadPending())
        return;

    LogDebugFlag(LF_MAP, "Unloading Cell [%u][%u] on map %u (instance %u)...", x, y, _mapId, m_instanceID);

    c->Unload();
}

void MapMgr::EventRespawnCreature(Creature* c, uint16 x, uint16 y)
{
    MapCell* cell = GetCell(x, y);
    if (cell == nullptr)    //cell got deleted while waiting for respawn.
        return;

    ObjectSet::iterator itr = cell->_respawnObjects.find(c);
    if (itr != cell->_respawnObjects.end())
    {
        c->m_respawnCell = nullptr;
        cell->_respawnObjects.erase(itr);
        c->OnRespawn(this);
    }
}

void MapMgr::EventRespawnGameObject(GameObject* o, uint16 x, uint16 y)
{
    MapCell* cell = GetCell(x, y);
    if (cell == nullptr)   //cell got deleted while waiting for respawn.
        return;

    ObjectSet::iterator itr = cell->_respawnObjects.find(o);
    if (itr != cell->_respawnObjects.end())
    {
        o->m_respawnCell = nullptr;
        cell->_respawnObjects.erase(itr);
        o->Spawn(this);
    }
}

void MapMgr::SendChatMessageToCellPlayers(Object* obj, WorldPacket* packet, uint32 cell_radius, uint32 langpos, int32 lang, WorldSession* originator)
{
    uint32 cellX = GetPosX(obj->GetPositionX());
    uint32 cellY = GetPosY(obj->GetPositionY());
    uint32 endX = ((cellX + cell_radius) <= _sizeX) ? cellX + cell_radius : (_sizeX - 1);
    uint32 endY = ((cellY + cell_radius) <= _sizeY) ? cellY + cell_radius : (_sizeY - 1);
    uint32 startX = cellX > cell_radius ? cellX - cell_radius : 0;
    uint32 startY = cellY > cell_radius ? cellY - cell_radius : 0;

    MapCell::ObjectSet::iterator iter, iend;
    for (uint32 posX = startX; posX <= endX; ++posX)
    {
        for (uint32 posY = startY; posY <= endY; ++posY)
        {
            MapCell* cell = GetCell(posX, posY);
            if (cell && cell->HasPlayers())
            {
                iter = cell->Begin();
                iend = cell->End();
                for (; iter != iend; ++iter)
                {
                    if ((*iter)->IsPlayer())
                    {
                        //TO< Player* >(*iter)->GetSession()->SendPacket(packet);
                        if (static_cast< Player* >(*iter)->GetPhase() & obj->GetPhase())
                            static_cast< Player* >(*iter)->GetSession()->SendChatPacket(packet, langpos, lang, originator);
                    }
                }
            }
        }
    }
}

Creature* MapMgr::GetSqlIdCreature(uint32 sqlid)
{
    CreatureSqlIdMap::iterator itr = _sqlids_creatures.find(sqlid);
    return (itr == _sqlids_creatures.end()) ? nullptr : itr->second;
}

GameObject* MapMgr::GetSqlIdGameObject(uint32 sqlid)
{
    GameObjectSqlIdMap::iterator itr = _sqlids_gameobjects.find(sqlid);
    return (itr == _sqlids_gameobjects.end()) ? nullptr : itr->second;
}

uint64 MapMgr::GenerateCreatureGUID(uint32 entry)
{
    uint64 newguid = 0;

    CreatureProperties const* creature_properties = sMySQLStore.getCreatureProperties(entry);
    if ((creature_properties == nullptr) || (creature_properties->vehicleid == 0))
        newguid = static_cast<uint64>(HIGHGUID_TYPE_UNIT) << 32;
    else
        newguid = static_cast<uint64>(HIGHGUID_TYPE_VEHICLE) << 32;

    char* pHighGuid = reinterpret_cast<char*>(&newguid);
    char* pEntry = reinterpret_cast<char*>(&entry);

    pHighGuid[3] |= pEntry[0];
    pHighGuid[4] |= pEntry[1];
    pHighGuid[5] |= pEntry[2];
    pHighGuid[6] |= pEntry[3];

    uint32 guid = 0;

    if (_reusable_guids_creature.size() > 0)
    {
        guid = _reusable_guids_creature.front();
        _reusable_guids_creature.pop_front();

    }
    else
    {
        m_CreatureHighGuid++;

        if (m_CreatureHighGuid >= CreatureStorage.size())
        {
            // Reallocate array with larger size.
            size_t newsize = CreatureStorage.size() + RESERVE_EXPAND_SIZE;
            CreatureStorage.resize(newsize, NULL);
        }
        guid = m_CreatureHighGuid;
    }

    newguid |= guid;

    return newguid;
}

Creature* MapMgr::CreateCreature(uint32 entry)
{
    uint64 guid = GenerateCreatureGUID(entry);
    return new Creature(guid);
}

Creature* MapMgr::CreateAndSpawnCreature(uint32 pEntry, float pX, float pY, float pZ, float pO)
{
    auto creature = CreateCreature(pEntry);
    auto cp = sMySQLStore.getCreatureProperties(pEntry);
    if (cp == nullptr)
        return nullptr;

    creature->Load(cp, pX, pY, pZ, pO);
    creature->AddToWorld(this);
    return creature;
}

Creature* MapMgr::GetCreature(uint32 guid)
{
    if (guid > m_CreatureHighGuid)
        return nullptr;

    return CreatureStorage[guid];
}

Summon* MapMgr::CreateSummon(uint32 entry, SummonType type)
{
    uint64 guid = GenerateCreatureGUID(entry);

    switch (type)
    {
        case SUMMONTYPE_GUARDIAN:
            return new GuardianSummon(guid);
            break;

        case SUMMONTYPE_WILD:
            return new WildSummon(guid);
            break;

        case SUMMONTYPE_TOTEM:
            return new TotemSummon(guid);
            break;

        case SUMMONTYPE_COMPANION:
            return new CompanionSummon(guid);
            break;

        case SUMMONTYPE_POSSESSED:
            return new PossessedSummon(guid);
            break;
    }

    return new Summon(guid);
}


// Spawns the object too, without which you can not interact with the object
GameObject* MapMgr::CreateAndSpawnGameObject(uint32 entryID, float x, float y, float z, float o, float scale)
{
    auto gameobject_info = sMySQLStore.getGameObjectProperties(entryID);
    if (gameobject_info == nullptr)
    {
        LogDebugFlag(LF_MAP, "Error looking up entry in CreateAndSpawnGameObject");
        return nullptr;
    }

    LogDebugFlag(LF_MAP, "CreateAndSpawnGameObject: By Entry '%u'", entryID);

    GameObject* go = CreateGameObject(entryID);

    //Player* chr = m_session->GetPlayer();
    uint32 mapid = GetMapId();
    // Setup game object
    go->CreateFromProto(entryID, mapid, x, y, z, o);
    go->SetScale(scale);
    go->InitAI();
    go->PushToWorld(this);

    // Create spawn instance
    auto go_spawn = new GameobjectSpawn;
    go_spawn->entry = go->GetEntry();
    go_spawn->id = objmgr.GenerateGameObjectSpawnID();
    go_spawn->map = go->GetMapId();
    go_spawn->position_x = go->GetPositionX();
    go_spawn->position_y = go->GetPositionY();
    go_spawn->position_z = go->GetPositionZ();
    go_spawn->orientation = go->GetOrientation();
    go_spawn->rotation_0 = go->GetParentRotation(0);
    go_spawn->rotation_1 = go->GetParentRotation(1);
    go_spawn->rotation_2 = go->GetParentRotation(2);
    go_spawn->rotation_3 = go->GetParentRotation(3);
    go_spawn->state = go->GetState();
    go_spawn->flags = go->GetFlags();
    go_spawn->faction = go->GetFaction();
    go_spawn->scale = go->GetScale();
    //go_spawn->stateNpcLink = 0;
    go_spawn->phase = go->GetPhase();
    go_spawn->overrides = go->GetOverrides();

    uint32 cx = GetPosX(x);
    uint32 cy = GetPosY(y);

    GetBaseMap()->GetSpawnsListAndCreate(cx, cy)->GameobjectSpawns.push_back(go_spawn);
    go->m_spawn = go_spawn;

    MapCell* mCell = GetCell(cx, cy);

    if (mCell != nullptr)
        mCell->SetLoaded();

    return go;
}

GameObject* MapMgr::GetGameObject(uint32 guid)
{
    if (guid > m_GOHighGuid)
        return nullptr;

    return GOStorage[guid];
}

GameObject* MapMgr::CreateGameObject(uint32 entry)
{
    uint32 GUID = 0;

    if (_reusable_guids_gameobject.size() > GO_GUID_RECYCLE_INTERVAL)
    {
        uint32 guid = _reusable_guids_gameobject.front();
        _reusable_guids_gameobject.pop_front();

        GUID = guid;
    }
    else
    {
        if (++m_GOHighGuid >= GOStorage.size())
        {
            // Reallocate array with larger size.
            size_t newsize = GOStorage.size() + RESERVE_EXPAND_SIZE;
            GOStorage.resize(newsize, NULL);
        }

        GUID = m_GOHighGuid;
    }

    GameObject* gameobject = ObjectFactory.CreateGameObject(entry, GUID);
    if (gameobject == nullptr)
        return nullptr;

    return gameobject;
}

DynamicObject* MapMgr::CreateDynamicObject()
{
    return new DynamicObject(HIGHGUID_TYPE_DYNAMICOBJECT, (++m_DynamicObjectHighGuid));
}

DynamicObject* MapMgr::GetDynamicObject(uint32 guid)
{
    DynamicObjectStorageMap::iterator itr = m_DynamicObjectStorage.find(guid);
    return (itr != m_DynamicObjectStorage.end()) ? itr->second : nullptr;
}

Pet* MapMgr::GetPet(uint32 guid)
{
    PetStorageMap::iterator itr = m_PetStorage.find(guid);
    return (itr != m_PetStorage.end()) ? itr->second : nullptr;
}

Player* MapMgr::GetPlayer(uint32 guid)
{
    PlayerStorageMap::iterator itr = m_PlayerStorage.find(guid);
    return (itr != m_PlayerStorage.end()) ? itr->second : nullptr;
}

void MapMgr::AddCombatInProgress(uint64 guid)
{
    _combatProgress.insert(guid);
}

void MapMgr::RemoveCombatInProgress(uint64 guid)
{
    _combatProgress.erase(guid);
}

void MapMgr::AddForcedCell(MapCell* c)
{
    uint8 cellNumber = worldConfig.server.mapCellNumber;

    m_forcedcells.insert(c);
    UpdateCellActivity(c->GetPositionX(), c->GetPositionY(), cellNumber);
}

void MapMgr::RemoveForcedCell(MapCell* c)
{
    uint8 cellNumber = worldConfig.server.mapCellNumber;

    m_forcedcells.erase(c);
    UpdateCellActivity(c->GetPositionX(), c->GetPositionY(), cellNumber);
}

float MapMgr::GetFirstZWithCPZ(float x, float y, float z)
{
    if (!worldConfig.terrainCollision.isCollisionEnabled)
        return NO_WMO_HEIGHT;

    float posZ = NO_WMO_HEIGHT;
    for (int i = Z_SEARCH_RANGE; i >= -Z_SEARCH_RANGE; i--)
    {
        VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
        //if (i== 0 && !IsUnderground(x,y,z)) return GetBaseMap()->GetLandHeight(x, y);
        posZ = mgr->getHeight(GetMapId(), x, y, z + (float)i, 10000.0f);
        if (posZ != NO_WMO_HEIGHT)
            break;
    }
    return posZ;
}

GameObject* MapMgr::FindNearestGoWithType(Object* o, uint32 type)
{
    GameObject* go = nullptr;
    float r = FLT_MAX;

    for (std::set<Object*>::iterator itr = o->GetInRangeSetBegin(); itr != o->GetInRangeSetEnd(); ++itr)
    {
        Object* iro = *itr;
        if (!iro->IsGameObject())
            continue;

        GameObject* irgo = static_cast<GameObject*>(iro);

        if (irgo->GetType() != type)
            continue;

        if ((irgo->GetPhase() & o->GetPhase()) == 0)
            continue;

        float range = o->getDistanceSq(iro);

        if (range < r)
        {
            r = range;
            go = irgo;
        }
    }

    return go;
}

void MapMgr::SendPvPCaptureMessage(int32 ZoneMask, uint32 ZoneId, const char* Message, ...)
{
    va_list ap;
    va_start(ap, Message);

    WorldPacket data(SMSG_DEFENSE_MESSAGE, 208);
    char msgbuf[200];
    vsnprintf(msgbuf, 200, Message, ap);
    va_end(ap);

    data << ZoneId;
    data << uint32(strlen(msgbuf) + 1);
    data << msgbuf;

    for (PlayerStorageMap::iterator itr = m_PlayerStorage.begin(); itr != m_PlayerStorage.end();)
    {
        Player* plr = itr->second;
        ++itr;

        if ((ZoneMask != ZONE_MASK_ALL && plr->GetZoneId() != (uint32)ZoneMask))
            continue;

        plr->GetSession()->SendPacket(&data);
    }
}

void MapMgr::SendPacketToAllPlayers(WorldPacket* packet) const
{
    for (PlayerStorageMap::const_iterator itr = m_PlayerStorage.begin(); itr != m_PlayerStorage.end(); ++itr)
    {
        Player* p = itr->second;

        if (p->GetSession() != nullptr)
            p->GetSession()->SendPacket(packet);
    }
}

void MapMgr::SendPacketToPlayersInZone(uint32 zone, WorldPacket* packet) const
{
    for (PlayerStorageMap::const_iterator itr = m_PlayerStorage.begin(); itr != m_PlayerStorage.end(); ++itr)
    {
        Player* p = itr->second;

        if ((p->GetSession() != NULL) && (p->GetZoneId() == zone))
            p->GetSession()->SendPacket(packet);
    }
}

InstanceScript* MapMgr::GetScript()
{
    return mInstanceScript;
}

void MapMgr::LoadInstanceScript()
{
    mInstanceScript = sScriptMgr.CreateScriptClassForInstance(_mapId, this);
};

void MapMgr::CallScriptUpdate()
{
    ARCEMU_ASSERT(mInstanceScript != NULL);
    mInstanceScript->UpdateEvent();
    mInstanceScript->updateTimers();
};

const uint16 MapMgr::GetAreaFlag(float x, float y, float z, bool *is_outdoors /* = nullptr */)
{
    uint32 mogp_flags;
    int32 adt_id;
    int32 root_id;
    int32 group_id;

    bool have_area_info = _terrain->GetAreaInfo(x, y, z, mogp_flags, adt_id, root_id, group_id);
    auto area_flag_without_adt_id = _terrain->GetAreaFlagWithoutAdtId(x, y);
    return MapManagement::AreaManagement::AreaStorage::GetFlagByPosition(area_flag_without_adt_id, have_area_info, mogp_flags, adt_id, root_id, group_id, _mapId, x, y, z, nullptr);
}


WorldStatesHandler& MapMgr::GetWorldStatesHandler()
{
    return worldstateshandler;
}

void MapMgr::onWorldStateUpdate(uint32 zone, uint32 field, uint32 value)
{
    WorldPacket data(SMSG_UPDATE_WORLD_STATE, 8);
    data << uint32(field);
    data << uint32(value);

    SendPacketToPlayersInZone(zone, &data);
}
