/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

// These are listed as uint32_t but I'm pretty sure they're uint64_t with half the field count
// TODO: Verify. -Evairfairy
//#define WOWPLAYER_INVENTORY_SLOT_COUNT 46
//#define WOWPLAYER_PACK_SLOT_COUNT 32
//#define WOWPLAYER_BANK_SLOT_COUNT 56
//#define WOWPLAYER_BANK_BAG_SLOT_COUNT 14
//#define WOWPLAYER_VENDOR_BUY_BACK_SLOT_COUNT 24
//#define WOWPLAYER_KEYRING_SLOT_COUNT 64
//#define WOWPLAYER_VANITY_PET_SLOT_COUNT 36

// Adjusted values.
#if VERSION_STRING == Classic
#define WOWPLAYER_VISIBLE_ITEM_0_COUNT 8
#define WOWPLAYER_INVENTORY_SLOT_COUNT 23
#define WOWPLAYER_PACK_SLOT_COUNT 16
#define WOWPLAYER_BANK_SLOT_COUNT 24
#define WOWPLAYER_BANK_BAG_SLOT_COUNT 6

#define WOWPLAYER_EXPLORED_ZONES_COUNT 64
#define WOWPLAYER_COMBAT_RATING_COUNT 20

#elif VERSION_STRING == TBC
#define WOWPLAYER_VISIBLE_ITEM_0_COUNT 12
#define WOWPLAYER_INVENTORY_SLOT_COUNT 23
#define WOWPLAYER_PACK_SLOT_COUNT 16
#define WOWPLAYER_BANK_SLOT_COUNT 28
#define WOWPLAYER_BANK_BAG_SLOT_COUNT 7

#define WOWPLAYER_VANITY_PET_SLOT_COUNT 18

#define WOWPLAYER_EXPLORED_ZONES_COUNT 128
#define WOWPLAYER_COMBAT_RATING_COUNT 24

#elif VERSION_STRING == WotLK
#define WOWPLAYER_EXPLORED_ZONES_COUNT 128

#elif VERSION_STRING == Cata
#define WOWPLAYER_EXPLORED_ZONES_COUNT 156
#endif

#define WOWPLAYER_VENDOR_BUY_BACK_SLOT_COUNT 12
#define WOWPLAYER_KEYRING_SLOT_COUNT 32
#define WOWPLAYER_SKILL_INFO_COUNT 384

#define WOWPLAYER_SPELL_CRIT_PCT_COUNT 7

#define WOWPLAYER_MOD_DAMAGE_DONE_COUNT 7
#define WOWPLAYER_BUY_BACK_COUNT 12

#define WOWPLAYER_ARENA_TEAM_INFO_COUNT 18
#define WOWPLAYER_DAILY_QUESTS_COUNT 25

