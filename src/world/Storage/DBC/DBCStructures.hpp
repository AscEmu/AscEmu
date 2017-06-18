/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldConf.h"

#if VERSION_STRING == Cata
    #include "../world/GameCata/Storage/DBCStructures.h"
#else
#include "Common.hpp"
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

namespace DBC
{
    namespace Structures
    {
        namespace
        {
            char const achievement_format[] = "niixssssssssssssssssxssssssssssssssssxiixixssssssssssssssssxii";
            char const achievement_criteria_format[] = "niiiiiiiissssssssssssssssxiiiii";
            char const area_group_format[] = "niiiiiii";
            char const area_table_entry_format[] = "iiinixxxxxissssssssssssssssxiiiiixxx";
            char const area_trigger_entry_format[] = "niffffffff";
            char const auction_house_format[] = "niiixxxxxxxxxxxxxxxxx";
            char const bank_bag_slot_prices_format[] = "ni";
            char const barber_shop_style_entry_format[] = "nixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiii";
            char const char_titles_format[] = "nxssssssssssssssssxssssssssssssssssxi";
            char const chat_channels_format[] = "nixssssssssssssssssxxxxxxxxxxxxxxxxxx";
            char const chr_classes_format[] = "nxixssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxixii";
            char const chr_races_format[] = "niixiixixxxxixssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxi";
            char const creature_display_info_format[] = "nxxxxxxxxxxxxxxx";
            char const creature_family_format[] = "nfifiiiiixssssssssssssssssxx";
            char const creature_spell_data_format[] = "niiiiiiii";
            char const currency_types_format[] = "xnxi";
            char const durability_costs_format[] = "niiiiiiiiiiiiiiiiiiiiiiiiiiiii";
            char const durability_quality_format[] = "nf";
            char const emotes_text_format[] = "nxiiiixixixxxxxxxxx";
            char const faction_format[] = "niiiiiiiiiiiiiiiiiiffixssssssssssssssssxxxxxxxxxxxxxxxxxx";
            char const faction_template_format[] = "niiiiiiiiiiiii";
            char const game_object_display_info_format[] = "nsxxxxxxxxxxffffffx";
            char const gem_properties_format[] = "nixxi";
            char const glyph_properties_format[] = "niii";
            char const glyph_slot_format[] = "nii";
            char const gt_barber_shop_cost_format[] = "f";
            char const gt_chance_to_melee_crit_format[] = "f";
            char const gt_chance_to_melee_crit_base_format[] = "f";
            char const gt_chance_to_spell_crit_format[] = "f";
            char const gt_chance_to_spell_crit_base_format[] = "f";
            char const gt_combat_ratings_format[] = "f";
            char const gt_oct_regen_hp_format[] = "f";
            char const gt_oct_regen_mp_format[] = "f";
            char const gt_regen_hp_per_spt_format[] = "f";
            char const gt_regen_mp_per_spt_format[] = "f";
            char const holidays_format[] = "niiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiixxsiix";
            char const item_entry_format[] = "niiiiiii";
            char const item_extended_cost_format[] = "niiiiiiiiiiiiiix";
            char const item_limit_category_format[] = "nxxxxxxxxxxxxxxxxxii";
            char const item_random_properties_format[] = "nxiiixxssssssssssssssssx";
            char const item_random_suffix_format[] = "nssssssssssssssssxxiiixxiiixx";
            char const item_set_format[] = "nssssssssssssssssxiiiiiiiiiixxxxxxxiiiiiiiiiiiiiiiiii";
            char const lfg_dungeon_entry_format[] = "nssssssssssssssssxiiiiiiiiixxixixxxxxxxxxxxxxxxxx";
            char const liquid_type_entry_format[] = "nxxixixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
            char const lock_format[] = "niiiiiiiiiiiiiiiiiiiiiiiixxxxxxxx";
            char const mail_template_format[] = "nsxxxxxxxxxxxxxxxxsxxxxxxxxxxxxxxxx";
            char const map_format[] = "nxiixssssssssssssssssxixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxixiffxiii";
            char const name_gen_format[] = "nsii";
            char const quest_xp_format[] = "niiiiiiiiii";
            char const scaling_stat_distribution_format[] = "niiiiiiiiiiiiiiiiiiiii";
            char const scaling_stat_values_format[] = "iniiiiiiiiiiiiiiiiiiiiii";
            char const skill_line_format[] = "nixssssssssssssssssxxxxxxxxxxxxxxxxxxixxxxxxxxxxxxxxxxxi";
            char const skill_line_ability_format[] = "niiiixxiiiiixx";
            char const stable_slot_prices_format[] = "ni";
            char const spell_cast_times_format[] = "nixx";
            char const spell_difficulty_format[] = "niiii";
            char const spell_duration_format[] = "niii";
            char const spell_entry_format[] = "niiiiiiiiiiiixixiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifiiiiiiiiiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiifffiiiiiiiiiiiiiisxxxxxxxxxxxxxxxxsxxxxxxxxxxxxxxxxsxxxxxxxxxxxxxxxxsxxxxxxxxxxxxxxxxiiiiiiiiiiiifffiiiiiiiixxxxxxi";
            char const spell_item_enchantment_format[] = "nxiiiiiiiiiiiissssssssssssssssxiiiiiii";
            char const spell_radius_format[] = "nfff";
            char const spell_range_format[] = "nffffixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
            char const spell_rune_cost_format[] = "niiii";
            char const spell_shapeshift_form_format[] = "nxxxxxxxxxxxxxxxxxxiixiiixxiiiiiiii";
            char const summon_properties_format[] = "niiiii";
            char const talent_format[] = "niiiiiiiixxxxixxixxxxxx";
            char const talent_tab_format[] = "nxxxxxxxxxxxxxxxxxxxiiix";
            char const taxi_nodes_format[] = "nifffssssssssssssssssxii";
            char const taxi_path_format[] = "niii";
            char const taxi_path_node_format[] = "niiifffiiii";
            char const vehicle_format[] = "niffffiiiiiiiifffffffffffffffssssfifiixx";
            char const vehicle_seat_format[] = "niiffffffffffiiiiiifffffffiiifffiiiiiiiffiiiiixxxxxxxxxxxx";
            char const wmo_area_table_format[] = "niiixxxxxiixxxxxxxxxxxxxxxxx";
            char const world_map_overlay_format[] = "nxiiiixxxxxxxxxxx";
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
            char* name[16];                 // 9-24
                                            //uint32 name_flags;            // 25
            uint32 completionFlag;          // 26
            uint32 groupFlag;               // 27
            uint32 unk1;                    // 28
            uint32 timeLimit;               // 29 time limit in seconds
            uint32 index;                   // 30
        };

