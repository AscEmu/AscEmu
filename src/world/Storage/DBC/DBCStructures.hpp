/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _DBC_STRUCTURES_HPP
#define _DBC_STRUCTURES_HPP

#include "Common.h"
#include "Spell/SpellDefines.hpp"

enum AreaFlags
{
    AREA_FLAG_UNK0               = 0x00000001,                // Unknown
    AREA_FLAG_UNK1               = 0x00000002,                // Razorfen Downs, Naxxramas and Acherus: The Ebon Hold (3.3.5a)
    AREA_FLAG_UNK2               = 0x00000004,                // Only used for areas on map 571 (development before)
    AREA_FLAG_SLAVE_CAPITAL      = 0x00000008,                // city and city subsones
    AREA_FLAG_UNK3               = 0x00000010,                // can't find common meaning
    AREA_FLAG_SLAVE_CAPITAL2     = 0x00000020,                // slave capital city flag?
    AREA_FLAG_ALLOW_DUELS        = 0x00000040,                // allow to duel here
    AREA_FLAG_ARENA              = 0x00000080,                // arena, both instanced and world arenas
    AREA_FLAG_CAPITAL            = 0x00000100,                // main capital city flag
    AREA_FLAG_CITY               = 0x00000200,                // only for one zone named "City" (where it located?)
    AREA_FLAG_OUTLAND            = 0x00000400,                // expansion zones? (only Eye of the Storm not have this flag, but have 0x00004000 flag)
    AREA_FLAG_SANCTUARY          = 0x00000800,                // sanctuary area (PvP disabled)
    AREA_FLAG_NEED_FLY           = 0x00001000,                // Respawn alive at the graveyard without corpse
    AREA_FLAG_UNUSED1            = 0x00002000,                // Unused in 3.3.5a
    AREA_FLAG_OUTLAND2           = 0x00004000,                // expansion zones? (only Circle of Blood Arena not have this flag, but have 0x00000400 flag)
    AREA_FLAG_OUTDOOR_PVP        = 0x00008000,                // pvp objective area? (Death's Door also has this flag although it's no pvp object area)
    AREA_FLAG_ARENA_INSTANCE     = 0x00010000,                // used by instanced arenas only
    AREA_FLAG_UNUSED2            = 0x00020000,                // Unused in 3.3.5a
    AREA_FLAG_CONTESTED_AREA     = 0x00040000,                // On PvP servers these areas are considered contested, even though the zone it is contained in is a Horde/Alliance territory.
    AREA_FLAG_UNK4               = 0x00080000,                // Valgarde and Acherus: The Ebon Hold
    AREA_FLAG_LOWLEVEL           = 0x00100000,                // used for some starting areas with area_level <= 15
    AREA_FLAG_TOWN               = 0x00200000,                // small towns with Inn
    AREA_FLAG_REST_ZONE_HORDE    = 0x00400000,                // Instead of using areatriggers, the zone will act as one for Horde players (Warsong Hold, Acherus: The Ebon Hold, New Agamand Inn, Vengeance Landing Inn, Sunreaver Pavilion, etc)
    AREA_FLAG_REST_ZONE_ALLIANCE = 0x00800000,                // Instead of using areatriggers, the zone will act as one for Alliance players (Valgarde, Acherus: The Ebon Hold, Westguard Inn, Silver Covenant Pavilion, etc)
    AREA_FLAG_WINTERGRASP        = 0x01000000,                // Wintergrasp and it's subzones
    AREA_FLAG_INSIDE             = 0x02000000,                // used for determinating spell related inside/outside questions in Map::IsOutdoors
    AREA_FLAG_OUTSIDE            = 0x04000000,                // used for determinating spell related inside/outside questions in Map::IsOutdoors
    AREA_FLAG_WINTERGRASP_2      = 0x08000000,                // Can Hearth And Resurrect From Area
    AREA_FLAG_NO_FLY_ZONE        = 0x20000000                 // Marks zones where you cannot fly
};

struct WMOAreaTableTripple
{
    WMOAreaTableTripple(int32 r, int32 a, int32 g) : groupId(g), rootId(r), adtId(a)
    { }

    bool operator <(const WMOAreaTableTripple & b) const
    {
        return memcmp(this, &b, sizeof(WMOAreaTableTripple)) < 0;
    }

