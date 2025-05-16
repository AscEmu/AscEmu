/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "WoWUnit.hpp"

#pragma pack(push, 1)

union player_bytes_union
{
    struct parts
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
};

union player_bytes_2_union
{
    struct parts
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
};

// Adjusted values.
#if VERSION_STRING == Classic
static inline constexpr uint8_t WOWPLAYER_QUEST_COUNT = 20;
static inline constexpr uint8_t WOWPLAYER_VISIBLE_ITEM_COUNT = 19;
static inline constexpr uint8_t WOWPLAYER_VISIBLE_ITEM_UNK0_COUNT = 7;
static inline constexpr uint8_t WOWPLAYER_INVENTORY_SLOT_COUNT = 23;
static inline constexpr uint8_t WOWPLAYER_PACK_SLOT_COUNT = 16;
static inline constexpr uint8_t WOWPLAYER_BANK_SLOT_COUNT = 24;
static inline constexpr uint8_t WOWPLAYER_BANK_BAG_SLOT_COUNT = 6;
static inline constexpr uint8_t WOWPLAYER_BUY_BACK_COUNT = 12;
static inline constexpr uint8_t WOWPLAYER_KEYRING_SLOT_COUNT = 20;
static inline constexpr uint8_t WOWPLAYER_SKILL_INFO_COUNT = 128;
static inline constexpr uint8_t WOWPLAYER_EXPLORED_ZONES_COUNT = 64;
static inline constexpr uint8_t WOWPLAYER_STAT_COUNT = 5;
static inline constexpr uint8_t WOWPLAYER_SPELL_SCHOOL_COUNT = 7;
static inline constexpr uint8_t WOWPLAYER_COMBAT_RATING_COUNT = 20;

union player_bytes_3_union
{
    struct parts
    {
        uint8_t gender;
        uint8_t drunk_value;
        uint8_t pvp_city_protector_rank; // not used
        uint8_t pvp_rank;
    } s;
    uint32_t raw;
};

union player_field_bytes_union
{
    struct parts
    {
        uint8_t misc_flags;
        uint8_t combo_points; // not used
        uint8_t enabled_action_bars;
        uint8_t max_pvp_rank; // not used
    } s;
    uint32_t raw;
};

union player_field_bytes_2_union
{
    struct parts
    {
        uint8_t unk0; // related to pvp
        uint8_t aura_vision;
        uint8_t unk2;
        uint8_t unk3;
    } s;
    uint32_t raw;
};

struct WoWPlayer_Quest
{
    uint32_t quest_id;
    uint32_t required_count_state;
    uint32_t expire_time;
};

struct WoWPlayer_VisibleItem
{
    uint64_t creator;
    uint32_t entry;
    std::array<uint32_t, WOWPLAYER_VISIBLE_ITEM_UNK0_COUNT> enchantment;
    uint32_t properties;
    uint32_t padding;
};

