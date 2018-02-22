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
#include "WoWObject.h"
#include "GuidData.h"
#pragma pack(push, 1)

union
{
    struct
    {
        uint8_t race;
        uint8_t unit_class;
        uint8_t gender;
        uint8_t power_type;
    } s;
    uint32_t raw;
} typedef field_bytes_0_union;

union
{
    struct
    {
        uint8_t stand_state;
        uint8_t pet_talent_points; // 0 for non pet creature
        uint8_t stand_state_flag;
        uint8_t animation_flag;
    } s;
    uint32_t raw;
} typedef field_bytes_1_union;

union
{
    struct
    {
        uint8_t sheath_type;
        uint8_t flag;
        uint8_t rename_flag;
        uint8_t shape_shift_form;
    } s;
    uint32_t raw;
} typedef field_bytes_2_union;

#if VERSION_STRING == Classic
#define WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT 3
#define WOWUNIT_VIRTUAL_ITEM_INFO_COUNT 6
#define WOWUNIT_AURA_COUNT 48
#define WOWUNIT_AURA_FLAGS_COUNT 6
#define WOWUNIT_AURA_LEVELS_COUNT 12
#define WOWUNIT_AURA_APPLICATIONS_COUNT 12
#define WOWUNIT_RESISTANCE_COUNT 7
#define WOWUNIT_POWER_COST_MODIFIER 7
#define WOWUNIT_POWER_COST_MULTIPLIER 7

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
    union
    {
        uint32_t max_power_1;
        uint32_t max_mana;
    };
    uint32_t max_power_2;
    uint32_t max_power_3;
    uint32_t max_power_4;
    uint32_t max_power_5;
    uint32_t level;
    uint32_t faction_template;
    field_bytes_0_union field_bytes_0;
    uint32_t virtual_item_slot_display[WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT];
    uint32_t virtual_item_info[WOWUNIT_VIRTUAL_ITEM_INFO_COUNT];
    uint32_t unit_flags;
    uint32_t aura[WOWUNIT_AURA_COUNT];
    uint32_t aura_flags[WOWUNIT_AURA_FLAGS_COUNT];
    uint32_t aura_levels[WOWUNIT_AURA_LEVELS_COUNT];
    uint32_t aura_applications[WOWUNIT_AURA_APPLICATIONS_COUNT];
    uint32_t aura_state;
    uint64_t base_attack_time;
    uint32_t ranged_attack_time;
    float_t bounding_radius;
    float_t combat_reach;
    uint32_t display_id;
    uint32_t native_display_id;
    uint32_t mount_display_id;
    float_t minimum_damage;
    float_t maximum_damage;
    float_t minimum_offhand_damage;
    float_t maximum_offhand_damage;
    field_bytes_1_union field_bytes_1;
    uint32_t pet_number;
    uint32_t pet_name_timestamp;
    uint32_t pet_experience;
    uint32_t pet_next_level_experience;
    uint32_t dynamic_flags;
    uint32_t channel_spell_id;
    float_t mod_cast_speed;
    uint32_t created_by_spell_id;
    uint32_t npc_flags;
    uint32_t npc_emote_state;
    uint32_t training_points;
    uint32_t stat_0;
    uint32_t stat_1;
    uint32_t stat_2;
    uint32_t stat_3;
    uint32_t stat_4;
    uint32_t resistance[WOWUNIT_RESISTANCE_COUNT];
    uint32_t base_mana;
    uint32_t base_health;
    field_bytes_2_union field_bytes_2;
    uint32_t attack_power;
    uint32_t attack_power_mods;
    float_t attack_power_multiplier;
    uint32_t ranged_attack_power;
    uint32_t ranged_attack_power_mods;
    float_t ranged_attack_power_multiplier;
    float_t minimum_ranged_damage;
    float_t maximum_ranged_ddamage;
    uint32_t power_cost_modifier[WOWUNIT_POWER_COST_MODIFIER];
    float_t power_cost_multiplier[WOWUNIT_POWER_COST_MULTIPLIER];
    uint32_t unit_padding;
};
#elif VERSION_STRING == TBC
#define WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT 3
#define WOWUNIT_VIRTUAL_ITEM_INFO_COUNT 6
#define WOWUNIT_AURA_COUNT 56
#define WOWUNIT_AURA_FLAGS_COUNT 14
#define WOWUNIT_AURA_LEVELS_COUNT 14
#define WOWUNIT_AURA_APPLICATIONS_COUNT 14
#define WOWUNIT_RESISTANCE_COUNT 7
#define WOWUNIT_RESISTANCE_BUFF_MOD_POSITIVE_COUNT 7
#define WOWUNIT_RESISTANCE_BUFF_MOD_NEGATIVE_COUNT 7
#define WOWUNIT_POWER_COST_MODIFIER 7
#define WOWUNIT_POWER_COST_MULTIPLIER 7
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
    union
    {
        uint32_t max_power_1;
        uint32_t max_mana;
    };
    uint32_t max_power_2;
    uint32_t max_power_3;
    uint32_t max_power_4;
    uint32_t max_power_5;
    uint32_t level;
    uint32_t faction_template;
    field_bytes_0_union field_bytes_0;
    uint32_t virtual_item_slot_display[WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT];
    uint32_t virtual_item_info[WOWUNIT_VIRTUAL_ITEM_INFO_COUNT];
    uint32_t unit_flags;
    uint32_t unit_flags_2;
    uint32_t aura[WOWUNIT_AURA_COUNT];
    uint32_t aura_flags[WOWUNIT_AURA_FLAGS_COUNT];
    uint32_t aura_levels[WOWUNIT_AURA_LEVELS_COUNT];
    uint32_t aura_applications[WOWUNIT_AURA_APPLICATIONS_COUNT];
    uint32_t aura_state;
    uint64_t base_attack_time;
    uint32_t ranged_attack_time;
    float_t bounding_radius;
    float_t combat_reach;
    uint32_t display_id;
    uint32_t native_display_id;
    uint32_t mount_display_id;
    float_t minimum_damage;
    float_t maximum_damage;
    float_t minimum_offhand_damage;
    float_t maximum_offhand_damage;
    field_bytes_1_union field_bytes_1;
    uint32_t pet_number;
    uint32_t pet_name_timestamp;
    uint32_t pet_experience;
    uint32_t pet_next_level_experience;
    uint32_t dynamic_flags;
    uint32_t channel_spell_id;
    float_t mod_cast_speed;
    uint32_t created_by_spell_id;
    uint32_t npc_flags;
    uint32_t npc_emote_state;
    uint32_t training_points;
    uint32_t stat_0;
    uint32_t stat_1;
    uint32_t stat_2;
    uint32_t stat_3;
    uint32_t stat_4;
    uint32_t positive_stat_0;
    uint32_t positive_stat_1;
    uint32_t positive_stat_2;
    uint32_t positive_stat_3;
    uint32_t positive_stat_4;
    uint32_t negative_stat_0;
    uint32_t negative_stat_1;
    uint32_t negative_stat_2;
    uint32_t negative_stat_3;
    uint32_t negative_stat_4;
    uint32_t resistance[WOWUNIT_RESISTANCE_COUNT];
    uint32_t resistance_buff_mod_positive[WOWUNIT_RESISTANCE_BUFF_MOD_POSITIVE_COUNT];
    uint32_t resistance_buff_mod_negative[WOWUNIT_RESISTANCE_BUFF_MOD_NEGATIVE_COUNT];
    uint32_t base_mana;
    uint32_t base_health;
    field_bytes_2_union field_bytes_2;
    uint32_t attack_power;
    uint32_t attack_power_mods;
    float_t attack_power_multiplier;
    uint32_t ranged_attack_power;
    uint32_t ranged_attack_power_mods;
    float_t ranged_attack_power_multiplier;
    float_t minimum_ranged_damage;
    float_t maximum_ranged_ddamage;
    uint32_t power_cost_modifier[WOWUNIT_POWER_COST_MODIFIER];
    float_t power_cost_multiplier[WOWUNIT_POWER_COST_MULTIPLIER];
    float_t max_health_modifier;
    uint32_t unit_padding;
};
#elif VERSION_STRING == WotLK
#define WOWUNIT_POWER_COUNT 7
#define WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT 3
#define WOWUNIT_RESISTANCE_COUNT 7
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
    union
    {
        uint32_t max_power_1;
        uint32_t max_mana;
    };
    uint32_t max_power_2;
    uint32_t max_power_3;
    uint32_t max_power_4;
    uint32_t max_power_5;
    uint32_t max_power_6;
    uint32_t max_power_7;
    float_t power_regen_flat_modifier[WOWUNIT_POWER_COUNT];
    float_t power_regen_interrupted_flat_modifier[WOWUNIT_POWER_COUNT];
    uint32_t level;
    uint32_t faction_template;
    uint32_t virtual_item_slot_display[WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT];
    uint32_t unit_flags;
    uint32_t unit_flags_2;
    uint32_t aura_state;
    uint64_t base_attack_time;
    uint32_t ranged_attack_time;
    float_t bounding_radius;
    float_t combat_reach;
    uint32_t display_id;
    uint32_t native_display_id;
    uint32_t mount_display_id;
    float_t minimum_damage;
    float_t maximum_damage;
    float_t minimum_offhand_damage;
    float_t maximum_offhand_damage;
    field_bytes_1_union field_bytes_1;
    uint32_t pet_number;
    uint32_t pet_name_timestamp;
    uint32_t pet_experience;
    uint32_t pet_next_level_experience;
    uint32_t dynamic_flags;
    float_t mod_cast_speed;
    uint32_t created_by_spell_id;
    uint32_t npc_flags;
    uint32_t npc_emote_state;
    uint32_t stat_0;
    uint32_t stat_1;
    uint32_t stat_2;
    uint32_t stat_3;
    uint32_t stat_4;
    uint32_t positive_stat_0;
    uint32_t positive_stat_1;
    uint32_t positive_stat_2;
    uint32_t positive_stat_3;
    uint32_t positive_stat_4;
    uint32_t negative_stat_0;
    uint32_t negative_stat_1;
    uint32_t negative_stat_2;
    uint32_t negative_stat_3;
    uint32_t negative_stat_4;
    uint32_t resistance[WOWUNIT_RESISTANCE_COUNT];
    uint32_t resistance_buff_mod_positive[WOWUNIT_RESISTANCE_COUNT];
    uint32_t resistance_buff_mod_negative[WOWUNIT_RESISTANCE_COUNT];
    uint32_t base_mana;
    uint32_t base_health;
    field_bytes_2_union field_bytes_2;
    uint32_t attack_power;
    uint32_t attack_power_mods;
    float_t attack_power_multiplier;
    uint32_t ranged_attack_power;
    uint32_t ranged_attack_power_mods;
    float_t ranged_attack_power_multiplier;
    float_t minimum_ranged_damage;
    float_t maximum_ranged_ddamage;
    uint32_t power_cost_modifier[WOWUNIT_POWER_COUNT];
    float_t power_cost_multiplier[WOWUNIT_POWER_COUNT];
    float_t max_health_modifier;
    float_t hover_height;
    uint32_t unit_padding;
};
#elif VERSION_STRING == Cata
#define WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT 3
#define WOWUNIT_RESISTANCE_COUNT 7
#define WOWUNIT_RESISTANCE_BUFF_MOD_POSITIVE_COUNT 7
#define WOWUNIT_RESISTANCE_BUFF_MOD_NEGATIVE_COUNT 7
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
    union
    {
        uint32_t max_power_1;
        uint32_t max_mana;
    };
    uint32_t max_power_2;
    uint32_t max_power_3;
    uint32_t max_power_4;
    uint32_t max_power_5;
    float_t power_regen_flat_modifier[5];
    float_t power_regen_interrupted_flat_modifier[5];
    uint32_t level;
    uint32_t faction_template;
    uint32_t virtual_item_slot_display[WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT];
    uint32_t unit_flags;
    uint32_t unit_flags_2;
    uint32_t aura_state;
    uint64_t base_attack_time;
    uint32_t ranged_attack_time;
    float_t bounding_radius;
    float_t combat_reach;
    uint32_t display_id;
    uint32_t native_display_id;
    uint32_t mount_display_id;
    float_t minimum_damage;
    float_t maximum_damage;
    float_t minimum_offhand_damage;
    float_t maximum_offhand_damage;
    field_bytes_1_union field_bytes_1;
    uint32_t pet_number;
    uint32_t pet_name_timestamp;
    uint32_t pet_experience;
    uint32_t pet_next_level_experience;
    uint32_t dynamic_flags;
    float_t mod_cast_speed;
    float_t mod_cast_haste;
    uint32_t created_by_spell_id;
    uint32_t npc_flags;
    uint32_t npc_emote_state;
    uint32_t stat_0;
    uint32_t stat_1;
    uint32_t stat_2;
    uint32_t stat_3;
    uint32_t stat_4;
    uint32_t positive_stat_0;
    uint32_t positive_stat_1;
    uint32_t positive_stat_2;
    uint32_t positive_stat_3;
    uint32_t positive_stat_4;
    uint32_t negative_stat_0;
    uint32_t negative_stat_1;
    uint32_t negative_stat_2;
    uint32_t negative_stat_3;
    uint32_t negative_stat_4;
    uint32_t resistance[WOWUNIT_RESISTANCE_COUNT];
    uint32_t resistance_buff_mod_positive[WOWUNIT_RESISTANCE_BUFF_MOD_POSITIVE_COUNT];
    uint32_t resistance_buff_mod_negative[WOWUNIT_RESISTANCE_BUFF_MOD_NEGATIVE_COUNT];
    uint32_t base_mana;
    uint32_t base_health;
    field_bytes_2_union field_bytes_2;
    uint32_t attack_power;
    uint32_t attack_power_mod_pos;
    uint32_t attack_power_mod_neg;
    float_t attack_power_multiplier;
    uint32_t ranged_attack_power;
    uint32_t ranged_attack_power_mods_pos;
    uint32_t ranged_attack_power_mods_neg;
    float_t ranged_attack_power_multiplier;
    float_t minimum_ranged_damage;
    float_t maximum_ranged_ddamage;
    uint32_t power_cost_modifier[2];
    float_t power_cost_multiplier[2];
    float_t max_health_modifier;
    float_t hover_height;
    uint32_t max_item_level;
    uint32_t unit_padding;
};
#endif
#pragma pack(pop)
