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

#include "WoWObject.hpp"

#include <array>

#pragma pack(push, 1)

union field_bytes_0_union
{
    struct parts
    {
        uint8_t race;
        uint8_t unit_class;
        uint8_t gender;
        uint8_t power_type;
    } s;
    uint32_t raw;
};

union unit_virtual_item_info
{
    struct parts
    {
        uint8_t item_class;
        uint8_t item_subclass;
        int8_t unk0;
        uint8_t material;
        uint8_t inventory_type;
        uint8_t sheath;
    } fields;
    uint64_t raw;
};

#if VERSION_STRING == Classic
static inline constexpr uint8_t WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT = 3;
static inline constexpr uint8_t WOWUNIT_AURA_COUNT = 48;
static inline constexpr uint8_t WOWUNIT_AURA_FLAGS_COUNT = 6;
static inline constexpr uint8_t WOWUNIT_AURA_LEVELS_COUNT = 12;
static inline constexpr uint8_t WOWUNIT_AURA_APPLICATIONS_COUNT = 12;
static inline constexpr uint8_t WOWUNIT_SPELL_SCHOOL_COUNT = 7;
static inline constexpr uint8_t WOWUNIT_ATTACK_TIME_COUNT = 3;
static inline constexpr uint8_t WOWUNIT_STAT_COUNT = 5;

union field_bytes_1_union
{
    struct parts
    {
        uint8_t stand_state;
        uint8_t pet_loyalty; // 0 for non pet creature
        uint8_t shape_shift_form;
        uint8_t stand_state_flag;
    } s;
    uint32_t raw;
};

union field_bytes_2_union
{
    struct parts
    {
        uint8_t sheath_type;
        uint8_t unk1; // some sort of flag
        uint8_t unk2;
        uint8_t unk3;
    } s;
    uint32_t raw;
};

struct WoWUnit : WoWObject
{
    guid_union charm_guid;
    guid_union summon_guid;
    guid_union charmed_by_guid;
    guid_union summoned_by_guid;
    guid_union created_by_guid;
    guid_union target_guid;
    guid_union persuaded_guid;
    guid_union channel_object_guid;
    uint32_t health;
    uint32_t power_1;
    uint32_t power_2;
    uint32_t power_3;
    uint32_t power_4;
    uint32_t power_5;
    uint32_t max_health;
    uint32_t max_power_1;
    uint32_t max_power_2;
    uint32_t max_power_3;
    uint32_t max_power_4;
    uint32_t max_power_5;
    uint32_t level;
    uint32_t faction_template;
    field_bytes_0_union field_bytes_0;
    std::array<uint32_t, WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT> virtual_item_slot_display;       //0 = melee, 1 = offhand, 2 = ranged
    std::array<unit_virtual_item_info, WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT> virtual_item_info; //0 = melee, 1 = offhand, 2 = ranged
    uint32_t unit_flags;
    std::array<uint32_t, WOWUNIT_AURA_COUNT> aura;
    std::array<uint32_t, WOWUNIT_AURA_FLAGS_COUNT> aura_flags;
    std::array<uint32_t, WOWUNIT_AURA_LEVELS_COUNT> aura_levels;
    std::array<uint32_t, WOWUNIT_AURA_APPLICATIONS_COUNT> aura_applications;
    uint32_t aura_state;
    std::array<uint32_t, WOWUNIT_ATTACK_TIME_COUNT> base_attack_time;  //0 = melee, 1 = offhand, 2 = ranged
    float bounding_radius;
    float combat_reach;
    uint32_t display_id;
    uint32_t native_display_id;
    uint32_t mount_display_id;
    float minimum_damage;
    float maximum_damage;
    float minimum_offhand_damage;
    float maximum_offhand_damage;
    field_bytes_1_union field_bytes_1;
    uint32_t pet_number;
    uint32_t pet_name_timestamp;
    uint32_t pet_experience;
    uint32_t pet_next_level_experience;
    uint32_t dynamic_flags;
    uint32_t channel_spell;
    float mod_cast_speed;
    uint32_t created_by_spell_id;
    uint32_t npc_flags;
    uint32_t npc_emote_state;
    uint32_t training_points;
    std::array<uint32_t, WOWUNIT_STAT_COUNT> stat;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> resistance;
    uint32_t base_mana;
    uint32_t base_health;
    field_bytes_2_union field_bytes_2;
    uint32_t attack_power;
    int32_t attack_power_mods;
    float attack_power_multiplier;
    int32_t ranged_attack_power;
    int32_t ranged_attack_power_mods;
    float ranged_attack_power_multiplier;
    float minimum_ranged_damage;
    float maximum_ranged_ddamage;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> power_cost_modifier;
    std::array<float, WOWUNIT_SPELL_SCHOOL_COUNT> power_cost_multiplier;
    uint32_t unit_padding;
};
#elif VERSION_STRING == TBC
static inline constexpr uint8_t WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT = 3;
static inline constexpr uint8_t WOWUNIT_AURA_COUNT = 56;
static inline constexpr uint8_t WOWUNIT_AURA_FLAGS_COUNT = 14;
static inline constexpr uint8_t WOWUNIT_AURA_LEVELS_COUNT = 14;
static inline constexpr uint8_t WOWUNIT_AURA_APPLICATIONS_COUNT = 14;
static inline constexpr uint8_t WOWUNIT_SPELL_SCHOOL_COUNT = 7;
static inline constexpr uint8_t WOWUNIT_ATTACK_TIME_COUNT = 3;
static inline constexpr uint8_t WOWUNIT_STAT_COUNT = 5;