        struct AchievementEntry
        {
            uint32 ID;                      // 0
            int32 factionFlag;              // 1 -1=all, 0=horde, 1=alliance
            int32 mapID;                    // 2 -1=none
            //uint32 unknown1;              // 3
            char* name[16];                 // 4-19
            //uint32 name_flags;            // 20
            char* description[16];          // 21-36
            //uint32 desc_flags;            // 37
            uint32 categoryId;              // 38
            uint32 points;                  // 39 reward points
            //uint32 orderInCategory;       // 40
            uint32 flags;                   // 41
            //uint32 unknown2;              // 42
            char* rewardName[16];           // 43-58 title/item reward name
            //uint32 rewardName_flags;      // 59
            uint32 count;                   // 60
            uint32 refAchievement;          // 61
        };

        struct AreaGroupEntry
        {
            uint32 AreaGroupId;             // 0
            uint32 AreaId[6];               // 1-6
            uint32 next_group;              // 7
        };

        struct AreaTableEntry
        {
            uint32 id;                      // 0
            uint32 map_id;                  // 1
            uint32 zone;                    // 2 if 0 then it's zone, else it's zone id of this area
            uint32 explore_flag;            // 3, main index
            uint32 flags;                   // 4, unknown value but 312 for all cities
                                            // 5-9 unused
            int32 area_level;               // 10
            char* area_name[16];            // 11-26
                                            // 27, string flags, unused
            uint32 team;                    // 28
            uint32 liquid_type_override[4]; // 29-32 liquid override by type
        };

        struct AreaTriggerEntry
        {
            uint32 id;              // 0
            uint32 mapid;           // 1
            float x;                // 2
            float y;                // 3
            float z;                // 4
            float o;                // 5 radius?
            float box_x;            // 6 extent x edge
            float box_y;            // 7 extent y edge
            float box_z;            // 8 extent z edge
            float box_o;            // 9 extent rotation by about z axis
        };

