/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include <G3D/Vector3.h>
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Server/Packets/SmsgTransferPending.h"
#include "../Movement/Spline/New/Spline.h"

using namespace AscEmu::Packets;

Transporter::Transporter(uint64 guid) : GameObject(guid), _transportInfo(nullptr), _isMoving(true), _pendingStop(false), _triggeredArrivalEvent(false), _triggeredDepartureEvent(false), _passengerTeleportItr(_passengers.begin())
{
#if VERSION_STRING <= TBC
    m_updateFlag = (UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_LOWGUID | UPDATEFLAG_TRANSPORT);
#elif VERSION_STRING == WotLK
    m_updateFlag = (UPDATEFLAG_TRANSPORT | UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_ROTATION);
#elif VERSION_STRING == Cata
    m_updateFlag = UPDATEFLAG_TRANSPORT;
#endif
    positionUpdateDelay = 100;
}

Transporter::~Transporter()
{
    ASSERT(_passengers.empty());
    _passengers.clear();
}

bool Transporter::Create(uint32_t entry, uint32_t mapid, float x, float y, float z, float ang, uint32_t animprogress)
{
    gameobject_properties = sMySQLStore.getGameObjectProperties(entry);
    if (gameobject_properties == nullptr)
    {
        LOG_ERROR("Something tried to create a GameObject with invalid entry %u", entry);
        return false;
    }

    TransportTemplate const* tInfo = sTransportHandler.getTransportTemplate(entry);
    if (!tInfo)
    {
        LOG_ERROR("Transport %u will not be created, missing `transport_template` entry.", entry);
        return false;
    }

    if (!CreateFromProto(entry, mapid, x, y, z, ang))
        return false;

    // Set Pathtime
    setLevel(tInfo->pathTime);
    setAnimationProgress(animprogress);

    _transportInfo = tInfo;

    // initialize waypoints
    _nextFrame = tInfo->keyFrames.begin();
    _currentFrame = _nextFrame++;

    _triggeredArrivalEvent = false;
    _triggeredDepartureEvent = false;

    return true;
}

void Transporter::Update(unsigned long time_passed)
{
    if (GetKeyFrames().size() <= 1)
        return;

    if (IsMoving() || !_pendingStop)
        mTransValues.PathProgress += time_passed;

    uint32_t timer = mTransValues.PathProgress % getTransportPeriod();
    bool justStopped = false;

    LogDebug("Transporter: current node %u and pathprogress %u \n", _currentFrame->Index, GetTimer());

    for (;;)
    {
        if (timer >= _currentFrame->ArriveTime)
        {
            if (!_triggeredArrivalEvent)
            {
                DoEventIfAny(*_currentFrame, false);
                _triggeredArrivalEvent = true;
            }

            if (timer < _currentFrame->DepartureTime)
            {
                justStopped = IsMoving();
                SetMoving(false);
                if (_pendingStop && getState() != GO_STATE_CLOSED)
                {
                    setState(GO_STATE_CLOSED);
                    mTransValues.PathProgress = (mTransValues.PathProgress / getTransportPeriod());
                    mTransValues.PathProgress *= getTransportPeriod();
                    mTransValues.PathProgress += _currentFrame->ArriveTime;
                }
                break; // its a stop frame and we are waiting
            }
        }

        if (timer >= _currentFrame->DepartureTime && !_triggeredDepartureEvent)
        {
            DoEventIfAny(*_currentFrame, true); // departure event
            _triggeredDepartureEvent = true;
        }

        // not waiting anymore
        SetMoving(true);

        // Enable movement
        if (GetGameObjectProperties()->mo_transport.can_be_stopped)
            setState(GO_STATE_OPEN);

        if (timer >= _currentFrame->DepartureTime && timer < _currentFrame->NextArriveTime)
            break; // found current waypoint

        MoveToNextWaypoint();

        // Departure event
        if (_currentFrame->isTeleportFrame())
            if (TeleportTransport(_nextFrame->Node.mapid, _nextFrame->Node.x, _nextFrame->Node.y, _nextFrame->Node.z, _nextFrame->InitialOrientation))
                return; // Update more in new map thread
    }

    // Set position
    _positionChangeTimer -= time_passed;
    if (_positionChangeTimer <= 0)
    {
        _positionChangeTimer = positionUpdateDelay;
        if (IsMoving())
        {
            // Return a Value between 0 and 1 which represents the time from 0 to 1 between current and next node.
            float t = !justStopped ? CalculateSegmentPos(float(timer) * 0.001f) : 1.0f;
            G3D::Vector3 pos, dir;
            _currentFrame->Spline->evaluate_percent(_currentFrame->Index, t, pos);
            _currentFrame->Spline->evaluate_derivative(_currentFrame->Index, t, dir);
            UpdatePosition(pos.x, pos.y, pos.z, std::atan2(dir.y, dir.x) + float(M_PI));
        }
        else if (justStopped)
            UpdatePosition(_currentFrame->Node.x, _currentFrame->Node.y, _currentFrame->Node.z, _currentFrame->InitialOrientation);
    }
}

