/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <set>
#include <string>

#define DEFINE_QUEST_REPEATABLE 1
#define DEFINE_QUEST_REPEATABLE_DAILY 2
#define MAX_REQUIRED_QUEST_ITEM 6

class QuestScript;

struct QuestProperties
{
    uint32_t id;
    uint32_t zone_id;
    uint32_t quest_sort;
    uint32_t quest_flags;
    uint32_t min_level;
    int32_t questlevel;
    uint32_t type;
    uint32_t required_races;
    uint32_t required_class;
    uint16_t required_tradeskill;
    uint32_t required_tradeskill_value;
    uint32_t required_rep_faction;
    uint32_t required_rep_value;

    uint32_t time;
    uint32_t special_flags;

    uint32_t previous_quest_id;
    uint32_t next_quest_id;

    uint32_t srcitem;
    uint32_t srcitemcount;

    std::string title;
    std::string details;
    std::string objectives;
    std::string completiontext;
    std::string incompletetext;
    std::string endtext;

    std::string objectivetexts[4];

    uint32_t required_item[MAX_REQUIRED_QUEST_ITEM];
    uint32_t required_itemcount[MAX_REQUIRED_QUEST_ITEM];

    int32_t required_mob_or_go[4];              // positive is NPC, negative is GO
    uint32_t required_mob_or_go_count[4];
    uint32_t required_spell[4];
    uint32_t required_emote[4];

    uint32_t reward_choiceitem[6];
    uint32_t reward_choiceitemcount[6];

    uint32_t reward_item[4];
    uint32_t reward_itemcount[4];

    uint32_t reward_repfaction[6];
    int32_t reward_repvalue[6];
    uint32_t reward_replimit;

    int32_t reward_money;
    uint32_t reward_xp;
    uint32_t reward_spell;
    uint32_t effect_on_player;

    uint32_t MailTemplateId;
    uint32_t MailDelaySecs;
    uint32_t MailSendItem;

    uint32_t point_mapid;
    float point_x;
    float point_y;
    uint32_t point_opt;

    uint32_t rew_money_at_max_level;
    uint32_t required_triggers[4];
    std::string x_or_y_quest_string;
    uint32_t required_quests[4];
    std::string remove_quests;
    uint32_t receive_items[4];
    uint32_t receive_itemcount[4];
    int is_repeatable;
    uint32_t GetRewardItemCount() const;

    uint32_t bonushonor;
    uint32_t bonusarenapoints;
    uint32_t rewardtitleid;
    uint32_t rewardtalents;
    uint32_t suggestedplayers;

    // emotes
    uint32_t detailemotecount;
    uint32_t detailemote[4];
    uint32_t detailemotedelay[4];
    uint32_t completionemotecount;
    uint32_t completionemote[4];
    uint32_t completionemotedelay[4];
    uint32_t completeemote;
    uint32_t incompleteemote;
    uint32_t iscompletedbyspelleffect;
    uint32_t RewXPId;

    // this marks the end of the fields loaded from db - don't remove the comment please

    uint8_t count_required_mob;
    uint8_t count_requiredquests;
    uint8_t count_requiredtriggers;
    uint8_t count_receiveitems;
    uint8_t count_reward_choiceitem;
    uint8_t count_required_item;
    uint8_t required_mobtype[4];
    uint8_t count_reward_item;

    std::set<uint32_t> quest_list;
    std::set<uint32_t> remove_quest_list;

    QuestScript* pQuestScript;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Tells if the quest has a specific flag.
    //
    // \param uint32_t flag  -  flag to check
    //
    // \return true if the quest has this flag, false if the quest doesn't have this flag.
    //
    //////////////////////////////////////////////////////////////////////////////////////////
    bool HasFlag(uint32_t flag) const
    {
        if ((quest_flags & flag) != 0)
            return true;

        return false;
    }
};
