/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgRequestVehicleSwitchSeat.h"
#include "Server/Packets/CmsgChangeSeatsOnControlledVehicle.h"
#include "Server/Packets/CmsgPlayerVehicleEnter.h"
#include "Server/Packets/CmsgEjectPassenger.h"
#include "Server/WorldSession.h"
#include "Units/Players/Player.h"
#include "Map/MapMgr.h"
#include "Units/Creatures/Vehicle.h"

using namespace AscEmu::Packets;

#if VERSION_STRING > TBC
void WorldSession::handleDismissVehicle(WorldPacket& /*recvPacket*/)
{
    if (_player->getCurrentVehicle() == nullptr)
        return;

    const auto unit = _player->GetMapMgr()->GetUnit(_player->getCharmGuid());
    if (unit == nullptr)
        return;

    if (unit->getVehicleComponent() == nullptr)
        return;

    unit->getVehicleComponent()->EjectPassenger(_player);
}

void WorldSession::handleRequestVehiclePreviousSeat(WorldPacket& /*recvPacket*/)
{
    if (_player->getCurrentVehicle() == nullptr)
        return;

    _player->getCurrentVehicle()->MovePassengerToPrevSeat(_player);
}

void WorldSession::handleRequestVehicleNextSeat(WorldPacket& /*recvPacket*/)
{
    if (_player->getCurrentVehicle() == nullptr)
        return;

    _player->getCurrentVehicle()->MovePassengerToNextSeat(_player);
}

void WorldSession::handleRequestVehicleSwitchSeat(WorldPacket& recvPacket)
{
    if (_player->getCurrentVehicle() == nullptr)
        return;

    CmsgRequestVehicleSwitchSeat srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.guid.GetOldGuid() == _player->getCurrentVehicle()->GetOwner()->getGuid())
    {
        _player->getCurrentVehicle()->MovePassengerToSeat(_player, srlPacket.seat);
    }
    else
    {
        const auto unit = _player->GetMapMgr()->GetUnit(srlPacket.guid.GetOldGuid());
        if (unit == nullptr)
            return;

        if (unit->getVehicleComponent() == nullptr)
            return;

        if (_player->getVehicleBase()->getGuid() != unit->getVehicleBase()->getGuid())
            return;

        _player->getCurrentVehicle()->EjectPassenger(_player);
        unit->getVehicleComponent()->AddPassengerToSeat(_player, srlPacket.seat);
    }
}

void WorldSession::handleChangeSeatsOnControlledVehicle(WorldPacket& recvPacket)
{
    if (_player->getCurrentVehicle() == nullptr)
        return;

#if VERSION_STRING == WotLK
    CmsgChangeSeatsOnControlledVehicle srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto sourceUnit = _player->GetMapMgr()->GetUnit(srlPacket.sourceGuid.GetOldGuid());
    if (sourceUnit == nullptr)
        return;

    if (sourceUnit->getVehicleComponent() == nullptr)
        return;

    if (sourceUnit->getGuid() != _player->getCurrentVehicle()->GetOwner()->getGuid())
        return;

    const auto destinationUnit = _player->GetMapMgr()->GetUnit(srlPacket.destinationGuid.GetOldGuid());
    if (destinationUnit == nullptr)
        return;

    if (destinationUnit->getVehicleComponent() == nullptr)
        return;

    if (sourceUnit->getGuid() == destinationUnit->getGuid())
    {
        sourceUnit->getVehicleComponent()->MovePassengerToSeat(_player, srlPacket.seat);
    }
    else
    {
        if (sourceUnit->getVehicleBase()->getGuid() != destinationUnit->getVehicleBase()->getGuid())
            return;

        _player->getCurrentVehicle()->EjectPassenger(_player);
        destinationUnit->getVehicleComponent()->AddPassengerToSeat(_player, srlPacket.seat);
    }
#endif
}

void WorldSession::handleRemoveVehiclePassenger(WorldPacket& recvPacket)
{
    Vehicle* vehicle = nullptr;
    if (_player->isVehicle())
        vehicle = _player->getVehicleComponent();
    else
        vehicle = _player->getCurrentVehicle();

    if (vehicle == nullptr)
        return;

    CmsgEjectPassenger srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.guid == 0)
        return;

    const auto passengerUnit = _player->GetMapMgr()->GetUnit(srlPacket.guid);
    if (passengerUnit == nullptr)
        return;

    vehicle->EjectPassenger(passengerUnit);
}

void WorldSession::handleLeaveVehicle(WorldPacket& /*recvPacket*/)
{
    if (_player->getCurrentVehicle() == nullptr)
        return;

    _player->getCurrentVehicle()->EjectPassenger(_player);
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

    if (unit->getVehicleComponent() == nullptr)
        return;

    unit->getVehicleComponent()->AddPassenger(_player);
}
#endif