union field_bytes_1_union
{
    struct parts
    {
        uint8_t stand_state;
        uint8_t pet_loyalty; // 0 for non pet creature
        uint8_t stand_state_flag;
        uint8_t animation_flag;
    } s;
    uint32_t raw;
};

union field_bytes_2_union
{
    struct parts
    {
        uint8_t sheath_type;
        uint8_t positive_aura_limit;
        uint8_t pet_flag;
        uint8_t shape_shift_form;
    } s;
    uint32_t raw;
};

struct WoWUnit : WoWObject
{
    guid_union charm_guid;
    guid_union summon_guid;
    guid_union charmed_by_guid;
    guid_union summoned_by_guid;
    guid_union created_by_guid;
    guid_union target_guid;
    guid_union persuaded_guid;
    guid_union channel_object_guid;
    uint32_t health;
    uint32_t power_1;
    uint32_t power_2;
    uint32_t power_3;
    uint32_t power_4;
    uint32_t power_5;
    uint32_t max_health;
    uint32_t max_power_1;
    uint32_t max_power_2;
    uint32_t max_power_3;
    uint32_t max_power_4;
    uint32_t max_power_5;
    uint32_t level;
    uint32_t faction_template;
    field_bytes_0_union field_bytes_0;
    std::array<uint32_t, WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT> virtual_item_slot_display;       //0 = melee, 1 = offhand, 2 = ranged
    std::array<unit_virtual_item_info, WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT> virtual_item_info; //0 = melee, 1 = offhand, 2 = ranged
    uint32_t unit_flags;
    uint32_t unit_flags_2;
    std::array<uint32_t, WOWUNIT_AURA_COUNT> aura;
    std::array<uint32_t, WOWUNIT_AURA_FLAGS_COUNT> aura_flags;
    std::array<uint32_t, WOWUNIT_AURA_LEVELS_COUNT> aura_levels;
    std::array<uint32_t, WOWUNIT_AURA_APPLICATIONS_COUNT> aura_applications;
    uint32_t aura_state;
    std::array<uint32_t, WOWUNIT_ATTACK_TIME_COUNT> base_attack_time;  //0 = melee, 1 = offhand, 2 = ranged
    float bounding_radius;
    float combat_reach;
    uint32_t display_id;
    uint32_t native_display_id;
    uint32_t mount_display_id;
    float minimum_damage;
    float maximum_damage;
    float minimum_offhand_damage;
    float maximum_offhand_damage;
    field_bytes_1_union field_bytes_1;
    uint32_t pet_number;
    uint32_t pet_name_timestamp;
    uint32_t pet_experience;
    uint32_t pet_next_level_experience;
    uint32_t dynamic_flags;
    uint32_t channel_spell;
    float mod_cast_speed;
    uint32_t created_by_spell_id;
    uint32_t npc_flags;
    uint32_t npc_emote_state;
    uint32_t training_points;
    std::array<uint32_t, WOWUNIT_STAT_COUNT> stat;
    std::array<uint32_t, WOWUNIT_STAT_COUNT> positive_stat;
    std::array<uint32_t, WOWUNIT_STAT_COUNT> negative_stat;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> resistance;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> resistance_buff_mod_positive;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> resistance_buff_mod_negative;
    uint32_t base_mana;
    uint32_t base_health;
    field_bytes_2_union field_bytes_2;
    uint32_t attack_power;
    int32_t attack_power_mods;
    float attack_power_multiplier;
    int32_t ranged_attack_power;
    int32_t ranged_attack_power_mods;
    float ranged_attack_power_multiplier;
    float minimum_ranged_damage;
    float maximum_ranged_ddamage;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> power_cost_modifier;
    std::array<float, WOWUNIT_SPELL_SCHOOL_COUNT> power_cost_multiplier;
    float max_health_modifier;    // not used
    uint32_t unit_padding;
};
#elif VERSION_STRING == WotLK
static inline constexpr uint8_t WOWUNIT_POWER_COUNT = 7;
static inline constexpr uint8_t WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT = 3;
static inline constexpr uint8_t WOWUNIT_SPELL_SCHOOL_COUNT = 7;
static inline constexpr uint8_t WOWUNIT_ATTACK_TIME_COUNT = 3;
static inline constexpr uint8_t WOWUNIT_STAT_COUNT = 5;

