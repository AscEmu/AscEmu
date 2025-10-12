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
#if VERSION_STRING >= WotLK
    public:
        WoWGuid sourceGuid;
        WoWGuid destinationGuid;
        int8_t seat;
        MovementInfo movementInfo;

        bool hasTransportData = false;
        bool hasMovementFlags = false;
        bool hasMovementFlags2 = false;

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
#if VERSION_STRING == WotLK
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

#elif VERSION_STRING == Cata
            packet >> movementInfo.position.y;
            packet >> movementInfo.position.x;
            packet >> movementInfo.position.z;

            packet >> seat;
            destinationGuid[2] = packet.readBit();
            destinationGuid[4] = packet.readBit();
            destinationGuid[7] = packet.readBit();
            destinationGuid[6] = packet.readBit();
            destinationGuid[5] = packet.readBit();
            destinationGuid[0] = packet.readBit();
            destinationGuid[1] = packet.readBit();
            destinationGuid[3] = packet.readBit();
            packet.ReadByteSeq(destinationGuid[6]);
            packet.ReadByteSeq(destinationGuid[1]);
            packet.ReadByteSeq(destinationGuid[2]);
            packet.ReadByteSeq(destinationGuid[5]);
            packet.ReadByteSeq(destinationGuid[3]);
            packet.ReadByteSeq(destinationGuid[0]);
            packet.ReadByteSeq(destinationGuid[4]);
            packet.ReadByteSeq(destinationGuid[7]);

            hasMovementFlags = !packet.readBit();
            hasTransportData = packet.readBit();
            movementInfo.guid[2] = packet.readBit();
            movementInfo.guid[6] = packet.readBit();
            movementInfo.guid[4] = packet.readBit();
            movementInfo.guid2[2] = packet.readBit();
            movementInfo.guid2[4] = packet.readBit();
            movementInfo.status_info.hasOrientation = !packet.readBit();

            packet.readBit();
            movementInfo.guid2[7] = packet.readBit();
            movementInfo.guid[7] = packet.readBit();
            movementInfo.guid2[6] = packet.readBit();
            movementInfo.status_info.hasTimeStamp = !packet.readBit();
            movementInfo.status_info.hasSplineElevation = !packet.readBit();
            movementInfo.guid[5] = packet.readBit();
            movementInfo.guid2[5] = packet.readBit();

            hasMovementFlags2 = !packet.readBit();
            movementInfo.status_info.hasPitch = !packet.readBit();
            movementInfo.guid2[0] = packet.readBit();
            movementInfo.guid[0] = packet.readBit();
            movementInfo.guid2[1] = packet.readBit();
            movementInfo.status_info.hasFallData = packet.readBit();
            movementInfo.guid[1] = packet.readBit();
            movementInfo.status_info.hasSpline = packet.readBit();

            if (hasMovementFlags)
                movementInfo.flags = packet.readBits(30);

            movementInfo.guid2[3] = packet.readBit();
            movementInfo.guid[3] = packet.readBit();

            if (hasTransportData)
            {
                movementInfo.transport_guid[3] = packet.readBit();
                movementInfo.transport_guid[0] = packet.readBit();
                movementInfo.transport_guid[7] = packet.readBit();
                movementInfo.transport_guid[5] = packet.readBit();
                movementInfo.status_info.hasTransportTime3 = packet.readBit();
                movementInfo.transport_guid[1] = packet.readBit();

                movementInfo.transport_guid[2] = packet.readBit();
                movementInfo.status_info.hasTransportTime2 = packet.readBit();
                movementInfo.transport_guid[4] = packet.readBit();
                movementInfo.transport_guid[6] = packet.readBit();
            }
            if (hasMovementFlags2)
                movementInfo.flags2 = static_cast<uint16_t>(packet.readBits(12));

            if (movementInfo.status_info.hasFallData)
                movementInfo.status_info.hasFallDirection = packet.readBit();

            packet.ReadByteSeq(movementInfo.guid2[6]);
            packet.ReadByteSeq(movementInfo.guid[7]);
            packet.ReadByteSeq(movementInfo.guid[5]);
            packet.ReadByteSeq(movementInfo.guid2[1]);
            packet.ReadByteSeq(movementInfo.guid2[2]);
            packet.ReadByteSeq(movementInfo.guid[6]);
            packet.ReadByteSeq(movementInfo.guid2[5]);
            packet.ReadByteSeq(movementInfo.guid2[3]);
            packet.ReadByteSeq(movementInfo.guid[3]);
            packet.ReadByteSeq(movementInfo.guid2[0]);
            packet.ReadByteSeq(movementInfo.guid[0]);
            packet.ReadByteSeq(movementInfo.guid2[4]);
            packet.ReadByteSeq(movementInfo.guid[4]);
            packet.ReadByteSeq(movementInfo.guid[1]);
            packet.ReadByteSeq(movementInfo.guid2[7]);
            packet.ReadByteSeq(movementInfo.guid[2]);

            if (movementInfo.status_info.hasPitch)
                packet >> movementInfo.pitch_rate;

            if (movementInfo.status_info.hasFallData)
            {
                if (movementInfo.status_info.hasFallDirection)
                {
                    packet >> movementInfo.jump_info.sinAngle;
                    packet >> movementInfo.jump_info.cosAngle;
                    packet >> movementInfo.jump_info.xyspeed;
                }

                packet >> movementInfo.fall_time;
                packet >> movementInfo.jump_info.velocity;
            }

            if (hasTransportData)
            {
                packet.ReadByteSeq(movementInfo.transport_guid[2]);

                if (movementInfo.status_info.hasTransportTime2)
                    packet >> movementInfo.transport_time2;

                if (movementInfo.status_info.hasTransportTime3)
                    packet >> movementInfo.transport_time3;

                packet.ReadByteSeq(movementInfo.transport_guid[0]);
                packet >> movementInfo.transport_time;
                packet >> movementInfo.transport_seat;
                packet >> movementInfo.transport_position.x;
                packet >> movementInfo.transport_position.o;
                packet.ReadByteSeq(movementInfo.transport_guid[7]);
                packet.ReadByteSeq(movementInfo.transport_guid[4]);
                packet.ReadByteSeq(movementInfo.transport_guid[3]);
                packet.ReadByteSeq(movementInfo.transport_guid[5]);
                packet >> movementInfo.transport_position.z;
                packet.ReadByteSeq(movementInfo.transport_guid[1]);
                packet.ReadByteSeq(movementInfo.transport_guid[6]);
                packet >> movementInfo.transport_position.y;
            }

            if (movementInfo.status_info.hasSplineElevation)
                packet >> movementInfo.spline_elevation;

            if (movementInfo.status_info.hasOrientation)
                packet >> movementInfo.position.o;

            if (movementInfo.status_info.hasTimeStamp)
                packet >> movementInfo.update_time;

#else //TODO: Mop
#endif
            return true;
        }
#endif
    };
}
