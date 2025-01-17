/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"
#include "Objects/MovementDefines.hpp"
#include "Objects/MovementInfo.hpp"

namespace AscEmu::Packets
{
    class CmsgChangeSeatsOnControlledVehicle : public ManagedPacket
    {
#if VERSION_STRING == WotLK
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> sourceGuid;
            packet >> movementInfo.flags >> movementInfo.flags2 >> movementInfo.update_time >> movementInfo.position >> movementInfo.position.o;

            if (movementInfo.hasMovementFlag(MOVEFLAG_TRANSPORT))
            {
                packet >> movementInfo.transport_guid >> movementInfo.transport_position
                    >> movementInfo.transport_position.o >> movementInfo.transport_time >> movementInfo.transport_seat;

                if (movementInfo.hasMovementFlag2(MOVEFLAG2_INTERPOLATED_MOVE))
                    packet >> movementInfo.transport_time2;
            }

            if (movementInfo.hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) || movementInfo.hasMovementFlag2(MOVEFLAG2_ALLOW_PITCHING))
                packet >> movementInfo.pitch_rate;

            packet >> movementInfo.fall_time;

            if (movementInfo.hasMovementFlag(MOVEFLAG_FALLING))
                packet >> movementInfo.jump_info.velocity >> movementInfo.jump_info.sinAngle >> movementInfo.jump_info.cosAngle >> movementInfo.jump_info.xyspeed;

            if (movementInfo.hasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
                packet >> movementInfo.spline_elevation;

            packet >> destinationGuid;
            packet >> seat;

            return true;
        }
#endif
    };
}