#pragma pack(push, 1)
#if VERSION_STRING <= TBC
struct WoWPlayer : WoWUnit
{
    uint64_t duel_arbiter;
    uint32_t player_flags;
    uint32_t guild_id;
    uint32_t guild_rank;
    uint32_t player_bytes;
    uint32_t player_bytes_2;
    uint32_t player_bytes_3;
    uint32_t duel_team;
    uint32_t guild_timestamp;
    uint32_t quest_log_1_1;
    uint32_t quest_log_1_2;
    uint32_t quest_log_1_3;
    uint32_t quest_log_1_4;
    uint32_t quest_log_2_1;
    uint32_t quest_log_2_2;
    uint32_t quest_log_2_3;
    uint32_t quest_log_2_4;
    uint32_t quest_log_3_1;
    uint32_t quest_log_3_2;
    uint32_t quest_log_3_3;
    uint32_t quest_log_3_4;
    uint32_t quest_log_4_1;
    uint32_t quest_log_4_2;
    uint32_t quest_log_4_3;
    uint32_t quest_log_4_4;
    uint32_t quest_log_5_1;
    uint32_t quest_log_5_2;
    uint32_t quest_log_5_3;
    uint32_t quest_log_5_4;
    uint32_t quest_log_6_1;
    uint32_t quest_log_6_2;
    uint32_t quest_log_6_3;
    uint32_t quest_log_6_4;
    uint32_t quest_log_7_1;
    uint32_t quest_log_7_2;
    uint32_t quest_log_7_3;
    uint32_t quest_log_7_4;
    uint32_t quest_log_8_1;
    uint32_t quest_log_8_2;
    uint32_t quest_log_8_3;
    uint32_t quest_log_8_4;
    uint32_t quest_log_9_1;
    uint32_t quest_log_9_2;
    uint32_t quest_log_9_3;
    uint32_t quest_log_9_4;
    uint32_t quest_log_10_1;
    uint32_t quest_log_10_2;
    uint32_t quest_log_10_3;
    uint32_t quest_log_10_4;
    uint32_t quest_log_11_1;
    uint32_t quest_log_11_2;
    uint32_t quest_log_11_3;
    uint32_t quest_log_11_4;
    uint32_t quest_log_12_1;
    uint32_t quest_log_12_2;
    uint32_t quest_log_12_3;
    uint32_t quest_log_12_4;
    uint32_t quest_log_13_1;
    uint32_t quest_log_13_2;
    uint32_t quest_log_13_3;
    uint32_t quest_log_13_4;
    uint32_t quest_log_14_1;
    uint32_t quest_log_14_2;
    uint32_t quest_log_14_3;
    uint32_t quest_log_14_4;
    uint32_t quest_log_15_1;
    uint32_t quest_log_15_2;
    uint32_t quest_log_15_3;
    uint32_t quest_log_15_4;
#if VERSION_STRING > Classic
    uint32_t quest_log_16_1;
    uint32_t quest_log_16_2;
    uint32_t quest_log_16_3;
    uint32_t quest_log_16_4;
    uint32_t quest_log_17_1;
    uint32_t quest_log_17_2;
    uint32_t quest_log_17_3;
    uint32_t quest_log_17_4;
    uint32_t quest_log_18_1;
    uint32_t quest_log_18_2;
    uint32_t quest_log_18_3;
    uint32_t quest_log_18_4;
    uint32_t quest_log_19_1;
    uint32_t quest_log_19_2;
    uint32_t quest_log_19_3;
    uint32_t quest_log_19_4;
    uint32_t quest_log_20_1;
    uint32_t quest_log_20_2;
    uint32_t quest_log_20_3;
    uint32_t quest_log_20_4;
    uint32_t quest_log_21_1;
    uint32_t quest_log_21_2;
    uint32_t quest_log_21_3;
    uint32_t quest_log_21_4;
    uint32_t quest_log_22_1;
    uint32_t quest_log_22_2;
    uint32_t quest_log_22_3;
    uint32_t quest_log_22_4;
    uint32_t quest_log_23_1;
    uint32_t quest_log_23_2;
    uint32_t quest_log_23_3;
    uint32_t quest_log_23_4;
    uint32_t quest_log_24_1;
    uint32_t quest_log_24_2;
    uint32_t quest_log_24_3;
    uint32_t quest_log_24_4;
    uint32_t quest_log_25_1;
    uint32_t quest_log_25_2;
    uint32_t quest_log_25_3;
    uint32_t quest_log_25_4;
#endif
    uint64_t visible_item_1_creator;
    uint32_t visible_item_1_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_1_properties;
    uint32_t visible_item_1_padding;
    uint64_t visible_item_2_creator;
    uint32_t visible_item_2_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_2_properties;
    uint32_t visible_item_2_padding;
    uint64_t visible_item_3_creator;
    uint32_t visible_item_3_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_3_properties;
    uint32_t visible_item_3_padding;
    uint64_t visible_item_4_creator;
    uint32_t visible_item_4_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_4_properties;
    uint32_t visible_item_4_padding;
    uint64_t visible_item_5_creator;
    uint32_t visible_item_5_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_5_properties;
    uint32_t visible_item_5_padding;
    uint64_t visible_item_6_creator;
    uint32_t visible_item_6_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_6_properties;
    uint32_t visible_item_6_padding;
    uint64_t visible_item_7_creator;
    uint32_t visible_item_7_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_7_properties;
    uint32_t visible_item_7_padding;
    uint64_t visible_item_8_creator;
    uint32_t visible_item_8_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_8_properties;
    uint32_t visible_item_8_padding;
    uint64_t visible_item_9_creator;
    uint32_t visible_item_9_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_9_properties;
    uint32_t visible_item_9_padding;
    uint64_t visible_item_10_creator;
    uint32_t visible_item_10_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_10_properties;
    uint32_t visible_item_10_padding;
    uint64_t visible_item_11_creator;
    uint32_t visible_item_11_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_11_properties;
    uint32_t visible_item_11_padding;
    uint64_t visible_item_12_creator;
    uint32_t visible_item_12_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_12_properties;
    uint32_t visible_item_12_padding;
    uint64_t visible_item_13_creator;
    uint32_t visible_item_13_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_13_properties;
    uint32_t visible_item_13_padding;
    uint64_t visible_item_14_creator;
    uint32_t visible_item_14_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_14_properties;
    uint32_t visible_item_14_padding;
    uint64_t visible_item_15_creator;
    uint32_t visible_item_15_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_15_properties;
    uint32_t visible_item_15_padding;
    uint64_t visible_item_16_creator;
    uint32_t visible_item_16_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_16_properties;
    uint32_t visible_item_16_padding;
    uint64_t visible_item_17_creator;
    uint32_t visible_item_17_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_17_properties;
    uint32_t visible_item_17_padding;
    uint64_t visible_item_18_creator;
    uint32_t visible_item_18_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_18_properties;
    uint32_t visible_item_18_padding;
    uint64_t visible_item_19_creator;
    uint32_t visible_item_19_0[WOWPLAYER_VISIBLE_ITEM_0_COUNT];
    uint32_t visible_item_19_properties;
    uint32_t visible_item_19_padding;
#if VERSION_STRING == TBC
    uint32_t chosen_title;
    uint32_t player_padding_0;
#endif
    // Current player fields say long - client memory dump says int.
    uint64_t inventory_slot[WOWPLAYER_INVENTORY_SLOT_COUNT];
    uint64_t pack_slot[WOWPLAYER_PACK_SLOT_COUNT];
    uint64_t bank_slot[WOWPLAYER_BANK_SLOT_COUNT];
    uint64_t bank_bag_slot[WOWPLAYER_BANK_BAG_SLOT_COUNT];
    uint64_t vendor_buy_back_slot[WOWPLAYER_VENDOR_BUY_BACK_SLOT_COUNT];
    uint64_t key_ring_slot[WOWPLAYER_KEYRING_SLOT_COUNT];
#if VERSION_STRING == TBC
    uint64_t vanity_pet_slot[WOWPLAYER_VANITY_PET_SLOT_COUNT];
#endif
    uint64_t farsight_guid;
    uint64_t known_titles;
    uint32_t xp;
    uint32_t next_level_xp;
    uint32_t skill_info[WOWPLAYER_SKILL_INFO_COUNT];
    uint32_t character_points_1;
    uint32_t character_points_2;
    uint32_t track_creatures;
    uint32_t track_resources;
    float_t block_pct;
    float_t dodge_pct;
    float_t parry_pct;
#if VERSION_STRING == TBC
    uint32_t expertise;
    uint32_t offhand_expertise;
#endif
    float_t crit_pct;
    float_t ranged_crit_pct;
#if VERSION_STRING == TBC
    float_t offhand_crit_pct;
    float_t spell_crit_pct[WOWPLAYER_SPELL_CRIT_PCT_COUNT];
    uint32_t shield_block;
#endif
    uint32_t explored_zones[WOWPLAYER_EXPLORED_ZONES_COUNT];
    uint32_t rest_state_xp;
    uint32_t field_coinage;
#if VERSION_STRING == Classic
    uint32_t pos_stat[4];
    uint32_t neg_stat[4];
    uint32_t resistance_buff_mod_pos[7];
    uint32_t resistance_buff_mod_neg[7];
#endif
    uint32_t field_mod_damage_done_positive[WOWPLAYER_MOD_DAMAGE_DONE_COUNT];
    uint32_t field_mod_damage_done_negative[WOWPLAYER_MOD_DAMAGE_DONE_COUNT];
    // Listed as an int... but this seems like it would be a float? TODO: Verify
    uint32_t field_mod_damage_done_pct[WOWPLAYER_MOD_DAMAGE_DONE_COUNT];
#if VERSION_STRING == TBC
    uint32_t field_mod_healing_done;
    uint32_t field_mod_target_resistance;
    uint32_t field_mod_target_physical_resistance;
#endif
    uint32_t field_bytes;
    uint32_t ammo_id;
    uint32_t self_resurrection_spell;
    uint32_t field_pvp_medals;
    uint32_t field_buy_back_price[WOWPLAYER_BUY_BACK_COUNT];
    uint32_t field_buy_back_timestamp[WOWPLAYER_BUY_BACK_COUNT];
    uint32_t field_kills;
#if VERSION_STRING == Classic
    uint32_t field_yesterday_kills;
    uint32_t field_last_week_kills;
    uint32_t field_this_week_kills;
    uint32_t field_this_week_contribution;
#endif
#if VERSION_STRING == TBC
    uint32_t field_contribution_today;
    uint32_t field_contribution_yesterday;
#endif
    uint32_t field_lifetime_honorable_kills;
#if VERSION_STRING == Classic
    uint32_t field_lifetime_dishonorable_kills;
    uint32_t field_yersterday_contribution;
    uint32_t field_last_week_contribution;
    uint32_t field_last_week_rank;
#endif
    uint32_t player_field_bytes_2;
    uint32_t field_watched_faction_idx;
    uint32_t field_combat_rating[WOWPLAYER_COMBAT_RATING_COUNT];
#if VERSION_STRING == TBC
    uint32_t field_arena_team_info[WOWPLAYER_ARENA_TEAM_INFO_COUNT];
    uint32_t field_honor_currency;
    uint32_t field_arena_currency;
    float_t field_mod_mana_regen;
    float_t field_mod_mana_regen_interrupt;
    uint32_t field_max_level;
    uint32_t field_daily_quests[WOWPLAYER_DAILY_QUESTS_COUNT];
#endif
};
#elif VERSION_STRING == WotLK
struct WoWPlayer : WoWUnit
{
    uint64_t duel_arbiter;
    uint32_t player_flags;
    uint32_t guild_id;
    uint32_t guild_rank;
    uint32_t player_bytes;
    uint32_t player_bytes_2;
    uint32_t player_bytes_3;
    uint32_t duel_team;
    uint32_t guild_timestamp;
    uint32_t quest_log_1_1;
    uint32_t quest_log_1_2;
    uint32_t quest_log_1_3;
    uint32_t quest_log_1_4;
    uint32_t quest_log_1_5;
    uint32_t quest_log_2_1;
    uint32_t quest_log_2_2;
    uint32_t quest_log_2_3;
    uint32_t quest_log_2_4;
    uint32_t quest_log_2_5;
    uint32_t quest_log_3_1;
    uint32_t quest_log_3_2;
    uint32_t quest_log_3_3;
    uint32_t quest_log_3_4;
    uint32_t quest_log_3_5;
    uint32_t quest_log_4_1;
    uint32_t quest_log_4_2;
    uint32_t quest_log_4_3;
    uint32_t quest_log_4_4;
    uint32_t quest_log_4_5;
    uint32_t quest_log_5_1;
    uint32_t quest_log_5_2;
    uint32_t quest_log_5_3;
    uint32_t quest_log_5_4;
    uint32_t quest_log_5_5;
    uint32_t quest_log_6_1;
    uint32_t quest_log_6_2;
    uint32_t quest_log_6_3;
    uint32_t quest_log_6_4;
    uint32_t quest_log_6_5;
    uint32_t quest_log_7_1;
    uint32_t quest_log_7_2;
    uint32_t quest_log_7_3;
    uint32_t quest_log_7_4;
    uint32_t quest_log_7_5;
    uint32_t quest_log_8_1;
    uint32_t quest_log_8_2;
    uint32_t quest_log_8_3;
    uint32_t quest_log_8_4;
    uint32_t quest_log_8_5;
    uint32_t quest_log_9_1;
    uint32_t quest_log_9_2;
    uint32_t quest_log_9_3;
    uint32_t quest_log_9_4;
    uint32_t quest_log_9_5;
    uint32_t quest_log_10_1;
    uint32_t quest_log_10_2;
    uint32_t quest_log_10_3;
    uint32_t quest_log_10_4;
    uint32_t quest_log_10_5;
    uint32_t quest_log_11_1;
    uint32_t quest_log_11_2;
    uint32_t quest_log_11_3;
    uint32_t quest_log_11_4;
    uint32_t quest_log_11_5;
    uint32_t quest_log_12_1;
    uint32_t quest_log_12_2;
    uint32_t quest_log_12_3;
    uint32_t quest_log_12_4;
    uint32_t quest_log_12_5;
    uint32_t quest_log_13_1;
    uint32_t quest_log_13_2;
    uint32_t quest_log_13_3;
    uint32_t quest_log_13_4;
    uint32_t quest_log_13_5;
    uint32_t quest_log_14_1;
    uint32_t quest_log_14_2;
    uint32_t quest_log_14_3;
    uint32_t quest_log_14_4;
    uint32_t quest_log_14_5;
    uint32_t quest_log_15_1;
    uint32_t quest_log_15_2;
    uint32_t quest_log_15_3;
    uint32_t quest_log_15_4;
    uint32_t quest_log_15_5;
    uint32_t quest_log_16_1;
    uint32_t quest_log_16_2;
    uint32_t quest_log_16_3;
    uint32_t quest_log_16_4;
    uint32_t quest_log_16_5;
    uint32_t quest_log_17_1;
    uint32_t quest_log_17_2;
    uint32_t quest_log_17_3;
    uint32_t quest_log_17_4;
    uint32_t quest_log_17_5;
    uint32_t quest_log_18_1;
    uint32_t quest_log_18_2;
    uint32_t quest_log_18_3;
    uint32_t quest_log_18_4;
    uint32_t quest_log_18_5;
    uint32_t quest_log_19_1;
    uint32_t quest_log_19_2;
    uint32_t quest_log_19_3;
    uint32_t quest_log_19_4;
    uint32_t quest_log_19_5;
    uint32_t quest_log_20_1;
    uint32_t quest_log_20_2;
    uint32_t quest_log_20_3;
    uint32_t quest_log_20_4;
    uint32_t quest_log_20_5;
    uint32_t quest_log_21_1;
    uint32_t quest_log_21_2;
    uint32_t quest_log_21_3;
    uint32_t quest_log_21_4;
    uint32_t quest_log_21_5;
    uint32_t quest_log_22_1;
    uint32_t quest_log_22_2;
    uint32_t quest_log_22_3;
    uint32_t quest_log_22_4;
    uint32_t quest_log_22_5;
    uint32_t quest_log_23_1;
    uint32_t quest_log_23_2;
    uint32_t quest_log_23_3;
    uint32_t quest_log_23_4;
    uint32_t quest_log_23_5;
    uint32_t quest_log_24_1;
    uint32_t quest_log_24_2;
    uint32_t quest_log_24_3;
    uint32_t quest_log_24_4;
    uint32_t quest_log_24_5;
    uint32_t quest_log_25_1;
    uint32_t quest_log_25_2;
    uint32_t quest_log_25_3;
    uint32_t quest_log_25_4;
    uint32_t quest_log_25_5;
    uint64_t visible_item_1_entry;
    uint32_t visible_item_1_enchantment;
    uint64_t visible_item_2_entry;
    uint32_t visible_item_2_enchantment;
    uint64_t visible_item_3_entry;
    uint32_t visible_item_3_enchantment;
    uint64_t visible_item_4_entry;
    uint32_t visible_item_4_enchantment;
    uint64_t visible_item_5_entry;
    uint32_t visible_item_5_enchantment;
    uint64_t visible_item_6_entry;
    uint32_t visible_item_6_enchantment;
    uint64_t visible_item_7_entry;
    uint32_t visible_item_7_enchantment;
    uint64_t visible_item_8_entry;
    uint32_t visible_item_8_enchantment;
    uint64_t visible_item_9_entry;
    uint32_t visible_item_9_enchantment;
    uint64_t visible_item_10_entry;
    uint32_t visible_item_10_enchantment;
    uint64_t visible_item_11_entry;
    uint32_t visible_item_11_enchantment;
    uint64_t visible_item_12_entry;
    uint32_t visible_item_12_enchantment;
    uint64_t visible_item_13_entry;
    uint32_t visible_item_13_enchantment;
    uint64_t visible_item_14_entry;
    uint32_t visible_item_14_enchantment;
    uint64_t visible_item_15_entry;
    uint32_t visible_item_15_enchantment;
    uint64_t visible_item_16_entry;
    uint32_t visible_item_16_enchantment;
    uint64_t visible_item_17_entry;
    uint32_t visible_item_17_enchantment;
    uint64_t visible_item_18_entry;
    uint32_t visible_item_18_enchantment;
    uint64_t visible_item_19_entry;
    uint32_t visible_item_19_enchantment;
    uint32_t chosen_title;
    uint32_t inebriation;
    uint32_t player_padding_0;
    // Current player fields say long - client memory dump says int.
    uint64_t inventory_slot[32];
    uint64_t pack_slot[16];
    uint64_t bank_slot[28];
    uint64_t bank_bag_slot[7];
    uint64_t vendor_buy_back_slot[12];
    uint64_t key_ring_slot[32];
    uint64_t currencytoken_slot[32];
    uint64_t farsight_guid;
    uint64_t known_titles;
    uint64_t known_titles1;
    uint64_t known_titles2;
    uint64_t known_currency;
    uint32_t xp;
    uint32_t next_level_xp;
    uint32_t skill_info[WOWPLAYER_SKILL_INFO_COUNT];
    uint32_t character_points_1;
    uint32_t character_points_2;
    uint32_t track_creatures;
    uint32_t track_resources;
    float_t block_pct;
    float_t dodge_pct;
    float_t parry_pct;
    uint32_t expertise;
    uint32_t offhand_expertise;
    float_t crit_pct;
    float_t ranged_crit_pct;
    float_t offhand_crit_pct;
    float_t spell_crit_pct[WOWPLAYER_SPELL_CRIT_PCT_COUNT];
    uint32_t shield_block;
    uint32_t shield_block_crit_pct;
    uint32_t explored_zones[WOWPLAYER_EXPLORED_ZONES_COUNT];
    uint32_t rest_state_xp;
    uint32_t field_coinage;
    uint32_t field_mod_damage_done_positive[WOWPLAYER_MOD_DAMAGE_DONE_COUNT];
    uint32_t field_mod_damage_done_negative[WOWPLAYER_MOD_DAMAGE_DONE_COUNT];
    // Listed as an int... but this seems like it would be a float? TODO: Verify
    uint32_t field_mod_damage_done_pct[WOWPLAYER_MOD_DAMAGE_DONE_COUNT];
    uint32_t field_mod_healing_done;
    uint32_t field_mod_healing_pct;
    uint32_t field_mod_healing_done_pct;
    uint32_t field_mod_target_resistance;
    uint32_t field_mod_target_physical_resistance;
    uint32_t field_bytes;
    uint32_t ammo_id;
    uint32_t self_resurrection_spell;
    uint32_t field_pvp_medals;
    uint32_t field_buy_back_price[WOWPLAYER_BUY_BACK_COUNT];
    uint32_t field_buy_back_timestamp[WOWPLAYER_BUY_BACK_COUNT];
    uint32_t field_kills;
    uint32_t field_contribution_today;
    uint32_t field_contribution_yesterday;
    uint32_t field_lifetime_honorable_kills;
    uint32_t player_field_bytes_2;
    uint32_t field_watched_faction_idx;
    uint32_t field_combat_rating[25];
    uint32_t field_arena_team_info[21];
    uint32_t field_honor_currency;
    uint32_t field_arena_currency;
    uint32_t field_max_level;
    uint32_t field_daily_quests[WOWPLAYER_DAILY_QUESTS_COUNT];
    uint32_t rune_regen[4];
    uint32_t no_reagent_cost[3];
    uint32_t field_glyph_slots[6];
    uint32_t field_glyphs[6];
    uint32_t glyphs_enabled;
    uint32_t pet_spell_power;

};
#elif VERSION_STRING == Cata
struct WoWPlayer : WoWUnit
{
    uint64_t duel_arbiter;
    uint32_t player_flags;
    uint32_t guild_rank;
    uint32_t guild_delete_date;
    uint32_t guild_level;
    uint32_t player_bytes;
    uint32_t player_bytes_2;
    uint32_t player_bytes_3;
    uint32_t duel_team;
    uint32_t guild_timestamp;
    uint32_t quest_log_1_1;
    uint32_t quest_log_1_2;
    uint32_t quest_log_1_3;
    uint32_t quest_log_1_4;
    uint32_t quest_log_1_5;
    uint32_t quest_log_2_1;
    uint32_t quest_log_2_2;
    uint32_t quest_log_2_3;
    uint32_t quest_log_2_4;
    uint32_t quest_log_2_5;
    uint32_t quest_log_3_1;
    uint32_t quest_log_3_2;
    uint32_t quest_log_3_3;
    uint32_t quest_log_3_4;
    uint32_t quest_log_3_5;
    uint32_t quest_log_4_1;
    uint32_t quest_log_4_2;
    uint32_t quest_log_4_3;
    uint32_t quest_log_4_4;
    uint32_t quest_log_4_5;
    uint32_t quest_log_5_1;
    uint32_t quest_log_5_2;
    uint32_t quest_log_5_3;
    uint32_t quest_log_5_4;
    uint32_t quest_log_5_5;
    uint32_t quest_log_6_1;
    uint32_t quest_log_6_2;
    uint32_t quest_log_6_3;
    uint32_t quest_log_6_4;
    uint32_t quest_log_6_5;
    uint32_t quest_log_7_1;
    uint32_t quest_log_7_2;
    uint32_t quest_log_7_3;
    uint32_t quest_log_7_4;
    uint32_t quest_log_7_5;
    uint32_t quest_log_8_1;
    uint32_t quest_log_8_2;
    uint32_t quest_log_8_3;
    uint32_t quest_log_8_4;
    uint32_t quest_log_8_5;
    uint32_t quest_log_9_1;
    uint32_t quest_log_9_2;
    uint32_t quest_log_9_3;
    uint32_t quest_log_9_4;
    uint32_t quest_log_9_5;
    uint32_t quest_log_10_1;
    uint32_t quest_log_10_2;
    uint32_t quest_log_10_3;
    uint32_t quest_log_10_4;
    uint32_t quest_log_10_5;
    uint32_t quest_log_11_1;
    uint32_t quest_log_11_2;
    uint32_t quest_log_11_3;
    uint32_t quest_log_11_4;
    uint32_t quest_log_11_5;
    uint32_t quest_log_12_1;
    uint32_t quest_log_12_2;
    uint32_t quest_log_12_3;
    uint32_t quest_log_12_4;
    uint32_t quest_log_12_5;
    uint32_t quest_log_13_1;
    uint32_t quest_log_13_2;
    uint32_t quest_log_13_3;
    uint32_t quest_log_13_4;
    uint32_t quest_log_13_5;
    uint32_t quest_log_14_1;
    uint32_t quest_log_14_2;
    uint32_t quest_log_14_3;
    uint32_t quest_log_14_4;
    uint32_t quest_log_14_5;
    uint32_t quest_log_15_1;
    uint32_t quest_log_15_2;
    uint32_t quest_log_15_3;
    uint32_t quest_log_15_4;
    uint32_t quest_log_15_5;
    uint32_t quest_log_16_1;
    uint32_t quest_log_16_2;
    uint32_t quest_log_16_3;
    uint32_t quest_log_16_4;
    uint32_t quest_log_16_5;
    uint32_t quest_log_17_1;
    uint32_t quest_log_17_2;
    uint32_t quest_log_17_3;
    uint32_t quest_log_17_4;
    uint32_t quest_log_17_5;
    uint32_t quest_log_18_1;
    uint32_t quest_log_18_2;
    uint32_t quest_log_18_3;
    uint32_t quest_log_18_4;
    uint32_t quest_log_18_5;
    uint32_t quest_log_19_1;
    uint32_t quest_log_19_2;
    uint32_t quest_log_19_3;
    uint32_t quest_log_19_4;
    uint32_t quest_log_19_5;
    uint32_t quest_log_20_1;
    uint32_t quest_log_20_2;
    uint32_t quest_log_20_3;
    uint32_t quest_log_20_4;
    uint32_t quest_log_20_5;
    uint32_t quest_log_21_1;
    uint32_t quest_log_21_2;
    uint32_t quest_log_21_3;
    uint32_t quest_log_21_4;
    uint32_t quest_log_21_5;
    uint32_t quest_log_22_1;
    uint32_t quest_log_22_2;
    uint32_t quest_log_22_3;
    uint32_t quest_log_22_4;
    uint32_t quest_log_22_5;
    uint32_t quest_log_23_1;
    uint32_t quest_log_23_2;
    uint32_t quest_log_23_3;
    uint32_t quest_log_23_4;
    uint32_t quest_log_23_5;
    uint32_t quest_log_24_1;
    uint32_t quest_log_24_2;
    uint32_t quest_log_24_3;
    uint32_t quest_log_24_4;
    uint32_t quest_log_24_5;
    uint32_t quest_log_25_1;
    uint32_t quest_log_25_2;
    uint32_t quest_log_25_3;
    uint32_t quest_log_25_4;
    uint32_t quest_log_25_5;
    uint32_t quest_log_26_1;
    uint32_t quest_log_26_2;
    uint32_t quest_log_26_3;
    uint32_t quest_log_26_4;
    uint32_t quest_log_26_5;
    uint32_t quest_log_27_1;
    uint32_t quest_log_27_2;
    uint32_t quest_log_27_3;
    uint32_t quest_log_27_4;
    uint32_t quest_log_27_5;
    uint32_t quest_log_28_1;
    uint32_t quest_log_28_2;
    uint32_t quest_log_28_3;
    uint32_t quest_log_28_4;
    uint32_t quest_log_28_5;
    uint32_t quest_log_29_1;
    uint32_t quest_log_29_2;
    uint32_t quest_log_29_3;
    uint32_t quest_log_29_4;
    uint32_t quest_log_29_5;
    uint32_t quest_log_30_1;
    uint32_t quest_log_30_2;
    uint32_t quest_log_30_3;
    uint32_t quest_log_30_4;
    uint32_t quest_log_30_5;
    uint32_t quest_log_31_1;
    uint32_t quest_log_31_2;
    uint32_t quest_log_31_3;
    uint32_t quest_log_31_4;
    uint32_t quest_log_31_5;
    uint32_t quest_log_32_1;
    uint32_t quest_log_32_2;
    uint32_t quest_log_32_3;
    uint32_t quest_log_32_4;
    uint32_t quest_log_32_5;
    uint32_t quest_log_33_1;
    uint32_t quest_log_33_2;
    uint32_t quest_log_33_3;
    uint32_t quest_log_33_4;
    uint32_t quest_log_33_5;
    uint32_t quest_log_34_1;
    uint32_t quest_log_34_2;
    uint32_t quest_log_34_3;
    uint32_t quest_log_34_4;
    uint32_t quest_log_34_5;
    uint32_t quest_log_35_1;
    uint32_t quest_log_35_2;
    uint32_t quest_log_35_3;
    uint32_t quest_log_35_4;
    uint32_t quest_log_35_5;
    uint32_t quest_log_36_1;
    uint32_t quest_log_36_2;
    uint32_t quest_log_36_3;
    uint32_t quest_log_36_4;
    uint32_t quest_log_36_5;
    uint32_t quest_log_37_1;
    uint32_t quest_log_37_2;
    uint32_t quest_log_37_3;
    uint32_t quest_log_37_4;
    uint32_t quest_log_37_5;
    uint32_t quest_log_38_1;
    uint32_t quest_log_38_2;
    uint32_t quest_log_38_3;
    uint32_t quest_log_38_4;
    uint32_t quest_log_38_5;
    uint32_t quest_log_39_1;
    uint32_t quest_log_39_2;
    uint32_t quest_log_39_3;
    uint32_t quest_log_39_4;
    uint32_t quest_log_39_5;
    uint32_t quest_log_40_1;
    uint32_t quest_log_40_2;
    uint32_t quest_log_40_3;
    uint32_t quest_log_40_4;
    uint32_t quest_log_40_5;
    uint32_t quest_log_41_1;
    uint32_t quest_log_41_2;
    uint32_t quest_log_41_3;
    uint32_t quest_log_41_4;
    uint32_t quest_log_41_5;
    uint32_t quest_log_42_1;
    uint32_t quest_log_42_2;
    uint32_t quest_log_42_3;
    uint32_t quest_log_42_4;
    uint32_t quest_log_42_5;
    uint32_t quest_log_43_1;
    uint32_t quest_log_43_2;
    uint32_t quest_log_43_3;
    uint32_t quest_log_43_4;
    uint32_t quest_log_43_5;
    uint32_t quest_log_44_1;
    uint32_t quest_log_44_2;
    uint32_t quest_log_44_3;
    uint32_t quest_log_44_4;
    uint32_t quest_log_44_5;
    uint32_t quest_log_45_1;
    uint32_t quest_log_45_2;
    uint32_t quest_log_45_3;
    uint32_t quest_log_45_4;
    uint32_t quest_log_45_5;
    uint32_t quest_log_46_1;
    uint32_t quest_log_46_2;
    uint32_t quest_log_46_3;
    uint32_t quest_log_46_4;
    uint32_t quest_log_46_5;
    uint32_t quest_log_47_1;
    uint32_t quest_log_47_2;
    uint32_t quest_log_47_3;
    uint32_t quest_log_47_4;
    uint32_t quest_log_47_5;
    uint32_t quest_log_48_1;
    uint32_t quest_log_48_2;
    uint32_t quest_log_48_3;
    uint32_t quest_log_48_4;
    uint32_t quest_log_48_5;
    uint32_t quest_log_49_1;
    uint32_t quest_log_49_2;
    uint32_t quest_log_49_3;
    uint32_t quest_log_49_4;
    uint32_t quest_log_49_5;
    uint32_t quest_log_50_1;
    uint32_t quest_log_50_2;
    uint32_t quest_log_50_3;
    uint32_t quest_log_50_4;
    uint32_t quest_log_50_5;
    uint64_t visible_item_1_entry;
    uint32_t visible_item_1_enchantment;
    uint64_t visible_item_2_entry;
    uint32_t visible_item_2_enchantment;
    uint64_t visible_item_3_entry;
    uint32_t visible_item_3_enchantment;
    uint64_t visible_item_4_entry;
    uint32_t visible_item_4_enchantment;
    uint64_t visible_item_5_entry;
    uint32_t visible_item_5_enchantment;
    uint64_t visible_item_6_entry;
    uint32_t visible_item_6_enchantment;
    uint64_t visible_item_7_entry;
    uint32_t visible_item_7_enchantment;
    uint64_t visible_item_8_entry;
    uint32_t visible_item_8_enchantment;
    uint64_t visible_item_9_entry;
    uint32_t visible_item_9_enchantment;
    uint64_t visible_item_10_entry;
    uint32_t visible_item_10_enchantment;
    uint64_t visible_item_11_entry;
    uint32_t visible_item_11_enchantment;
    uint64_t visible_item_12_entry;
    uint32_t visible_item_12_enchantment;
    uint64_t visible_item_13_entry;
    uint32_t visible_item_13_enchantment;
    uint64_t visible_item_14_entry;
    uint32_t visible_item_14_enchantment;
    uint64_t visible_item_15_entry;
    uint32_t visible_item_15_enchantment;
    uint64_t visible_item_16_entry;
    uint32_t visible_item_16_enchantment;
    uint64_t visible_item_17_entry;
    uint32_t visible_item_17_enchantment;
    uint64_t visible_item_18_entry;
    uint32_t visible_item_18_enchantment;
    uint64_t visible_item_19_entry;
    uint32_t visible_item_19_enchantment;
    uint32_t chosen_title;
    uint32_t inebriation;
    uint32_t player_padding_0;
    // Current player fields say long - client memory dump says int.
    uint64_t inventory_slot[32];
    uint64_t pack_slot[16];
    uint64_t bank_slot[28];
    uint64_t bank_bag_slot[7];
    uint64_t vendor_buy_back_slot[12];
    uint64_t farsight_guid;
    uint64_t known_titles;
    uint64_t known_titles1;
    uint64_t known_titles2;
    uint64_t known_currency;
    uint32_t xp;
    uint32_t next_level_xp;
    uint32_t skill_line[64];
    uint32_t skill_step[64];
    uint32_t skill_rank[64];
    uint32_t skill_mod[64];
    uint32_t skill_talent[64];
    uint32_t character_points_1;
    uint32_t track_creatures;
    uint32_t track_resources;
    uint32_t expertise;
    uint32_t offhand_expertise;
    float_t block_pct;
    float_t dodge_pct;
    float_t parry_pct;
    float_t crit_pct;
    float_t ranged_crit_pct;
    float_t offhand_crit_pct;
    float_t spell_crit_pct[WOWPLAYER_SPELL_CRIT_PCT_COUNT];
    uint32_t shield_block;
    uint32_t shield_block_crit_pct;
    uint32_t mastery;
    uint32_t explored_zones[WOWPLAYER_EXPLORED_ZONES_COUNT];
    uint32_t rest_state_xp;
    uint32_t field_coinage;
    uint32_t field_mod_damage_done_positive[WOWPLAYER_MOD_DAMAGE_DONE_COUNT];
    uint32_t field_mod_damage_done_negative[WOWPLAYER_MOD_DAMAGE_DONE_COUNT];
    // Listed as an int... but this seems like it would be a float? TODO: Verify
    uint32_t field_mod_damage_done_pct[WOWPLAYER_MOD_DAMAGE_DONE_COUNT];
    uint32_t field_mod_healing_done;
    uint32_t field_mod_healing_pct;
    uint32_t field_mod_healing_done_pct;
    uint32_t weapon_dmg_multiplier[2];
    uint32_t mod_spell_power_pct;
    uint32_t override_spell_power_by_ap_pct;
    uint32_t field_mod_target_resistance;
    uint32_t field_mod_target_physical_resistance;
    uint32_t field_bytes;
    uint32_t self_resurrection_spell;
    uint32_t field_pvp_medals;
    uint32_t field_buy_back_price[WOWPLAYER_BUY_BACK_COUNT];
    uint32_t field_buy_back_timestamp[WOWPLAYER_BUY_BACK_COUNT];
    uint32_t field_kills;
    uint32_t field_lifetime_honorable_kills;
    uint32_t player_field_bytes_2;
    uint32_t field_watched_faction_idx;
    uint32_t field_combat_rating[26];
    uint32_t field_arena_team_info[21];
    uint32_t battleground_rating;
    uint32_t field_max_level;
    uint32_t field_daily_quests[WOWPLAYER_DAILY_QUESTS_COUNT];
    uint32_t rune_regen[4];
    uint32_t no_reagent_cost[3];
    uint32_t field_glyph_slots[9];
    uint32_t field_glyphs[9];
    uint32_t glyphs_enabled;
    uint32_t pet_spell_power;
    uint32_t researching[8];
    uint32_t research_site[8];
    uint32_t profession_skill_line[2];
    uint32_t ui_hit_mod;
    uint32_t ui_hit_spell_mod;
    uint32_t ui_home_realm_time_offset;
    uint32_t mod_haste;
    uint32_t mod_ranged_haste;
    uint32_t mod_pet_haste;
    uint32_t mod_haste_regen;
};
#endif
#pragma pack(pop)