    int32 groupId;
    int32 rootId;
    int32 adtId;
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
            //char const gt_oct_class_combat_rating_scalar_format[] = "df"; new
            //char const gt_oct_hp_per_stamina_format[] = "df"; new
            //char const gt_oct_regen_hp_format[] = "xf";
            char const gt_oct_regen_mp_format[] = "f";
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
            uint32 ID;                 // 0
            uint32 parentCategory;     // 1 -1 for main category
            const char* name;          // 2-17
            uint32 name_flags;         // 18
            uint32 sortOrder;          // 19
        };

        struct AchievementCriteriaEntry
        {
            uint32 ID;                      // 0
            uint32 referredAchievement;     // 1
            uint32 requiredType;            // 2
            union
            {
                // ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE = 0
                ///\todo also used for player deaths..
                struct
                {
                    uint32 creatureID;                             // 3
                    uint32 creatureCount;                          // 4
                } kill_creature;

                // ACHIEVEMENT_CRITERIA_TYPE_WIN_BG = 1
                ///\todo there are further criterias instead just winning
                struct
                {
                    uint32 bgMapID;                                // 3
                    uint32 winCount;                               // 4
                } win_bg;

                // ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL = 5
                struct
                {
                    uint32 unused;                                 // 3
                    uint32 level;                                  // 4
                } reach_level;

                // ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL = 7
                struct
                {
                    uint32 skillID;                                // 3
                    uint32 skillLevel;                             // 4
                } reach_skill_level;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT = 8
                struct
                {
                    uint32 linkedAchievement;                      // 3
                } complete_achievement;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT = 9
                struct
                {
                    uint32 unused;                                 // 3
                    uint32 totalQuestCount;                        // 4
                } complete_quest_count;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY = 10
                struct
                {
                    uint32 unused;                                 // 3
                    uint32 numberOfDays;                           // 4
                } complete_daily_quest_daily;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE = 11
                struct
                {
                    uint32 zoneID;                                 // 3
                    uint32 questCount;                             // 4
                } complete_quests_in_zone;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST = 14
                struct
                {
                    uint32 unused;                                 // 3
                    uint32 questCount;                             // 4
                } complete_daily_quest;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND= 15
                struct
                {
                    uint32 mapID;                                  // 3
                } complete_battleground;

                // ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP= 16
                struct
                {
                    uint32 mapID;                                  // 3
                } death_at_map;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_RAID = 19
                struct
                {
                    uint32 groupSize;                              // 3 can be 5, 10 or 25
                } complete_raid;

                // ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE = 20
                struct
                {
                    uint32 creatureEntry;                          // 3
                } killed_by_creature;

                // ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING = 24
                struct
                {
                    uint32 unused;                                 // 3
                    uint32 fallHeight;                             // 4
                } fall_without_dying;

                // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST = 27
                struct
                {
                    uint32 questID;                                // 3
                    uint32 questCount;                             // 4
                } complete_quest;

                // ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET = 28
                // ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2= 69
                struct
                {
                    uint32 spellID;                                // 3
                    uint32 spellCount;                             // 4
                } be_spell_target;

                // ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL= 29
                struct
                {
                    uint32 spellID;                                // 3
                    uint32 castCount;                              // 4
                } cast_spell;

                // ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA = 31
                struct
                {
                    uint32 areaID;                                 // 3 Reference to AreaTable.dbc
                    uint32 killCount;                              // 4
                } honorable_kill_at_area;

                // ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA = 32
                struct
                {
                    uint32 mapID;                                  // 3 Reference to Map.dbc
                } win_arena;

                // ACHIEVEMENT_CRITERIA_TYPE_PLAY_ARENA = 33
                struct
                {
                    uint32 mapID;                                  // 3 Reference to Map.dbc
                } play_arena;

                // ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL = 34
                struct
                {
                    uint32 spellID;                                // 3 Reference to Map.dbc
                } learn_spell;

                // ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM = 36
                struct
                {
                    uint32 itemID;                                 // 3
                    uint32 itemCount;                              // 4
                } own_item;

                // ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA = 37
                struct
                {
                    uint32 unused;                                 // 3
                    uint32 count;                                  // 4
                    uint32 flag;                                   // 5 4=in a row
                } win_rated_arena;

                // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_TEAM_RATING = 38
                struct
                {
                    uint32 teamtype;                               // 3 {2,3,5}
                } highest_team_rating;

                // ACHIEVEMENT_CRITERIA_TYPE_REACH_TEAM_RATING = 39
                struct
                {
                    uint32 teamtype;                               // 3 {2,3,5}
                    uint32 teamrating;                             // 4
                } reach_team_rating;

                // ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL = 40
                struct
                {
                    uint32 skillID;                                // 3
                    uint32 skillLevel;                             // 4 apprentice=1, journeyman=2, expert=3, artisan=4, master=5, grand master=6
                } learn_skill_level;

                // ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM = 41
                struct
                {
                    uint32 itemID;                                 // 3
                    uint32 itemCount;                              // 4
                } use_item;

                // ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM = 42
                struct
                {
                    uint32 itemID;                                 // 3
                    uint32 itemCount;                              // 4
                } loot_item;

                // ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA = 43
                struct
                {
                    uint32 areaReference;                          // 3 - this is an index to WorldMapOverlay
                } explore_area;

                // ACHIEVEMENT_CRITERIA_TYPE_OWN_RANK= 44
                struct
                {
                    ///\todo This rank is _NOT_ the index from CharTitles.dbc
                    uint32 rank;                                   // 3
                } own_rank;

                // ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT= 45
                struct
                {
                    uint32 unused;                                 // 3
                    uint32 numberOfSlots;                          // 4
                } buy_bank_slot;

                // ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION= 46
                struct
                {
                    uint32 factionID;                              // 3
                    uint32 reputationAmount;                       // 4 Total reputation amount, so 42000 = exalted
                } gain_reputation;

                // ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION= 47
                struct
                {
                    uint32 unused;                                 // 3
                    uint32 numberOfExaltedFactions;                // 4
                } gain_exalted_reputation;

                // ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM = 49
                ///\todo where is the required itemlevel stored?
                struct
                {
                    uint32 itemSlot;                               // 3
                } equip_epic_item;

                // ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT= 50
                struct
                {
                    uint32 rollValue;                              // 3
                    uint32 count;                                  // 4
                } roll_need_on_loot;

                // ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS = 52
                struct
                {
                    uint32 classID;                                // 3
                    uint32 count;                                  // 4
                } hk_class;

                // ACHIEVEMENT_CRITERIA_TYPE_HK_RACE = 53
                struct
                {
                    uint32 raceID;                                 // 3
                    uint32 count;                                  // 4
                } hk_race;

                // ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE = 54
                ///\todo where is the information about the target stored?
                struct
                {
                    uint32 emoteID;                                // 3
                } do_emote;

                // ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE = 55
                struct
                {
                    uint32 unused;                                 // 3
                    uint32 count;                                  // 4
                    uint32 flag;                                   // 5 =3 for battleground healing
                    uint32 mapid;                                  // 6
                } healing_done;

                // ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM = 57
                struct
                {
                    uint32 itemID;                                 // 3
                } equip_item;

                // ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD = 62
                struct
                {
                    uint32 unknown;                                 // 3
                    uint32 goldInCopper;                            // 4
                } quest_reward_money;

                // ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY = 67
                struct
                {
                    uint32 unused;                                 // 3
                    uint32 goldInCopper;                           // 4
                } loot_money;

                // ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT = 68
                struct
                {
                    uint32 goEntry;                                // 3
                    uint32 useCount;                               // 4
                } use_gameobject;

                // ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL= 70
                ///\todo are those special criteria stored in the dbc or do we have to add another sql table?
                struct
                {
                    uint32 unused;                                 // 3
                    uint32 killCount;                              // 4
                } special_pvp_kill;

                // ACHIEVEMENT_CRITERIA_TYPE_FISH_IN_GAMEOBJECT = 72
                struct
                {
                    uint32 goEntry;                                // 3
                    uint32 lootCount;                              // 4
                } fish_in_gameobject;

                // ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS= 75
                struct
                {
                    uint32 unknown;                                // 3 777=?
                    uint32 mountCount;                             // 4
                } number_of_mounts;

                // ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL = 76
                struct
                {
                    uint32 unused;                                 // 3
                    uint32 duelCount;                              // 4
                } win_duel;

                // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_POWER = 96
                struct
                {
                    uint32 powerType;                              // 3 mana= 0, 1=rage, 3=energy, 6=runic power
                } highest_power;

                // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_STAT = 97
                struct
                {
                    uint32 statType;                               // 3 4=spirit, 3=int, 2=stamina, 1=agi, 0=strength
                } highest_stat;

                // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_SPELLPOWER = 98
                struct
                {
                    uint32 spellSchool;                            // 3
                } highest_spellpower;

                // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_RATING = 100
                struct
                {
                    uint32 ratingType;                             // 3
                } highest_rating;

                // ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE = 109
                struct
                {
                    uint32 lootType;                               // 3 3=fishing, 2=pickpocket, 4=disentchant
                    uint32 lootTypeCount;                          // 4
                } loot_type;

                // ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2 = 110
                struct
                {
                    uint32 skillLine;                              // 3
                    uint32 spellCount;                             // 4
                } cast_spell2;

                // ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE= 112
                struct
                {
                    uint32 skillLine;                              // 3
                    uint32 spellCount;                             // 4
                } learn_skill_line;

                // ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL = 113
                struct
                {
                    uint32 unused;                                 // 3
                    uint32 killCount;                              // 4
                } honorable_kill;

                struct
                {
                    uint32 field3;                                 // 3 main requirement
                    uint32 field4;                                 // 4 main requirement count
                    uint32 additionalRequirement1_type;            // 5 additional requirement 1 type
                    uint32 additionalRequirement1_value;           // 6 additional requirement 1 value
                    uint32 additionalRequirement2_type;            // 7 additional requirement 2 type
                    uint32 additionalRequirement2_value;           // 8 additional requirement 1 value
                } raw;
            };
            //uint32 unk                    // 9
            char* name;                     // 10
            uint32 completionFlag;          // 11
            uint32 groupFlag;               // 12 timed criteria
            uint32 unk1;                    // 13 timed criteria misc id
            uint32 timeLimit;               // 14 time limit in seconds
            uint32 index;                   // 15 order
        };

        struct AchievementEntry     //cata
        {
            uint32 ID;                      // 0
            int32 factionFlag;              // 1 -1=all, 0=horde, 1=alliance
            int32 mapID;                    // 2 -1=none
            uint32 parentAchievement;       // 3
            char* name;                     // 4
            char* description;              // 5
            uint32 categoryId;              // 6
            uint32 points;                  // 7 reward points
            uint32 orderInCategory;         // 8
            uint32 flags;                   // 9
            uint32 icon;                    // 10
            char* rewardName;               // 11 title/item reward name
            uint32 count;                   // 12
            uint32 refAchievement;          // 13
        };

        struct AreaGroupEntry       //cata
        {
            uint32 AreaGroupId;             // 0
            uint32 AreaId[6];               // 1-6
            uint32 next_group;              // 7
        };

        struct AreaTableEntry               //cata
        {
            uint32 id;                      // 0
            uint32 map_id;                  // 1
            uint32 zone;                    // 2 if 0 then it's zone, else it's zone id of this area
            uint32 explore_flag;            // 3, main index
            uint32 flags;                   // 4, unknown value but 312 for all cities
                                            // 5-9 unused
            int32 area_level;               // 10
            char* area_name;                // 11
            uint32 team;                    // 12
            uint32 liquid_type_override[4]; // 13-16 liquid override by type
                                            // 17
                                            // 18
                                            // 19
            //uint32 unk20;                 // 20
            //uint32 unk21;                 // 21
            //uint32 unk22;                 // 22
            //uint32 unk23;                 // 23
            //uint32 unk24;                 // 24
        };

        struct AreaTriggerEntry     //cata
        {
            uint32 id;              // 0
            uint32 mapid;           // 1
            float x;                // 2
            float y;                // 3
            float z;                // 4
            //uint32                // 5
            //uint32                // 6
            //uint32                // 7
            float o;                // 5 radius?
            float box_x;            // 6 extent x edge
            float box_y;            // 7 extent y edge
            float box_z;            // 8 extent z edge
            float box_o;            // 9 extent rotation by about z axis
        };

        //\todo danko
        //struct ArmorLocationEntry   
        //{
        //    uint32 InventoryType;   // 0
        //    float Value[5];         // 1-5 multiplier for armor types (cloth...plate, no armor?)
        //};

        struct AuctionHouseEntry    //cata
        {
            uint32 id;              // 0
            uint32 faction;         // 1
            uint32 fee;             // 2
            uint32 tax;             // 3
            //char* name;           // 4
        };

        struct BankBagSlotPrices    //cata
        {
            uint32 Id;              // 0
            uint32 Price;           // 1
        };

        struct BarberShopStyleEntry //cata
        {
            uint32 id;              // 0
            uint32 type;            // 1 value 0 -> hair, value 2 -> facialhair
            //char* name;           // 2
            //uint32 unk_name;      // 3
            //float CostMultiplier; // 4
            uint32 race;            // 5
            uint32 gender;          // 6 0 male, 1 female
            uint32 hair_id;         // 7 Hair ID
        };

        #define OUTFIT_ITEMS 24

        //\todo danko
        //struct CharStartOutfitEntry
        //{
        //    //uint32 Id;                                // 0
        //    uint32 RaceClassGender;                     // 1
        //    int32 ItemId[OUTFIT_ITEMS];                 // 2-25
        //    //int32 ItemDisplayId[OUTFIT_ITEMS];        // 26-29
        //    //int32 ItemInventorySlot[OUTFIT_ITEMS];    // 50-73
        //    //uint32 Unknown1;                          // 74
        //    //uint32 Unknown2;                          // 75
        //    //uint32 Unknown3;                          // 76
        //    //uint32 Unknown4;                          // 77
        //    //uint32 Unknown5;                          // 78
        //};

        struct CharTitlesEntry  //cata
        {
            uint32 ID;                      // 0, title ids
            //uint32 unk1;                  // 1 flags?
            char* name;                     // 2
            //char* name2;                  // 3
            uint32 bit_index;               // 4 used in PLAYER_CHOSEN_TITLE and 1<<index in PLAYER__FIELD_KNOWN_TITLES
            //uint32 unk                    // 5
        };

        struct ChatChannelsEntry            //cata
        {
            uint32 id;                      // 0
            uint32 flags;                   // 1
            //uint32 faction                // 2
            char* name_pattern;             // 3
            //char* channel_name;           // 4
        };

        struct ChrClassesEntry
        {
            uint32 class_id;                // 0
            uint32 power_type;              // 1
            //uint32 unk2;                  // 2
            char* name;                     // 3
            //char* name_female;            // 4
            //char* name_neutral;           // 5
            //char* name_capitalized        // 6
            uint32 spellfamily;             // 7
            //uint32 unk4;                  // 8
            uint32 cinematic_sequence;      // 9 CinematicSequences.dbc
            uint32 expansion;               // 10
            uint32 apPerStr;                // 11
            uint32 apPerAgi;                // 12
            uint32 rapPerAgi;               // 13
        };

        struct ChrRacesEntry        //cata
        {
            uint32 race_id;                 // 0
            //uint32 flags;                 // 1
            uint32 faction_id;              // 2
            //uint32 unk1;                  // 3
            uint32 model_male;              // 4
            uint32 model_female;            // 5
            // uint32 unk2;                 // 6
            uint32 team_id;                 // 7
            //uint32 unk3[4];               // 8-11
            uint32 cinematic_id;            // 12 CinematicSequences.dbc
            //uint32 unk4                   // 13
            char* name;                     // 14
            //char* name_female;            // 15
            //char* name_neutral;           // 16
            //uint32 unk5[2]                // 17-18
                                            // 19
            uint32 expansion;               // 20
            //uint32                        // 21
            //uint32                        // 22
            //uint32                        // 23
        };

        //\todo danko
        //struct ChrPowerTypesEntry
        //{
        //    uint32 entry;                   // 0
        //    uint32 classId;                 // 1
        //    uint32 power;                   // 2
        //};

        struct CreatureDisplayInfoEntry
        {
            uint32 Displayid;               // 0
            uint32 ModelId;                 // 1
            //uint32 sound_id;              // 2
            uint32 ExtendedDisplayInfoID;   // 3
            float scale;                    // 4
            //uint32 unk01;                 // 5
            //uint32 unk02[2];              // 6-8
            //uint32 unk03;                 // 9
            //uint32 unk04;                 // 10
            //uint32 unk05;                 // 11
            //uint32 unk06;                 // 12
            //uint32 unk07;                 // 13
            //uint32 unk08;                 // 14
            //uint32 unk09;                 // 15
            //uint32 unk10;                 // 16
        };

        struct CreatureDisplayInfoExtraEntry
        {
            uint32 DisplayExtraId;          // 0
            uint32 Race;                    // 1
            //uint32 Gender;                // 2
            //uint32 SkinColor;             // 3
            //uint32 FaceType;              // 4
            //uint32 HairType;              // 5
            //uint32 HairStyle;             // 6
            //uint32 BeardStyle;            // 7
            //uint32 Equipment[11];         // 8-18
            //uint32 CanEquip;              // 19
            //char*                         // 20
        };

        struct CreatureFamilyEntry          //cata
        {
            uint32 ID;                      // 0
            float minsize;                  // 1
            uint32 minlevel;                // 2
            float maxsize;                  // 3
            uint32 maxlevel;                // 4
            uint32 skilline;                // 5
            uint32 tameable;                // 6 second skill line - 270 Generic
            uint32 petdietflags;            // 7
            uint32 talenttree;              // 8 (-1 = none, 0 = ferocity(410), 1 = tenacity(409), 2 = cunning(411))
            //uint32 unk;                   // 9 some index 0 - 63
            char* name;                     // 10
            //uint32 nameflags;             // 11
        };

        struct CreatureSpellDataEntry   //cata
        {
            uint32 id;                      // 0
            uint32 Spells[3];
            uint32 PHSpell;
            uint32 Cooldowns[3];
            uint32 PH;
        };

        //\todo danko
        //struct CreatureTypeEntry
        //{
        //    uint32 ID;                      // 0
        //    //char* Name;                   // 1
        //    //uint32 no_expirience;         // 2
        //};

        struct CurrencyTypesEntry       //cata
        {
            uint32 item_id;         // 0
            uint32 Category;        // 1
            char* name;             // 2
            //char* unk             // 3
            //char* unk2            // 4
            //uint32 unk5;          // 5
            //uint32 unk6;          // 6
            uint32 TotalCap;        // 7
            uint32 WeekCap;         // 8
            uint32 Flags;           // 9
            //char* description;    // 10
        };

        struct DurabilityCostsEntry //cata
        {
            uint32 itemlevel;       // 0
            uint32 modifier[29];    // 1-29
        };

        struct DurabilityQualityEntry   //cata
        {
            uint32 id;              // 0
            float quality_modifier; // 1
        };

        struct EmotesEntry
        {
            uint32 Id;              // 0
            //char* name;           // 1
            //uint32 animationId;   // 2
            uint32 Flags;           // 3
            uint32 EmoteType;       // 4
            uint32 UnitStandState;  // 5
            //uint32 soundId;       // 6
            //uint32 unk;           // 7
        };

        struct EmotesTextEntry  //cata
        {
            uint32 Id;              // 0
            //uint32 name;          // 1
            uint32 textid;          // 2
            //uint32 unk1;          // 4
        };

        struct FactionEntry //cata
        {
            uint32 ID;                      // 0
            int32 RepListId;                // 1
            uint32 RaceMask[4];             // 2-5
            uint32 ClassMask[4];            // 6-9
            int32 baseRepValue[4];          // 10-13
            uint32 repFlags[4];             // 14-17
            uint32 parentFaction;           // 18
            float spillover_rate_in;        // 19
            float spillover_rate_out;       // 20
            uint32 spillover_max_in;        // 21
            //uint32 unk1;                  // 22
            char* Name;                     // 23
            //uint32 Description;           // 24
            //uint32 description_flags;     // 25
        };

        struct FactionTemplateEntry //cata
        {
            uint32 ID;                      // 0
            uint32 Faction;                 // 1
            uint32 FactionGroup;            // 2
            uint32 Mask;                    // 3
            uint32 FriendlyMask;            // 4
            uint32 HostileMask;             // 5
            uint32 EnemyFactions[4];        // 6-9
            uint32 FriendlyFactions[4];     // 10-13
        };

        struct GameObjectDisplayInfoEntry   //cata
        {
            uint32 Displayid;               // 0
            char* filename;                 // 1
            //uint32  unk1[10];             // 2-11
            float minX;                     // 12
            float minY;                     // 13
            float minZ;                     // 14
            float maxX;                     // 15
            float maxY;                     // 16
            float maxZ;                     // 17
            //uint32 transport;             // 18
            //uint32 unk;                   // 19
            //uint32 unk;                   // 20
        };

        struct GemPropertiesEntry       //cata
        {
            uint32 Entry;                   // 0
            uint32 EnchantmentID;           // 1
            //uint32 unk1;                  // 2 bool
            //uint32 unk2;                  // 3 bool
            uint32 SocketMask;              // 4
        };

        struct GlyphPropertiesEntry     //cata
        {
            uint32 Entry;                   // 0
            uint32 SpellID;                 // 1
            uint32 Type;                    // 2 (0 = Major, 1 = Minor)
            uint32 unk;                     // 3 glyph_icon spell.dbc
        };

        struct GlyphSlotEntry   //cata
        {
            uint32 Id;              // 0
            uint32 Type;            // 1
            uint32 Slot;            // 2
        };

        struct GtBarberShopCostBaseEntry    //cata
        {
            float cost;             // 0 cost base
        };

        struct GtChanceToMeleeCritEntry     //cata
        {
            float val;              // 0
        };

        struct GtChanceToMeleeCritBaseEntry //cata
        {
            float val;              // 0
        };

        struct GtChanceToSpellCritEntry //cata
        {
            float val;              // 0
        };

        struct GtChanceToSpellCritBaseEntry //cata
        {
            float val;              // 0
        };

        struct GtCombatRatingsEntry //cata
        {
            float val;              // 0
        };

        //\todo danko
        //struct GtOCTRegenHPEntry
        //{
        //    float ratio;            // 0
        //};

        struct GtOCTRegenMPEntry    //cata
        {
            float ratio;            // 0
        };

        //\todo danko
        //struct GtRegenHPPerSptEntry
        //{
        //    float ratio;            // 0 regen base
        //};

        struct GtRegenMPPerSptEntry //cata
        {
            float ratio;            // 0 regen base
        };

        struct GuildPerkSpellsEntry
        {
            //uint32 Id;            // 0
            uint32 Level;           // 1
            uint32 SpellId;         // 2
        };

