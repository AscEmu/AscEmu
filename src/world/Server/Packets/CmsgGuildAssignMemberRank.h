/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGuildAssignMemberRank : public ManagedPacket
    {
    public:
        uint32_t rankId = 0;
        WoWGuid targetGuid;
        WoWGuid setterGuid;

        CmsgGuildAssignMemberRank() : ManagedPacket(CMSG_GUILD_ASSIGN_MEMBER_RANK, 20)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet >> rankId;

                targetGuid[1] = packet.readBit();
                targetGuid[7] = packet.readBit();
                setterGuid[4] = packet.readBit();
                setterGuid[2] = packet.readBit();
                targetGuid[4] = packet.readBit();
                targetGuid[5] = packet.readBit();
                targetGuid[6] = packet.readBit();
                setterGuid[1] = packet.readBit();
                setterGuid[7] = packet.readBit();
                targetGuid[2] = packet.readBit();
                targetGuid[3] = packet.readBit();
                targetGuid[0] = packet.readBit();
                setterGuid[6] = packet.readBit();
                setterGuid[3] = packet.readBit();
                setterGuid[0] = packet.readBit();
                setterGuid[5] = packet.readBit();

                packet.readByteSeq(targetGuid[0]);
                packet.readByteSeq(setterGuid[1]);
                packet.readByteSeq(setterGuid[3]);
                packet.readByteSeq(setterGuid[5]);
                packet.readByteSeq(targetGuid[7]);
                packet.readByteSeq(targetGuid[3]);
                packet.readByteSeq(setterGuid[0]);
                packet.readByteSeq(targetGuid[1]);
                packet.readByteSeq(setterGuid[6]);
                packet.readByteSeq(targetGuid[2]);
                packet.readByteSeq(targetGuid[5]);
                packet.readByteSeq(targetGuid[4]);
                packet.readByteSeq(setterGuid[2]);
                packet.readByteSeq(setterGuid[4]);
                packet.readByteSeq(targetGuid[6]);
                packet.readByteSeq(setterGuid[7]);

                return true;
            }
            else if (m_protocol.isMop())
            {
                // Mop dropped the setter guid from the wire - the server infers it as the
                // calling player's own guid instead (see handleGuildAssignRankOpcode).
                packet >> rankId;

                targetGuid[2] = packet.readBit();
                targetGuid[3] = packet.readBit();
                targetGuid[1] = packet.readBit();
                targetGuid[6] = packet.readBit();
                targetGuid[0] = packet.readBit();
                targetGuid[4] = packet.readBit();
                targetGuid[7] = packet.readBit();
                targetGuid[5] = packet.readBit();

                packet.readByteSeq(targetGuid[7]);
                packet.readByteSeq(targetGuid[3]);
                packet.readByteSeq(targetGuid[2]);
                packet.readByteSeq(targetGuid[5]);
                packet.readByteSeq(targetGuid[6]);
                packet.readByteSeq(targetGuid[0]);
                packet.readByteSeq(targetGuid[4]);
                packet.readByteSeq(targetGuid[1]);

                return true;
            }

            return false;
        }
    };
}
