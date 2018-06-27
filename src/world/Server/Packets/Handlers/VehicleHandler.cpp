/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgRequestVehicleSwitchSeat.h"
#include "Server/Packets/CmsgChangeSeatsOnControlledVehicle.h"
#include "Server/Packets/CmsgPlayerVehicleEnter.h"
#include "Server/Packets/CmsgEjectPassenger.h"

using namespace AscEmu::Packets;

#if VERSION_STRING > TBC
void WorldSession::handleDismissVehicle(WorldPacket& /*recvPacket*/)
{
    if (GetPlayer()->GetCurrentVehicle() == nullptr)
        return;

    const auto unit = GetPlayer()->GetMapMgr()->GetUnit(GetPlayer()->getCharmGuid());
    if (unit == nullptr)
        return;

    if (unit->GetVehicleComponent() == nullptr)
        return;

    unit->GetVehicleComponent()->EjectPassenger(GetPlayer());
}

void WorldSession::handleRequestVehiclePreviousSeat(WorldPacket& /*recvPacket*/)
{
    if (GetPlayer()->GetCurrentVehicle() == nullptr)
        return;

    GetPlayer()->GetCurrentVehicle()->MovePassengerToPrevSeat(GetPlayer());
}

void WorldSession::handleRequestVehicleNextSeat(WorldPacket& /*recvPacket*/)
{
    if (GetPlayer()->GetCurrentVehicle() == nullptr)
        return;

    GetPlayer()->GetCurrentVehicle()->MovePassengerToNextSeat(GetPlayer());
}

void WorldSession::handleRequestVehicleSwitchSeat(WorldPacket& recvPacket)
{
    if (GetPlayer()->GetCurrentVehicle() == nullptr)
        return;

    CmsgRequestVehicleSwitchSeat recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (recv_packet.guid.GetOldGuid() == GetPlayer()->GetCurrentVehicle()->GetOwner()->getGuid())
    {
        GetPlayer()->GetCurrentVehicle()->MovePassengerToSeat(GetPlayer(), recv_packet.seat);
    }
    else
    {
        const auto unit = GetPlayer()->GetMapMgr()->GetUnit(recv_packet.guid.GetOldGuid());
        if (unit == nullptr)
            return;

        if (unit->GetVehicleComponent() == nullptr)
            return;

        if (GetPlayer()->GetVehicleBase()->getGuid() != unit->GetVehicleBase()->getGuid())
            return;

        GetPlayer()->GetCurrentVehicle()->EjectPassenger(GetPlayer());
        unit->GetVehicleComponent()->AddPassengerToSeat(GetPlayer(), recv_packet.seat);
    }
}

void WorldSession::handleChangeSeatsOnControlledVehicle(WorldPacket& recvPacket)
{
    if (GetPlayer()->GetCurrentVehicle() == nullptr)
        return;

    CmsgChangeSeatsOnControlledVehicle recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto sourceUnit = GetPlayer()->GetMapMgr()->GetUnit(recv_packet.sourceGuid.GetOldGuid());
    if (sourceUnit == nullptr)
        return;

    if (sourceUnit->GetVehicleComponent() == nullptr)
        return;

    if (sourceUnit->getGuid() != GetPlayer()->GetCurrentVehicle()->GetOwner()->getGuid())
        return;

    const auto destinationUnit = GetPlayer()->GetMapMgr()->GetUnit(recv_packet.destinationGuid.GetOldGuid());
    if (destinationUnit == nullptr)
        return;

    if (destinationUnit->GetVehicleComponent() == nullptr)
        return;

    if (sourceUnit->getGuid() == destinationUnit->getGuid())
    {
        sourceUnit->GetVehicleComponent()->MovePassengerToSeat(GetPlayer(), recv_packet.seat);
    }
    else
    {
        if (sourceUnit->GetVehicleBase()->getGuid() != destinationUnit->GetVehicleBase()->getGuid())
            return;

        GetPlayer()->GetCurrentVehicle()->EjectPassenger(GetPlayer());
        destinationUnit->GetVehicleComponent()->AddPassengerToSeat(GetPlayer(), recv_packet.seat);
    }
}

void WorldSession::handleRemoveVehiclePassenger(WorldPacket& recvPacket)
{
    Vehicle* vehicle = nullptr;
    if (GetPlayer()->isVehicle())
        vehicle = GetPlayer()->GetVehicleComponent();
    else
        vehicle = GetPlayer()->GetCurrentVehicle();

    if (vehicle == nullptr)
        return;

    CmsgEjectPassenger recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (recv_packet.guid == 0)
        return;

    const auto passengerUnit = GetPlayer()->GetMapMgr()->GetUnit(recv_packet.guid);
    if (passengerUnit == nullptr)
        return;

    vehicle->EjectPassenger(passengerUnit);
}

void WorldSession::handleLeaveVehicle(WorldPacket& /*recvPacket*/)
{
    if (GetPlayer()->GetCurrentVehicle() == nullptr)
        return;

    GetPlayer()->GetCurrentVehicle()->EjectPassenger(GetPlayer());
}

void WorldSession::handleEnterVehicle(WorldPacket& recvPacket)
{
    CmsgPlayerVehicleEnter recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto unit = GetPlayer()->GetMapMgr()->GetUnit(recv_packet.guid);
    if (unit == nullptr)
        return;

    if (!GetPlayer()->isInRange(unit, MAX_INTERACTION_RANGE))
        return;

    if (unit->GetVehicleComponent() == nullptr)
        return;

    unit->GetVehicleComponent()->AddPassenger(GetPlayer());
}
#endif
