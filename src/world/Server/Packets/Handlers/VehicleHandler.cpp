/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "Server/Packets/CmsgRequestVehicleSwitchSeat.h"
#include "Server/Packets/CmsgChangeSeatsOnControlledVehicle.h"
#include "Server/Packets/CmsgPlayerVehicleEnter.h"
#include "Server/Packets/CmsgEjectPassenger.h"
#include "Server/WorldSession.h"
#include "Objects/Units/Players/Player.h"
#include "Map/MapMgr.h"
#include "Objects/Units/Creatures/Vehicle.h"

using namespace AscEmu::Packets;

#if VERSION_STRING > TBC
void WorldSession::handleDismissVehicle(WorldPacket& recvPacket)
{
    uint64_t vehicleGUID = _player->getCharmGuid();

    if (!vehicleGUID)   // something wrong here...
    {
        recvPacket.rfinish();   // prevent warnings spam
        return;
    }

    _player->obj_movement_info.readMovementInfo(recvPacket, CMSG_DISMISS_CONTROLLED_VEHICLE);
    _player->callExitVehicle();
}

void WorldSession::handleRequestVehiclePreviousSeat(WorldPacket& recvPacket)
{
    if (GetPlayer()->getVehicleBase() == nullptr)
    {
        recvPacket.rfinish();
        return;
    }

    auto seat = GetPlayer()->getVehicle()->getSeatForPassenger(GetPlayer());
    if (!seat->canSwitchFromSeat())
    {
        recvPacket.rfinish();
        return;
    }

    GetPlayer()->callChangeSeat(-1, false);
}

void WorldSession::handleRequestVehicleNextSeat(WorldPacket& recvPacket)
{
    if (GetPlayer()->getVehicleBase() == nullptr)
    {
        recvPacket.rfinish();
        return;
    }

    auto seat = GetPlayer()->getVehicle()->getSeatForPassenger(GetPlayer());
    if (!seat->canSwitchFromSeat())
    {
        recvPacket.rfinish();
        return;
    }

    GetPlayer()->callChangeSeat(-1, true);
}

void WorldSession::handleRequestVehicleSwitchSeat(WorldPacket& recvPacket)
{
    Unit* vehicle_base = GetPlayer()->getVehicleBase();
    if (!vehicle_base)
        return;

    if (auto seat = GetPlayer()->getVehicle()->getSeatForPassenger(GetPlayer()))
        if (!seat->canSwitchFromSeat())
            return;

    WoWGuid guid;
    int8_t seatId;

    recvPacket >> guid;
    recvPacket >> seatId;

    if (vehicle_base->getGuid() == guid.getRawGuid())
    {
        GetPlayer()->callChangeSeat(seatId);
    }
    else if (Unit* vehUnit = GetPlayer()->GetMapMgr()->GetUnit(guid.getRawGuid()))
    {
        if (Vehicle* vehicle = vehUnit->getVehicleKit())
        {
            if (vehicle->hasEmptySeat(seatId))
            {
                vehUnit->handleSpellClick(GetPlayer(), seatId);
            }
        }
    }
}