#define MAX_HOLIDAY_DURATIONS 10
#define MAX_HOLIDAY_DATES 26
#define MAX_HOLIDAY_FLAGS 10

        struct HolidaysEntry    //cata
        {
            uint32 Id;                                  // 0
            //uint32 Duration[MAX_HOLIDAY_DURATIONS];     // 1-10
            //uint32 Date[MAX_HOLIDAY_DATES];             // 11-36
            //uint32 Region;                              // 37
            //uint32 Looping;                             // 38
            //uint32 CalendarFlags[MAX_HOLIDAY_FLAGS];    // 39-48
            ////uint32 holidayNameId;                     // 49 HolidayNames.dbc
            ////uint32 holidayDescriptionId;              // 50 HolidayDescriptions.dbc
            //char* TextureFilename;                      // 51
            //uint32 Priority;                            // 52
            //uint32 CalendarFilterType;                  // 53
            ////uint32 flags;                             // 54
        };

        struct ItemLimitCategoryEntry   //cata
        {
            uint32 Id;                      // 0
            //char* name;                   // 1
            uint32 maxAmount;               // 2
            uint32 equippedFlag;            // 3
        };

        struct ItemRandomPropertiesEntry    //cata
        {
            uint32 ID;                      // 0
            //char* name1;                  // 1
            uint32 spells[5];               // 2-6 enchant_id
            char* name_suffix;              // 7
        };

        struct ItemRandomSuffixEntry    //cata
        {
            uint32 id;                      // 0
            char* name_suffix;              // 1
            //uint32 name_suffix_flags;     // 2
            uint32 enchantments[5];         // 3-7
            uint32 prefixes[5];             // 8-12
        };

        struct ItemSetEntry //cata
        {
            //uint32 id;                    // 0
            char* name;                     // 1
            //uint32 itemid[17];            // 2-18
            uint32 SpellID[8];              // 19-26
            uint32 itemscount[8];           // 27-34
            uint32 RequiredSkillID;         // 35
            uint32 RequiredSkillAmt;        // 36
        };

        struct LFGDungeonEntry
        {
            uint32 ID;              // 0
            char* name;             // 1
            uint32 minlevel;        // 2
            uint32 maxlevel;        // 3
            uint32 reclevel;        // 4
            uint32 recminlevel;     // 5
            uint32 recmaxlevel;     // 6
            int32 map;              // 7
            uint32 difficulty;      // 8
            uint32 unk;             // 9
            uint32 flags;           // 10
            int32 type;             // 11
            char* iconname;         // 12
            uint32 expansion;       // 13
            uint32 unk4;            // 14
            uint32 unk5;            // 15
            char* unk_text;         // 16
            uint32 grouptype;       // 17
            uint32 unkflags1;       // 18
            uint32 unkflags2;       // 19
            uint32 unk7;            // 20

            // Helpers
            uint32 Entry() const { return ID + (type << 24); }
        };

        struct LiquidTypeEntry  //cata
        {
            uint32 Id;                  // 0
            //char* Name;               // 1
            //uint32 Flags;             // 2
            uint32 Type;                // 3
            //uint32 SoundId;           // 4
            uint32 SpellId;             // 5
            //float MaxDarkenDepth;     // 6
            //float FogDarkenIntensity; // 7
            //float AmbDarkenIntensity; // 8
            //float DirDarkenIntensity; // 9
            //uint32 LightID;           // 10
            //float ParticleScale;      // 11
            //uint32 ParticleMovement;  // 12
            //uint32 ParticleTexSlots;  // 13
            //uint32 LiquidMaterialID;  // 14
            //char* Texture[6];         // 15-20
            //uint32 Color[2];          // 21-22
            //float Unk1[18];           // 23-40
            //uint32 Unk2[4];           // 41-44
        };

