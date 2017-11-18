/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"
#include "Spell/SpellDefines.hpp"

struct WMOAreaTableTripple
{
    WMOAreaTableTripple(int32_t r, int32_t a, int32_t g) : groupId(g), rootId(r), adtId(a)
    { }

    bool operator <(const WMOAreaTableTripple & b) const
    {
        return memcmp(this, &b, sizeof(WMOAreaTableTripple)) < 0;
    }

    int32_t groupId;
    int32_t rootId;
    int32_t adtId;
};

enum Powers
{
    POWER_MANA                          = 0,
    POWER_RAGE                          = 1,
    POWER_FOCUS                         = 2,
    POWER_ENERGY                        = 3,
    //POWER_HAPPINESS                     = 4,  unused 4.x.x
    POWER_RUNE                          = 5,
    POWER_RUNIC_POWER                   = 6,
    POWER_SOUL_SHARDS                   = 7,
    POWER_ECLIPSE                       = 8,
    POWER_HOLY_POWER                    = 9,
    POWER_ALTERNATIVE                   = 10,
    MAX_POWERS                          = 11,
    POWER_HEALTH                        = 0xFFFFFFFE    // (-2 as signed value)
};

enum Targets
{
    TARGET_NONE                        = 0,
    TARGET_SELF_ONE                     = 1,
    TARGET_RANDOM_ENEMY_CHAIN_IN_AREA  = 2,                 // only one spell has that, but regardless, it's a target type after all
    TARGET_RANDOM_FRIEND_CHAIN_IN_AREA = 3,
    TARGET_RANDOM_UNIT_CHAIN_IN_AREA   = 4,                 // some plague spells that are infectious - maybe targets not-infected friends inrange
    TARGET_PET                         = 5,
    TARGET_CHAIN_DAMAGE                = 6,
    TARGET_AREAEFFECT_INSTANT          = 7,                 // targets around provided destination point
    TARGET_AREAEFFECT_CUSTOM           = 8,
    TARGET_INNKEEPER_COORDINATES       = 9,                 // uses in teleport to innkeeper spells
    TARGET_11                          = 11,                // used by spell 4 'Word of Recall Other'
    TARGET_ALL_ENEMY_IN_AREA           = 15,
    TARGET_ALL_ENEMY_IN_AREA_INSTANT   = 16,
    TARGET_TABLE_X_Y_Z_COORDINATES     = 17,                // uses in teleport spells and some other
    TARGET_EFFECT_SELECT               = 18,                // highly depends on the spell effect
    TARGET_ALL_PARTY_AROUND_CASTER     = 20,
    TARGET_SINGLE_FRIEND               = 21,
    TARGET_CASTER_COORDINATES          = 22,                // used only in TargetA, target selection dependent from TargetB
    TARGET_GAMEOBJECT                  = 23,
    TARGET_IN_FRONT_OF_CASTER          = 24,
    TARGET_DUELVSPLAYER                = 25,
    TARGET_GAMEOBJECT_ITEM             = 26,
    TARGET_MASTER                      = 27,
    TARGET_ALL_ENEMY_IN_AREA_CHANNELED = 28,
    TARGET_29                          = 29,
    TARGET_ALL_FRIENDLY_UNITS_AROUND_CASTER = 30,           // select friendly for caster object faction (in different original caster faction) in TargetB used only with TARGET_ALL_AROUND_CASTER and in self casting range in TargetA
    TARGET_ALL_FRIENDLY_UNITS_IN_AREA  = 31,
    TARGET_MINION                      = 32,
    TARGET_ALL_PARTY                   = 33,
    TARGET_ALL_PARTY_AROUND_CASTER_2   = 34,                // used in Tranquility
    TARGET_SINGLE_PARTY                = 35,
    TARGET_ALL_HOSTILE_UNITS_AROUND_CASTER = 36,
    TARGET_AREAEFFECT_PARTY            = 37,
    TARGET_SCRIPT                      = 38,
    TARGET_SELF_FISHING                = 39,
    TARGET_FOCUS_OR_SCRIPTED_GAMEOBJECT = 40,
    TARGET_TOTEM_EARTH                 = 41,
    TARGET_TOTEM_WATER                 = 42,
    TARGET_TOTEM_AIR                   = 43,
    TARGET_TOTEM_FIRE                  = 44,
    TARGET_CHAIN_HEAL                  = 45,
    TARGET_SCRIPT_COORDINATES          = 46,
    TARGET_DYNAMIC_OBJECT_FRONT        = 47,
    TARGET_DYNAMIC_OBJECT_BEHIND       = 48,
    TARGET_DYNAMIC_OBJECT_LEFT_SIDE    = 49,
    TARGET_DYNAMIC_OBJECT_RIGHT_SIDE   = 50,
    TARGET_AREAEFFECT_GO_AROUND_SOURCE = 51,
    TARGET_AREAEFFECT_GO_AROUND_DEST   = 52,                // gameobject around destination, select by spell_script_target
    TARGET_CURRENT_ENEMY_COORDINATES   = 53,                // set unit coordinates as dest, only 16 target B imlemented
    TARGET_LARGE_FRONTAL_CONE          = 54,
    TARGET_ALL_RAID_AROUND_CASTER      = 56,
    TARGET_SINGLE_FRIEND_2             = 57,
    TARGET_58                          = 58,
    TARGET_FRIENDLY_FRONTAL_CONE       = 59,
    TARGET_NARROW_FRONTAL_CONE         = 60,
    TARGET_AREAEFFECT_PARTY_AND_CLASS  = 61,
    TARGET_DUELVSPLAYER_COORDINATES    = 63,
    TARGET_INFRONT_OF_VICTIM           = 64,
    TARGET_BEHIND_VICTIM               = 65,                // used in teleport behind spells, caster/target dependent from spell effect
    TARGET_RIGHT_FROM_VICTIM           = 66,
    TARGET_LEFT_FROM_VICTIM            = 67,
    TARGET_68                          = 68,
    TARGET_69                          = 69,
    TARGET_70                          = 70,
    TARGET_RANDOM_NEARBY_LOC           = 72,                // used in teleport onto nearby locations
    TARGET_RANDOM_CIRCUMFERENCE_POINT  = 73,
    TARGET_74                          = 74,
    TARGET_75                          = 75,
    TARGET_DYNAMIC_OBJECT_COORDINATES  = 76,
    TARGET_SINGLE_ENEMY                = 77,
    TARGET_POINT_AT_NORTH              = 78,                // 78-85 possible _COORDINATES at radius with pi/4 step around target in unknown order, N?
    TARGET_POINT_AT_SOUTH              = 79,                // S?
    TARGET_POINT_AT_EAST               = 80,                // 80/81 must be symmetric from line caster->target, E (base at 82/83, 84/85 order) ?
    TARGET_POINT_AT_WEST               = 81,                // 80/81 must be symmetric from line caster->target, W (base at 82/83, 84/85 order) ?
    TARGET_POINT_AT_NE                 = 82,                // from spell desc: "(NE)"
    TARGET_POINT_AT_NW                 = 83,                // from spell desc: "(NW)"
    TARGET_POINT_AT_SE                 = 84,                // from spell desc: "(SE)"
    TARGET_POINT_AT_SW                 = 85,                // from spell desc: "(SW)"
    TARGET_RANDOM_NEARBY_DEST          = 86,                // "Test Nearby Dest Random" - random around selected destination
    TARGET_SELF2                       = 87,
    TARGET_88                          = 88,                // Smoke Flare(s) and Hurricane
    TARGET_DIRECTLY_FORWARD            = 89,
    TARGET_NONCOMBAT_PET               = 90,
    TARGET_91                          = 91,
    TARGET_SUMMONER                    = 92,
    TARGET_CONTROLLED_VEHICLE          = 94,
    TARGET_VEHICLE_DRIVER              = 95,
    TARGET_VEHICLE_PASSENGER_0         = 96,
    TARGET_VEHICLE_PASSENGER_1         = 97,
    TARGET_VEHICLE_PASSENGER_2         = 98,
    TARGET_VEHICLE_PASSENGER_3         = 99,
    TARGET_VEHICLE_PASSENGER_4         = 100,
    TARGET_VEHICLE_PASSENGER_5         = 101,
    TARGET_VEHICLE_PASSENGER_6         = 102,
    TARGET_VEHICLE_PASSENGER_7         = 103,
    TARGET_IN_FRONT_OF_CASTER_30       = 104,
    TARGET_105                         = 105,
    TARGET_106                         = 106,
    TARGET_107                         = 107,               // possible TARGET_WMO(GO?)_IN_FRONT_OF_CASTER(_30 ?) TODO: Verify the angle!
    TARGET_GO_IN_FRONT_OF_CASTER_90    = 108,
    TARGET_109                         = 109,               // spell 89008
    TARGET_NARROW_FRONTAL_CONE_2       = 110,
    TARGET_111                         = 111,               // not used
    TARGET_112                         = 112,               // spell 89549
    TARGET_113                         = 113,               // not used
    TARGET_114                         = 114,               // not used
    TARGET_115                         = 115,               // not used
    TARGET_116                         = 116,               // not used
    TARGET_117                         = 117,               // test spell 83658
    TARGET_118                         = 118,               // test spell 79579
    TARGET_119                         = 119,               // mass ressurection 83968
    TARGET_120                         = 120,
    TARGET_121                         = 121,               // spell 95750
    TARGET_122                         = 122,               // spell 100661
    TARGET_123                         = 123,
    TARGET_124                         = 124,
    TARGET_125                         = 125,
    TARGET_126                         = 126,
    TARGET_127                         = 127,
};

enum SpellEffectIndex
{
    EFFECT_INDEX_0 = 0,
    EFFECT_INDEX_1 = 1,
    EFFECT_INDEX_2 = 2
};

enum SpellFamily
{
    SPELLFAMILY_GENERIC     = 0,
    SPELLFAMILY_UNK1        = 1,    // events, holidays
    // 2 - unused
    SPELLFAMILY_MAGE        = 3,
    SPELLFAMILY_WARRIOR     = 4,
    SPELLFAMILY_WARLOCK     = 5,
    SPELLFAMILY_PRIEST      = 6,
    SPELLFAMILY_DRUID       = 7,
    SPELLFAMILY_ROGUE       = 8,
    SPELLFAMILY_HUNTER      = 9,
    SPELLFAMILY_PALADIN     = 10,
    SPELLFAMILY_SHAMAN      = 11,
    SPELLFAMILY_UNK2        = 12,   // 2 spells (silence resistance)
    SPELLFAMILY_POTION      = 13,
    // 14 - unused
    SPELLFAMILY_DEATHKNIGHT = 15,
    // 16 - unused
    SPELLFAMILY_PET         = 17
};


