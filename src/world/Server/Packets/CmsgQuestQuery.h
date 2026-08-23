/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgQuestQuery : public ManagedPacket
    {
    public:
        uint32_t questId;

        CmsgQuestQuery() : CmsgQuestQuery(0)
        {
        }

        CmsgQuestQuery(uint32_t questId) :
            ManagedPacket(CMSG_QUEST_QUERY, 4),
            questId(questId)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet >> questId;
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> questId;

                // guid is sent but not used by the response handler
                WoWGuid guid;
                guid[0] = packet.readBit();
                guid[5] = packet.readBit();
                guid[2] = packet.readBit();
                guid[7] = packet.readBit();
                guid[6] = packet.readBit();
                guid[4] = packet.readBit();
                guid[1] = packet.readBit();
                guid[3] = packet.readBit();

                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[0]);
                return true;
            }

            return false;
        }
    };
}
