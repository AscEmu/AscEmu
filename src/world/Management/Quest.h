/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WOWSERVER_QUEST_H
#define WOWSERVER_QUEST_H

#include "QuestDefines.hpp"

#define QUEST_REPEATABLE 1
#define QUEST_REPEATABLE_DAILY 2
#define MAX_REQUIRED_QUEST_ITEM 6
#define QUEST_REWARD_CURRENCY_COUNT 4
#define QUEST_REQUIRED_CURRENCY_COUNT 4

class QuestScript;

#pragma pack(push,1)

#define QUEST_SPECIAL_FLAG_DB_ALLOWED (QUEST_SPECIAL_FLAG_REPEATABLE | QUEST_SPECIAL_FLAG_EXPLORATION_OR_EVENT | QUEST_SPECIAL_FLAG_AUTO_ACCEPT | QUEST_SPECIAL_FLAG_DF_QUEST)

struct QuestProperties
{
    uint32 QuestId;
    uint32 QuestMethod;
    int32 ZoneOrSort;
    int32 SkillOrClassMask;
    uint32 MinLevel;
    uint32 MaxLevel;
    int32 QuestLevel;
    uint32 Type;
    uint32 RequiredRaces;
    uint32 RequiredSkillId;
    uint32 RequiredSkillValue;
    uint32 RepObjectiveFaction;
    int32 RepObjectiveValue;
    uint32 RepObjectiveFaction2;
    int32 RepObjectiveValue2;
    uint32 RequiredMinRepFaction;
    int32 RequiredMinRepValue;
    uint32 RequiredMaxRepFaction;
    int32 RequiredMaxRepValue;
    uint32 SuggestedPlayers;
    uint32 LimitTime;
    uint32 QuestFlags;
    uint32 SpecialFlags;
    uint32 CharTitleId;
    uint32 PlayersSlain;
    uint32 BonusTalents;
    int32 RewArenaPoints;
    int32 PrevQuestId;
    int32 NextQuestId;
    int32 ExclusiveGroup;
    uint32 NextQuestInChain;
    uint32 XPId;
    uint32 SrcItemId;
    uint32 SrcItemCount;
    uint32 SrcSpell;
    std::string Title;
    std::string Details;
    std::string Objectives;
    std::string OfferRewardText;
    std::string RequestItemsText;
    std::string EndText;
    std::string CompletedText;
    uint32 RewHonorAddition;
    float RewHonorMultiplier;
    int32 RewOrReqMoney;
    uint32 RewMoneyMaxLevel;
    uint32 RewSpell;
    int32 RewSpellCast;
    uint32 RewMailTemplateId;
    uint32 RewMailDelaySecs;
    uint32 PointMapId;
    float PointX;
    float PointY;
    uint32 PointOpt;
    uint32 IncompleteEmote;
    uint32 CompleteEmote;
    uint32 MinimapTargetMark;
    uint32 RewSkillLineId;
    uint32 RewSkillPoints;
    uint32 RewRepMask;
    uint32 QuestGiverPortrait;
    uint32 QuestTurnInPortrait;
    std::string ObjectiveText[QUEST_OBJECTIVES_COUNT];
    uint32 ReqItemId[QUEST_ITEM_OBJECTIVES_COUNT];
    uint32 ReqItemCount[QUEST_ITEM_OBJECTIVES_COUNT];
    uint32 ReqSourceId[QUEST_SOURCE_ITEM_IDS_COUNT];
    uint32 ReqSourceCount[QUEST_SOURCE_ITEM_IDS_COUNT];
    int32 ReqCreatureOrGOId[QUEST_OBJECTIVES_COUNT];
    uint32 ReqCreatureOrGOCount[QUEST_OBJECTIVES_COUNT];
    uint32 ReqSpell[QUEST_OBJECTIVES_COUNT];
    uint32 RequiredSpellCast[QUEST_OBJECTIVES_COUNT];
    uint32 RewChoiceItemId[QUEST_REWARD_CHOICES_COUNT];
    uint32 RewChoiceItemCount[QUEST_REWARD_CHOICES_COUNT];
    uint32 RewItemId[QUEST_REWARDS_COUNT];
    uint32 RewItemCount[QUEST_REWARDS_COUNT];
    uint32 RewRepFaction[QUEST_REPUTATIONS_COUNT];
    int32 RewRepValueId[QUEST_REPUTATIONS_COUNT];
    int32 RewRepValue[QUEST_REPUTATIONS_COUNT];
    uint32 DetailsEmote[QUEST_EMOTE_COUNT];
    uint32 DetailsEmoteDelay[QUEST_EMOTE_COUNT];
    uint32 OfferRewardEmote[QUEST_EMOTE_COUNT];
    uint32 OfferRewardEmoteDelay[QUEST_EMOTE_COUNT];
    uint32 RewCurrencyId[QUEST_CURRENCY_COUNT];
    uint32 RewCurrencyCount[QUEST_CURRENCY_COUNT];
    uint32 ReqCurrencyId[QUEST_CURRENCY_COUNT];
    uint32 ReqCurrencyCount[QUEST_CURRENCY_COUNT];
    std::string QuestGiverTextWindow;
    std::string QuestGiverTargetName;
    std::string QuestTurnTextWindow;
    std::string QuestTurnTargetName;
    uint32 QuestTargetMark;
    uint16 QuestStartType;
    uint32 SoundAccept;
    uint32 SoundTurnIn;
    uint32 RequiredSpell;
    uint32 ReqEmoteId;

