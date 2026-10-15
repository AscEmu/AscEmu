/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Storage/MySQLDataStore.hpp"
#include "Management/QuestProperties.hpp"
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
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
                return m_minimum_size;
            else if (m_protocol.isMop())
                return 9; // uint32 rewardSlot + uint32 questId + packed guid mask byte
            return 0;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                WoWGuid128 clientGuid;
                packet >> clientGuid;
                questgiverGuid = WoWGuid::fromGuid128(clientGuid);
                packet >> questId;

                // the client names the chosen item (9.x and later: as reward choice with loot type), the
                // handler takes the slot of the item in the choices of the quest
                uint32_t itemId = 0;
                if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA())
                {
                    packet >> itemId;
                }
                else
                {
                    packet.resetBitPos();
                    packet.readBits(2);             // loot type
                    if (m_protocol.isTheWarWithinOrLater())
                        packet.readBit();           // context flags
                    packet >> itemId;
                }

                rewardSlot = itemId == 0 ? 0 : 6;
                if (itemId != 0)
                {
                    if (QuestProperties const* quest = sMySQLStore.getQuestProperties(questId))
                    {
                        for (uint32_t i = 0; i < 6; ++i)
                        {
                            if (quest->reward_choiceitem[i] == itemId)
                            {
                                rewardSlot = i;
                                break;
                            }
                        }
                    }
                }
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
