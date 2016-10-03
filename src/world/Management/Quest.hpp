/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _QUEST_HPP
#define _QUEST_HPP

#include "QuestDefines.hpp"

class QuestScript;

#pragma pack(push,1)

#define QUEST_SPECIAL_FLAG_DB_ALLOWED (QUEST_SPECIAL_FLAG_REPEATABLE | QUEST_SPECIAL_FLAG_EXPLORATION_OR_EVENT | QUEST_SPECIAL_FLAG_AUTO_ACCEPT | QUEST_SPECIAL_FLAG_DF_QUEST)

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

    std::string objectivetexts[QUEST_MAX_OBJECTIVES];

    uint32 required_item[MAX_REQUIRED_QUEST_ITEM];
    uint32 required_itemcount[MAX_REQUIRED_QUEST_ITEM];

    int32 required_mob_or_go[4];              // positive is NPC, negative is GO
    uint32 required_mob_or_go_count[4];
    uint32 required_spell[4];
    uint32 required_emote[4];

    uint32 reward_choiceitem[6];
    uint32 reward_choiceitemcount[6];

    uint32 reward_item[QUEST_MAX_REWARD_ITEM];
    uint32 reward_itemcount[QUEST_MAX_REWARD_ITEM];

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

    //count
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

    bool HasFlag(uint32 flag) const
    {
        if ((quest_flags & flag) != 0)
            return true;

        return false;
    }
};

#pragma pack(pop)

class QuestScript;

#endif // _QUEST_HPP