    uint32 XPValue(Player *pPlayer) const;

    uint32 GetQuestFlags() const { return QuestFlags; }
    bool HasFlag(uint32 flag) const { return (QuestFlags & flag) != 0; }
    bool HasSpecialFlag(uint32 flag) const { return (SpecialFlags & flag) != 0; }
    void SetSpecialFlag(uint32 flag) { SpecialFlags |= flag; }

    // table data accessors:
    uint32 GetQuestId() const { return QuestId; }
    uint32 GetQuestMethod() const { return QuestMethod; }
    int32 GetZoneOrSort() const { return ZoneOrSort; }
    int32 GetSkillOrClassMask() const { return SkillOrClassMask; }
    uint32 GetMinLevel() const { return MinLevel; }
    uint32 GetMaxLevel() const { return MaxLevel; }
    uint32 GetQuestLevel() const { return QuestLevel; }
    uint32 GetType() const { return Type; }
    uint32 GetRequiredRaces() const { return RequiredRaces; }
    uint32 GetRequiredSkillValue() const { return RequiredSkillValue; }
    uint32 GetRepObjectiveFaction() const { return RepObjectiveFaction; }
    int32 GetRepObjectiveValue() const { return RepObjectiveValue; }
    uint32 GetRepObjectiveFaction2() const { return RepObjectiveFaction2; }
    int32 GetRepObjectiveValue2() const { return RepObjectiveValue2; }
    uint32 GetRequiredMinRepFaction() const { return RequiredMinRepFaction; }
    int32 GetRequiredMinRepValue() const { return RequiredMinRepValue; }
    uint32 GetRequiredMaxRepFaction() const { return RequiredMaxRepFaction; }
    int32 GetRequiredMaxRepValue() const { return RequiredMaxRepValue; }
    uint32 GetSuggestedPlayers() const { return SuggestedPlayers; }
    uint32 GetLimitTime() const { return LimitTime; }
    int32 GetPrevQuestId() const { return PrevQuestId; }
    int32 GetNextQuestId() const { return NextQuestId; }
    int32 GetExclusiveGroup() const { return ExclusiveGroup; }
    uint32 GetNextQuestInChain() const { return NextQuestInChain; }
    uint32 GetCharTitleId() const { return CharTitleId; }
    uint32 GetPlayersSlain() const { return PlayersSlain; }
    uint32 GetBonusTalents() const { return BonusTalents; }
    int32 GetRewArenaPoints() const { return RewArenaPoints; }
    uint32 GetRewSkillLineId() const { return RewSkillLineId; }
    uint32 GetRewSkillPoints() const { return RewSkillPoints; }
    uint32 GetRewRepMask() const { return RewRepMask; }
    uint32 GetQuestGiverPortrait() const { return QuestGiverPortrait; }
    uint32 GetQuestTurnInPortrait() const { return QuestTurnInPortrait; }
    uint32 GetXPId() const { return XPId; }
    uint32 GetSrcItemId() const { return SrcItemId; }
    uint32 GetSrcItemCount() const { return SrcItemCount; }
    uint32 GetSrcSpell() const { return SrcSpell; }
    std::string GetTitle() const { return Title; }
    std::string GetDetails() const { return Details; }
    std::string GetObjectives() const { return Objectives; }
    std::string GetOfferRewardText() const { return OfferRewardText; }
    std::string GetRequestItemsText() const { return RequestItemsText; }
    std::string GetEndText() const { return EndText; }
    std::string GetCompletedText() const { return CompletedText; }
    int32 GetRewOrReqMoney() const;
    uint32 GetRewHonorAddition() const { return RewHonorAddition; }
    float GetRewHonorMultiplier() const { return RewHonorMultiplier; }
    uint32 GetRewMoneyMaxLevel() const { return RewMoneyMaxLevel; }
    uint32 GetRewCurrencyId(uint32 n) const { return RewCurrencyId[n]; }
    uint32 GetRewCurrencyCount(uint32 n) const { return RewCurrencyCount[n]; }
    // use in XP calculation at client
    uint32 GetRewSpell() const { return RewSpell; }
    int32 GetRewSpellCast() const { return RewSpellCast; }
    uint32 GetRewMailTemplateId() const { return RewMailTemplateId; }
    uint32 GetRewMailDelaySecs() const { return RewMailDelaySecs; }
    uint32 GetPointMapId() const { return PointMapId; }
    float GetPointX() const { return PointX; }
    float GetPointY() const { return PointY; }
    uint32 GetPointOpt() const { return PointOpt; }
    uint32 GetIncompleteEmote() const { return IncompleteEmote; }
    uint32 GetCompleteEmote() const { return CompleteEmote; }
    std::string GetQuestGiverPortraitText() const { return QuestGiverTextWindow; }
    std::string GetQuestGiverPortraitUnk() const { return QuestGiverTargetName; }
    std::string GetQuestTurnInPortraitText() const { return QuestTurnTextWindow; }
    std::string GetQuestTurnInPortraitUnk() const { return QuestTurnTargetName; }
    uint32 GetQuestTargetMark() const { return QuestTargetMark; }
    uint16 GetQuestStartType() const { return QuestStartType; }
    uint32 GetSoundAccept() const { return SoundAccept; }
    uint32 GetSoundTurnIn() const { return SoundTurnIn; }
    uint32 GetRequiredSpell() const { return RequiredSpell; }
    bool IsRepeatable() const { return SpecialFlags & QUEST_SPECIAL_FLAG_REPEATABLE; }
    bool IsAutoComplete() const { return QuestMethod ? false : true; }
    uint32 GetFlags() const { return QuestFlags; }

