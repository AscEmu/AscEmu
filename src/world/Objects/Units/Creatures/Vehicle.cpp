/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Vehicle.hpp"

#include "Logging/Logger.hpp"
#include "Management/ObjectMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Server/Packets/SmsgControlVehicle.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Movement/MovementManager.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Storage/WDB/WDBStructures.hpp"
#include "Storage/WDB/WDBStores.hpp"

#ifdef FT_VEHICLES

Vehicle::Vehicle(Unit* unit, WDB::Structures::VehicleEntry const* vehInfo, uint32_t creatureEntry) :
    usableSeatNum(0), _owner(unit), _vehicleInfo(vehInfo), _creatureEntry(creatureEntry), _status(STATUS_NONE), _lastShootPos()
{
    initialize();
}

Vehicle::~Vehicle()
{
    if (_status == STATUS_DEACTIVATED)
    {
        for (SeatMap::const_iterator itr = Seats.begin(); itr != Seats.end(); ++itr)
            if (!itr->second.isEmpty())
                sLogger.failure("Vehicle is not Empty");
    }
    else
    {
        sLogger.failure("Vehicle Accessory Status is not on STATUS_DEACTIVATED");
    }
}

void Vehicle::initialize()
{
    // Initialize our Vehicle
    initSeats();
    initVehiclePowerTypes();
    initMovementFlags();
    applyAllImmunities();
    
    // Script Hooks
    if (getBase()->IsInWorld() && getBase()->isCreature() && static_cast<Creature*>(getBase())->GetScript() != nullptr)
        static_cast<Creature*>(getBase())->GetScript()->OnVehicleInitialize();

    _status = STATUS_INITALIZED;
}

void Vehicle::deactivate()
{
    if (_status == STATUS_DEACTIVATED && !getBase()->hasUnitStateFlag(UNIT_STATE_ACCESSORY))
    {
        sLogger.failure("Vehicle {} attempts to deactivate, but already has STATUS_DEACTIVATED! ", std::to_string(getBase()->getGuid()));
        return;
    }

    _status = STATUS_DEACTIVATED;
    removeAllPassengers();

    // Script Hooks
    if (getBase()->IsInWorld() && getBase()->isCreature() && static_cast<Creature*>(getBase())->GetScript())
        static_cast<Creature*>(getBase())->GetScript()->OnVehicleDeactivate();
}

void Vehicle::initSeats()
{
    for (uint8_t i = 0; i < MAX_VEHICLE_SEATS; ++i)
    {
        if (uint32_t seatId = _vehicleInfo->seatID[i])
            if (auto veSeat = sVehicleSeatStore.lookupEntry(seatId))
            {
                VehicleSeatAddon const* addon = sObjectMgr.getVehicleSeatAddon(seatId);
                Seats.insert(std::make_pair(i, VehicleSeat(veSeat, addon)));
                if (veSeat->canEnterOrExit())
                    ++usableSeatNum;
            }
    }

    // Set correct Flags to make the Vehicle clickable dependant on if its a Player or a Creature
    // to prevent any mistakes overwrited Database Data
    if (usableSeatNum)
        getBase()->setNpcFlags((getBase()->isPlayer() ? UNIT_NPC_FLAG_PLAYER_VEHICLE : UNIT_NPC_FLAG_SPELLCLICK));
    else
        getBase()->removeNpcFlags((getBase()->isPlayer() ? UNIT_NPC_FLAG_PLAYER_VEHICLE : UNIT_NPC_FLAG_SPELLCLICK));
}

void Vehicle::initMovementFlags()
{
    if (hasVehicleFlags(VEHICLE_FLAG_NO_STRAFING))
        getBase()->addExtraUnitMovementFlag(MOVEFLAG2_NO_STRAFING);
    if (hasVehicleFlags(VEHICLE_FLAG_NO_JUMPING))
        getBase()->addExtraUnitMovementFlag(MOVEFLAG2_NO_JUMPING);
    if (hasVehicleFlags(VEHICLE_FLAG_FULLSPEED_TURNING))
        getBase()->addExtraUnitMovementFlag(MOVEFLAG2_FULLSPEED_TURNING);
    if (hasVehicleFlags(VEHICLE_FLAG_ALLOW_PITCHING))
        getBase()->addExtraUnitMovementFlag(MOVEFLAG2_ALLOW_PITCHING);
    if (hasVehicleFlags(VEHICLE_FLAG_FULLSPEED_PITCHING))
        getBase()->addExtraUnitMovementFlag(MOVEFLAG2_FULLSPEED_PITCHING);
}

void Vehicle::initVehiclePowerTypes()
{
    // Set Correct Power Type
    switch (_vehicleInfo->powerType)
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
        default:
            break;
    }

    // Disable Power Regen by Default
    // when there will be exceptions add them here