struct WoWPlayer_SkillInfo
{
    uint16_t id;
    uint16_t step;
    uint16_t current_value;
    uint16_t max_value;
    uint16_t bonus_temporary;
    uint16_t bonus_permanent;
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
    std::array<WoWPlayer_Quest, WOWPLAYER_QUEST_COUNT> quests;
    std::array<WoWPlayer_VisibleItem, WOWPLAYER_VISIBLE_ITEM_COUNT> visible_items;
    // Current player fields say long - client memory dump says int.
    std::array<uint64_t, WOWPLAYER_INVENTORY_SLOT_COUNT> inventory_slot;
    std::array<uint64_t, WOWPLAYER_PACK_SLOT_COUNT> pack_slot;
    std::array<uint64_t, WOWPLAYER_BANK_SLOT_COUNT> bank_slot;
    std::array<uint64_t, WOWPLAYER_BANK_BAG_SLOT_COUNT> bank_bag_slot;
    std::array<uint64_t, WOWPLAYER_BUY_BACK_COUNT> vendor_buy_back_slot;
    std::array<uint64_t, WOWPLAYER_KEYRING_SLOT_COUNT> key_ring_slot;
    uint64_t farsight_guid;
    uint64_t field_combo_target;
    uint32_t xp;
    uint32_t next_level_xp;
    std::array<WoWPlayer_SkillInfo, WOWPLAYER_SKILL_INFO_COUNT> skill_info;
    uint32_t character_points_1;
    uint32_t character_points_2;
    uint32_t track_creatures;
    uint32_t track_resources;
    float block_pct;
    float dodge_pct;
    float parry_pct;
    float crit_pct;
    float ranged_crit_pct;
    std::array<uint32_t, WOWPLAYER_EXPLORED_ZONES_COUNT> explored_zones;
    uint32_t rest_state_xp;
    uint32_t field_coinage;
    std::array<uint32_t, WOWPLAYER_STAT_COUNT> pos_stat;
    std::array<uint32_t, WOWPLAYER_STAT_COUNT> neg_stat;
    std::array<uint32_t, WOWPLAYER_SPELL_SCHOOL_COUNT> resistance_buff_mod_positive;
    std::array<uint32_t, WOWPLAYER_SPELL_SCHOOL_COUNT> resistance_buff_mod_negative;
    std::array<uint32_t, WOWPLAYER_SPELL_SCHOOL_COUNT> field_mod_damage_done_positive;
    std::array<uint32_t, WOWPLAYER_SPELL_SCHOOL_COUNT> field_mod_damage_done_negative;
    std::array<float, WOWPLAYER_SPELL_SCHOOL_COUNT> field_mod_damage_done_pct;
    player_field_bytes_union player_field_bytes;
    uint32_t ammo_id;
    uint32_t self_resurrection_spell;
    uint32_t field_pvp_medals;
    std::array<uint32_t, WOWPLAYER_BUY_BACK_COUNT> field_buy_back_price;
    std::array<uint32_t, WOWPLAYER_BUY_BACK_COUNT> field_buy_back_timestamp;
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
    std::array<uint32_t, WOWPLAYER_COMBAT_RATING_COUNT> field_combat_rating;
};
#elif VERSION_STRING == TBC
static inline constexpr uint8_t WOWPLAYER_QUEST_COUNT = 25;
static inline constexpr uint8_t WOWPLAYER_VISIBLE_ITEM_COUNT = 19;
static inline constexpr uint8_t WOWPLAYER_VISIBLE_ITEM_UNK0_COUNT = 11;
static inline constexpr uint8_t WOWPLAYER_INVENTORY_SLOT_COUNT = 23;
static inline constexpr uint8_t WOWPLAYER_PACK_SLOT_COUNT = 16;
static inline constexpr uint8_t WOWPLAYER_BANK_SLOT_COUNT = 28;
static inline constexpr uint8_t WOWPLAYER_BANK_BAG_SLOT_COUNT = 7;
static inline constexpr uint8_t WOWPLAYER_KEYRING_SLOT_COUNT = 32;
static inline constexpr uint8_t WOWPLAYER_VANITY_PET_SLOT_COUNT = 18;
static inline constexpr uint8_t WOWPLAYER_SKILL_INFO_COUNT = 128;
static inline constexpr uint8_t WOWPLAYER_SPELL_SCHOOL_COUNT = 7;
static inline constexpr uint8_t WOWPLAYER_EXPLORED_ZONES_COUNT = 128;
static inline constexpr uint8_t WOWPLAYER_BUY_BACK_COUNT = 12;
static inline constexpr uint8_t WOWPLAYER_COMBAT_RATING_COUNT = 24;
static inline constexpr uint8_t WOWPLAYER_ARENA_TEAM_SLOTS = 3;
static inline constexpr uint8_t WOWPLAYER_DAILY_QUESTS_COUNT = 25;
static inline constexpr uint8_t WOWPLAYER_KNOWN_TITLES_SIZE = 1;

union player_bytes_3_union
{
    struct parts
    {
        uint8_t gender;
        uint8_t drunk_value;
        uint8_t pvp_rank;
        uint8_t arena_faction;
    } s;
    uint32_t raw;
};

union player_field_bytes_union
{
    struct parts
    {
        uint8_t misc_flags;
        uint8_t raf_level; // not used
        uint8_t enabled_action_bars;
        uint8_t max_pvp_rank; // not used
    } s;
    uint32_t raw;
};

union player_field_bytes_2_union
{
    struct parts
    {
        uint8_t unk0; // same as classic?
        uint8_t aura_vision;
        uint8_t unk2;
        uint8_t unk3;
    } s;
    uint32_t raw;
};

struct WoWPlayer_Quest
{
    uint32_t quest_id;
    uint32_t state;
    uint32_t required_mob_or_go;
    uint32_t expire_time;
};

struct WoWPlayer_VisibleItem
{
    uint64_t creator;
    uint32_t entry;
    std::array<uint32_t, WOWPLAYER_VISIBLE_ITEM_UNK0_COUNT> enchantment;
    uint32_t properties;
    uint32_t padding;
};

struct WoWPlayer_ArenaTeamInfo
{
    uint32_t team_id;
    uint32_t member_rank;
    uint32_t games_week;
    uint32_t games_season;
    uint32_t wins_season;
    uint32_t personal_rating;
};