    bool IsDaily() const
    { 
        if (QuestFlags == QUEST_FLAGS_DAILY)
            return true;
        else
            return false;
    }

    bool IsWeekly() const
    {
        if (QuestFlags == QUEST_FLAGS_WEEKLY)
            return true;
        else
            return false;
    
    }    

    bool IsDailyOrWeekly() const
    {
        if (QuestFlags == QUEST_FLAGS_DAILY || QUEST_FLAGS_WEEKLY)
            return true;
        else
            return false;
    }      
    
    bool IsAutoAccept() const
    {
        if (QuestFlags == QUEST_FLAGS_AUTO_ACCEPT)
            return true;
        else
            return false;
    }

    bool IsRaidQuest() const
    {
        if (Type == QUEST_TYPE_RAID || Type == QUEST_TYPE_RAID_10 || Type == QUEST_TYPE_RAID_25)
            return true;
        else
            return false;
    }

    bool IsDFQuest() const
    {
        if (QuestFlags == QUEST_SPECIAL_FLAG_DF_QUEST)
            return true;
        else
            return false;
    }

    uint32 RewardCurrencyId[QUEST_REWARD_CURRENCY_COUNT];
    uint32 RewardCurrencyCount[QUEST_REWARD_CURRENCY_COUNT];
    uint32 RequiredCurrencyId[QUEST_REQUIRED_CURRENCY_COUNT];
    uint32 RequiredCurrencyCount[QUEST_REQUIRED_CURRENCY_COUNT];

    uint32 GetReqItemsCount() const { return m_reqitemscount; }
    uint32 GetReqCreatureOrGOcount() const { return m_reqCreatureOrGOcount; }
    uint32 GetRewChoiceItemsCount() const { return m_rewchoiceitemscount; }
    uint32 GetRewItemsCount() const { return m_rewitemscount; }
    uint32 GetRewCurrencyCount() const { return m_rewCurrencyCount; }
    uint32 GetReqCurrencyCount() const { return m_reqCurrencyCount; }

    typedef std::vector<int32> PrevQuests;
    PrevQuests prevQuests;
    typedef std::vector<uint32> PrevChainQuests;
    PrevChainQuests prevChainQuests;

    std::set<uint32> quest_list;
    std::set<uint32> remove_quest_list;
    QuestScript* pQuestScript;

    uint32 m_reqitemscount;
    uint32 m_reqCreatureOrGOcount;
    uint32 m_rewchoiceitemscount;
    uint32 m_rewitemscount;
    uint32 m_rewCurrencyCount;
    uint32 m_reqCurrencyCount;
    uint8 m_reqMobType[4];
    uint32 m_reqExploreTrigger[QUEST_REQUIRED_AREA_TRIGGERS];
};

#pragma pack(pop)

class QuestScript;

#endif // WOWSERVER_QUEST_H
