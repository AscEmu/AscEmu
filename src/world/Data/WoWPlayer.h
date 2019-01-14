/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

/*
 * THIS IS A WOW DATA STRUCT FILE
 *
 * DO NOT ADD ADDITIONAL MEMBERS TO IT
 *
 * DO NOT EDIT IT UNLESS YOU KNOW EXACTLY WHAT YOU ARE DOING
 */

#pragma once
#include "WoWUnit.h"
#pragma pack(push, 1)

union
{
    struct
    {
        uint8_t skin_color;
        uint8_t face;
        union
        {
            uint8_t hair_style;
            uint8_t horn_style;     //tauren
        };
        union
        {
            uint8_t hair_color;
            uint8_t horn_color;     //tauren
        };
    } s;
    uint32_t raw;
} typedef player_bytes_union;

union
{
    struct
    {
        union
        {
            uint8_t facial_hair;    //human m, orc m, dwarf m, night elf m, tauren m, gnome m, draenei m, blood elf m, worgen m
            uint8_t piercings;      //human f, orc f, dwarf f
            uint8_t features;       //undead m + f
            uint8_t markings;       //night elf f
            uint8_t hair;           //tauren f
            uint8_t earrings;       //gnome f, blood elf f
            uint8_t tusks;          //troll (m+f)
            uint8_t horn_style;     //draenei f
            uint8_t ears;           //worgen f, goblin (m+f)
        };
        uint8_t unk1;           //gm?
        uint8_t bank_slots;
        uint8_t rest_state;
    } s;
    uint32_t raw;
} typedef player_bytes_2_union;

union
{
    struct
    {
        uint8_t gender;
        uint16_t drunk_value;   // not sure
        uint8_t pvp_rank;
    } s;
    uint32_t raw;
} typedef player_bytes_3_union;

union
{
    struct
    {
        uint8_t flags; // not used
        uint8_t rafLevel; // not used
        uint8_t actionBarId;
        uint8_t maxPvpRank; // not used
    } s;
    uint32_t raw;
} typedef player_field_bytes_union;

union
{
    struct
    {
        uint16_t overrideSpellId;
        uint8_t ignorePowerRegenPredictionMask; // not used
        uint8_t auraVision; //not used
    } s;
    uint32_t raw;
} typedef player_field_bytes_2_union;

// Adjusted values.
#if VERSION_STRING == Classic
#define WOWPLAYER_QUEST_COUNT 20
#define WOWPLAYER_VISIBLE_ITEM_COUNT 19
#define WOWPLAYER_VISIBLE_ITEM_UNK0_COUNT 8

#define WOWPLAYER_INVENTORY_SLOT_COUNT 23
#define WOWPLAYER_PACK_SLOT_COUNT 16
// WOWPLAYER_BANK_SLOT_COUNT and WOWPLAYER_BANK_BAG_SLOT_COUNT are different to TBC. TODO: Verify
#define WOWPLAYER_BANK_SLOT_COUNT 24
#define WOWPLAYER_BANK_BAG_SLOT_COUNT 6
#define WOWPLAYER_VENDOR_BUY_BACK_SLOT_COUNT 12
#define WOWPLAYER_KEYRING_SLOT_COUNT 20
#define WOWPLAYER_SKILL_INFO_COUNT 384
#define WOWPLAYER_EXPLORED_ZONES_COUNT 64
#define WOWPLAYER_STAT_COUNT 5
#define WOWPLAYER_SPELL_SCHOOL_COUNT 7
#define WOWPLAYER_COMBAT_RATING_COUNT 20

struct WoWPlayer_Quest
{
    uint32_t unk1;  // id
    uint64_t unk2;  // 4 x uint16_t mob_or_go_count
};

struct WoWPlayer_VisibleItem
{
    uint64_t creator;
    uint32_t unk0[WOWPLAYER_VISIBLE_ITEM_UNK0_COUNT];
    uint32_t properties;
    uint32_t padding;
};