#define MAX_DUNGEON_DIFFICULTY     2
#define MAX_RAID_DIFFICULTY        4
#define MAX_DIFFICULTY             4

namespace DBC
{
    namespace Structures
    {
        namespace
        {
            char const achievement_format[] = "niiissiiiiisii";
            char const achievement_criteria_format[] = "niiiiiiiixsiiiiixxxxxxx";
            char const area_group_format[] = "niiiiiii";
            char const area_table_entry_format[] = "iiinixxxxxisiiiiixxxxxxxxx";
            char const area_trigger_entry_format[] = "nifffxxxfffff";
            //char const armor_location_format[] = "nfffff"; new
            char const auction_house_format[] = "niiix";
            char const bank_bag_slot_prices_format[] = "ni";
            char const barber_shop_style_entry_format[] = "nixxxiii";
            char const banned_addons_entry_format[] = "nxxxxxxxxxx";
            //char const battlemaster_list_format[]="niiiiiiiiixsiiiiiiii"; new
            //char const char_start_outfit_format[]="diiiiiiiiiiiiiiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; new
            char const char_titles_format[] = "nxsxix";
            char const chat_channels_format[] = "iixsx";
            char const chr_classes_format[] = "nixsxxxixiiiii";
            char const chr_races_format[] = "nxixiixixxxxixsxxxxxixxx";
            //char const chr_classes_xpower_types_format[]="nii"; new
            //char const cinematic_sequences_format[]="nxxxxxxxxx"; new
            char const creature_display_info_format[]="nixifxxxxxxxxxxxx";
            char const creature_display_info_extra_format[]="nixxxxxxxxxxxxxxxxxxx";
            char const creature_family_format[] = "nfifiiiiixsx";
            //char const creature_model_data_format[] = "nxxxxxxxxxxxxxxffxxxxxxxxxxxxxx"; new
            char const creature_spell_data_format[] = "niiiiiiii";  //niiiixxxx
            //char const creature_type_format[]="nxx"; new
            char const currency_types_format[] = "nisxxxxiiix";
            //char const destructible_model_data_format[] = "nixxxixxxxixxxxixxxxixxx"; new
            //char const dungeon_encounter_format[]="niiiisxx"; new
            char const durability_costs_format[] = "niiiiiiiiiiiiiiiiiiiiiiiiiiiii";
            char const durability_quality_format[] = "nf";
            char const emotes_entry_format[] = "nxxiiixx";
            char const emotes_text_format[] = "nxixxxxxxxxxxxxxxxx";
            char const faction_format[] = "niiiiiiiiiiiiiiiiiiffixsxx";
            char const faction_template_format[] = "niiiiiiiiiiiii";
            char const game_object_display_info_format[] = "nsxxxxxxxxxxffffffxxx";
            char const gem_properties_format[] = "nixxix";
            char const glyph_properties_format[] = "niii";
            char const glyph_slot_format[] = "nii";
            char const gt_barber_shop_cost_format[] = "xf";
            char const gt_chance_to_melee_crit_format[] = "xf";
            char const gt_chance_to_melee_crit_base_format[] = "xf";
            char const gt_chance_to_spell_crit_format[] = "xf";
            char const gt_chance_to_spell_crit_base_format[] = "xf";
            char const gt_combat_ratings_format[] = "xf";
            //char const gt_oct_base_hp_by_class_format[] = "df"; new
            //char const gt_oct_base_mp_by_class_format[] = "df"; new
            char const gt_oct_class_combat_rating_scalar_format[] = "df";
            //char const gt_oct_hp_per_stamina_format[] = "df"; new
            //char const gt_oct_regen_hp_format[] = "xf";
            char const gt_oct_regen_mp_format[] = "df";
            char const gt_regen_hp_per_spt_format[] = "xf";
            char const gt_regen_mp_per_spt_format[] = "xf";
            //char const gt_spell_scaling_format[] = "df"; new
            char const guild_perk_spells_format[] = "xii";
            char const holidays_format[] = "nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
            //char const item_armor_quality_format[] = "nfffffffi"; new
            //char const item_armor_shield_format[] = "nifffffff"; new
            //char const item_armor_total_format[] = "niffff"; new
            //char const item_bag_family_format[] = "nx"; new
            //char const item_class_format[] = "nixxxs"; new
            //char const item_damage_format[] = "nfffffffi"; new
            char const item_random_properties_format[] = "nxiiiiis";
            char const item_random_suffix_format[] = "nsxiiiiiiiiii";
            char const item_set_format[] = "dsxxxxxxxxxxxxxxxxxiiiiiiiiiiiiiiiiii";
            char const item_limit_category_format[] = "nxii";
            char const lfg_dungeon_entry_format[] = "nsiiiiiiiiiisiiisiiii";
            char const liquid_type_entry_format[] = "nxxixixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
            char const lock_format[] = "niiiiiiiiiiiiiiiiiiiiiiiixxxxxxxx";
            char const mail_template_format[] = "nss";  //nxs
            char const map_format[] = "nsiiiisissififfiiiii";
            //char const map_difficulty_entry_format[] = "niisiis"; new
            //char const mount_capability_format[] = "niiiiiii"; new
            //char const mount_type_format[] = "niiiiiiiiiiiiiiiiiiiiiiii"; new
            //char const movie_entry_format[] = "nxxx"; new
            char const name_gen_format[] = "nsii";
            //char const num_talents_at_level_format[] = "df"; new
            //char const override_spell_data_format[] = "niiiiiiiiiixx"; new
            char const phase_entry_format[] = "nii";
            //char const power_display_format[] = "nixxxx"; new
            //char const pvp_difficulty_format[] = "diiiii"; new
            //char const quest_faction_reward_format[] = "niiiiiiiiii"; new
            char const quest_sort_entry_format[] = "nx";
            char const quest_xp_format[] = "niiiiiiiiii";
            //char const random_properties_points_format[] = "niiiiiiiiiiiiiii"; new
            char const scaling_stat_distribution_format[] = "niiiiiiiiiiiiiiiiiiiixi";
            char const scaling_stat_values_format[] = "iniiiiiiiiiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxx";
            char const skill_line_format[] = "nisxixi";
            char const skill_line_ability_format[] = "niiiixxiiiiiix";
            //char const sound_entries_format[] = "nissssssssssssssssssssssxxxxxxxxxxx"; new
            char const spell_aura_options_format[] = "diiii";
            char const spell_aura_restrictions_format[] = "diiiiiiii";
            char const spell_cast_times_format[] = "niii";
            char const spell_casting_requirements_format[] = "dixxixi";
            char const spell_categories_format[] = "diiiiii";
            char const spell_class_options_format[] = "dxiiiix";
            char const spell_cooldowns_format[] = "diii";
            char const spell_difficulty_format[] = "niiii";
            char const spell_duration_format[] = "niii";
            char const spell_entry_format[] = "niiiiiiiiiiiiiiifiiiissssiixxixiiiiiiixiiiiiiiix";
            char const spell_item_enchantment_format[] = "nxiiiiiixxxiiisiiiiiiix";
            //char const skill_race_class_info_format[] = "diiiiixxx"; new
            char const spell_radius_format[] = "nfff";
            char const spell_range_format[] = "nffffixx";
            char const spell_rune_cost_format[] = "niiii";
            char const spell_shapeshift_form_format[] = "nxxiixiiixxiiiiiiiixx";
            char const spell_effect_format[] = "difiiiffiiiiiifiifiiiiiiiix";
            char const spell_equipped_items_format[] = "diii";
            //char const spell_focus_object_format[] = "nx"; new
            char const spell_interrupts_format[] = "dixixi";
            //char const spell_item_enchantment_condition_format[] = "nbbbbbxxxxxbbbbbbbbbbiiiiixxxxx"; new
            char const spell_levels_format[] = "diii";
            char const spell_power_format[] = "diiiiixf";
            char const spell_reagents_format[] = "diiiiiiiiiiiiiiii";
            char const spell_scaling_format[] = "diiiiffffffffffi";
            char const spell_shapeshift_format[] = "dixixx";
            char const spell_target_restrictions_format[] = "dfiiii";
            char const spell_totems_format[] = "diiii";
            //char const stable_slot_prices_format[] = "ni"; NA
            char const summon_properties_format[] = "niiiii";
            char const talent_format[] = "niiiiiiiiixxixxxxxx";
            char const talent_tab_format[] = "nxxiiixxiii";
            char const talent_tree_primary_spells_format[] = "iiix";
            char const taxi_nodes_format[] = "nifffsiixxx";
            char const taxi_path_format[] = "niii";
            char const taxi_path_node_format[] = "diiifffiiii";
            //char const totem_category_entry_format[] = "nxii"; new
            //char const transport_animation_entry_format[] = "diixxxx"; new
            char const vehicle_format[] = "niffffiiiiiiiifffffffffffffffssssfifiixx";
            char const vehicle_seat_format[] = "niiffffffffffiiiiiifffffffiiifffiiiiiiiffiiiiixxxxxxxxxxxxxxxxxxxx";
            char const wmo_area_table_format[] = "niiixxxxxiixxxx";
            //char const world_map_area_entry_format[] = "xinxffffixxxxx"; new
            char const world_map_overlay_format[] = "nxiiiixxxxxxxxx";
            //char const world_pvp_area_enrty_format[] = "niiiiii"; new
            //char const world_safe_locs_entry_format[] = "nifffx"; new
        }

        #pragma pack(push, 1)
        struct AchievementCategoryEntry
        {
            uint32_t ID;                 // 0
            uint32_t parentCategory;     // 1 -1 for main category
            const char* name;          // 2-17
            uint32_t name_flags;         // 18
            uint32_t sortOrder;          // 19
        };

        struct AchievementCriteriaEntry
        {
            uint32_t ID;                      // 0
            uint32_t referredAchievement;     // 1
            uint32_t requiredType;            // 2
            union
            {
                // ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE = 0
                ///\todo also used for player deaths..
                struct
                {
                    uint32_t creatureID;                             // 3
                    uint32_t creatureCount;                          // 4
                } kill_creature;

                // ACHIEVEMENT_CRITERIA_TYPE_WIN_BG = 1
                ///\todo there are further criterias instead just winning
                struct
                {
                    uint32_t bgMapID;                                // 3
                    uint32_t winCount;                               // 4
                } win_bg;

                // ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL = 5
                struct
                {
                    uint32_t unused;                                 // 3
                    uint32_t level;                                  // 4
                } reach_level;

                // ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL = 7
                struct
                {
                    uint32_t skillID;                                // 3
                    uint32_t skillLevel;                             // 4
                } reach_skill_level;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT = 8
                struct
                {
                    uint32_t linkedAchievement;                      // 3
                } complete_achievement;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT = 9
                struct
                {
                    uint32_t unused;                                 // 3
                    uint32_t totalQuestCount;                        // 4
                } complete_quest_count;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY = 10
                struct
                {
                    uint32_t unused;                                 // 3
                    uint32_t numberOfDays;                           // 4
                } complete_daily_quest_daily;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE = 11
                struct
                {
                    uint32_t zoneID;                                 // 3
                    uint32_t questCount;                             // 4
                } complete_quests_in_zone;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST = 14
                struct
                {
                    uint32_t unused;                                 // 3
                    uint32_t questCount;                             // 4
                } complete_daily_quest;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND= 15
                struct
                {
                    uint32_t mapID;                                  // 3
                } complete_battleground;

                // ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP= 16
                struct
                {
                    uint32_t mapID;                                  // 3
                } death_at_map;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_RAID = 19
                struct
                {
                    uint32_t groupSize;                              // 3 can be 5, 10 or 25
                } complete_raid;

                // ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE = 20
                struct
                {
                    uint32_t creatureEntry;                          // 3
                } killed_by_creature;

                // ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING = 24
                struct
                {
                    uint32_t unused;                                 // 3
                    uint32_t fallHeight;                             // 4
                } fall_without_dying;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST = 27
                struct
                {
                    uint32_t questID;                                // 3
                    uint32_t questCount;                             // 4
                } complete_quest;

                // ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET = 28
                // ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2= 69
                struct
                {
                    uint32_t spellID;                                // 3
                    uint32_t spellCount;                             // 4
                } be_spell_target;

                // ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL= 29
                struct
                {
                    uint32_t spellID;                                // 3
                    uint32_t castCount;                              // 4
                } cast_spell;

                // ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA = 31
                struct
                {
                    uint32_t areaID;                                 // 3 Reference to AreaTable.dbc
                    uint32_t killCount;                              // 4
                } honorable_kill_at_area;

                // ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA = 32
                struct
                {
                    uint32_t mapID;                                  // 3 Reference to Map.dbc
                } win_arena;

                // ACHIEVEMENT_CRITERIA_TYPE_PLAY_ARENA = 33
                struct
                {
                    uint32_t mapID;                                  // 3 Reference to Map.dbc
                } play_arena;

                // ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL = 34
                struct
                {
                    uint32_t spellID;                                // 3 Reference to Map.dbc
                } learn_spell;

                // ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM = 36
                struct
                {
                    uint32_t itemID;                                 // 3
                    uint32_t itemCount;                              // 4
                } own_item;

                // ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA = 37
                struct
                {
                    uint32_t unused;                                 // 3
                    uint32_t count;                                  // 4
                    uint32_t flag;                                   // 5 4=in a row
                } win_rated_arena;

                // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_TEAM_RATING = 38
                struct
                {
                    uint32_t teamtype;                               // 3 {2,3,5}
                } highest_team_rating;

                // ACHIEVEMENT_CRITERIA_TYPE_REACH_TEAM_RATING = 39
                struct
                {
                    uint32_t teamtype;                               // 3 {2,3,5}
                    uint32_t teamrating;                             // 4
                } reach_team_rating;

                // ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL = 40
                struct
                {
                    uint32_t skillID;                                // 3
                    uint32_t skillLevel;                             // 4 apprentice=1, journeyman=2, expert=3, artisan=4, master=5, grand master=6
                } learn_skill_level;

                // ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM = 41
                struct
                {
                    uint32_t itemID;                                 // 3
                    uint32_t itemCount;                              // 4
                } use_item;

                // ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM = 42
                struct
                {
                    uint32_t itemID;                                 // 3
                    uint32_t itemCount;                              // 4
                } loot_item;

                // ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA = 43
                struct
                {
                    uint32_t areaReference;                          // 3 - this is an index to WorldMapOverlay
                } explore_area;

                // ACHIEVEMENT_CRITERIA_TYPE_OWN_RANK= 44
                struct
                {
                    ///\todo This rank is _NOT_ the index from CharTitles.dbc
                    uint32_t rank;                                   // 3
                } own_rank;

                // ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT= 45
                struct
                {
                    uint32_t unused;                                 // 3
                    uint32_t numberOfSlots;                          // 4
                } buy_bank_slot;

                // ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION= 46
                struct
                {
                    uint32_t factionID;                              // 3
                    uint32_t reputationAmount;                       // 4 Total reputation amount, so 42000 = exalted
                } gain_reputation;

                // ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION= 47
                struct
                {
                    uint32_t unused;                                 // 3
                    uint32_t numberOfExaltedFactions;                // 4
                } gain_exalted_reputation;

                // ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM = 49
                ///\todo where is the required itemlevel stored?
                struct
                {
                    uint32_t itemSlot;                               // 3
                } equip_epic_item;

                // ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT= 50
                struct
                {
                    uint32_t rollValue;                              // 3
                    uint32_t count;                                  // 4
                } roll_need_on_loot;

                // ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS = 52
                struct
                {
                    uint32_t classID;                                // 3
                    uint32_t count;                                  // 4
                } hk_class;

                // ACHIEVEMENT_CRITERIA_TYPE_HK_RACE = 53
                struct
                {
                    uint32_t raceID;                                 // 3
                    uint32_t count;                                  // 4
                } hk_race;

                // ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE = 54
                ///\todo where is the information about the target stored?
                struct
                {
                    uint32_t emoteID;                                // 3
                } do_emote;

                // ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE = 55
                struct
                {
                    uint32_t unused;                                 // 3
                    uint32_t count;                                  // 4
                    uint32_t flag;                                   // 5 =3 for battleground healing
                    uint32_t mapid;                                  // 6
                } healing_done;

                // ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM = 57
                struct
                {
                    uint32_t itemID;                                 // 3
                } equip_item;

                // ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD = 62
                struct
                {
                    uint32_t unknown;                                 // 3
                    uint32_t goldInCopper;                            // 4
                } quest_reward_money;

                // ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY = 67
                struct
                {
                    uint32_t unused;                                 // 3
                    uint32_t goldInCopper;                           // 4
                } loot_money;

                // ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT = 68
                struct
                {
                    uint32_t goEntry;                                // 3
                    uint32_t useCount;                               // 4
                } use_gameobject;

                // ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL= 70
                ///\todo are those special criteria stored in the dbc or do we have to add another sql table?
                struct
                {
                    uint32_t unused;                                 // 3
                    uint32_t killCount;                              // 4
                } special_pvp_kill;

                // ACHIEVEMENT_CRITERIA_TYPE_FISH_IN_GAMEOBJECT = 72
                struct
                {
                    uint32_t goEntry;                                // 3
                    uint32_t lootCount;                              // 4
                } fish_in_gameobject;

                // ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS= 75
                struct
                {
                    uint32_t unknown;                                // 3 777=?
                    uint32_t mountCount;                             // 4
                } number_of_mounts;

                // ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL = 76
                struct
                {
                    uint32_t unused;                                 // 3
                    uint32_t duelCount;                              // 4
                } win_duel;

                // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_POWER = 96
                struct
                {
                    uint32_t powerType;                              // 3 mana= 0, 1=rage, 3=energy, 6=runic power
                } highest_power;

                // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_STAT = 97
                struct
                {
                    uint32_t statType;                               // 3 4=spirit, 3=int, 2=stamina, 1=agi, 0=strength
                } highest_stat;

                // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_SPELLPOWER = 98
                struct
                {
                    uint32_t spellSchool;                            // 3
                } highest_spellpower;

                // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_RATING = 100
                struct
                {
                    uint32_t ratingType;                             // 3
                } highest_rating;

                // ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE = 109
                struct
                {
                    uint32_t lootType;                               // 3 3=fishing, 2=pickpocket, 4=disentchant
                    uint32_t lootTypeCount;                          // 4
                } loot_type;

                // ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2 = 110
                struct
                {
                    uint32_t skillLine;                              // 3
                    uint32_t spellCount;                             // 4
                } cast_spell2;

                // ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE= 112
                struct
                {
                    uint32_t skillLine;                              // 3
                    uint32_t spellCount;                             // 4
                } learn_skill_line;

                // ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL = 113
                struct
                {
                    uint32_t unused;                                 // 3
                    uint32_t killCount;                              // 4
                } honorable_kill;

                struct
                {
                    uint32_t field3;                                 // 3 main requirement
                    uint32_t field4;                                 // 4 main requirement count
                    uint32_t additionalRequirement1_type;            // 5 additional requirement 1 type
                    uint32_t additionalRequirement1_value;           // 6 additional requirement 1 value
                    uint32_t additionalRequirement2_type;            // 7 additional requirement 2 type
                    uint32_t additionalRequirement2_value;           // 8 additional requirement 1 value
                } raw;
            };
            //uint32_t unk                      // 9
            char* name;                         // 10
            uint32_t completionFlag;            // 11
            uint32_t groupFlag;                 // 12 timed criteria
            uint32_t unk1;                      // 13 timed criteria misc id
            uint32_t timeLimit;                 // 14 time limit in seconds
            uint32_t index;                     // 15 order
        };

        struct AchievementEntry
        {
            uint32_t ID;                        // 0
            int32_t factionFlag;                // 1 -1=all, 0=horde, 1=alliance
            int32_t mapID;                      // 2 -1=none
            uint32_t parentAchievement;         // 3
            char* name;                         // 4
            char* description;                  // 5
            uint32_t categoryId;                // 6
            uint32_t points;                    // 7 reward points
            uint32_t orderInCategory;           // 8
            uint32_t flags;                     // 9
            uint32_t icon;                      // 10
            char* rewardName;                   // 11 title/item reward name
            uint32_t count;                     // 12
            uint32_t refAchievement;            // 13
        };

        struct AreaGroupEntry
        {
            uint32_t AreaGroupId;               // 0
            uint32_t AreaId[6];                 // 1-6
            uint32_t next_group;                // 7
        };