#define LOCK_NUM_CASES 8

        struct LockEntry    //cata
        {
            uint32 Id;                              // 0
            uint32 locktype[LOCK_NUM_CASES];        // 1-8 If this is 1, then the next lockmisc is an item ID, if it's 2, then it's an iRef to LockTypes.dbc.
            uint32 lockmisc[LOCK_NUM_CASES];        // 9-16 Item to unlock or iRef to LockTypes.dbc depending on the locktype.
            uint32 minlockskill[LOCK_NUM_CASES];    // 17-24 Required skill needed for lockmisc (if locktype = 2).
            //uint32 action[LOCK_NUM_CASES];        // 25-32 Something to do with direction / opening / closing.
        };

        struct MailTemplateEntry    //cata
        {
            uint32 ID;              // 0
            char* subject;          // 1
            char* content;          // 2
        };

        struct MapEntry //cata
        {
            uint32 id;                      // 0
            char* name_internal;            // 1
            uint32 map_type;                // 2
            uint32 map_flags;               // 3
            uint32 unk4;                    // 4
            uint32 is_pvp_zone;             // 5
            char* map_name;                 // 6
            uint32 linked_zone;             // 7 common zone for instance and continent map
            char* horde_intro;              // 8 horde text for PvP Zones
            char* alliance_intro;           // 9 alliance text for PvP Zones
            uint32 multimap_id;             // 10
            float battlefield_map_icon;     // 11
            int32 parent_map;               // 12 ghost map_id of parent map
            float start_x;                  // 13 ghost enter x coordinate (if exist single entry)
            float start_y;                  // 14 ghost enter y coordinate (if exist single entry)
            uint32 dayTime_override;        // 15
            uint32 addon;                   // 16 0-original maps, 1-tbc addon, 2-wotlk addon
            uint32 unk_time;                // 17
            uint32 max_players;             // 18
            uint32 next_phase_map;          // 19
        };

        struct NameGenEntry
        {
            uint32 ID;              // 0
            char* Name;             // 1
            uint32 unk1;            // 2
            uint32 type;            // 3
        };

        struct PhaseEntry
        {
            uint32 Id;              // 0
            uint32 PhaseShift;      // 1
            uint32 Flags;           // 2
        };

        struct QuestSortEntry
        {
            uint32 id;              // 0
            //char* name;           // 1
        };

        struct QuestXP  //cata
        {
            uint32 questLevel;     // 0
            uint32 xpIndex[10];    // 1-10
        };

        struct ScalingStatDistributionEntry //cata
        {
            uint32 id;                  // 0
            int32 stat[10];             // 1-10
            uint32 statmodifier[10];    // 11-20
            //uint32 unk;               // 21
            uint32 maxlevel;            // 22
        };

        struct ScalingStatValuesEntry   //cata
        {
            uint32 id;                  // 0
            uint32 level;               // 1
            uint32 multiplier[20];      // 
            //\todo danko Rewrite GetStatScalingStatValueColumn!
            //uint32 dpsMod[6];           // 2-7
            //uint32 spellBonus;          // 8
            //uint32 scalingstatmultipl[5]; // 9-13
            //uint32 amor_mod[4];         // 14-17
            //uint32 amor_mod2[4];        // 18-21
            ////uint32 junk[24];          // 22-45
            ////uint32 unk2;              // 46
        };

        struct SkillLineEntry   //cata
        {
            uint32 id;                  // 0
            uint32 type;                // 1
            char* Name;                 // 2
            //char* Description;        // 3
            uint32 spell_icon;          // 4
            //char* add_name;           // 5
            uint32 linkable;            // 6
        };

        struct SkillLineAbilityEntry    //cata
        {
            uint32 Id;                      // 0
            uint32 skilline;                // 1 skill id
            uint32 spell;                   // 2
            uint32 race_mask;               // 3
            uint32 class_mask;              // 4
            //uint32 excludeRace;           // 5
            //uint32 excludeClass;          // 6
            uint32 minSkillLineRank;        // 7 req skill value
            uint32 next;                    // 8
            uint32 acquireMethod;           // 9 auto learn
            uint32 grey;                    // 10 max
            uint32 green;                   // 11 min
            uint32 characterPoints;         // 12
            //uint32 unk;                   // 13
        };

        //\todo danko
        //struct StableSlotPrices
        //{
        //    uint32 Id;              // 0
        //    uint32 Price;           // 1
        //};

        // SpellAuraOptions.dbc
        struct SpellAuraOptionsEntry
        {
            //uint32    Id;                                         // 0       m_ID
            uint32    StackAmount;                                  // 1       m_cumulativeAura
            uint32    procChance;                                   // 2       m_procChance
            uint32    procCharges;                                  // 3       m_procCharges
            uint32    procFlags;                                    // 4       m_procTypeMask
        };

        // SpellAuraRestrictions.dbc
        struct SpellAuraRestrictionsEntry
        {
            //uint32    Id;                                         // 0        m_ID
            uint32    CasterAuraState;                              // 1        m_casterAuraState
            uint32    TargetAuraState;                              // 2        m_targetAuraState
            uint32    CasterAuraStateNot;                           // 3        m_excludeCasterAuraState
            uint32    TargetAuraStateNot;                           // 4        m_excludeTargetAuraState
            uint32    casterAuraSpell;                              // 5        m_casterAuraSpell
            uint32    targetAuraSpell;                              // 6        m_targetAuraSpell
            uint32    excludeCasterAuraSpell;                       // 7        m_excludeCasterAuraSpell
            uint32    excludeTargetAuraSpell;                       // 8        m_excludeTargetAuraSpell
        };

        // SpellCastingRequirements.dbc
        struct SpellCastingRequirementsEntry
        {
            //uint32    Id;                                         // 0        m_ID
            uint32    FacingCasterFlags;                            // 1        m_facingCasterFlags
            //uint32    MinFactionId;                               // 2        m_minFactionID not used
            //uint32    MinReputation;                              // 3        m_minReputation not used
            int32     AreaGroupId;                                  // 4        m_requiredAreaGroupId
            //uint32    RequiredAuraVision;                         // 5        m_requiredAuraVision not used
            uint32    RequiresSpellFocus;                           // 6        m_requiresSpellFocus
        };

        // SpellCastTimes.dbc
        struct SpellCastTimesEntry  //cata
        {
            uint32 ID;              // 0
            uint32 CastTime;        // 1
            float CastTimePerLevel; // 2
            int32 MinCastTime;      // 3
        };

        // SpellCategories.dbc
        struct SpellCategoriesEntry
        {
            //uint32    Id;                                         // 0        m_ID
            uint32    Category;                                     // 1        m_category
            uint32    DmgClass;                                     // 2        m_defenseType
            uint32    Dispel;                                       // 3        m_dispelType
            uint32    Mechanic;                                     // 4        m_mechanic
            uint32    PreventionType;                               // 5        m_preventionType
            uint32    StartRecoveryCategory;                        // 6        m_startRecoveryCategory
        };

        struct ClassFamilyMask
        {
            uint64 Flags;
            uint32 Flags2;

            ClassFamilyMask() : Flags(0), Flags2(0) {}
            explicit ClassFamilyMask(uint64 familyFlags, uint32 familyFlags2 = 0) : Flags(familyFlags), Flags2(familyFlags2) {}

            bool Empty() const { return Flags == 0 && Flags2 == 0; }
            bool operator! () const { return Empty(); }
            operator void const* () const { return Empty() ? nullptr : this; }// for allow normal use in if(mask)

            bool IsFitToFamilyMask(uint64 familyFlags, uint32 familyFlags2 = 0) const
            {
                return (Flags & familyFlags) || (Flags2 & familyFlags2);
            }

            bool IsFitToFamilyMask(ClassFamilyMask const& mask) const
            {
                return (Flags & mask.Flags) || (Flags2 & mask.Flags2);
            }

            uint64 operator& (uint64 mask) const                     // possible will removed at finish convertion code use IsFitToFamilyMask
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
            //uint32    Id;                                         // 0        m_ID
            //uint32    modalNextSpell;                             // 1        m_modalNextSpell not used
            uint32 SpellFamilyFlags[3];                       // 2-4      m_spellClassMask NOTE: size is 12 bytes!!!
            uint32 SpellFamilyName;                              // 5        m_spellClassSet
                                                                    //char*   Description;                                  // 6 4.0.0
                                                                    // helpers

            bool IsFitToFamilyMask(uint64 familyFlags, uint32 familyFlags2 = 0) const
            {
                return true; // SpellFamilyFlags.IsFitToFamilyMask(familyFlags, familyFlags2);
            }

            bool IsFitToFamily(SpellFamily family, uint64 familyFlags, uint32 familyFlags2 = 0) const
            {
                return SpellFamily(SpellFamilyName) == family && IsFitToFamilyMask(familyFlags, familyFlags2);
            }

            bool IsFitToFamilyMask(ClassFamilyMask const& mask) const
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
            //uint32    Id;                                         // 0        m_ID
            uint32    CategoryRecoveryTime;                         // 1        m_categoryRecoveryTime
            uint32    RecoveryTime;                                 // 2        m_recoveryTime
            uint32    StartRecoveryTime;                            // 3        m_startRecoveryTime
        };

        // SpellDifficulty.dbc
        struct SpellDifficultyEntry //cata
        {
            uint32 ID;                          // 0
            int32 SpellId[MAX_DIFFICULTY];      // 1-4 (instance modes)
        };

        // SpellDuration.dbc
        struct SpellDurationEntry   //cata
        {
            uint32 ID;              // 0
            uint32 Duration1;       // 1
            uint32 Duration2;       // 2
            uint32 Duration3;       // 3
        };

        // SpellEffect.dbc
        struct SpellEffectEntry
        {
            //uint32    Id;                                         // 0        m_ID
            uint32    Effect;                                       // 1        m_effect
            float     EffectMultipleValue;                          // 2        m_effectAmplitude
            uint32    EffectApplyAuraName;                          // 3        m_effectAura
            uint32    EffectAmplitude;                              // 4        m_effectAuraPeriod
            int32     EffectBasePoints;                             // 5        m_effectBasePoints (don't must be used in spell/auras explicitly, must be used cached Spell::m_currentBasePoints)
            float     EffectBonusMultiplier;                        // 6        m_effectBonus
            float     EffectDamageMultiplier;                       // 7        m_effectChainAmplitude
            uint32    EffectChainTarget;                            // 8        m_effectChainTargets
            int32     EffectDieSides;                               // 9        m_effectDieSides
            uint32    EffectItemType;                               // 10       m_effectItemType
            uint32    EffectMechanic;                               // 11       m_effectMechanic
            int32     EffectMiscValue;                              // 12       m_effectMiscValue
            int32     EffectMiscValueB;                             // 13       m_effectMiscValueB
            float     EffectPointsPerComboPoint;                    // 14       m_effectPointsPerCombo
            uint32    EffectRadiusIndex;                            // 15       m_effectRadiusIndex - spellradius.dbc
            uint32    EffectRadiusMaxIndex;                         // 16       4.0.0
            float     EffectRealPointsPerLevel;                     // 17       m_effectRealPointsPerLevel
            uint32 EffectSpellClassMask[3];
            //ClassFamilyMask EffectSpellClassMask;                   // 18 19 20 m_effectSpellClassMask
            uint32    EffectTriggerSpell;                           // 21       m_effectTriggerSpell
            uint32    EffectImplicitTargetA;                        // 22       m_implicitTargetA
            uint32    EffectImplicitTargetB;                        // 23       m_implicitTargetB
            uint32    EffectSpellId;                                // 24       m_spellId - spell.dbc
            uint32    EffectIndex;                                  // 25       m_spellEffectIdx
            //uint32 unk;                                           // 26       4.2.0 only 0 or 1

            // helpers
            int32 CalculateSimpleValue() const { return EffectBasePoints; }

            uint32 GetRadiusIndex() const
            {
                if (EffectRadiusIndex != 0)
                    return EffectRadiusIndex;

                return EffectRadiusMaxIndex;
            }
        };

        // SpellEquippedItems.dbc
        struct SpellEquippedItemsEntry
        {
            //uint32    Id;                                         // 0        m_ID
            int32     EquippedItemClass;                            // 1        m_equippedItemClass (value)
            int32     EquippedItemInventoryTypeMask;                // 2        m_equippedItemInvTypes (mask)
            int32     EquippedItemSubClassMask;                     // 3        m_equippedItemSubclass (mask)
        };

        // SpellFocusObject.dbc
        struct SpellFocusObjectEntry
        {
            uint32    ID;                                           // 0        m_ID
                                                                    //char*     Name;                                       // 1        m_name_lang
        };

        // SpellInterrupts.dbc
        struct SpellInterruptsEntry
        {
            //uint32    Id;                                         // 0        m_ID
            uint32    AuraInterruptFlags;                           // 1        m_auraInterruptFlags
                                                                    //uint32                                                // 2        4.0.0
            uint32    ChannelInterruptFlags;                        // 3        m_channelInterruptFlags
                                                                    //uint32                                                // 4        4.0.0
            uint32    InterruptFlags;                               // 5        m_interruptFlags
        };

        // SpellItemEnchantment.dbc
        struct SpellItemEnchantmentEntry    //cata
        {
            uint32 Id;                  // 0
            //uint32 charges;           // 1
            uint32 type[3];             // 2-4
            uint32 min[3];              // 5-7 for combat, in practice min==max
            //uint32 max[3];            // 8-10
            uint32 spell[3];            // 11-13
            char* Name;                 // 14-29
            //uint32 NameFlags;         // 30
            uint32 visual;              // 31 aura
            uint32 EnchantGroups;       // 32 slot
            uint32 GemEntry;            // 33
            uint32 ench_condition;      // 34
            uint32 req_skill;           // 35
            uint32 req_skill_value;     // 36
            uint32 req_level;           // 37
        };

        // SpellItemEnchantmentCondition.dbc
        struct SpellItemEnchantmentConditionEntry
        {
            uint32  ID;                                             // 0        m_ID
            uint8   Color[5];                                       // 1-5      m_lt_operandType[5]
                                                                    //uint32  LT_Operand[5];                                // 6-10     m_lt_operand[5]
            uint8   Comparator[5];                                  // 11-15    m_operator[5]
            uint8   CompareColor[5];                                // 15-20    m_rt_operandType[5]
            uint32  Value[5];                                       // 21-25    m_rt_operand[5]
                                                                    //uint8   Logic[5]                                      // 25-30    m_logic[5]
        };

        // SpellLevels.dbc
        struct SpellLevelsEntry
        {
            //uint32    Id;                                         // 0        m_ID
            uint32    baseLevel;                                    // 1        m_baseLevel
            uint32    maxLevel;                                     // 2        m_maxLevel
            uint32    spellLevel;                                   // 3        m_spellLevel
        };

        // SpellPower.dbc
        struct SpellPowerEntry
        {
            //uint32    Id;                                         // 0 - m_ID
            uint32    manaCost;                                     // 1 - m_manaCost
            uint32    manaCostPerlevel;                             // 2 - m_manaCostPerLevel
            uint32    ManaCostPercentage;                           // 3 - m_manaCostPct
            uint32    manaPerSecond;                                // 4 - m_manaPerSecond
            uint32    manaPerSecondPerLevel;                        // 5   m_manaPerSecondPerLevel
                                                                    //uint32  PowerDisplayId;                               // 6 - m_powerDisplayID - id from PowerDisplay.dbc, new in 3.1
            float     ManaCostPercentageFloat;                      // 7   4.3.0
        };

        // SpellRadius.dbc
        struct SpellRadiusEntry //cata
        {
            uint32 ID;                  // 0
            float radius_min;           // 1 Radius
            float radius_per_level;     // 2
            float radius_max;           // 3 Radius2
        };

        // SpellRange.dbc
        struct SpellRangeEntry  //cata
        {
            uint32 ID;                  // 0
            float minRange;             // 1
            float minRangeFriendly;     // 2
            float maxRange;             // 3
            float maxRangeFriendly;     // 4
            uint32 range_type;          // 5
            //char* name1[16]           // 6-21
            //uint32 name1_falgs;       // 22
            //char* name2[16]           // 23-38
            //uint32 name2_falgs;       // 39
        };

        // SpellReagents.dbc
        struct SpellReagentsEntry
        {
            //uint32    Id;                                         // 0        m_ID
            int32     Reagent[MAX_SPELL_REAGENTS];                  // 54-61    m_reagent
            uint32    ReagentCount[MAX_SPELL_REAGENTS];             // 62-69    m_reagentCount
        };

        // SpellRuneCost.dbc
        struct SpellRuneCostEntry   //cata
        {
            uint32 ID;              // 0
            uint32 bloodRuneCost;   // 1
            uint32 frostRuneCost;   // 2
            uint32 unholyRuneCost;  // 3
            uint32 runePowerGain;   // 4
        };

        // SpellScaling.dbc
        struct SpellScalingEntry
        {
            //uint32    Id;                                         // 0        m_ID
            uint32    castTimeMin;                                  // 1
            uint32    castTimeMax;                                  // 2
            uint32    castScalingMaxLevel;                          // 3
            uint32    playerClass;                                  // 4        (index * 100) + charLevel => gtSpellScaling.dbc
            float     coeff1[3];                                    // 5-7
            float     coeff2[3];                                    // 8-10
            float     coeff3[3];                                    // 11-13
            float     coefBase;                                     // 14       some coefficient, mostly 1.0f
            uint32    coefLevelBase;                                // 15       some level

            bool IsScalableEffect(SpellEffectIndex i) const { return coeff1[i] != 0.0f; };
        };

        // SpellShapeshift.dbc
        struct SpellShapeshiftEntry
        {
            //uint32    Id;                                         // 0        m_ID
            uint32    StancesNot;                                   // 1        m_shapeshiftMask
                                                                    // uint32 unk_320_2;                                    // 2        3.2.0
            uint32    Stances;                                      // 3        m_shapeshiftExclude
                                                                    // uint32 unk_320_3;                                    // 4        3.2.0
                                                                    // uint32    StanceBarOrder;                            // 5        m_stanceBarOrder not used
        };

        // SpellTargetRestrictions.dbc
        struct SpellTargetRestrictionsEntry
        {
            //uint32    Id;                                         // 0        m_ID
            float     MaxTargetRadius;                              // 1 - m_maxTargetRadius
            uint32    MaxAffectedTargets;                           // 1 - m_maxTargets
            uint32    MaxTargetLevel;                               // 2 - m_maxTargetLevel
            uint32    TargetCreatureType;                           // 3 - m_targetCreatureType
            uint32    Targets;                                      // 4 - m_targets
        };

        // SpellTotems.dbc
        struct SpellTotemsEntry
        {
            //uint32    Id;                                         // 0        m_ID
            uint32    TotemCategory[MAX_SPELL_TOTEM_CATEGORIES];    // 1 2      m_requiredTotemCategoryID
            uint32    Totem[MAX_SPELL_TOTEMS];                      // 3 4      m_totem
        };

        struct SERVER_DECL SpellEntry
        {
            uint32 Id;                                          // 0
            uint32 Attributes;                                  // 1
            uint32 AttributesEx;                                // 2
            uint32 AttributesExB;                               // 3
            uint32 AttributesExC;                               // 4
            uint32 AttributesExD;                               // 5
            uint32 AttributesExE;                               // 6
            uint32 AttributesExF;                               // 7
            uint32 AttributesExG;                               // 8
            uint32 AttributesExH;                               // 9
            uint32 AttributesExI;                               // 10
            uint32 AttributesExJ;                               // 11
            uint32 CastingTimeIndex;                            // 12
            uint32 DurationIndex;                               // 13
            uint32 powerType;                                   // 14
            uint32 rangeIndex;                                  // 15
            float speed;                                        // 16
            uint32 SpellVisual[2];                              // 17-18
            uint32 spellIconID;                                 // 19
            uint32 activeIconID;                                // 20
            char* Name;                                         // 21
            char* Rank;                                         // 22
            char* Description;                                  // 23
            char* BuffDescription;                              // 24
            uint32 School;                                      // 25
            uint32 RuneCostID;                                  // 26
            //uint32 spellMissileID;                            // 27
            //uint32 spellDescriptionVariableID;                // 28
            uint32 SpellDifficultyId;                           // 29
            //float unk_1;                                      // 30
            uint32 SpellScalingId;                              // 31 SpellScaling.dbc
            uint32 SpellAuraOptionsId;                          // 32 SpellAuraOptions.dbc
            uint32 SpellAuraRestrictionsId;                     // 33 SpellAuraRestrictions.dbc
            uint32 SpellCastingRequirementsId;                  // 34 SpellCastingRequirements.dbc
            uint32 SpellCategoriesId;                           // 35 SpellCategories.dbc
            uint32 SpellClassOptionsId;                         // 36 SpellClassOptions.dbc
            uint32 SpellCooldownsId;                            // 37 SpellCooldowns.dbc
            //uint32 unk_2;                                     // 38 all zeros...
            uint32 SpellEquippedItemsId;                        // 39 SpellEquippedItems.dbc
            uint32 SpellInterruptsId;                           // 40 SpellInterrupts.dbc
            uint32 SpellLevelsId;                               // 41 SpellLevels.dbc
            uint32 SpellPowerId;                                // 42 SpellPower.dbc
            uint32 SpellReagentsId;                             // 43 SpellReagents.dbc
            uint32 SpellShapeshiftId;                           // 44 SpellShapeshift.dbc
            uint32 SpellTargetRestrictionsId;                   // 45 SpellTargetRestrictions.dbc
            uint32 SpellTotemsId;                               // 46 SpellTotems.dbc
            //uint32 ResearchProject;                           // 47 ResearchProject.dbc

            // helpers
            int32 CalculateSimpleValue(SpellEffectIndex eff) const;
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
            uint32 GetManaCost() const;
            uint32 GetPreventionType() const;
            uint32 GetCategory() const;
            uint32 GetStartRecoveryTime() const;
            uint32 GetMechanic() const;
            uint32 GetRecoveryTime() const;
            uint32 GetCategoryRecoveryTime() const;
            uint32 GetStartRecoveryCategory() const;
            uint32 GetSpellLevel() const;
            int32 GetEquippedItemClass() const;
            SpellFamily GetSpellFamilyName() const;
            uint32 GetDmgClass() const;
            uint32 GetDispel() const;
            uint32 GetMaxAffectedTargets() const;
            uint32 GetStackAmount() const;
            uint32 GetManaCostPercentage() const;
            uint32 GetProcCharges() const;
            uint32 GetProcChance() const;
            uint32 GetMaxLevel() const;
            uint32 GetTargetAuraState() const;
            uint32 GetManaPerSecond() const;
            uint32 GetRequiresSpellFocus() const;
            uint32 GetSpellEffectIdByIndex(SpellEffectIndex index) const;
            uint32 GetAuraInterruptFlags() const;
            uint32 GetEffectImplicitTargetAByIndex(SpellEffectIndex index) const;
            int32 GetAreaGroupId() const;
            uint32 GetFacingCasterFlags() const;
            uint32 GetBaseLevel() const;
            uint32 GetInterruptFlags() const;
            uint32 GetTargetCreatureType() const;
            int32 GetEffectMiscValue(SpellEffectIndex index) const;
            uint32 GetStances() const;
            uint32 GetStancesNot() const;
            uint32 GetProcFlags() const;
            uint32 GetChannelInterruptFlags() const;
            uint32 GetManaCostPerLevel() const;
            uint32 GetCasterAuraState() const;
            uint32 GetTargets() const;
            uint32 GetEffectApplyAuraNameByIndex(SpellEffectIndex index) const;

            bool IsFitToFamilyMask(uint64 familyFlags, uint32 familyFlags2 = 0) const
            {
                SpellClassOptionsEntry const* classOpt = GetSpellClassOptions();
                return classOpt && classOpt->IsFitToFamilyMask(familyFlags, familyFlags2);
            }

            bool IsFitToFamily(SpellFamily family, uint64 familyFlags, uint32 familyFlags2 = 0) const
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
            // prevent creating custom entries (copy data from original in fact)
            SpellEntry(SpellEntry const&);                      // DON'T must have implementation

                                                                // catch wrong uses
            template<typename T>
            bool IsFitToFamilyMask(SpellFamily family, T t) const;
        };

        struct SpellShapeshiftFormEntry //cata
        {
            uint32 id;                  // 0
            //uint32 button_pos;        // 1
            //char* name[16];           // 2-17
            //uint32 name_flags;        // 18
            uint32 Flags;               // 19
            uint32 unit_type;           // 20
            //uint32 unk1               // 21
            uint32 AttackSpeed;         // 22
            uint32 modelId;             // 23 alliance?
            uint32 modelId2;            // 24 horde?
            //uint32 unk2               // 25
            //uint32 unk3               // 26
            uint32 spells[8];           // 27-34
        };

        struct SummonPropertiesEntry    //cata
        {
            uint32 ID;                  // 0
            uint32 ControlType;         // 1
            uint32 FactionID;           // 2
            uint32 Type;                // 3
            uint32 Slot;                // 4
            uint32 Flags;               // 5
        };

        struct TalentEntry  //cata
        {
            uint32 TalentID;            // 0
            uint32 TalentTree;          // 1
            uint32 Row;                 // 2
            uint32 Col;                 // 3
            uint32 RankID[5];           // 4-8
            //uint32 unk[4];            // 9-12
            uint32 DependsOn;           // 13
            //uint32 unk1[2];           // 14-15
            uint32 DependsOnRank;       // 16
            //uint32 unk2[2];           // 17-18
            //uint32 unk3;              // 19
            //uint32 unk4;              // 20
            //uint32 unk5;              // 21
        };

        struct TalentTabEntry   //cata
        {
            uint32 TalentTabID;         // 0
            //char* Name;               // 1
            //uint32 unk5;              // 2
            uint32 ClassMask;           // 3
            uint32 PetTalentMask;       // 4
            uint32 TabPage;             // 5
            //char* InternalName;       // 6
            //char* description;        // 7
            uint32 rolesMask;           // 8
            uint32 masterySpells[2];   // 9-10
        };

        struct TalentTreePrimarySpells
        {
            uint32 ID;                  // 0
            uint32 tabID;               // 1
            uint32 SpellID;             // 2
            //unk                       // 3
        };

        struct TaxiNodesEntry   //cata
        {
            uint32 id;                  // 0
            uint32 mapid;               // 1
            float x;                    // 2
            float y;                    // 3
            float z;                    // 4
            char* name;                 // 5
            uint32 horde_mount;         // 6
            uint32 alliance_mount;      // 7
        };

        struct TaxiPathEntry    //cata
        {
            uint32 id;                  // 0
            uint32 from;                // 1
            uint32 to;                  // 2
            uint32 price;               // 3
        };

        struct TaxiPathNodeEntry    //cata
        {
            //uint32 id;                // 0
            uint32 path;                // 1
            uint32 seq;                 // 2 nodeIndex
            uint32 mapid;               // 3
            float x;                    // 4
            float y;                    // 5
            float z;                    // 6
            uint32 flags;               // 7
            uint32 waittime;            // 8
            uint32 arivalEventID;       // 9
            uint32 departureEventID;    // 10
        };

        #define MAX_VEHICLE_SEATS 8

        struct VehicleEntry //cata
        {
            uint32 ID;                                          // 0
            uint32 flags;                                       // 1
            float turnSpeed;                                    // 2
            float pitchSpeed;                                   // 3
            float pitchMin;                                     // 4
            float pitchMax;                                     // 5
            uint32 seatID[MAX_VEHICLE_SEATS];                   // 6-13
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
            uint32 uiLocomotionType;                            // 34
            float msslTrgtImpactTexRadius;                      // 35
            uint32 uiSeatIndicatorType;                         // 36
            uint32 powerType;                                   // 37, new in 3.1
            //uint32 unk1;                                      // 38
            //uint32 unk2;                                      // 39  
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

        struct VehicleSeatEntry //cata
        {
            uint32 ID;                                          // 0
            uint32 flags;                                       // 1
            int32 attachmentID;                                 // 2
            float attachmentOffsetX;                            // 3
            float attachmentOffsetY;                            // 4
            float attachmentOffsetZ;                            // 5
            float enterPreDelay;                                // 6
            float enterSpeed;                                   // 7
            float enterGravity;                                 // 8
            float enterMinDuration;                             // 9
            float enterMaxDuration;                             // 10
            float enterMinArcHeight;                            // 11
            float enterMaxArcHeight;                            // 12
            int32 enterAnimStart;                               // 13
            int32 enterAnimLoop;                                // 14
            int32 rideAnimStart;                                // 15
            int32 rideAnimLoop;                                 // 16
            int32 rideUpperAnimStart;                           // 17
            int32 rideUpperAnimLoop;                            // 18
            float exitPreDelay;                                 // 19
            float exitSpeed;                                    // 20
            float exitGravity;                                  // 21
            float exitMinDuration;                              // 22
            float exitMaxDuration;                              // 23
            float exitMinArcHeight;                             // 24
            float exitMaxArcHeight;                             // 25
            int32 exitAnimStart;                                // 26
            int32 exitAnimLoop;                                 // 27
            int32 exitAnimEnd;                                  // 28
            float passengerYaw;                                 // 29
            float passengerPitch;                               // 30
            float passengerRoll;                                // 31
            int32 passengerAttachmentID;                        // 32
            int32 vehicleEnterAnim;                             // 33
            int32 vehicleExitAnim;                              // 34
            int32 vehicleRideAnimLoop;                          // 35
            int32 vehicleEnterAnimBone;                         // 36
            int32 vehicleExitAnimBone;                          // 37
            int32 vehicleRideAnimLoopBone;                      // 38
            float vehicleEnterAnimDelay;                        // 39
            float vehicleExitAnimDelay;                         // 40
            uint32 vehicleAbilityDisplay;                       // 41
            uint32 enterUISoundID;                              // 42
            uint32 exitUISoundID;                               // 43
            int32 uiSkin;                                       // 44
            uint32 flagsB;                                      // 45

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

        struct WMOAreaTableEntry    //cata
        {
            uint32 id;              // 0
            int32 rootId;           // 1
            int32 adtId;            // 2
            int32 groupId;          // 3
            //uint32 field4;        // 4
            //uint32 field5;        // 5
            //uint32 field6;        // 6
            //uint32 field7;        // 7
            //uint32 field8;        // 8
            uint32 flags;           // 9
            uint32 areaId;          // 10  ref -> AreaTableEntry
            //char Name[16];        // 11-26
            //uint32 nameflags;     // 27
        };

        struct WorldMapOverlayEntry //cata
        {
            uint32 ID;              // 0
            //uint32 worldMapID;    // 1
            uint32 areaID;          // 2 - index to AreaTable
            uint32 areaID_2;        // 3 - index to AreaTable
            uint32 areaID_3;        // 4 - index to AreaTable
            uint32 areaID_4;        // 5 - index to AreaTable
            //uint32 unk1[2];       // 6-7
            //uint32 unk2;          // 8
            //uint32 unk3[7];       // 9-16
        };

        #pragma pack(pop)

        typedef std::set<uint32> SpellCategorySet;
        typedef std::map<uint32, SpellCategorySet> SpellCategoryStore;
        struct SpellEffect
        {
            SpellEffect()
            {
                effects[0] = NULL;
                effects[1] = NULL;
                effects[2] = NULL;
            }
            SpellEffectEntry const* effects[3];
        };
        typedef std::map<uint32, SpellEffect> SpellEffectMap;
    }
}

#endif // _DBC_STRUCTURES_HPP
