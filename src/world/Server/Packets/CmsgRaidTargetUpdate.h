/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgRaidTargetUpdate : public ManagedPacket
    {
    public:
        uint8_t symbol = 0;
        uint8_t index = 0;
        WoWGuid targetGuid;

        CmsgRaidTargetUpdate() : ManagedPacket(CMSG_RAID_TARGET_UPDATE, 2)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
                return false;

            packet >> symbol >> index;

            if (symbol == 0xFF)
                return true;

            targetGuid[3] = packet.readBit();
            targetGuid[2] = packet.readBit();
            targetGuid[1] = packet.readBit();
            targetGuid[5] = packet.readBit();
            targetGuid[0] = packet.readBit();
            targetGuid[6] = packet.readBit();
            targetGuid[7] = packet.readBit();
            targetGuid[4] = packet.readBit();

            packet.readByteSeq(targetGuid[2]);
            packet.readByteSeq(targetGuid[3]);
            packet.readByteSeq(targetGuid[0]);
            packet.readByteSeq(targetGuid[7]);
            packet.readByteSeq(targetGuid[5]);
            packet.readByteSeq(targetGuid[1]);
            packet.readByteSeq(targetGuid[6]);
            packet.readByteSeq(targetGuid[4]);

            return true;
        }
    };
}
