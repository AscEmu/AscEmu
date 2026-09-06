/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <G3D/Vector3.h>
#include "Objects/Transporter.hpp"

#include "GameObjectProperties.hpp"
#include "Data/Flags.hpp"
#include "Logging/Logger.hpp"
#include "Management/TransporterHandler.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Map/Visibility/VisibilitySystem.hpp"
#include "Models/GameObjectModel.h"
#include "Server/Packets/SmsgTransferPending.h"
#include "Movement/Spline/Spline.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Server/Definitions.h"
#include "Server/WorldSession.h"
#include "Server/Script/InstanceScript.hpp"
#include "Units/Creatures/Creature.h"
#include "Units/Creatures/Vehicle.hpp"
#include "Units/Players/Player.hpp"

using namespace AscEmu::Packets;

namespace
{
    void applyTransportPassengerInterest(WorldMap* map, Object* passenger)
    {
        if (!map || !passenger)
            return;

        auto h = map->getSpatialIndex().handleByGuid(passenger->GetNewGUID());
        if (!h.id)
            return;

        auto interest = map->getVisibilitySystem().buildInterestProfile(passenger);
        interest.publishMode = visibility::PublishMode::CellRadius;
        interest.publishCells = 4;
        interest.publishPlayersOnly = true;

        map->getVisibilitySystem().applyInterestProfile(h, interest);
    }

    void restorePassengerInterest(WorldMap* map, Object* passenger)
    {
        if (!map || !passenger)
            return;

        auto h = map->getSpatialIndex().handleByGuid(passenger->GetNewGUID());
        if (!h.id)
            return;

        map->getVisibilitySystem().applyInterestProfile(
            h, map->getVisibilitySystem().buildInterestProfile(passenger));
    }
}

Transporter::Transporter(uint64_t guid) : GameObject(guid), _passengerTeleportItr(_passengers.begin())
{
#if VERSION_STRING == Classic
    m_updateFlag = (UPDATEFLAG_TRANSPORT | UPDATEFLAG_ALL | UPDATEFLAG_HAS_POSITION);
#endif
#if VERSION_STRING == TBC
    m_updateFlag = (UPDATEFLAG_TRANSPORT | UPDATEFLAG_LOWGUID | UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION);
#endif
#if VERSION_STRING == WotLK
    m_updateFlag = (UPDATEFLAG_TRANSPORT | UPDATEFLAG_LOWGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_ROTATION);
#endif
#if VERSION_STRING == Cata
    m_updateFlag = (UPDATEFLAG_HAS_POSITION | UPDATEFLAG_ROTATION);
#endif
#if VERSION_STRING == Mop
    m_updateFlag = (UPDATEFLAG_HAS_POSITION | UPDATEFLAG_ROTATION);
#endif
}

