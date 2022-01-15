/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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


#include "Storage/MySQLDataStore.hpp"
#include "Map/MapMgr.h"
#include "Spell/SpellAuras.h"
#include "Spell/Definitions/PowerType.hpp"
#include "Server/Packets/SmsgControlVehicle.h"
#include "Server/Script/CreatureAIScript.h"
#include "Movement/MovementManager.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Storage/DBC/DBCStructures.hpp"
#include "Pet.h"

Vehicle::Vehicle(Unit* unit, DBC::Structures::VehicleEntry const* vehInfo, uint32_t creatureEntry) :
    usableSeatNum(0), _owner(unit), _vehicleInfo(vehInfo), _creatureEntry(creatureEntry), _status(STATUS_NONE), _lastShootPos()
{
    for (uint32_t i = 0; i < MAX_VEHICLE_SEATS; ++i)
    {
        if (uint32_t seatId = _vehicleInfo->seatID[i])
            if (auto veSeat = sVehicleSeatStore.LookupEntry(seatId))
            {
                VehicleSeatAddon const* addon = sObjectMgr.getVehicleSeatAddon(seatId);
                Seats.insert(std::make_pair(i, VehicleSeat(veSeat, addon)));
                if (veSeat->canEnterOrExit())
                    ++usableSeatNum;
            }
    }

    // Set or remove correct flags based on available seats. Will overwrite db data (if wrong).
    if (usableSeatNum)
        getBase()->setNpcFlags((getBase()->getObjectTypeId() == TYPEID_PLAYER ? UNIT_NPC_FLAG_PLAYER_VEHICLE : UNIT_NPC_FLAG_SPELLCLICK));
    else
        getBase()->removeNpcFlags((getBase()->getObjectTypeId() == TYPEID_PLAYER ? UNIT_NPC_FLAG_PLAYER_VEHICLE : UNIT_NPC_FLAG_SPELLCLICK));


    switch (vehInfo->powerType)
    {
        case POWER_TYPE_STEAM:
        case POWER_TYPE_HEAT:
        case POWER_TYPE_BLOOD:
        case POWER_TYPE_OOZE:
        case POWER_TYPE_WRATH:
            _owner->setPowerType(POWER_TYPE_ENERGY);
            _owner->setMaxPower(POWER_TYPE_ENERGY, 100);
            _owner->setPower(POWER_TYPE_ENERGY, 100);
            break;

        case POWER_TYPE_PYRITE:
            _owner->setPowerType(POWER_TYPE_ENERGY);
            _owner->setMaxPower(POWER_TYPE_ENERGY, 50);
            _owner->setPower(POWER_TYPE_ENERGY, 50);
            break;
    }

    initMovementInfoForBase();
}

void Vehicle::initMovementInfoForBase()
{
    uint32_t vehicleFlags = getVehicleInfo()->flags;

    if (vehicleFlags & VEHICLE_FLAG_NO_STRAFE)
        getBase()->addExtraUnitMovementFlag(MOVEFLAG2_NO_STRAFING);
    if (vehicleFlags & VEHICLE_FLAG_NO_JUMPING)
        getBase()->addExtraUnitMovementFlag(MOVEFLAG2_NO_JUMPING);
    if (vehicleFlags & VEHICLE_FLAG_FULLSPEEDTURNING)
        getBase()->addExtraUnitMovementFlag(MOVEFLAG2_FULLSPEED_TURNING);
    if (vehicleFlags & VEHICLE_FLAG_ALLOW_PITCHING)
        getBase()->addExtraUnitMovementFlag(MOVEFLAG2_ALLOW_PITCHING);
    if (vehicleFlags & VEHICLE_FLAG_FULLSPEEDPITCHING)
        getBase()->addExtraUnitMovementFlag(MOVEFLAG2_FULLSPEED_PITCHING);
}

Vehicle::~Vehicle()
{
    if (_status == STATUS_UNINSTALLING)
    {
        for (SeatMap::const_iterator itr = Seats.begin(); itr != Seats.end(); ++itr)
            if (!itr->second.isEmpty())
                sLogger.failure("Vehicle is not Empty");
    }
    else
        sLogger.failure("Vehicle Accessory Status is not on STATUS_UNINSTALLING");
}

