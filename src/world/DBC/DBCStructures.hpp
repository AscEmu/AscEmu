/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DBC_STRUCTURES_HPP
#define _DBC_STRUCTURES_HPP

#include "Common.h"

namespace DBC
{
    namespace Structures
    {
        namespace
        {
            char const area_table_entry_format[] = "iiinixxxxxissssssssssssssssxiiiiixxx";
            char const area_trigger_entry_format[] = "niffffffff";
            char const bank_bag_slot_prices_format[] = "ni";
            char const barber_shop_style_entry_format[] = "nixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxiii";
            char const char_titles_format[] = "nxssssssssssssssssxssssssssssssssssxi";
            char const currency_types_format[] = "xnxi";
            char const durability_costs_format[] = "niiiiiiiiiiiiiiiiiiiiiiiiiiiii";
            char const emotes_text_format[] = "nxiiiixixixxxxxxxxx";
            char const gt_barber_shop_cost_format[] = "f";
            char const gt_oct_regen_hp_format[] = "f";
            char const gt_oct_regen_mp_format[] = "f";
            char const gt_regen_hp_per_spt_format[] = "f";
            char const gt_regen_mp_per_spt_format[] = "f";
            char const item_entry_format[] = "niiiiiii";
            char const item_set_format[] = "issssssssssssssssxiiiiiiiiiixxxxxxxiiiiiiiiiiiiiiiiii";
            char const item_limit_category_format[] = "nxxxxxxxxxxxxxxxxxii";
            char const lfg_dungeon_entry_format[] = "nssssssssssssssssxiiiiiiiiixxixixxxxxxxxxxxxxxxxx";
            char const lock_format[] = "niiiiiiiiiiiiiiiiiiiiiiiixxxxxxxx";
            char const mail_template_format[] = "nsxxxxxxxxxxxxxxxxsxxxxxxxxxxxxxxxx";
            char const map_format[] = "nxiixssssssssssssssssxixxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxixiffxiii";
            char const name_gen_format[] = "nsii";
            char const quest_xp_format[] = "niiiiiiiiii";
            char const scaling_stat_distribution_format[] = "niiiiiiiiiiiiiiiiiiiii";
            char const stable_slot_prices_format[] = "ni";
        }

        #pragma pack(push, 1)
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

        struct GtBarberShopCostBaseEntry
        {
            float cost;             // 0 cost base
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

        struct ItemLimitCategoryEntry
        {
            uint32 Id;                      // 0
            //char* name[16];               // 1-16 name langs
            //uint32 name_flags             // 17
            uint32 maxAmount;               // 18
            uint32 equippedFlag;            // 19 - equipped (bool?)
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

        struct StableSlotPrices
        {
            uint32 Id;              // 0
            uint32 Price;           // 1
        };
        #pragma pack(pop)
    }
}

#endif // _DBC_STRUCTURES_HPP
