/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "Objects/MovementInfo.hpp"

namespace AscEmu::Packets
{
    class SmsgMoveUpdateTeleport : public ManagedPacket
    {
    public:
        WoWGuid guid;
        MovementInfo mi;

        SmsgMoveUpdateTeleport() : SmsgMoveUpdateTeleport(WoWGuid(), MovementInfo())
        {
        }

        SmsgMoveUpdateTeleport(WoWGuid guid, MovementInfo mi) :
            ManagedPacket(SMSG_MOVE_UPDATE_TELEPORT, 0),
            guid(guid),
            mi(mi)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 4 + 4 + 4 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING >= Cata
            bool hasTransportData = !mi.transport_guid.isEmpty();

            packet << float(mi.position.z);
            packet << float(mi.position.y);
            packet << float(mi.position.x);

            packet.writeBit(!mi.status_info.hasOrientation);
            packet.writeBit(mi.status_info.hasSpline);
            packet.writeBit(!mi.flags);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[6]);
            packet.writeBit(mi.status_info.hasFallData);
            packet.writeBit(guid[0]);
            packet.writeBit(hasTransportData);
            packet.writeBit(guid[5]);

            if (hasTransportData)
            {
                packet.writeBit(mi.transport_guid[1]);
                packet.writeBit(mi.transport_guid[4]);
                packet.writeBit(mi.transport_guid[5]);
                packet.writeBit(mi.transport_guid[3]);
                packet.writeBit(mi.transport_guid[0]);
                packet.writeBit(mi.status_info.hasTransportTime2);
                packet.writeBit(mi.transport_guid[7]);
                packet.writeBit(mi.transport_guid[6]);
                packet.writeBit(mi.status_info.hasTransportTime3);
                packet.writeBit(mi.transport_guid[2]);
            }

            packet.writeBit(false);

            packet.writeBit(guid[7]);
            packet.writeBit(guid[3]);
            packet.writeBit(!mi.status_info.hasPitch);
            packet.writeBit(!mi.flags2);
            packet.writeBit(!mi.status_info.hasTimeStamp);

            if (mi.status_info.hasFallData)
                packet.writeBit(mi.status_info.hasFallDirection);

            if (mi.flags2)
                packet.writeBits(mi.flags2, 12);

            packet.writeBit(!mi.status_info.hasSplineElevation);

            if (mi.flags)
                packet.writeBits(mi.flags, 30);

            packet.writeBit(guid[1]);

            packet.WriteByteSeq(guid[7]);

            if (hasTransportData)
            {
                packet.WriteByteSeq(mi.transport_guid[3]);
                packet.WriteByteSeq(mi.transport_guid[4]);
                packet << float(normalizeOrientation(mi.transport_position.o));

                if (mi.status_info.hasTransportTime3)
                    packet << uint32_t(mi.transport_time3);

                packet.WriteByteSeq(mi.transport_guid[1]);

                if (mi.status_info.hasTransportTime2)
                    packet << uint32_t(mi.transport_time2);

                packet << float(mi.transport_position.z);
                packet.WriteByteSeq(mi.transport_guid[7]);
                packet.WriteByteSeq(mi.transport_guid[0]);
                packet.WriteByteSeq(mi.transport_guid[6]);
                packet.WriteByteSeq(mi.transport_guid[5]);
                packet.WriteByteSeq(mi.transport_guid[2]);
                packet << int8_t(mi.transport_seat);
                packet << uint32_t(mi.transport_time);
                packet << float(mi.transport_position.y);
                packet << float(mi.transport_position.x);
            }

            packet.WriteByteSeq(guid[6]);

            if (mi.status_info.hasPitch)
                packet << float(mi.pitch_rate);
            if (mi.status_info.hasSplineElevation)
                packet << float(mi.spline_elevation);
            if (mi.status_info.hasOrientation)
                packet << float(normalizeOrientation(mi.position.o));


            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[1]);

            if (mi.status_info.hasFallData)
            {
                packet << uint32_t(mi.fall_time);
                if (mi.status_info.hasFallDirection)
                {
                    packet << float(mi.jump_info.xyspeed);
                    packet << float(mi.jump_info.cosAngle);
                    packet << float(mi.jump_info.sinAngle);
                }
                packet << float(mi.jump_info.velocity);
            }

            packet.WriteByteSeq(guid[5]);
            packet.WriteByteSeq(guid[4]);

            if (mi.status_info.hasTimeStamp)
                packet << ::Util::getMSTime();

            packet.WriteByteSeq(guid[0]);
// TODO: Mop
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