void Vehicle::install()
{
    _status = STATUS_INSTALLED;
    if (getBase()->getObjectTypeId() == TYPEID_UNIT)
    {
        // Script call here
    }
}

void Vehicle::uninstall()
{
    if (_status == STATUS_UNINSTALLING)
    {
        sLogger.failure("Vehicle %s attempts to uninstall, but already has STATUS_UNINSTALLING! ", getBase()->getGuid());
        return;
    }

    _status = STATUS_UNINSTALLING;
    removeAllPassengers();

    if (getBase()->getObjectTypeId() == TYPEID_UNIT)
    {
        // Script call here
    }
}

void Vehicle::installAllAccessories(bool evading)
{
    if (getBase()->getObjectTypeId() == TYPEID_PLAYER || !evading)
        removeAllPassengers();

    VehicleAccessoryList const* accessories = sObjectMgr.getVehicleAccessories(this);
    if (!accessories)
        return;

    for (VehicleAccessoryList::const_iterator itr = accessories->begin(); itr != accessories->end(); ++itr)
        if (!evading || itr->isMinion)  // only install minions on evade mode
            installAccessory(itr->accessoryEntry, itr->seatId, itr->isMinion, itr->summonedType, itr->summonTime);
}

void Vehicle::installAccessory(uint32_t entry, int8_t seatId, bool minion, uint8_t type, uint32_t summonTime)
{
    if (_status == STATUS_UNINSTALLING)
    {
        sLogger.failure("Vehicle (%s, Entry: %u) attempts to install accessory (Entry: %u) on seat %d with STATUS_UNINSTALLING! ", getBase()->getGuid(), getEntry(), entry, (int32_t)seatId);
        return;
    }

    CreatureProperties const* cp = sMySQLStore.getCreatureProperties(entry);
    if (cp == nullptr)
        return;

    Creature* accessory = getBase()->GetMapMgr()->CreateCreature(entry);
    accessory->Load(cp, getBase()->GetPositionX(), getBase()->GetPositionY(), getBase()->GetPositionZ(), getBase()->GetOrientation());
    accessory->setPhase(PHASE_SET, getBase()->GetPhase());
    accessory->setFaction(getBase()->getFactionTemplate());
    accessory->PushToWorld(getBase()->GetMapMgr());

    accessory->obj_movement_info.addMovementFlag(MOVEFLAG_TRANSPORT);
    accessory->addUnitMovementFlag(MOVEFLAG_TRANSPORT);

    if (minion)
        accessory->addUnitStateFlag(UNIT_STATE_ACCESSORY);

    //sEventMgr.AddEvent(getBase()->ToUnit(), &Unit::handleSpellClick, accessory->ToUnit(), seatId, 0, 2000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    getBase()->handleSpellClick(accessory, seatId);
}