        struct AuctionHouseEntry
        {
            uint32 id;              // 0
            uint32 faction;         // 1
            uint32 fee;             // 2
            uint32 tax;             // 3
            //char* name[16];       // 4-19
            //uint32 name_flags;    // 20
        };

        struct BankBagSlotPrices
        {
            uint32 Id;              // 0
            uint32 Price;           // 1
        };

        struct BarberShopStyleEntry
        {
            uint32 id;              // 0
            uint32 type;            // 1 value 0 -> hair, value 2 -> facialhair
            //char* name;           // 2 string hairstyle name
            //char* name[15];       // 3-17 name of hair style
            //uint32 name_flags;    // 18
            //uint32 unk_name[16];  // 19-34, all empty
            //uint32 unk_flags;     // 35
            //float unk3;           // 36 values 1 and 0,75
            uint32 race;            // 37 race
            uint32 gender;          // 38 0 male, 1 female
            uint32 hair_id;         // 39 Hair ID
        };

        struct CharTitlesEntry
        {
            uint32 ID;                      // 0, title ids
            //uint32 unk1;                  // 1 flags?
            char* name_male[16];            // 2-17
            //uint32 name_flag;             // 18 string flag, unused
            char* name_female[16];          // 19-34
            //const char* name2_flag;       // 35 string flag, unused
            uint32 bit_index;               // 36 used in PLAYER_CHOSEN_TITLE and 1<<index in PLAYER__FIELD_KNOWN_TITLES
        };

        struct ChatChannelsEntry
        {
            uint32 id;                      // 0
            uint32 flags;                   // 1
            char* name_pattern[16];         // 3-18
            //uint32 name_pattern_flags;    // 19
            //char* channel_name[16];       // 20-35
            //uint32 channel_name_flags;    // 36
        };

        struct ChrClassesEntry
        {
            uint32 class_id;                // 0
            //uint32 unk1;                  // 1
            uint32 power_type;              // 2
            //uint32 unk2[2];               // 3-4
            char* name[16];                 // 5-20
            //uint32 nameflags;             // 21
            //char* name_female[16];        // 22-36
            //uint32 name_female_flags;     // 37
            //char* name_neutral[16];       // 38-53
            //uint32 name_neutral_flags;    // 54
            //uint32 unk3;                  // 55
            uint32 spellfamily;             // 56
            //uint32 unk4;                  // 57
            uint32 cinematic_sequence;      // 58 CinematicSequences.dbc
            uint32 expansion;               // 59
        };

        struct ChrRacesEntry
        {
            uint32 race_id;                 // 0
            uint32 flags;                   // 1
            uint32 faction_id;              // 2
            //uint32 unk1;                  // 3
            uint32 model_male;              // 4
            uint32 model_female;            // 5
            // uint32 unk2;                 // 6
            uint32 team_id;                 // 7
            //uint32 unk3[4];               // 8-11
            uint32 cinematic_id;            // 12 CinematicSequences.dbc
            //uint32 unk4                   // 13
            char* name[16];                 // 14-29
            //uint32 name_flags             // 30
            //char* name_female[16];        // 31-46
            //uint32 name_female_flags      // 47
            //char* name_neutral[16];       // 48-63
            //uint32 name_neutral_flags     // 64 string flags, unused
            //uint32 unk5[3]                // 65-67 unused
            uint32 expansion;               // 68
        };

        struct CreatureDisplayInfoEntry
        {
            uint32_t display_id;            // 0
            //uint32_t model;               // 1
            //uint32_t unk0                 // 2
            //uint32_t InfoExtra;           // 3
            //float size;                   // 4
                                            // 5 - 15 unk
        };

        struct CreatureFamilyEntry
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
            char* name[16];                 // 10-25
            //uint32 nameflags;             // 26
            //uint32 iconFile;              // 27
        };

        struct CreatureSpellDataEntry
        {
            uint32 id;                      // 0
            uint32 Spells[3];               // 1-3
            uint32 PHSpell;                 // 4
            uint32 Cooldowns[3];            // 5-7
            uint32 PH;                      // 8
        };

