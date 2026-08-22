/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgRaidReadyCheck : public ManagedPacket
    {
    public:
        WoWGuid groupGuid;
        WoWGuid playerGuid;
        uint32_t duration;

        SmsgRaidReadyCheck() : SmsgRaidReadyCheck(0, 0, 35000)
        {
        }

        SmsgRaidReadyCheck(uint64_t groupGuid, uint64_t playerGuid, uint32_t duration) :
            ManagedPacket(SMSG_RAID_READY_CHECK, 23),
            groupGuid(groupGuid),
            playerGuid(playerGuid),
            duration(duration)
        {
        }

    protected:
        size_t expectedSize() const override { return 23; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
                return false;

            packet.writeBit(groupGuid[4]);
            packet.writeBit(groupGuid[2]);
            packet.writeBit(playerGuid[4]);
            packet.writeBit(groupGuid[3]);
            packet.writeBit(groupGuid[7]);
            packet.writeBit(groupGuid[1]);
            packet.writeBit(groupGuid[0]);
            packet.writeBit(playerGuid[6]);
            packet.writeBit(playerGuid[5]);
            packet.writeBit(groupGuid[6]);
            packet.writeBit(groupGuid[5]);
            packet.writeBit(playerGuid[0]);
            packet.writeBit(playerGuid[1]);
            packet.writeBit(playerGuid[2]);
            packet.writeBit(playerGuid[7]);
            packet.writeBit(playerGuid[3]);
            packet.flushBits();

            packet << duration;

            packet.writeByteSeq(groupGuid[2]);
            packet.writeByteSeq(groupGuid[7]);
            packet.writeByteSeq(groupGuid[3]);
            packet.writeByteSeq(playerGuid[4]);
            packet.writeByteSeq(groupGuid[1]);
            packet.writeByteSeq(groupGuid[0]);
            packet.writeByteSeq(playerGuid[1]);
            packet.writeByteSeq(playerGuid[2]);
            packet.writeByteSeq(playerGuid[6]);
            packet.writeByteSeq(playerGuid[5]);
            packet.writeByteSeq(groupGuid[6]);
            packet.writeByteSeq(playerGuid[0]);
            packet << static_cast<uint8_t>(0);
            packet.writeByteSeq(playerGuid[7]);
            packet.writeByteSeq(groupGuid[4]);
            packet.writeByteSeq(playerGuid[3]);
            packet.writeByteSeq(groupGuid[5]);

            return true;
        }
    };
}