union field_bytes_1_union
{
    struct parts
    {
        uint8_t stand_state;
        uint8_t pet_talent_points; // 0 for non pet creature
        uint8_t stand_state_flag;
        uint8_t animation_flag;
    } s;
    uint32_t raw;
};

union field_bytes_2_union
{
    struct parts
    {
        uint8_t sheath_type;
        uint8_t pvp_flag;
        uint8_t pet_flag;
        uint8_t shape_shift_form;
    } s;
    uint32_t raw;
};

struct WoWUnit : WoWObject
{
    guid_union charm_guid;
    guid_union summon_guid;
    guid_union critter_guid;
    guid_union charmed_by_guid;
    guid_union summoned_by_guid;
    guid_union created_by_guid;
    guid_union target_guid;
    guid_union channel_object_guid;
    uint32_t channel_spell;
    field_bytes_0_union field_bytes_0;
    uint32_t health;
    uint32_t power_1;
    uint32_t power_2;
    uint32_t power_3;
    uint32_t power_4;
    uint32_t power_5;
    uint32_t power_6;
    uint32_t power_7;
    uint32_t max_health;
    uint32_t max_power_1;
    uint32_t max_power_2;
    uint32_t max_power_3;
    uint32_t max_power_4;
    uint32_t max_power_5;
    uint32_t max_power_6;
    uint32_t max_power_7;
    std::array<float, WOWUNIT_POWER_COUNT> power_regen_flat_modifier;
    std::array<float, WOWUNIT_POWER_COUNT> power_regen_interrupted_flat_modifier;
    uint32_t level;
    uint32_t faction_template;
    std::array<uint32_t, WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT> virtual_item_slot_display;    //0 = melee, 1 = offhand, 2 = ranged
    uint32_t unit_flags;
    uint32_t unit_flags_2;
    uint32_t aura_state;
    std::array<uint32_t, WOWUNIT_ATTACK_TIME_COUNT> base_attack_time;  //0 = melee, 1 = offhand, 2 = ranged
    float bounding_radius;
    float combat_reach;
    uint32_t display_id;
    uint32_t native_display_id;
    uint32_t mount_display_id;
    float minimum_damage;
    float maximum_damage;
    float minimum_offhand_damage;
    float maximum_offhand_damage;
    field_bytes_1_union field_bytes_1;
    uint32_t pet_number;
    uint32_t pet_name_timestamp;
    uint32_t pet_experience;
    uint32_t pet_next_level_experience;
    uint32_t dynamic_flags;
    float mod_cast_speed;
    uint32_t created_by_spell_id;
    uint32_t npc_flags;
    uint32_t npc_emote_state;
    std::array<uint32_t, WOWUNIT_STAT_COUNT> stat;
    std::array<uint32_t, WOWUNIT_STAT_COUNT> positive_stat;
    std::array<uint32_t, WOWUNIT_STAT_COUNT> negative_stat;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> resistance;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> resistance_buff_mod_positive;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> resistance_buff_mod_negative;
    uint32_t base_mana;
    uint32_t base_health;
    field_bytes_2_union field_bytes_2;
    uint32_t attack_power;
    int32_t attack_power_mods;
    float attack_power_multiplier;
    int32_t ranged_attack_power;
    int32_t ranged_attack_power_mods;
    float ranged_attack_power_multiplier;
    float minimum_ranged_damage;
    float maximum_ranged_ddamage;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> power_cost_modifier;
    std::array<float, WOWUNIT_SPELL_SCHOOL_COUNT> power_cost_multiplier;
    float max_health_modifier;
    float hover_height;
    uint32_t unit_padding;
};
#elif VERSION_STRING == Cata
static inline constexpr uint8_t WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT = 3;
static inline constexpr uint8_t WOWUNIT_POWER_COUNT = 5;
static inline constexpr uint8_t WOWUNIT_SPELL_SCHOOL_COUNT = 7;
static inline constexpr uint8_t WOWUNIT_ATTACK_TIME_COUNT = 3;
static inline constexpr uint8_t WOWUNIT_STAT_COUNT = 5;

