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
    struct QuestObjectiveSimpleEntry
    {
        int32_t id = 0;
        int32_t type = 0;
        int32_t objectId = 0;
        int32_t amount = 0;
    };

    // Populated by QuestMgr, which owns the locale/DB lookups (sMySQLStore.getLocalizedQuest,
    // getItemProperties) and the XP/money reward formulas (GenerateQuestXP/GenerateRewardMoney).
    struct QuestgiverQuestDetailsInput
    {
        uint64_t questGiverGuid = 0;
        uint64_t questSharerGuid = 0; // pre-Cata, >TBC only: qst_giver->isPlayer() ? guid : 0. Cata always sends 0.
        uint16_t mapId = 0;
        uint32_t questGiverCreatureId = 0;
        uint32_t questStartItemId = 0;
        uint32_t questId = 0;
        std::string title;
        std::string details;
        std::string objectives;
        uint32_t questFlags = 0;
        uint32_t suggestedPlayers = 0;
        uint32_t countRewardChoiceItem = 0;
        QuestRewardItemEntry rewardChoiceItems[6];
        uint32_t countRewardItem = 0;    // pre-Cata reward-item count header
        uint32_t countRequiredItem = 0;  // Cata reward-item count header (yes, Cata reads count_required_item here)
        QuestRewardItemEntry rewardItems[4];
        uint32_t rewardCurrencyId[4] = {};
        uint32_t rewardCurrencyCount[4] = {};
        uint32_t rewardRepFaction[5] = {};
        int32_t rewardRepValue[5] = {};
        uint32_t rewardMoney = 0;
        uint32_t xp = 0;         // Cata only - pre-Cata deliberately sends 0 here, not the real XP
        uint32_t bonusHonor = 0; // pre-Cata, >TBC only
        uint32_t rewardSpell = 0; // pre-Cata only - Cata always sends 0 here
        uint32_t effectOnPlayer = 0; // pre-Cata, >TBC only
        uint32_t rewardTitleId = 0;
        uint32_t rewardTalents = 0;
        uint32_t bonusArenaPoints = 0;
        uint32_t detailEmoteCount = 0; // pre-Cata, >TBC only: how many of detailEmotes to send
        QuestEmoteEntry detailEmotes[4];
        std::vector<QuestObjectiveSimpleEntry> objectiveEntries;
    };

    class SmsgQuestgiverQuestDetails : public ManagedPacket
    {
    public:
        QuestgiverQuestDetailsInput input;

        SmsgQuestgiverQuestDetails() : SmsgQuestgiverQuestDetails(QuestgiverQuestDetailsInput{})
        {
        }

        explicit SmsgQuestgiverQuestDetails(QuestgiverQuestDetailsInput input) :
            ManagedPacket(SMSG_QUESTGIVER_QUEST_DETAILS, 0),
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
            if (m_protocol.isForever())
            {
                // Forever 1.60.1.70009/70124 uses the modern QuestGiverQuestDetails layout.
                // The field order below is verified against the captured quest 33 packet
                // (Wolves Across the Border) instead of reusing the incompatible Cata layout.
                const WoWGuid questGiverGuid = WoWGuid::createModernFromLegacy(input.questGiverGuid, worldConfig.battleNetComm.realmId, input.mapId, 0);
                const WoWGuid informUnit = WoWGuid::createModernFromLegacy(input.questSharerGuid, worldConfig.battleNetComm.realmId, input.mapId, 0);
                const auto packedQuestGiverGuid = questGiverGuid.packModern();
                const auto packedInformUnit = informUnit.packModern();
                packet.append(packedQuestGiverGuid.data(), packedQuestGiverGuid.size());
                packet.append(packedInformUnit.data(), packedInformUnit.size());

                packet << static_cast<int32_t>(input.questId);
                packet << int32_t(0); // QuestPackageID
                packet << int32_t(0); // PortraitGiver
                packet << int32_t(0); // PortraitGiverMount
                packet << int32_t(0); // PortraitGiverModelSceneID
                packet << int32_t(0); // PortraitTurnIn

                packet << static_cast<uint32_t>(input.questFlags); // QuestFlags[0]
                packet << uint32_t(0);                             // QuestFlags[1]
                packet << uint32_t(0);                             // QuestFlags[2]
                packet << uint32_t(0);                             // QuestFlags[3]
                packet << static_cast<int32_t>(input.suggestedPlayers);
                packet << uint32_t(0); // LearnSpells count

                // QuestRewards.Items[4]
                for (const auto& item : input.rewardItems)
                {
                    packet << static_cast<int32_t>(item.itemId);
                    packet << static_cast<int32_t>(item.count);
                    packet.writeBit(false); // ContextFlags absent
                    packet.flushBits();
                }

                // QuestRewards.Currencies[4]
                for (uint8_t i = 0; i < 4; ++i)
                {
                    packet << static_cast<int32_t>(input.rewardCurrencyId[i]);
                    packet << static_cast<int32_t>(input.rewardCurrencyCount[i]);
                    packet << int32_t(0); // BonusQty
                    packet.writeBit(false); // ContextFlags absent
                    packet.flushBits();
                }

                packet << static_cast<int32_t>(input.countRewardChoiceItem);

                // QuestRewards.ChoiceItems[6]. AscEmu does not model item modifiers or
                // bonus lists here, so send a valid empty ItemInstance around the item id.
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

                packet << static_cast<int32_t>(input.rewardSpell);
                packet << int32_t(0);
                packet << int32_t(0); // SpellCompletionDisplayID[3]
                packet << static_cast<int32_t>(input.effectOnPlayer); // SpellCompletionID
                packet << int32_t(0); // SkillLineID
                packet << int32_t(0); // NumSkillUps
                packet << uint32_t(0); // TreasurePickerID count
                packet.writeBit(false); // IsBoostSpell
                packet.flushBits();

                packet << static_cast<uint32_t>(input.detailEmoteCount);
                packet << static_cast<uint32_t>(input.objectiveEntries.size()); // Objectives count
                packet << static_cast<int32_t>(input.questStartItemId);
                packet << int32_t(0);  // QuestInfoID
                packet << int32_t(0);  // QuestSessionBonus
                packet << static_cast<int32_t>(input.questGiverCreatureId);
                packet << uint32_t(0); // ConditionalDescriptionText count

                for (uint32_t i = 0; i < input.detailEmoteCount && i < 4; ++i)
                {
                    packet << static_cast<int32_t>(input.detailEmotes[i].emote);
                    packet << static_cast<uint32_t>(input.detailEmotes[i].delay);
                }

                for (const auto& objective : input.objectiveEntries)
                {
                    packet << objective.id;
                    packet << objective.type;
                    packet << objective.objectId;
                    packet << objective.amount;
                }

                packet.writeBits(static_cast<uint32_t>(input.title.size()), 9);
                packet.writeBits(static_cast<uint32_t>(input.details.size()), 12);
                packet.writeBits(static_cast<uint32_t>(input.objectives.size()), 12);
                packet.writeBits(0U, 10); // PortraitGiverText
                packet.writeBits(0U, 8);  // PortraitGiverName
                packet.writeBits(0U, 10); // PortraitTurnInText
                packet.writeBits(0U, 8);  // PortraitTurnInName
                packet.writeBit(false); // AutoLaunched
                packet.writeBit(false); // FromContentPush
                packet.writeBit(false); // ReplayQuest
                packet.writeBit(false); // ResetByScheduler
                packet.writeBit(false); // StartCheat
                packet.writeBit(false); // DisplayPopup
                packet.flushBits();

                packet.writeString(input.title);
                packet.writeString(input.details);
                packet.writeString(input.objectives);
                return true;
            }

            if (m_protocol.isMop())
            {
                // Fields AscEmu's QuestProperties doesn't model (currency rewards, reward package id,
                // reputation reward arrays, skill reward, portraits, quest objectives list, the extra
                // 4.x text windows) are written as 0/empty - the wire shape stays correct, those UI
                // elements just show nothing extra.
                const WoWGuid questGiverGuid(input.questGiverGuid);
                const WoWGuid dividerGuid(input.questSharerGuid);

                packet << input.rewardItems[3].count;
                packet << input.rewardChoiceItems[4].displayId;
                packet << input.rewardChoiceItems[2].itemId;

                for (uint8_t i = 0; i < 4; ++i) // QUEST_REWARD_CURRENCY_COUNT - not modelled by AscEmu
                {
                    packet << uint32_t(0); // RewardCurrencyCount
                    packet << uint32_t(0); // RewardCurrencyId
                }

                packet << uint32_t(input.countRewardChoiceItem);
                packet << input.rewardChoiceItems[2].count;
                packet << input.rewardItems[1].count;
                packet << input.rewardChoiceItems[5].displayId;
                packet << input.rewardItems[0].count;
                packet << input.rewardItems[3].displayId;
                packet << input.rewardChoiceItems[0].itemId;
                packet << input.rewardChoiceItems[3].count;
                packet << uint32_t(0); // QuestGiverPortrait - not modelled
                packet << input.rewardChoiceItems[3].displayId;
                packet << input.rewardItems[0].itemId;
                packet << input.questId;
                packet << input.suggestedPlayers;
                packet << input.rewardChoiceItems[0].displayId;
                packet << input.rewardChoiceItems[4].count;
                packet << input.rewardChoiceItems[5].count;
                packet << input.rewardTalents;
                packet << input.rewardChoiceItems[1].count;
                packet << input.rewardChoiceItems[2].displayId;

                for (uint8_t i = 0; i < 5; ++i) // QUEST_REPUTATIONS_COUNT - not modelled by AscEmu
                {
                    packet << uint32_t(0); // RewardFactionValueId
                    packet << uint32_t(0); // RewardFactionValueIdOverride
                    packet << uint32_t(0); // RewardFactionId
                }

                packet << input.rewardItems[3].itemId;
                packet << uint32_t(0); // RewardSkillId - not modelled
                packet << input.xp;
                packet << uint32_t(0); // RewardReputationMask - not modelled
                packet << input.rewardItems[2].displayId;
                packet << input.rewardItems[1].itemId;
                packet << input.rewardChoiceItems[1].itemId;
                packet << input.rewardChoiceItems[5].itemId;
                packet << input.rewardSpell;
                packet << input.questFlags;
                packet << input.rewardTitleId;
                packet << input.rewardItems[2].itemId;
                packet << input.rewardMoney;
                packet << input.rewardItems[2].count;
                packet << uint32_t(0); // Flags2 - not modelled
                packet << input.rewardSpell;
                packet << input.rewardChoiceItems[3].itemId;
                packet << uint32_t(input.countRewardItem);
                packet << uint32_t(0); // RewardSkillPoints - not modelled
                packet << input.rewardItems[0].displayId;
                packet << input.rewardChoiceItems[4].itemId;
                packet << uint32_t(0); // RewardPackageItemId - not modelled
                packet << input.rewardChoiceItems[0].count;
                packet << input.rewardItems[1].displayId;
                packet << input.rewardChoiceItems[1].displayId;
                packet << uint32_t(0); // QuestTurnInPortrait - not modelled

                packet.writeBit(dividerGuid[7]);
                packet.writeBit(questGiverGuid[1]);

                packet.writeBits(0, 8); // questTurnTargetName - not modelled, empty

                packet.writeBit(questGiverGuid[2]);

                packet.writeBits(0, 10); // questGiverTextWindow - not modelled, empty
                packet.writeBit(0); // displayPopup - no AscEmu caller sets this

                packet.writeBit(dividerGuid[2]);

                packet.writeBits(input.title.length(), 9);
                packet.writeBits(4, 21); // QUEST_EMOTE_COUNT

                packet.writeBit(dividerGuid[0]);
                packet.writeBit(questGiverGuid[6]);
                packet.writeBit(questGiverGuid[5]);

                packet.writeBits(0, 8); // questGiverTargetName - not modelled, empty

                packet.writeBit(questGiverGuid[3]);
                packet.writeBit(dividerGuid[1]);
                packet.writeBit(questGiverGuid[0]);

                packet.writeBit(0); // StartCheat

                packet.writeBit(questGiverGuid[4]);
                packet.writeBit(dividerGuid[3]);
                packet.writeBit(dividerGuid[5]);
                packet.writeBit(dividerGuid[4]);

                packet.writeBits(0, 10); // questTurnTextWindow - not modelled, empty
                packet.writeBit(1); // Activate accept

                packet.writeBit(dividerGuid[6]);
                packet.writeBit(questGiverGuid[7]);

                packet.writeBits(input.details.length(), 12);
                packet.writeBits(0, 22); // Learned Spells - not modelled
                packet.writeBits(0, 20); // quest objectives list - not modelled
                packet.writeBits(input.objectives.length(), 12);

                packet.flushBits();

                packet.writeByteSeq(dividerGuid[0]);

                packet.writeString(""); // questGiverTargetName
                packet.writeString(""); // questTurnTextWindow
                packet.writeString(input.title);

                packet.writeByteSeq(questGiverGuid[6]);

                packet.writeString(input.objectives);

                packet.writeByteSeq(dividerGuid[2]);

                packet.writeString(""); // questGiverTextWindow

                // quest objectives list data - not modelled, nothing to append

                packet.writeString(""); // questTurnTargetName
                packet.writeString(input.details);

                packet.writeByteSeq(dividerGuid[5]);
                packet.writeByteSeq(dividerGuid[7]);
                packet.writeByteSeq(questGiverGuid[7]);
                packet.writeByteSeq(questGiverGuid[3]);
                packet.writeByteSeq(questGiverGuid[0]);

                for (uint8_t i = 0; i < 4; ++i) // QUEST_EMOTE_COUNT
                {
                    packet << input.detailEmotes[i].delay;
                    packet << input.detailEmotes[i].emote;
                }

                packet.writeByteSeq(dividerGuid[4]);
                packet.writeByteSeq(dividerGuid[3]);
                packet.writeByteSeq(questGiverGuid[5]);
                packet.writeByteSeq(questGiverGuid[1]);
                packet.writeByteSeq(questGiverGuid[2]);
                packet.writeByteSeq(dividerGuid[1]);
                packet.writeByteSeq(dividerGuid[6]);
                packet.writeByteSeq(questGiverGuid[4]);

                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << uint64_t(input.questGiverGuid); // npc guid
                if (m_protocol.expansion > WoW::Expansion::_TBC)
                    packet << uint64_t(input.questSharerGuid); // (questsharer?) guid

                packet << input.questId; // quest id

                packet << input.title;
                packet << input.details;
                packet << input.objectives;

                if (m_protocol.expansion > WoW::Expansion::_TBC)
                {
                    packet << uint8_t(1);                    // Activate accept
                    packet << input.questFlags;
                    packet << input.suggestedPlayers;         // "Suggested players"
                    packet << uint8_t(0);                    // MANGOS: IsFinished? value is sent back to server in quest accept packet
                }
                else
                {
                    packet << uint32_t(1);                   // active quest
                    packet << input.suggestedPlayers;
                }

                packet << uint32_t(input.countRewardChoiceItem);

                for (const auto& item : input.rewardChoiceItems)
                {
                    if (!item.itemId)
                        continue;

                    packet << item.itemId;
                    packet << item.count;
                    packet << item.displayId;
                }

                packet << uint32_t(input.countRewardItem);

                for (const auto& item : input.rewardItems)
                {
                    if (!item.itemId)
                        continue;

                    packet << item.itemId;
                    packet << item.count;
                    packet << item.displayId;
                }

                packet << input.rewardMoney;     // Money reward

                if (m_protocol.expansion > WoW::Expansion::_TBC)
                {
                    packet << uint32_t(0);                       // New 3.3 - this is the XP you'll see on the quest reward panel too, but I think it is fine not to show it, because it can change if the player levels up before completing the quest.
                    packet << (input.bonusHonor * 10);            // Honor reward
                    packet << float(0);                          // New 3.3
                }

                packet << input.rewardSpell;                 // this is the spell (id) the quest finisher teaches you, or the icon of the spell if effect_on_player is not 0

                if (m_protocol.expansion > WoW::Expansion::_TBC)
                {
                    packet << input.effectOnPlayer;             // this is the spell (id) the quest finisher casts on you as a reward
                    packet << input.rewardTitleId;                // Title reward (ID)
                    packet << input.rewardTalents;                // Talent reward
                    packet << input.bonusArenaPoints;             // Arena Points reward
                    packet << input.xp;                           // new 3.3.0 (always 0 here, see struct comment)

                    for (uint8_t i = 0; i < 5; ++i)
                        packet << uint32_t(0);

                    for (uint8_t i = 0; i < 5; ++i)
                        packet << uint32_t(0);

                    for (uint8_t i = 0; i < 5; ++i)
                        packet << uint32_t(0);

                    packet << input.detailEmoteCount;             // Amount of emotes (4?)

                    for (uint32_t i = 0; i < input.detailEmoteCount && i < 4; i++)
                    {
                        packet << input.detailEmotes[i].emote;           // Emote ID
                        packet << input.detailEmotes[i].delay;      // Emote Delay
                    }
                }
                else
                {
                    packet << uint32_t(0);                       //unk
                    packet << uint32_t(0);                       //unk
                    packet << uint32_t(0);                       //reward pvp title
                    packet << uint32_t(1);                       //emotecount
                    packet << uint32_t(1);                       // EMOTE_ONESHOT_TALK
                    packet << uint32_t(0);                       // emote delay
                }

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                std::string questGiverTextWindow;
                std::string questGiverTargetName;
                std::string questTurnTextWindow;
                std::string questTurnTargetName;

                packet << uint64_t(input.questGiverGuid);                                // npc guid
                packet << uint64_t(0);                                                   // (questsharer?) guid
                packet << uint32_t(input.questId);

                packet << input.title;
                packet << input.details;
                packet << input.objectives;

                packet << questGiverTextWindow;                                          // 4.x
                packet << questGiverTargetName;                                          // 4.x
                packet << questTurnTextWindow;                                           // 4.x
                packet << questTurnTargetName;                                           // 4.x

                packet << uint32_t(0);                                                   // 4.x - qgportait
                packet << uint32_t(0);                                                   // 4.x - qgturninportrait

                packet << uint8_t(1);                                                    // Activate accept

                packet << uint32_t(input.questFlags);
                packet << uint32_t(input.suggestedPlayers);

                packet << uint8_t(0);                                                    // finished? value is sent back to server in quest accept packet
                packet << uint8_t(0);                                                    // 4.x Starts at AreaTrigger
                packet << uint32_t(0);                                                   // required spell

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

                packet << uint32_t(input.rewardMoney);                                  // Money reward
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
                    packet << emote.emote;
                    packet << emote.delay;
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
