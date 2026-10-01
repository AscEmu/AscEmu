/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "QuestPacketCommon.h"
#include "Server/World.h"
#include "WoWGuid.hpp"

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
        uint16_t mapId = 0;
        uint32_t questGiverCreatureId = 0;
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
        uint32_t rewardCurrencyId[4] = {};
        uint32_t rewardCurrencyCount[4] = {};
        uint32_t rewardRepFaction[5] = {};
        int32_t rewardRepValue[5] = {};
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
            if (m_protocol.isForever())
            {
                // Forever 1.60.1.70124 uses the modern QuestGiverOfferRewardMessage layout.
                // Opcode 0x00660014 is sniff-verified for quest 783. The QuestRewards block
                // intentionally matches the already working Forever QuestDetails serializer.
                const WoWGuid questGiverGuid = WoWGuid::createModernFromLegacy(input.questGiverGuid, worldConfig.battleNetComm.realmId, input.mapId, 0);
                const auto packedQuestGiverGuid = questGiverGuid.packModern();

                // QuestRewards.Items[4]
                for (const auto& item : input.rewardItems)
                {
                    packet << static_cast<int32_t>(item.itemId);
                    packet << static_cast<int32_t>(item.count);
                    packet.writeBit(false); // ContextFlags absent
                    packet.flushBits();
                }

                // QuestRewards.Currencies[4] - not modelled by this legacy input yet.
                for (uint8_t i = 0; i < 4; ++i)
                {
                    packet << static_cast<int32_t>(input.rewardCurrencyId[i]); // CurrencyID
                    packet << static_cast<int32_t>(input.rewardCurrencyCount[i]); // CurrencyQty
                    packet << int32_t(0); // BonusQty
                    packet.writeBit(false); // ContextFlags absent
                    packet.flushBits();
                }

                packet << static_cast<int32_t>(input.countRewardChoiceItem);

                // QuestRewards.ChoiceItems[6]
                for (const auto& item : input.rewardChoiceItems)
                {
                    packet.writeBits(0U, 2); // LootItemType
                    packet.writeBit(false);  // ContextFlags absent
                    packet << static_cast<int32_t>(item.itemId);
                    packet.writeBits(0U, 7); // ItemModList count
                    packet.flushBits();
                    packet.writeBit(false);  // ItemBonus absent
                    packet.flushBits();
                    packet << static_cast<int32_t>(item.count);
                }

                packet << static_cast<int32_t>(input.countRewardItem);
                packet << static_cast<int32_t>(input.rewardMoney);
                packet << static_cast<int32_t>(input.xp);
                packet << int64_t(0); // ArtifactXP
                packet << int32_t(0); // ArtifactCategoryID
                packet << static_cast<int32_t>(input.bonusHonor);
                packet << static_cast<int32_t>(input.rewardTitleId);
                packet << int32_t(1); // FactionFlags; sniff-verified 70124 quest reward block

                for (uint8_t i = 0; i < 5; ++i)
                {
                    packet << static_cast<int32_t>(input.rewardRepFaction[i]);
                    packet << static_cast<int32_t>(input.rewardRepValue[i]);
                    packet << int32_t(0); // FactionOverride
                    packet << int32_t(7); // FactionCapIn; 70124 sends Exalted cap for all five slots
                }

                // SpellCompletionDisplayID[3]
                packet << static_cast<int32_t>(input.rewardSpell);
                packet << int32_t(0);
                packet << int32_t(0);
                packet << static_cast<int32_t>(input.effectOnPlayer); // SpellCompletionID
                packet << int32_t(0); // SkillLineID
                packet << int32_t(0); // NumSkillUps
                packet << uint32_t(0); // TreasurePickerID count
                packet.writeBit(false); // IsBoostSpell
                packet.flushBits();

                // QuestGiverOfferReward
                packet << static_cast<int32_t>(input.completionEmotes.size());
                packet.append(packedQuestGiverGuid.data(), packedQuestGiverGuid.size());

                packet << static_cast<uint32_t>(input.questFlags); // QuestFlags[0]
                packet << uint32_t(0);                             // QuestFlags[1]
                packet << uint32_t(0);                             // QuestFlags[2]
                packet << uint32_t(0);                             // QuestFlags[3]

                packet << static_cast<int32_t>(input.questGiverCreatureId);
                packet << static_cast<int32_t>(input.questId);
                packet << static_cast<int32_t>(input.suggestedPlayers);
                packet << int32_t(0); // QuestInfoID

                for (const auto& emote : input.completionEmotes)
                {
                    packet << static_cast<int32_t>(emote.emote);
                    packet << static_cast<uint32_t>(emote.delay);
                }

                packet.writeBit(false); // AutoLaunched
                packet.writeBit(false); // Unused
                packet.writeBit(false); // ResetByScheduler
                packet.flushBits();

                // QuestGiverOfferRewardMessage
                packet << int32_t(0); // QuestPackageID
                packet << int32_t(0); // PortraitGiver
                packet << int32_t(0); // PortraitGiverMount
                packet << int32_t(0); // PortraitGiverModelSceneID
                packet << int32_t(0); // PortraitTurnIn
                packet << static_cast<int32_t>(input.questGiverCreatureId);
                packet << uint32_t(0); // ConditionalRewardText count

                packet.writeBits(static_cast<uint32_t>(input.title.size()), 9);
                packet.writeBits(static_cast<uint32_t>(input.completionText.size()), 12);
                packet.writeBits(0U, 10); // PortraitGiverText
                packet.writeBits(0U, 8);  // PortraitGiverName
                packet.writeBits(0U, 10); // PortraitTurnInText
                packet.writeBits(0U, 8);  // PortraitTurnInName
                packet.flushBits();

                packet.writeString(input.title);
                packet.writeString(input.completionText);

                return true;
            }

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