Transporter::~Transporter()
{
    ASSERT(_passengers.empty());
    _passengers.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Essential functions
void Transporter::onAttachToWorld()
{
    GameObject::onAttachToWorld();
}

void Transporter::onPreDetachFromWorld()
{
    sTransportHandler.removeInstancedTransport(this, GetInstanceID());

    UnloadStaticPassengers();
    while (!_passengers.empty())
    {
        Object* obj = *_passengers.begin();
        RemovePassenger(obj);
    }

    GameObject::onPreDetachFromWorld();
}

bool Transporter::Create(uint32_t entry, uint32_t mapid, float x, float y, float z, float ang, uint8_t animprogress)
{
    gameobject_properties = sMySQLStore.getGameObjectProperties(entry);
    if (gameobject_properties == nullptr)
    {
        sLogger.failure("Something tried to create a GameObject with invalid entry {}", entry);
        return false;
    }

    TransportTemplate const* tInfo = sTransportHandler.getTransportTemplate(entry);
    if (!tInfo)
    {
        sLogger.failure("Transport {} will not be created, missing `transport_template` entry.", entry);
        return false;
    }

    Object::_Create(mapid, x, y, z, ang);
    setEntry(entry);
    setLocalRotation(0.0f, 0.0f, 0.0f, 1.0f);
    setParentRotation(QuaternionData());
    SetPosition(LocationVector(x, y, z, ang));
    setDisplayId(gameobject_properties->display_id);
    setGoType(static_cast<uint8_t>(gameobject_properties->type));

    m_overrides = GAMEOBJECT_INFVIS | GAMEOBJECT_ONMOVEWIDE; //Make it forever visible on the same map;
    setFlags(GO_FLAG_TRANSPORT | GO_FLAG_NEVER_DESPAWN);
    setState(gameobject_properties->mo_transport.can_be_stopped ? GO_STATE_CLOSED : GO_STATE_OPEN);
    m_goValue.PathProgress = 0;

    setScale(gameobject_properties->size);

    InitAI();

    // Set Pathtime
    setLevel(tInfo->pathTime);
    setAnimationProgress(animprogress);

    m_model = createModel();

    updatePathProgress();

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
    if (!IsInWorld())
        return;

    if (GetKeyFrames().size() <= 1)
        return;

    if (IsMoving() || !_pendingStop)
        m_goValue.PathProgress += time_passed;

    uint32_t timer = m_goValue.PathProgress % getTransportPeriod();
    bool justStopped = false;

    //sLogger.debug("Transporter: current node {} and pathprogress {}.", _currentFrame->Index, GetTimer());

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
#if VERSION_STRING >= WotLK
                if (justStopped)
                    setDynamicFlags(GO_DYN_FLAG_TRANSPORT_STOPPED);
#endif

                SetMoving(false);
                if (_pendingStop && getState() != GO_STATE_CLOSED)
                {
                    setState(GO_STATE_CLOSED);
                    m_goValue.PathProgress = (m_goValue.PathProgress / getTransportPeriod());
                    m_goValue.PathProgress *= getTransportPeriod();
                    m_goValue.PathProgress += _currentFrame->ArriveTime;
                }
                break; // its a stop frame and we are waiting
            }
        }

        if (timer >= _currentFrame->DepartureTime && !_triggeredDepartureEvent)
        {
            DoEventIfAny(*_currentFrame, true); // departure event
            _triggeredDepartureEvent = true;
        }

#if VERSION_STRING >= WotLK
        if (!IsMoving())
            setDynamicFlags(GO_DYN_FLAG_NONE);
#endif

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

    // Add model to map after we are fully done with moving maps
    if (_delayedAddModel)
    {
        _delayedAddModel = false;
        if (m_model && getWorldMap())
            getWorldMap()->insertGameObjectModel(*m_model);
    }

    // Set position
    _positionChangeTimer -= time_passed;
    if (_positionChangeTimer <= 0)
    {
        _positionChangeTimer = positionUpdateDelay;
        if (IsMoving())
        {
            updatePathProgress();

            // Return a Value between 0 and 1 which represents the time from 0 to 1 between current and next node.
            float t = !justStopped ? CalculateSegmentPos(float(timer) * 0.001f) : 1.0f;
            G3D::Vector3 pos, dir;
            _currentFrame->Spline->evaluate_percent(_currentFrame->Index, t, pos);
            _currentFrame->Spline->evaluate_derivative(_currentFrame->Index, t, dir);
            UpdatePosition(pos.x, pos.y, pos.z, std::atan2(dir.y, dir.x) + AscEmu::Math::PiF);
        }
        else if (justStopped)
        {
            updatePathProgress();
            UpdatePosition(_currentFrame->Node.x, _currentFrame->Node.y, _currentFrame->Node.z, _currentFrame->InitialOrientation);
        }
        else // When Transport Stopped keep updating players position
            UpdatePlayerPositions(_passengers);
    }
}

void Transporter::delayedUpdate(unsigned long /*time_passed*/)
{
    if (GetKeyFrames().size() <= 1)
        return;

    delayedTeleportTransport();
}