struct WoWPlayer : WoWUnit
{
    uint64_t duel_arbiter;
    uint32_t player_flags;
    uint32_t guild_id;
    uint32_t guild_rank;
    player_bytes_union player_bytes;
    player_bytes_2_union player_bytes_2;
    player_bytes_3_union player_bytes_3;
    uint32_t duel_team;
    uint32_t guild_timestamp;
    WoWPlayer_Quest quests[WOWPLAYER_QUEST_COUNT];
    WoWPlayer_VisibleItem visible_items[WOWPLAYER_VISIBLE_ITEM_COUNT];
    // Current player fields say long - client memory dump says int.
    uint64_t inventory_slot[WOWPLAYER_INVENTORY_SLOT_COUNT];
    uint64_t pack_slot[WOWPLAYER_PACK_SLOT_COUNT];
    uint64_t bank_slot[WOWPLAYER_BANK_SLOT_COUNT];
    uint64_t bank_bag_slot[WOWPLAYER_BANK_BAG_SLOT_COUNT];
    uint64_t vendor_buy_back_slot[WOWPLAYER_VENDOR_BUY_BACK_SLOT_COUNT];
    uint64_t key_ring_slot[WOWPLAYER_KEYRING_SLOT_COUNT];
    uint64_t farsight_guid;
    uint64_t field_combo_target;
    uint32_t xp;
    uint32_t next_level_xp;
    uint32_t skill_info[WOWPLAYER_SKILL_INFO_COUNT];
    uint32_t character_points_1;
    uint32_t character_points_2;
    uint32_t track_creatures;
    uint32_t track_resources;
    float block_pct;
    float dodge_pct;
    float parry_pct;
    float crit_pct;
    float ranged_crit_pct;
    uint32_t explored_zones[WOWPLAYER_EXPLORED_ZONES_COUNT];
    uint32_t rest_state_xp;
    uint32_t field_coinage;
    uint32_t pos_stat[WOWPLAYER_STAT_COUNT];
    uint32_t neg_stat[WOWPLAYER_STAT_COUNT];
    uint32_t resistance_buff_mod_pos[WOWPLAYER_SPELL_SCHOOL_COUNT];
    uint32_t resistance_buff_mod_neg[WOWPLAYER_SPELL_SCHOOL_COUNT];
    uint32_t field_mod_damage_done_positive[WOWPLAYER_SPELL_SCHOOL_COUNT];
    uint32_t field_mod_damage_done_negative[WOWPLAYER_SPELL_SCHOOL_COUNT];
    float field_mod_damage_done_pct[WOWPLAYER_SPELL_SCHOOL_COUNT];
    player_field_bytes_union player_field_bytes;
    uint32_t ammo_id;
    uint32_t self_resurrection_spell;
    uint32_t field_pvp_medals;
    uint32_t field_buy_back_price[WOWPLAYER_VENDOR_BUY_BACK_SLOT_COUNT];
    uint32_t field_buy_back_timestamp[WOWPLAYER_VENDOR_BUY_BACK_SLOT_COUNT];
    uint32_t field_session_kills;
    uint32_t field_yesterday_kills;
    uint32_t field_last_week_kills;
    uint32_t field_this_week_kills;
    uint32_t field_this_week_contribution;
    uint32_t field_lifetime_honorable_kills;
    uint32_t field_lifetime_dishonorable_kills;
    uint32_t field_yersterday_contribution;
    uint32_t field_last_week_contribution;
    uint32_t field_last_week_rank;
    player_field_bytes_2_union player_field_bytes_2;
    uint32_t field_watched_faction_idx;
    uint32_t field_combat_rating[WOWPLAYER_COMBAT_RATING_COUNT];
};
#elif VERSION_STRING == TBC
#define WOWPLAYER_QUEST_COUNT 25
#define WOWPLAYER_VISIBLE_ITEM_COUNT 19
#define WOWPLAYER_VISIBLE_ITEM_0_COUNT 12
#define WOWPLAYER_INVENTORY_SLOT_COUNT 23
#define WOWPLAYER_PACK_SLOT_COUNT 16
#define WOWPLAYER_BANK_SLOT_COUNT 28
#define WOWPLAYER_BANK_BAG_SLOT_COUNT 7
#define WOWPLAYER_VENDOR_BUY_BACK_SLOT_COUNT 12
#define WOWPLAYER_KEYRING_SLOT_COUNT 32
#define WOWPLAYER_VANITY_PET_SLOT_COUNT 18
#define WOWPLAYER_SKILL_INFO_COUNT 384
#define WOWPLAYER_SPELL_SCHOOL_COUNT 7
#define WOWPLAYER_EXPLORED_ZONES_COUNT 128
#define WOWPLAYER_BUY_BACK_COUNT 12
#define WOWPLAYER_COMBAT_RATING_COUNT 24
#define WOWPLAYER_ARENA_TEAM_INFO_COUNT 18
#define WOWPLAYER_DAILY_QUESTS_COUNT 25

