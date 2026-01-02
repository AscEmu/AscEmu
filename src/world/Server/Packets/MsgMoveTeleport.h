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
    class MsgMoveTeleport : public ManagedPacket
    {
    public:
        WoWGuid guid;
        LocationVector lv;
        MovementInfo mi;

        MsgMoveTeleport() : MsgMoveTeleport(WoWGuid(), LocationVector(), MovementInfo())
        {
        }

        MsgMoveTeleport(WoWGuid guid, LocationVector lv, MovementInfo mi) :
            ManagedPacket(MSG_MOVE_TELEPORT, 0),
            guid(guid),
            lv(lv),
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
            mi.position = lv;
            packet << guid;
            mi.writeMovementInfo(packet, 0, false);

#elif VERSION_STRING == Cata
            bool hasTransportData = !mi.transport_guid.isEmpty();

            packet.writeBit(guid[6]);
            packet.writeBit(guid[0]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[2]);

            packet.writeBit(0); // unk bool vehicle
            packet.writeBit(hasTransportData);
            packet.writeBit(guid[1]);

            if (hasTransportData)
            {
                packet.writeBit(mi.transport_guid[1]);
                packet.writeBit(mi.transport_guid[3]);
                packet.writeBit(mi.transport_guid[2]);
                packet.writeBit(mi.transport_guid[5]);
                packet.writeBit(mi.transport_guid[0]);
                packet.writeBit(mi.transport_guid[7]);
                packet.writeBit(mi.transport_guid[6]);
                packet.writeBit(mi.transport_guid[4]);
            }

            packet.writeBit(guid[4]);
            packet.writeBit(guid[7]);
            packet.writeBit(guid[5]);

            packet.flushBits();

            if (hasTransportData)
            {
                packet.WriteByteSeq(mi.transport_guid[5]);
                packet.WriteByteSeq(mi.transport_guid[6]);
                packet.WriteByteSeq(mi.transport_guid[1]);
                packet.WriteByteSeq(mi.transport_guid[7]);
                packet.WriteByteSeq(mi.transport_guid[0]);
                packet.WriteByteSeq(mi.transport_guid[2]);
                packet.WriteByteSeq(mi.transport_guid[4]);
                packet.WriteByteSeq(mi.transport_guid[3]);
            }

            packet << uint32_t(0); // unk
            packet.WriteByteSeq(guid[1]);
            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[5]);
            packet << lv.x;
            packet.WriteByteSeq(guid[4]);
            packet << lv.o;
            packet.WriteByteSeq(guid[7]);
            packet << lv.z;
            packet.WriteByteSeq(guid[0]);
            packet.WriteByteSeq(guid[6]);
            packet << lv.y;
#else // TODO: Mop
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