union field_bytes_1_union
{
    struct parts
    {
        uint8_t stand_state;
        uint8_t pet_talent_points; // 0 for non pet creature
        uint8_t stand_state_flag;
        uint8_t animation_flag;
    } s;
    uint32_t raw;
};

union field_bytes_2_union
{
    struct parts
    {
        uint8_t sheath_type;
        uint8_t pvp_flag;
        uint8_t pet_flag;
        uint8_t shape_shift_form;
    } s;
    uint32_t raw;
};

struct WoWUnit : WoWObject
{
    guid_union charm_guid;
    guid_union summon_guid;
    guid_union critter_guid;
    guid_union charmed_by_guid;
    guid_union summoned_by_guid;
    guid_union created_by_guid;
    guid_union target_guid;
    guid_union channel_object_guid;
    uint32_t channel_spell;
    field_bytes_0_union field_bytes_0;
    uint32_t health;
    uint32_t power_1;
    uint32_t power_2;
    uint32_t power_3;
    uint32_t power_4;
    uint32_t power_5;
    uint32_t max_health;
    uint32_t max_power_1;
    uint32_t max_power_2;
    uint32_t max_power_3;
    uint32_t max_power_4;
    uint32_t max_power_5;
    std::array<float, WOWUNIT_POWER_COUNT> power_regen_flat_modifier;
    std::array<float, WOWUNIT_POWER_COUNT> power_regen_interrupted_flat_modifier;
    uint32_t level;
    uint32_t faction_template;
    std::array<uint32_t, WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT> virtual_item_slot_display;    //0 = melee, 1 = offhand, 2 = ranged
    uint32_t unit_flags;
    uint32_t unit_flags_2;
    uint32_t aura_state;
    std::array<uint32_t, WOWUNIT_ATTACK_TIME_COUNT> base_attack_time;  //0 = melee, 1 = offhand, 2 = ranged
    float bounding_radius;
    float combat_reach;
    uint32_t display_id;
    uint32_t native_display_id;
    uint32_t mount_display_id;
    float minimum_damage;
    float maximum_damage;
    float minimum_offhand_damage;
    float maximum_offhand_damage;
    field_bytes_1_union field_bytes_1;
    uint32_t pet_number;
    uint32_t pet_name_timestamp;
    uint32_t pet_experience;
    uint32_t pet_next_level_experience;
    uint32_t dynamic_flags;
    float mod_cast_speed;
    float mod_cast_haste;
    uint32_t created_by_spell_id;
    uint32_t npc_flags;
    uint32_t npc_emote_state;
    std::array<uint32_t, WOWUNIT_STAT_COUNT> stat;
    std::array<uint32_t, WOWUNIT_STAT_COUNT> positive_stat;
    std::array<uint32_t, WOWUNIT_STAT_COUNT> negative_stat;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> resistance;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> resistance_buff_mod_positive;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> resistance_buff_mod_negative;
    uint32_t base_mana;
    uint32_t base_health;
    field_bytes_2_union field_bytes_2;
    uint32_t attack_power;
    uint32_t attack_power_mod_pos;
    uint32_t attack_power_mod_neg;
    float attack_power_multiplier;
    int32_t ranged_attack_power;
    uint32_t ranged_attack_power_mods_pos;
    uint32_t ranged_attack_power_mods_neg;
    float ranged_attack_power_multiplier;
    float minimum_ranged_damage;
    float maximum_ranged_ddamage;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> power_cost_modifier;
    std::array<float, WOWUNIT_SPELL_SCHOOL_COUNT> power_cost_multiplier;
    float max_health_modifier;
    float hover_height;
    uint32_t max_item_level;
    uint32_t unit_padding;
};
#elif VERSION_STRING == Mop
static inline constexpr uint8_t WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT = 3;
static inline constexpr uint8_t WOWUNIT_POWER_COUNT = 5;
static inline constexpr uint8_t WOWUNIT_SPELL_SCHOOL_COUNT = 7;
static inline constexpr uint8_t WOWUNIT_ATTACK_TIME_COUNT = 2;
static inline constexpr uint8_t WOWUNIT_STAT_COUNT = 5;