struct WoWPlayer_SkillInfo
{
    uint16_t id;
    uint16_t step;
    uint16_t current_value;
    uint16_t max_value;
    uint16_t bonus_temporary;
    uint16_t bonus_permanent;
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
    std::array<WoWPlayer_Quest, WOWPLAYER_QUEST_COUNT> quests;
    std::array<WoWPlayer_VisibleItem, WOWPLAYER_VISIBLE_ITEM_COUNT> visible_items;
    uint32_t chosen_title;
    uint32_t player_padding_0;
    // Current player fields say long - client memory dump says int.
    std::array<uint64_t, WOWPLAYER_INVENTORY_SLOT_COUNT> inventory_slot;
    std::array<uint64_t, WOWPLAYER_PACK_SLOT_COUNT> pack_slot;
    std::array<uint64_t, WOWPLAYER_BANK_SLOT_COUNT> bank_slot;
    std::array<uint64_t, WOWPLAYER_BANK_BAG_SLOT_COUNT> bank_bag_slot;
    std::array<uint64_t, WOWPLAYER_BUY_BACK_COUNT> vendor_buy_back_slot;
    std::array<uint64_t, WOWPLAYER_KEYRING_SLOT_COUNT> key_ring_slot;
    std::array<uint64_t, WOWPLAYER_VANITY_PET_SLOT_COUNT> vanity_pet_slot;
    uint64_t farsight_guid;
    std::array<uint64_t, WOWPLAYER_KNOWN_TITLES_SIZE> field_known_titles;
    uint32_t xp;
    uint32_t next_level_xp;
    std::array<WoWPlayer_SkillInfo, WOWPLAYER_SKILL_INFO_COUNT> skill_info;
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
    std::array<float, WOWPLAYER_SPELL_SCHOOL_COUNT> spell_crit_pct;
    uint32_t shield_block;
    std::array<uint32_t, WOWPLAYER_EXPLORED_ZONES_COUNT> explored_zones;
    uint32_t rest_state_xp;
    uint32_t field_coinage;
    std::array<uint32_t, WOWPLAYER_SPELL_SCHOOL_COUNT> field_mod_damage_done_positive;
    std::array<uint32_t, WOWPLAYER_SPELL_SCHOOL_COUNT> field_mod_damage_done_negative;
    std::array<float, WOWPLAYER_SPELL_SCHOOL_COUNT> field_mod_damage_done_pct;
    uint32_t field_mod_healing_done;
    uint32_t field_mod_target_resistance;
    uint32_t field_mod_target_physical_resistance;
    player_field_bytes_union player_field_bytes;
    uint32_t ammo_id;
    uint32_t self_resurrection_spell;
    uint32_t field_pvp_medals;
    std::array<uint32_t, WOWPLAYER_BUY_BACK_COUNT> field_buy_back_price;
    std::array<uint32_t, WOWPLAYER_BUY_BACK_COUNT> field_buy_back_timestamp;
    union field_kills_union
    {
        struct parts
        {
            uint16_t kills_today;
            uint16_t kills_yesterday;
        } kills_field_parts;
        uint32_t raw;
    } field_kills;
    uint32_t field_contribution_today;
    uint32_t field_contribution_yesterday;
    uint32_t field_lifetime_honorable_kills;
    player_field_bytes_2_union player_field_bytes_2;
    uint32_t field_watched_faction_idx;
    std::array<uint32_t, WOWPLAYER_COMBAT_RATING_COUNT> field_combat_rating;
    std::array<WoWPlayer_ArenaTeamInfo, WOWPLAYER_ARENA_TEAM_SLOTS> field_arena_team_info;
    uint32_t field_honor_currency;
    uint32_t field_arena_currency;
    float field_mod_mana_regen;
    float field_mod_mana_regen_interrupt;
    uint32_t field_max_level;
    std::array<uint32_t, WOWPLAYER_DAILY_QUESTS_COUNT> field_daily_quests;
};
#elif VERSION_STRING == WotLK
static inline constexpr uint8_t WOWPLAYER_QUEST_COUNT = 25;
static inline constexpr uint8_t WOWPLAYER_VISIBLE_ITEM_COUNT = 19;
static inline constexpr uint8_t WOWPLAYER_EXPLORED_ZONES_COUNT = 128;
static inline constexpr uint8_t WOWPLAYER_INVENTORY_SLOT_COUNT = 23;
static inline constexpr uint8_t WOWPLAYER_PACK_SLOT_COUNT = 16;
static inline constexpr uint8_t WOWPLAYER_BANK_SLOT_COUNT = 28;
static inline constexpr uint8_t WOWPLAYER_BANK_BAG_SLOT_COUNT = 7;
static inline constexpr uint8_t WOWPLAYER_KEYRING_SLOT_COUNT = 32;
static inline constexpr uint8_t WOWPLAYER_CURRENCY_TOKEN_SLOT_COUNT = 32;
static inline constexpr uint8_t WOWPLAYER_SKILL_INFO_COUNT = 128;
static inline constexpr uint8_t WOWPLAYER_SPELL_SCHOOL_COUNT = 7;
static inline constexpr uint8_t WOWPLAYER_BUY_BACK_COUNT = 12;
static inline constexpr uint8_t WOWPLAYER_COMBAT_RATING_COUNT = 25;
static inline constexpr uint8_t WOWPLAYER_ARENA_TEAM_SLOTS = 3;
static inline constexpr uint8_t WOWPLAYER_DAILY_QUESTS_COUNT = 25;
static inline constexpr uint8_t WOWPLAYER_RUNE_REGEN_COUNT = 4;
static inline constexpr uint8_t WOWPLAYER_NO_REAGENT_COST_COUNT = 3;
static inline constexpr uint8_t WOWPLAYER_GLYPH_SLOT_COUNT = 6;
static inline constexpr uint8_t WOWPLAYER_KNOWN_TITLES_SIZE = 3;