void WorldSession::handleChangeSeatsOnControlledVehicle([[maybe_unused]]WorldPacket& recvPacket)
{
    Unit* vehicle_base = GetPlayer()->getVehicleBase();
    if (!vehicle_base)
        return;

    auto seat = GetPlayer()->getVehicle()->getSeatForPassenger(GetPlayer());
    if (!seat->canSwitchFromSeat())
        return;

#if VERSION_STRING < Cata
    CmsgChangeSeatsOnControlledVehicle srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    uint64_t guid = srlPacket.sourceGuid;               // current vehicle guid
    uint64_t accessory = srlPacket.destinationGuid;     //  accessory guid

    vehicle_base->obj_movement_info = srlPacket.movementInfo;
    int8_t seatId = srlPacket.seat;

    if (vehicle_base->getGuid() != guid)
        return;

    if (!accessory)
    {
        GetPlayer()->callChangeSeat(-1, seatId > 0); // prev/next
    }
    else if (Unit* vehUnit = GetPlayer()->GetMapMgrUnit(accessory))
    {
        if (Vehicle* vehicle = vehUnit->getVehicleKit())
            if (vehicle->hasEmptySeat(seatId))
                vehUnit->handleSpellClick(GetPlayer(), seatId);
    }
    else
    {
        if (vehicle_base->getVehicle())
            if (vehicle_base->getVehicle()->hasEmptySeat(seatId))
                vehicle_base->getVehicleBase()->handleSpellClick(GetPlayer(), seatId);
    }

#else
    MovementInfo movementInfo; 
    movementInfo.readMovementInfo(recvPacket, CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE);
    vehicle_base->obj_movement_info = movementInfo;

    ObjectGuid guid;
    int8_t seatId;
    recvPacket >> seatId;

    guid[2] = recvPacket.readBit();
    guid[4] = recvPacket.readBit();
    guid[7] = recvPacket.readBit();
    guid[6] = recvPacket.readBit();
    guid[5] = recvPacket.readBit();
    guid[0] = recvPacket.readBit();
    guid[1] = recvPacket.readBit();
    guid[3] = recvPacket.readBit();
    
    recvPacket.ReadByteSeq(guid[6]);
    recvPacket.ReadByteSeq(guid[1]);
    recvPacket.ReadByteSeq(guid[2]);
    recvPacket.ReadByteSeq(guid[5]);
    recvPacket.ReadByteSeq(guid[3]);
    recvPacket.ReadByteSeq(guid[0]);
    recvPacket.ReadByteSeq(guid[4]);
    recvPacket.ReadByteSeq(guid[7]);

    if (vehicle_base->getGuid() != movementInfo.guid)
        return;

    if (!guid)
        GetPlayer()->callChangeSeat(-1, seatId > 0); // prev/next
    else if (Unit* vehUnit = GetPlayer()->GetMapMgrUnit(guid))
    {
        if (Vehicle* vehicle = vehUnit->getVehicleKit())
            if (vehicle->hasEmptySeat(seatId))
                vehUnit->handleSpellClick(GetPlayer(), seatId);
    }
#endif
}

void WorldSession::handleRemoveVehiclePassenger(WorldPacket& recvPacket)
{
    Vehicle* vehicle = _player->getVehicleKit();
    if (!vehicle)
    {
        recvPacket.rfinish();   // prevent warnings spam
        return;
    }

    CmsgEjectPassenger srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.guid == 0)
        return;

    const auto passengerUnit = _player->GetMapMgr()->GetUnit(srlPacket.guid);
    if(!passengerUnit)
        return;

    if (!passengerUnit->isOnVehicle(vehicle->getBase()))
        return;

    auto seat = vehicle->getSeatForPassenger(passengerUnit);
    if(seat)
        if (seat->isEjectable())
            passengerUnit->callExitVehicle();
}

void WorldSession::handleLeaveVehicle(WorldPacket& /*recvPacket*/)
{
    if (Vehicle* vehicle = GetPlayer()->getVehicle())
    {
        if (DBC::Structures::VehicleSeatEntry const* seat = vehicle->getSeatForPassenger(GetPlayer()))
        {
            if (seat->canEnterOrExit())
                GetPlayer()->callExitVehicle();
        }
    }
}

void WorldSession::handleEnterVehicle(WorldPacket& recvPacket)
{
    CmsgPlayerVehicleEnter srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto unit = _player->GetMapMgr()->GetUnit(srlPacket.guid);
    if (unit == nullptr)
        return;

    if (!_player->isInRange(unit, MAX_INTERACTION_RANGE))
        return;

    if (unit->getVehicleKit() == nullptr)
        return;

    _player->callEnterVehicle(unit);
}
#endif
