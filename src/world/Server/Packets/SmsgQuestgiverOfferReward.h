/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "QuestPacketCommon.h"

#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    // Populated by QuestMgr, which owns the locale/DB lookups (sMySQLStore.getLocalizedQuest,
    // getItemProperties) and the XP/money reward formulas (GenerateQuestXP/GenerateRewardMoney).
    struct QuestgiverOfferRewardInput
    {
        uint64_t questGiverGuid = 0;
        uint32_t questId = 0;
        std::string title;
        std::string completionText;
        bool hasNextQuest = false;
        uint32_t questFlags = 0;
        uint32_t suggestedPlayers = 0;
        std::vector<QuestEmoteEntry> completionEmotes;
        uint32_t countRewardChoiceItem = 0;
        QuestRewardItemEntry rewardChoiceItems[6];
        uint32_t countRewardItem = 0;    // pre-Cata reward-item count header
        uint32_t countRequiredItem = 0;  // Cata reward-item count header (yes, Cata reads count_required_item here)
        QuestRewardItemEntry rewardItems[4];
        uint32_t xp = 0;
        uint32_t bonusHonor = 0;
        uint32_t rewardSpell = 0;
        uint32_t effectOnPlayer = 0;
        uint32_t rewardTitleId = 0;
        uint32_t rewardTalents = 0;
        uint32_t bonusArenaPoints = 0;
        uint32_t rewardMoney = 0; // Cata only
        QuestEmoteEntry detailEmotes[4]; // Cata only, always 4 entries
    };

    class SmsgQuestgiverOfferReward : public ManagedPacket
    {
    public:
        QuestgiverOfferRewardInput input;

        SmsgQuestgiverOfferReward() : SmsgQuestgiverOfferReward(QuestgiverOfferRewardInput{})
        {
        }

        explicit SmsgQuestgiverOfferReward(QuestgiverOfferRewardInput input) :
            ManagedPacket(SMSG_QUESTGIVER_OFFER_REWARD, 0),
            input(std::move(input))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 50;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                // Fields AscEmu's QuestProperties doesn't model (currency rewards, reward package id,
                // reputation reward arrays, skill reward, portraits, ender NPC/GO entry, the 4.x text
                // windows) are written as 0/empty.
                const WoWGuid questGiverGuid(input.questGiverGuid);

                packet << input.rewardItems[2].count;
                packet << input.questId;
                packet << input.rewardItems[3].itemId;
                packet << input.rewardChoiceItems[2].displayId;

                for (uint8_t i = 0; i < 5; ++i) // QUEST_REPUTATIONS_COUNT - not modelled by AscEmu
                {
                    packet << uint32_t(0); // RewardFactionId
                    packet << uint32_t(0); // RewardFactionValueId
                    packet << uint32_t(0); // RewardFactionValueIdOverride
                }

                packet << input.rewardItems[0].count;
                packet << input.rewardItems[3].count;
                packet << input.rewardItems[3].displayId;
                packet << input.rewardItems[1].itemId;
                packet << input.rewardChoiceItems[3].itemId;
                packet << input.rewardChoiceItems[3].displayId;
                packet << uint32_t(input.countRewardChoiceItem);
                packet << input.effectOnPlayer; // RewSpellCast
                packet << input.rewardItems[1].displayId;
                packet << input.rewardChoiceItems[5].count;
                packet << input.rewardChoiceItems[4].displayId;
                packet << input.rewardChoiceItems[1].count;
                packet << input.rewardChoiceItems[0].displayId;
                packet << input.rewardItems[0].displayId;
                packet << uint32_t(0); // RewardPackageItemId - not modelled
                packet << uint32_t(0); // QuestTurnInPortrait - not modelled
                packet << input.rewardItems[1].count;
                packet << uint32_t(0); // RewardReputationMask - not modelled
                packet << input.rewardChoiceItems[0].itemId;
                packet << input.rewardChoiceItems[3].count;
                packet << input.rewardChoiceItems[4].count;
                packet << input.rewardChoiceItems[1].itemId;
                packet << input.rewardTalents;
                packet << uint32_t(0); // RewardSkillId - not modelled

                for (uint8_t i = 0; i < 4; ++i) // QUEST_REWARD_CURRENCY_COUNT - not modelled by AscEmu
                {
                    packet << uint32_t(0); // RewardCurrencyId
                    packet << uint32_t(0); // RewardCurrencyCount
                }

                packet << input.questFlags;
                packet << uint32_t(0); // Flags2 - not modelled
                packet << input.xp;
                packet << input.rewardTitleId;
                packet << input.rewardChoiceItems[2].itemId;
                packet << uint32_t(input.countRewardItem);
                packet << input.suggestedPlayers;
                packet << input.rewardChoiceItems[4].itemId;
                packet << uint32_t(0); // QuestTakerEntry (ender NPC/GO entry) - not modelled
                packet << input.rewardItems[2].itemId;
                packet << input.rewardChoiceItems[0].count;
                packet << input.rewardChoiceItems[5].displayId;
                packet << uint32_t(0); // QuestGiverPortrait - not modelled
                packet << input.rewardMoney;
                packet << input.rewardChoiceItems[5].itemId;
                packet << input.rewardChoiceItems[1].displayId;
                packet << input.rewardChoiceItems[2].count;
                packet << input.rewardItems[2].displayId;
                packet << input.rewardSpell;
                packet << input.rewardItems[0].itemId;
                packet << uint32_t(0); // RewardSkillPoints - not modelled

                packet.writeBits(0, 10); // questTurnTextWindow - not modelled, empty
                packet.writeBits(0, 8); // questGiverTargetName - not modelled, empty

                packet.writeBit(questGiverGuid[6]);

                packet.writeBits(input.completionEmotes.size(), 21);

                packet.writeBit(questGiverGuid[3]);
                packet.writeBit(questGiverGuid[7]);

                packet.writeBits(input.title.length(), 9);

                packet.writeBit(questGiverGuid[4]);

                packet.writeBits(0, 8); // questTurnTargetName - not modelled, empty
                packet.writeBits(0, 10); // questGiverTextWindow - not modelled, empty
                packet.writeBits(input.completionText.length(), 12);

                packet.writeBit(questGiverGuid[1]);
                packet.writeBit(questGiverGuid[2]);
                packet.writeBit(questGiverGuid[0]);
                packet.writeBit(questGiverGuid[5]);

                packet.writeBit(input.hasNextQuest);

                packet.flushBits();

                packet.writeString(""); // questGiverTargetName
                packet.writeString(input.title);

                for (const auto& emote : input.completionEmotes)
                {
                    packet << emote.delay;
                    packet << emote.emote;
                }

                packet.writeByteSeq(questGiverGuid[2]);

                packet.writeString(input.completionText);
                packet.writeString(""); // questTurnTextWindow
                packet.writeString(""); // questTurnTargetName

                packet.writeByteSeq(questGiverGuid[5]);
                packet.writeByteSeq(questGiverGuid[1]);

                packet.writeString(""); // questGiverTextWindow

                packet.writeByteSeq(questGiverGuid[0]);
                packet.writeByteSeq(questGiverGuid[7]);
                packet.writeByteSeq(questGiverGuid[6]);
                packet.writeByteSeq(questGiverGuid[4]);
                packet.writeByteSeq(questGiverGuid[3]);

                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << uint64_t(input.questGiverGuid);
                packet << uint32_t(input.questId);

                packet << input.title;
                packet << input.completionText;

                packet << (input.hasNextQuest ? uint8_t(1) : uint8_t(0));  // next quest shit
                packet << input.questFlags;
                packet << input.suggestedPlayers;

                packet << uint32_t(input.completionEmotes.size());
                for (const auto& emote : input.completionEmotes)
                {
                    packet << emote.emote;
                    packet << emote.delay;
                }

                packet << uint32_t(input.countRewardChoiceItem);
                if (input.countRewardChoiceItem)
                {
                    for (const auto& item : input.rewardChoiceItems)
                    {
                        if (item.itemId)
                        {
                            packet << item.itemId;
                            packet << item.count;
                            packet << item.displayId;
                        }
                    }
                }

                packet << uint32_t(input.countRewardItem);
                if (input.countRewardItem)
                {
                    for (const auto& item : input.rewardItems)
                    {
                        if (item.itemId)
                        {
                            packet << item.itemId;
                            packet << item.count;
                            packet << item.displayId;
                        }
                    }
                }

                packet << uint32_t(0);
                packet << uint32_t(input.xp); //VLack: The quest will give you this amount of XP

                packet << (input.bonusHonor * 10);
                packet << float(0);
                packet << uint32_t(0);
                packet << input.rewardSpell;
                packet << input.effectOnPlayer;
                packet << input.rewardTitleId;
                packet << input.rewardTalents;
                packet << input.bonusArenaPoints;
                packet << uint32_t(0);

                for (uint8_t i = 0; i < 5; ++i)              // reward factions ids
                    packet << uint32_t(0);

                for (uint8_t i = 0; i < 5; ++i)              // columnid in QuestFactionReward.dbc (zero based)?
                    packet << uint32_t(0);

                for (uint8_t i = 0; i < 5; ++i)              // reward reputation override?
                    packet << uint32_t(0);

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                std::string questGiverTextWindow;
                std::string questGiverTargetName;
                std::string questTurnTextWindow;
                std::string questTurnTargetName;

                packet << uint64_t(input.questGiverGuid);
                packet << uint32_t(input.questId);

                packet << input.title;
                packet << input.completionText;

                packet << questGiverTextWindow;
                packet << questGiverTargetName;
                packet << questTurnTextWindow;
                packet << questTurnTargetName;

                packet << uint32_t(0);                                                   // giver portrait
                packet << uint32_t(0);                                                   // turn in portrait

                packet << uint8_t(input.hasNextQuest ? 1 : 0);
                packet << uint32_t(input.questFlags);
                packet << uint32_t(input.suggestedPlayers);

                packet << uint32_t(input.completionEmotes.size());
                for (const auto& emote : input.completionEmotes)
                {
                    packet << uint32_t(emote.emote);
                    packet << uint32_t(emote.delay);
                }

                packet << uint32_t(input.countRewardChoiceItem);
                for (const auto& item : input.rewardChoiceItems)
                    packet << uint32_t(item.itemId);

                for (const auto& item : input.rewardChoiceItems)
                    packet << uint32_t(item.count);

                for (const auto& item : input.rewardChoiceItems)
                    packet << uint32_t(item.displayId);

                packet << uint32_t(input.countRequiredItem);
                for (const auto& item : input.rewardItems)
                    packet << uint32_t(item.itemId);

                for (const auto& item : input.rewardItems)
                    packet << uint32_t(item.count);

                for (const auto& item : input.rewardItems)
                    packet << uint32_t(item.displayId);

                packet << uint32_t(input.rewardMoney);            // Money reward
                packet << uint32_t(input.xp);

                packet << uint32_t(input.rewardTitleId);
                packet << uint32_t(0);                                                   // Honor reward
                packet << float(0.0f);                                                   // New 3.3
                packet << uint32_t(0);                                                   // reward talent
                packet << uint32_t(0);                                                   // unk
                packet << uint32_t(0);                                                   // reputationmask

                for (uint8_t i = 0; i < 5; ++i)
                    packet << uint32_t(0);

                for (uint8_t i = 0; i < 5; ++i)
                    packet << int32_t(0);

                for (uint8_t i = 0; i < 5; ++i)
                    packet << uint32_t(0);

                packet << uint32_t(0);                                                   // reward spell
                packet << uint32_t(0);                                                   // reward spell cast

                for (uint8_t i = 0; i < 4; ++i)
                    packet << uint32_t(0);

                for (uint8_t i = 0; i < 4; ++i)
                    packet << uint32_t(0);

                packet << uint32_t(0);                                                   // rewskill
                packet << uint32_t(0);                                                   // rewskillpoint

                packet << uint32_t(4);                                                   // emote count
                for (const auto& emote : input.detailEmotes)
                {
                    packet << uint32_t(emote.emote);
                    packet << uint32_t(emote.delay);
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
