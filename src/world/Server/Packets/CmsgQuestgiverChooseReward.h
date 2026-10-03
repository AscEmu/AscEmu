/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <cstring>

namespace AscEmu::Packets
{
    class CmsgQuestgiverChooseReward : public ManagedPacket
    {
    public:
        WoWGuid questgiverGuid;
        uint32_t questId;
        uint32_t rewardSlot;
        uint32_t rewardItemId;
        uint32_t rewardQuantity;
        uint8_t rewardItemType;

        CmsgQuestgiverChooseReward() : CmsgQuestgiverChooseReward(0, 0, 0)
        {
        }

        CmsgQuestgiverChooseReward(uint64_t questgiverGuid, uint32_t questId, uint32_t rewardSlot) :
            ManagedPacket(CMSG_QUESTGIVER_CHOOSE_REWARD, 12),
            questgiverGuid(questgiverGuid),
            questId(questId),
            rewardSlot(rewardSlot),
            rewardItemId(0),
            rewardQuantity(0),
            rewardItemType(0)
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
                return 16; // packed modern GUID + questId + modern QuestChoiceItem
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

                // Forever sends a modern QuestChoiceItem, not a legacy reward-slot index.
                // The simple item-choice form used by normal quest rewards is:
                // bits: LootItemType(2), ContextFlagsPresent(1); ItemID; ItemModListCount(7);
                // ItemBonusPresent(1); Quantity; optional ContextFlags.
                if (packet.remaining() < 11)
                    return false;

                const uint8_t* choice = packet.contents() + packet.rpos();
                const std::size_t choiceSize = packet.remaining();
                rewardItemType = static_cast<uint8_t>((choice[0] >> 6) & 0x03U);
                const bool hasContextFlags = (choice[0] & 0x20U) != 0;

                int32_t itemId = 0;
                std::memcpy(&itemId, choice + 1, sizeof(itemId));
                if (itemId < 0)
                    return false;
                rewardItemId = static_cast<uint32_t>(itemId);

                const uint32_t modificationCount = static_cast<uint32_t>(choice[5] >> 1);
                if (modificationCount != 0)
                    return false; // not emitted by AscEmu's current quest reward serializer

                const bool hasItemBonus = (choice[6] & 0x80U) != 0;
                if (hasItemBonus)
                    return false; // not emitted by AscEmu's current quest reward serializer

                const std::size_t quantityOffset = 7;
                const std::size_t expectedChoiceSize = quantityOffset + sizeof(int32_t) + (hasContextFlags ? sizeof(int32_t) : 0);
                if (choiceSize != expectedChoiceSize)
                    return false;

                int32_t quantity = 0;
                std::memcpy(&quantity, choice + quantityOffset, sizeof(quantity));
                if (quantity < 0)
                    return false;
                rewardQuantity = static_cast<uint32_t>(quantity);

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
