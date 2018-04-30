/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
 */

#include "StdAfx.h"
#include "Map/MapMgr.h"
#include "Server/WorldSession.h"
#include "Units/Creatures/Vehicle.h"
#include "Units/Players/Player.h"

#if VERSION_STRING != Cata
#if VERSION_STRING > TBC
void WorldSession::HandleDismissVehicle(WorldPacket& /*recv_data*/)
{
    uint64 current_vehicle_guid = _player->getCharmGuid();

    // wait what no vehicle
    if (current_vehicle_guid == 0)
        return;

    Unit* v = _player->GetMapMgr()->GetUnit(current_vehicle_guid);
    if (v == NULL)
        return;

    if (v->GetVehicleComponent() == NULL)
        return;

    v->GetVehicleComponent()->EjectPassenger(_player);
}


void WorldSession::HandleChangeVehicleSeat(WorldPacket& recv_data)
{
    if (_player->GetCurrentVehicle() == NULL)
        return;

    switch (recv_data.GetOpcode())
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

            recv_data >> vehicle;
            recv_data >> seat;

            if (vehicle.GetOldGuid() == _player->GetCurrentVehicle()->GetOwner()->getGuid())
            {
                _player->GetCurrentVehicle()->MovePassengerToSeat(_player, seat);
            }
            else
            {
                Unit* u = _player->GetMapMgr()->GetUnit(vehicle.GetOldGuid());
                if (u == NULL)
                    return;

                if (u->GetVehicleComponent() == NULL)
                    return;

                // Has to be same vehicle, or an accessory of the vehicle
                if (_player->GetVehicleBase()->getGuid() != u->GetVehicleBase()->getGuid())
                    return;

                _player->GetCurrentVehicle()->EjectPassenger(_player);
                u->GetVehicleComponent()->AddPassengerToSeat(_player, seat);
            }

            break;
        }

        // Used when switching from controlling seat to accessory, or from accessory to accessory
        case CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE:
        {
#if VERSION_STRING != Cata
            WoWGuid src_guid;
            WoWGuid dst_guid;
            uint8 seat = 0;
            MovementInfo mov;

            recv_data >> src_guid;
            
            recv_data >> mov.flags >> mov.flags2 >> mov.time
                >> mov.position >> mov.position.o;

            if (mov.isOnTransport())
            {
                recv_data >> mov.transport_data.transportGuid >> mov.transport_data.relativePosition
                    >> mov.transport_data.relativePosition.o >> mov.transport_time >> mov.transport_seat;

                if (mov.isInterpolated())
                    recv_data >> mov.transport_time2;
            }

            if (mov.isSwimmingOrFlying())
                recv_data >> mov.pitch;

            recv_data >> mov.fall_time;

            if (mov.isFallingOrRedirected())
                recv_data >> mov.redirect_velocity >> mov.redirect_sin >> mov.redirect_cos >> mov.redirect_2d_speed;

            if (mov.isSplineMover())
                recv_data >> mov.spline_elevation;

            recv_data >> dst_guid;
            recv_data >> seat;

            Unit* src_vehicle = _player->GetMapMgr()->GetUnit(src_guid.GetOldGuid());
            if (src_vehicle == NULL)
                return;

            if (src_vehicle->GetVehicleComponent() == NULL)
                return;

            if (src_vehicle->getGuid() != _player->GetCurrentVehicle()->GetOwner()->getGuid())
                return;

            Unit* dst_vehicle = _player->GetMapMgr()->GetUnit(dst_guid.GetOldGuid());
            if (dst_vehicle == NULL)
                return;

            if (dst_vehicle->GetVehicleComponent() == NULL)
                return;

            if (src_vehicle->getGuid() == dst_vehicle->getGuid())
            {
                src_vehicle->GetVehicleComponent()->MovePassengerToSeat(_player, seat);
            }
            else
            {
                // Has to be the same vehicle or an accessory of the vehicle
                if (src_vehicle->GetVehicleBase()->getGuid() != dst_vehicle->GetVehicleBase()->getGuid())
                    return;

                _player->GetCurrentVehicle()->EjectPassenger(_player);
                dst_vehicle->GetVehicleComponent()->AddPassengerToSeat(_player, seat);
            }

            break;
#endif
        }
    }
}


void WorldSession::HandleRemoveVehiclePassenger(WorldPacket& recv_data)
{
    Vehicle* v = NULL;
    if (_player->isVehicle())
        v = _player->GetVehicleComponent();
    else
        v = _player->GetCurrentVehicle();

    if (v == NULL)
        return;

    uint64 guid = 0;
    recv_data >> guid;

    if (guid == 0)
        return;

    Unit* passenger = _player->GetMapMgr()->GetUnit(guid);
    if (passenger == NULL)
        return;

    v->EjectPassenger(passenger);
}


void WorldSession::HandleLeaveVehicle(WorldPacket& /*recv_data*/)
{
    if (_player->GetCurrentVehicle() == NULL)
        return;

    _player->GetCurrentVehicle()->EjectPassenger(_player);
}


void WorldSession::HandleEnterVehicle(WorldPacket& recv_data)
{
    uint64 guid;

    recv_data >> guid;

    Unit* v = _player->GetMapMgr()->GetUnit(guid);
    if (v == NULL)
        return;

    if (!_player->isInRange(v, MAX_INTERACTION_RANGE))
        return;

    if (v->GetVehicleComponent() == NULL)
        return;

    v->GetVehicleComponent()->AddPassenger(_player);
}
#endif
#endif