union player_bytes_3_union
{
    struct parts
    {
        uint8_t gender;
        uint8_t drunk_value;
        uint8_t pvp_rank;
        uint8_t arena_faction;
    } s;
    uint32_t raw;
};

union player_field_bytes_union
{
    struct parts
    {
        uint8_t misc_flags;
        uint8_t raf_level; // not used
        uint8_t enabled_action_bars;
        uint8_t max_pvp_rank; // not used
    } s;
    uint32_t raw;
};

union player_field_bytes_2_union
{
    struct parts
    {
        uint16_t override_spell_id; // not used
        uint8_t ignore_power_regen_prediction_mask; // not used
        uint8_t aura_vision;
    } s;
    uint32_t raw;
};

struct WoWPlayer_Quest
{
    uint32_t quest_id;
    uint32_t state;
    uint64_t required_mob_or_go;
    uint32_t expire_time;
};

struct WoWPlayer_VisibleItem
{
    uint32_t entry;
    union enchantment_union
    {
        struct parts
        {
            uint16_t perm_enchantment;
            uint16_t temp_enchantment;
        } enchantment_field_parts;
        std::array<uint16_t, 2> raw;
    } enchantment;
};

struct WoWPlayer_ArenaTeamInfo
{
    uint32_t team_id;
    uint32_t type;
    uint32_t member_rank;
    uint32_t games_week;
    uint32_t games_season;
    uint32_t wins_season;
    uint32_t personal_rating;
};

