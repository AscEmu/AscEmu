/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "Objects/MovementInfo.hpp"

namespace AscEmu::Packets
{
    class SmsgForceWalkSpeedChange : public ManagedPacket
    {
    public:
        WoWGuid guid;
        float rate;
        MovementInfo mi;

        SmsgForceWalkSpeedChange() : SmsgForceWalkSpeedChange(WoWGuid(), 0, MovementInfo())
        {
        }

        SmsgForceWalkSpeedChange(WoWGuid guid, float rate, MovementInfo mi) :
//Zyres: Due to wrong understanding of these opcodes the logic gets turned around here
#if VERSION_STRING < Cata
            ManagedPacket(MSG_MOVE_SET_WALK_SPEED, 0),
#else
            ManagedPacket(SMSG_FORCE_WALK_SPEED_CHANGE, 0),
#endif
            guid(guid),
            rate(rate),
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
#if VERSION_STRING < Cata
            packet << guid << uint32_t(0) << rate;

            ByteBuffer addition;
            mi.writeMovementInfo(addition, 0, false);
            packet.append(addition);
#elif VERSION_STRING == Cata
            bool hasTransportData = !mi.transport_guid.isEmpty();

            packet.writeBit(!mi.status_info.hasOrientation);
            packet.writeBit(false);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[2]);

            if (hasTransportData)
            {
                packet.writeBit(mi.transport_guid[6]);
                packet.writeBit(mi.transport_guid[3]);
                packet.writeBit(mi.transport_guid[2]);
                packet.writeBit(mi.transport_guid[0]);
                packet.writeBit(mi.transport_guid[4]);
                packet.writeBit(mi.transport_guid[7]);
                packet.writeBit(mi.transport_guid[1]);
                packet.writeBit(mi.transport_guid[5]);
            }

            packet.writeBit(guid[7]);
            packet.writeBit(guid[5]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[0]);
            packet.writeBit(!mi.flags);
            packet.writeBit(guid[6]);

            if (mi.status_info.hasFallData)
                packet.writeBit(mi.status_info.hasFallDirection);

            if (mi.flags)
                packet.writeBits(mi.flags, 30);

            packet.writeBit(guid[4]);

            if (mi.status_info.hasPitch)
                packet << float(mi.pitch_rate);

            if (hasTransportData)
            {
                packet.WriteByteSeq(mi.transport_guid[6]);
                packet.WriteByteSeq(mi.transport_guid[0]);
                packet.WriteByteSeq(mi.transport_guid[4]);
                packet.WriteByteSeq(mi.transport_guid[2]);
                packet << float(mi.transport_position.x);
                packet.WriteByteSeq(mi.transport_guid[7]);
                packet << uint32_t(mi.transport_time);
                packet << float(mi.transport_position.z);
                packet << int8_t(mi.transport_seat);
                packet.WriteByteSeq(mi.transport_guid[5]);
                packet << float(normalizeOrientation(mi.transport_position.o));
                packet.WriteByteSeq(mi.transport_guid[1]);
                packet << float(mi.transport_position.y);
                packet.WriteByteSeq(mi.transport_guid[3]);
            }

            if (mi.status_info.hasFallData)
            {
                packet << float(mi.jump_info.velocity);

                if (mi.status_info.hasFallDirection)
                {
                    packet << float(mi.jump_info.xyspeed);
                    packet << float(mi.jump_info.sinAngle);
                    packet << float(mi.jump_info.cosAngle);
                }

                packet << uint32_t(mi.fall_time);
            }

            packet.WriteByteSeq(guid[1]);
            packet.WriteByteSeq(guid[4]);
            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[6]);
            packet.WriteByteSeq(guid[7]);

            if (mi.status_info.hasTimeStamp)
                packet << ::Util::getMSTime();

            if (mi.status_info.hasOrientation)
                packet << float(normalizeOrientation(mi.position.o));

            packet << float(mi.position.y);

            packet.WriteByteSeq(guid[0]);

            packet << float(mi.position.z);
            packet << float(mi.position.x);

            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[5]);

            packet << float(rate);
#else // TODO: Mop
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