void Transporter::AddPassenger(Player* passenger)
{
    if (!IsInWorld())
        return;

    if (_passengers.insert(passenger).second)
    {
        passenger->SetTransport(this);
        passenger->obj_movement_info.addMovementFlag(MOVEFLAG_TRANSPORT);
    }
}

void Transporter::RemovePassenger(Object* passenger)
{
    bool erased = false;

    if (_passengerTeleportItr != _passengers.end())
    {
        PassengerSet::iterator itr = _passengers.find(passenger);
        if (itr != _passengers.end())
        {
            if (itr == _passengerTeleportItr)
                ++_passengerTeleportItr;

            _passengers.erase(itr);
            erased = true;
        }
    }
    else
        erased = _passengers.erase(passenger) > 0;

    if (erased || _staticPassengers.erase(passenger))
    {
        passenger->SetTransport(nullptr);
        passenger->obj_movement_info.removeMovementFlag(MOVEFLAG_TRANSPORT);
    }
}

Creature* Transporter::createNPCPassenger(MySQLStructure::CreatureSpawn* data)
{
    MapMgr* map = GetMapMgr();

    CreatureProperties const* creature_properties = sMySQLStore.getCreatureProperties(data->entry);
    if (creature_properties == nullptr || map == nullptr)
        return 0;

    Creature* pCreature = map->CreateCreature(data->entry);

    float x, y, z, o;
    x = data->x;
    y = data->y;
    z = data->z;
    o = data->o;

    pCreature->SetTransport(this);
    pCreature->obj_movement_info.setTransportData(this->getGuid(), x, y, z, o, 0, 0);

    CalculatePassengerPosition(x, y, z, &o);
    pCreature->SetPosition(x, y, z, o);
    pCreature->SetSpawnLocation(x, y, z, o);
    pCreature->SetTransportHomePosition(pCreature->obj_movement_info.transport_position);

    // Create Creature
    pCreature->Create(map->GetMapId(), x, y, z, o);
    pCreature->Load(creature_properties, x, y, z, o);

    // AddToWorld
    pCreature->AddToWorld(map);
    pCreature->setUnitMovementFlags(MOVEFLAG_TRANSPORT);
    pCreature->obj_movement_info.addMovementFlag(MOVEFLAG_TRANSPORT);

    // Equipment
    pCreature->setVirtualItemSlotId(MELEE, sMySQLStore.getItemDisplayIdForEntry(creature_properties->itemslot_1));
    pCreature->setVirtualItemSlotId(OFFHAND, sMySQLStore.getItemDisplayIdForEntry(creature_properties->itemslot_2));
    pCreature->setVirtualItemSlotId(RANGED, sMySQLStore.getItemDisplayIdForEntry(creature_properties->itemslot_3));

    if (data->emote_state)
        pCreature->setEmoteState(data->emote_state);

    if (creature_properties->NPCFLags)
        pCreature->setNpcFlags(creature_properties->NPCFLags);

    _staticPassengers.insert(pCreature);
    return pCreature;
}