struct WoWPlayer_SkillInfo
{
    uint16_t id;
    uint16_t step;
    uint16_t current_value;
    uint16_t max_value;
    uint16_t bonus_temporary;
    uint16_t bonus_permanent;
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
    std::array<WoWPlayer_Quest, WOWPLAYER_QUEST_COUNT> quests;
    std::array<WoWPlayer_VisibleItem, WOWPLAYER_VISIBLE_ITEM_COUNT> visible_items;
    uint32_t chosen_title;
    uint32_t inebriation;
    uint32_t player_padding_0;
    std::array<uint64_t, WOWPLAYER_INVENTORY_SLOT_COUNT> inventory_slot;
    std::array<uint64_t, WOWPLAYER_PACK_SLOT_COUNT> pack_slot;
    std::array<uint64_t, WOWPLAYER_BANK_SLOT_COUNT> bank_slot;
    std::array<uint64_t, WOWPLAYER_BANK_BAG_SLOT_COUNT> bank_bag_slot;
    std::array<uint64_t, WOWPLAYER_BUY_BACK_COUNT> vendor_buy_back_slot;
    std::array<uint64_t, WOWPLAYER_KEYRING_SLOT_COUNT> key_ring_slot;
    std::array<uint64_t, WOWPLAYER_CURRENCY_TOKEN_SLOT_COUNT> currencytoken_slot;
    uint64_t farsight_guid;
    std::array<uint64_t, WOWPLAYER_KNOWN_TITLES_SIZE> field_known_titles;
    uint64_t field_known_currencies;
    uint32_t xp;
    uint32_t next_level_xp;
    std::array<WoWPlayer_SkillInfo, WOWPLAYER_SKILL_INFO_COUNT> skill_info;
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
    std::array<float, WOWPLAYER_SPELL_SCHOOL_COUNT> spell_crit_pct;
    uint32_t shield_block;
    float shield_block_crit_pct;
    std::array<uint32_t, WOWPLAYER_EXPLORED_ZONES_COUNT> explored_zones;
    uint32_t rest_state_xp;
    uint32_t field_coinage;
    std::array<uint32_t, WOWPLAYER_SPELL_SCHOOL_COUNT> field_mod_damage_done_positive;
    std::array<uint32_t, WOWPLAYER_SPELL_SCHOOL_COUNT> field_mod_damage_done_negative;
    std::array<float, WOWPLAYER_SPELL_SCHOOL_COUNT> field_mod_damage_done_pct;
    uint32_t field_mod_healing_done;
    uint32_t field_mod_healing_pct;
    uint32_t field_mod_healing_done_pct;
    uint32_t field_mod_target_resistance;
    uint32_t field_mod_target_physical_resistance;
    player_field_bytes_union player_field_bytes;
    uint32_t ammo_id;
    uint32_t self_resurrection_spell;
    uint32_t field_pvp_medals;
    std::array<uint32_t, WOWPLAYER_BUY_BACK_COUNT> field_buy_back_price;
    std::array<uint32_t, WOWPLAYER_BUY_BACK_COUNT> field_buy_back_timestamp;
    union field_kills_union
    {
        struct parts
        {
            uint16_t kills_today;
            uint16_t kills_yesterday;
        } kills_field_parts;
        uint32_t raw;
    } field_kills;
    uint32_t field_contribution_today;
    uint32_t field_contribution_yesterday;
    uint32_t field_lifetime_honorable_kills;
    player_field_bytes_2_union player_field_bytes_2;
    uint32_t field_watched_faction_idx;
    std::array<uint32_t, WOWPLAYER_COMBAT_RATING_COUNT> field_combat_rating;
    std::array<WoWPlayer_ArenaTeamInfo, WOWPLAYER_ARENA_TEAM_SLOTS> field_arena_team_info;
    uint32_t field_honor_currency;
    uint32_t field_arena_currency;
    uint32_t field_max_level;
    std::array<uint32_t, WOWPLAYER_DAILY_QUESTS_COUNT> field_daily_quests;
    std::array<float, WOWPLAYER_RUNE_REGEN_COUNT> rune_regen;
    std::array<uint32_t, WOWPLAYER_NO_REAGENT_COST_COUNT> no_reagent_cost;
    std::array<uint32_t, WOWPLAYER_GLYPH_SLOT_COUNT> field_glyph_slots;
    std::array<uint32_t, WOWPLAYER_GLYPH_SLOT_COUNT> field_glyphs;
    uint32_t glyphs_enabled;
    uint32_t pet_spell_power;
};
#elif VERSION_STRING == Cata
static inline constexpr uint8_t WOWPLAYER_EXPLORED_ZONES_COUNT = 156;
static inline constexpr uint8_t WOWPLAYER_WEAPON_DMG_MULTIPLIER_COUNT = 3;
static inline constexpr uint8_t WOWPLAYER_SPELL_SCHOOL_COUNT = 7;
static inline constexpr uint8_t WOWPLAYER_BUY_BACK_COUNT = 12;
static inline constexpr uint8_t WOWPLAYER_ARENA_TEAM_SLOTS = 3;
static inline constexpr uint8_t WOWPLAYER_DAILY_QUESTS_COUNT = 25;
static inline constexpr uint8_t WOWPLAYER_QUEST_COUNT = 50;
static inline constexpr uint8_t WOWPLAYER_VISIBLE_ITEM_COUNT = 19;
static inline constexpr uint8_t WOWPLAYER_INVENTORY_SLOT_COUNT = 23;
static inline constexpr uint8_t WOWPLAYER_PACK_SLOT_COUNT = 16;
static inline constexpr uint8_t WOWPLAYER_BANK_SLOT_COUNT = 28;
static inline constexpr uint8_t WOWPLAYER_BANK_BAG_SLOT_COUNT = 7;
static inline constexpr uint8_t WOWPLAYER_KEYRING_SLOT_COUNT = 32;
static inline constexpr uint8_t WOWPLAYER_CURRENCY_TOKEN_SLOT_COUNT = 32;
static inline constexpr uint8_t WOWPLAYER_COMBAT_RATING_COUNT = 26;
static inline constexpr uint8_t WOWPLAYER_RUNE_REGEN_COUNT = 4;
static inline constexpr uint8_t WOWPLAYER_NO_REAGENT_COST_COUNT = 3;
static inline constexpr uint8_t WOWPLAYER_GLYPH_SLOT_COUNT = 9;
static inline constexpr uint8_t WOWPLAYER_KNOWN_TITLES_SIZE = 4;
static inline constexpr uint8_t WOWPLAYER_SKILL_INFO_COUNT = 128;
static inline constexpr uint8_t WOWPLAYER_RESEARCHING_COUNT = 8;
static inline constexpr uint8_t WOWPLAYER_PROFESSION_SKILL_COUNT = 2;

union player_bytes_3_union
{
    struct parts
    {
        uint8_t gender;
        uint8_t drunk_value;
        uint8_t pvp_rank;
        uint8_t arena_faction;
    } s;
    uint32_t raw;
};

union player_field_bytes_union
{
    struct parts
    {
        uint8_t misc_flags;
        uint8_t raf_level; // not used
        uint8_t enabled_action_bars;
        uint8_t max_pvp_rank; // not used
    } s;
    uint32_t raw;
};