        struct CurrencyTypesEntry
        {
            //uint32 ID;            // 0 not used
            uint32 item_id;         // 1 used as index
            //uint32 Category;      // 2 may be category
            uint32 bit_index;       // 3 bit index in PLAYER_FIELD_KNOWN_CURRENCIES (1 << (index-1))
        };

        struct DurabilityCostsEntry
        {
            uint32 itemlevel;       // 0
            uint32 modifier[29];    // 1-29
        };

        struct DurabilityQualityEntry
        {
            uint32 id;              // 0
            float quality_modifier; // 1
        };

        struct EmotesTextEntry
        {
            uint32 Id;              // 0
            //uint32 name;          // 1
            uint32 textid;          // 2
            uint32 textid2;         // 3
            uint32 textid3;         // 4
            uint32 textid4;         // 5
            //uint32 unk1;          // 6
            uint32 textid5;         // 7
            //uint32 unk2;          // 8
            uint32 textid6;         // 9
            //uint32 unk3;          // 10
            //uint32 unk4;          // 11
            //uint32 unk5;          // 12
            //uint32 unk6;          // 13
            //uint32 unk7;          // 14
            //uint32 unk8;          // 15
            //uint32 unk9;          // 16
            //uint32 unk10;         // 17
            //uint32 unk11;         // 18
        };

        struct FactionEntry
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
            char* Name[16];                 // 23-38
            //uint32 name_flags;            // 39
            //uint32 Description[16];       // 40-55
            //uint32 description_flags;     // 56
        };

        struct FactionTemplateEntry
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

        struct GameObjectDisplayInfoEntry
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
        };

        struct GemPropertiesEntry
        {
            uint32 Entry;                   // 0
            uint32 EnchantmentID;           // 1
            //uint32 unk1;                  // 2 bool
            //uint32 unk2;                  // 3 bool
            uint32 SocketMask;              // 4
        };

        struct GlyphPropertiesEntry
        {
            uint32 Entry;                   // 0
            uint32 SpellID;                 // 1
            uint32 Type;                    // 2 (0 = Major, 1 = Minor)
            uint32 unk;                     // 3 glyph_icon spell.dbc
        };

        struct GlyphSlotEntry
        {
            uint32 Id;              // 0
            uint32 Type;            // 1
            uint32 Slot;            // 2
        };

        struct GtBarberShopCostBaseEntry
        {
            float cost;             // 0 cost base
        };

        struct GtChanceToMeleeCritEntry
        {
            float val;              // 0
        };

        struct GtChanceToMeleeCritBaseEntry
        {
            float val;              // 0
        };

        struct GtChanceToSpellCritEntry
        {
            float val;              // 0
        };

        struct GtChanceToSpellCritBaseEntry
        {
            float val;              // 0
        };

        struct GtCombatRatingsEntry
        {
            float val;              // 0
        };

        struct GtOCTRegenHPEntry
        {
            float ratio;            // 0
        };

        struct GtOCTRegenMPEntry
        {
            float ratio;            // 0
        };

        struct GtRegenHPPerSptEntry
        {
            float ratio;            // 0 regen base
        };

        struct GtRegenMPPerSptEntry
        {
            float ratio;            // 0 regen base
        };