GameObject* Transporter::createGOPassenger(MySQLStructure::GameobjectSpawn* data)
{
    MapMgr* map = GetMapMgr();

    const auto properties = sMySQLStore.getGameObjectProperties(data->entry);
    if (properties == nullptr || map == nullptr)
        return 0;

    float x, y, z, o;

    x = data->position_x;
    y = data->position_y;
    z = data->position_z;
    o = data->rotation_0;

    GameObject* pGameobject = map->CreateGameObject(data->entry);

    if (!pGameobject->CreateFromProto(data->entry, map->GetMapId(), x, y, z, o))
        return 0;

    pGameobject->SetTransport(this);
    pGameobject->obj_movement_info.setTransportData(this->getGuid(), x, y, z, o, 0, 0);

    CalculatePassengerPosition(x, y, z, &o);
    pGameobject->SetPosition(x, y, z, o);

    pGameobject->setAnimationProgress(255);

    // AddToWorld
    pGameobject->AddToWorld(map);

    _staticPassengers.insert(pGameobject);
    return pGameobject;
}

void Transporter::UpdatePosition(float x, float y, float z, float o)
{
    // Update Gameobjects Position
    SetPosition(x, y, z, o);

    // Update Player Position
    UpdatePassengerPositions(_passengers);

    // Update Static Passengers ( Gameobjects and Creatures )
    UpdatePassengerPositions(_staticPassengers);
}

void Transporter::LoadStaticPassengers()
{
    LogNotice("TransportHandler : Start populating transport %u ", getEntry());
    {
        for (auto creature_spawn : sMySQLStore._creatureSpawnsStore[GetGameObjectProperties()->mo_transport.map_id])
        {
            if (createNPCPassenger(creature_spawn) == 0)
                LOG_ERROR("Failed to add npc entry: %u to transport: %u", creature_spawn->entry, getGuid());
        }

        /*for (auto go_spawn : sMySQLStore._gameobjectSpawnsStore[GetGameObjectProperties()->mo_transport.map_id])
        {
            if (createGOPassenger(go_spawn) == 0)
                LOG_ERROR("Failed to add go entry: %u to transport: %u", go_spawn->entry, getGuid());
        }*/
    }
}

void Transporter::UnloadStaticPassengers()
{
    while (!_staticPassengers.empty())
    {
        Object* obj = *_staticPassengers.begin();
        if (obj->IsInWorld())
            obj->Delete();

        RemovePassenger(obj);
    }
}

void Transporter::UpdatePassengerPositions(PassengerSet& passengers)
{
    for (PassengerSet::iterator itr = passengers.begin(); itr != passengers.end(); ++itr)
    {
        Object* passenger = *itr;
        // transport teleported but passenger not yet (can happen for players)
        if (passenger->GetMapId() != GetMapId())
            continue;

        float x, y, z, o;
        passenger->obj_movement_info.transport_position.getPosition(x, y, z, o);
        CalculatePassengerPosition(x, y, z, &o);
        switch (passenger->getObjectTypeId())
        {
        case TYPEID_PLAYER:
        {
            Player* player = reinterpret_cast<Player*>(passenger);
            // Relocate only passengers in world and skip any player that might be still logging in/teleporting
            if (passenger->IsInWorld())
                player->SetPosition(x, y, z, o);
            break;
        }
        case TYPEID_UNIT:
        {
            Creature* creature = static_cast<Creature*>(passenger);
            creature->SetPosition(x, y, z, o, false);
            creature->GetTransportHomePosition(x, y, z, o);
            CalculatePassengerPosition(x, y, z, &o);
            creature->SetSpawnLocation(x, y, z, o);
            break;
        }
        case TYPEID_GAMEOBJECT:
        {
            GameObject* gameobject = static_cast<GameObject*>(passenger);
            gameobject->SetPosition(x, y, z, o, false);
            break;
        }
        }
    }
}