union player_field_bytes_2_union
{
    struct parts
    {
        uint16_t override_spell_id; // not used
        uint8_t ignore_power_regen_prediction_mask; // not used
        uint8_t aura_vision;
    } s;
    uint32_t raw;
};

struct WoWPlayer_Quest
{
    uint32_t quest_id;
    uint32_t state;
    uint64_t required_mob_or_go;
    uint32_t expire_time;
};

struct WoWPlayer_VisibleItem
{
    uint32_t entry;
    union enchantment_union
    {
        struct parts
        {
            uint16_t perm_enchantment;
            uint16_t temp_enchantment;
        } enchantment_field_parts;
        std::array<uint16_t, 2> raw;
    } enchantment;
};

struct WoWPlayer_ArenaTeamInfo
{
    uint32_t team_id;
    uint32_t type;
    uint32_t member_rank;
    uint32_t games_week;
    uint32_t games_season;
    uint32_t wins_season;
    uint32_t personal_rating;
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
    std::array<WoWPlayer_Quest, WOWPLAYER_QUEST_COUNT> quests;
    std::array<WoWPlayer_VisibleItem, WOWPLAYER_VISIBLE_ITEM_COUNT> visible_items;
    uint32_t chosen_title;
    uint32_t inebriation;
    uint32_t player_padding_0;
    std::array<uint64_t, WOWPLAYER_INVENTORY_SLOT_COUNT> inventory_slot;
    std::array<uint64_t, WOWPLAYER_PACK_SLOT_COUNT> pack_slot;
    std::array<uint64_t, WOWPLAYER_BANK_SLOT_COUNT> bank_slot;
    std::array<uint64_t, WOWPLAYER_BANK_BAG_SLOT_COUNT> bank_bag_slot;
    std::array<uint64_t, WOWPLAYER_BUY_BACK_COUNT> vendor_buy_back_slot;
    uint64_t farsight_guid;
    std::array<uint64_t, WOWPLAYER_KNOWN_TITLES_SIZE> field_known_titles;
    uint32_t xp;
    uint32_t next_level_xp;