        struct AreaTableEntry
        {
            uint32_t id;                        // 0
            uint32_t map_id;                    // 1
            uint32_t zone;                      // 2 if 0 then it's zone, else it's zone id of this area
            uint32_t explore_flag;              // 3, main index
            uint32_t flags;                     // 4, unknown value but 312 for all cities
                                                // 5-9 unused
            int32_t area_level;                 // 10
            char* area_name;                    // 11
            uint32_t team;                      // 12
            uint32_t liquid_type_override[4];   // 13-16 liquid override by type
            //uint32_t unk17;                   // 17
            //uint32_t unk18;                   // 18
            //uint32_t unk19;                   // 19
            //uint32_t unk20;                   // 20
            //uint32_t unk21;                   // 21
            //uint32_t unk22;                   // 22
            //uint32_t unk23;                   // 23
            //uint32_t unk24;                   // 24
        };

        struct AreaTriggerEntry
        {
            uint32_t id;                        // 0
            uint32_t mapid;                     // 1
            float x;                            // 2
            float y;                            // 3
            float z;                            // 4
            //uint32_t                          // 5
            //uint32_t                          // 6
            //uint32_t                          // 7
            float o;                            // 5 radius
            float box_x;                        // 6 extent x edge
            float box_y;                        // 7 extent y edge
            float box_z;                        // 8 extent z edge
            float box_o;                        // 9
        };

        //\todo danko
        //struct ArmorLocationEntry   
        //{
        //    uint32_t InventoryType;           // 0
        //    float Value[5];                   // 1-5
        //};

        struct AuctionHouseEntry
        {
            uint32_t id;                        // 0
            uint32_t faction;                   // 1
            uint32_t fee;                       // 2
            uint32_t tax;                       // 3
            //char* name;                       // 4
        };

        struct BankBagSlotPrices
        {
            uint32_t Id;                        // 0
            uint32_t Price;                     // 1
        };

        struct BarberShopStyleEntry
        {
            uint32_t id;                        // 0
            uint32_t type;                      // 1 value 0 -> hair, value 2 -> facialhair
            //char* name;                       // 2
            //uint32_t unk_name;                // 3
            //float CostMultiplier;             // 4
            uint32_t race;                      // 5
            uint32_t gender;                    // 6 0 male, 1 female
            uint32_t hair_id;                   // 7 Hair ID
        };

        struct BannedAddOnsEntry
        {
            uint32_t Id;                        // 0
            //uint32_t nameMD5[4];              // 1-4
            //uint32_t versionMD5[4];           // 5-8
            //uint32_t timestamp;               // 9
            //uint32_t state;                   // 10
        };

        #define OUTFIT_ITEMS 24

        //\todo danko
        //struct CharStartOutfitEntry
        //{
        //    //uint32_t Id;                                // 0
        //    uint32_t RaceClassGender;                     // 1
        //    int32_t ItemId[OUTFIT_ITEMS];                 // 2-25
        //    //int32_t ItemDisplayId[OUTFIT_ITEMS];        // 26-29
        //    //int32_t ItemInventorySlot[OUTFIT_ITEMS];    // 50-73
        //    //uint32_t Unknown1;                          // 74
        //    //uint32_t Unknown2;                          // 75
        //    //uint32_t Unknown3;                          // 76
        //    //uint32_t Unknown4;                          // 77
        //    //uint32_t Unknown5;                          // 78
        //};

        struct CharTitlesEntry
        {
            uint32_t ID;                        // 0
            //uint32_t unk1;                    // 1
            char* name;                         // 2
            //char* name2;                      // 3
            uint32_t bit_index;                 // 4
            //uint32_t unk                      // 5
        };

        struct ChatChannelsEntry
        {
            uint32_t id;                        // 0
            uint32_t flags;                     // 1
            //uint32_t faction                  // 2
            char* name_pattern;                 // 3
            //char* channel_name;               // 4
        };

        struct ChrClassesEntry
        {
            uint32_t class_id;                  // 0
            uint32_t power_type;                // 1
            //uint32_t unk2;                    // 2
            char* name;                         // 3
            //char* name_female;                // 4
            //char* name_neutral;               // 5
            //char* name_capitalized            // 6
            uint32_t spellfamily;               // 7
            //uint32_t unk4;                    // 8
            uint32_t cinematic_sequence;        // 9 CinematicSequences.dbc
            uint32_t expansion;                 // 10
            uint32_t apPerStr;                  // 11
            uint32_t apPerAgi;                  // 12
            uint32_t rapPerAgi;                 // 13
        };

        struct ChrRacesEntry
        {
            uint32_t race_id;                   // 0
            //uint32_t flags;                   // 1
            uint32_t faction_id;                // 2
            //uint32_t unk1;                    // 3
            uint32_t model_male;                // 4
            uint32_t model_female;              // 5
            // uint32_t unk2;                   // 6
            uint32_t team_id;                   // 7
            //uint32_t unk3[4];                 // 8-11
            uint32_t cinematic_id;              // 12 CinematicSequences.dbc
            //uint32_t unk4                     // 13
            char* name;                         // 14
            //char* name_female;                // 15
            //char* name_neutral;               // 16
            //uint32_t unk5[2]                  // 17-18
            //uint32_t unk19                    // 19
            uint32_t expansion;                 // 20
            //uint32_t unk21                    // 21
            //uint32_t unk22                    // 22
            //uint32_t unk23                    // 23
        };

        //\todo danko
        //struct ChrPowerTypesEntry
        //{
        //    uint32_t entry;                   // 0
        //    uint32_t classId;                 // 1
        //    uint32_t power;                   // 2
        //};

        struct CreatureDisplayInfoEntry
        {
            uint32_t Displayid;                 // 0
            uint32_t ModelId;                   // 1
            //uint32_t sound_id;                // 2
            uint32_t ExtendedDisplayInfoID;     // 3
            float scale;                        // 4
            //uint32_t unk01;                   // 5
            //uint32_t unk02[2];                // 6-8
            //uint32_t unk03;                   // 9
            //uint32_t unk04;                   // 10
            //uint32_t unk05;                   // 11
            //uint32_t unk06;                   // 12
            //uint32_t unk07;                   // 13
            //uint32_t unk08;                   // 14
            //uint32_t unk09;                   // 15
            //uint32_t unk10;                   // 16
        };

        struct CreatureDisplayInfoExtraEntry
        {
            uint32_t DisplayExtraId;            // 0
            uint32_t Race;                      // 1
            //uint32_t Gender;                  // 2
            //uint32_t SkinColor;               // 3
            //uint32_t FaceType;                // 4
            //uint32_t HairType;                // 5
            //uint32_t HairStyle;               // 6
            //uint32_t BeardStyle;              // 7
            //uint32_t Equipment[11];           // 8-18
            //uint32_t CanEquip;                // 19
            //char* unk                         // 20
        };

        struct CreatureFamilyEntry
        {
            uint32_t ID;                        // 0
            float minsize;                      // 1
            uint32_t minlevel;                  // 2
            float maxsize;                      // 3
            uint32_t maxlevel;                  // 4
            uint32_t skilline;                  // 5
            uint32_t tameable;                  // 6 second skill line - 270 Generic
            uint32_t petdietflags;              // 7
            uint32_t talenttree;                // 8 (-1 = none, 0 = ferocity(410), 1 = tenacity(409), 2 = cunning(411))
            //uint32_t unk;                     // 9 some index 0 - 63
            char* name;                         // 10
            //uint32_t nameflags;               // 11
        };

        struct CreatureSpellDataEntry
        {
            uint32_t id;                        // 0
            uint32_t Spells[3];
            uint32_t PHSpell;
            uint32_t Cooldowns[3];
            uint32_t PH;
        };

        //\todo danko
        //struct CreatureTypeEntry
        //{
        //    uint32_t ID;                      // 0
        //    //char* Name;                     // 1
        //    //uint32_t no_expirience;         // 2
        //};

        struct CurrencyTypesEntry
        {
            uint32_t item_id;                   // 0
            uint32_t Category;                  // 1
            char* name;                         // 2
            //char* unk                         // 3
            //char* unk2                        // 4
            //uint32_t unk5;                    // 5
            //uint32_t unk6;                    // 6
            uint32_t TotalCap;                  // 7
            uint32_t WeekCap;                   // 8
            uint32_t Flags;                     // 9
            //char* description;                // 10
        };

        struct DurabilityCostsEntry
        {
            uint32_t itemlevel;                 // 0
            uint32_t modifier[29];              // 1-29
        };

        struct DurabilityQualityEntry
        {
            uint32_t id;                        // 0
            float quality_modifier;             // 1
        };

        struct EmotesEntry
        {
            uint32_t Id;                        // 0
            //char* name;                       // 1
            //uint32_t animationId;             // 2
            uint32_t Flags;                     // 3
            uint32_t EmoteType;                 // 4
            uint32_t UnitStandState;            // 5
            //uint32_t soundId;                 // 6
            //uint32_t unk;                     // 7
        };

        struct EmotesTextEntry
        {
            uint32_t Id;                        // 0
            //uint32_t name;                    // 1
            uint32_t textid;                    // 2
            //uint32_t unk1;                    // 4
        };

        struct FactionEntry
        {
            uint32_t ID;                        // 0
            int32_t RepListId;                  // 1
            uint32_t RaceMask[4];               // 2-5
            uint32_t ClassMask[4];              // 6-9
            int32_t baseRepValue[4];            // 10-13
            uint32_t repFlags[4];               // 14-17
            uint32_t parentFaction;             // 18
            float spillover_rate_in;            // 19
            float spillover_rate_out;           // 20
            uint32_t spillover_max_in;          // 21
            //uint32_t unk1;                    // 22
            char* Name;                         // 23
            //uint32_t Description;             // 24
            //uint32_t description_flags;       // 25
        };

        struct FactionTemplateEntry
        {
            uint32_t ID;                        // 0
            uint32_t Faction;                   // 1
            uint32_t FactionGroup;              // 2
            uint32_t Mask;                      // 3
            uint32_t FriendlyMask;              // 4
            uint32_t HostileMask;               // 5
            uint32_t EnemyFactions[4];          // 6-9
            uint32_t FriendlyFactions[4];       // 10-13
        };

        struct GameObjectDisplayInfoEntry
        {
            uint32_t Displayid;                 // 0
            char* filename;                     // 1
            //uint32_t  unk1[10];               // 2-11
            float minX;                         // 12
            float minY;                         // 13
            float minZ;                         // 14
            float maxX;                         // 15
            float maxY;                         // 16
            float maxZ;                         // 17
            //uint32_t transport;               // 18
            //uint32_t unk;                     // 19
            //uint32_t unk;                     // 20
        };