void Transporter::AddPassenger(Object* passenger, const LocationVector& transportOffset)
{
    if (!passenger || !IsInWorld())
        return;

    if (_passengers.insert(passenger).second)
    {
        passenger->SetTransport(this);
        passenger->obj_movement_info.setTransportData(
            GetNewGUID(),
            transportOffset.x,
            transportOffset.y,
            transportOffset.z,
            transportOffset.o,
            GetTimer(),
            -1);

#if VERSION_STRING <= WotLK
        passenger->obj_movement_info.addMovementFlag(MOVEFLAG_TRANSPORT);
        if (auto* unit = passenger->ToUnit())
            unit->addUnitMovementFlag(MOVEFLAG_TRANSPORT);
#endif

        applyTransportPassengerInterest(getWorldMap(), passenger);

        if (auto* player = passenger->ToPlayer())
        {
            if (getWorldMap() && getWorldMap()->getScript())
                getWorldMap()->getScript()->TransportBoarded(player, this);
        }
    }
}

void Transporter::RestorePassengerAfterTeleport(Object* passenger, const LocationVector& transportOffset)
{
    if (!passenger || !IsInWorld())
        return;

    _passengers.insert(passenger);
    passenger->SetTransport(this);
    passenger->obj_movement_info.setTransportData(
        GetNewGUID(),
        transportOffset.x,
        transportOffset.y,
        transportOffset.z,
        transportOffset.o,
        GetTimer(),
        -1);

#if VERSION_STRING <= WotLK
    passenger->obj_movement_info.addMovementFlag(MOVEFLAG_TRANSPORT);
    if (auto* unit = passenger->ToUnit())
        unit->addUnitMovementFlag(MOVEFLAG_TRANSPORT);
#endif
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

    const bool staticPassengerErased = _staticPassengers.erase(passenger) > 0;
    if (erased || staticPassengerErased)
    {
        passenger->SetTransport(nullptr);
#if VERSION_STRING <= WotLK
        passenger->obj_movement_info.removeMovementFlag(MOVEFLAG_TRANSPORT);
        if (auto* unit = passenger->ToUnit())
            unit->removeUnitMovementFlag(MOVEFLAG_TRANSPORT);
#endif
        passenger->obj_movement_info.clearTransportData();

        if (erased)
            restorePassengerInterest(getWorldMap(), passenger);

        if (auto* player = passenger->ToPlayer())
        {
            if (getWorldMap() && getWorldMap()->getScript())
                getWorldMap()->getScript()->TransportUnboarded(player, this);
        }
    }
}

