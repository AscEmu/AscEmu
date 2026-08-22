/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgRaidReadyCheckCompleted : public ManagedPacket
    {
    public:
        WoWGuid groupGuid;

        SmsgRaidReadyCheckCompleted() : SmsgRaidReadyCheckCompleted(0)
        {
        }

        SmsgRaidReadyCheckCompleted(uint64_t groupGuid) :
            ManagedPacket(SMSG_RAID_READY_CHECK_COMPLETED, 10),
            groupGuid(groupGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return 10; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
                return false;

            packet.writeBit(groupGuid[4]);
            packet.writeBit(groupGuid[2]);
            packet.writeBit(groupGuid[5]);
            packet.writeBit(groupGuid[7]);
            packet.writeBit(groupGuid[1]);
            packet.writeBit(groupGuid[0]);
            packet.writeBit(groupGuid[3]);
            packet.writeBit(groupGuid[6]);

            packet.flushBits();

            packet.writeByteSeq(groupGuid[6]);
            packet.writeByteSeq(groupGuid[0]);
            packet.writeByteSeq(groupGuid[3]);
            packet.writeByteSeq(groupGuid[1]);
            packet.writeByteSeq(groupGuid[5]);
            packet << static_cast<uint8_t>(0);
            packet.writeByteSeq(groupGuid[7]);
            packet.writeByteSeq(groupGuid[2]);
            packet.writeByteSeq(groupGuid[4]);

            return true;
        }
    };
}