#ifdef ENABLE_WHEN_POWER_REGEN_NEEDS_IT
    switch (getEntry())
    {
        default:
#endif
            getBase()->addNpcFlags(UNIT_NPC_FLAG_DISABLE_PWREGEN);
#ifdef ENABLE_WHEN_POWER_REGEN_NEEDS_IT
            break;
    }
#endif
}

void Vehicle::applyAllImmunities()
{
    // Vehicles should be immune to Knockback effects
    getBase()->addSpellImmunity(SPELL_IMMUNITY_KNOCKBACK, true);

    // Mechanical units & vehicles (which are not Bosses) 
    // should also be immune on healing ( exceptions in switch below )
    if (getBase()->ToCreature() && getBase()->ToCreature()->GetCreatureProperties()->Type == UNIT_TYPE_MECHANICAL && getBase()->ToCreature()->GetCreatureProperties()->Rank != ELITE_WORLDBOSS)
    {
        //  Heal & dispel ...
        //  ToDo missing

        //  ... Shield & Immunity grant spells ...
        //  ToDo add more
        getBase()->addSpellImmunity(SPELL_IMMUNITY_BANISH, true);

        //  ... Resistance, Split damage, Change stats
        //  ToDo missing
    }

    // When Flag VEHICLE_FLAG_POSITION_FIXED is set or one of the followed Hardcoded units is set then set them to rooted
    if (hasVehicleFlags(VEHICLE_FLAG_POSITION_FIXED))
        getBase()->setControlled(true, UNIT_STATE_ROOTED);

    switch (getBase()->getEntry())
    {
        case 30236: //  | Argent Cannon
        case 39759: //  | Tankbuster Cannon
            getBase()->setControlled(true, UNIT_STATE_ROOTED);
            break;
        default:
            break;
    }

    // More Hardcoded Immunities
    switch (getVehicleInfo()->ID)
    {
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
            //  Not Supported at the moment for
            //  SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN
            break;
        default:
            break;
    }
}

void Vehicle::loadAllAccessories(bool evading)
{
    if (getBase()->getObjectTypeId() == TYPEID_PLAYER || !evading)
        removeAllPassengers();

    VehicleAccessoryList const* accessories = sObjectMgr.getVehicleAccessories(_creatureEntry);
    if (!accessories)
        return;

    for (VehicleAccessoryList::const_iterator itr = accessories->begin(); itr != accessories->end(); ++itr)
        if (!evading || itr->isMinion)  // only install accessories on evade mode
            loadAccessory(itr->accessoryEntry, itr->seatId, itr->isMinion, itr->summonedType, itr->summonTime);
}

