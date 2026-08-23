/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgQuestgiverAcceptQuest : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t questId;

        CmsgQuestgiverAcceptQuest() : CmsgQuestgiverAcceptQuest(0, 0)
        {
        }

        CmsgQuestgiverAcceptQuest(uint64_t guid, uint32_t questId) :
            ManagedPacket(CMSG_QUESTGIVER_ACCEPT_QUEST, 12),
            guid(guid),
            questId(questId)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid >> questId;
                guid.init(unpackedGuid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> questId;

                guid[6] = packet.readBit();
                guid[0] = packet.readBit();
                packet.readBit();
                guid[2] = packet.readBit();
                guid[7] = packet.readBit();
                guid[5] = packet.readBit();
                guid[4] = packet.readBit();
                guid[3] = packet.readBit();
                guid[1] = packet.readBit();

                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[7]);
                return true;
            }

            return false;
        }
    };
}
