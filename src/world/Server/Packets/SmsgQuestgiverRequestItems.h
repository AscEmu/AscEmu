/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "QuestPacketCommon.h"

#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    // Populated by QuestMgr, which owns the locale/DB lookups (sMySQLStore.getLocalizedQuest,
    // getItemProperties) and the QuestStatus -> emote/text resolution.
    struct QuestgiverRequestItemsInput
    {
        uint64_t questGiverGuid = 0;
        uint32_t questId = 0;
        std::string title;
        std::string requestItemsText;
        bool isNotFinished = false; // status == QuestStatus::NotFinished
        uint32_t statusEmote = 0;   // incompleteemote or completeemote, already picked by isNotFinished
        uint32_t questFlags = 0;
        uint32_t suggestedPlayers = 0;
        uint32_t requiredMoney = 0;
        uint32_t countRequiredItem = 0;
        QuestRewardItemEntry requiredItems[6]; // MAX_REQUIRED_QUEST_ITEM
    };

    class SmsgQuestgiverRequestItems : public ManagedPacket
    {
    public:
        QuestgiverRequestItemsInput input;

        SmsgQuestgiverRequestItems() : SmsgQuestgiverRequestItems(QuestgiverRequestItemsInput{})
        {
        }

        explicit SmsgQuestgiverRequestItems(QuestgiverRequestItemsInput input) :
            ManagedPacket(SMSG_QUESTGIVER_REQUEST_ITEMS, 0),
            input(std::move(input))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 100;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                // Fields AscEmu's QuestProperties doesn't model (currency objectives, ender NPC/GO entry,
                // the offer-reward emote delay used here, Flags2) are written as 0.
                const WoWGuid questGiverGuid(input.questGiverGuid);
                const bool canComplete = !input.isNotFinished;

                uint32_t itemCounter = 0;
                for (const auto& item : input.requiredItems)
                    if (item.itemId != 0)
                        ++itemCounter;

                packet << input.suggestedPlayers;
                packet << input.questFlags;
                packet << uint32_t(0); // OfferRewardEmoteDelay[0] - not modelled
                packet << uint32_t(canComplete ? 0x5F : 0x5B); // status flags
                packet << input.requiredMoney;
                packet << uint32_t(0); // QuestTakerEntry (ender NPC/GO entry) - not modelled
                packet << uint32_t(0); // Flags2 - not modelled
                packet << input.statusEmote;
                packet << input.questId;

                packet.writeBits(0, 21); // currency objective count - not modelled by AscEmu
                packet.writeBit(true); // closeOnCancel

                packet.writeBit(questGiverGuid[2]);
                packet.writeBit(questGiverGuid[5]);
                packet.writeBit(questGiverGuid[1]);

                packet.writeBits(input.title.length(), 9);
                packet.writeBits(input.requestItemsText.length(), 12);

                packet.writeBit(questGiverGuid[6]);
                packet.writeBit(questGiverGuid[0]);

                packet.writeBits(itemCounter, 20);

                packet.writeBit(questGiverGuid[4]);
                packet.writeBit(questGiverGuid[7]);
                packet.writeBit(questGiverGuid[3]);

                packet.flushBits();

                packet.writeByteSeq(questGiverGuid[0]);
                packet.writeByteSeq(questGiverGuid[2]);

                packet.writeString(input.title);

                // currency objective data - not modelled by AscEmu, nothing to append

                for (const auto& item : input.requiredItems)
                {
                    if (!item.itemId)
                        continue;

                    packet << item.displayId;
                    packet << item.itemId;
                    packet << item.count;
                }

                packet.writeByteSeq(questGiverGuid[3]);
                packet.writeByteSeq(questGiverGuid[1]);

                packet.writeString(input.requestItemsText);

                packet.writeByteSeq(questGiverGuid[4]);
                packet.writeByteSeq(questGiverGuid[5]);
                packet.writeByteSeq(questGiverGuid[7]);
                packet.writeByteSeq(questGiverGuid[6]);

                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << input.questGiverGuid;
                packet << input.questId;

                packet << input.title;
                packet << input.requestItemsText;

                if (m_protocol.expansion < WoW::Expansion::_WotLK)
                {
                    packet << input.statusEmote;
                    packet << uint32_t(1);
                    packet << input.questFlags;
                    packet << input.suggestedPlayers;
                    packet << input.requiredMoney;
                }
                else
                {
                    packet << uint32_t(0);
                    packet << input.statusEmote;
                    packet << uint32_t(0);
                    packet << input.questFlags;
                    packet << input.suggestedPlayers;
                    packet << input.requiredMoney; // Required Money
                }

                // item count
                packet << uint32_t(input.countRequiredItem);

                // (loop for each item)
                for (const auto& item : input.requiredItems)
                {
                    if (item.itemId != 0)
                    {
                        packet << item.itemId;
                        packet << item.count;
                        packet << item.displayId;
                    }
                    else
                    {
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                        packet << uint32_t(0);
                    }
                }

                // wtf is this?
                if (input.isNotFinished)
                    packet << uint32_t(0); //incomplete button
                else
                    packet << uint32_t(3);

                if (m_protocol.expansion > WoW::Expansion::_TBC)
                    packet << uint32_t(4);

                packet << uint32_t(8);
                packet << uint32_t(10);

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                packet << uint64_t(input.questGiverGuid);
                packet << uint32_t(input.questId);

                packet << input.title;
                packet << input.requestItemsText;

                packet << uint32_t(0);
                packet << input.statusEmote;

                packet << uint32_t(1);                                                   // close on cancel
                packet << uint32_t(input.questFlags);
                packet << uint32_t(input.suggestedPlayers);

                packet << input.requiredMoney;      // Required Money

                packet << uint32_t(input.countRequiredItem);                            // item count

                // (loop for each item)
                for (const auto& item : input.requiredItems)
                {
                    if (!item.itemId)
                        continue;

                    packet << uint32_t(item.itemId);
                    packet << uint32_t(item.count);
                    packet << uint32_t(item.displayId);
                }

                packet << uint32_t(0);                                                   // required currency count


                if (input.isNotFinished)
                    packet << uint32_t(0);                                               // incomplete button
                else
                    packet << uint32_t(2);

                packet << uint32_t(4);
                packet << uint32_t(8);
                packet << uint32_t(16);
                packet << uint32_t(64);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
