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
        uint8_t pvp_flag;
        uint8_t pet_flag;
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
#define WOWUNIT_ATTACK_TIME_COUNT 3
#define WOWUNIT_STAT_COUNT 5

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
    uint32_t virtual_item_slot_display[WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT];    //0 = melee, 1 = offhand, 2 = ranged
    uint32_t virtual_item_info[WOWUNIT_VIRTUAL_ITEM_INFO_COUNT];
    uint32_t unit_flags;
    uint32_t aura[WOWUNIT_AURA_COUNT];
    uint32_t aura_flags[WOWUNIT_AURA_FLAGS_COUNT];
    uint32_t aura_levels[WOWUNIT_AURA_LEVELS_COUNT];
    uint32_t aura_applications[WOWUNIT_AURA_APPLICATIONS_COUNT];
    uint32_t aura_state;
    uint32_t base_attack_time[WOWUNIT_ATTACK_TIME_COUNT];  //0 = melee, 1 = offhand, 2 = ranged
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
    uint32_t stat[WOWUNIT_STAT_COUNT];
    uint32_t resistance[WOWUNIT_RESISTANCE_COUNT];
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
    uint32_t power_cost_modifier[WOWUNIT_POWER_COST_MODIFIER];
    float power_cost_multiplier[WOWUNIT_POWER_COST_MULTIPLIER];
    uint32_t unit_padding;
};
#elif VERSION_STRING == TBC
#define WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT 3
#define WOWUNIT_VIRTUAL_ITEM_INFO_COUNT 6
#define WOWUNIT_AURA_COUNT 56
#define WOWUNIT_AURA_FLAGS_COUNT 14
#define WOWUNIT_AURA_LEVELS_COUNT 14
#define WOWUNIT_AURA_APPLICATIONS_COUNT 14
#define WOWUNIT_RESISTANCE_COUNT 7                      //school?
#define WOWUNIT_RESISTANCE_BUFF_MOD_POSITIVE_COUNT 7    //school?
#define WOWUNIT_RESISTANCE_BUFF_MOD_NEGATIVE_COUNT 7    //school?
#define WOWUNIT_POWER_COST_MODIFIER 7                   //school?
#define WOWUNIT_POWER_COST_MULTIPLIER 7                 //school?
#define WOWUNIT_ATTACK_TIME_COUNT 3
#define WOWUNIT_STAT_COUNT 5

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
    uint32_t virtual_item_slot_display[WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT];    //0 = melee, 1 = offhand, 2 = ranged
    uint32_t virtual_item_info[WOWUNIT_VIRTUAL_ITEM_INFO_COUNT];
    uint32_t unit_flags;
    uint32_t unit_flags_2;
    uint32_t aura[WOWUNIT_AURA_COUNT];
    uint32_t aura_flags[WOWUNIT_AURA_FLAGS_COUNT];
    uint32_t aura_levels[WOWUNIT_AURA_LEVELS_COUNT];
    uint32_t aura_applications[WOWUNIT_AURA_APPLICATIONS_COUNT];
    uint32_t aura_state;
    uint32_t base_attack_time[WOWUNIT_ATTACK_TIME_COUNT];  //0 = melee, 1 = offhand, 2 = ranged
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
    uint32_t stat[WOWUNIT_STAT_COUNT];
    uint32_t positive_stat[WOWUNIT_STAT_COUNT];
    uint32_t negative_stat[WOWUNIT_STAT_COUNT];
    uint32_t resistance[WOWUNIT_RESISTANCE_COUNT];
    uint32_t resistance_buff_mod_positive[WOWUNIT_RESISTANCE_BUFF_MOD_POSITIVE_COUNT];
    uint32_t resistance_buff_mod_negative[WOWUNIT_RESISTANCE_BUFF_MOD_NEGATIVE_COUNT];
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
    uint32_t power_cost_modifier[WOWUNIT_POWER_COST_MODIFIER];
    float power_cost_multiplier[WOWUNIT_POWER_COST_MULTIPLIER];
    float max_health_modifier;    // not used
    uint32_t unit_padding;
};
#elif VERSION_STRING == WotLK
#define WOWUNIT_POWER_COUNT 7
#define WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT 3
#define WOWUNIT_RESISTANCE_COUNT 7
#define WOWUNIT_ATTACK_TIME_COUNT 3
#define WOWUNIT_STAT_COUNT 5

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
    float power_regen_flat_modifier[WOWUNIT_POWER_COUNT];
    float power_regen_interrupted_flat_modifier[WOWUNIT_POWER_COUNT];
    uint32_t level;
    uint32_t faction_template;
    uint32_t virtual_item_slot_display[WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT];    //0 = melee, 1 = offhand, 2 = ranged
    uint32_t unit_flags;
    uint32_t unit_flags_2;
    uint32_t aura_state;
    uint32_t base_attack_time[WOWUNIT_ATTACK_TIME_COUNT];  //0 = melee, 1 = offhand, 2 = ranged
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
    uint32_t stat[WOWUNIT_STAT_COUNT];
    uint32_t positive_stat[WOWUNIT_STAT_COUNT];
    uint32_t negative_stat[WOWUNIT_STAT_COUNT];
    uint32_t resistance[WOWUNIT_RESISTANCE_COUNT];
    uint32_t resistance_buff_mod_positive[WOWUNIT_RESISTANCE_COUNT];
    uint32_t resistance_buff_mod_negative[WOWUNIT_RESISTANCE_COUNT];
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
    uint32_t power_cost_modifier[WOWUNIT_POWER_COUNT];
    float power_cost_multiplier[WOWUNIT_POWER_COUNT];
    float max_health_modifier;
    float hover_height;
    uint32_t unit_padding;
};
#elif VERSION_STRING == Cata
#define WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT 3
#define WOWUNIT_RESISTANCE_COUNT 7
#define WOWUNIT_RESISTANCE_BUFF_MOD_POSITIVE_COUNT 7
#define WOWUNIT_RESISTANCE_BUFF_MOD_NEGATIVE_COUNT 7
#define WOWUNIT_ATTACK_TIME_COUNT 3
#define WOWUNIT_STAT_COUNT 5

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
    float power_regen_flat_modifier[5];
    float power_regen_interrupted_flat_modifier[5];
    uint32_t level;
    uint32_t faction_template;
    uint32_t virtual_item_slot_display[WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT];    //0 = melee, 1 = offhand, 2 = ranged
    uint32_t unit_flags;
    uint32_t unit_flags_2;
    uint32_t aura_state;
    uint32_t base_attack_time[WOWUNIT_ATTACK_TIME_COUNT];  //0 = melee, 1 = offhand, 2 = ranged
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
    uint32_t stat[WOWUNIT_STAT_COUNT];
    uint32_t positive_stat[WOWUNIT_STAT_COUNT];
    uint32_t negative_stat[WOWUNIT_STAT_COUNT];
    uint32_t resistance[WOWUNIT_RESISTANCE_COUNT];
    uint32_t resistance_buff_mod_positive[WOWUNIT_RESISTANCE_BUFF_MOD_POSITIVE_COUNT];
    uint32_t resistance_buff_mod_negative[WOWUNIT_RESISTANCE_BUFF_MOD_NEGATIVE_COUNT];
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
    uint32_t power_cost_modifier[7];
    float power_cost_multiplier[7];
    float max_health_modifier;
    float hover_height;
    uint32_t max_item_level;
    uint32_t unit_padding;
};
#elif VERSION_STRING == Mop
#define WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT 3
#define WOWUNIT_RESISTANCE_COUNT 7
#define WOWUNIT_RESISTANCE_BUFF_MOD_POSITIVE_COUNT 7
#define WOWUNIT_RESISTANCE_BUFF_MOD_NEGATIVE_COUNT 7
#define WOWUNIT_ATTACK_TIME_COUNT 2
#define WOWUNIT_STAT_COUNT 5