    union skill_info_union
    {
        struct parts
        {
            std::array<uint32_t, 64> skill_line;
            std::array<uint32_t, 64> skill_step;
            std::array<uint32_t, 64> skill_rank;
            std::array<uint32_t, 64> skill_max_rank;
            std::array<uint32_t, 64> skill_mod;
            std::array<uint32_t, 64> skill_talent;
        } skill_info_parts;
    } field_skill_info;

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
    std::array<float, WOWPLAYER_SPELL_SCHOOL_COUNT> spell_crit_pct;
    uint32_t shield_block;
    float shield_block_crit_pct;
    uint32_t mastery;
    std::array<uint32_t, WOWPLAYER_EXPLORED_ZONES_COUNT> explored_zones;
    uint32_t rest_state_xp;
    uint64_t field_coinage;
    std::array<uint32_t, WOWPLAYER_SPELL_SCHOOL_COUNT> field_mod_damage_done_positive;
    std::array<uint32_t, WOWPLAYER_SPELL_SCHOOL_COUNT> field_mod_damage_done_negative;
    std::array<float, WOWPLAYER_SPELL_SCHOOL_COUNT> field_mod_damage_done_pct;
    uint32_t field_mod_healing_done;
    float field_mod_healing_pct;
    float field_mod_healing_done_pct;
    std::array<float, WOWPLAYER_WEAPON_DMG_MULTIPLIER_COUNT> weapon_dmg_multiplier;
    float mod_spell_power_pct;
    float override_spell_power_by_ap_pct;
    uint32_t field_mod_target_resistance;
    uint32_t field_mod_target_physical_resistance;
    player_field_bytes_union player_field_bytes;
    uint32_t self_resurrection_spell;
    uint32_t field_pvp_medals;
    std::array<uint32_t, WOWPLAYER_BUY_BACK_COUNT> field_buy_back_price;
    std::array<uint32_t, WOWPLAYER_BUY_BACK_COUNT> field_buy_back_timestamp;
    union field_kills_union
    {
        struct parts
        {
            uint16_t kills_today;
            uint16_t kills_yesterday;
        } kills_field_parts;
        uint32_t raw;
    } field_kills;
    uint32_t field_lifetime_honorable_kills;
    player_field_bytes_2_union player_field_bytes_2;
    uint32_t field_watched_faction_idx;
    std::array<uint32_t, WOWPLAYER_COMBAT_RATING_COUNT> field_combat_rating;
    std::array<WoWPlayer_ArenaTeamInfo, WOWPLAYER_ARENA_TEAM_SLOTS> field_arena_team_info;
    uint32_t battleground_rating;
    uint32_t field_max_level;
    std::array<uint32_t, WOWPLAYER_DAILY_QUESTS_COUNT> field_daily_quests;
    std::array<float, WOWPLAYER_RUNE_REGEN_COUNT> rune_regen;
    std::array<uint32_t, WOWPLAYER_NO_REAGENT_COST_COUNT> no_reagent_cost;
    std::array<uint32_t, WOWPLAYER_GLYPH_SLOT_COUNT> field_glyph_slots;
    std::array<uint32_t, WOWPLAYER_GLYPH_SLOT_COUNT> field_glyphs;
    uint32_t glyphs_enabled;
    uint32_t pet_spell_power;
    std::array<uint32_t, WOWPLAYER_RESEARCHING_COUNT> researching;
    std::array<uint32_t, WOWPLAYER_RESEARCHING_COUNT> research_site;
    std::array<uint32_t, WOWPLAYER_PROFESSION_SKILL_COUNT> profession_skill_line;
    float ui_hit_mod;
    float ui_hit_spell_mod;
    uint32_t ui_home_realm_time_offset;
    float mod_haste;
    float mod_ranged_haste;
    float mod_pet_haste;
    float mod_haste_regen;
};
#elif VERSION_STRING == Mop
static inline constexpr uint8_t WOWPLAYER_EXPLORED_ZONES_COUNT = 200;
static inline constexpr uint8_t WOWPLAYER_WEAPON_DMG_MULTIPLIER_COUNT = 3;
static inline constexpr uint8_t WOWPLAYER_SPELL_SCHOOL_COUNT = 7;
static inline constexpr uint8_t WOWPLAYER_BUY_BACK_COUNT = 12;
static inline constexpr uint8_t WOWPLAYER_ARENA_TEAM_SLOTS = 3;
static inline constexpr uint8_t WOWPLAYER_DAILY_QUESTS_COUNT = 25;
static inline constexpr uint8_t WOWPLAYER_QUEST_COUNT = 150;
static inline constexpr uint8_t WOWPLAYER_VISIBLE_ITEM_COUNT = 19;
static inline constexpr uint8_t WOWPLAYER_INVENTORY_SLOT_COUNT = 23;
static inline constexpr uint8_t WOWPLAYER_PACK_SLOT_COUNT = 16;
static inline constexpr uint8_t WOWPLAYER_BANK_SLOT_COUNT = 28;
static inline constexpr uint8_t WOWPLAYER_BANK_BAG_SLOT_COUNT = 7;
static inline constexpr uint8_t WOWPLAYER_KEYRING_SLOT_COUNT = 32;
static inline constexpr uint8_t WOWPLAYER_CURRENCY_TOKEN_SLOT_COUNT = 32;
static inline constexpr uint8_t WOWPLAYER_KNOWN_TITLES_SIZE = 5;
static inline constexpr uint16_t WOWPLAYER_SKILL_INFO_COUNT = 448;
static inline constexpr uint8_t WOWPLAYER_COMBAT_RATING_COUNT = 27;
static inline constexpr uint8_t WOWPLAYER_RUNE_REGEN_COUNT = 4;
static inline constexpr uint8_t WOWPLAYER_NO_REAGENT_COST_COUNT = 4;
static inline constexpr uint8_t WOWPLAYER_GLYPH_SLOT_COUNT = 6;
static inline constexpr uint8_t WOWPLAYER_RESEARCHING_COUNT = 8;
static inline constexpr uint8_t WOWPLAYER_PROFESSION_SKILL_COUNT = 2;

union player_bytes_3_union
{
    struct parts
    {
        uint8_t gender;
        uint8_t drunk_value;
        uint8_t pvp_rank;
        uint8_t arena_faction;
    } s;
    uint32_t raw;
};

union player_field_bytes_union
{
    struct parts
    {
        uint8_t misc_flags;
        uint8_t raf_level; // not used
        uint8_t enabled_action_bars;
        uint8_t max_pvp_rank; // not used
    } s;
    uint32_t raw;
};

struct WoWPlayer_Quest
{
    uint32_t quest_id;
    uint32_t state;
    uint64_t required_mob_or_go;
    uint32_t expire_time;
};

struct WoWPlayer_VisibleItem
{
    uint32_t entry;
    union enchantment_union
    {
        struct parts
        {
            uint16_t perm_enchantment;
            uint16_t temp_enchantment;
        } enchantment_field_parts;
        std::array<uint16_t, 2> raw;
    } enchantment;
};

