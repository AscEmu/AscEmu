/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _DBC_STRUCTURES_HPP
#define _DBC_STRUCTURES_HPP

#include "Common.h"

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
            //char const creature_display_info_format[]="nixifxxxxxxxxxxxx"; new
            //char const creature_display_info_extra_format[]="nixxxxxxxxxxxxxxxxxxx"; new
            char const creature_family_format[] = "nfifiiiiixsx";
            //char const creature_model_data_format[] = "nxxxxxxxxxxxxxxffxxxxxxxxxxxxxx"; new
            char const creature_spell_data_format[] = "niiiiiiii";  //niiiixxxx
            //char const creature_type_format[]="nxx"; new
            char const currency_types_format[] = "nisxxxxiiix";
            //char const destructible_model_data_format[] = "nixxxixxxxixxxxixxxxixxx"; new
            //char const dungeon_encounter_format[]="niiiisxx"; new
            char const durability_costs_format[] = "niiiiiiiiiiiiiiiiiiiiiiiiiiiii";
            char const durability_quality_format[] = "nf";
            //char const emotes_format[] = "nxxiiixx"; new
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
            char const holidays_format[] = "nxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
            //char const item_armor_quality_format[] = "nfffffffi"; new
            //char const item_armor_shield_format[] = "nifffffff"; new
            //char const item_armor_total_format[] = "niffff"; new
            //char const item_bag_family_format[] = "nx"; new
            //char const item_class_format[] = "nixxxs"; new
            //char const item_damage_format[] = "nfffffffi"; new
            //char const item_entry_format[] = "niiiiiii"; db2
            //char const item_extended_cost_format[] = "niiiiiiiiiiiiiix"; db2
            char const item_random_properties_format[] = "nxiiiiis";
            char const item_random_suffix_format[] = "nsxiiiiiiiiii";
            char const item_set_format[] = "dsxxxxxxxxxxxxxxxxxiiiiiiiiiiiiiiiiii";
            char const item_limit_category_format[] = "nxii";
            char const lfg_dungeon_entry_format[] = "nssssssssssssssssxiiiiiiiiixxixixxxxxxxxxxxxxxxxx";
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
            //char const phase_format[] = "nii"; new
            //char const power_display_format[] = "nixxxx"; new
            //char const pvp_difficulty_format[] = "diiiii"; new
            //char const quest_faction_reward_format[] = "niiiiiiiiii"; new
            //char const quest_sort_entry_format[] = "nx"; new
            char const quest_xp_format[] = "niiiiiiiiii";
            //char const random_properties_points_format[] = "niiiiiiiiiiiiiii"; new
            char const scaling_stat_distribution_format[] = "niiiiiiiiiiiiiiiiiiiixi";
            char const scaling_stat_values_format[] = "iniiiiiiiiiiiiiiiiiiiixxxxxxxxxxxxxxxxxxxxxxxxx";
            char const skill_line_format[] = "nisxixi";
            char const skill_line_ability_format[] = "niiiixxiiiiiix";
            //char const sound_entries_format[] = "nissssssssssssssssssssssxxxxxxxxxxx"; new
            //char const spell_aura_optionsEntry_format[] = "diiii"; new
            //char const spell_aura_restrictions_entry_format[] = "diiiiiiii"; new
            char const spell_cast_times_format[] = "niii";
            //char const spell_casting_requirements_entry_format[] = "dixxixi"; new
            //char const spell_categories_entry_format[] = "diiiiii"; new
            //char const spell_class_options_entry_format[] = "dxiiiix"; new
            //char const spell_cooldowns_entry_format[] = "diii"; new
            char const spell_difficulty_format[] = "niiii";
            char const spell_duration_format[] = "niii";
            char const spell_entry_format[] = "niiiiiiiiiiiiiiifiiiissssiixxixiiiiiiixiiiiiiiix";
            char const spell_item_enchantment_format[] = "nxiiiiiixxxiiisiiiiiiix";
            //char const skill_race_class_info_format[] = "diiiiixxx"; new
            char const spell_radius_format[] = "nfff";
            char const spell_range_format[] = "nffffixx";
            char const spell_rune_cost_format[] = "niiii";
            char const spell_shapeshift_form_format[] = "nxxiixiiixxiiiiiiiixx";
            //char const spell_effect_entry_format[] = "difiiiffiiiiiifiifiiiiiiiix"; new
            //char const spell_equipped_items_entry_format[] = "diii"; new
            //char const spell_focus_object_format[] = "nx"; new
            //char const spell_interrupts_entry_format[] = "dixixi"; new
            //char const spell_item_enchantment_condition_format[] = "nbbbbbxxxxxbbbbbbbbbbiiiiixxxxx"; new
            //char const spell_levels_entry_format[] = "diii"; new
            //char const spell_power_entry_format[] = "diiiiixf"; new
            //char const spell_reagents_entry_format[] = "diiiiiiiiiiiiiiii"; new
            //char const spell_scaling_entry_format[] = "diiiiffffffffffi"; new
            //char const spell_shapeshift_entry_format[] = "dixixx"; new
            //char const spell_target_restrictions_entry_format[] = "dfiiii"; new
            //char const spell_totems_entry_format[] = "diiii"; new
            //char const stable_slot_prices_format[] = "ni"; NA
            char const summon_properties_format[] = "niiiii";
            char const talent_format[] = "niiiiiiiiixxixxxxxx";
            char const talent_tab_format[] = "nxxiiixxiii";
            //char const talent_tree_primary_spells_format[] = "diix"; new
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

        struct SpellCastTimesEntry  //cata
        {
            uint32 ID;              // 0
            uint32 CastTime;        // 1
            float CastTimePerLevel; // 2
            int32 MinCastTime;      // 3
        };

        struct SpellDifficultyEntry //cata
        {
            uint32 ID;              // 0
            int32 SpellId[4];       // 1-4 (instance modes)
        };

        struct SpellDurationEntry   //cata
        {
            uint32 ID;              // 0
            uint32 Duration1;       // 1
            uint32 Duration2;       // 2
            uint32 Duration3;       // 3
        };

        #define MAX_SPELL_EFFECTS 3
        #define MAX_EFFECT_MASK 7
        #define MAX_SPELL_REAGENTS 8

        struct SpellEntry
        {
            uint32 Id;                                          // 0
            uint32 Category;                                    // 1
            uint32 Dispel;                                      // 2
            uint32 Mechanic;                                    // 3
            uint32 Attributes;                                  // 4
            uint32 AttributesEx;                                // 5
            uint32 AttributesExB;                               // 6
            uint32 AttributesExC;                               // 7
            uint32 AttributesExD;                               // 8
            uint32 AttributesExE;                               // 9
            uint32 AttributesExF;                               // 10
            uint32 AttributesExG;                               // 11
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
        };

        struct SpellItemEnchantmentEntry    //cata
        {
            uint32 Id;                  // 0
            //uint32 charges;           // 1
            uint32 type[3];             // 2-4
            uint32 min[3];               // 5-7 for combat, in practice min==max
            //uint32 max[3];               // 8-10
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

        struct SpellRadiusEntry //cata
        {
            uint32 ID;                  // 0
            float radius_min;           // 1 Radius
            float radius_per_level;     // 2
            float radius_max;           // 3 Radius2
        };

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

        struct SpellRuneCostEntry   //cata
        {
            uint32 ID;              // 0
            uint32 bloodRuneCost;   // 1
            uint32 frostRuneCost;   // 2
            uint32 unholyRuneCost;  // 3
            uint32 runePowerGain;   // 4
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
    }
}

#endif // _DBC_STRUCTURES_HPP
