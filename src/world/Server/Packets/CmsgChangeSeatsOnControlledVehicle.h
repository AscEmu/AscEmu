/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgChangeSeatsOnControlledVehicle : public ManagedPacket
    {
    public:
        WoWGuid sourceGuid;
        WoWGuid destinationGuid;
        uint8_t seat;
        MovementInfo movementInfo;

        CmsgChangeSeatsOnControlledVehicle() : CmsgChangeSeatsOnControlledVehicle(0, 0, 0, {})
        {
        }

        CmsgChangeSeatsOnControlledVehicle(uint64_t sourceGuid, uint64_t destinationGuid, uint8_t seat, MovementInfo movementInfo) :
            ManagedPacket(CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE, 0),
            sourceGuid(sourceGuid),
            destinationGuid(destinationGuid),
            seat(seat),
            movementInfo(movementInfo)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING != Cata
            packet >> sourceGuid;
            packet >> movementInfo.flags >> movementInfo.flags2 >> movementInfo.time >> movementInfo.position >> movementInfo.position.o;

            if (movementInfo.isOnTransport())
            {
                packet >> movementInfo.transport_data.transportGuid >> movementInfo.transport_data.relativePosition
                    >> movementInfo.transport_data.relativePosition.o >> movementInfo.transport_time >> movementInfo.transport_seat;

                if (movementInfo.isInterpolated())
                    packet >> movementInfo.transport_time2;
            }

            if (movementInfo.isSwimmingOrFlying())
                packet >> movementInfo.pitch;

            packet >> movementInfo.fall_time;

            if (movementInfo.isFallingOrRedirected())
                packet >> movementInfo.redirect_velocity >> movementInfo.redirect_sin >> movementInfo.redirect_cos >> movementInfo.redirect_2d_speed;

            if (movementInfo.isSplineMover())
                packet >> movementInfo.spline_elevation;

            packet >> destinationGuid;
            packet >> seat;
#endif
            return true;
        }
    };
}}