struct WoWPlayer_Quest
{
    uint32_t unk1;  // id
    uint32_t unk2;  // state
    uint64_t unk3;  // 4 x uint16_t mob_or_go_count
};

struct WoWPlayer_VisibleItem
{
    uint64_t creator;
    uint32_t visible_items[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t properties;
    uint32_t padding;
};

struct WoWPlayer : WoWUnit
{
    uint64_t duel_arbiter;
    uint32_t player_flags;
    uint32_t guild_id;
    uint32_t guild_rank;
    player_bytes_union player_bytes;
    player_bytes_2_union player_bytes_2;
    player_bytes_3_union player_bytes_3;
    uint32_t duel_team;
    uint32_t guild_timestamp;
    WoWPlayer_Quest quests[WOWPLAYER_QUEST_COUNT];
    WoWPlayer_VisibleItem visible_items[WOWPLAYER_VISIBLE_ITEM_COUNT];
    uint32_t chosen_title;
    uint32_t player_padding_0;
    // Current player fields say long - client memory dump says int.
    uint64_t inventory_slot[WOWPLAYER_INVENTORY_SLOT_COUNT];
    uint64_t pack_slot[WOWPLAYER_PACK_SLOT_COUNT];
    uint64_t bank_slot[WOWPLAYER_BANK_SLOT_COUNT];
    uint64_t bank_bag_slot[WOWPLAYER_BANK_BAG_SLOT_COUNT];
    uint64_t vendor_buy_back_slot[WOWPLAYER_VENDOR_BUY_BACK_SLOT_COUNT];
    uint64_t key_ring_slot[WOWPLAYER_KEYRING_SLOT_COUNT];
    uint64_t vanity_pet_slot[WOWPLAYER_VANITY_PET_SLOT_COUNT];
    uint64_t farsight_guid;
    uint64_t known_titles;
    uint32_t xp;
    uint32_t next_level_xp;
    uint32_t skill_info[WOWPLAYER_SKILL_INFO_COUNT];
    uint32_t character_points_1;
    uint32_t character_points_2;
    uint32_t track_creatures;
    uint32_t track_resources;
    float block_pct;
    float dodge_pct;
    float parry_pct;
    uint32_t expertise;
    uint32_t offhand_expertise;
    float crit_pct;
    float ranged_crit_pct;
    float offhand_crit_pct;
    float spell_crit_pct[WOWPLAYER_SPELL_SCHOOL_COUNT];
    uint32_t shield_block;
    uint32_t explored_zones[WOWPLAYER_EXPLORED_ZONES_COUNT];
    uint32_t rest_state_xp;
    uint32_t field_coinage;
    uint32_t field_mod_damage_done_positive[WOWPLAYER_SPELL_SCHOOL_COUNT];
    uint32_t field_mod_damage_done_negative[WOWPLAYER_SPELL_SCHOOL_COUNT];
    float field_mod_damage_done_pct[WOWPLAYER_SPELL_SCHOOL_COUNT];
    uint32_t field_mod_healing_done;
    uint32_t field_mod_target_resistance;
    uint32_t field_mod_target_physical_resistance;
    player_field_bytes_union player_field_bytes;
    uint32_t ammo_id;
    uint32_t self_resurrection_spell;
    uint32_t field_pvp_medals;
    uint32_t field_buy_back_price[WOWPLAYER_BUY_BACK_COUNT];
    uint32_t field_buy_back_timestamp[WOWPLAYER_BUY_BACK_COUNT];
    uint32_t field_kills;
    uint32_t field_contribution_today;
    uint32_t field_contribution_yesterday;
    uint32_t field_lifetime_honorable_kills;
    player_field_bytes_2_union player_field_bytes_2;
    uint32_t field_watched_faction_idx;
    uint32_t field_combat_rating[WOWPLAYER_COMBAT_RATING_COUNT];
    uint32_t field_arena_team_info[WOWPLAYER_ARENA_TEAM_INFO_COUNT];
    uint32_t field_honor_currency;
    uint32_t field_arena_currency;
    float field_mod_mana_regen;
    float field_mod_mana_regen_interrupt;
    uint32_t field_max_level;
    uint32_t field_daily_quests[WOWPLAYER_DAILY_QUESTS_COUNT];
};
#elif VERSION_STRING == WotLK
#define WOWPLAYER_QUEST_COUNT 25
#define WOWPLAYER_VISIBLE_ITEM_COUNT 19
#define WOWPLAYER_EXPLORED_ZONES_COUNT 128
#define WOWPLAYER_INVENTORY_SLOT_COUNT 23
#define WOWPLAYER_PACK_SLOT_COUNT 16
#define WOWPLAYER_BANK_SLOT_COUNT 28
#define WOWPLAYER_BANK_BAG_SLOT_COUNT 7
#define WOWPLAYER_KEYRING_SLOT_COUNT 32
#define WOWPLAYER_CURRENCY_TOKEN_SLOT_COUNT 32
#define WOWPLAYER_SKILL_INFO_COUNT 384
#define WOWPLAYER_SPELL_SCHOOL_COUNT 7
#define WOWPLAYER_BUY_BACK_COUNT 12
#define WOWPLAYER_COMBAT_RATING_COUNT 25
#define WOWPLAYER_ARENA_TEAM_INFO_COUNT 21
#define WOWPLAYER_DAILY_QUESTS_COUNT 25
#define WOWPLAYER_RUNE_REGEN_COUNT 4
#define WOWPLAYER_NO_REAGENT_COST_COUNT 3
#define WOWPLAYER_GLYPH_SLOT_COUNT 6

struct WoWPlayer_Quest
{
    uint32_t unk1;  // id
    uint32_t unk2;  // state
    uint64_t unk3;  // 4 x uint16_t mob_or_go_count
    uint32_t unk5;  // time
};

struct WoWPlayer_VisibleItem
{
    uint32_t entry;
    uint32_t enchantment;
};

struct WoWPlayer : WoWUnit
{
    uint64_t duel_arbiter;
    uint32_t player_flags;
    uint32_t guild_id;
    uint32_t guild_rank;
    player_bytes_union player_bytes;
    player_bytes_2_union player_bytes_2;
    player_bytes_3_union player_bytes_3;
    uint32_t duel_team;
    uint32_t guild_timestamp;
    WoWPlayer_Quest quests[WOWPLAYER_QUEST_COUNT];
    WoWPlayer_VisibleItem visible_items[WOWPLAYER_VISIBLE_ITEM_COUNT];
    uint32_t chosen_title;
    uint32_t inebriation;
    uint32_t player_padding_0;
    uint64_t inventory_slot[WOWPLAYER_INVENTORY_SLOT_COUNT];
    uint64_t pack_slot[WOWPLAYER_PACK_SLOT_COUNT];
    uint64_t bank_slot[WOWPLAYER_BANK_SLOT_COUNT];
    uint64_t bank_bag_slot[WOWPLAYER_BANK_BAG_SLOT_COUNT];
    uint64_t vendor_buy_back_slot[WOWPLAYER_BUY_BACK_COUNT];
    uint64_t key_ring_slot[WOWPLAYER_KEYRING_SLOT_COUNT];
    uint64_t currencytoken_slot[WOWPLAYER_CURRENCY_TOKEN_SLOT_COUNT];
    uint64_t farsight_guid;
    uint64_t field_known_titles;
    uint64_t field_known_titles1;
    uint64_t field_known_titles2;
    uint64_t field_known_currencies;
    uint32_t xp;
    uint32_t next_level_xp;
    uint32_t skill_info[WOWPLAYER_SKILL_INFO_COUNT];
    uint32_t character_points_1;
    uint32_t character_points_2;
    uint32_t track_creatures;
    uint32_t track_resources;
    float block_pct;
    float dodge_pct;
    float parry_pct;
    uint32_t expertise;
    uint32_t offhand_expertise;
    float crit_pct;
    float ranged_crit_pct;
    float offhand_crit_pct;
    float spell_crit_pct[WOWPLAYER_SPELL_SCHOOL_COUNT];
    uint32_t shield_block;
    uint32_t shield_block_crit_pct;
    uint32_t explored_zones[WOWPLAYER_EXPLORED_ZONES_COUNT];
    uint32_t rest_state_xp;
    uint32_t field_coinage;
    uint32_t field_mod_damage_done_positive[WOWPLAYER_SPELL_SCHOOL_COUNT];
    uint32_t field_mod_damage_done_negative[WOWPLAYER_SPELL_SCHOOL_COUNT];
    float field_mod_damage_done_pct[WOWPLAYER_SPELL_SCHOOL_COUNT];
    uint32_t field_mod_healing_done;
    uint32_t field_mod_healing_pct;
    uint32_t field_mod_healing_done_pct;
    uint32_t field_mod_target_resistance;
    uint32_t field_mod_target_physical_resistance;
    player_field_bytes_union player_field_bytes;
    uint32_t ammo_id;
    uint32_t self_resurrection_spell;
    uint32_t field_pvp_medals;
    uint32_t field_buy_back_price[WOWPLAYER_BUY_BACK_COUNT];
    uint32_t field_buy_back_timestamp[WOWPLAYER_BUY_BACK_COUNT];
    uint32_t field_kills;
    uint32_t field_contribution_today;
    uint32_t field_contribution_yesterday;
    uint32_t field_lifetime_honorable_kills;
    player_field_bytes_2_union player_field_bytes_2;
    uint32_t field_watched_faction_idx;
    uint32_t field_combat_rating[WOWPLAYER_COMBAT_RATING_COUNT];
    uint32_t field_arena_team_info[WOWPLAYER_ARENA_TEAM_INFO_COUNT];
    uint32_t field_honor_currency;
    uint32_t field_arena_currency;
    uint32_t field_max_level;
    uint32_t field_daily_quests[WOWPLAYER_DAILY_QUESTS_COUNT];
    uint32_t rune_regen[WOWPLAYER_RUNE_REGEN_COUNT];
    uint32_t no_reagent_cost[WOWPLAYER_NO_REAGENT_COST_COUNT];
    uint32_t field_glyph_slots[WOWPLAYER_GLYPH_SLOT_COUNT];
    uint32_t field_glyphs[6];
    uint32_t glyphs_enabled;
    uint32_t pet_spell_power;
};
#elif VERSION_STRING == Cata
#define WOWPLAYER_EXPLORED_ZONES_COUNT 156
#define WOWPLAYER_SPELL_SCHOOL_COUNT 7
#define WOWPLAYER_BUY_BACK_COUNT 12
#define WOWPLAYER_DAILY_QUESTS_COUNT 25
#define WOWPLAYER_QUEST_COUNT 50
#define WOWPLAYER_VISIBLE_ITEM_COUNT 19
#define WOWPLAYER_INVENTORY_SLOT_COUNT 23
#define WOWPLAYER_PACK_SLOT_COUNT 16
#define WOWPLAYER_BANK_SLOT_COUNT 28
#define WOWPLAYER_BANK_BAG_SLOT_COUNT 7
#define WOWPLAYER_KEYRING_SLOT_COUNT 32
#define WOWPLAYER_CURRENCY_TOKEN_SLOT_COUNT 32

struct WoWPlayer_Quest
{
    uint32_t unk1;  // id
    uint32_t unk2;  // state
    uint64_t unk3;  // 4 x uint16_t mob_or_go_count
    uint32_t unk5;  // time
};

struct WoWPlayer_VisibleItem
{
    uint32_t entry;
    uint32_t enchantment;
};

struct WoWPlayer : WoWUnit
{
    uint64_t duel_arbiter;
    uint32_t player_flags;
    uint32_t guild_rank;
    uint32_t guild_delete_date;
    uint32_t guild_level;
    player_bytes_union player_bytes;
    player_bytes_2_union player_bytes_2;
    player_bytes_3_union player_bytes_3;
    uint32_t duel_team;
    uint32_t guild_timestamp;
    WoWPlayer_Quest quests[WOWPLAYER_QUEST_COUNT];
    WoWPlayer_VisibleItem visible_items[WOWPLAYER_VISIBLE_ITEM_COUNT];
    uint32_t chosen_title;
    uint32_t inebriation;
    uint32_t player_padding_0;
    uint64_t inventory_slot[WOWPLAYER_INVENTORY_SLOT_COUNT];
    uint64_t pack_slot[WOWPLAYER_PACK_SLOT_COUNT];
    uint64_t bank_slot[WOWPLAYER_BANK_SLOT_COUNT];
    uint64_t bank_bag_slot[WOWPLAYER_BANK_BAG_SLOT_COUNT];
    uint64_t vendor_buy_back_slot[WOWPLAYER_BUY_BACK_COUNT];
    uint64_t farsight_guid;
    uint64_t known_titles;
    uint64_t known_titles1;
    uint64_t known_titles2;
    uint64_t known_titles3;
    uint32_t xp;
    uint32_t next_level_xp;
    uint32_t skill_line[64];
    uint32_t skill_step[64];
    uint32_t skill_rank[64];
    uint32_t skill_max_rank[64];
    uint32_t skill_mod[64];
    uint32_t skill_talent[64];
    uint32_t character_points_1;
    uint32_t track_creatures;
    uint32_t track_resources;
    uint32_t expertise;
    uint32_t offhand_expertise;
    float block_pct;
    float dodge_pct;
    float parry_pct;
    float crit_pct;
    float ranged_crit_pct;
    float offhand_crit_pct;
    float spell_crit_pct[WOWPLAYER_SPELL_SCHOOL_COUNT];
    uint32_t shield_block;
    uint32_t shield_block_crit_pct;
    uint32_t mastery;
    uint32_t explored_zones[WOWPLAYER_EXPLORED_ZONES_COUNT];
    uint32_t rest_state_xp;
    uint64_t field_coinage;
    uint32_t field_mod_damage_done_positive[WOWPLAYER_SPELL_SCHOOL_COUNT];
    uint32_t field_mod_damage_done_negative[WOWPLAYER_SPELL_SCHOOL_COUNT];
    float field_mod_damage_done_pct[WOWPLAYER_SPELL_SCHOOL_COUNT];
    uint32_t field_mod_healing_done;
    float field_mod_healing_pct;
    float field_mod_healing_done_pct;
    float weapon_dmg_multiplier[3];
    float mod_spell_power_pct;
    float override_spell_power_by_ap_pct;
    uint32_t field_mod_target_resistance;
    uint32_t field_mod_target_physical_resistance;
    player_field_bytes_union player_field_bytes;
    uint32_t self_resurrection_spell;
    uint32_t field_pvp_medals;
    uint32_t field_buy_back_price[WOWPLAYER_BUY_BACK_COUNT];
    uint32_t field_buy_back_timestamp[WOWPLAYER_BUY_BACK_COUNT];
    uint32_t field_kills;
    uint32_t field_lifetime_honorable_kills;
    player_field_bytes_2_union player_field_bytes_2;
    uint32_t field_watched_faction_idx;
    uint32_t field_combat_rating[26];
    uint32_t field_arena_team_info[21];
    uint32_t battleground_rating;
    uint32_t field_max_level;
    uint32_t field_daily_quests[WOWPLAYER_DAILY_QUESTS_COUNT];
    float rune_regen[4];
    uint32_t no_reagent_cost[3];
    uint32_t field_glyph_slots[9];
    uint32_t field_glyphs[9];
    uint32_t glyphs_enabled;
    uint32_t pet_spell_power;
    uint32_t researching[8];
    uint32_t research_site[8];
    uint32_t profession_skill_line[2];
    float ui_hit_mod;
    float ui_hit_spell_mod;
    uint32_t ui_home_realm_time_offset;
    float mod_haste;
    float mod_ranged_haste;
    float mod_pet_haste;
    float mod_haste_regen;
};
#elif VERSION_STRING == Mop
#define WOWPLAYER_EXPLORED_ZONES_COUNT 200
#define WOWPLAYER_SPELL_SCHOOL_COUNT 7
#define WOWPLAYER_BUY_BACK_COUNT 12
#define WOWPLAYER_DAILY_QUESTS_COUNT 25
#define WOWPLAYER_QUEST_COUNT 150
#define WOWPLAYER_VISIBLE_ITEM_COUNT 19
#define WOWPLAYER_INVENTORY_SLOT_COUNT 23
#define WOWPLAYER_PACK_SLOT_COUNT 16
#define WOWPLAYER_BANK_SLOT_COUNT 28
#define WOWPLAYER_BANK_BAG_SLOT_COUNT 7
#define WOWPLAYER_KEYRING_SLOT_COUNT 32
#define WOWPLAYER_CURRENCY_TOKEN_SLOT_COUNT 32

struct WoWPlayer_Quest
{
    uint32_t unk1;  // id
    uint32_t unk2;  // state
    uint64_t unk3;  // 4 x uint16_t mob_or_go_count
    uint32_t unk5;  // time
};

struct WoWPlayer_VisibleItem
{
    uint32_t entry;
    uint32_t enchantment;
};

struct WoWPlayer : WoWUnit
{
    uint64_t duel_arbiter;
    uint32_t player_flags;
    uint32_t guild_rank;
    uint32_t guild_delete_date;
    uint32_t guild_level;
    player_bytes_union player_bytes;
    player_bytes_2_union player_bytes_2;
    player_bytes_3_union player_bytes_3;
    uint32_t duel_team;
    uint32_t guild_timestamp;
    WoWPlayer_Quest quests[WOWPLAYER_QUEST_COUNT];
    WoWPlayer_VisibleItem visible_items[WOWPLAYER_VISIBLE_ITEM_COUNT];
    uint32_t chosen_title;
    uint32_t inebriation;
    uint32_t virtual_player_realm;
    uint32_t current_spec_id;
    uint32_t taxi_mount_anim_kit_id;
    uint32_t current_battle_pet_breed_quality;
    uint64_t inventory_slot[WOWPLAYER_INVENTORY_SLOT_COUNT];
    uint64_t pack_slot[WOWPLAYER_PACK_SLOT_COUNT];
    uint64_t bank_slot[WOWPLAYER_BANK_SLOT_COUNT];
    uint64_t bank_bag_slot[WOWPLAYER_BANK_BAG_SLOT_COUNT];
    uint64_t vendor_buy_back_slot[WOWPLAYER_BUY_BACK_COUNT];
    uint64_t farsight_guid;
    uint64_t known_titles;
    uint64_t known_titles1;
    uint64_t known_titles2;
    uint64_t known_titles3;
    uint64_t known_titles4;
    uint64_t field_coinage;
    uint32_t xp;
    uint32_t next_level_xp;
    uint32_t skill_line[64];
    uint32_t skill_step[64];
    uint32_t skill_rank[64];
    uint32_t skill_starting_rank[64];
    uint32_t skill_max_rank[64];
    uint32_t skill_mod[64];
    uint32_t skill_talent[64];
    uint32_t character_points_1;
    uint32_t max_talent_tiers;
    uint32_t track_creatures;
    uint32_t track_resources;
    uint32_t expertise;
    uint32_t offhand_expertise;
    uint32_t ranged_expertise;
    uint32_t combat_rating_expertise;
    float block_pct;
    float dodge_pct;
    float parry_pct;
    float crit_pct;
    float ranged_crit_pct;
    float offhand_crit_pct;
    float spell_crit_pct[WOWPLAYER_SPELL_SCHOOL_COUNT];
    uint32_t shield_block;
    uint32_t shield_block_crit_pct;
    uint32_t mastery;
    uint32_t pvp_power_damage;
    uint32_t pvp_power_healing;
    uint32_t explored_zones[WOWPLAYER_EXPLORED_ZONES_COUNT];
    uint32_t rest_state_xp;
    uint32_t field_mod_damage_done_positive[WOWPLAYER_SPELL_SCHOOL_COUNT];
    uint32_t field_mod_damage_done_negative[WOWPLAYER_SPELL_SCHOOL_COUNT];
    float field_mod_damage_done_pct[WOWPLAYER_SPELL_SCHOOL_COUNT];
    uint32_t field_mod_healing_done;
    float field_mod_healing_pct;
    float field_mod_healing_done_pct;
    float field_mod_periodic_healing_done_pct;
    float weapon_dmg_multiplier[3];
    float mod_spell_power_pct;
    float mod_resilience_pct;
    float override_spell_power_by_ap_pct;
    float override_ap_by_spell_power_pct;
    uint32_t field_mod_target_resistance;
    uint32_t field_mod_target_physical_resistance;
    player_field_bytes_union player_field_bytes;
    uint32_t self_resurrection_spell;
    uint32_t field_pvp_medals;
    uint32_t field_buy_back_price[WOWPLAYER_BUY_BACK_COUNT];
    uint32_t field_buy_back_timestamp[WOWPLAYER_BUY_BACK_COUNT];
    uint32_t field_kills;
    uint32_t field_lifetime_honorable_kills;
    uint32_t field_watched_faction_idx;
    uint32_t field_combat_rating[27];
    uint32_t field_arena_team_info[24];
    uint32_t field_max_level;
    float rune_regen[4];
    uint32_t no_reagent_cost[4];
    uint32_t field_glyph_slots[6];
    uint32_t field_glyphs[6];
    uint32_t glyphs_enabled;
    uint32_t pet_spell_power;
    uint32_t researching[8];
    uint32_t profession_skill_line[2];
    float ui_hit_mod;
    float ui_hit_spell_mod;
    uint32_t ui_home_realm_time_offset;
    float mod_pet_haste;
    uint64_t summoned_battle_pet_guid;
    uint32_t override_spell_id;
    uint32_t lfg_bonus_faction_id;
    uint32_t loot_spec_id;
    uint32_t override_zone_pvp_type;
    uint32_t item_level_delta;
};
#endif
#pragma pack(pop)
