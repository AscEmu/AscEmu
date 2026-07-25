/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgItemTimeUpdate : public ManagedPacket
    {
    public:
        uint64_t itemGuid;
        uint32_t duration;

        SmsgItemTimeUpdate() : SmsgItemTimeUpdate(0, 0)
        {
        }

        SmsgItemTimeUpdate(uint64_t itemGuid, uint32_t duration) :
            ManagedPacket(SMSG_ITEM_TIME_UPDATE, 12),
            itemGuid(itemGuid),
            duration(duration)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                WoWGuid guid = itemGuid;
                packet.writeBit(guid[5]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[7]);

                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[1]);
                packet << duration;
            }
            else
            {
                packet << itemGuid << duration;
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
