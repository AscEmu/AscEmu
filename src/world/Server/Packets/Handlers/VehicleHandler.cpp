/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Server/Packets/CmsgRequestVehicleSwitchSeat.h"
#include "Server/Packets/CmsgChangeSeatsOnControlledVehicle.h"
#include "Server/Packets/CmsgPlayerVehicleEnter.h"
#include "Server/Packets/CmsgEjectPassenger.h"
#include "Server/WorldSession.h"
#include "Objects/Units/Players/Player.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/Vehicle.hpp"
#include "Storage/WDB/WDBStructures.hpp"

using namespace AscEmu::Packets;

void WorldSession::handleDismissVehicle(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    uint64_t vehicleGUID = _player->getCharmGuid();

    if (!vehicleGUID)   // something wrong here...
    {
        recvPacket.rfinish();   // prevent warnings spam
        return;
    }

    _player->obj_movement_info.readMovementInfo(recvPacket, recvPacket.GetOpcode());
    _player->callExitVehicle();
#endif
}

void WorldSession::handleRequestVehiclePreviousSeat(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
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
#endif
}

void WorldSession::handleRequestVehicleNextSeat(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
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
#endif
}

void WorldSession::handleRequestVehicleSwitchSeat(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
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
    else if (Unit* vehUnit = GetPlayer()->getWorldMap()->getUnit(guid.getRawGuid()))
    {
        if (Vehicle* vehicle = vehUnit->getVehicleKit())
        {
            if (vehicle->hasEmptySeat(seatId))
            {
                vehUnit->handleSpellClick(GetPlayer(), seatId);
            }
        }
    }
#endif
}

void WorldSession::handleChangeSeatsOnControlledVehicle([[maybe_unused]]WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
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
    uint64_t accessory = srlPacket.destinationGuid;     // accessory guid

    vehicle_base->obj_movement_info = srlPacket.movementInfo;
    int8_t seatId = srlPacket.seat;

    if (vehicle_base->getGuid() != guid)
        return;

    if (!accessory)
    {
        GetPlayer()->callChangeSeat(seatId, seatId > 0); // prev/next
    }
    else if (Unit* vehUnit = GetPlayer()->getWorldMapUnit(accessory))
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
    static MovementStatusElements const accessoryGuid[] =
    {
        MSEExtraInt8,
        MSEGuidBit2,
        MSEGuidBit4,
        MSEGuidBit7,
        MSEGuidBit6,
        MSEGuidBit5,
        MSEGuidBit0,
        MSEGuidBit1,
        MSEGuidBit3,
        MSEGuidByte6,
        MSEGuidByte1,
        MSEGuidByte2,
        MSEGuidByte5,
        MSEGuidByte3,
        MSEGuidByte0,
        MSEGuidByte4,
        MSEGuidByte7,
    };

    ExtraMovementStatusElement extra(accessoryGuid);
    MovementInfo movementInfo;
    movementInfo.readMovementInfo(recvPacket, recvPacket.GetOpcode(), &extra);
    vehicle_base->obj_movement_info = movementInfo;

    ObjectGuid accessory = extra.Data.guid;
    int8_t seatId = extra.Data.byteData;

    if (vehicle_base->getGuid() != movementInfo.guid)
        return;

    if (!accessory)
    {
        GetPlayer()->callChangeSeat(seatId, seatId > 0); // prev/next
    }
    else if (Unit* vehUnit = GetPlayer()->getWorldMapUnit(accessory))
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
#endif
#endif
}

void WorldSession::handleRemoveVehiclePassenger(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
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

    const auto passengerUnit = _player->getWorldMap()->getUnit(srlPacket.guid);
    if (!passengerUnit)
        return;

    if (!passengerUnit->isOnVehicle(vehicle->getBase()))
        return;

    auto seat = vehicle->getSeatForPassenger(passengerUnit);
    if (seat)
        if (seat->isEjectable())
            passengerUnit->callExitVehicle();
#endif
}

void WorldSession::handleLeaveVehicle(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING > TBC
    if (Vehicle* vehicle = GetPlayer()->getVehicle())
    {
        if (WDB::Structures::VehicleSeatEntry const* seat = vehicle->getSeatForPassenger(GetPlayer()))
        {
            if (seat->canEnterOrExit())
                GetPlayer()->callExitVehicle();
        }
    }
#endif
}

void WorldSession::handleEnterVehicle(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    CmsgPlayerVehicleEnter srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto unit = _player->getWorldMap()->getUnit(srlPacket.guid);
    if (unit == nullptr)
        return;

    if (!_player->isInRange(unit, MAX_INTERACTION_RANGE))
        return;

    if (unit->getVehicleKit() == nullptr)
        return;

    _player->callEnterVehicle(unit);
#endif
}