        struct GemPropertiesEntry
        {
            uint32_t Entry;                     // 0
            uint32_t EnchantmentID;             // 1
            //uint32_t unk1;                    // 2 bool
            //uint32_t unk2;                    // 3 bool
            uint32_t SocketMask;                // 4
        };

        struct GlyphPropertiesEntry
        {
            uint32_t Entry;                     // 0
            uint32_t SpellID;                   // 1
            uint32_t Type;                      // 2 (0 = Major, 1 = Minor)
            uint32_t unk;                       // 3 glyph_icon spell.dbc
        };

        struct GlyphSlotEntry
        {
            uint32_t Id;                        // 0
            uint32_t Type;                      // 1
            uint32_t Slot;                      // 2
        };

        struct GtBarberShopCostBaseEntry
        {
            float cost;                         // 0
        };

        struct GtChanceToMeleeCritEntry
        {
            float val;                          // 0
        };

        struct GtChanceToMeleeCritBaseEntry
        {
            float val;                          // 0
        };

        struct GtChanceToSpellCritEntry
        {
            float val;                          // 0
        };

        struct GtChanceToSpellCritBaseEntry
        {
            float val;                          // 0
        };

        struct GtCombatRatingsEntry
        {
            float val;                          // 0
        };

        struct GtOCTClassCombatRatingScalarEntry
        {
            float val;                          // 0
        };

        //\todo danko
        //struct GtOCTRegenHPEntry
        //{
        //    float ratio;                      // 0
        //};

        struct GtOCTRegenMPEntry
        {
            float ratio;                        // 0
        };

        //\todo danko
        //struct GtRegenHPPerSptEntry
        //{
        //    float ratio;                      // 0
        //};

        struct GtRegenMPPerSptEntry
        {
            float ratio;                        // 0
        };

        struct GuildPerkSpellsEntry
        {
            //uint32_t Id;                      // 0
            uint32_t Level;                     // 1
            uint32_t SpellId;                   // 2
        };

#define MAX_HOLIDAY_DURATIONS 10
#define MAX_HOLIDAY_DATES 26
#define MAX_HOLIDAY_FLAGS 10

        struct HolidaysEntry
        {
            uint32_t Id;                                    // 0
            //uint32_t Duration[MAX_HOLIDAY_DURATIONS];     // 1-10
            //uint32_t Date[MAX_HOLIDAY_DATES];             // 11-36
            //uint32_t Region;                              // 37
            //uint32_t Looping;                             // 38
            //uint32_t CalendarFlags[MAX_HOLIDAY_FLAGS];    // 39-48
            //uint32_t holidayNameId;                       // 49 HolidayNames.dbc
            //uint32_t holidayDescriptionId;                // 50 HolidayDescriptions.dbc
            //char* TextureFilename;                        // 51
            //uint32_t Priority;                            // 52
            //uint32_t CalendarFilterType;                  // 53
            //uint32_t flags;                               // 54
        };

        struct ItemLimitCategoryEntry
        {
            uint32_t Id;                        // 0
            //char* name;                       // 1
            uint32_t maxAmount;                 // 2
            uint32_t equippedFlag;              // 3
        };

        struct ItemRandomPropertiesEntry
        {
            uint32_t ID;                        // 0
            //char* name1;                      // 1
            uint32_t spells[5];                 // 2-6 enchant_id
            char* name_suffix;                  // 7
        };

        struct ItemRandomSuffixEntry
        {
            uint32_t id;                        // 0
            char* name_suffix;                  // 1
            //uint32_t name_suffix_flags;       // 2
            uint32_t enchantments[5];           // 3-7
            uint32_t prefixes[5];               // 8-12
        };

        struct ItemSetEntry
        {
            //uint32_t id;                      // 0
            char* name;                         // 1
            //uint32_t itemid[17];              // 2-18
            uint32_t SpellID[8];                // 19-26
            uint32_t itemscount[8];             // 27-34
            uint32_t RequiredSkillID;           // 35
            uint32_t RequiredSkillAmt;          // 36
        };

        struct LFGDungeonEntry
        {
            uint32_t ID;                        // 0
            char* name;                         // 1
            uint32_t minlevel;                  // 2
            uint32_t maxlevel;                  // 3
            uint32_t reclevel;                  // 4
            uint32_t recminlevel;               // 5
            uint32_t recmaxlevel;               // 6
            int32_t map;                        // 7
            uint32_t difficulty;                // 8
            uint32_t unk;                       // 9
            uint32_t flags;                     // 10
            int32_t type;                       // 11
            char* iconname;                     // 12
            uint32_t expansion;                 // 13
            uint32_t unk4;                      // 14
            uint32_t unk5;                      // 15
            char* unk_text;                     // 16
            uint32_t grouptype;                 // 17
            uint32_t unkflags1;                 // 18
            uint32_t unkflags2;                 // 19
            uint32_t unk7;                      // 20

            // Helpers
            uint32_t Entry() const { return ID + (type << 24); }
        };

        struct LiquidTypeEntry
        {
            uint32_t Id;                        // 0
            //char* Name;                       // 1
            //uint32_t Flags;                   // 2
            uint32_t Type;                      // 3
            //uint32_t SoundId;                 // 4
            uint32_t SpellId;                   // 5
            //float MaxDarkenDepth;             // 6
            //float FogDarkenIntensity;         // 7
            //float AmbDarkenIntensity;         // 8
            //float DirDarkenIntensity;         // 9
            //uint32_t LightID;                 // 10
            //float ParticleScale;              // 11
            //uint32_t ParticleMovement;        // 12
            //uint32_t ParticleTexSlots;        // 13
            //uint32_t LiquidMaterialID;        // 14
            //char* Texture[6];                 // 15-20
            //uint32_t Color[2];                // 21-22
            //float Unk1[18];                   // 23-40
            //uint32_t Unk2[4];                 // 41-44
        };

#define LOCK_NUM_CASES 8

        struct LockEntry
        {
            uint32_t Id;                              // 0
            uint32_t locktype[LOCK_NUM_CASES];        // 1-8 If this is 1, then the next lockmisc is an item ID, if it's 2, then it's an iRef to LockTypes.dbc.
            uint32_t lockmisc[LOCK_NUM_CASES];        // 9-16 Item to unlock or iRef to LockTypes.dbc depending on the locktype.
            uint32_t minlockskill[LOCK_NUM_CASES];    // 17-24 Required skill needed for lockmisc (if locktype = 2).
            //uint32_t action[LOCK_NUM_CASES];        // 25-32 Something to do with direction / opening / closing.
        };

        struct MailTemplateEntry
        {
            uint32_t ID;                        // 0
            char* subject;                      // 1
            char* content;                      // 2
        };

        struct MapEntry
        {
            uint32_t id;                        // 0
            char* name_internal;                // 1
            uint32_t map_type;                  // 2
            uint32_t map_flags;                 // 3
            uint32_t unk4;                      // 4
            uint32_t is_pvp_zone;               // 5
            char* map_name;                     // 6
            uint32_t linked_zone;               // 7 common zone for instance and continent map
            char* horde_intro;                  // 8 horde text for PvP Zones
            char* alliance_intro;               // 9 alliance text for PvP Zones
            uint32_t multimap_id;               // 10
            float battlefield_map_icon;         // 11
            int32_t parent_map;                 // 12 ghost map_id of parent map
            float start_x;                      // 13 ghost enter x coordinate (if exist single entry)
            float start_y;                      // 14 ghost enter y coordinate (if exist single entry)
            uint32_t dayTime_override;          // 15
            uint32_t addon;                     // 16 0-original maps, 1-tbc addon, 2-wotlk addon
            uint32_t unk_time;                  // 17
            uint32_t max_players;               // 18
            uint32_t next_phase_map;            // 19
        };

        struct NameGenEntry
        {
            uint32_t ID;                        // 0
            char* Name;                         // 1
            uint32_t unk1;                      // 2
            uint32_t type;                      // 3
        };

        struct PhaseEntry
        {
            uint32_t Id;                        // 0
            uint32_t PhaseShift;                // 1
            uint32_t Flags;                     // 2
        };

        struct QuestSortEntry
        {
            uint32_t id;                        // 0
            //char* name;                       // 1
        };

        struct QuestXP
        {
            uint32_t questLevel;                // 0
            uint32_t xpIndex[10];               // 1-10
        };

        struct ScalingStatDistributionEntry
        {
            uint32_t id;                        // 0
            int32_t stat[10];                   // 1-10
            uint32_t statmodifier[10];          // 11-20
            //uint32_t unk;                     // 21
            uint32_t maxlevel;                  // 22
        };

        struct ScalingStatValuesEntry
        {
            uint32_t id;                        // 0
            uint32_t level;                     // 1
            uint32_t multiplier[20];            // 
            //\todo danko Rewrite GetStatScalingStatValueColumn!
            //uint32_t dpsMod[6];               // 2-7
            //uint32_t spellBonus;              // 8
            //uint32_t scalingstatmultipl[5];   // 9-13
            //uint32_t amor_mod[4];             // 14-17
            //uint32_t amor_mod2[4];            // 18-21
            //uint32_t junk[24];                // 22-45
            //uint32_t unk2;                    // 46
        };

        struct SkillLineEntry
        {
            uint32_t id;                        // 0
            uint32_t type;                      // 1
            char* Name;                         // 2
            //char* Description;                // 3
            uint32_t spell_icon;                // 4
            //char* add_name;                   // 5
            uint32_t linkable;                  // 6
        };

        struct SkillLineAbilityEntry
        {
            uint32_t Id;                        // 0
            uint32_t skilline;                  // 1 skill id
            uint32_t spell;                     // 2
            uint32_t race_mask;                 // 3
            uint32_t class_mask;                // 4
            //uint32_t excludeRace;             // 5
            //uint32_t excludeClass;            // 6
            uint32_t minSkillLineRank;          // 7 req skill value
            uint32_t next;                      // 8
            uint32_t acquireMethod;             // 9 auto learn
            uint32_t grey;                      // 10 max
            uint32_t green;                     // 11 min
            uint32_t characterPoints;           // 12
            //uint32_t unk;                     // 13
        };

        //\todo danko
        //struct StableSlotPrices
        //{
        //    uint32_t Id;                      // 0
        //    uint32_t Price;                   // 1
        //};

