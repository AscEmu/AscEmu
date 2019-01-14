/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
            char const char_start_outfit_format[] = "dbbbXiiiiiiiiiiiiiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
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
            char const spell_entry_format[] = "niiiiiiiiiiiixixiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiifxiiiiiiiiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiiiiiiiifffiiiiiiiiiiiiiiifffiiiiiiiiiixiiissssssssssssssssxssssssssssssssssxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiiiiiiiiiiixfffxxxiiiiixxfffxi";
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
            char const totem_category_format[] = "nxxxxxxxxxxxxxxxxxii";
            char const vehicle_format[] = "niffffiiiiiiiifffffffffffffffssssfifiixx";
            char const vehicle_seat_format[] = "niiffffffffffiiiiiifffffffiiifffiiiiiiiffiiiiixxxxxxxxxxxx";
            char const wmo_area_table_format[] = "niiixxxxxiixxxxxxxxxxxxxxxxx";
            char const world_map_area_entry_format[] = "xinxxxxxixx";
            char const world_map_overlay_format[] = "nxiiiixxxxxxxxxxx";
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
            char* name[16];                 // 9-24
                                            //uint32_t name_flags;            // 25
            uint32_t completionFlag;          // 26
            uint32_t groupFlag;               // 27
            uint32_t unk1;                    // 28
            uint32_t timeLimit;               // 29 time limit in seconds
            uint32_t index;                   // 30
        };

        struct AchievementEntry
        {
            uint32_t ID;                      // 0
            int32_t factionFlag;              // 1 -1=all, 0=horde, 1=alliance
            int32_t mapID;                    // 2 -1=none
            //uint32_t unknown1;              // 3
            char* name[16];                 // 4-19
            //uint32_t name_flags;            // 20
            char* description[16];          // 21-36
            //uint32_t desc_flags;            // 37
            uint32_t categoryId;              // 38
            uint32_t points;                  // 39 reward points
            //uint32_t orderInCategory;       // 40
            uint32_t flags;                   // 41
            //uint32_t unknown2;              // 42
            char* rewardName[16];           // 43-58 title/item reward name
            //uint32_t rewardName_flags;      // 59
            uint32_t count;                   // 60
            uint32_t refAchievement;          // 61
        };

        struct AreaGroupEntry
        {
            uint32_t AreaGroupId;             // 0
            uint32_t AreaId[6];               // 1-6
            uint32_t next_group;              // 7
        };

        struct AreaTableEntry
        {
            uint32_t id;                      // 0
            uint32_t map_id;                  // 1
            uint32_t zone;                    // 2 if 0 then it's zone, else it's zone id of this area
            uint32_t explore_flag;            // 3, main index
            uint32_t flags;                   // 4, unknown value but 312 for all cities
                                            // 5-9 unused
            int32_t area_level;               // 10
            char* area_name[16];            // 11-26
                                            // 27, string flags, unused
            uint32_t team;                    // 28
            uint32_t liquid_type_override[4]; // 29-32 liquid override by type
        };

        struct AreaTriggerEntry
        {
            uint32_t id;              // 0
            uint32_t mapid;           // 1
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
            uint32_t id;              // 0
            uint32_t faction;         // 1
            uint32_t fee;             // 2
            uint32_t tax;             // 3
            //char* name[16];       // 4-19
            //uint32_t name_flags;    // 20
        };

        struct BankBagSlotPrices
        {
            uint32_t Id;              // 0
            uint32_t Price;           // 1
        };

        struct BarberShopStyleEntry
        {
            uint32_t id;              // 0
            uint32_t type;            // 1 value 0 -> hair, value 2 -> facialhair
            //char* name;           // 2 string hairstyle name
            //char* name[15];       // 3-17 name of hair style
            //uint32_t name_flags;    // 18
            //uint32_t unk_name[16];  // 19-34, all empty
            //uint32_t unk_flags;     // 35
            //float unk3;           // 36 values 1 and 0,75
            uint32_t race;            // 37 race
            uint32_t gender;          // 38 0 male, 1 female
            uint32_t hair_id;         // 39 Hair ID
        };

        #define OUTFIT_ITEMS 24

        struct CharStartOutfitEntry
        {
            //uint32_t Id;                                    // 0
            uint8_t Race;                                     // 1
            uint8_t Class;                                    // 2
            uint8_t Gender;                                   // 3
            //uint8_t Unused;                                 // 4
            int32_t ItemId[OUTFIT_ITEMS];                     // 5-28
            //int32_t ItemDisplayId[OUTFIT_ITEMS];            // 29-52
            //int32_t ItemInventorySlot[OUTFIT_ITEMS];        // 53-76
        };


        struct CharTitlesEntry
        {
            uint32_t ID;                      // 0, title ids
            //uint32_t unk1;                  // 1 flags?
            char* name_male[16];            // 2-17
            //uint32_t name_flag;             // 18 string flag, unused
            char* name_female[16];          // 19-34
            //const char* name2_flag;       // 35 string flag, unused
            uint32_t bit_index;               // 36 used in PLAYER_CHOSEN_TITLE and 1<<index in PLAYER__FIELD_KNOWN_TITLES
        };

        struct ChatChannelsEntry
        {
            uint32_t id;                      // 0
            uint32_t flags;                   // 1
            char* name_pattern[16];         // 3-18
            //uint32_t name_pattern_flags;    // 19
            //char* channel_name[16];       // 20-35
            //uint32_t channel_name_flags;    // 36
        };

        struct ChrClassesEntry
        {
            uint32_t class_id;                // 0
            //uint32_t unk1;                  // 1
            uint32_t power_type;              // 2
            //uint32_t unk2[2];               // 3-4
            char* name[16];                 // 5-20
            //uint32_t nameflags;             // 21
            //char* name_female[16];        // 22-36
            //uint32_t name_female_flags;     // 37
            //char* name_neutral[16];       // 38-53
            //uint32_t name_neutral_flags;    // 54
            //uint32_t unk3;                  // 55
            uint32_t spellfamily;             // 56
            //uint32_t unk4;                  // 57
            uint32_t cinematic_id;            // 58 CinematicSequences.dbc
            uint32_t expansion;               // 59
        };

        struct ChrRacesEntry
        {
            uint32_t race_id;                 // 0
            uint32_t flags;                   // 1
            uint32_t faction_id;              // 2
            //uint32_t unk1;                  // 3
            uint32_t model_male;              // 4
            uint32_t model_female;            // 5
            // uint32_t unk2;                 // 6
            uint32_t team_id;                 // 7
            //uint32_t unk3[4];               // 8-11
            uint32_t cinematic_id;            // 12 CinematicSequences.dbc
            //uint32_t unk4                   // 13
            char* name[16];                 // 14-29
            //uint32_t name_flags             // 30
            //char* name_female[16];        // 31-46
            //uint32_t name_female_flags      // 47
            //char* name_neutral[16];       // 48-63
            //uint32_t name_neutral_flags     // 64 string flags, unused
            //uint32_t unk5[3]                // 65-67 unused
            uint32_t expansion;               // 68
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
            uint32_t ID;                      // 0
            float minsize;                  // 1
            uint32_t minlevel;                // 2
            float maxsize;                  // 3
            uint32_t maxlevel;                // 4
            uint32_t skilline;                // 5
            uint32_t tameable;                // 6 second skill line - 270 Generic
            uint32_t petdietflags;            // 7
            uint32_t talenttree;              // 8 (-1 = none, 0 = ferocity(410), 1 = tenacity(409), 2 = cunning(411))
            //uint32_t unk;                   // 9 some index 0 - 63
            char* name[16];                 // 10-25
            //uint32_t nameflags;             // 26
            //uint32_t iconFile;              // 27
        };

        struct CreatureSpellDataEntry
        {
            uint32_t id;                      // 0
            uint32_t Spells[3];               // 1-3
            uint32_t PHSpell;                 // 4
            uint32_t Cooldowns[3];            // 5-7
            uint32_t PH;                      // 8
        };

        struct CurrencyTypesEntry
        {
            //uint32_t ID;            // 0 not used
            uint32_t item_id;         // 1 used as index
            //uint32_t Category;      // 2 may be category
            uint32_t bit_index;       // 3 bit index in PLAYER_FIELD_KNOWN_CURRENCIES (1 << (index-1))
        };

        struct DurabilityCostsEntry
        {
            uint32_t itemlevel;       // 0
            uint32_t modifier[29];    // 1-29
        };

        struct DurabilityQualityEntry
        {
            uint32_t id;              // 0
            float quality_modifier; // 1
        };

        struct EmotesTextEntry
        {
            uint32_t Id;              // 0
            //uint32_t name;          // 1
            uint32_t textid;          // 2
            uint32_t textid2;         // 3
            uint32_t textid3;         // 4
            uint32_t textid4;         // 5
            //uint32_t unk1;          // 6
            uint32_t textid5;         // 7
            //uint32_t unk2;          // 8
            uint32_t textid6;         // 9
            //uint32_t unk3;          // 10
            //uint32_t unk4;          // 11
            //uint32_t unk5;          // 12
            //uint32_t unk6;          // 13
            //uint32_t unk7;          // 14
            //uint32_t unk8;          // 15
            //uint32_t unk9;          // 16
            //uint32_t unk10;         // 17
            //uint32_t unk11;         // 18
        };

        struct FactionEntry
        {
            uint32_t ID;                      // 0
            int32_t RepListId;                // 1
            uint32_t RaceMask[4];             // 2-5
            uint32_t ClassMask[4];            // 6-9
            int32_t baseRepValue[4];          // 10-13
            uint32_t repFlags[4];             // 14-17
            uint32_t parentFaction;           // 18
            float spillover_rate_in;        // 19
            float spillover_rate_out;       // 20
            uint32_t spillover_max_in;        // 21
            //uint32_t unk1;                  // 22
            char* Name[16];                 // 23-38
            //uint32_t name_flags;            // 39
            //uint32_t Description[16];       // 40-55
            //uint32_t description_flags;     // 56
        };

        struct FactionTemplateEntry
        {
            uint32_t ID;                      // 0
            uint32_t Faction;                 // 1
            uint32_t FactionGroup;            // 2
            uint32_t Mask;                    // 3
            uint32_t FriendlyMask;            // 4
            uint32_t HostileMask;             // 5
            uint32_t EnemyFactions[4];        // 6-9
            uint32_t FriendlyFactions[4];     // 10-13
        };

        struct GameObjectDisplayInfoEntry
        {
            uint32_t Displayid;               // 0
            char* filename;                 // 1
            //uint32_t  unk1[10];             // 2-11
            float minX;                     // 12
            float minY;                     // 13
            float minZ;                     // 14
            float maxX;                     // 15
            float maxY;                     // 16
            float maxZ;                     // 17
            //uint32_t transport;             // 18
        };

        struct GemPropertiesEntry
        {
            uint32_t Entry;                   // 0
            uint32_t EnchantmentID;           // 1
            //uint32_t unk1;                  // 2 bool
            //uint32_t unk2;                  // 3 bool
            uint32_t SocketMask;              // 4
        };

        struct GlyphPropertiesEntry
        {
            uint32_t Entry;                   // 0
            uint32_t SpellID;                 // 1
            uint32_t Type;                    // 2 (0 = Major, 1 = Minor)
            uint32_t unk;                     // 3 glyph_icon spell.dbc
        };

        struct GlyphSlotEntry
        {
            uint32_t Id;              // 0
            uint32_t Type;            // 1
            uint32_t Slot;            // 2
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
            uint32_t Id;                                  // 0
            uint32_t Duration[MAX_HOLIDAY_DURATIONS];     // 1-10
            uint32_t Date[MAX_HOLIDAY_DATES];             // 11-36
            uint32_t Region;                              // 37
            uint32_t Looping;                             // 38
            uint32_t CalendarFlags[MAX_HOLIDAY_FLAGS];    // 39-48
            //uint32_t holidayNameId;                     // 49 HolidayNames.dbc
            //uint32_t holidayDescriptionId;              // 50 HolidayDescriptions.dbc
            char* TextureFilename;                      // 51
            uint32_t Priority;                            // 52
            uint32_t CalendarFilterType;                  // 53
            //uint32_t flags;                             // 54
        };

        struct ItemEntry
        {
            uint32_t ID;                      // 0
            uint32_t Class;                   // 1
            uint32_t SubClass;                // 2 some items have strange subclasses
            int32_t SoundOverrideSubclass;    // 3
            int32_t Material;                 // 4
            uint32_t DisplayId;               // 5
            uint32_t InventoryType;           // 6
            uint32_t Sheath;                  // 7
        };

        struct ItemExtendedCostEntry
        {
            uint32_t costid;                  // 0
            uint32_t honor_points;            // 1
            uint32_t arena_points;            // 2
            uint32_t arena_slot;              // 3
            uint32_t item[5];                 // 4-8
            uint32_t count[5];                // 9-13
            uint32_t personalrating;          // 14
            //uint32_t unk;                   // 15
        };

        struct ItemLimitCategoryEntry
        {
            uint32_t Id;                      // 0
            //char* name[16];               // 1-16 name langs
            //uint32_t name_flags             // 17
            uint32_t maxAmount;               // 18
            uint32_t equippedFlag;            // 19 - equipped (bool?)
        };

        struct ItemRandomPropertiesEntry
        {
            uint32_t ID;                      // 0
            //uint32_t name1;                 // 1
            uint32_t spells[3];               // 2-4
            //uint32_t unk1;                  // 5
            //uint32_t unk2;                  // 6
            char* name_suffix[16];          // 7-22
            //uint32_t name_suffix_flags;     // 23
        };

        struct ItemRandomSuffixEntry
        {
            uint32_t id;                      // 0
            char* name_suffix[16];          // 1-16
            //uint32_t name_suffix_flags;     // 17
            //uint32_t unk1;                  // 18
            uint32_t enchantments[3];         // 19-21
            //uint32_t unk2[2];               // 22-23
            uint32_t prefixes[3];             // 24-26
            //uint32_t[2];                    // 27-28
        };

        struct ItemSetEntry
        {
            uint32_t id;                      // 1
            char* name[16];                 // 1-16 name (lang)
            //uint32_t localeflag;            // 17 constant
            uint32_t itemid[10];              // 18-27 item set items
            //uint32_t unk[7];                // 28-34 all 0
            uint32_t SpellID[8];              // 35-42
            uint32_t itemscount[8];           // 43-50
            uint32_t RequiredSkillID;         // 51
            uint32_t RequiredSkillAmt;        // 52
        };

        struct LFGDungeonEntry
        {
            uint32_t ID;              // 0
            char* name[16];         // 1-17 Name lang
            uint32_t minlevel;        // 18
            uint32_t maxlevel;        // 19
            uint32_t reclevel;        // 20
            uint32_t recminlevel;     // 21
            uint32_t recmaxlevel;     // 22
            int32_t map;              // 23
            uint32_t difficulty;      // 24
            uint32_t flags;           // 25
            uint32_t type;            // 26
            //uint32_t  unk;          // 27
            //char* iconname;       // 28
            uint32_t expansion;       // 29
            //uint32_t unk4;          // 30
            uint32_t grouptype;       // 31
            //char* desc[16];       // 32-47 Description

            // Helpers
            uint32_t Entry() const { return ID + (type << 24); }
        };

        struct LiquidTypeEntry
        {
            uint32_t Id;                  // 0
            //char* Name;               // 1
            //uint32_t Flags;             // 2
            uint32_t Type;                // 3
            //uint32_t SoundId;           // 4
            uint32_t SpellId;             // 5
            //float MaxDarkenDepth;     // 6
            //float FogDarkenIntensity; // 7
            //float AmbDarkenIntensity; // 8
            //float DirDarkenIntensity; // 9
            //uint32_t LightID;           // 10
            //float ParticleScale;      // 11
            //uint32_t ParticleMovement;  // 12
            //uint32_t ParticleTexSlots;  // 13
            //uint32_t LiquidMaterialID;  // 14
            //char* Texture[6];         // 15-20
            //uint32_t Color[2];          // 21-22
            //float Unk1[18];           // 23-40
            //uint32_t Unk2[4];           // 41-44
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
            uint32_t ID;              // 0
            char* subject;          // 1
            //float unused1[15]     // 2-16
            //uint32_t flags1         // 17 name flags, unused
            char* content;          // 18
            //float unused2[15]     // 19-34
            //uint32_t flags2         // 35 name flags, unused
        };

        struct MapEntry
        {
            uint32_t id;                      // 0
            //char* name_internal;          // 1
            uint32_t map_type;                // 2
            uint32_t map_flags;               // 3
            //uint32_t is_pvp_zone;           // 4 -0 false -1 true
            char* map_name[16];             // 5-20
            //uint32_t name_flags;            // 21
            uint32_t linked_zone;             // 22 common zone for instance and continent map
            //char* horde_intro[16];        // 23-38 horde text for PvP Zones
            //uint32_t hordeIntro_flags;      // 39
            //char* alliance_intro[16];     // 40-55 alliance text for PvP Zones
            //uint32_t allianceIntro_flags;   // 56
            uint32_t multimap_id;             // 57
            //uint32_t battlefield_map_icon;  // 58
            int32_t parent_map;               // 59 map_id of parent map
            float start_x;                  // 60 enter x coordinate (if exist single entry)
            float start_y;                  // 61 enter y coordinate (if exist single entry)
            //uint32_t dayTime;               // 62 
            uint32_t addon;                   // 63 0-original maps, 1-tbc addon, 2-wotlk addon
            uint32_t unk_time;                // 64
            uint32_t max_players;             // 65
        };

        struct NameGenEntry
        {
            uint32_t ID;              // 0
            char* Name;             // 1
            uint32_t unk1;            // 2
            uint32_t type;            // 3
        };

        struct QuestXP
        {
            uint32_t questLevel;     // 0
            uint32_t xpIndex[10];    // 1-10
        };

        struct ScalingStatDistributionEntry
        {
            uint32_t id;                  // 0
            int32_t stat[10];             // 1-10
            uint32_t statmodifier[10];    // 11-20
            uint32_t maxlevel;            // 21
        };

        struct ScalingStatValuesEntry
        {
            uint32_t id;                  // 0
            uint32_t level;               // 1
            uint32_t multiplier[16];      // 2-17 ///\todo split this
            uint32_t unk1;                // 18
            uint32_t amor_mod[5];         // 19-23
        };

        struct SkillLineEntry
        {
            uint32_t id;                  // 0
            uint32_t type;                // 1
            //uint32_t skillCostsID;      // 2
            char* Name[16];             // 3-18
            //uint32_t NameFlags;         // 19
            //char* Description[16];    // 20-35
            //uint32_t DescriptionFlags;  // 36
            uint32_t spell_icon;          // 37
            //char* add_name[16];       // 38-53
            //uint32_t add_name_flags;    // 54
            uint32_t linkable;            // 55
        };

        struct SkillLineAbilityEntry
        {
            uint32_t Id;                      // 0
            uint32_t skilline;                // 1 skill id
            uint32_t spell;                   // 2
            uint32_t race_mask;               // 3
            uint32_t class_mask;              // 4
            //uint32_t excludeRace;           // 5
            //uint32_t excludeClass;          // 6
            uint32_t minSkillLineRank;        // 7 req skill value
            uint32_t next;                    // 8
            uint32_t acquireMethod;           // 9 auto learn
            uint32_t grey;                    // 10 max
            uint32_t green;                   // 11 min
            //uint32_t abandonable;           // 12
            //uint32_t reqTP;                 // 13
        };

        struct StableSlotPrices
        {
            uint32_t Id;              // 0
            uint32_t Price;           // 1
        };

        struct SpellCastTimesEntry
        {
            uint32_t ID;              // 0
            uint32_t CastTime;        // 1
            //uint32_t unk1;          // 2
            //uint32_t unk2;          // 3
        };

        struct SpellDifficultyEntry
        {
            uint32_t ID;              // 0
            int32_t SpellId[4];       // 1-4 (instance modes)
        };

        struct SpellDurationEntry
        {
            uint32_t ID;              // 0
            uint32_t Duration1;       // 1
            uint32_t Duration2;       // 2
            uint32_t Duration3;       // 3
        };

        #define MAX_SPELL_EFFECTS 3
        #define MAX_EFFECT_MASK 7
        #define MAX_SPELL_REAGENTS 8

        struct SpellEntry
        {
            uint32_t Id;                                                // 0
            uint32_t Category;                                          // 1
            uint32_t DispelType;                                        // 2
            uint32_t MechanicsType;                                     // 3
            uint32_t Attributes;                                        // 4
            uint32_t AttributesEx;                                      // 5
            uint32_t AttributesExB;                                     // 6
            uint32_t AttributesExC;                                     // 7
            uint32_t AttributesExD;                                     // 8
            uint32_t AttributesExE;                                     // 9
            uint32_t AttributesExF;                                     // 10
            uint32_t AttributesExG;                                     // 11 
            uint32_t Shapeshifts;                                       // 12
            //uint32_t Shapeshifts1;                                    // 13 not used, all zeros
            uint32_t ShapeshiftsExcluded;                               // 14
            //uint32_t ShapeshiftsExcluded1;                            // 15 not used, all zeros
            uint32_t Targets;                                           // 16
            uint32_t TargetCreatureType;                                // 17
            uint32_t RequiresSpellFocus;                                // 18
            uint32_t FacingCasterFlags;                                 // 19
            uint32_t CasterAuraState;                                   // 20
            uint32_t TargetAuraState;                                   // 21
            uint32_t CasterAuraStateNot;                                // 22
            uint32_t TargetAuraStateNot;                                // 23
            uint32_t casterAuraSpell;                                   // 24
            uint32_t targetAuraSpell;                                   // 25
            uint32_t casterAuraSpellNot;                                // 26
            uint32_t targetAuraSpellNot;                                // 27
            uint32_t CastingTimeIndex;                                  // 28
            uint32_t RecoveryTime;                                      // 29
            uint32_t CategoryRecoveryTime;                              // 30
            uint32_t InterruptFlags;                                    // 31
            uint32_t AuraInterruptFlags;                                // 32
            uint32_t ChannelInterruptFlags;                             // 33
            uint32_t procFlags;                                         // 34
            uint32_t procChance;                                        // 35
            uint32_t procCharges;                                       // 36
            uint32_t maxLevel;                                          // 37
            uint32_t baseLevel;                                         // 38
            uint32_t spellLevel;                                        // 39
            uint32_t DurationIndex;                                     // 40
            int32_t powerType;                                          // 41
            uint32_t manaCost;                                          // 42
            uint32_t manaCostPerlevel;                                  // 43
            uint32_t manaPerSecond;                                     // 44
            uint32_t manaPerSecondPerLevel;                             // 45
            uint32_t rangeIndex;                                        // 46
            float speed;                                                // 47
            //uint32_t modalNextSpell;                                  // 48 not used
            uint32_t MaxStackAmount;                                    // 49
            uint32_t Totem[MAX_SPELL_TOTEMS];                           // 50 - 51
            int32_t Reagent[MAX_SPELL_REAGENTS];                        // 52 - 59
            uint32_t ReagentCount[MAX_SPELL_REAGENTS];                  // 60 - 67
            int32_t EquippedItemClass;                                  // 68
            int32_t EquippedItemSubClass;                               // 69
            int32_t EquippedItemInventoryTypeMask;                      // 70
            uint32_t Effect[MAX_SPELL_EFFECTS];                         // 71 - 73
            int32_t EffectDieSides[MAX_SPELL_EFFECTS];                  // 74 - 76
            float EffectRealPointsPerLevel[MAX_SPELL_EFFECTS];          // 77 - 79
            int32_t EffectBasePoints[MAX_SPELL_EFFECTS];                // 80 - 82
            uint32_t EffectMechanic[MAX_SPELL_EFFECTS];                 // 83 - 85
            uint32_t EffectImplicitTargetA[MAX_SPELL_EFFECTS];          // 86 - 88
            uint32_t EffectImplicitTargetB[MAX_SPELL_EFFECTS];          // 89 - 91
            uint32_t EffectRadiusIndex[MAX_SPELL_EFFECTS];              // 92 - 94
            uint32_t EffectApplyAuraName[MAX_SPELL_EFFECTS];            // 95 - 97
            uint32_t EffectAmplitude[MAX_SPELL_EFFECTS];                // 98 - 100
            float EffectMultipleValue[MAX_SPELL_EFFECTS];               // 101 - 103
            uint32_t EffectChainTarget[MAX_SPELL_EFFECTS];              // 104 - 106
            uint32_t EffectItemType[MAX_SPELL_EFFECTS];                 // 107 - 109 
            int32_t EffectMiscValue[MAX_SPELL_EFFECTS];                 // 110 - 112
            int32_t EffectMiscValueB[MAX_SPELL_EFFECTS];                // 113 - 115
            uint32_t EffectTriggerSpell[MAX_SPELL_EFFECTS];             // 116 - 118
            float EffectPointsPerComboPoint[MAX_SPELL_EFFECTS];         // 119 - 121
            uint32_t EffectSpellClassMask[MAX_SPELL_EFFECTS][3];        // 122 - 130
            uint32_t SpellVisual;                                       // 131
            //uint32_t SpellVisual1;                                    // 132 not used
            uint32_t spellIconID;                                       // 133
            uint32_t activeIconID;                                      // 134 activeIconID;
            uint32_t spellPriority;                                     // 135
            const char* Name[16];                                       // 136 - 151
            //uint32_t NameFlags;                                       // 152 not used
            const char* Rank[16];                                       // 153 - 168
            //uint32_t RankFlags;                                       // 169 not used
            //const char* Description[16];                              // 170 - 185 not used
            //uint32_t DescriptionFlags;                                // 186 not used
            //const char* BuffDescription[16];                          // 187 - 202 not used
            //uint32_t buffdescflags;                                   // 203 not used
            uint32_t ManaCostPercentage;                                // 204
            uint32_t StartRecoveryCategory;                             // 205
            uint32_t StartRecoveryTime;                                 // 206
            uint32_t MaxTargetLevel;                                    // 207
            uint32_t SpellFamilyName;                                   // 208
            uint32_t SpellFamilyFlags[MAX_SPELL_EFFECTS];               // 209 - 211
            uint32_t MaxTargets;                                        // 212
            uint32_t DmgClass;                                          // 213
            uint32_t PreventionType;                                    // 214
            //int32_t StanceBarOrder;                                   // 215 not used
            float EffectDamageMultiplier[MAX_SPELL_EFFECTS];            // 216 - 218
            //uint32_t MinFactionID;                                    // 219 not used
            //uint32_t MinReputation;                                   // 220 not used
            //uint32_t RequiredAuraVision;                              // 221 not used
            uint32_t TotemCategory[MAX_SPELL_TOTEM_CATEGORIES];         // 222 - 223
            int32_t AreaGroupId;                                        // 224
            uint32_t School;                                            // 225
            uint32_t RuneCostID;                                        // 226
            //uint32_t SpellMissileID;                                  // 227 not used
            //uint32_t PowerDisplayId;                                  // 228 not used
            float EffectBonusMultiplier[MAX_SPELL_EFFECTS];             // 229 - 231
            //uint32_t SpellDescriptionVariable;                        // 232 not used
            uint32_t SpellDifficultyId;                                 // 233
        };

        struct SpellItemEnchantmentEntry
        {
            uint32_t Id;                  // 0
            //uint32_t charges;           // 1
            uint32_t type[3];             // 2-4
            int32_t min[3];               // 5-7 for combat, in practice min==max
            int32_t max[3];               // 8-10
            uint32_t spell[3];            // 11-13
            char* Name[16];             // 14-29
            //uint32_t NameFlags;         // 30
            uint32_t visual;              // 31 aura
            uint32_t EnchantGroups;       // 32 slot
            uint32_t GemEntry;            // 33
            uint32_t ench_condition;      // 34
            uint32_t req_skill;           // 35
            uint32_t req_skill_value;     // 36
            uint32_t req_level;           // 37
        };

        struct SpellRadiusEntry
        {
            uint32_t ID;                  // 0
            float radius_min;           // 1 Radius
            float radius_per_level;     // 2
            float radius_max;           // 3 Radius2
        };

        struct SpellRangeEntry
        {
            uint32_t ID;                  // 0
            float minRange;             // 1
            float minRangeFriendly;     // 2
            float maxRange;             // 3
            float maxRangeFriendly;     // 4
            uint32_t range_type;          // 5
            //char* name1[16]           // 6-21
            //uint32_t name1_falgs;       // 22
            //char* name2[16]           // 23-38
            //uint32_t name2_falgs;       // 39
        };

        struct SpellRuneCostEntry
        {
            uint32_t ID;              // 0
            uint32_t bloodRuneCost;   // 1
            uint32_t frostRuneCost;   // 2
            uint32_t unholyRuneCost;  // 3
            uint32_t runePowerGain;   // 4
        };

        struct SpellShapeshiftFormEntry
        {
            uint32_t id;                  // 0
            //uint32_t button_pos;        // 1
            //char* name[16];           // 2-17
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
            //char* Name[16];           // 1-16
            //uint32_t name_flags;        // 17
            //uint32_t unk4;              // 18
            //uint32_t unk5;              // 19
            uint32_t ClassMask;           // 20
            uint32_t PetTalentMask;       // 21
            uint32_t TabPage;             // 22
            //char* InternalName;       // 23
        };

        struct TaxiNodesEntry
        {
            uint32_t id;                  // 0
            uint32_t mapid;               // 1
            float x;                    // 2
            float y;                    // 3
            float z;                    // 4
            char* name[16];             // 5-21
            //uint32_t nameflags;         // 22
            uint32_t horde_mount;         // 23
            uint32_t alliance_mount;      // 24
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
            uint32_t id;                  // 0
            uint32_t path;                // 1
            uint32_t seq;                 // 2 nodeIndex
            uint32_t mapid;               // 3
            float x;                    // 4
            float y;                    // 5
            float z;                    // 6
            uint32_t flags;               // 7
            uint32_t waittime;            // 8
            uint32_t arivalEventID;       // 9
            uint32_t departureEventID;    // 10
        };

        struct TotemCategoryEntry
        {
            uint32_t id;            // 0
            //char* name[16];       // 1-16
            //uint32_t unk;         // 17
            uint32_t categoryType;  // 18
            uint32_t categoryMask;  // 19
        };

        #define MAX_VEHICLE_SEATS 8

        struct VehicleEntry
        {
            uint32_t ID;                                          // 0
            uint32_t flags;                                       // 1
            float turnSpeed;                                    // 2
            float pitchSpeed;                                   // 3
            float pitchMin;                                     // 4
            float pitchMax;                                     // 5
            uint32_t seatID[MAX_VEHICLE_SEATS];                   // 6-13
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
            uint32_t uiLocomotionType;                            // 34
            float msslTrgtImpactTexRadius;                      // 35
            uint32_t uiSeatIndicatorType;                         // 36
            uint32_t powerType;                                   // 37, new in 3.1
            //uint32_t unk1;                                      // 38
            //uint32_t unk2;                                      // 39  
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
            int32_t enterAnimStart;                               // 13
            int32_t enterAnimLoop;                                // 14
            int32_t rideAnimStart;                                // 15
            int32_t rideAnimLoop;                                 // 16
            int32_t rideUpperAnimStart;                           // 17
            int32_t rideUpperAnimLoop;                            // 18
            float exitPreDelay;                                 // 19
            float exitSpeed;                                    // 20
            float exitGravity;                                  // 21
            float exitMinDuration;                              // 22
            float exitMaxDuration;                              // 23
            float exitMinArcHeight;                             // 24
            float exitMaxArcHeight;                             // 25
            int32_t exitAnimStart;                                // 26
            int32_t exitAnimLoop;                                 // 27
            int32_t exitAnimEnd;                                  // 28
            float passengerYaw;                                 // 29
            float passengerPitch;                               // 30
            float passengerRoll;                                // 31
            int32_t passengerAttachmentID;                        // 32
            int32_t vehicleEnterAnim;                             // 33
            int32_t vehicleExitAnim;                              // 34
            int32_t vehicleRideAnimLoop;                          // 35
            int32_t vehicleEnterAnimBone;                         // 36
            int32_t vehicleExitAnimBone;                          // 37
            int32_t vehicleRideAnimLoopBone;                      // 38
            float vehicleEnterAnimDelay;                        // 39
            float vehicleExitAnimDelay;                         // 40
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
            uint32_t id;              // 0
            int32_t rootId;           // 1
            int32_t adtId;            // 2
            int32_t groupId;          // 3
            //uint32_t field4;        // 4
            //uint32_t field5;        // 5
            //uint32_t field6;        // 6
            //uint32_t field7;        // 7
            //uint32_t field8;        // 8
            uint32_t flags;           // 9
            uint32_t areaId;          // 10  ref -> AreaTableEntry
            //char Name[16];        // 11-26
            //uint32_t nameflags;     // 27
        };

        struct WorldMapAreaEntry
        {
            //uint32_t id;              // 0
            uint32_t mapId;             // 1
            uint32_t zoneId;            // 2
            //const char* name;         // 3
            //float y1;                 // 4
            //float y2;                 // 5
            //float x1;                 // 6
            //float x2;                 // 7
            int32_t continentMapId;     // 8 Map id of the continent where the area actually exists (-1 value means that mapId already has the continent map id)
            //uint32_t unk1             // 9
            //uint32_t parentId         // 10
        };

        struct WorldMapOverlayEntry
        {
            uint32_t ID;              // 0
            //uint32_t worldMapID;    // 1
            uint32_t areaID;          // 2 - index to AreaTable
            uint32_t areaID_2;        // 3 - index to AreaTable
            uint32_t areaID_3;        // 4 - index to AreaTable
            uint32_t areaID_4;        // 5 - index to AreaTable
            //uint32_t unk1[2];       // 6-7
            //uint32_t unk2;          // 8
            //uint32_t unk3[7];       // 9-16
        };

        #pragma pack(pop)
    }
}