void Transporter::EnableMovement(bool enabled, MapMgr* instance)
{
    if (!GetGameObjectProperties()->mo_transport.can_be_stopped)
        return;

    _pendingStop = !enabled;
    UpdateForMap(instance);
}

void Transporter::MoveToNextWaypoint()
{
    // Clear events flagging
    _triggeredArrivalEvent = false;
    _triggeredDepartureEvent = false;

    // Set frames
    _currentFrame = _nextFrame++;
    if (_nextFrame == GetKeyFrames().end())
        _nextFrame = GetKeyFrames().begin();
}

float Transporter::CalculateSegmentPos(float now)
{
    KeyFrame const& frame = *_currentFrame;
    const float speed = float(GetGameObjectProperties()->mo_transport.move_speed);
    const float accel = float(GetGameObjectProperties()->mo_transport.accel_rate);
    float timeSinceStop = frame.TimeFrom + (now - (1.0f / IN_MILLISECONDS) * frame.DepartureTime);
    float timeUntilStop = frame.TimeTo - (now - (1.0f / IN_MILLISECONDS) * frame.DepartureTime);
    float segmentPos, dist;
    float accelTime = _transportInfo->accelTime;
    float accelDist = _transportInfo->accelDist;
    // calculate from nearest stop, less confusing calculation...
    if (timeSinceStop < timeUntilStop)
    {
        if (timeSinceStop < accelTime)
            dist = 0.5f * accel * timeSinceStop * timeSinceStop;
        else
            dist = accelDist + (timeSinceStop - accelTime) * speed;
        segmentPos = dist - frame.DistSinceStop;
    }
    else
    {
        if (timeUntilStop < _transportInfo->accelTime)
            dist = 0.5f * accel * timeUntilStop * timeUntilStop;
        else
            dist = accelDist + (timeUntilStop - accelTime) * speed;
        segmentPos = frame.DistUntilStop - dist;
    }

    return segmentPos / frame.NextDistFromPrev;
}