Creature* Transporter::createNPCPassenger(MySQLStructure::CreatureSpawn* data)
{
    WorldMap* map = getWorldMap();
    if (data == nullptr || map == nullptr)
        return nullptr;

    CreatureProperties const* creatureProperties = sMySQLStore.getCreatureProperties(data->entry);
    if (creatureProperties == nullptr)
        return nullptr;

    float localX = data->spawnPoint.x;
    float localY = data->spawnPoint.y;
    float localZ = data->spawnPoint.z;
    float localO = data->spawnPoint.o;

    float worldX = localX;
    float worldY = localY;
    float worldZ = localZ;
    float worldO = localO;
    calculatePassengerPosition(worldX, worldY, worldZ, &worldO);

    // Transport spawn rows use transport-local coordinates and may omit values
    // that normal map spawn rows override. Initialize the creature from its
    // creature properties while detached, then attach it only after the
    // transport-specific state is complete.
    Creature* creature = map->getObjectFactory().createCreature(data->entry);
    if (creature == nullptr)
        return nullptr;

    creature->Create(map->getBaseMap()->getMapId(), worldX, worldY, worldZ, worldO);
    creature->Load(creatureProperties, worldX, worldY, worldZ, worldO);

    creature->SetTransport(this);
    creature->obj_movement_info.setTransportData(getGuid(), localX, localY, localZ, localO, 0, 0);
    creature->SetPosition(worldX, worldY, worldZ, worldO);
    creature->SetSpawnLocation(worldX, worldY, worldZ, worldO);
    creature->SetTransportHomePosition(creature->obj_movement_info.transport_position);
    creature->addUnitStateFlag(UNIT_STATE_IGNORE_PATHFINDING);

#if VERSION_STRING <= WotLK
    creature->addUnitMovementFlag(MOVEFLAG_TRANSPORT);
    creature->obj_movement_info.addMovementFlag(MOVEFLAG_TRANSPORT);
#endif

    creature->setVirtualItemSlotId(MELEE, creatureProperties->itemslot_1);
    creature->setVirtualItemSlotId(OFFHAND, creatureProperties->itemslot_2);
    creature->setVirtualItemSlotId(RANGED, creatureProperties->itemslot_3);

    if (data->emote_state)
        creature->setEmoteState(data->emote_state);

    if (creatureProperties->NPCFLags)
        creature->setNpcFlags(creatureProperties->NPCFLags);

    map->getObjectFactory().attachToWorld(creature);
    if (!creature->IsInWorld())
    {
        map->getObjectFactory().recycleAndDestroy(creature, true);
        return nullptr;
    }

    applyTransportPassengerInterest(map, creature);
    _staticPassengers.insert(creature);
    return creature;
}

GameObject* Transporter::createGOPassenger(MySQLStructure::GameobjectSpawn* data)
{
    WorldMap* map = getWorldMap();
    if (data == nullptr || map == nullptr)
        return nullptr;

    if (sMySQLStore.getGameObjectProperties(data->entry) == nullptr)
        return nullptr;

    // Load the complete DB-backed object before it is inserted into collision or
    // spatial structures. Reinitializing an attached GameObject can replace its
    // collision model while the old model is still referenced by the map tree.
    GameObject* gameObject = map->getObjectFactory().createGameObjectFromSpawns(*data);
    if (gameObject == nullptr)
        return nullptr;

    float x, y, z, o;
    data->spawnPoint.getPosition(x, y, z, o);

    gameObject->SetTransport(this);
    gameObject->obj_movement_info.setTransportData(getGuid(), x, y, z, o, 0, 0);

    calculatePassengerPosition(x, y, z, &o);
    gameObject->SetPosition(x, y, z, o);
    gameObject->setAnimationProgress(255);

    map->getObjectFactory().attachToWorld(gameObject);
    if (!gameObject->IsInWorld())
    {
        map->getObjectFactory().recycleAndDestroy(gameObject, true);
        return nullptr;
    }

    applyTransportPassengerInterest(map, gameObject);
    _staticPassengers.insert(gameObject);
    return gameObject;
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
    // If parameter_6 (map id) is set to 0, then transport shouldn't have any passengers
    if (GetGameObjectProperties()->mo_transport.map_id == 0)
        return;

    sLogger.debugMap("TransportHandler : Start populating transport {}.", getEntry());
    {
        for (auto creature_spawn : sMySQLStore._creatureSpawnsStore[GetGameObjectProperties()->mo_transport.map_id])
        {
            if (createNPCPassenger(creature_spawn) == 0)
                sLogger.failure("Failed to add npc entry: {} to transport: {}", creature_spawn->entry, getGuid());
        }

        for (auto go_spawn : sMySQLStore._gameobjectSpawnsStore[GetGameObjectProperties()->mo_transport.map_id])
        {
            if (createGOPassenger(go_spawn) == 0)
                sLogger.failure("Failed to add go entry: {} to transport: {}", go_spawn->entry, getGuid());
        }
    }
}

