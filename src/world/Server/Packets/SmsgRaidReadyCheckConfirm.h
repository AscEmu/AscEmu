/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgRaidReadyCheckConfirm : public ManagedPacket
    {
    public:
        WoWGuid groupGuid;
        WoWGuid playerGuid;
        bool status;

        SmsgRaidReadyCheckConfirm() : SmsgRaidReadyCheckConfirm(0, 0, false)
        {
        }

        SmsgRaidReadyCheckConfirm(uint64_t groupGuid, uint64_t playerGuid, bool status) :
            ManagedPacket(SMSG_RAID_READY_CHECK_CONFIRM, 18),
            groupGuid(groupGuid),
            playerGuid(playerGuid),
            status(status)
        {
        }

    protected:
        size_t expectedSize() const override { return 18; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
                return false;

            packet.writeBit(groupGuid[4]);
            packet.writeBit(playerGuid[5]);
            packet.writeBit(playerGuid[3]);
            packet.writeBit(status);
            packet.writeBit(groupGuid[2]);
            packet.writeBit(playerGuid[6]);
            packet.writeBit(groupGuid[3]);
            packet.writeBit(playerGuid[0]);
            packet.writeBit(playerGuid[1]);
            packet.writeBit(groupGuid[1]);
            packet.writeBit(groupGuid[5]);
            packet.writeBit(playerGuid[7]);
            packet.writeBit(playerGuid[4]);
            packet.writeBit(groupGuid[6]);
            packet.writeBit(playerGuid[2]);
            packet.writeBit(groupGuid[0]);
            packet.writeBit(groupGuid[7]);

            packet.flushBits();

            packet.writeByteSeq(playerGuid[4]);
            packet.writeByteSeq(playerGuid[2]);
            packet.writeByteSeq(playerGuid[1]);
            packet.writeByteSeq(groupGuid[4]);
            packet.writeByteSeq(groupGuid[2]);
            packet.writeByteSeq(playerGuid[0]);
            packet.writeByteSeq(groupGuid[5]);
            packet.writeByteSeq(groupGuid[3]);
            packet.writeByteSeq(playerGuid[7]);
            packet.writeByteSeq(groupGuid[6]);
            packet.writeByteSeq(groupGuid[1]);
            packet.writeByteSeq(playerGuid[6]);
            packet.writeByteSeq(playerGuid[3]);
            packet.writeByteSeq(playerGuid[5]);
            packet.writeByteSeq(groupGuid[0]);
            packet.writeByteSeq(groupGuid[7]);

            return true;
        }
    };
}