bool Transporter::TeleportTransport(uint32_t newMapid, float x, float y, float z, float o)
{
    MapMgr* oldMap = GetMapMgr();

    if (oldMap->GetMapId() != newMapid) // New Map case
    {
        // Unload at old Map
        _delayedTeleport = true;
        UnloadStaticPassengers();
        // Wait a bit before we Procced in new MapMgr
        sEventMgr.AddEvent(this, &Transporter::DelayedTeleportTransport, (oldMap), EVENT_TRANSPORTER_DELAYED_TELEPORT, 500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        return true;
    }
    else // Same Map Case
    {
        // Teleport Players
        TeleportPlayers(x, y, z, o, oldMap->GetMapId(), oldMap->GetMapId(), false);

        // Update Transport Positions
        UpdatePosition(x, y, z, o);
        return false;
    }
}

void Transporter::DelayedTeleportTransport(MapMgr* oldMap)
{
    if (!_delayedTeleport)
        return;

    _delayedTeleport = false;
    oldMap->RemoveFromMapMgr(this, false);
    RemoveFromWorld(false);

    // Set new Map Information
    SetMapId(_nextFrame->Node.mapid);

    float x = _nextFrame->Node.x,
        y = _nextFrame->Node.y,
        z = _nextFrame->Node.z,
        o = _nextFrame->InitialOrientation;

    SetPosition(x, y, z, o, false);

    // Add new Object to new MapMgr
    AddToWorld();
    GetMapMgr()->AddToMapMgr(this);

    // Teleport Players
    TeleportPlayers(x, y, z, o, _nextFrame->Node.mapid, oldMap->GetMapId(), true);

    // Update Transport Positions
    UpdatePosition(x, y, z, o);

    LoadStaticPassengers();
}

void Transporter::TeleportPlayers(float x, float y, float z, float o, uint32_t newMapId, uint32_t oldMapId, bool newMap)
{
    for (PassengerSet::iterator itr = _passengers.begin(); itr != _passengers.end(); ++itr)
    {
        if ((*itr)->getObjectTypeId() == TYPEID_PLAYER)
        {
            Player* player = reinterpret_cast<Player*>(*itr);

            float destX, destY, destZ, destO;
            LocationVector transPos = (*itr)->obj_movement_info.transport_position;
            transPos.getPosition(destX, destY, destZ, destO);
            TransportBase::CalculatePassengerPosition(destX, destY, destZ, &destO, x, y, z, o);

            if (newMap)
                player->GetSession()->SendPacket(SmsgTransferPending(newMapId, true, getEntry(), oldMapId).serialise().get());

            bool teleport_successful = player->Teleport(LocationVector(destX, destY, destZ, destO), GetMapMgr());
            if (!teleport_successful)
            {
                player->RepopAtGraveyard(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId());
            }
        }
    }
}

void Transporter::UpdateForMap(MapMgr* targetMap)
{
    if (!targetMap->HasPlayers())
        return;

    if (GetMapId() == targetMap->GetMapId())
    {
        for (auto itr = targetMap->m_PlayerStorage.begin(); itr != targetMap->m_PlayerStorage.end(); ++itr)
        {
            ByteBuffer transData(500);
            uint32_t count = 0;
            count = Object::buildCreateUpdateBlockForPlayer(&transData, itr->second);
            itr->second->getUpdateMgr().pushUpdateData(&transData, count);
        }
    }
}

uint32_t Transporter::buildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target)
{
    uint32_t cnt = Object::buildCreateUpdateBlockForPlayer(data, target);

    // add all the npcs and gos to the packet
    for (auto itr = _staticPassengers.begin(); itr != _staticPassengers.end(); ++itr)
    {
        Object* passenger = *itr;
        float x, y, z, o;
        passenger->obj_movement_info.transport_position.getPosition(x, y, z, o);
        CalculatePassengerPosition(x, y, z, &o);
        switch (passenger->getObjectTypeId())
        {
        case TYPEID_UNIT:
        {
            Creature* creature = static_cast<Creature*>(passenger);
            creature->SetPosition(x, y, z, o, false);
            creature->GetTransportHomePosition(x, y, z, o);
            CalculatePassengerPosition(x, y, z, &o);
            creature->SetSpawnLocation(x, y, z, o);
            break;
        }
        case TYPEID_GAMEOBJECT:
        {
            GameObject* gameobject = static_cast<GameObject*>(passenger);
            gameobject->SetPosition(x, y, z, o, false);
            break;
        }
        }
        cnt += passenger->buildCreateUpdateBlockForPlayer(data, target);
    }
    return cnt;
}

void Transporter::DoEventIfAny(KeyFrame const& node, bool departure)
{
    if (uint32_t eventid = departure ? node.Node.DepartureEventID : node.Node.ArrivalEventID)
    {
        LOG_DETAIL("Taxi %s event %u", departure ? "departure" : "arrival", eventid);

        // Use MapScript Interface to Handle these if not handle it here
        if (GetMapMgr()->GetScript())
            GetMapMgr()->GetScript()->TransporterEvents(this, eventid);
        else
        {
            // TODO Sort out ships and zeppelins
            switch (eventid)
            {
            case 16501:
            case 16400:
            case 19126:
            case 15318:
            case 19032:
            case 10301:
            case 19124:
            case 16398:
            case 19139:
            case 16396:
            case 16402:
            case 15314:
                PlaySoundToSet(5154);   // ShipDocked         LightHouseFogHorn.wav
                break;
            case 16401:
                PlaySoundToSet(11804);  // ZeppelinDocked     ZeppelinHorn.wav
                break;
            }
        }
    }
}