void Vehicle::loadAccessory(uint32_t entry, int8_t seatId, bool minion, uint8_t /*type*/, uint32_t /*summonTime*/)
{
    if (_status == STATUS_DEACTIVATED)
    {
        sLogger.failure("Vehicle ({}, Entry: {}) attempts to load accessory (Entry: {}) on seat {} with STATUS_DEACTIVATED! ", getBase()->getGuid(), getEntry(), entry, (int32_t)seatId);
        return;
    }

    CreatureProperties const* cp = sMySQLStore.getCreatureProperties(entry);
    if (cp == nullptr)
        return;

    Creature* accessory = getBase()->getWorldMap()->createCreature(entry);
    accessory->Load(cp, getBase()->GetPositionX(), getBase()->GetPositionY(), getBase()->GetPositionZ(), getBase()->GetOrientation());
    accessory->setPhase(PHASE_SET, getBase()->GetPhase());
    accessory->setFaction(getBase()->getFactionTemplate());
    accessory->PushToWorld(getBase()->getWorldMap());

#if VERSION_STRING <= WotLK
    accessory->obj_movement_info.addMovementFlag(MOVEFLAG_TRANSPORT);
    accessory->addUnitMovementFlag(MOVEFLAG_TRANSPORT);
#endif

    if (minion)
        accessory->addUnitStateFlag(UNIT_STATE_ACCESSORY);

    // Delay for a bit so Accessory has time to get Pushed to World
    sEventMgr.AddEvent(getBase()->ToUnit(), &Unit::handleSpellClick, accessory->ToUnit(), seatId, 0, 50, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

Unit* Vehicle::getBase() const { return _owner; }
WDB::Structures::VehicleEntry const* Vehicle::getVehicleInfo() const { return _vehicleInfo; }
uint32_t Vehicle::getEntry() const { return _creatureEntry; }

void Vehicle::removeAllPassengers()
{
    // Passengers always cast an aura with SPELL_AURA_CONTROL_VEHICLE on the vehicle
    // Lets remove the Aura
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
    for (const auto& seat : Seats)
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

    return getBase()->getWorldMapUnit(seat->second._passenger.guid);
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

        // don't loop infinitly
        if (seat->first == seatId)
            return Seats.end();
    }

    return seat;
}

bool Vehicle::isControler(Unit* _unit)
{
    for (const auto& seats : Seats)
        if (seats.second._passenger.guid == _unit->getGuid())
            if (seats.second._seatInfo->IsController())
                return true;

    return false;
}

void Vehicle::setLastShootPos(LocationVector const& pos) { _lastShootPos.ChangeCoords(pos); }
LocationVector const& Vehicle::getLastShootPos() const { return _lastShootPos; }

VehicleSeatAddon const* Vehicle::getSeatAddonForSeatOfPassenger(Unit const* passenger) const
{
    for (SeatMap::const_iterator itr = Seats.begin(); itr != Seats.end(); ++itr)
        if (!itr->second.isEmpty() && itr->second._passenger.guid == passenger->getGuid())
            return itr->second._seatAddon;

    return nullptr;
}

bool Vehicle::addPassenger(Unit* unit, int8_t seatId)
{
    if (_status == STATUS_DEACTIVATED)
    {
        sLogger.failure("Passenger {}, attempting to board vehicle {} during deactivating! SeatId: {}", unit->getGuid(), getBase()->getGuidHigh(), (int32_t)seatId);
        return false;
    }

    SeatMap::iterator seat;
    if (seatId < 0)
    {
        for (seat = Seats.begin(); seat != Seats.end(); ++seat)
            if (seat->second.isEmpty() && (seat->second._seatInfo->canEnterOrExit() || seat->second._seatInfo->isUsableByOverride()))
                break;

        // no seat available
        if (seat == Seats.end())
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
            Unit* passenger = getBase()->getWorldMapUnit(seat->second._passenger.guid);
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

    if (seat->second._seatInfo->flags & WDB::Structures::VehicleSeatFlags::VEHICLE_SEAT_FLAG_PASSENGER_NOT_SELECTABLE && !seat->second._passenger.isUnselectable)
        unit->removeUnitFlags(UNIT_FLAG_NOT_SELECTABLE);

    seat->second._passenger.reset();

    if (getBase()->getObjectTypeId() == TYPEID_UNIT && unit->getObjectTypeId() == TYPEID_PLAYER && seat->second._seatInfo->flags & WDB::Structures::VehicleSeatFlags::VEHICLE_SEAT_FLAG_CAN_CONTROL)
    {
        unit->setCharmGuid(0);
        getBase()->setCharmedByGuid(0);

        static_cast<Player*>(unit)->setFarsightGuid(0);
        getBase()->removeUnitFlags(UNIT_FLAG_PLAYER_CONTROLLED_CREATURE | UNIT_FLAG_PVP_ATTACKABLE);

        static_cast<Player*>(unit)->sendClientControlPacket(getBase(), 0);

        // send null spells if needed
        static_cast<Player*>(unit)->sendEmptyPetSpellList();
        static_cast<Player*>(unit)->setMover(unit);

        // set old Faction
        if (getBase()->isCreature())
        {
            Creature* c = static_cast<Creature*>(getBase());
            if (getBase()->getVehicleBase())
                c->setFaction(sMySQLStore.getCreatureProperties(getBase()->getVehicleBase()->getEntry())->Faction);
            else
                c->setFaction(sMySQLStore.getCreatureProperties(getEntry())->Faction);
        }
    }

    if (getBase()->IsInWorld())
    {
        if (!getBase()->GetTransport())
        {
#if VERSION_STRING <= WotLK
            unit->removeUnitMovementFlag(MOVEFLAG_TRANSPORT);
#endif
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

    // give us a Parachute when we leave a flyable Vehicle
    if (unit->IsFlying())
        getBase()->castSpell(unit, VEHICLE_SPELL_PARACHUTE, true);

    // Script Hooks
    if (getBase()->IsInWorld() && getBase()->isCreature() && static_cast<Creature*>(getBase())->GetScript())
        static_cast<Creature*>(getBase())->GetScript()->OnRemovePassenger(unit);

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
        if (Unit* passenger = getBase()->getWorldMapUnit(itr->second._passenger.guid))
        {
            if (passenger && passenger->IsInWorld())
            {
                float px, py, pz, po;
                passenger->obj_movement_info.transport_position.getPosition(px, py, pz, po);
                calculatePassengerPosition(px, py, pz, &po);
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
        if (itr->second._seatInfo->IsController())
            return true;

    return false;
}

WDB::Structures::VehicleSeatEntry const* Vehicle::getSeatForPassenger(Unit const* passenger) const
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

bool Vehicle::hasVehicleFlags(uint32_t flags) { return getVehicleInfo()->flags & flags; }

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

    // we cannot mount as a corpse
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

    WDB::Structures::VehicleSeatEntry const* veSeat = Seat->second._seatInfo;
    VehicleSeatAddon const* veSeatAddon = Seat->second._seatAddon;

    Player* player = passenger->ToPlayer();
    if (player)
    {
        WorldPacket data(SMSG_ON_CANCEL_EXPECTED_RIDE_VEHICLE_AURA, 0);
        player->sendPacket(&data);

        if (!veSeat->hasFlag(WDB::Structures::VehicleSeatFlagsB::VEHICLE_SEAT_FLAG_B_KEEP_PET))
        {
            // Unsummon Pets
            player->unSummonPetTemporarily();
        }
    }

    if (veSeat->hasFlag(WDB::Structures::VehicleSeatFlags::VEHICLE_SEAT_FLAG_PASSENGER_NOT_SELECTABLE))
        passenger->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);

    passenger->sendPacket(AscEmu::Packets::SmsgControlVehicle().serialise().get());

    float o = veSeatAddon ? veSeatAddon->seatOrientationOffset : 0.f;
    float x = veSeat->attachmentOffsetX;
    float y = veSeat->attachmentOffsetY;
    float z = veSeat->attachmentOffsetZ;

#if VERSION_STRING <= WotLK
    passenger->addUnitMovementFlag(MOVEFLAG_TRANSPORT);
#endif
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
        veSeat->hasFlag(WDB::Structures::VehicleSeatFlags::VEHICLE_SEAT_FLAG_CAN_CONTROL))
    {
        passenger->sendPacket(AscEmu::Packets::SmsgControlVehicle().serialise().get());
        static_cast<Player*>(passenger)->setFarsightGuid(getBase()->getGuid());
        static_cast<Player*>(passenger)->sendClientControlPacket(getBase(), 1);

        passenger->setCharmGuid(getBase()->getGuid());
        getBase()->setCharmedByGuid(passenger->getGuid());
        getBase()->addUnitFlags(UNIT_FLAG_PLAYER_CONTROLLED_CREATURE);

        WorldPacket spells(SMSG_PET_SPELLS, 100);
        getBase()->buildPetSpellList(spells);
        passenger->sendPacket(&spells);

        static_cast<Player*>(passenger)->setMover(getBase());

        // set Correct Faction
        if (getBase()->isCreature())
        {
            Creature* c = static_cast<Creature*>(getBase());
            c->setFaction(passenger->getFactionTemplate());
        }
    }

    passenger->setTargetGuid(0);
    passenger->setControlled(true, UNIT_STATE_ROOTED);
    
    // Send movement Spline
    MovementMgr::MoveSplineInit init(passenger);
    init.DisableTransportPathTransformations();
    init.MoveTo(x, y, z, false, true);
    init.SetFacing(o);
    init.SetTransportEnter();
    passenger->getMovementManager()->launchMoveSpline(std::move(init), EVENT_VEHICLE_BOARD, MOTION_PRIORITY_HIGHEST);

    // Add threat from my Targets
    for (auto const& [guid, threatRef] : passenger->getThreatManager().getThreatenedByMeList())
        threatRef->getOwner()->getThreatManager().addThreat(getBase(), threatRef->getThreat(), nullptr, true, true);

    // Script Hooks
    if (getBase()->IsInWorld() && getBase()->isCreature() && dynamic_cast<Creature*>(getBase())->GetScript())
        dynamic_cast<Creature*>(getBase())->GetScript()->OnAddPassenger(passenger, Seat->first);

    if (passenger->hasUnitStateFlag(UNIT_STATE_ACCESSORY))
    {
        if (getBase()->IsInWorld() && getBase()->isCreature() && dynamic_cast<Creature*>(getBase())->GetScript())
            dynamic_cast<Creature*>(getBase())->GetScript()->OnInstallAccessory(passenger->ToCreature());
    }

    return true;
}

    void Vehicle::calculatePassengerPosition(float& x, float& y, float& z, float* o)
    {
        TransportBase::CalculatePassengerPosition(x, y, z, o,
            getBase()->GetPositionX(), getBase()->GetPositionY(),
            getBase()->GetPositionZ(), getBase()->GetOrientation());
    }

    void Vehicle::calculatePassengerOffset(float& x, float& y, float& z, float* o)
    {
        TransportBase::CalculatePassengerOffset(x, y, z, o,
            getBase()->GetPositionX(), getBase()->GetPositionY(),
            getBase()->GetPositionZ(), getBase()->GetOrientation());
    }
#endif