union field_bytes_1_union
{
    struct parts
    {
        uint8_t stand_state;
        uint8_t unk1; // possibly pet specialization
        uint8_t stand_state_flag;
        uint8_t animation_flag;
    } s;
    uint32_t raw;
};

union field_bytes_2_union
{
    struct parts
    {
        uint8_t sheath_type;
        uint8_t pvp_flag;
        uint8_t pet_flag;
        uint8_t shape_shift_form;
    } s;
    uint32_t raw;
};

struct WoWUnit : WoWObject
{
    guid_union charm_guid;
    guid_union summon_guid;
    guid_union critter_guid;
    guid_union charmed_by_guid;
    guid_union summoned_by_guid;
    guid_union created_by_guid;
    guid_union demon_creator_guid;
    guid_union target_guid;
    guid_union battle_pet_companion_guid;
    guid_union channel_object_guid;
    uint32_t channel_spell;
    uint32_t summoned_by_home_realm;
    field_bytes_0_union field_bytes_0;
    uint32_t display_power;
    uint32_t override_display_power_id;
    uint32_t health;
    uint32_t power_1;
    uint32_t power_2;
    uint32_t power_3;
    uint32_t power_4;
    uint32_t power_5;
    uint32_t max_health;
    uint32_t max_power_1;
    uint32_t max_power_2;
    uint32_t max_power_3;
    uint32_t max_power_4;
    uint32_t max_power_5;
    std::array<float, WOWUNIT_POWER_COUNT> power_regen_flat_modifier;
    std::array<float, WOWUNIT_POWER_COUNT> power_regen_interrupted_flat_modifier;
    uint32_t level;
    uint32_t effective_level;
    uint32_t faction_template;
    std::array<uint32_t, WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT> virtual_item_slot_display;    //0 = melee, 1 = offhand, 2 = ranged
    uint32_t unit_flags;
    uint32_t unit_flags_2;
    uint32_t aura_state;
    std::array<uint32_t, WOWUNIT_ATTACK_TIME_COUNT> base_attack_time;  //0 = melee, 1 = offhand
    uint32_t ranged_attack_time;
    float bounding_radius;
    float combat_reach;
    uint32_t display_id;
    uint32_t native_display_id;
    uint32_t mount_display_id;
    float minimum_damage;
    float maximum_damage;
    float minimum_offhand_damage;
    float maximum_offhand_damage;
    field_bytes_1_union field_bytes_1;
    uint32_t pet_number;
    uint32_t pet_name_timestamp;
    uint32_t pet_experience;
    uint32_t pet_next_level_experience;
    float mod_cast_speed;
    float mod_cast_haste;
    float mod_haste;
    float mod_ranged_haste;
    float mod_haste_regen;
    uint32_t created_by_spell_id;
    uint64_t npc_flags;
    uint32_t npc_emote_state;
    std::array<uint32_t, WOWUNIT_STAT_COUNT> stat;
    std::array<uint32_t, WOWUNIT_STAT_COUNT> positive_stat;
    std::array<uint32_t, WOWUNIT_STAT_COUNT> negative_stat;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> resistance;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> resistance_buff_mod_positive;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> resistance_buff_mod_negative;
    uint32_t base_mana;
    uint32_t base_health;
    field_bytes_2_union field_bytes_2;
    uint32_t attack_power;
    uint32_t attack_power_mod_pos;
    uint32_t attack_power_mod_neg;
    float attack_power_multiplier;
    int32_t ranged_attack_power;
    uint32_t ranged_attack_power_mods_pos;
    uint32_t ranged_attack_power_mods_neg;
    float ranged_attack_power_multiplier;
    float minimum_ranged_damage;
    float maximum_ranged_ddamage;
    std::array<uint32_t, WOWUNIT_SPELL_SCHOOL_COUNT> power_cost_modifier;
    std::array<float, WOWUNIT_SPELL_SCHOOL_COUNT> power_cost_multiplier;
    float max_health_modifier;
    float hover_height;
    uint32_t min_item_level;
    uint32_t max_item_level;
    uint32_t wild_battle_pet_level;
    uint32_t battle_pet_companion_name_timestamp;
    uint32_t interact_spell_id;
};
#endif
#pragma pack(pop)