        // SpellAuraOptions.dbc
        struct SpellAuraOptionsEntry
        {
            //uint32_t Id;                      // 0
            uint32_t StackAmount;               // 1
            uint32_t procChance;                // 2
            uint32_t procCharges;               // 3
            uint32_t procFlags;                 // 4
        };

        // SpellAuraRestrictions.dbc
        struct SpellAuraRestrictionsEntry
        {
            //uint32_t Id;                      // 0
            uint32_t CasterAuraState;           // 1
            uint32_t TargetAuraState;           // 2
            uint32_t CasterAuraStateNot;        // 3
            uint32_t TargetAuraStateNot;        // 4
            uint32_t casterAuraSpell;           // 5
            uint32_t targetAuraSpell;           // 6
            uint32_t excludeCasterAuraSpell;    // 7
            uint32_t excludeTargetAuraSpell;    // 8
        };

        // SpellCastingRequirements.dbc
        struct SpellCastingRequirementsEntry
        {
            //uint32_t Id;                      // 0
            uint32_t FacingCasterFlags;         // 1
            //uint32_t MinFactionId;            // 2
            //uint32_t MinReputation;           // 3
            int32_t AreaGroupId;                // 4
            //uint32_t RequiredAuraVision;      // 5
            uint32_t RequiresSpellFocus;        // 6
        };

        // SpellCastTimes.dbc
        struct SpellCastTimesEntry
        {
            uint32_t ID;                        // 0
            uint32_t CastTime;                  // 1
            float CastTimePerLevel;             // 2
            int32_t MinCastTime;                // 3
        };

        // SpellCategories.dbc
        struct SpellCategoriesEntry
        {
            //uint32_t Id;                      // 0
            uint32_t Category;                  // 1
            uint32_t DmgClass;                  // 2
            uint32_t Dispel;                    // 3
            uint32_t Mechanic;                  // 4
            uint32_t PreventionType;            // 5
            uint32_t StartRecoveryCategory;     // 6
        };

        struct ClassFamilyMask
        {
            uint64 Flags;
            uint32_t Flags2;

            ClassFamilyMask() : Flags(0), Flags2(0) {}
            explicit ClassFamilyMask(uint64 familyFlags, uint32_t familyFlags2 = 0) : Flags(familyFlags), Flags2(familyFlags2) {}

            bool Empty() const { return Flags == 0 && Flags2 == 0; }
            bool operator! () const { return Empty(); }
            operator void const* () const { return Empty() ? nullptr : this; }

            bool IsFitToFamilyMask(uint64 familyFlags, uint32_t familyFlags2 = 0) const
            {
                return (Flags & familyFlags) || (Flags2 & familyFlags2);
            }

            bool IsFitToFamilyMask(ClassFamilyMask const& mask) const
            {
                return (Flags & mask.Flags) || (Flags2 & mask.Flags2);
            }

            uint64 operator& (uint64 mask) const
            {
                return Flags & mask;
            }

            ClassFamilyMask& operator|= (ClassFamilyMask const& mask)
            {
                Flags |= mask.Flags;
                Flags2 |= mask.Flags2;
                return *this;
            }
        };

        // SpellClassOptions.dbc
        struct SpellClassOptionsEntry
        {
            //uint32_t Id;                      // 0
            //uint32_t modalNextSpell;          // 1
            uint32_t SpellFamilyFlags[3];       // 2-4
            uint32_t SpellFamilyName;           // 5
            //char* Description;                // 6
            
            // helpers
            bool IsFitToFamilyMask(uint64 /*familyFlags*/, uint32_t /*familyFlags2*/ = 0) const
            {
                return true; // SpellFamilyFlags.IsFitToFamilyMask(familyFlags, familyFlags2);
            }

            bool IsFitToFamily(SpellFamily family, uint64 familyFlags, uint32_t familyFlags2 = 0) const
            {
                return SpellFamily(SpellFamilyName) == family && IsFitToFamilyMask(familyFlags, familyFlags2);
            }

            bool IsFitToFamilyMask(ClassFamilyMask const& /*mask*/) const
            {
                return true;// SpellFamilyFlags.IsFitToFamilyMask(mask);
            }

            bool IsFitToFamily(SpellFamily family, ClassFamilyMask const& mask) const
            {
                return SpellFamily(SpellFamilyName) == family && IsFitToFamilyMask(mask);
            }

        private:
            // catch wrong uses
            template<typename T>
            bool IsFitToFamilyMask(SpellFamily family, T t) const;
        };

        // SpellCooldowns.dbc
        struct SpellCooldownsEntry
        {
            //uint32_t Id;                      // 0
            uint32_t CategoryRecoveryTime;      // 1
            uint32_t RecoveryTime;              // 2
            uint32_t StartRecoveryTime;         // 3
        };

        // SpellDifficulty.dbc
        struct SpellDifficultyEntry
        {
            uint32_t ID;                        // 0
            int32_t SpellId[MAX_DIFFICULTY];    // 1-4
        };

        // SpellDuration.dbc
        struct SpellDurationEntry
        {
            uint32_t ID;                        // 0
            uint32_t Duration1;                 // 1
            uint32_t Duration2;                 // 2
            uint32_t Duration3;                 // 3
        };

        // SpellEffect.dbc
        struct SpellEffectEntry
        {
            //uint32_t Id;                      // 0
            uint32_t Effect;                    // 1
            float EffectMultipleValue;          // 2
            uint32_t EffectApplyAuraName;       // 3
            uint32_t EffectAmplitude;           // 4
            int32_t EffectBasePoints;           // 5
            float EffectBonusMultiplier;        // 6
            float EffectDamageMultiplier;       // 7
            uint32_t EffectChainTarget;         // 8
            int32_t EffectDieSides;             // 9
            uint32_t EffectItemType;            // 10
            uint32_t EffectMechanic;            // 11
            int32_t EffectMiscValue;            // 12
            int32_t EffectMiscValueB;           // 13
            float EffectPointsPerComboPoint;    // 14
            uint32_t EffectRadiusIndex;         // 15
            uint32_t EffectRadiusMaxIndex;      // 16
            float EffectRealPointsPerLevel;     // 17
            uint32_t EffectSpellClassMask[3];   // 18-20
            //ClassFamilyMask EffectSpellClassMask;
            uint32_t EffectTriggerSpell;        // 21
            uint32_t EffectImplicitTargetA;     // 22
            uint32_t EffectImplicitTargetB;     // 23
            uint32_t EffectSpellId;             // 24
            uint32_t EffectIndex;               // 25
            //uint32_t unk;                     // 26

            // helpers
            int32_t CalculateSimpleValue() const { return EffectBasePoints; }

            uint32_t GetRadiusIndex() const
            {
                if (EffectRadiusIndex != 0)
                    return EffectRadiusIndex;

                return EffectRadiusMaxIndex;
            }
        };

        // SpellEquippedItems.dbc
        struct SpellEquippedItemsEntry
        {
            //uint32_t Id;                          // 0
            int32_t EquippedItemClass;              // 1
            int32_t EquippedItemInventoryTypeMask;  // 2
            int32_t EquippedItemSubClassMask;       // 3
        };

        // SpellFocusObject.dbc
        struct SpellFocusObjectEntry
        {
            uint32_t ID;                        // 0
            //char* Name;                       // 1
        };

        // SpellInterrupts.dbc
        struct SpellInterruptsEntry
        {
            //uint32_t Id;                      // 0
            uint32_t AuraInterruptFlags;        // 1
            //uint32_t unk2                     // 2
            uint32_t ChannelInterruptFlags;     // 3
            //uint32_t unk4                     // 4
            uint32_t InterruptFlags;            // 5
        };

        // SpellItemEnchantment.dbc
        struct SpellItemEnchantmentEntry
        {
            uint32_t Id;                        // 0
            //uint32_t charges;                 // 1
            uint32_t type[3];                   // 2-4
            uint32_t min[3];                    // 5-7 for combat, in practice min==max
            //uint32_t max[3];                  // 8-10
            uint32_t spell[3];                  // 11-13
            char* Name;                         // 14-29
            //uint32_t NameFlags;               // 30
            uint32_t visual;                    // 31 aura
            uint32_t EnchantGroups;             // 32 slot
            uint32_t GemEntry;                  // 33
            uint32_t ench_condition;            // 34
            uint32_t req_skill;                 // 35
            uint32_t req_skill_value;           // 36
            uint32_t req_level;                 // 37
        };

        // SpellItemEnchantmentCondition.dbc
        struct SpellItemEnchantmentConditionEntry
        {
            uint32_t ID;                        // 0
            uint8_t Color[5];                   // 1-5
            //uint32_t LT_Operand[5];           // 6-10
            uint8_t Comparator[5];              // 11-15
            uint8_t CompareColor[5];            // 15-20
            uint32_t Value[5];                  // 21-25
            //uint8_t Logic[5]                  // 25-30
        };

        // SpellLevels.dbc
        struct SpellLevelsEntry
        {
            //uint32_t Id;                      // 0
            uint32_t baseLevel;                 // 1
            uint32_t maxLevel;                  // 2
            uint32_t spellLevel;                // 3
        };

        // SpellPower.dbc
        struct SpellPowerEntry
        {
            //uint32_t Id;                      // 0
            uint32_t manaCost;                  // 1
            uint32_t manaCostPerlevel;          // 2
            uint32_t ManaCostPercentage;        // 3
            uint32_t manaPerSecond;             // 4
            uint32_t manaPerSecondPerLevel;     // 5
            //uint32_t PowerDisplayId;          // 6 
            float ManaCostPercentageFloat;      // 7
        };

        // SpellRadius.dbc
        struct SpellRadiusEntry
        {
            uint32_t ID;                        // 0
            float radius_min;                   // 1
            float radius_per_level;             // 2
            float radius_max;                   // 3
        };

        // SpellRange.dbc
        struct SpellRangeEntry
        {
            uint32_t ID;                        // 0
            float minRange;                     // 1
            float minRangeFriendly;             // 2
            float maxRange;                     // 3
            float maxRangeFriendly;             // 4
            uint32_t range_type;                // 5
            //char* name1[16]                   // 6-21
            //uint32_t name1_falgs;             // 22
            //char* name2[16]                   // 23-38
            //uint32_t name2_falgs;             // 39
        };

        // SpellReagents.dbc
        struct SpellReagentsEntry
        {
            //uint32_t Id;                              // 0
            int32_t Reagent[MAX_SPELL_REAGENTS];        // 54-61
            uint32_t ReagentCount[MAX_SPELL_REAGENTS];  // 62-69
        };

