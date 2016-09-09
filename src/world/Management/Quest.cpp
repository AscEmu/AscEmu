/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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
 *
 */

#include "StdAfx.h"

//////////////////////////////////////////////////////////////////////////////////////////
///Packet Building
WorldPacket* WorldSession::BuildQuestQueryResponse(QuestProperties const* pQuest)
{
    std::string Title, Details, Objectives, EndText, CompletedText, QuestGiverTextWindow, QuestGiverTargetName, QuestTurnTextWindow, QuestTurnTargetName;
    std::string ObjectiveText[QUEST_OBJECTIVES_COUNT];
    Title = pQuest->GetTitle();
    Details = pQuest->GetDetails();
    Objectives = pQuest->GetObjectives();
    EndText = pQuest->GetEndText();
    CompletedText = pQuest->GetCompletedText();
    QuestGiverTextWindow = pQuest->GetQuestGiverPortraitText();
    QuestGiverTargetName = pQuest->GetQuestGiverPortraitUnk();
    QuestTurnTextWindow = pQuest->GetQuestTurnInPortraitText();
    QuestTurnTargetName = pQuest->GetQuestTurnInPortraitUnk();

    for (int i = 0; i<QUEST_OBJECTIVES_COUNT; ++i)
        ObjectiveText[i] = pQuest->ObjectiveText[i];

    WorldPacket* data = new WorldPacket(SMSG_QUEST_QUERY_RESPONSE, 248);
    LocalizedQuest* lci = (language > 0) ? sLocalizationMgr.GetLocalizedQuest(pQuest->GetQuestId(), language) : NULL;

    *data << uint32(pQuest->GetQuestId());                   // quest id
    *data << uint32(pQuest->GetQuestMethod());               // Accepted values: 0, 1 or 2. 0 == IsAutoComplete() (skip objectives/details)
    *data << uint32(pQuest->GetQuestLevel());                // may be -1, static data, in other cases must be used dynamic level: Player::GetQuestLevel (0 is not known, but assuming this is no longer valid for quest intended for client)
    *data << uint32(pQuest->GetMinLevel());                  // min level
    *data << uint32(pQuest->GetZoneOrSort());                // zone or sort to display in quest log

    *data << uint32(pQuest->GetType());                      // quest type
    *data << uint32(pQuest->GetSuggestedPlayers());          // suggested players count

    *data << uint32(pQuest->GetRepObjectiveFaction());       // shown in quest log as part of quest objective
    *data << uint32(pQuest->GetRepObjectiveValue());         // shown in quest log as part of quest objective

    *data << uint32(pQuest->GetRepObjectiveFaction2());       // shown in quest log as part of quest objective OPOSITE faction
    *data << uint32(pQuest->GetRepObjectiveValue2());         // shown in quest log as part of quest objective OPPOSITE faction

    *data << uint32(pQuest->GetNextQuestInChain());          // client will request this quest from NPC, if not 0
    *data << uint32(pQuest->GetXPId());                      // used for calculating rewarded experience

    *data << uint32(pQuest->GetRewOrReqMoney());             // reward money (below max lvl)
    *data << uint32(pQuest->GetRewMoneyMaxLevel());
    *data << uint32(pQuest->GetRewSpell());                  // reward spell, this spell will display (icon) (casted if RewSpellCast == 0)
    *data << int32(pQuest->GetRewSpellCast());               // casted spell
    *data << uint32(0);
    *data << uint32(0);

    *data << uint32(pQuest->GetSrcItemId());                 // source item id
    *data << uint32(pQuest->GetFlags() & 0xFFFF);                      // quest flags
    *data << uint32(pQuest->GetQuestTargetMark());           // Minimap Target Mark, 1-Skull, 16-Unknown
    *data << uint32(pQuest->GetCharTitleId());               // CharTitleId, new 2.4.0, player gets this title (id from CharTitles)
    *data << uint32(pQuest->GetPlayersSlain());              // players slain
    *data << uint32(pQuest->GetBonusTalents());              // bonus talents
    *data << uint32(pQuest->GetRewArenaPoints());            // bonus arena points
    *data << uint32(pQuest->GetRewSkillLineId());            // reward skill line id
    *data << uint32(pQuest->GetRewSkillPoints());            // reward skill points
    *data << uint32(pQuest->GetRewRepMask());                // review rep show mask
    *data << uint32(pQuest->GetQuestGiverPortrait());        // questgiver portrait ID
    *data << uint32(pQuest->GetQuestTurnInPortrait());       // quest turn in portrait ID

    int iI;

    if (pQuest->HasFlag(QUEST_FLAGS_HIDDEN_REWARDS))
    {
        for (iI = 0; iI < QUEST_REWARDS_COUNT; ++iI)
            *data << uint32(0) << uint32(0);
        for (iI = 0; iI < QUEST_REWARD_CHOICES_COUNT; ++iI)
            *data << uint32(0) << uint32(0);
    }
    else
    {
        for (iI = 0; iI < QUEST_REWARDS_COUNT; ++iI)
        {
            *data << uint32(pQuest->RewItemId[iI]);
            *data << uint32(pQuest->RewItemCount[iI]);
        }
        for (iI = 0; iI < QUEST_REWARD_CHOICES_COUNT; ++iI)
        {
            *data << uint32(pQuest->RewChoiceItemId[iI]);
            *data << uint32(pQuest->RewChoiceItemCount[iI]);
        }
    }

    for (int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)         // reward factions ids
        *data << uint32(pQuest->RewRepFaction[i]);

    for (int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)         // columnid+1 QuestFactionReward.dbc?
        *data << int32(pQuest->RewRepValueId[i]);

    for (int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)         // unk (0)
        *data << int32(pQuest->RewRepValue[i]);

    *data << uint32(pQuest->GetPointMapId());
    *data << float(pQuest->GetPointX());
    *data << float(pQuest->GetPointY());
    *data << uint32(pQuest->GetPointOpt());

    if (lci)
    {
        *data << lci->Title;
        *data << lci->Objectives;
        *data << lci->Details;
        *data << lci->CompletionText;
        *data << lci->EndText;
    }
    else
    {
        *data << Title;
        *data << Objectives;
        *data << Details;
        *data << CompletedText;                                  // display in quest objectives window once all objectives are completed
        *data << EndText;
    }

    for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; ++iI)
    {
        if (pQuest->ReqCreatureOrGOId[iI] < 0)
        {
            // client expected gameobject template id in form (id|0x80000000)
            *data << uint32((pQuest->ReqCreatureOrGOId[iI] * (-1)) | 0x80000000);
        }
        else
        {
            *data << uint32(pQuest->ReqCreatureOrGOId[iI]);
        }
        *data << uint32(pQuest->ReqCreatureOrGOCount[iI]);
        *data << uint32(pQuest->ReqSourceId[iI]);            // item drop intermediate ID
        *data << uint32(pQuest->ReqSourceCount[iI]);         // item drop intermediate count
    }

    for (iI = 0; iI < QUEST_ITEM_OBJECTIVES_COUNT; ++iI)
    {
        *data << uint32(pQuest->ReqItemId[iI]);
        *data << uint32(pQuest->ReqItemCount[iI]);
    }

    *data << uint32(pQuest->GetRequiredSpell());

    for (iI = 0; iI < QUEST_OBJECTIVES_COUNT; ++iI)
        *data << ObjectiveText[iI];

    for (iI = 0; iI < 4; ++iI)                               // 4.0.0 currency reward id and count
    {
        *data << uint32(pQuest->RewCurrencyId[iI]);
        *data << uint32(pQuest->RewCurrencyCount[iI]);
    }

    for (iI = 0; iI < 4; ++iI)                               // 4.0.0 currency required id and count
    {
        *data << uint32(pQuest->ReqCurrencyId[iI]);
        *data << uint32(pQuest->ReqCurrencyCount[iI]);
    }

    *data << QuestGiverTextWindow;
    *data << QuestGiverTargetName;
    *data << QuestTurnTextWindow;
    *data << QuestTurnTargetName;
    
    *data << uint32(pQuest->GetSoundAccept());
    *data << uint32(pQuest->GetSoundTurnIn());

    return data;
}


uint32 QuestProperties::XPValue(Player *pPlayer) const
{
    if (pPlayer)
    {
        int32 quest_level = (QuestLevel == -1 ? pPlayer->getLevel() : QuestLevel);
        auto xpentry = sQuestXPStore.LookupEntry(quest_level);
        if (!xpentry)
            return XPId;

        int32 diffFactor = 2 * (quest_level - pPlayer->getLevel()) + 20;
        if (diffFactor < 1)
            diffFactor = 1;
        else if (diffFactor > 10)
            diffFactor = 10;

        uint32 xp = diffFactor * xpentry->xpIndex[XPId] / 10;
        if (xp <= 100)
            xp = 5 * ((xp + 2) / 5);
        else if (xp <= 500)
            xp = 10 * ((xp + 5) / 10);
        else if (xp <= 1000)
            xp = 25 * ((xp + 12) / 25);
        else
            xp = 50 * ((xp + 25) / 50);

        return xp;
    }

    return 0;
}

int32 QuestProperties::GetRewOrReqMoney() const
{
    if (RewOrReqMoney <= 0)
        return RewOrReqMoney;

    return int32(RewOrReqMoney);
}