void Vehicle::applyAllImmunities()
{
    // This couldn't be done in DB, because some spells have MECHANIC_NONE

    // Vehicles should be immune on Knockback ...
    // toDo

    // Mechanical units & vehicles ( which are not Bosses, they have own immunities in DB ) should be also immune on healing ( exceptions in switch below )
    if (getBase()->ToCreature() && getBase()->ToCreature()->GetCreatureProperties()->Type == UNIT_TYPE_MECHANICAL && !getBase()->ToCreature()->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
    {
        // Heal & dispel ...
        // toDo

        // ... Shield & Immunity grant spells ...
        // toDo

        // ... Resistance, Split damage, Change stats ...
        // toDo
    }

    // If vehicle flag for fixed position set (cannons), or if the following hardcoded units, then set state rooted
    //  30236 | Argent Cannon
    //  39759 | Tankbuster Cannon
    if ((getVehicleInfo()->flags & VEHICLE_FLAG_FIXED_POSITION))
        getBase()->setControlled(true, UNIT_STATE_ROOTED);

    switch (getBase()->getEntry())
    {
        case 30236:
        case 39759:
            getBase()->setControlled(true, UNIT_STATE_ROOTED);
        break;

    }

    // Different immunities for vehicles goes below
    switch (getVehicleInfo()->ID)
    {
        // code below prevents a bug with movable cannons
    case 160: // Strand of the Ancients
    case 244: // Wintergrasp
    case 510: // Isle of Conquest
    case 452: // Isle of Conquest
    case 543: // Isle of Conquest
        getBase()->setControlled(true, UNIT_STATE_ROOTED);
        // toDo
        break;
    case 335: // Salvaged Chopper
    case 336: // Salvaged Siege Engine
    case 338: // Salvaged Demolisher
        // toDo
        break;
    default:
        break;
    }
}

void Vehicle::removeAllPassengers()
{
    // Passengers always cast an aura with SPELL_AURA_CONTROL_VEHICLE on the vehicle
    getBase()->removeAllAurasByAuraEffect(SPELL_AURA_CONTROL_VEHICLE);
}

bool Vehicle::hasEmptySeat(int8_t seatId) const
{
    SeatMap::const_iterator seat = Seats.find(seatId);
    if (seat == Seats.end())
        return false;
    return seat->second.isEmpty();
}

Unit* Vehicle::getPassenger(int8_t seatId) const
{
    SeatMap::const_iterator seat = Seats.find(seatId);
    if (seat == Seats.end())
        return nullptr;

    return getBase()->GetMapMgrUnit(seat->second._passenger.guid);
}

SeatMap::const_iterator Vehicle::getNextEmptySeat(int8_t seatId, bool next) const
{
    SeatMap::const_iterator seat = Seats.find(seatId);
    if (seat == Seats.end())
        return seat;

    while (!seat->second.isEmpty() || (!seat->second._seatInfo->canEnterOrExit() && !seat->second._seatInfo->isUsableByOverride()))
    {
        if (next)
        {
            if (++seat == Seats.end())
                seat = Seats.begin();
        }
        else
        {
            if (seat == Seats.begin())
                seat = Seats.end();
            --seat;
        }

        // Make sure we don't loop indefinetly
        if (seat->first == seatId)
            return Seats.end();
    }

    return seat;
}

VehicleSeatAddon const* Vehicle::getSeatAddonForSeatOfPassenger(Unit const* passenger) const
{
    for (SeatMap::const_iterator itr = Seats.begin(); itr != Seats.end(); itr++)
        if (!itr->second.isEmpty() && itr->second._passenger.guid == passenger->getGuid())
            return itr->second._seatAddon;

    return nullptr;
}

bool Vehicle::addPassenger(Unit* unit, int8_t seatId)
{
    if (_status == STATUS_UNINSTALLING)
    {
        sLogger.failure("Passenger %s, attempting to board vehicle %s during uninstall! SeatId: %d", unit->getGuid(), getBase()->getGuidHigh(), (int32_t)seatId);
        return false;
    }

    SeatMap::iterator seat;
    if (seatId < 0) // no specific seat requirement
    {
        for (seat = Seats.begin(); seat != Seats.end(); ++seat)
            if (seat->second.isEmpty() && (seat->second._seatInfo->canEnterOrExit() || seat->second._seatInfo->isUsableByOverride()))
                break;

        if (seat == Seats.end()) // no available seat
            return false;

        tryAddPassenger(unit, seat);
    }
    else
    {
        seat = Seats.find(seatId);
        if (seat == Seats.end())
            return false;

        // when there is already an Unit in the requested seat remove him
        if (!seat->second.isEmpty())
        {
            Unit* passenger = getBase()->GetMapMgrUnit(seat->second._passenger.guid);
            if (passenger)
                passenger->exitVehicle();
        }

        tryAddPassenger(unit, seat);
    }

    return true;
}

Vehicle* Vehicle::removePassenger(Unit* unit)
{
    if (unit->getVehicle() != this)
        return nullptr;

    SeatMap::iterator seat = getSeatIteratorForPassenger(unit);
    ASSERT(seat != Seats.end());

    if (seat->second._seatInfo->canEnterOrExit() && ++usableSeatNum)
        getBase()->setNpcFlags((getBase()->getObjectTypeId() == TYPEID_PLAYER ? UNIT_NPC_FLAG_PLAYER_VEHICLE : UNIT_NPC_FLAG_SPELLCLICK));

    // Remove UNIT_FLAG_NOT_SELECTABLE if passenger did not have it before entering vehicle
    if (seat->second._seatInfo->flags & DBC::Structures::VehicleSeatFlags::VEHICLE_SEAT_FLAG_PASSENGER_NOT_SELECTABLE && !seat->second._passenger.isUnselectable)
        unit->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);

    seat->second._passenger.reset();

    if (getBase()->getObjectTypeId() == TYPEID_UNIT && unit->getObjectTypeId() == TYPEID_PLAYER && seat->second._seatInfo->flags & DBC::Structures::VehicleSeatFlags::VEHICLE_SEAT_FLAG_CAN_CONTROL)
        getBase()->setCharmedByGuid(0);
        //getBase()->RemoveCharmedBy(unit);

    if (getBase()->IsInWorld())
    {
        if (!getBase()->GetTransport())
        {
            unit->removeUnitMovementFlag(MOVEFLAG_TRANSPORT);
            unit->obj_movement_info.clearTransportData();
        }
        else
        {
            unit->obj_movement_info.transport_guid = getBase()->obj_movement_info.transport_guid;
            unit->obj_movement_info.transport_position = getBase()->obj_movement_info.transport_position;
            unit->obj_movement_info.transport_seat = getBase()->obj_movement_info.transport_seat;
            unit->obj_movement_info.transport_time = getBase()->obj_movement_info.transport_time;
        }
    }

    // only for flyable vehicles
    if (unit->IsFlying())
        getBase()->castSpell(unit, VEHICLE_SPELL_PARACHUTE, true);

    //if (getBase()->getObjectTypeId() == TYPEID_UNIT && _me->ToCreature()->IsAIEnabled())
    //    getBase()->ToCreature()->GetScript()->PassengerBoarded(unit, seat->first, false);

    if (getBase()->getObjectTypeId() == TYPEID_UNIT)
    {
        // onRemovePassenger Event
    }

    unit->setVehicle(nullptr);
    return this;
}