void Transporter::UnloadStaticPassengers()
{
    while (!_staticPassengers.empty())
    {
        Object* obj = *_staticPassengers.begin();
        if (obj)
        {
            RemovePassenger(obj);

            WorldMap* map = getWorldMap();
            if (map != nullptr)
                map->getObjectFactory().removeAndDestroy(obj, true);
        }
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

#ifdef FT_VEHICLES
        // if passenger is on vehicle we have to assume the vehicle is also on transport
        // and its the vehicle that will be updating its passengers
        if (Unit* unit = passenger->ToUnit())
            if (unit->getVehicle())
                continue;
#endif

        float x, y, z, o;
        passenger->obj_movement_info.transport_position.getPosition(x, y, z, o);
        calculatePassengerPosition(x, y, z, &o);
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
                calculatePassengerPosition(x, y, z, &o);
                creature->SetSpawnLocation(x, y, z, o);
                break;
            }
            case TYPEID_GAMEOBJECT:
            {
                GameObject* gameobject = static_cast<GameObject*>(passenger);
                gameobject->SetPosition(x, y, z, o, false);
                break;
            }
            default:
                break;
        }
#ifdef FT_VEHICLES
        if (Unit* unit = passenger->ToUnit())
            if (Vehicle* vehicle = unit->getVehicleKit())
                vehicle->relocatePassengers();
#endif
    }
}

void Transporter::UpdatePlayerPositions(PassengerSet& passengers)
{
    for (PassengerSet::iterator itr = passengers.begin(); itr != passengers.end(); ++itr)
    {
        Object* passenger = *itr;
        // transport teleported but passenger not yet (can happen for players)
        if (passenger->GetMapId() != GetMapId())
            continue;

        float x, y, z, o;
        passenger->obj_movement_info.transport_position.getPosition(x, y, z, o);
        calculatePassengerPosition(x, y, z, &o);
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
        }
    }
}

