/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgQuestgiverChooseReward : public ManagedPacket
    {
    public:
        WoWGuid questgiverGuid;
        uint32_t questId;
        uint32_t rewardSlot;

        CmsgQuestgiverChooseReward() : CmsgQuestgiverChooseReward(0, 0, 0)
        {
        }

        CmsgQuestgiverChooseReward(uint64_t questgiverGuid, uint32_t questId, uint32_t rewardSlot) :
            ManagedPacket(CMSG_QUESTGIVER_CHOOSE_REWARD, 12),
            questgiverGuid(questgiverGuid),
            questId(questId),
            rewardSlot(rewardSlot)
        {
        }

        bool deserialise(WorldPacket& packet) override
        {
            if (packet.remaining() < expectedSize())
                return false;

            return internalDeserialise(packet);
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.isForever())
                return 16; // packed modern GUID + questId + empty QuestChoiceItem observed in 70009
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
                return m_minimum_size;
            else if (m_protocol.isMop())
                return 9; // uint32 rewardSlot + uint32 questId + packed guid mask byte
            return 0;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                WoWGuid modernGuid;
                std::size_t consumed = 0;
                if (!WoWGuid::unpackModern(packet.contents() + packet.rpos(), packet.remaining(), modernGuid, consumed))
                    return false;
                packet.rpos(packet.rpos() + consumed);
                questgiverGuid.init(modernGuid.toLegacyRaw());

                if (packet.remaining() < sizeof(uint32_t))
                    return false;
                packet >> questId;

                // Build 70009 capture had no selectable reward: QuestChoiceItem = 11 zero bytes.
                if (packet.remaining() != 11)
                    return false;
                for (std::size_t i = packet.rpos(); i < packet.size(); ++i)
                    if (packet.contents()[i] != 0)
                        return false;
                packet.rpos(packet.size());
                rewardSlot = 0;
                return true;
            }

            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid >> questId >> rewardSlot;
                questgiverGuid.init(unpackedGuid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                // reward is now an item id, not a slot index
                packet >> rewardSlot;
                packet >> questId;

                questgiverGuid[2] = packet.readBit();
                questgiverGuid[6] = packet.readBit();
                questgiverGuid[0] = packet.readBit();
                questgiverGuid[5] = packet.readBit();
                questgiverGuid[1] = packet.readBit();
                questgiverGuid[3] = packet.readBit();
                questgiverGuid[7] = packet.readBit();
                questgiverGuid[4] = packet.readBit();

                packet.readByteSeq(questgiverGuid[1]);
                packet.readByteSeq(questgiverGuid[2]);
                packet.readByteSeq(questgiverGuid[5]);
                packet.readByteSeq(questgiverGuid[7]);
                packet.readByteSeq(questgiverGuid[0]);
                packet.readByteSeq(questgiverGuid[3]);
                packet.readByteSeq(questgiverGuid[6]);
                packet.readByteSeq(questgiverGuid[4]);
                return true;
            }

            return false;
        }
    };
}