struct WoWUnit : WoWObject
{
    guid_union charm_guid;
    guid_union summon_guid;
    guid_union critter_guid;
    guid_union charmed_by_guid;
    guid_union summoned_by_guid;
    guid_union created_by_guid;
    guid_union demon_creator_guid;;
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
    union
    {
        uint32_t max_power_1;
        uint32_t max_mana;
    };
    uint32_t max_power_2;
    uint32_t max_power_3;
    uint32_t max_power_4;
    uint32_t max_power_5;
    float power_regen_flat_modifier[5];
    float power_regen_interrupted_flat_modifier[5];
    uint32_t level;
    uint32_t effective_level;
    uint32_t faction_template;
    uint32_t virtual_item_slot_display[WOWUNIT_VIRTUAL_ITEM_SLOT_DISPLAY_COUNT];    //0 = melee, 1 = offhand, 2 = ranged
    uint32_t unit_flags;
    uint32_t unit_flags_2;
    uint32_t aura_state;
    uint32_t base_attack_time[WOWUNIT_ATTACK_TIME_COUNT];  //0 = melee, 1 = offhand
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
    uint32_t npc_flags;
    uint32_t dynamic_flags;
    uint32_t npc_emote_state;
    uint32_t stat[WOWUNIT_STAT_COUNT];
    uint32_t positive_stat[WOWUNIT_STAT_COUNT];
    uint32_t negative_stat[WOWUNIT_STAT_COUNT];
    uint32_t resistance[WOWUNIT_RESISTANCE_COUNT];
    uint32_t resistance_buff_mod_positive[WOWUNIT_RESISTANCE_BUFF_MOD_POSITIVE_COUNT];
    uint32_t resistance_buff_mod_negative[WOWUNIT_RESISTANCE_BUFF_MOD_NEGATIVE_COUNT];
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
    uint32_t power_cost_modifier[7];
    float power_cost_multiplier[7];
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
