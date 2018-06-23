/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Map/WorldCreatorDefines.hpp"
#include "Map/MapMgr.h"
#include "Management/ItemInterface.h"

void WorldSession::HandleQuestPOIQueryOpcode(WorldPacket& recvData)
{
    uint32_t count = 0;
    recvData >> count;

    if (count > MAX_QUEST_LOG_SIZE)
    {
        LOG_DEBUG("Client sent Quest POI query for more than MAX_QUEST_LOG_SIZE quests.");

        count = MAX_QUEST_LOG_SIZE;
    }

    WorldPacket data(SMSG_QUEST_POI_QUERY_RESPONSE, 4 + (4 + 4) * count);
    data << uint32_t(count);

    for (uint32_t i = 0; i < count; ++i)
    {
        uint32_t questId;
        recvData >> questId;

        sQuestMgr.BuildQuestPOIResponse(data, questId);
    }

    SendPacket(&data);
}

WorldPacket* WorldSession::BuildQuestQueryResponse(QuestProperties const* qst)
{
    WorldPacket* data = new WorldPacket(SMSG_QUEST_QUERY_RESPONSE, 100);
    MySQLStructure::LocalesQuest const* lci = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;

    *data << uint32_t(qst->id);                                        // Quest ID
    *data << uint32_t(2);                                              // Unknown, always seems to be 2
    *data << int32_t(qst->questlevel);                                 // Quest level
    *data << uint32_t(qst->min_level);                                 // Quest required level

    if (qst->quest_sort > 0)
    {
        *data << int32_t(-(int32_t)qst->quest_sort);                     // Negative if pointing to a sort.
    }
    else
    {
        *data << uint32_t(qst->zone_id);                               // Positive if pointing to a zone.
    }

    *data << uint32_t(qst->type);                                      // Info ID / Type
    *data << qst->suggestedplayers;                                  // suggested players

    *data << uint32_t(qst->required_rep_faction);                      // Faction ID
    *data << uint32_t(qst->required_rep_value);                        // Faction Amount

    *data << uint32_t(0);                                              // Unknown (always 0)
    *data << uint32_t(0);                                              // Unknown (always 0)

    *data << uint32_t(qst->next_quest_id);                             // Next Quest ID
    *data << uint32_t(0);                                              // Column id +1 from QuestXp.dbc, entry is quest level

    *data << uint32_t(sQuestMgr.GenerateRewardMoney(_player, qst));    // Copper reward
    *data << uint32_t(qst->reward_money < 0 ? -qst->reward_money : 0); // Required Money

    *data << uint32_t(qst->reward_spell);                              // Spell added to spellbook upon completion
    *data << int32_t(qst->effect_on_player);                           // Spell casted on player upon completion

    *data << uint32_t(qst->bonushonor);
    *data << float(0);                                               // 3.3.0 - some multiplier for honor
    *data << uint32_t(qst->srcitem);                                   // Item given at the start of a quest (srcitem)
    *data << uint32_t(qst->quest_flags);                               // Quest Flags
    *data << uint32_t(0);                                              // target minimap
    *data << uint32_t(qst->rewardtitleid);
    *data << uint32_t(0);                                              // playerkillcount
    *data << uint32_t(qst->rewardtalents);
    *data << uint32_t(0);                                              // arena points
    *data << uint32_t(0);                                              // reward skill id
    *data << uint32_t(0);                                              // reward skill points
    *data << uint32_t(0);                                              // rep mask (unsure on what it does)
    *data << uint32_t(0);                                              // quest giver entry ?
    *data << uint32_t(0);                                              // quest turnin entry ?

    if (qst->HasFlag(QUEST_FLAGS_HIDDEN_REWARDS))
    {
        for (uint8_t i = 0; i < 4; ++i)
        {
            *data << uint32_t(0);
            *data << uint32_t(0);
        }

        for (uint8_t i = 0; i < 6; ++i)
        {
            *data << uint32_t(0);
            *data << uint32_t(0);
        }
    }
    else
    {
        for (uint8_t i = 0; i < 4; ++i)
        {
            *data << uint32_t(qst->reward_item[i]);
            *data << uint32_t(qst->reward_itemcount[i]);
        }
        for (uint8_t i = 0; i < 6; ++i)
        {
            *data << uint32_t(qst->reward_choiceitem[i]);
            *data << uint32_t(qst->reward_choiceitemcount[i]);
        }
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << uint32_t(qst->reward_repfaction[i]);
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << uint32_t(0);                                          // column index in QuestFactionReward.dbc but use unknown
    }

    for (uint8_t i = 0; i < 5; ++i)
    {
        *data << uint32_t(0);                                          // unk
    }

    *data << qst->point_mapid;
    *data << qst->point_x;
    *data << qst->point_y;
    *data << qst->point_opt;

    *data << (lci ? lci->title : qst->title);
    *data << (lci ? lci->objectives : qst->objectives);
    *data << (lci ? lci->details : qst->details);
    *data << (lci ? lci->endText : qst->endtext);
    *data << "";                                                     // completed text

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(qst->required_mob_or_go[i]);                 // Kill mob entry ID [i]
        *data << uint32_t(qst->required_mob_or_go_count[i]);           // Kill mob count [i]
        *data << uint32_t(0);                                          // req src item
        *data << uint32_t(0);                                          // req src item count
    }

    for (uint8_t i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
    {
        *data << uint32_t(qst->required_item[i]);                      // Collect item [i]
        *data << uint32_t(qst->required_itemcount[i]);                 // Collect item count [i]
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << (lci ? lci->objectiveText[i] : qst->objectivetexts[i]);
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(0);                                          // currency id
        *data << uint32_t(0);                                          // currency count
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        *data << uint32_t(0);                                          // req currency id
        *data << uint32_t(0);                                          // req currency count
    }

    *data << "";                                                     //questGiverTextWindow;
    *data << "";                                                     //questGiverTargetName;
    *data << "";                                                     //questTurnTextWindow;
    *data << "";                                                     //questTurnTargetName;
    *data << uint32_t(0);                                              // accept sound?
    *data << uint32_t(0);                                              // sound turn in?

    return data;
}