        // SpellRuneCost.dbc
        struct SpellRuneCostEntry
        {
            uint32_t ID;                        // 0
            uint32_t bloodRuneCost;             // 1
            uint32_t frostRuneCost;             // 2
            uint32_t unholyRuneCost;            // 3
            uint32_t runePowerGain;             // 4
        };

        // SpellScaling.dbc
        struct SpellScalingEntry
        {
            //uint32_t Id;                      // 0
            uint32_t castTimeMin;               // 1
            uint32_t castTimeMax;               // 2
            uint32_t castScalingMaxLevel;       // 3
            uint32_t playerClass;               // 4
            float coeff1[3];                    // 5-7
            float coeff2[3];                    // 8-10
            float coeff3[3];                    // 11-13
            float coefBase;                     // 14
            uint32_t coefLevelBase;             // 15

            bool IsScalableEffect(SpellEffectIndex i) const { return coeff1[i] != 0.0f; };
        };

        // SpellShapeshift.dbc
        struct SpellShapeshiftEntry
        {
            //uint32_t Id;                      // 0
            uint32_t StancesNot;                // 1
            // uint32_t unk_320_2;              // 2
            uint32_t Stances;                   // 3
            // uint32_t unk_320_3;              // 4
            // uint32_t StanceBarOrder;         // 5
        };

        // SpellTargetRestrictions.dbc
        struct SpellTargetRestrictionsEntry
        {
            //uint32_t Id;                      // 0
            float MaxTargetRadius;              // 1
            uint32_t MaxAffectedTargets;        // 2
            uint32_t MaxTargetLevel;            // 3
            uint32_t TargetCreatureType;        // 4
            uint32_t Targets;                   // 5
        };

        // SpellTotems.dbc
        struct SpellTotemsEntry
        {
            //uint32_t Id;                                         // 0
            uint32_t TotemCategory[MAX_SPELL_TOTEM_CATEGORIES];    // 1 2
            uint32_t Totem[MAX_SPELL_TOTEMS];                      // 3 4
        };

        struct SERVER_DECL SpellEntry
        {
            uint32_t Id;                                          // 0
            uint32_t Attributes;                                  // 1
            uint32_t AttributesEx;                                // 2
            uint32_t AttributesExB;                               // 3
            uint32_t AttributesExC;                               // 4
            uint32_t AttributesExD;                               // 5
            uint32_t AttributesExE;                               // 6
            uint32_t AttributesExF;                               // 7
            uint32_t AttributesExG;                               // 8
            uint32_t AttributesExH;                               // 9
            uint32_t AttributesExI;                               // 10
            uint32_t AttributesExJ;                               // 11
            uint32_t CastingTimeIndex;                            // 12
            uint32_t DurationIndex;                               // 13
            int32_t powerType;                                    // 14
            uint32_t rangeIndex;                                  // 15
            float speed;                                          // 16
            uint32_t SpellVisual[2];                              // 17-18
            uint32_t spellIconID;                                 // 19
            uint32_t activeIconID;                                // 20
            char* Name;                                           // 21
            char* Rank;                                           // 22
            char* Description;                                    // 23
            char* BuffDescription;                                // 24
            uint32_t School;                                      // 25
            uint32_t RuneCostID;                                  // 26
            //uint32_t spellMissileID;                            // 27
            //uint32_t spellDescriptionVariableID;                // 28
            uint32_t SpellDifficultyId;                           // 29
            //float unk_1;                                        // 30
            uint32_t SpellScalingId;                              // 31 SpellScaling.dbc
            uint32_t SpellAuraOptionsId;                          // 32 SpellAuraOptions.dbc
            uint32_t SpellAuraRestrictionsId;                     // 33 SpellAuraRestrictions.dbc
            uint32_t SpellCastingRequirementsId;                  // 34 SpellCastingRequirements.dbc
            uint32_t SpellCategoriesId;                           // 35 SpellCategories.dbc
            uint32_t SpellClassOptionsId;                         // 36 SpellClassOptions.dbc
            uint32_t SpellCooldownsId;                            // 37 SpellCooldowns.dbc
            //uint32_t unk_2;                                     // 38 all zeros...
            uint32_t SpellEquippedItemsId;                        // 39 SpellEquippedItems.dbc
            uint32_t SpellInterruptsId;                           // 40 SpellInterrupts.dbc
            uint32_t SpellLevelsId;                               // 41 SpellLevels.dbc
            uint32_t SpellPowerId;                                // 42 SpellPower.dbc
            uint32_t SpellReagentsId;                             // 43 SpellReagents.dbc
            uint32_t SpellShapeshiftId;                           // 44 SpellShapeshift.dbc
            uint32_t SpellTargetRestrictionsId;                   // 45 SpellTargetRestrictions.dbc
            uint32_t SpellTotemsId;                               // 46 SpellTotems.dbc
            //uint32_t ResearchProject;                           // 47 ResearchProject.dbc

            // helpers
            int32_t CalculateSimpleValue(SpellEffectIndex eff) const;
            ClassFamilyMask const& GetEffectSpellClassMask(SpellEffectIndex eff) const;

            // struct access functions
            SpellAuraOptionsEntry const* GetSpellAuraOptions() const;
            SpellAuraRestrictionsEntry const* GetSpellAuraRestrictions() const;
            SpellCastingRequirementsEntry const* GetSpellCastingRequirements() const;
            SpellCategoriesEntry const* GetSpellCategories() const;
            SpellClassOptionsEntry const* GetSpellClassOptions() const;
            SpellCooldownsEntry const* GetSpellCooldowns() const;
            SpellEffectEntry const* GetSpellEffect(SpellEffectIndex eff) const;
            SpellEquippedItemsEntry const* GetSpellEquippedItems() const;
            SpellInterruptsEntry const* GetSpellInterrupts() const;
            SpellLevelsEntry const* GetSpellLevels() const;
            SpellPowerEntry const* GetSpellPower() const;
            SpellReagentsEntry const* GetSpellReagents() const;
            SpellScalingEntry const* GetSpellScaling() const;
            SpellShapeshiftEntry const* GetSpellShapeshift() const;
            SpellTargetRestrictionsEntry const* GetSpellTargetRestrictions() const;
            SpellTotemsEntry const* GetSpellTotems() const;

            // single fields
            uint32_t GetManaCost() const;
            uint32_t GetPreventionType() const;
            uint32_t GetCategory() const;
            uint32_t GetStartRecoveryTime() const;
            uint32_t GetMechanic() const;
            uint32_t GetRecoveryTime() const;
            uint32_t GetCategoryRecoveryTime() const;
            uint32_t GetStartRecoveryCategory() const;
            uint32_t GetSpellLevel() const;
            int32_t GetEquippedItemClass() const;
            SpellFamily GetSpellFamilyName() const;
            uint32_t GetDmgClass() const;
            uint32_t GetDispel() const;
            uint32_t GetMaxAffectedTargets() const;
            uint32_t GetStackAmount() const;
            uint32_t GetManaCostPercentage() const;
            uint32_t GetProcCharges() const;
            uint32_t GetProcChance() const;
            uint32_t GetMaxLevel() const;
            uint32_t GetTargetAuraState() const;
            uint32_t GetManaPerSecond() const;
            uint32_t GetRequiresSpellFocus() const;
            uint32_t GetSpellEffectIdByIndex(SpellEffectIndex index) const;
            uint32_t GetAuraInterruptFlags() const;
            uint32_t GetEffectImplicitTargetAByIndex(SpellEffectIndex index) const;
            int32_t GetAreaGroupId() const;
            uint32_t GetFacingCasterFlags() const;
            uint32_t GetBaseLevel() const;
            uint32_t GetInterruptFlags() const;
            uint32_t GetTargetCreatureType() const;
            int32_t GetEffectMiscValue(SpellEffectIndex index) const;
            uint32_t GetStances() const;
            uint32_t GetStancesNot() const;
            uint32_t GetProcFlags() const;
            uint32_t GetChannelInterruptFlags() const;
            uint32_t GetManaCostPerLevel() const;
            uint32_t GetCasterAuraState() const;
            uint32_t GetTargets() const;
            uint32_t GetEffectApplyAuraNameByIndex(SpellEffectIndex index) const;

            bool IsFitToFamilyMask(uint64 familyFlags, uint32_t familyFlags2 = 0) const
            {
                SpellClassOptionsEntry const* classOpt = GetSpellClassOptions();
                return classOpt && classOpt->IsFitToFamilyMask(familyFlags, familyFlags2);
            }

            bool IsFitToFamily(SpellFamily family, uint64 familyFlags, uint32_t familyFlags2 = 0) const
            {
                SpellClassOptionsEntry const* classOpt = GetSpellClassOptions();
                return classOpt && classOpt->IsFitToFamily(family, familyFlags, familyFlags2);
            }

            bool IsFitToFamilyMask(ClassFamilyMask const& mask) const
            {
                SpellClassOptionsEntry const* classOpt = GetSpellClassOptions();
                return classOpt && classOpt->IsFitToFamilyMask(mask);
            }

            bool IsFitToFamily(SpellFamily family, ClassFamilyMask const& mask) const
            {
                SpellClassOptionsEntry const* classOpt = GetSpellClassOptions();
                return classOpt && classOpt->IsFitToFamily(family, mask);
            }

            inline bool HasAttribute(SpellAttributes attribute) const { return (Attributes & attribute) != 0; }
            inline bool HasAttribute(SpellAttributesEx attribute) const { return (AttributesEx & attribute) != 0; }
            inline bool HasAttribute(SpellAttributesExB attribute) const { return (AttributesExB & attribute) != 0; }
            inline bool HasAttribute(SpellAttributesExC attribute) const { return (AttributesExC & attribute) != 0; }
            inline bool HasAttribute(SpellAttributesExD attribute) const { return (AttributesExD & attribute) != 0; }
            inline bool HasAttribute(SpellAttributesExE attribute) const { return (AttributesExE & attribute) != 0; }
            inline bool HasAttribute(SpellAttributesExF attribute) const { return (AttributesExF & attribute) != 0; }
            inline bool HasAttribute(SpellAttributesExG attribute) const { return (AttributesExG & attribute) != 0; }
            inline bool HasAttribute(SpellAttributesExH attribute) const { return (AttributesExH & attribute) != 0; }
            inline bool HasAttribute(SpellAttributesExI attribute) const { return (AttributesExI & attribute) != 0; }
            inline bool HasAttribute(SpellAttributesExJ attribute) const { return (AttributesExJ & attribute) != 0; }

