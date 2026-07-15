/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class MsgQuestPushResult : public ManagedPacket
    {
    public:
        uint64_t giverGuid;
        uint32_t questId;
        uint8_t pushResult;

        MsgQuestPushResult() : MsgQuestPushResult(0, 0, 0)
        {
        }

        MsgQuestPushResult(uint64_t giverGuid, uint32_t questId, uint8_t pushResult) :
            ManagedPacket(MSG_QUEST_PUSH_RESULT, 9),
            giverGuid(giverGuid),
            questId(questId),
            pushResult(pushResult)
        {
        }

    protected:
        size_t expectedSize() const override { return 9; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << giverGuid << pushResult;
            }
            else // Mop
            {
                WoWGuid guid(giverGuid);

                packet.writeBit(guid[3]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[2]);

                packet.writeByteSeq(guid[4]);

                packet << pushResult;

                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[0]);
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet >> giverGuid;
                if (packet.size() >= 13)
                    packet >> questId;

                packet >> pushResult;
                return true;
            }
            else
            {
                return false;
            }
        }
    };
}