#define MAX_HOLIDAY_DURATIONS 10
#define MAX_HOLIDAY_DATES 26
#define MAX_HOLIDAY_FLAGS 10

        struct HolidaysEntry
        {
            uint32 Id;                                  // 0
            uint32 Duration[MAX_HOLIDAY_DURATIONS];     // 1-10
            uint32 Date[MAX_HOLIDAY_DATES];             // 11-36
            uint32 Region;                              // 37
            uint32 Looping;                             // 38
            uint32 CalendarFlags[MAX_HOLIDAY_FLAGS];    // 39-48
            //uint32 holidayNameId;                     // 49 HolidayNames.dbc
            //uint32 holidayDescriptionId;              // 50 HolidayDescriptions.dbc
            char* TextureFilename;                      // 51
            uint32 Priority;                            // 52
            uint32 CalendarFilterType;                  // 53
            //uint32 flags;                             // 54
        };

        struct ItemEntry
        {
            uint32 ID;                      // 0
            uint32 Class;                   // 1
            uint32 SubClass;                // 2 some items have strange subclasses
            int32 SoundOverrideSubclass;    // 3
            int32 Material;                 // 4
            uint32 DisplayId;               // 5
            uint32 InventoryType;           // 6
            uint32 Sheath;                  // 7
        };

        struct ItemExtendedCostEntry
        {
            uint32 costid;                  // 0
            uint32 honor_points;            // 1
            uint32 arena_points;            // 2
            uint32 arena_slot;              // 3
            uint32 item[5];                 // 4-8
            uint32 count[5];                // 9-13
            uint32 personalrating;          // 14
            //uint32 unk;                   // 15
        };

        struct ItemLimitCategoryEntry
        {
            uint32 Id;                      // 0
            //char* name[16];               // 1-16 name langs
            //uint32 name_flags             // 17
            uint32 maxAmount;               // 18
            uint32 equippedFlag;            // 19 - equipped (bool?)
        };

        struct ItemRandomPropertiesEntry
        {
            uint32 ID;                      // 0
            //uint32 name1;                 // 1
            uint32 spells[3];               // 2-4
            //uint32 unk1;                  // 5
            //uint32 unk2;                  // 6
            char* name_suffix[16];          // 7-22
            //uint32 name_suffix_flags;     // 23
        };

        struct ItemRandomSuffixEntry
        {
            uint32 id;                      // 0
            char* name_suffix[16];          // 1-16
            //uint32 name_suffix_flags;     // 17
            //uint32 unk1;                  // 18
            uint32 enchantments[3];         // 19-21
            //uint32 unk2[2];               // 22-23
            uint32 prefixes[3];             // 24-26
            //uint32[2];                    // 27-28
        };

        struct ItemSetEntry
        {
            uint32 id;                      // 1
            char* name[16];                 // 1-16 name (lang)
            //uint32 localeflag;            // 17 constant
            uint32 itemid[10];              // 18-27 item set items
            //uint32 unk[7];                // 28-34 all 0
            uint32 SpellID[8];              // 35-42
            uint32 itemscount[8];           // 43-50
            uint32 RequiredSkillID;         // 51
            uint32 RequiredSkillAmt;        // 52
        };

        struct LFGDungeonEntry
        {
            uint32 ID;              // 0
            char* name[16];         // 1-17 Name lang
            uint32 minlevel;        // 18
            uint32 maxlevel;        // 19
            uint32 reclevel;        // 20
            uint32 recminlevel;     // 21
            uint32 recmaxlevel;     // 22
            int32 map;              // 23
            uint32 difficulty;      // 24
            uint32 flags;           // 25
            uint32 type;            // 26
            //uint32  unk;          // 27
            //char* iconname;       // 28
            uint32 expansion;       // 29
            //uint32 unk4;          // 30
            uint32 grouptype;       // 31
            //char* desc[16];       // 32-47 Description

            // Helpers
            uint32 Entry() const { return ID + (type << 24); }
        };

        struct LiquidTypeEntry
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

        struct LockEntry
        {
            uint32 Id;                              // 0
            uint32 locktype[LOCK_NUM_CASES];        // 1-8 If this is 1, then the next lockmisc is an item ID, if it's 2, then it's an iRef to LockTypes.dbc.
            uint32 lockmisc[LOCK_NUM_CASES];        // 9-16 Item to unlock or iRef to LockTypes.dbc depending on the locktype.
            uint32 minlockskill[LOCK_NUM_CASES];    // 17-24 Required skill needed for lockmisc (if locktype = 2).
            //uint32 action[LOCK_NUM_CASES];        // 25-32 Something to do with direction / opening / closing.
        };

        struct MailTemplateEntry
        {
            uint32 ID;              // 0
            char* subject;          // 1
            //float unused1[15]     // 2-16
            //uint32 flags1         // 17 name flags, unused
            char* content;          // 18
            //float unused2[15]     // 19-34
            //uint32 flags2         // 35 name flags, unused
        };

        struct MapEntry
        {
            uint32 id;                      // 0
            //char* name_internal;          // 1
            uint32 map_type;                // 2
            uint32 map_flags;               // 3
            //uint32 is_pvp_zone;           // 4 -0 false -1 true
            char* map_name[16];             // 5-20
            //uint32 name_flags;            // 21
            uint32 linked_zone;             // 22 common zone for instance and continent map
            //char* horde_intro[16];        // 23-38 horde text for PvP Zones
            //uint32 hordeIntro_flags;      // 39
            //char* alliance_intro[16];     // 40-55 alliance text for PvP Zones
            //uint32 allianceIntro_flags;   // 56
            uint32 multimap_id;             // 57
            //uint32 battlefield_map_icon;  // 58
            int32 parent_map;               // 59 map_id of parent map
            float start_x;                  // 60 enter x coordinate (if exist single entry)
            float start_y;                  // 61 enter y coordinate (if exist single entry)
            //uint32 dayTime;               // 62 
            uint32 addon;                   // 63 0-original maps, 1-tbc addon, 2-wotlk addon
            uint32 unk_time;                // 64
            uint32 max_players;             // 65
        };

        struct NameGenEntry
        {
            uint32 ID;              // 0
            char* Name;             // 1
            uint32 unk1;            // 2
            uint32 type;            // 3
        };

        struct QuestXP
        {
            uint32 questLevel;     // 0
            uint32 xpIndex[10];    // 1-10
        };

        struct ScalingStatDistributionEntry
        {
            uint32 id;                  // 0
            int32 stat[10];             // 1-10
            uint32 statmodifier[10];    // 11-20
            uint32 maxlevel;            // 21
        };

        struct ScalingStatValuesEntry
        {
            uint32 id;                  // 0
            uint32 level;               // 1
            uint32 multiplier[16];      // 2-17 ///\todo split this
            uint32 unk1;                // 18
            uint32 amor_mod[5];         // 19-23
        };

        struct SkillLineEntry
        {
            uint32 id;                  // 0
            uint32 type;                // 1
            //uint32 skillCostsID;      // 2
            char* Name[16];             // 3-18
            //uint32 NameFlags;         // 19
            //char* Description[16];    // 20-35
            //uint32 DescriptionFlags;  // 36
            uint32 spell_icon;          // 37
            //char* add_name[16];       // 38-53
            //uint32 add_name_flags;    // 54
            uint32 linkable;            // 55
        };

        struct SkillLineAbilityEntry
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
            //uint32 abandonable;           // 12
            //uint32 reqTP;                 // 13
        };

        struct StableSlotPrices
        {
            uint32 Id;              // 0
            uint32 Price;           // 1
        };

        struct SpellCastTimesEntry
        {
            uint32 ID;              // 0
            uint32 CastTime;        // 1
            //uint32 unk1;          // 2
            //uint32 unk2;          // 3
        };

        struct SpellDifficultyEntry
        {
            uint32 ID;              // 0
            int32 SpellId[4];       // 1-4 (instance modes)
        };

        struct SpellDurationEntry
        {
            uint32 ID;              // 0
            uint32 Duration1;       // 1
            uint32 Duration2;       // 2
            uint32 Duration3;       // 3
        };

        #define MAX_SPELL_EFFECTS 3
        #define MAX_EFFECT_MASK 7
        #define MAX_SPELL_REAGENTS 8

        struct SpellEntry_New
        {
            uint32 Id;                                                // 0
            uint32 Category;                                          // 1
            uint32 DispelType;                                        // 2
            uint32 MechanicsType;                                     // 3
            uint32 Attributes;                                        // 4
            uint32 AttributesEx;                                      // 5
            uint32 AttributesExB;                                     // 6
            uint32 AttributesExC;                                     // 7
            uint32 AttributesExD;                                     // 8
            uint32 AttributesExE;                                     // 9
            uint32 AttributesExF;                                     // 10
            uint32 AttributesExG;                                     // 11 
            uint32 RequiredShapeShift;                                // 12
          //uint32 Unknown;                                           // 13 (12-13 Stances[2])
            uint32 ShapeshiftExclude;                                 // 14 
          //uint32 Unknown;                                           // 15 (14-15 StancesExcluded[2])
            uint32 Targets;                                           // 16
            uint32 TargetCreatureType;                                // 17
            uint32 RequiresSpellFocus;                                // 18
            uint32 FacingCasterFlags;                                 // 19
            uint32 CasterAuraState;                                   // 20
            uint32 TargetAuraState;                                   // 21
            uint32 CasterAuraStateNot;                                // 22
            uint32 TargetAuraStateNot;                                // 23
            uint32 casterAuraSpell;                                   // 24
            uint32 targetAuraSpell;                                   // 25
            uint32 casterAuraSpellNot;                                // 26
            uint32 targetAuraSpellNot;                                // 27
            uint32 CastingTimeIndex;                                  // 28
            uint32 RecoveryTime;                                      // 29
            uint32 CategoryRecoveryTime;                              // 30
            uint32 InterruptFlags;                                    // 31
            uint32 AuraInterruptFlags;                                // 32
            uint32 ChannelInterruptFlags;                             // 33
            uint32 procFlags;                                         // 34
            uint32 procChance;                                        // 35
            uint32 procCharges;                                       // 36
            uint32 maxLevel;                                          // 37
            uint32 baseLevel;                                         // 38
            uint32 spellLevel;                                        // 39
            uint32 DurationIndex;                                     // 40
            int32 powerType;                                         // 41
            uint32 manaCost;                                          // 42
            uint32 manaCostPerlevel;                                  // 43
            uint32 manaPerSecond;                                     // 44
            uint32 manaPerSecondPerLevel;                             // 45
            uint32 rangeIndex;                                        // 46
            float speed;                                              // 47
            uint32 modalNextSpell;                                    // 48 comment this out
            uint32 maxstack;                                          // 49
            uint32 Totem[2];                                          // 50 - 51
            uint32 Reagent[8];                                        // 52 - 59 int32
            uint32 ReagentCount[8];                                   // 60 - 67
            int32  EquippedItemClass;                                 // 68
            uint32 EquippedItemSubClass;                              // 69 int32
            uint32 RequiredItemFlags;                                 // 70 int32
            uint32 Effect[MAX_SPELL_EFFECTS];                         // 71 - 73
            uint32 EffectDieSides[MAX_SPELL_EFFECTS];                 // 74 - 76
            float EffectRealPointsPerLevel[MAX_SPELL_EFFECTS];        // 77 - 79
            int32 EffectBasePoints[MAX_SPELL_EFFECTS];                // 80 - 82
            int32 EffectMechanic[MAX_SPELL_EFFECTS];                  // 83 - 85 uint32
            uint32 EffectImplicitTargetA[MAX_SPELL_EFFECTS];          // 86 - 88
            uint32 EffectImplicitTargetB[MAX_SPELL_EFFECTS];          // 89 - 91
            uint32 EffectRadiusIndex[MAX_SPELL_EFFECTS];              // 92 - 94
            uint32 EffectApplyAuraName[MAX_SPELL_EFFECTS];            // 95 - 97
            uint32 EffectAmplitude[MAX_SPELL_EFFECTS];                // 98 - 100
            float EffectMultipleValue[MAX_SPELL_EFFECTS];             // 101 - 103
            uint32 EffectChainTarget[MAX_SPELL_EFFECTS];              // 104 - 106
            uint32 EffectItemType[MAX_SPELL_EFFECTS];                 // 107 - 109 
            uint32 EffectMiscValue[MAX_SPELL_EFFECTS];                // 110 - 112 int32
            uint32 EffectMiscValueB[MAX_SPELL_EFFECTS];               // 113 - 115 int32
            uint32 EffectTriggerSpell[MAX_SPELL_EFFECTS];             // 116 - 118
            float EffectPointsPerComboPoint[MAX_SPELL_EFFECTS];       // 119 - 121
            uint32 EffectSpellClassMask[3][3];                        // 122 - 130
            uint32 SpellVisual;                                       // 131
            uint32 field114;                                          // 132 (131-132 SpellVisual[2])
            uint32 spellIconID;                                       // 133
            uint32 activeIconID;                                      // 134 activeIconID;
            uint32 spellPriority;                                     // 135
            const char* Name;                                         // 136
          //char* NameAlt[15];                                        // 137 - 151 (136-151 Name[16])
          //uint32 NameFlags;                                         // 152 not used
            const char* Rank;                                         // 153
          //char* RankAlt[15];                                        // 154 - 168 (153-168 Rank[16])
          //uint32 RankFlags;                                         // 169 not used
            char* Description;                                        // 170  comment this out
          //char* DescriptionAlt[15];                                 // 171 - 185 (170-185 Description[16])
          //uint32 DescriptionFlags;                                  // 186 not used
            const char* BuffDescription;                              // 187  comment this out
          //char* BuffDescription[15];                                // 188 - 202 (187-202 BuffDescription[16])
          //uint32 buffdescflags;                                     // 203 not used
            uint32 ManaCostPercentage;                                // 204
            uint32 StartRecoveryCategory;                             // 205
            uint32 StartRecoveryTime;                                 // 206
            uint32 MaxTargetLevel;                                    // 207
            uint32 SpellFamilyName;                                   // 208
            uint32 SpellGroupType[MAX_SPELL_EFFECTS];                 // 209 - 211
            uint32 MaxTargets;                                        // 212
            uint32 Spell_Dmg_Type;                                    // 213
            uint32 PreventionType;                                    // 214
            int32 StanceBarOrder;                                     // 215  comment this out
            float dmg_multiplier[MAX_SPELL_EFFECTS];                  // 216 - 218
            uint32 MinFactionID;                                      // 219  comment this out
            uint32 MinReputation;                                     // 220  comment this out
            uint32 RequiredAuraVision;                                // 221  comment this out
            uint32 TotemCategory[2];                                  // 222 - 223
            int32 RequiresAreaId;                                     // 224
            uint32 School;                                            // 225
            uint32 RuneCostID;                                        // 226
          //uint32 SpellMissileID;                                    // 227
          //uint32 PowerDisplayId;                                    // 228
          //float EffectBonusMultiplier[MAX_SPELL_EFFECTS];           // 229 - 231
          //uint32 SpellDescriptionVariable;                          // 232
            uint32 SpellDifficultyID;                                 // 233  comment this out
        };

        struct SpellItemEnchantmentEntry
        {
            uint32 Id;                  // 0
            //uint32 charges;           // 1
            uint32 type[3];             // 2-4
            int32 min[3];               // 5-7 for combat, in practice min==max
            int32 max[3];               // 8-10
            uint32 spell[3];            // 11-13
            char* Name[16];             // 14-29
            //uint32 NameFlags;         // 30
            uint32 visual;              // 31 aura
            uint32 EnchantGroups;       // 32 slot
            uint32 GemEntry;            // 33
            uint32 ench_condition;      // 34
            uint32 req_skill;           // 35
            uint32 req_skill_value;     // 36
            uint32 req_level;           // 37
        };

        struct SpellRadiusEntry
        {
            uint32 ID;                  // 0
            float radius_min;           // 1 Radius
            float radius_per_level;     // 2
            float radius_max;           // 3 Radius2
        };

        struct SpellRangeEntry
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

        struct SpellRuneCostEntry
        {
            uint32 ID;              // 0
            uint32 bloodRuneCost;   // 1
            uint32 frostRuneCost;   // 2
            uint32 unholyRuneCost;  // 3
            uint32 runePowerGain;   // 4
        };

        struct SpellShapeshiftFormEntry
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

        struct SummonPropertiesEntry
        {
            uint32 ID;                  // 0
            uint32 ControlType;         // 1
            uint32 FactionID;           // 2
            uint32 Type;                // 3
            uint32 Slot;                // 4
            uint32 Flags;               // 5
        };

        struct TalentEntry
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

        struct TalentTabEntry
        {
            uint32 TalentTabID;         // 0
            //char* Name[16];           // 1-16
            //uint32 name_flags;        // 17
            //uint32 unk4;              // 18
            //uint32 unk5;              // 19
            uint32 ClassMask;           // 20
            uint32 PetTalentMask;       // 21
            uint32 TabPage;             // 22
            //char* InternalName;       // 23
        };

        struct TaxiNodesEntry
        {
            uint32 id;                  // 0
            uint32 mapid;               // 1
            float x;                    // 2
            float y;                    // 3
            float z;                    // 4
            char* name[16];             // 5-21
            //uint32 nameflags;         // 22
            uint32 horde_mount;         // 23
            uint32 alliance_mount;      // 24
        };

        struct TaxiPathEntry
        {
            uint32 id;                  // 0
            uint32 from;                // 1
            uint32 to;                  // 2
            uint32 price;               // 3
        };

        struct TaxiPathNodeEntry
        {
            uint32 id;                  // 0
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

        struct VehicleEntry
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

        struct VehicleSeatEntry
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

        struct WMOAreaTableEntry
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

        struct WorldMapOverlayEntry
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
    }
}
#endif
