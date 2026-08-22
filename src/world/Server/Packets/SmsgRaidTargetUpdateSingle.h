/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgRaidTargetUpdateSingle : public ManagedPacket
    {
    public:
        WoWGuid whoGuid;
        WoWGuid targetGuid;
        uint8_t id;
        uint8_t index;

        SmsgRaidTargetUpdateSingle() : SmsgRaidTargetUpdateSingle(0, 0, 0, 0)
        {
        }

        SmsgRaidTargetUpdateSingle(uint64_t whoGuid, uint64_t targetGuid, uint8_t id, uint8_t index) :
            ManagedPacket(SMSG_RAID_TARGET_UPDATE_SINGLE, 18),
            whoGuid(whoGuid),
            targetGuid(targetGuid),
            id(id),
            index(index)
        {
        }

    protected:
        size_t expectedSize() const override { return 18; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
                return false;

            packet.writeBit(whoGuid[6]);
            packet.writeBit(targetGuid[4]);
            packet.writeBit(whoGuid[0]);
            packet.writeBit(whoGuid[7]);
            packet.writeBit(targetGuid[6]);
            packet.writeBit(whoGuid[5]);
            packet.writeBit(whoGuid[3]);
            packet.writeBit(whoGuid[4]);
            packet.writeBit(targetGuid[7]);
            packet.writeBit(targetGuid[2]);
            packet.writeBit(targetGuid[5]);
            packet.writeBit(targetGuid[1]);
            packet.writeBit(whoGuid[2]);
            packet.writeBit(whoGuid[1]);
            packet.writeBit(targetGuid[0]);
            packet.writeBit(targetGuid[3]);
            packet.flushBits();

            packet.writeByteSeq(targetGuid[1]);
            packet << id;
            packet.writeByteSeq(whoGuid[0]);
            packet.writeByteSeq(whoGuid[5]);
            packet.writeByteSeq(whoGuid[3]);
            packet.writeByteSeq(targetGuid[7]);
            packet.writeByteSeq(targetGuid[6]);
            packet.writeByteSeq(whoGuid[1]);
            packet.writeByteSeq(targetGuid[2]);
            packet.writeByteSeq(targetGuid[4]);
            packet.writeByteSeq(targetGuid[0]);
            packet.writeByteSeq(targetGuid[3]);
            packet.writeByteSeq(targetGuid[5]);
            packet.writeByteSeq(whoGuid[6]);
            packet << index;

            return true;
        }
    };
}