void Transporter::EnableMovement(bool enabled, WorldMap* /*instance*/)
{
    if (!GetGameObjectProperties()->mo_transport.can_be_stopped)
        return;

    _pendingStop = !enabled;
    //UpdateForMap(instance);
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
    float timeSinceStop = frame.TimeFrom + (now - (1.0f / static_cast<float>(IN_MILLISECONDS)) * frame.DepartureTime);
    float timeUntilStop = frame.TimeTo - (now - (1.0f / static_cast<float>(IN_MILLISECONDS)) * frame.DepartureTime);
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

void Transporter::removeFromMap()
{
    UnloadStaticPassengers();
    destroy();
}

void Transporter::calculatePassengerPosition(float& x, float& y, float& z, float* o)
{
    TransportBase::CalculatePassengerPosition(x, y, z, o, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
}

void Transporter::calculatePassengerOffset(float& x, float& y, float& z, float* o)
{
    TransportBase::CalculatePassengerOffset(x, y, z, o, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
}

bool Transporter::TeleportTransport(uint32_t newMapid, float x, float y, float z, float o)
{
    WorldMap* oldMap = getWorldMap();

    if (oldMap->getBaseMap()->getMapId() != newMapid) // New Map case
    {
        // Unload at old Map
        UnloadStaticPassengers();
        _delayedTeleport = true;
        return true;
    }
    else // Same Map Case
    {
        // Teleport Players
        TeleportPlayers(x, y, z, o, oldMap->getBaseMap()->getMapId(), oldMap->getBaseMap()->getMapId(), false);

        // Update Transport Positions
        UpdatePosition(x, y, z, o);
        return false;
    }
}

void Transporter::delayedTeleportTransport()
{
    if (!_delayedTeleport)
        return;

    _delayedTeleport = false;

    WorldMap* newMap = sMapMgr.findWorldMap(_nextFrame->Node.mapid, GetInstanceID());
    WorldMap* oldMap = getWorldMap();

    // Set new Map Information
    SetMapId(_nextFrame->Node.mapid);

    float x = _nextFrame->Node.x,
        y = _nextFrame->Node.y,
        z = _nextFrame->Node.z,
        o = _nextFrame->InitialOrientation;

    SetPosition(x, y, z, o, false);

    // The target map finishes the attach on its own thread.
    oldMap->getObjectFactory().transferWorld(this, newMap);
}

void Transporter::TeleportPlayers(float x, float y, float z, float o, uint32_t newMapId, uint32_t oldMapId, bool newMap, WorldMap* targetMap)
{
    for (_passengerTeleportItr = _passengers.begin(); _passengerTeleportItr != _passengers.end();)
    {
        Object* obj = (*_passengerTeleportItr++);

        if (obj->isPlayer())
        {
            Player* player = reinterpret_cast<Player*>(obj);

            float destX, destY, destZ, destO;
            LocationVector transPos = obj->obj_movement_info.transport_position;
            transPos.getPosition(destX, destY, destZ, destO);
            TransportBase::CalculatePassengerPosition(destX, destY, destZ, &destO, x, y, z, o);

            if (newMap)
            {
                // Preserve the transport identity and local passenger offset across
                // the worldport. The relationship is restored explicitly when the
                // client acknowledges the destination map.
                player->setTeleportTransport(GetNewGUID(), transPos);

                SmsgTransferPending managedPacket(newMapId, true, getEntry(), oldMapId);
                player->getSession()->sendManagedPacket(managedPacket);
            }

            WorldMap* destinationMap = targetMap ? targetMap : getWorldMap();
            bool teleport_successful = player->teleport(LocationVector(destX, destY, destZ, destO), destinationMap);
            if (!teleport_successful)
            {
                player->repopAtGraveyard(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId());
            }
        }
    }
}

void Transporter::UpdateForMap(WorldMap* targetMap)
{
    if (!targetMap->getRegistry().countPlayers())
        return;

    if (GetMapId() == targetMap->getBaseMap()->getMapId())
    {
        thread_local std::vector<Player*> s_players;
        targetMap->getRegistry().snapshotPlayers(s_players);

        targetMap->getRegistry().forEachPinned(s_players, [&](Player& player)
            {
                if (player.getWorldMap() != targetMap)
                    return;

                ByteBuffer transData(500);
                uint32_t count = 0;
                count = Object::buildCreateUpdateBlockForPlayer(&transData, &player);
                player.getUpdateMgr().pushUpdateData(&transData, count);
            });
    }
}

void Transporter::DoEventIfAny(KeyFrame const& node, bool departure)
{
    if (uint32_t eventid = departure ? node.Node.DepartureEventID : node.Node.ArrivalEventID)
    {
        sLogger.debugMap("Taxi {} event {}.", departure ? "departure" : "arrival", eventid);

        // Use MapScript Interface to Handle these if not handle it here
        if (getWorldMap() && getWorldMap()->getScript())
            getWorldMap()->getScript()->TransporterEvents(this, eventid);

        switch (eventid)
        {
            case 16400:
            case 16395:
            case 16399:
            case 19033:
            case 10302:
            case 19031:
            case 16397:
            case 16402:
            case 16396:
            case 16398:
            case 19032:
            case 19030:
            case 10301:
            case 19124:
            case 15314:
                PlaySoundToSet(5154);   // ShipDocked         LightHouseFogHorn.wav
                break;

            case 19139:
            case 15318:
            case 19126:
            case 19137:
            case 15312:
            case 15320:
            case 15322:
            case 19127:
            case 21870:
            case 15324:
            case 16401:
                PlaySoundToSet(11804);  // ZeppelinDocked     ZeppelinHorn.wav
                break;

            default:
                break;
            }
    }
}

void Transporter::updatePathProgress()
{
#if VERSION_STRING >= WotLK
    int16_t pathProgress = -1; // dynamic Path Progress

    if (uint32_t transportPeriod = getTransportPeriod())
    {
        float timer = float(getGOValue()->PathProgress % transportPeriod);
        pathProgress = int16_t(timer / float(transportPeriod) * 65535.0f);
    }

    // Set Updatemask
    setDynamicPathProgress(pathProgress);
#endif
}
