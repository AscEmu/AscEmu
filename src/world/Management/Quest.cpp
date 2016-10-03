/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

void WorldSession::BuildQuestQueryResponse(QuestProperties const* qst)
{
    WorldPacket data(SMSG_QUEST_QUERY_RESPONSE, 100);
    LocalizedQuest* lci = (language > 0) ? sLocalizationMgr.GetLocalizedQuest(qst->id, language) : nullptr;

    data << uint32(qst->id);                                        // Quest ID
    data << uint32(2);                                              // Unknown, always seems to be 2
    data << int32(qst->questlevel);                                 // Quest level
    data << uint32(qst->min_level);                                 // Quest required level

    if (qst->quest_sort > 0)
        data << int32(-(int32)qst->quest_sort);                     // Negative if pointing to a sort.
    else
        data << uint32(qst->zone_id);                               // Positive if pointing to a zone.

    data << uint32(qst->type);                                      // Info ID / Type
    data << qst->suggestedplayers;                                  // suggested players

    data << uint32(qst->required_rep_faction);                      // Faction ID
    data << uint32(qst->required_rep_value);                        // Faction Amount

    data << uint32(0);                                              // Unknown (always 0)
    data << uint32(0);                                              // Unknown (always 0)

    data << uint32(qst->next_quest_id);                             // Next Quest ID
    data << uint32(0);                                              // Column id +1 from QuestXp.dbc, entry is quest level

    data << uint32(sQuestMgr.GenerateRewardMoney(_player, qst));    // Copper reward
    data << uint32(qst->reward_money < 0 ? -qst->reward_money : 0); // Required Money

    data << uint32(qst->reward_spell);                              // Spell added to spellbook upon completion
    data << int32(qst->effect_on_player);                           // Spell casted on player upon completion

    data << uint32(qst->bonushonor);
    data << float(0);                                               // 3.3.0 - some multiplier for honor
    data << uint32(qst->srcitem);                                   // Item given at the start of a quest (srcitem)
    data << uint32(qst->quest_flags);                               // Quest Flags
    data << uint32(0);                                              // target minimap
    data << uint32(qst->rewardtitleid);
    data << uint32(0);                                              // playerkillcount
    data << uint32(qst->rewardtalents);
    data << uint32(0);                                              // arena points
    data << uint32(0);                                              // reward skill id
    data << uint32(0);                                              // reward skill points
    data << uint32(0);                                              // rep mask (unsure on what it does)
    data << uint32(0);                                              // quest giver entry ?
    data << uint32(0);                                              // quest turnin entry ?

    if (qst->HasFlag(QUEST_FLAGS_HIDDEN_REWARDS))
    {
        for (uint8 i = 0; i < QUEST_MAX_REWARD_ITEM; ++i)
            data << uint32(0) << uint32(0);
        for (uint8 i = 0; i < 6; ++i)
            data << uint32(0) << uint32(0);
    }
    else
    {
        for (uint8 i = 0; i < QUEST_MAX_REWARD_ITEM; ++i)
        {
            data << uint32(qst->reward_item[i]);
            data << uint32(qst->reward_itemcount[i]);
        }
        for (uint8 i = 0; i < 6; ++i)
        {
            data << uint32(qst->reward_choiceitem[i]);
            data << uint32(qst->reward_choiceitemcount[i]);
        }
    }

    for (uint8 i = 0; i < 5; ++i)
        data << uint32(qst->reward_repfaction[i]);

    for (uint8 i = 0; i < 5; ++i)
        data << uint32(0);                                          // column index in QuestFactionReward.dbc but use unknown

    for (uint8 i = 0; i < 5; ++i)
        data << uint32(0);                                          // unk

    data << qst->point_mapid;
    data << qst->point_x;
    data << qst->point_y;
    data << qst->point_opt;

    data << (lci ? lci->Title : qst->title);
    data << (lci ? lci->Objectives : qst->objectives);
    data << (lci ? lci->Details : qst->details);
    data << (lci ? lci->EndText : qst->endtext);
    data << "";                                                     // completed text

    for (uint8 i = 0; i < 4; ++i)
    {
        data << uint32(qst->required_mob_or_go[i]);                 // Kill mob entry ID [i]
        data << uint32(qst->required_mob_or_go_count[i]);           // Kill mob count [i]
        data << uint32(0);                                          // req src item
        data << uint32(0);                                          // req src item count
    }

    for (uint8 i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
    {
        data << uint32(qst->required_item[i]);                      // Collect item [i]
        data << uint32(qst->required_itemcount[i]);                 // Collect item count [i]
    }

    for (uint8  i = 0; i < QUEST_MAX_OBJECTIVES; ++i)
    {
        data << (lci ? lci->ObjectiveText[i] : qst->objectivetexts[i]);
    }

    for (uint8  i = 0; i < QUEST_REWARD_CURRENCY_COUNT; ++i)
    {
        data << uint32(0);                                          // currency id
        data << uint32(0);                                          // currency count
    }

    for (uint8  i = 0; i < QUEST_REQUIRED_CURRENCY_COUNT; ++i)
    {
        data << uint32(0);                                          // req currency id
        data << uint32(0);                                          // req currency count
    }

    data << "";                                                     //questGiverTextWindow;
    data << "";                                                     //questGiverTargetName;
    data << "";                                                     //questTurnTextWindow;
    data << "";                                                     //questTurnTargetName;
    data << uint32(0);                                              // accept sound?
    data << uint32(0);                                              // sound turn in?
    
    _player->SendPacket(&data);
}

uint32 QuestProperties::GetRewardItemCount() const
{
    uint32 count = 0;
    for (uint8 i = 0; i < QUEST_MAX_REWARD_ITEM; ++i)
        if (reward_item[i] != 0)
            ++count;

    return count;
}
