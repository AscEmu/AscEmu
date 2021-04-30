/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

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
#if VERSION_STRING == Mop
            ObjectGuid guid = itemGuid;
            packet.writeBit(guid[5]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[0]);
            packet.writeBit(guid[7]);

            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[6]);
            packet.WriteByteSeq(guid[7]);
            packet.WriteByteSeq(guid[4]);
            packet.WriteByteSeq(guid[0]);
            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[5]);
            packet.WriteByteSeq(guid[1]);
            packet << duration;
#else
            packet << itemGuid << duration;
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