void Vehicle::relocatePassengers()
{
    std::vector<std::pair<Unit*, LocationVector>> seatRelocation;
    seatRelocation.reserve(Seats.size());

    // not sure that absolute position calculation is correct, it must depend on vehicle pitch angle
    for (SeatMap::const_iterator itr = Seats.begin(); itr != Seats.end(); ++itr)
    {
        if (Unit* passenger = getBase()->GetMapMgrUnit(itr->second._passenger.guid))
        {
            ASSERT(passenger->IsInWorld());

            float px, py, pz, po;
            passenger->obj_movement_info.transport_position.getPosition(px, py, pz, po);
            CalculatePassengerPosition(px, py, pz, &po);
            seatRelocation.emplace_back(passenger, LocationVector(px, py, pz, po));
        }
    }

    for (auto const& pair : seatRelocation)
        pair.first->SetPosition(pair.second);
}

bool Vehicle::isVehicleInUse() const
{
    for (SeatMap::const_iterator itr = Seats.begin(); itr != Seats.end(); ++itr)
        if (!itr->second.isEmpty())
            return true;

    return false;
}

bool Vehicle::isControllableVehicle() const
{
    for (SeatMap::const_iterator itr = Seats.begin(); itr != Seats.end(); ++itr)
        return (itr->second._seatInfo->IsController());

    return false;
}

DBC::Structures::VehicleSeatEntry const* Vehicle::getSeatForPassenger(Unit const* passenger) const
{
    for (SeatMap::const_iterator itr = Seats.begin(); itr != Seats.end(); ++itr)
        if (itr->second._passenger.guid == passenger->getGuid())
            return itr->second._seatInfo;

    return nullptr;
}

SeatMap::iterator Vehicle::getSeatIteratorForPassenger(Unit* passenger)
{
    SeatMap::iterator itr;
    for (itr = Seats.begin(); itr != Seats.end(); ++itr)
        if (itr->second._passenger.guid == passenger->getGuid())
            return itr;

    return Seats.end();
}

uint8_t Vehicle::getAvailableSeatCount() const
{
    uint8 ret = 0;
    SeatMap::const_iterator itr;
    for (itr = Seats.begin(); itr != Seats.end(); ++itr)
        if (itr->second.isEmpty() && (itr->second._seatInfo->canEnterOrExit() || itr->second._seatInfo->isUsableByOverride()))
            ++ret;

    return ret;
}

