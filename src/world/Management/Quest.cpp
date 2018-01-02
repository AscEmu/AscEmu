/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
#include "Storage/MySQLStructures.h"
#include "Storage/MySQLDataStore.hpp"
#include "Quest.h"
#include "Server/WorldSession.h"
#include "QuestMgr.h"

//////////////////////////////////////////////////////////////////////////////////////////
///Packet Building
#if VERSION_STRING != Cata
WorldPacket* WorldSession::BuildQuestQueryResponse(QuestProperties const* qst)
{
    // 2048 bytes should be more than enough. The fields cost ~200 bytes.
    // better to allocate more at startup than have to realloc the buffer later on.

    WorldPacket* data = new WorldPacket(SMSG_QUEST_QUERY_RESPONSE, 248);
    MySQLStructure::LocalesQuest const* lci = (language > 0) ? sMySQLStore.getLocalizedQuest(qst->id, language) : nullptr;
    uint32 i;

    *data << uint32(qst->id);                                       // Quest ID
    *data << uint32(2);                                             // Unknown, always seems to be 2
    *data << int32(qst->questlevel);                                // Quest level
    *data << uint32(qst->min_level);                                // Quest required level

    if (qst->quest_sort > 0)
    {
        *data << int32(-(int32)qst->quest_sort);                    // Negative if pointing to a sort.
    }
    else
    {
        *data << uint32(qst->zone_id);                              // Positive if pointing to a zone.
    }

    *data << uint32(qst->type);                                     // Info ID / Type
    *data << qst->suggestedplayers;                                 // suggested players
    *data << uint32(qst->required_rep_faction);                     // Faction ID
    *data << uint32(qst->required_rep_value);                       // Faction Amount
    *data << uint32(0);                                             // Unknown (always 0)
    *data << uint32(0);                                             // Unknown (always 0)
    *data << uint32(qst->next_quest_id);                            // Next Quest ID
    *data << uint32(0);                                             // Column id +1 from QuestXp.dbc, entry is quest level
    *data << uint32(sQuestMgr.GenerateRewardMoney(_player, qst));   // Copper reward
    *data << uint32(qst->reward_money < 0 ? -qst->reward_money : 0);    // Required Money
    *data << uint32(qst->reward_spell);                             // Spell added to spellbook upon completion
    *data << uint32(qst->effect_on_player);                         // Spell casted on player upon completion
    *data << uint32(qst->bonushonor);                               // 2.3.0 - bonus honor
    *data << float(0);                                              // 3.3.0 - some multiplier for honor
    *data << uint32(qst->srcitem);                                  // Item given at the start of a quest (srcitem)
    *data << uint32(qst->quest_flags);                              // Quest Flags
    *data << qst->rewardtitleid;                                    // 2.4.0 unk
    *data << uint32(0);                                             // playerkillcount
    *data << qst->rewardtalents;
    *data << uint32(0);                                             // 3.3.0 Unknown
    *data << uint32(0);                                             // 3.3.0 Unknown

    // (loop 4 times)
    for (i = 0; i < 4; ++i)
    {
        *data << qst->reward_item[i];               // Forced Reward Item [i]
        *data << qst->reward_itemcount[i];          // Forced Reward Item Count [i]
    }

    // (loop 6 times)
    for (i = 0; i < 6; ++i)
    {
        *data << qst->reward_choiceitem[i];         // Choice Reward Item [i]
        *data << qst->reward_choiceitemcount[i];    // Choice Reward Item Count [i]
    }

    // (loop 5 times) - these 3 loops are here to allow displaying rep rewards in client (not handled in core yet)
    for (i = 0; i < 5; ++i)
    {
        *data << uint32(qst->reward_repfaction[i]); // reward factions ids
    }

    for (i = 0; i < 5; ++i)
    {
        *data << uint32(0);                         // column index in QuestFactionReward.dbc but use unknown
    }

    for (i = 0; i < 5; ++i)                         // Unknown
    {
        *data << uint32(0);
    }

    *data << qst->point_mapid;
    *data << qst->point_x;
    *data << qst->point_y;
    *data << qst->point_opt;

    if (lci != nullptr)
    {
        *data << lci->title;
        *data << lci->objectives;
        *data << lci->details;
        *data << lci->endText;
        *data << uint8(0);
    }
    else
    {
        *data << qst->title;                        // Title / name of quest
        *data << qst->objectives;                   // Objectives / description
        *data << qst->details;                      // Details
        *data << qst->endtext;                      // Subdescription
        *data << uint8(0);                          // most 3.3.0 quests i seen have something like "Return to NPCNAME"
    }

    for (i = 0; i < 4; ++i)
    {
        *data << qst->required_mob_or_go[i];              // Kill mob entry ID [i]
        *data << qst->required_mob_or_go_count[i];         // Kill mob count [i]
        *data << uint32(0);                         // Unknown
        *data << uint32(0);                         // 3.3.0 Unknown
    }

    for (i = 0; i < MAX_REQUIRED_QUEST_ITEM; ++i)
    {
        *data << qst->required_item[i];             // Collect item [i]
        *data << qst->required_itemcount[i];        // Collect item count [i]
    }

    if (lci != nullptr)
    {
        *data << lci->objectiveText[0];
        *data << lci->objectiveText[1];
        *data << lci->objectiveText[2];
        *data << lci->objectiveText[3];
    }
    else
    {
        *data << qst->objectivetexts[0];            // Objective 1 - Used as text if mob not set
        *data << qst->objectivetexts[1];            // Objective 2 - Used as text if mob not set
        *data << qst->objectivetexts[2];            // Objective 3 - Used as text if mob not set
        *data << qst->objectivetexts[3];            // Objective 4 - Used as text if mob not set
    }

    return data;
}
#endif


uint32 QuestProperties::GetRewardItemCount() const
{
    uint32 count = 0;
    for (uint8 i = 0; i < 4; ++i)
    {
        if (reward_item[i] != 0)
        {
            count++;
        }
    }

    return count;
}

