/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

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

        bool internalSerialise([[maybe_unused]] WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                mi.position = lv;
                packet << guid;
                mi.writeMovementInfo(packet, 0, false);

            }
            else if (m_protocol.isCata())
            {
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
                    packet.writeByteSeq(mi.transport_guid[5]);
                    packet.writeByteSeq(mi.transport_guid[6]);
                    packet.writeByteSeq(mi.transport_guid[1]);
                    packet.writeByteSeq(mi.transport_guid[7]);
                    packet.writeByteSeq(mi.transport_guid[0]);
                    packet.writeByteSeq(mi.transport_guid[2]);
                    packet.writeByteSeq(mi.transport_guid[4]);
                    packet.writeByteSeq(mi.transport_guid[3]);
                }

                packet << uint32_t(0); // unk
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[5]);
                packet << lv.x;
                packet.writeByteSeq(guid[4]);
                packet << lv.o;
                packet.writeByteSeq(guid[7]);
                packet << lv.z;
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[6]);
                packet << lv.y;
            }
            // TODO: MoP
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