bool Vehicle::tryAddPassenger(Unit* passenger, SeatMap::iterator Seat)
{
    if (!passenger->IsInWorld() || !getBase()->IsInWorld())
        return false;

    // Passenger might've died in the meantime - abort if this is the case
    if (!passenger->isAlive())
        return false;

    if (passenger->getVehicle())
        passenger->exitVehicle();

    passenger->setVehicle(this);
    Seat->second._passenger.guid = passenger->getGuid();
    Seat->second._passenger.isUnselectable = passenger->hasUnitFlags(UNIT_FLAG_NOT_SELECTABLE);

    if (Seat->second._seatInfo->canEnterOrExit())
    {
        if (usableSeatNum > 0)
            --(usableSeatNum);

        if (!usableSeatNum)
        {
            if (getBase()->getObjectTypeId() == TYPEID_PLAYER)
                getBase()->removeUnitFlags(UNIT_NPC_FLAG_PLAYER_VEHICLE);
            else
                getBase()->removeUnitFlags( UNIT_NPC_FLAG_SPELLCLICK);
        }
    }

    passenger->removeAllAurasByAuraEffect(SPELL_AURA_MOUNTED);

    DBC::Structures::VehicleSeatEntry const* veSeat = Seat->second._seatInfo;
    VehicleSeatAddon const* veSeatAddon = Seat->second._seatAddon;

    Player* player = passenger->ToPlayer();
    if (player)
    {
        WorldPacket data(SMSG_ON_CANCEL_EXPECTED_RIDE_VEHICLE_AURA, 0);
        player->SendPacket(&data);

        if (!veSeat->hasFlag(DBC::Structures::VehicleSeatFlagsB::VEHICLE_SEAT_FLAG_B_KEEP_PET))
        {
            // Unsummon Pets
            if (Pet* summon = player->GetSummon())
            {
                summon->Despawn(1000, 0);
            }
        }
    }

    if (veSeat->hasFlag(DBC::Structures::VehicleSeatFlags::VEHICLE_SEAT_FLAG_PASSENGER_NOT_SELECTABLE))
        passenger->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);

    float o = veSeatAddon ? veSeatAddon->SeatOrientationOffset : 0.f;
    float x = veSeat->attachmentOffsetX;
    float y = veSeat->attachmentOffsetY;
    float z = veSeat->attachmentOffsetZ;

    passenger->addUnitMovementFlag(MOVEFLAG_TRANSPORT);
    passenger->obj_movement_info.transport_position.changeCoords(x, y, z, o);
    passenger->obj_movement_info.transport_time = 0;
    passenger->obj_movement_info.transport_seat = Seat->first;
    passenger->obj_movement_info.transport_guid = getBase()->getGuid();

    if (getBase()->getObjectTypeId() == TYPEID_UNIT && passenger->getObjectTypeId() == TYPEID_PLAYER &&
        veSeat->hasFlag(DBC::Structures::VehicleSeatFlags::VEHICLE_SEAT_FLAG_CAN_CONTROL))
    {
        // handles SMSG_CLIENT_CONTROL
        getBase()->setCharmedByGuid(passenger->getGuid());
        /*if (!getBase()->setCharmedBy(passenger->getGuid()))
            return;*/
    }

    // Remove Target and set UNIT_STATE_ROOTED
    passenger->setTargetGuid(0);
    passenger->setControlled(true, UNIT_STATE_ROOTED);
    
    // Send movement Spline
    MovementNew::MoveSplineInit init(passenger);
    init.DisableTransportPathTransformations();
    init.MoveTo(x, y, z, false, true);
    init.SetFacing(o);
    init.SetTransportEnter();
    passenger->getMovementManager()->launchMoveSpline(std::move(init), EVENT_VEHICLE_BOARD, MOTION_PRIORITY_HIGHEST);

    // Add threat from my Targets
    for (auto const& [guid, threatRef] : passenger->getThreatManager().getThreatenedByMeList())
        threatRef->getOwner()->getThreatManager().addThreat(getBase(), threatRef->getThreat(), nullptr, true, true);

    // Script Hooks
    if (Creature* creature = getBase()->ToCreature())
    {
        /*if (CreatureAIScript* ai = creature->GetScript())
            ai->PassengerBoarded(Passenger, Seat->first, true);

        sScriptMgr->OnAddPassenger(Target, Passenger, Seat->first);

        // Actually quite a redundant hook. Could just use OnAddPassenger and check for unit typemask inside script.
        if (passenger->hasUnitStateFlag(UNIT_STATE_ACCESSORY))
            sScriptMgr->OnInstallAccessory(Target, Passenger->ToCreature());*/
    }

    return true;
}