//\todo: guessed structure
struct WoWPlayer_ArenaTeamInfo
{
    uint32_t team_id;
    uint32_t type;
    uint32_t member_rank;
    uint32_t games_week;
    uint32_t games_season;
    uint32_t wins_season;
    uint32_t personal_rating;
    uint32_t unk;
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
    std::array<WoWPlayer_Quest, WOWPLAYER_QUEST_COUNT> quests;
    std::array<WoWPlayer_VisibleItem, WOWPLAYER_VISIBLE_ITEM_COUNT> visible_items;
    uint32_t chosen_title;
    uint32_t inebriation;
    uint32_t virtual_player_realm;
    uint32_t current_spec_id;
    uint32_t taxi_mount_anim_kit_id;
    uint32_t current_battle_pet_breed_quality;
    std::array<uint64_t, WOWPLAYER_INVENTORY_SLOT_COUNT> inventory_slot;
    std::array<uint64_t, WOWPLAYER_PACK_SLOT_COUNT> pack_slot;
    std::array<uint64_t, WOWPLAYER_BANK_SLOT_COUNT> bank_slot;
    std::array<uint64_t, WOWPLAYER_BANK_BAG_SLOT_COUNT> bank_bag_slot;
    std::array<uint64_t, WOWPLAYER_BUY_BACK_COUNT> vendor_buy_back_slot;
    uint64_t farsight_guid;
    std::array<uint64_t, WOWPLAYER_KNOWN_TITLES_SIZE> field_known_titles;
    uint64_t field_coinage;
    uint32_t xp;
    uint32_t next_level_xp;

    union skill_info_union
    {
        struct parts
        {
            std::array<uint32_t, 64> skill_line;
            std::array<uint32_t, 64> skill_step;
            std::array<uint32_t, 64> skill_rank;
            std::array<uint32_t, 64> skill_starting_rank;
            std::array<uint32_t, 64> skill_max_rank;
            std::array<uint32_t, 64> skill_mod;
            std::array<uint32_t, 64> skill_talent;
        } skill_info_parts;
        std::array<uint32_t, WOWPLAYER_SKILL_INFO_COUNT> skill_info;
    } field_skill_info;

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
    std::array<float, WOWPLAYER_SPELL_SCHOOL_COUNT> spell_crit_pct;
    uint32_t shield_block;
    float shield_block_crit_pct;
    uint32_t mastery;
    uint32_t pvp_power_damage;
    uint32_t pvp_power_healing;
    std::array<uint32_t, WOWPLAYER_EXPLORED_ZONES_COUNT> explored_zones;
    uint32_t rest_state_xp;
    std::array<uint32_t, WOWPLAYER_SPELL_SCHOOL_COUNT> field_mod_damage_done_positive;
    std::array<uint32_t, WOWPLAYER_SPELL_SCHOOL_COUNT> field_mod_damage_done_negative;
    std::array<float, WOWPLAYER_SPELL_SCHOOL_COUNT> field_mod_damage_done_pct;
    uint32_t field_mod_healing_done;
    float field_mod_healing_pct;
    float field_mod_healing_done_pct;
    float field_mod_periodic_healing_done_pct;
    std::array<float, WOWPLAYER_WEAPON_DMG_MULTIPLIER_COUNT> weapon_dmg_multiplier;
    float mod_spell_power_pct;
    float mod_resilience_pct;
    float override_spell_power_by_ap_pct;
    float override_ap_by_spell_power_pct;
    uint32_t field_mod_target_resistance;
    uint32_t field_mod_target_physical_resistance;
    player_field_bytes_union player_field_bytes;
    uint32_t self_resurrection_spell;
    uint32_t field_pvp_medals;
    std::array<uint32_t, WOWPLAYER_BUY_BACK_COUNT> field_buy_back_price;
    std::array<uint32_t, WOWPLAYER_BUY_BACK_COUNT> field_buy_back_timestamp;
    union field_kills_union
    {
        struct parts
        {
            uint16_t kills_today;
            uint16_t kills_yesterday;
        } kills_field_parts;
        uint32_t raw;
    } field_kills;
    uint32_t field_lifetime_honorable_kills;
    uint32_t field_watched_faction_idx;
    std::array<uint32_t, WOWPLAYER_COMBAT_RATING_COUNT> field_combat_rating;
    std::array<WoWPlayer_ArenaTeamInfo, WOWPLAYER_ARENA_TEAM_SLOTS> field_arena_team_info;
    uint32_t field_max_level;
    std::array<float, WOWPLAYER_RUNE_REGEN_COUNT> rune_regen;
    std::array<uint32_t, WOWPLAYER_NO_REAGENT_COST_COUNT> no_reagent_cost;
    std::array<uint32_t, WOWPLAYER_GLYPH_SLOT_COUNT> field_glyph_slots;
    std::array<uint32_t, WOWPLAYER_GLYPH_SLOT_COUNT> field_glyphs;
    uint32_t glyphs_enabled;
    uint32_t pet_spell_power;
    std::array<uint32_t, WOWPLAYER_RESEARCHING_COUNT> researching;
    std::array<uint32_t, WOWPLAYER_PROFESSION_SKILL_COUNT> profession_skill_line;
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
