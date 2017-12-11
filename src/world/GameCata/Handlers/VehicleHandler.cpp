/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Map/MapMgr.h"
#include "Server/WorldSession.h"
#include "Units/Creatures/Vehicle.h"
#include "Units/Players/Player.h"

//\todo Rewrite for cata - after this all functions are copied from wotlk

void WorldSession::HandleDismissVehicle(WorldPacket& /*recv_data*/)
{
    uint64 currentVehicleGuid = _player->GetCharmedUnitGUID();

    // wait what no vehicle
    if (currentVehicleGuid == 0)
        return;

    Unit* v = _player->GetMapMgr()->GetUnit(currentVehicleGuid);
    if (v == nullptr)
        return;

    if (v->GetVehicleComponent() == nullptr)
        return;

    v->GetVehicleComponent()->EjectPassenger(_player);
}

void WorldSession::HandleChangeVehicleSeat(WorldPacket& recvData)
{
    if (_player->GetCurrentVehicle() == nullptr)
        return;

    switch (recvData.GetOpcode())
    {
    case CMSG_REQUEST_VEHICLE_PREV_SEAT:
        _player->GetCurrentVehicle()->MovePassengerToPrevSeat(_player);
        break;

    case CMSG_REQUEST_VEHICLE_NEXT_SEAT:
        _player->GetCurrentVehicle()->MovePassengerToNextSeat(_player);
        break;

        // Used when switching from a normal seat to a controlling seat, or to an accessory
    case CMSG_REQUEST_VEHICLE_SWITCH_SEAT:
    {
        WoWGuid vehicle;
        uint8 seat = 0;

        recvData >> vehicle;
        recvData >> seat;

        if (vehicle.GetOldGuid() == _player->GetCurrentVehicle()->GetOwner()->GetGUID())
        {
            _player->GetCurrentVehicle()->MovePassengerToSeat(_player, seat);
        }
        else
        {
            Unit* u = _player->GetMapMgr()->GetUnit(vehicle.GetOldGuid());
            if (u == nullptr)
                return;

            if (u->GetVehicleComponent() == nullptr)
                return;

            // Has to be same vehicle, or an accessory of the vehicle
            if (_player->GetVehicleBase()->GetGUID() != u->GetVehicleBase()->GetGUID())
                return;

            _player->GetCurrentVehicle()->EjectPassenger(_player);
            u->GetVehicleComponent()->AddPassengerToSeat(_player, seat);
        }

        break;
    }

    // Used when switching from controlling seat to accessory, or from accessory to accessory
    case CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE:
    {

    }
    }
}

void WorldSession::HandleRemoveVehiclePassenger(WorldPacket& recvData)
{
    Vehicle* v = nullptr;
    if (_player->IsVehicle())
        v = _player->GetVehicleComponent();
    else
        v = _player->GetCurrentVehicle();

    if (v == nullptr)
        return;

    uint64 guid = 0;
    recvData >> guid;

    if (guid == 0)
        return;

    Unit* passenger = _player->GetMapMgr()->GetUnit(guid);
    if (passenger == nullptr)
        return;

    v->EjectPassenger(passenger);
}

void WorldSession::HandleLeaveVehicle(WorldPacket& /*recv_data*/)
{
    if (_player->GetCurrentVehicle() == nullptr)
        return;

    _player->GetCurrentVehicle()->EjectPassenger(_player);
}

void WorldSession::HandleEnterVehicle(WorldPacket& recvData)
{
    uint64 guid;

    recvData >> guid;

    Unit* v = _player->GetMapMgr()->GetUnit(guid);
    if (v == nullptr)
        return;

    if (!_player->isInRange(v, MAX_INTERACTION_RANGE))
        return;

    if (v->GetVehicleComponent() == nullptr)
        return;

    v->GetVehicleComponent()->AddPassenger(_player);
}
