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

#define arcemu_QUEST_REPEATABLE 1
#define arcemu_QUEST_REPEATABLE_DAILY 2
#define MAX_REQUIRED_QUEST_ITEM 6

class QuestScript;
#pragma pack(push,1)
struct QuestProperties
{
	uint32 id;
	uint32 zone_id;
	uint32 quest_sort;
	uint32 quest_flags;
	uint32 min_level;
	int32 questlevel;
	uint32 type;
	uint32 required_races;
	uint32 required_class;
	uint32 required_tradeskill;
	uint32 required_tradeskill_value;
	uint32 required_rep_faction;
	uint32 required_rep_value;

	uint32 time;
	uint32 special_flags;

	uint32 previous_quest_id;
	uint32 next_quest_id;

	uint32 srcitem;
	uint32 srcitemcount;

	std::string title;
    std::string details;
    std::string objectives;
    std::string completiontext;
    std::string incompletetext;
    std::string endtext;

    std::string objectivetexts[4];

	uint32 required_item[MAX_REQUIRED_QUEST_ITEM];
	uint32 required_itemcount[MAX_REQUIRED_QUEST_ITEM];

	int32 required_mob[4];              /// positive is NPC, negative is GO
	uint32 required_mobcount[4];
	uint32 required_spell[4];
	uint32 required_emote[4];

	uint32 reward_choiceitem[6];
	uint32 reward_choiceitemcount[6];

	uint32 reward_item[4];
	uint32 reward_itemcount[4];

	uint32 reward_repfaction[6];
	int32 reward_repvalue[6];
	uint32 reward_replimit;

	int32 reward_money;
	uint32 reward_xp;
	uint32 reward_spell;
	uint32 effect_on_player;

	uint32 MailTemplateId;
	uint32 MailDelaySecs;
	uint32 MailSendItem;

	uint32 point_mapid;
	uint32 point_x;
	uint32 point_y;
	uint32 point_opt;

	uint32 rew_money_at_max_level;
	uint32 required_triggers[4];
    std::string x_or_y_quest_string;
	uint32 required_quests[4];
    std::string remove_quests;
	uint32 receive_items[4];
	uint32 receive_itemcount[4];
	int is_repeatable;
    uint32 GetRewardItemCount() const;

	uint32 bonushonor;
	uint32 bonusarenapoints;
	uint32 rewardtitleid;
	uint32 rewardtalents;
	uint32 suggestedplayers;

	// emotes
	uint32 detailemotecount;
	uint32 detailemote[4];
	uint32 detailemotedelay[4];
	uint32 completionemotecount;
	uint32 completionemote[4];
	uint32 completionemotedelay[4];
	uint32 completeemote;
	uint32 incompleteemote;
	uint32 iscompletedbyspelleffect;
	uint32 RewXPId;

	// this marks the end of the fields loaded from db - don't remove the comment please

	uint32 count_required_mob;
	uint32 count_requiredquests;
	uint32 count_requiredtriggers;
	uint32 count_receiveitems;
	uint32 count_reward_choiceitem;
	uint32 count_required_item;
	uint32 required_mobtype[4];
	uint32 count_reward_item;

	std::set<uint32> quest_list;
	std::set<uint32> remove_quest_list;

	QuestScript* pQuestScript;

    //////////////////////////////////////////////////////////////////////////////////////////
	/// Tells if the quest has a specific flag.
	///
	/// \param uint32 flag  -  flag to check
	///
	/// \return true if the quest has this flag, false if the quest doesn't have this flag.
	///
    //////////////////////////////////////////////////////////////////////////////////////////
	bool HasFlag(uint32 flag) const
	{
		if ((quest_flags & flag) != 0)
			return true;
		else
			return false;
	}
};

#pragma pack(pop)

class QuestScript;

#endif // WOWSERVER_QUEST_H