        private:

            SpellEntry(SpellEntry const&);

            template<typename T>
            bool IsFitToFamilyMask(SpellFamily family, T t) const;
        };

        struct SpellShapeshiftFormEntry
        {
            uint32_t id;                  // 0
            //uint32_t button_pos;        // 1
            //char* name[16];             // 2-17
            //uint32_t name_flags;        // 18
            uint32_t Flags;               // 19
            uint32_t unit_type;           // 20
            //uint32_t unk1               // 21
            uint32_t AttackSpeed;         // 22
            uint32_t modelId;             // 23 alliance?
            uint32_t modelId2;            // 24 horde?
            //uint32_t unk2               // 25
            //uint32_t unk3               // 26
            uint32_t spells[8];           // 27-34
        };

        struct SummonPropertiesEntry
        {
            uint32_t ID;                  // 0
            uint32_t ControlType;         // 1
            uint32_t FactionID;           // 2
            uint32_t Type;                // 3
            uint32_t Slot;                // 4
            uint32_t Flags;               // 5
        };

        struct TalentEntry
        {
            uint32_t TalentID;            // 0
            uint32_t TalentTree;          // 1
            uint32_t Row;                 // 2
            uint32_t Col;                 // 3
            uint32_t RankID[5];           // 4-8
            //uint32_t unk[4];            // 9-12
            uint32_t DependsOn;           // 13
            //uint32_t unk1[2];           // 14-15
            uint32_t DependsOnRank;       // 16
            //uint32_t unk2[2];           // 17-18
            //uint32_t unk3;              // 19
            //uint32_t unk4;              // 20
            //uint32_t unk5;              // 21
        };

        struct TalentTabEntry
        {
            uint32_t TalentTabID;         // 0
            //char* Name;                 // 1
            //uint32_t unk5;              // 2
            uint32_t ClassMask;           // 3
            uint32_t PetTalentMask;       // 4
            uint32_t TabPage;             // 5
            //char* InternalName;         // 6
            //char* description;          // 7
            uint32_t rolesMask;           // 8
            uint32_t masterySpells[2];    // 9-10
        };

        struct TalentTreePrimarySpells
        {
            uint32_t ID;                  // 0
            uint32_t tabID;               // 1
            uint32_t SpellID;             // 2
            //uint32_t unk                // 3
        };

        struct TaxiNodesEntry
        {
            uint32_t id;                  // 0
            uint32_t mapid;               // 1
            float x;                      // 2
            float y;                      // 3
            float z;                      // 4
            char* name;                   // 5
            uint32_t horde_mount;         // 6
            uint32_t alliance_mount;      // 7
        };

        struct TaxiPathEntry
        {
            uint32_t id;                  // 0
            uint32_t from;                // 1
            uint32_t to;                  // 2
            uint32_t price;               // 3
        };

        struct TaxiPathNodeEntry
        {
            //uint32_t id;                // 0
            uint32_t path;                // 1
            uint32_t seq;                 // 2 nodeIndex
            uint32_t mapid;               // 3
            float x;                      // 4
            float y;                      // 5
            float z;                      // 6
            uint32_t flags;               // 7
            uint32_t waittime;            // 8
            uint32_t arivalEventID;       // 9
            uint32_t departureEventID;    // 10
        };

        #define MAX_VEHICLE_SEATS 8

        struct VehicleEntry
        {
            uint32_t ID;                                        // 0
            uint32_t flags;                                     // 1
            float turnSpeed;                                    // 2
            float pitchSpeed;                                   // 3
            float pitchMin;                                     // 4
            float pitchMax;                                     // 5
            uint32_t seatID[MAX_VEHICLE_SEATS];                 // 6-13
            float mouseLookOffsetPitch;                         // 14
            float cameraFadeDistScalarMin;                      // 15
            float cameraFadeDistScalarMax;                      // 16
            float cameraPitchOffset;                            // 17
            float facingLimitRight;                             // 18
            float facingLimitLeft;                              // 19
            float msslTrgtTurnLingering;                        // 20
            float msslTrgtPitchLingering;                       // 21
            float msslTrgtMouseLingering;                       // 22
            float msslTrgtEndOpacity;                           // 23
            float msslTrgtArcSpeed;                             // 24
            float msslTrgtArcRepeat;                            // 25
            float msslTrgtArcWidth;                             // 26
            float msslTrgtImpactRadius[2];                      // 27-28
            char* msslTrgtArcTexture;                           // 29
            char* msslTrgtImpactTexture;                        // 30
            char* msslTrgtImpactModel[2];                       // 31-32
            float cameraYawOffset;                              // 33
            uint32_t uiLocomotionType;                          // 34
            float msslTrgtImpactTexRadius;                      // 35
            uint32_t uiSeatIndicatorType;                       // 36
            uint32_t powerType;                                 // 37
            //uint32_t unk1;                                    // 38
            //uint32_t unk2;                                    // 39  
        };

        enum VehicleSeatFlags
        {
            VEHICLE_SEAT_FLAG_HIDE_PASSENGER             = 0x00000200,           // Passenger is hidden
            VEHICLE_SEAT_FLAG_UNK11                      = 0x00000400,
            VEHICLE_SEAT_FLAG_CAN_CONTROL                = 0x00000800,           // Lua_UnitInVehicleControlSeat
            VEHICLE_SEAT_FLAG_CAN_ATTACK                 = 0x00004000,           // Can attack, cast spells and use items from vehicle?
            VEHICLE_SEAT_FLAG_USABLE                     = 0x02000000,           // Lua_CanExitVehicle
            VEHICLE_SEAT_FLAG_CAN_SWITCH                 = 0x04000000,           // Lua_CanSwitchVehicleSeats
            VEHICLE_SEAT_FLAG_CAN_CAST                   = 0x20000000,           // Lua_UnitHasVehicleUI
        };

        enum VehicleSeatFlagsB
        {
            VEHICLE_SEAT_FLAG_B_NONE                     = 0x00000000,
            VEHICLE_SEAT_FLAG_B_USABLE_FORCED            = 0x00000002, 
            VEHICLE_SEAT_FLAG_B_USABLE_FORCED_2          = 0x00000040,
            VEHICLE_SEAT_FLAG_B_USABLE_FORCED_3          = 0x00000100,
        };

        struct VehicleSeatEntry
        {
            uint32_t ID;                                          // 0
            uint32_t flags;                                       // 1
            int32_t attachmentID;                                 // 2
            float attachmentOffsetX;                              // 3
            float attachmentOffsetY;                              // 4
            float attachmentOffsetZ;                              // 5
            float enterPreDelay;                                  // 6
            float enterSpeed;                                     // 7
            float enterGravity;                                   // 8
            float enterMinDuration;                               // 9
            float enterMaxDuration;                               // 10
            float enterMinArcHeight;                              // 11
            float enterMaxArcHeight;                              // 12
            int32_t enterAnimStart;                               // 13
            int32_t enterAnimLoop;                                // 14
            int32_t rideAnimStart;                                // 15
            int32_t rideAnimLoop;                                 // 16
            int32_t rideUpperAnimStart;                           // 17
            int32_t rideUpperAnimLoop;                            // 18
            float exitPreDelay;                                   // 19
            float exitSpeed;                                      // 20
            float exitGravity;                                    // 21
            float exitMinDuration;                                // 22
            float exitMaxDuration;                                // 23
            float exitMinArcHeight;                               // 24
            float exitMaxArcHeight;                               // 25
            int32_t exitAnimStart;                                // 26
            int32_t exitAnimLoop;                                 // 27
            int32_t exitAnimEnd;                                  // 28
            float passengerYaw;                                   // 29
            float passengerPitch;                                 // 30
            float passengerRoll;                                  // 31
            int32_t passengerAttachmentID;                        // 32
            int32_t vehicleEnterAnim;                             // 33
            int32_t vehicleExitAnim;                              // 34
            int32_t vehicleRideAnimLoop;                          // 35
            int32_t vehicleEnterAnimBone;                         // 36
            int32_t vehicleExitAnimBone;                          // 37
            int32_t vehicleRideAnimLoopBone;                      // 38
            float vehicleEnterAnimDelay;                          // 39
            float vehicleExitAnimDelay;                           // 40
            uint32_t vehicleAbilityDisplay;                       // 41
            uint32_t enterUISoundID;                              // 42
            uint32_t exitUISoundID;                               // 43
            int32_t uiSkin;                                       // 44
            uint32_t flagsB;                                      // 45

            bool IsUsable() const
            {
                if ((flags & VEHICLE_SEAT_FLAG_USABLE) != 0)
                    return true;
                else
                    return false;
            }

            bool IsController() const
            {
                if ((flags & VEHICLE_SEAT_FLAG_CAN_CONTROL) != 0)
                    return true;
                else
                    return false;
            }

            bool HidesPassenger() const
            {
                if ((flags & VEHICLE_SEAT_FLAG_HIDE_PASSENGER) != 0)
                    return true;
                else
                    return false;
            }
        };

        struct WMOAreaTableEntry
        {
            uint32_t id;                // 0
            int32_t rootId;             // 1
            int32_t adtId;              // 2
            int32_t groupId;            // 3
            //uint32_t field4;          // 4
            //uint32_t field5;          // 5
            //uint32_t field6;          // 6
            //uint32_t field7;          // 7
            //uint32_t field8;          // 8
            uint32_t flags;             // 9
            uint32_t areaId;            // 10
            //char Name[16];            // 11-26
            //uint32_t nameflags;       // 27
        };

        struct WorldMapOverlayEntry
        {
            uint32_t ID;                // 0
            //uint32_t worldMapID;      // 1
            uint32_t areaID;            // 2
            uint32_t areaID_2;          // 3
            uint32_t areaID_3;          // 4
            uint32_t areaID_4;          // 5
            //uint32_t unk1[2];         // 6-7
            //uint32_t unk2;            // 8
            //uint32_t unk3[7];         // 9-16
        };

        #pragma pack(pop)

        typedef std::set<uint32_t> SpellCategorySet;
        typedef std::map<uint32_t, SpellCategorySet> SpellCategoryStore;
        struct SpellEffect
        {
            SpellEffect()
            {
                effects[0] = nullptr;
                effects[1] = nullptr;
                effects[2] = nullptr;
            }
            SpellEffectEntry const* effects[3];
        };
        typedef std::map<uint32_t, SpellEffect> SpellEffectMap;
    }
}