/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
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


    // Set Correct Power Type
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

    // Disable Power Regen by Default
    // when there will be exceptions add them here
    switch (getEntry())
    {
        default:
            getBase()->addNpcFlags(UNIT_NPC_FLAG_DISABLE_PWREGEN);
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
    if (getBase()->isCreature())
    {
        if (CreatureAIScript* ai = getBase()->ToCreature()->GetScript())
        {
            ai->OnInstall();
        }
    }
}

void Vehicle::uninstall()
{
    if (_status == STATUS_UNINSTALLING && !getBase()->hasUnitStateFlag(UNIT_STATE_ACCESSORY))
    {
        sLogger.failure("Vehicle %s attempts to uninstall, but already has STATUS_UNINSTALLING! ", getBase()->getGuid());
        return;
    }

    _status = STATUS_UNINSTALLING;
    removeAllPassengers();

    if (getBase()->isCreature())
    {
        if (CreatureAIScript* ai = getBase()->ToCreature()->GetScript())
        {
            ai->OnUninstall();
        }
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

    // Delay for a bit so Accessory has time to get Pushed
    sEventMgr.AddEvent(getBase()->ToUnit(), &Unit::handleSpellClick, accessory->ToUnit(), seatId, 0, 50, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Vehicle::applyAllImmunities()
{
    // Vehicles should be immune on Knockback ...
    getBase()->addSpellImmunity(SPELL_IMMUNITY_KNOCKBACK, true);

    // Mechanical units & vehicles ( which are not Bosses, they SHOULD have own immunities in DATABASE ) should be also immune on healing ( exceptions in switch below )
    if (getBase()->ToCreature() && getBase()->ToCreature()->GetCreatureProperties()->Type == UNIT_TYPE_MECHANICAL && !getBase()->ToCreature()->GetCreatureProperties()->Rank == ELITE_WORLDBOSS)
    {
        // Heal & dispel ...
        /*
        *   Not Supported atm
        getBase()->addSpellImmunity(SPELL_EFFECT_HEAL, true);
        getBase()->addSpellImmunity(SPELL_EFFECT_HEAL_PCT, true);
        getBase()->addSpellImmunity(SPELL_EFFECT_DISPEL, true);
        getBase()->addSpellImmunity(SPELL_AURA_PERIODIC_HEAL, true);
        */

        // ... Shield & Immunity grant spells ...
        /*
        *   Not Supported atm
        getBase()->addSpellImmunity(SPELL_AURA_SCHOOL_IMMUNITY, true);
        getBase()->addSpellImmunity(SPELL_AURA_MOD_UNATTACKABLE, true);
        getBase()->addSpellImmunity(SPELL_AURA_SCHOOL_ABSORB, true);
        */
        getBase()->addSpellImmunity(SPELL_IMMUNITY_BANISH, true);

        // ... Resistance, Split damage, Change stats ...
        /*
        *   Not Supported atm
        getBase()->addSpellImmunity(SPELL_AURA_DAMAGE_SHIELD, true);
        getBase()->addSpellImmunity(SPELL_AURA_SPLIT_DAMAGE_PCT, true);
        getBase()->addSpellImmunity(SPELL_AURA_MOD_RESISTANCE, true);
        getBase()->addSpellImmunity(SPELL_AURA_MOD_STAT, true);
        getBase()->addSpellImmunity(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, true);
        */
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
            getBase()->addSpellImmunity(SPELL_IMMUNITY_SLOW, true);
            break;
        case 335: // Salvaged Chopper
        case 336: // Salvaged Siege Engine
        case 338: // Salvaged Demolisher
            /*
            *   Not Supported atm
            getBase()->addSpellImmunity(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, false); // Battering Ram
            */
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

bool Vehicle::hasEmptySeat() const
{
    for (auto& const seat : Seats)
    {
        if (seat.second.isEmpty())
            return true;
    }

    return false;
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

        // Make sure we don't loop infinitly
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

        return tryAddPassenger(unit, seat);
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
                passenger->callExitVehicle();
        }

        return tryAddPassenger(unit, seat);
    }

    return false;
}

Vehicle* Vehicle::removePassenger(Unit* unit)
{
    if (unit->getVehicle() != this)
        return nullptr;

    SeatMap::iterator seat = getSeatIteratorForPassenger(unit);
    ASSERT(seat != Seats.end());

    if (seat->second._seatInfo->canEnterOrExit() && ++usableSeatNum)
        getBase()->setNpcFlags((getBase()->getObjectTypeId() == TYPEID_PLAYER ? UNIT_NPC_FLAG_PLAYER_VEHICLE : UNIT_NPC_FLAG_SPELLCLICK));

    if (seat->second._seatInfo->flags & DBC::Structures::VehicleSeatFlags::VEHICLE_SEAT_FLAG_PASSENGER_NOT_SELECTABLE && !seat->second._passenger.isUnselectable)
        unit->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);

    seat->second._passenger.reset();

    if (getBase()->getObjectTypeId() == TYPEID_UNIT && unit->getObjectTypeId() == TYPEID_PLAYER && seat->second._seatInfo->flags & DBC::Structures::VehicleSeatFlags::VEHICLE_SEAT_FLAG_CAN_CONTROL)
    {
        unit->setCharmGuid(0);
        getBase()->setCharmedByGuid(0);

        static_cast<Player*>(unit)->setFarsightGuid(0);
        getBase()->removeUnitFlags(UNIT_FLAG_PLAYER_CONTROLLED_CREATURE | UNIT_FLAG_PVP_ATTACKABLE);

        unit->mControledUnit = unit;
        static_cast<Player*>(unit)->sendClientControlPacket(getBase(), 0);

        // send null spells if needed
        static_cast<Player*>(unit)->SendEmptyPetSpellList();
        static_cast<Player*>(unit)->SetMover(unit);

        // set old Faction
        if (getBase()->isCreature())
        {
            Creature* c = static_cast<Creature*>(getBase());
            c->setFaction(sMySQLStore.getCreatureProperties(getEntry())->Faction);
        }
    }

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

    // Script Hooks
    if (getBase()->isCreature())
    {
        if (CreatureAIScript* ai = getBase()->ToCreature()->GetScript())
        {
            ai->OnRemovePassenger(unit);
        }
    }

    unit->setVehicle(nullptr);
    return this;
}

void Vehicle::movePassengers(float x, float y, float z, float o)
{
    for (SeatMap::const_iterator itr = Seats.begin(); itr != Seats.end(); ++itr)
    {
        if (Unit* passenger = getBase()->GetMapMgrUnit(itr->second._passenger.guid))
        {
            if (passenger == nullptr)
                continue;

            passenger->SetPosition(x, y, z, o);
        }
    }
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
            if (passenger && passenger->IsInWorld())
            {
                float px, py, pz, po;
                passenger->obj_movement_info.transport_position.getPosition(px, py, pz, po);
                CalculatePassengerPosition(px, py, pz, &po);
                seatRelocation.emplace_back(passenger, LocationVector(px, py, pz, po));
            }
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

int8_t Vehicle::getSeatForNumberPassenger(Unit const* passenger) const
{
    for (SeatMap::const_iterator itr = Seats.begin(); itr != Seats.end(); ++itr)
        if (itr->second._passenger.guid == passenger->getGuid())
            return itr->first;

    return -1;
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
    uint8_t ret = 0;
    SeatMap::const_iterator itr;
    for (itr = Seats.begin(); itr != Seats.end(); ++itr)
        if (itr->second.isEmpty() && (itr->second._seatInfo->canEnterOrExit() || itr->second._seatInfo->isUsableByOverride()))
            ++ret;

    return ret;
}

bool Vehicle::tryAddPassenger(Unit* passenger, SeatMap::iterator &Seat)
{
    if (!passenger->IsInWorld() || !getBase()->IsInWorld())
        return false;

    // Passenger might've died in the meantime
    if (!passenger->isAlive())
        return false;

    if (passenger->getVehicle())
        passenger->callExitVehicle();

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
            player->DismissActivePets();
        }
    }

    if (veSeat->hasFlag(DBC::Structures::VehicleSeatFlags::VEHICLE_SEAT_FLAG_PASSENGER_NOT_SELECTABLE))
        passenger->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);

    passenger->SendPacket(AscEmu::Packets::SmsgControlVehicle().serialise().get());

    float o = veSeatAddon ? veSeatAddon->SeatOrientationOffset : 0.f;
    float x = veSeat->attachmentOffsetX;
    float y = veSeat->attachmentOffsetY;
    float z = veSeat->attachmentOffsetZ;

    passenger->addUnitMovementFlag(MOVEFLAG_TRANSPORT);
    passenger->obj_movement_info.transport_position.changeCoords(x, y, z, o);
    passenger->obj_movement_info.transport_time = 0;
    passenger->obj_movement_info.transport_seat = Seat->first;
    passenger->obj_movement_info.transport_guid = getBase()->getGuid();

    if (passenger->isPlayer())
    {
        passenger->addUnitFlags(UNIT_FLAG_PVP_ATTACKABLE);
    }

    // handles SMSG_CLIENT_CONTROL
    if (getBase()->getObjectTypeId() == TYPEID_UNIT && passenger->getObjectTypeId() == TYPEID_PLAYER &&
        veSeat->hasFlag(DBC::Structures::VehicleSeatFlags::VEHICLE_SEAT_FLAG_CAN_CONTROL))
    {
        passenger->SendPacket(AscEmu::Packets::SmsgControlVehicle().serialise().get());
        static_cast<Player*>(passenger)->setFarsightGuid(getBase()->getGuid());
        static_cast<Player*>(passenger)->sendClientControlPacket(getBase(), 1);

        passenger->setCharmGuid(getBase()->getGuid());
        getBase()->setCharmedByGuid(passenger->getGuid());
        getBase()->addUnitFlags(UNIT_FLAG_PLAYER_CONTROLLED_CREATURE);
        passenger->mControledUnit = getBase();

        WorldPacket spells(SMSG_PET_SPELLS, 100);
        getBase()->BuildPetSpellList(spells);
        passenger->SendPacket(&spells);

        static_cast<Player*>(passenger)->SetMover(getBase());

        // set Correct Faction
        if (getBase()->isCreature())
        {
            Creature* c = static_cast<Creature*>(getBase());
            c->setFaction(passenger->getFactionTemplate());
        }
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
    if (getBase()->isCreature())
    {
        if (CreatureAIScript* ai = getBase()->ToCreature()->GetScript())
        {
            ai->OnAddPassenger(passenger, Seat->first);

            if (passenger->hasUnitStateFlag(UNIT_STATE_ACCESSORY))
                ai->OnInstallAccessory(passenger->ToCreature());
        }
    }

    return true;
}
