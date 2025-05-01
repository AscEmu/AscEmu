/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "CommonTypes.hpp"
#include "WDBDefines.hpp"

#include <cstring>
#include <map>
#include <set>

enum FactionTemplateFlags
{
    FACTION_TEMPLATE_FLAG_PVP = 0x00000800,                 // flagged for PvP
    FACTION_TEMPLATE_FLAG_CONTESTED_GUARD = 0x00001000,     // faction will attack players that were involved in PvP combats
    FACTION_TEMPLATE_FLAG_HOSTILE_BY_DEFAULT = 0x00002000
};

enum FactionMasks
{
    FACTION_MASK_PLAYER = 1,        // any player
    FACTION_MASK_ALLIANCE = 2,      // player or creature from alliance team
    FACTION_MASK_HORDE = 4,         // player or creature from horde team
    FACTION_MASK_MONSTER = 8        // aggressive creature from monster team
    // if none flags set then non-aggressive creature
};

struct WMOAreaTableTripple
{
    WMOAreaTableTripple(int32_t r, int32_t a, int32_t g) : groupId(g), rootId(r), adtId(a) { }

    bool operator <(const WMOAreaTableTripple& b) const
    {
        return memcmp(this, &b, sizeof(WMOAreaTableTripple)) < 0;
    }

    int32_t groupId;
    int32_t rootId;
    int32_t adtId;
};

///\ These will be verified and ported to Spell/Definitions/SpellEffectTarget.hpp when spell targeting is being rewritten -Appled
enum Targets
{
    TARGET_NONE                                 = 0,
    TARGET_SELF_ONE                             = 1,
    TARGET_RANDOM_ENEMY_CHAIN_IN_AREA           = 2,  // only one spell has that, but regardless, it's a target type after all
    TARGET_RANDOM_FRIEND_CHAIN_IN_AREA          = 3,
    TARGET_RANDOM_UNIT_CHAIN_IN_AREA            = 4,  // some plague spells that are infectious - maybe targets not-infected friends inrange
    TARGET_PET                                  = 5,
    TARGET_CHAIN_DAMAGE                         = 6,
    TARGET_AREAEFFECT_INSTANT                   = 7,  // targets around provided destination point
    TARGET_AREAEFFECT_CUSTOM                    = 8,
    TARGET_INNKEEPER_COORDINATES                = 9,  // uses in teleport to innkeeper spells
    TARGET_11                                   = 11, // used by spell 4 'Word of Recall Other'
    TARGET_ALL_ENEMY_IN_AREA                    = 15,
    TARGET_ALL_ENEMY_IN_AREA_INSTANT            = 16,
    TARGET_TABLE_X_Y_Z_COORDINATES              = 17, // uses in teleport spells and some other
    TARGET_EFFECT_SELECT                        = 18, // highly depends on the spell effect
    TARGET_ALL_PARTY_AROUND_CASTER              = 20,
    TARGET_SINGLE_FRIEND                        = 21,
    TARGET_CASTER_COORDINATES                   = 22, // used only in TargetA, target selection dependent from TargetB
    TARGET_GAMEOBJECT                           = 23,
    TARGET_IN_FRONT_OF_CASTER                   = 24,
    TARGET_DUELVSPLAYER                         = 25,
    TARGET_GAMEOBJECT_ITEM                      = 26,
    TARGET_MASTER                               = 27,
    TARGET_ALL_ENEMY_IN_AREA_CHANNELED          = 28,
    TARGET_29                                   = 29,
    TARGET_ALL_FRIENDLY_UNITS_AROUND_CASTER     = 30, // select friendly for caster object faction (in different original caster faction) in TargetB used only with TARGET_ALL_AROUND_CASTER and in self casting range in TargetA
    TARGET_ALL_FRIENDLY_UNITS_IN_AREA           = 31,
    TARGET_MINION                               = 32,
    TARGET_ALL_PARTY                            = 33,
    TARGET_ALL_PARTY_AROUND_CASTER_2            = 34, // used in Tranquility
    TARGET_SINGLE_PARTY                         = 35,
    TARGET_ALL_HOSTILE_UNITS_AROUND_CASTER      = 36,
    TARGET_AREAEFFECT_PARTY                     = 37,
    TARGET_SCRIPT                               = 38,
    TARGET_SELF_FISHING                         = 39,
    TARGET_FOCUS_OR_SCRIPTED_GAMEOBJECT         = 40,
    TARGET_TOTEM_EARTH                          = 41,
    TARGET_TOTEM_WATER                          = 42,
    TARGET_TOTEM_AIR                            = 43,
    TARGET_TOTEM_FIRE                           = 44,
    TARGET_CHAIN_HEAL                           = 45,
    TARGET_SCRIPT_COORDINATES                   = 46,
    TARGET_DYNAMIC_OBJECT_FRONT                 = 47,
    TARGET_DYNAMIC_OBJECT_BEHIND                = 48,
    TARGET_DYNAMIC_OBJECT_LEFT_SIDE             = 49,
    TARGET_DYNAMIC_OBJECT_RIGHT_SIDE            = 50,
    TARGET_AREAEFFECT_GO_AROUND_SOURCE          = 51,
    TARGET_AREAEFFECT_GO_AROUND_DEST            = 52, // gameobject around destination, select by spell_script_target
    TARGET_CURRENT_ENEMY_COORDINATES            = 53, // set unit coordinates as dest, only 16 target B imlemented
    TARGET_LARGE_FRONTAL_CONE                   = 54,
    TARGET_ALL_RAID_AROUND_CASTER               = 56,
    TARGET_SINGLE_FRIEND_2                      = 57,
    TARGET_58                                   = 58,
    TARGET_FRIENDLY_FRONTAL_CONE                = 59,
    TARGET_NARROW_FRONTAL_CONE                  = 60,
    TARGET_AREAEFFECT_PARTY_AND_CLASS           = 61,
    TARGET_DUELVSPLAYER_COORDINATES             = 63,
    TARGET_INFRONT_OF_VICTIM                    = 64,
    TARGET_BEHIND_VICTIM                        = 65, // used in teleport behind spells, caster/target dependent from spell effect
    TARGET_RIGHT_FROM_VICTIM                    = 66,
    TARGET_LEFT_FROM_VICTIM                     = 67,
    TARGET_68                                   = 68,
    TARGET_69                                   = 69,
    TARGET_70                                   = 70,
    TARGET_RANDOM_NEARBY_LOC                    = 72, // used in teleport onto nearby locations
    TARGET_RANDOM_CIRCUMFERENCE_POINT           = 73,
    TARGET_74                                   = 74,
    TARGET_75                                   = 75,
    TARGET_DYNAMIC_OBJECT_COORDINATES           = 76,
    TARGET_SINGLE_ENEMY                         = 77,
    TARGET_POINT_AT_NORTH                       = 78, // 78-85 possible _COORDINATES at radius with pi/4 step around target in unknown order, N?
    TARGET_POINT_AT_SOUTH                       = 79, // S?
    TARGET_POINT_AT_EAST                        = 80, // 80/81 must be symmetric from line caster->target, E (base at 82/83, 84/85 order) ?
    TARGET_POINT_AT_WEST                        = 81, // 80/81 must be symmetric from line caster->target, W (base at 82/83, 84/85 order) ?
    TARGET_POINT_AT_NE                          = 82, // from spell desc: "(NE)"
    TARGET_POINT_AT_NW                          = 83, // from spell desc: "(NW)"
    TARGET_POINT_AT_SE                          = 84, // from spell desc: "(SE)"
    TARGET_POINT_AT_SW                          = 85, // from spell desc: "(SW)"
    TARGET_RANDOM_NEARBY_DEST                   = 86, // "Test Nearby Dest Random" - random around selected destination
    TARGET_SELF2                                = 87,
    TARGET_88                                   = 88, // Smoke Flare(s) and Hurricane
    TARGET_DIRECTLY_FORWARD                     = 89,
    TARGET_NONCOMBAT_PET                        = 90,
    TARGET_91                                   = 91,
    TARGET_SUMMONER                             = 92,
    TARGET_CONTROLLED_VEHICLE                   = 94,
    TARGET_VEHICLE_DRIVER                       = 95,
    TARGET_VEHICLE_PASSENGER_0                  = 96,
    TARGET_VEHICLE_PASSENGER_1                  = 97,
    TARGET_VEHICLE_PASSENGER_2                  = 98,
    TARGET_VEHICLE_PASSENGER_3                  = 99,
    TARGET_VEHICLE_PASSENGER_4                  = 100,
    TARGET_VEHICLE_PASSENGER_5                  = 101,
    TARGET_VEHICLE_PASSENGER_6                  = 102,
    TARGET_VEHICLE_PASSENGER_7                  = 103,
    TARGET_IN_FRONT_OF_CASTER_30                = 104,
    TARGET_105                                  = 105,
    TARGET_106                                  = 106,
    TARGET_107                                  = 107, // possible TARGET_WMO(GO?)_IN_FRONT_OF_CASTER(_30 ?) TODO: Verify the angle!
    TARGET_GO_IN_FRONT_OF_CASTER_90             = 108,
    TARGET_109                                  = 109, // spell 89008
    TARGET_NARROW_FRONTAL_CONE_2                = 110,
    TARGET_111                                  = 111, // not used
    TARGET_112                                  = 112, // spell 89549
    TARGET_113                                  = 113, // not used
    TARGET_114                                  = 114, // not used
    TARGET_115                                  = 115, // not used
    TARGET_116                                  = 116, // not used
    TARGET_117                                  = 117, // test spell 83658
    TARGET_118                                  = 118, // test spell 79579
    TARGET_119                                  = 119, // mass ressurection 83968
    TARGET_120                                  = 120,
    TARGET_121                                  = 121, // spell 95750
    TARGET_122                                  = 122, // spell 100661
    TARGET_123                                  = 123,
    TARGET_124                                  = 124,
    TARGET_125                                  = 125,
    TARGET_126                                  = 126,
    TARGET_127                                  = 127,
};

enum MountFlags
{
    MOUNT_FLAG_CAN_PITCH    = 0x4, // client checks MOVEMENTFLAG2_FULL_SPEED_PITCHING
    MOUNT_FLAG_CAN_SWIM     = 0x8, // client checks MOVEMENTFLAG_SWIMMING
};

struct DBCPosition3D
{
    float X;
    float Y;
    float Z;
};

enum MapTypes
{
    MAP_COMMON          = 0, // none
    MAP_INSTANCE        = 1, // party
    MAP_RAID            = 2, // raid
    MAP_BATTLEGROUND    = 3, // pvp
    MAP_ARENA           = 4  // arena
};

namespace WDB::Structures
{
#if VERSION_STRING <= Classic
    #define NAME_PATTERN 8
#else
    #if VERSION_STRING < Cata
        #define NAME_PATTERN 16
    #else
        #define NAME_PATTERN 1
    #endif
#endif

    #pragma pack(push, 1)
    struct AchievementCategoryEntry
    {
        uint32_t ID;                                                // 0
        uint32_t parentCategory;                                    // 1 -1 for main category
        const char* name;                                           // 2-17
        uint32_t name_flags;                                        // 18
        uint32_t sortOrder;                                         // 19
    };

#if VERSION_STRING >= WotLK
    struct AchievementCriteriaEntry
    {
        uint32_t ID;                                                // 0
        uint32_t referredAchievement;                               // 1
        uint32_t requiredType;                                      // 2
        union
        {
            // ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE = 0
            ///\todo also used for player deaths..
            struct
            {
                uint32_t creatureID;                                // 3
                uint32_t creatureCount;                             // 4
            } kill_creature;

            // ACHIEVEMENT_CRITERIA_TYPE_WIN_BG = 1
            ///\todo there are further criterias instead just winning
            struct
            {
                uint32_t bgMapID;                                   // 3
                uint32_t winCount;                                  // 4
            } win_bg;

            // ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL = 5
            struct
            {
                uint32_t unused;                                    // 3
                uint32_t level;                                     // 4
            } reach_level;

            // ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL = 7
            struct
            {
                uint32_t skillID;                                   // 3
                uint32_t skillLevel;                                // 4
            } reach_skill_level;

            // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT = 8
            struct
            {
                uint32_t linkedAchievement;                         // 3
            } complete_achievement;

            // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT = 9
            struct
            {
                uint32_t unused;                                    // 3
                uint32_t totalQuestCount;                           // 4
            } complete_quest_count;

            // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY = 10
            struct
            {
                uint32_t unused;                                    // 3
                uint32_t numberOfDays;                              // 4
            } complete_daily_quest_daily;

            // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE = 11
            struct
            {
                uint32_t zoneID;                                    // 3
                uint32_t questCount;                                // 4
            } complete_quests_in_zone;

            // ACHIEVEMENT_CRITERIA_TYPE_CURRENCY = 12
            struct
            {
                uint32_t currency;
                uint32_t count;
            } currencyGain;

            // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST = 14
            struct
            {
                uint32_t unused;                                    // 3
                uint32_t questCount;                                // 4
            } complete_daily_quest;

            // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND = 15
            struct
            {
                uint32_t mapID;                                     // 3
            } complete_battleground;

            // ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP= 16
            struct
            {
                uint32_t mapID;                                     // 3
            } death_at_map;

            // ACHIEVEMENT_CRITERIA_TYPE_DEATH_IN_DUNGEON = 18
            struct
            {
                uint32_t  manLimit;                                 // 3
            } death_in_dungeon;

            // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_RAID = 19
            struct
            {
                uint32_t groupSize;                                 // 3 can be 5, 10 or 25
            } complete_raid;

            // ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE = 20
            struct
            {
                uint32_t creatureEntry;                             // 3
            } killed_by_creature;

            // ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING = 24
            struct
            {
                uint32_t unused;                                    // 3
                uint32_t fallHeight;                                // 4
            } fall_without_dying;

            // ACHIEVEMENT_CRITERIA_TYPE_DEATHS_FROM = 26
            struct
            {
                uint32_t type;                                      // 3, see enum EnviromentalDamage
            } death_from;

            // ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST = 27
            struct
            {
                uint32_t questID;                                   // 3
                uint32_t questCount;                                // 4
            } complete_quest;

            // ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET = 28
            // ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2 = 69
            struct
            {
                uint32_t spellID;                                   // 3
                uint32_t spellCount;                                // 4
            } be_spell_target;

            // ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL = 29
            // ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2 = 110
            struct
            {
                uint32_t spellID;                                   // 3
                uint32_t castCount;                                 // 4
            } cast_spell;

            // ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE
            struct
            {
                uint32_t objectiveId;                               // 3
                uint32_t completeCount;                             // 4
            } bg_objective;

            // ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA = 31
            struct
            {
                uint32_t areaID;                                    // 3 Reference to AreaTable.dbc
                uint32_t killCount;                                 // 4
            } honorable_kill_at_area;

            // ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA = 32
            struct
            {
                uint32_t mapID;                                     // 3 Reference to Map.dbc
            } win_arena;

            // ACHIEVEMENT_CRITERIA_TYPE_PLAY_ARENA = 33
            struct
            {
                uint32_t mapID;                                     // 3 Reference to Map.dbc
            } play_arena;

            // ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL = 34
            struct
            {
                uint32_t spellID;                                   // 3 Reference to Map.dbc
            } learn_spell;

            // ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM = 36
            struct
            {
                uint32_t itemID;                                    // 3
                uint32_t itemCount;                                 // 4
            } own_item;

            // ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA = 37
            struct
            {
                uint32_t unused;                                    // 3
                uint32_t count;                                     // 4
                uint32_t flag;                                      // 5 4=in a row
            } win_rated_arena;

            // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_TEAM_RATING = 38
            struct
            {
                uint32_t teamtype;                                  // 3 {2,3,5}
            } highest_team_rating;

            // ACHIEVEMENT_CRITERIA_TYPE_REACH_TEAM_RATING = 39
            struct
            {
                uint32_t teamtype;                                  // 3 {2,3,5}
                uint32_t teamrating;                                // 4
            } reach_team_rating;

            // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_PERSONAL_RATING = 39
            struct
            {
                uint32_t teamtype; // 3 {2, 3, 5}
                uint32_t PersonalRating; // 4
            } highest_personal_rating;

            // ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL = 40
            struct
            {
                uint32_t skillID;                                   // 3
                uint32_t skillLevel;                                // 4 apprentice=1, journeyman=2, expert=3, artisan=4, master=5, grand master=6
            } learn_skill_level;

            // ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM = 41
            struct
            {
                uint32_t itemID;                                    // 3
                uint32_t itemCount;                                 // 4
            } use_item;

            // ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM = 42
            struct
            {
                uint32_t itemID;                                    // 3
                uint32_t itemCount;                                 // 4
            } loot_item;

            // ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA = 43
            struct
            {
                uint32_t areaReference;                             // 3 - this is an index to WorldMapOverlay
            } explore_area;

            // ACHIEVEMENT_CRITERIA_TYPE_OWN_RANK= 44
            struct
            {
                ///\todo This rank is _NOT_ the index from CharTitles.dbc
                uint32_t rank;                                      // 3
            } own_rank;

            // ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT= 45
            struct
            {
                uint32_t unused;                                    // 3
                uint32_t numberOfSlots;                             // 4
            } buy_bank_slot;

            // ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION= 46
            struct
            {
                uint32_t factionID;                                 // 3
                uint32_t reputationAmount;                          // 4 Total reputation amount, so 42000 = exalted
            } gain_reputation;

            // ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION= 47
            struct
            {
                uint32_t unused;                                    // 3
                uint32_t numberOfExaltedFactions;                   // 4
            } gain_exalted_reputation;

            // ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM = 49
            ///\todo where is the required itemlevel stored?
            struct
            {
                uint32_t itemSlot;                                  // 3
            } equip_epic_item;

            // ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT= 50
            struct
            {
                uint32_t rollValue;                                 // 3
                uint32_t count;                                     // 4
            } roll_need_on_loot;

            // ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS = 52
            struct
            {
                uint32_t classID;                                   // 3
                uint32_t count;                                     // 4
            } hk_class;

            // ACHIEVEMENT_CRITERIA_TYPE_HK_RACE = 53
            struct
            {
                uint32_t raceID;                                    // 3
                uint32_t count;                                     // 4
            } hk_race;

            // ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE = 54
            ///\todo where is the information about the target stored?
            struct
            {
                uint32_t emoteID;                                   // 3
            } do_emote;

            // ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE = 55
            struct
            {
                uint32_t unused;                                    // 3
                uint32_t count;                                     // 4
                uint32_t flag;                                      // 5 =3 for battleground healing
                uint32_t mapid;                                     // 6
            } healing_done;

            // ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM = 57
            struct
            {
                uint32_t itemID;                                    // 3
            } equip_item;

            // ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD = 62
            struct
            {
                uint32_t unknown;                                   // 3
                uint32_t goldInCopper;                              // 4
            } quest_reward_money;

            // ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY = 67
            struct
            {
                uint32_t unused;                                    // 3
                uint32_t goldInCopper;                              // 4
            } loot_money;

            // ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT = 68
            struct
            {
                uint32_t goEntry;                                   // 3
                uint32_t useCount;                                  // 4
            } use_gameobject;

            // ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL= 70
            ///\todo are those special criteria stored in the dbc or do we have to add another sql table?
            struct
            {
                uint32_t unused;                                    // 3
                uint32_t killCount;                                 // 4
            } special_pvp_kill;

            // ACHIEVEMENT_CRITERIA_TYPE_FISH_IN_GAMEOBJECT = 72
            struct
            {
                uint32_t goEntry;                                   // 3
                uint32_t lootCount;                                 // 4
            } fish_in_gameobject;

            // ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS= 75
            struct
            {
                uint32_t unknown;                                   // 3 777=?
                uint32_t mountCount;                                // 4
            } number_of_mounts;

            // ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL = 76
            struct
            {
                uint32_t unused;                                    // 3
                uint32_t duelCount;                                 // 4
            } win_duel;

            // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_POWER = 96
            struct
            {
                uint32_t powerType;                                 // 3 mana= 0, 1=rage, 3=energy, 6=runic power
            } highest_power;

            // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_STAT = 97
            struct
            {
                uint32_t statType;                                  // 3 4=spirit, 3=int, 2=stamina, 1=agi, 0=strength
            } highest_stat;

            // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_SPELLPOWER = 98
            struct
            {
                uint32_t spellSchool;                               // 3
            } highest_spellpower;

            // ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_RATING = 100
            struct
            {
                uint32_t ratingType;                                // 3
            } highest_rating;

            // ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE = 109
            struct
            {
                uint32_t lootType;                                  // 3 3=fishing, 2=pickpocket, 4=disentchant
                uint32_t lootTypeCount;                             // 4
            } loot_type;

            // ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2 = 110
            struct
            {
                uint32_t skillLine;                                 // 3
                uint32_t spellCount;                                // 4
            } cast_spell2;

            // ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE= 112
            struct
            {
                uint32_t skillLine;                                 // 3
                uint32_t spellCount;                                // 4
            } learn_skill_line;

            // ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL = 113
            struct
            {
                uint32_t unused;                                    // 3
                uint32_t killCount;                                 // 4
            } honorable_kill;

            struct
            {
                uint32_t field3;                                    // 3 main requirement
                uint32_t field4;                                    // 4 main requirement count
                uint32_t additionalRequirement1_type;               // 5 additional requirement 1 type
                uint32_t additionalRequirement1_value;              // 6 additional requirement 1 value
                uint32_t additionalRequirement2_type;               // 7 additional requirement 2 type
                uint32_t additionalRequirement2_value;              // 8 additional requirement 1 value
            } raw;
        };

        char* name[NAME_PATTERN];                                   // 9-24
        //uint32_t name_flags;                                      // 25
        uint32_t completionFlag;                                    // 26
        uint32_t groupFlag;                                         // 27
        uint32_t unk1;                                              // 28
        uint32_t timeLimit;                                         // 29 time limit in seconds
        uint32_t index;                                             // 30
    };

    struct AchievementEntry
    {
        uint32_t ID;                                                // 0
        int32_t factionFlag;                                        // 1 -1=all, 0=horde, 1=alliance
        int32_t mapID;                                              // 2 -1=none
        uint32_t parentAchievement;                                 // 3
        char* name[NAME_PATTERN];                                   // 4-19
        //uint32_t name_flags;                                      // 20
        char* description[NAME_PATTERN];                            // 21-36
        //uint32_t desc_flags;                                      // 37
        uint32_t categoryId;                                        // 38
        uint32_t points;                                            // 39 reward points
        uint32_t orderInCategory;                                   // 8
        uint32_t flags;                                             // 41
        uint32_t icon;                                              // 10
        char* rewardName[NAME_PATTERN];                             // 43-58 title/item reward name
        //uint32_t rewardName_flags;                                // 59
        uint32_t count;                                             // 60
        uint32_t refAchievement;                                    // 61
#if VERSION_STRING >= Mop
        uint32_t criteriaTreeID;
#endif
    };

    struct AreaGroupEntry
    {
        uint32_t AreaGroupId;                                       // 0
        uint32_t AreaId[6];                                         // 1-6
        uint32_t next_group;                                        // 7
    };
#endif

    struct AreaTableEntry
    {
        uint32_t id;                                                // 0
        uint32_t map_id;                                            // 1
        uint32_t zone;                                              // 2 if 0 then it's zone, else it's zone id of this area
        uint32_t explore_flag;                                      // 3, main index
        uint32_t flags;                                             // 4, unknown value but 312 for all cities
#if VERSION_STRING == Cata
        uint32_t SoundProviderPref;                                 // 5
        uint32_t SoundProviderPrefUnderwater;                       // 6
        uint32_t AmbienceID;                                        // 7
        uint32_t ZoneMusic;                                         // 8
        uint32_t IntroSound;                                        // 9 // 5-9 unused
#endif
        // 5-9 unused
        int32_t area_level;                                         // 10
        char* area_name[NAME_PATTERN];                              // 11-26
        // 27, string flags, unused
        uint32_t team;                                              // 28
#if VERSION_STRING == Classic
        uint32_t liquid_type_override;                              // 29-32 liquid override by type
#else
        uint32_t liquid_type_override[4];
#endif
#if VERSION_STRING == Cata
        float MinElevation;                                         // 17
        float AmbientMultiplier;                                    // 18 client only?
        uint32_t LightID;                                           // 19
        uint32_t MountFlags;                                        // 20
        uint32_t UwIntroSound;                                      // 21 4.0.0
        uint32_t UwZoneMusic;                                       // 22 4.0.0
        uint32_t UwAmbience;                                        // 23 4.0.0
        uint32_t World_pvp_ID;                                      // 24
        int32_t PvpCombatWorldStateID;                              // 25- worldStateId4
#endif
    };

    struct AreaTriggerEntry
    {
        uint32_t id;                                                // 0
        uint32_t mapid;                                             // 1
        float x;                                                    // 2
        float y;                                                    // 3
        float z;                                                    // 4
        float box_radius;                                           // 5 radius
        float box_x;                                                // 6 extent x edge
        float box_y;                                                // 7 extent y edge
        float box_z;                                                // 8 extent z edge
        float box_o;                                                // 9 extent rotation by about z axis
    };

    struct AuctionHouseEntry
    {
        uint32_t id;                                                // 0
        uint32_t faction;                                           // 1
        uint32_t fee;                                               // 2
        uint32_t tax;                                               // 3
        //char* name[16];                                           // 4-19
        //uint32_t name_flags;                                      // 20
    };

    struct BankBagSlotPrices
    {
        uint32_t Id;                                                // 0
        uint32_t Price;                                             // 1
    };

#if VERSION_STRING >= WotLK
    struct BarberShopStyleEntry
    {
        uint32_t id;                                                // 0
        uint32_t type;                                              // 1 value 0 -> hair, value 2 -> facialhair
        //char* name;                                               // 2 string hairstyle name
        //char* name[15];                                           // 3-17 name of hair style
        //uint32_t name_flags;                                      // 18
        //uint32_t unk_name[16];                                    // 19-34, all empty
        //uint32_t unk_flags;                                       // 35
        //float unk3;                                               // 36 values 1 and 0,75
        uint32_t race;                                              // 37 race
        uint32_t gender;                                            // 38 0 male, 1 female
        uint32_t hair_id;                                           // 39 Hair ID
    };
#endif

#if VERSION_STRING >= Cata
    struct BannedAddOnsEntry
    {
        uint32_t Id;                                                // 0
        //uint32_t nameMD5[4];                                      // 1-4
        //uint32_t versionMD5[4];                                   // 5-8
        //uint32_t timestamp;                                       // 9
        //uint32_t state;                                           // 10
    };
#endif

#if VERSION_STRING <= TBC
    #define OUTFIT_ITEMS 12
#else
    #define OUTFIT_ITEMS 24
#endif

    struct CharStartOutfitEntry
    {
        //uint32_t Id;                                              // 0
        uint8_t Race;                                               // 1
        uint8_t Class;                                              // 2
        uint8_t Gender;                                             // 3
        //uint8_t Unused;                                           // 4
        int32_t ItemId[OUTFIT_ITEMS];                               // 5-16
        //int32_t ItemDisplayId[OUTFIT_ITEMS];                      // 17-28
        //int32_t ItemInventorySlot[OUTFIT_ITEMS];                  // 29-40
#if VERSION_STRING >= Cata
        uint32_t PetDisplayId;                                      // 77
        uint32_t PetFamilyEntry;                                    // 78
#endif
    };

#if VERSION_STRING > Classic
    struct CharTitlesEntry
    {
        uint32_t ID;                                                // 0, title ids
        //uint32_t unk1;                                            // 1 flags?
        char* name_male[NAME_PATTERN];                              // 2-17
        //uint32_t name_flag;                                       // 18 string flag, unused
#if VERSION_STRING < Cata
        char* name_female[NAME_PATTERN];                            // 19-34
#endif
        //const char* name2_flag;                                   // 35 string flag, unused
        uint32_t bit_index;                                         // 36 used in PLAYER_CHOSEN_TITLE and 1<<index in PLAYER__FIELD_KNOWN_TITLES
    };
#endif

    struct ChatChannelsEntry
    {
        uint32_t id;                                                // 0
        uint32_t flags;                                             // 1
        char* name_pattern[NAME_PATTERN];                           // 3-18
        //uint32_t name_pattern_flags;                              // 19
        //char* channel_name[16];                                   // 20-35
        //uint32_t channel_name_flags;                              // 36
    };

    struct ChrClassesEntry
    {
        uint32_t class_id;                                          // 0
        //uint32_t unk1;                                            // 1
        //uint32_t unk2;                                            // 2
        uint32_t power_type;                                        // 3
        //uint32_t unk3;                                            // 4
        char* name[NAME_PATTERN];                                              // 5-12
        //uint32_t nameflags;                                       // 13
        //uint32_t unk4;                                            // 14
        uint32_t spellfamily;                                       // 15
        //uint32_t unk4;                                            // 16
#if VERSION_STRING > TBC
        uint32_t cinematic_id;                                      // 58 CinematicSequences.dbc
    #if VERSION_STRING < Mop
        uint32_t expansion;                                         // 59
    #endif
#endif
#if VERSION_STRING > WotLK
        uint32_t apPerStr;                                          // 11
        uint32_t apPerAgi;                                          // 12
        uint32_t rapPerAgi;                                         // 13
#endif
    };

    struct ChrRacesEntry
    {
        uint32_t race_id;                                           // 0
        uint32_t flags;                                             // 1
        uint32_t faction_id;                                        // 2
        //uint32_t unk1;                                            // 3
        uint32_t model_male;                                        // 4
        uint32_t model_female;                                      // 5
        //uint32_t unk2;                                            // 6
        //uint32_t unk3;                                            // 7
        uint32_t team_id;                                           // 8
        //uint32_t unk4[4];                                         // 9-12
        //uint32_t unk5;                                            // 13
#if VERSION_STRING == Classic
        uint32_t start_taxi_mask;                                   // 14
#endif
        //uint32_t unk6;                                            // 15
        uint32_t cinematic_id;                                      // 16 CinematicSequences.dbc
        char* name[NAME_PATTERN];                                   // 17-24
        //uint32_t name_flags                                       // 25
        //uint32_t unk7[2]                                          // 26-27
        //uint32_t unk8;                                            // 28
#if VERSION_STRING > Classic
        uint32_t expansion;                                         // 68
#endif
    };

    struct CreatureDisplayInfoEntry
    {
        uint32_t ID;                                                // 0
        uint32_t ModelID;                                           // 1
        //uint32_t SoundID                                          // 2
        uint32_t ExtendedDisplayInfoID;                             // 3
        float CreatureModelScale;                                   // 4
        // 5 - 15 unk
    };

    struct ChrPowerTypesEntry
    {
        uint32_t entry;                                             // 0
        uint32_t classId;                                           // 1 class
        uint32_t power;                                             // 2 power type
    };

    struct CreatureDisplayInfoExtraEntry
    {
        uint32_t      DisplayExtraId;                               // 0        m_ID CreatureDisplayInfoEntry::m_extendedDisplayInfoID
        uint32_t      Race;                                         // 1        m_DisplayRaceID
        uint32_t      DisplaySexID;                                 // 2        m_DisplaySexID
        // uint32_t    SkinColor;                                   // 3        m_SkinID
        // uint32_t    FaceType;                                    // 4        m_FaceID
        // uint32_t    HairType;                                    // 5        m_HairStyleID
        // uint32_t    HairStyle;                                   // 6        m_HairColorID
        // uint32_t    BeardStyle;                                  // 7        m_FacialHairID
        // uint32_t    Equipment[10];                               // 8-17     m_NPCItemDisplay equipped static items EQUIPMENT_SLOT_HEAD..EQUIPMENT_SLOT_HANDS, client show its by self
        // char*                                                    // 18       m_BakeName CreatureDisplayExtra-*.blp
    };

    enum CreatureModelDataFlags
    {
        CREATURE_MODEL_DATA_FLAGS_CAN_MOUNT = 0x00000080
    };

    struct CreatureModelDataEntry
    {
        uint32_t ID;                                                // 0
        uint32_t Flags;                                             // 1
        char const* ModelName;                                      // 2
        //uint32_t SizeClass;                                       // 3
#if VERSION_STRING <= WotLK
        float ModelScale;                                           // 4 Used in calculation of unit collision data
#endif
        //int32_t BloodID;                                          // 5
        //int32_t FootprintTextureID;                               // 6
        //uint32_t FootprintTextureLength;                          // 7
        //uint32_t FootprintTextureWidth;                           // 8
        //float FootprintParticleScale;                             // 9
        //uint32_t FoleyMaterialID;                                 // 10
        //float FootstepShakeSize;                                  // 11
        //uint32_t DeathThudShakeSize;                              // 12
        //uint32_t SoundID;                                         // 13
        //float CollisionWidth;                                     // 14
        float CollisionHeight;                                      // 15
#if VERSION_STRING > Classic
        float MountHeight;                                          // 16 Used in calculation of unit collision data when mounted
#endif

        inline bool hasFlag(CreatureModelDataFlags flag) const { return (Flags & flag) != 0; }
    };

    struct CreatureFamilyEntry
    {
        uint32_t ID;                                                // 0
        float minsize;                                              // 1
        uint32_t minlevel;                                          // 2
        float maxsize;                                              // 3
        uint32_t maxlevel;                                          // 4
        uint32_t skilline;                                          // 5
        uint32_t tameable;                                          // 6 second skill line - 270 Generic
        uint32_t petdietflags;                                      // 7
#if VERSION_STRING >= WotLK
        uint32_t talenttree;                                        // 8 (-1 = none, 0 = ferocity(410), 1 = tenacity(409), 2 = cunning(411))
#endif
        char* name[NAME_PATTERN];                                   // 8-23
        //uint32_t nameflags;                                       // 24
        //uint32_t iconFile;                                        // 25
    };

    struct CreatureSpellDataEntry
    {
        uint32_t id;                                                // 0
        uint32_t Spells[3];                                         // 1-3
        uint32_t PHSpell;                                           // 4
        uint32_t Cooldowns[3];                                      // 5-7
        uint32_t PH;                                                // 8
    };

#if VERSION_STRING >= WotLK
    struct CurrencyTypesEntry
    {
        //uint32_t ID;                                              // 0 not used
        uint32_t item_id;                                           // 1 used as index
        //uint32_t Category;                                        // 2 may be category
#if VERSION_STRING == WotLK
        uint32_t bit_index;                                         // 3 bit index in PLAYER_FIELD_KNOWN_CURRENCIES (1 << (index-1))
#else
        uint32_t Category;                                          // 1
        char* name;                                                 // 2
        //char* unk                                                 // 3
        //char* unk2                                                // 4
        //uint32_t unk5;                                            // 5
        //uint32_t unk6;                                            // 6
        uint32_t TotalCap;                                          // 7
        uint32_t WeekCap;                                           // 8
        uint32_t Flags;                                             // 9
        //char* description;                                        // 10
#endif
    };
#endif

#if VERSION_STRING >= WotLK
    struct DungeonEncounterEntry
    {
        uint32_t id;                                                // 0 unique id
        uint32_t mapId;                                             // 1 map id
        uint32_t difficulty;                                        // 2 instance mode
        //uint32_t unk0;                                            // 3
        uint32_t encounterIndex;                                    // 4 encounter index for creating completed mask
        char* encounterName[NAME_PATTERN];                          // 5 encounter name
        //uint32_t nameFlags;                                       // 21
        //uint32_t unk1;                                            // 22
    };
#endif

    struct DurabilityCostsEntry
    {
        uint32_t itemlevel;                                         // 0
        uint32_t modifier[29];                                      // 1-29
    };

    struct DurabilityQualityEntry
    {
        uint32_t id;                                                // 0
        float quality_modifier;                                     // 1
    };

#if VERSION_STRING >= Cata
    struct EmotesEntry
    {
        uint32_t Id;                                                // 0
        //char* name;                                               // 1
        //uint32_t animationId;                                     // 2
        uint32_t Flags;                                             // 3
        uint32_t EmoteType;                                         // 4
        uint32_t UnitStandState;                                    // 5
        //uint32_t soundId;                                         // 6
        //uint32_t unk;                                             // 7
    };
#endif

    struct EmotesTextEntry
    {
        uint32_t Id;                                                // 0
        //uint32_t name;                                            // 1
        uint32_t textid;                                            // 2
        uint32_t textid2;                                           // 3
        uint32_t textid3;                                           // 4
        uint32_t textid4;                                           // 5
        //uint32_t unk1;                                            // 6
        uint32_t textid5;                                           // 7
        //uint32_t unk2;                                            // 8
        uint32_t textid6;                                           // 9
        //uint32_t unk3;                                            // 10
        //uint32_t unk4;                                            // 11
        //uint32_t unk5;                                            // 12
        //uint32_t unk6;                                            // 13
        //uint32_t unk7;                                            // 14
        //uint32_t unk8;                                            // 15
        //uint32_t unk9;                                            // 16
        //uint32_t unk10;                                           // 17
        //uint32_t unk11;                                           // 18
    };

    struct FactionEntry
    {
        uint32_t ID;                                                // 0
        int32_t RepListId;                                          // 1
        uint32_t RaceMask[4];                                       // 2-5
        uint32_t ClassMask[4];                                      // 6-9
        int32_t baseRepValue[4];                                    // 10-13
        uint32_t repFlags[4];                                       // 14-17
        uint32_t parentFaction;                                     // 18
#if VERSION_STRING >= WotLK
        float spillover_rate_in;                                    // 19
        float spillover_rate_out;                                   // 20
        uint32_t spillover_max_in;                                  // 21
#endif
        char* Name[NAME_PATTERN];                                   // 19-34
        //uint32_t name_flags;                                      // 35
        //uint32_t Description[16];                                 // 36-51
        //uint32_t description_flags;                               // 52
#if VERSION_STRING >= Mop
        uint32_t GroupExpansion;                                    // 25
#endif

        // helpers
        bool canHaveReputation() const
        {
            return RepListId >= 0;
        }
    };

#define MAX_FACTION_RELATIONS 4
    struct FactionTemplateEntry
    {
        uint32_t ID;                                                // 0
        uint32_t Faction;                                           // 1
        uint32_t FactionGroup;                                      // 2
        uint32_t Mask;                                              // 3
        uint32_t FriendlyMask;                                      // 4
        uint32_t HostileMask;                                       // 5
        uint32_t EnemyFactions[MAX_FACTION_RELATIONS];              // 6-9
        uint32_t FriendlyFactions[MAX_FACTION_RELATIONS];           // 10-13

        // Helpers
        bool isFriendlyTo(FactionTemplateEntry const& entry) const
        {
            if (entry.Faction)
            {
                for (int i = 0; i < MAX_FACTION_RELATIONS; ++i)
                    if (EnemyFactions[i] == entry.Faction)
                        return false;
                for (int i = 0; i < MAX_FACTION_RELATIONS; ++i)
                    if (FriendlyFactions[i] == entry.Faction)
                        return true;
            }
            return (FriendlyMask & entry.Mask) || (Mask & entry.FriendlyMask);
        }
        bool isHostileTo(FactionTemplateEntry const& entry) const
        {
            if (entry.Faction)
            {
                for (int i = 0; i < MAX_FACTION_RELATIONS; ++i)
                    if (EnemyFactions[i] == entry.Faction)
                        return true;
                for (int i = 0; i < MAX_FACTION_RELATIONS; ++i)
                    if (FriendlyFactions[i] == entry.Faction)
                        return false;
            }
            return (HostileMask & entry.Mask) != 0;
        }
        bool isHostileToPlayers() const { return (HostileMask & FACTION_MASK_PLAYER) != 0; }
        bool isNeutralToAll() const
        {
            for (int i = 0; i < MAX_FACTION_RELATIONS; ++i)
                if (EnemyFactions[i] != 0)
                    return false;
            return HostileMask == 0 && FriendlyMask == 0;
        }
        bool isContestedGuardFaction() const { return (FactionGroup & FACTION_TEMPLATE_FLAG_CONTESTED_GUARD) != 0; }
    };

    struct GameObjectDisplayInfoEntry
    {
        uint32_t Displayid;                                         // 0
        char* filename;                                             // 1
        //uint32_t unk1[10];                                        // 2-11
#if VERSION_STRING > Classic
        float minX;                                                 // 12
        float minY;                                                 // 13
        float minZ;                                                 // 14
        float maxX;                                                 // 15
        float maxY;                                                 // 16
        float maxZ;                                                 // 17
#endif
    };

#if VERSION_STRING > Classic
    struct GemPropertiesEntry
    {
        uint32_t Entry;                                             // 0
        uint32_t EnchantmentID;                                     // 1
        //uint32_t unk1;                                            // 2 bool
        //uint32_t unk2;                                            // 3 bool
        uint32_t SocketMask;                                        // 4
    };
#endif

#if VERSION_STRING >= WotLK
    struct GlyphPropertiesEntry
    {
        uint32_t Entry;                                             // 0
        uint32_t SpellID;                                           // 1
        uint32_t Type;                                              // 2 (0 = Major, 1 = Minor)
        uint32_t unk;                                               // 3 glyph_icon spell.dbc
    };

    struct GlyphSlotEntry
    {
        uint32_t Id;                                                // 0
        uint32_t Type;                                              // 1
        uint32_t Slot;                                              // 2
    };

    struct GtBarberShopCostBaseEntry
    {
        float cost;                                                 // 0 cost base
    };
#endif

    struct GtChanceToMeleeCritEntry
    {
        float val;                                                  // 0
    };

    struct GtChanceToMeleeCritBaseEntry
    {
        float val;                                                  // 0
    };

    struct GtChanceToSpellCritEntry
    {
        float val;                                                  // 0
    };

    struct GtChanceToSpellCritBaseEntry
    {
        float val;                                                  // 0
    };

    struct GtCombatRatingsEntry
    {
        float val;                                                  // 0
    };

#if VERSION_STRING < Cata
    struct GtOCTRegenHPEntry
    {
        float ratio;                                                // 0
    };
#endif

    struct GtOCTRegenMPEntry
    {
        float ratio;                                                // 0
    };

#if VERSION_STRING < Cata
    struct GtRegenHPPerSptEntry
    {
        float ratio;                                                // 0 regen base
    };
#endif

    struct GtRegenMPPerSptEntry
    {
        float ratio;                                                // 0 regen base
    };

#if VERSION_STRING >= Cata
    struct GtOCTBaseHPByClassEntry
    {
        float ratio;
    };

    struct GtOCTBaseMPByClassEntry
    {
        float ratio;
    };

    struct GtOCTClassCombatRatingScalarEntry
    {
        float val;                                                  // 0
    };

    struct GuildPerkSpellsEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t Level;                                             // 1
        uint32_t SpellId;                                           // 2
    };
#endif

#if VERSION_STRING >= WotLK
#define MAX_HOLIDAY_DURATIONS 10
#define MAX_HOLIDAY_DATES 26
#define MAX_HOLIDAY_FLAGS 10

    struct HolidaysEntry
    {
        uint32_t Id;                                                // 0
#if VERSION_STRING < Cata
        uint32_t Duration[MAX_HOLIDAY_DURATIONS];                   // 1-10
        uint32_t Date[MAX_HOLIDAY_DATES];                           // 11-36
        uint32_t Region;                                            // 37
        uint32_t Looping;                                           // 38
        uint32_t CalendarFlags[MAX_HOLIDAY_FLAGS];                  // 39-48
        //uint32_t holidayNameId;                                   // 49 HolidayNames.dbc
        //uint32_t holidayDescriptionId;                            // 50 HolidayDescriptions.dbc
        char* TextureFilename;                                      // 51
        uint32_t Priority;                                          // 52
        uint32_t CalendarFilterType;                                // 53
        //uint32_t flags;                                           // 54
#endif
    };
#endif

    struct ItemEntry
    {
        uint32_t ID;                                                // 0
#if VERSION_STRING > TBC
        uint32_t Class;                                             // 1
        uint32_t SubClass;                                          // 2 some items have strange subclasses
        int32_t SoundOverrideSubclass;                              // 3
        int32_t Material;                                           // 4
#endif
        uint32_t DisplayId;                                         // 1
        uint32_t InventoryType;                                     // 2
        uint32_t Sheath;                                            // 3
    };

#if VERSION_STRING >= Cata
    struct ItemCurrencyCostEntry
    {
        //uint32_t id;                                              // 0
        uint32_t itemid;                                            // 1
    };
#endif

#if VERSION_STRING == TBC
    struct ItemDisplayInfo
    {
        uint32_t ID;                                                // 0
        uint32_t randomPropertyChance;                              // 1
    };
#endif

    struct ItemExtendedCostEntry
    {
        uint32_t costid;                                            // 0
        uint32_t honor_points;                                      // 1
        uint32_t arena_points;                                      // 2
#if VERSION_STRING >= WotLK
        uint32_t arena_slot;                                        // 3
#endif
        uint32_t item[5];                                           // 4-8
        uint32_t count[5];                                          // 9-13
        uint32_t personalrating;                                    // 14
#if VERSION_STRING >= Cata
        uint32_t reqcur[5];                                         // 16-20
        uint32_t reqcurrcount[5];                                   // 21-25
        //uint32_t unkunk[5];                                       // 26-30
#endif
    };

#if VERSION_STRING >= WotLK
    struct ItemLimitCategoryEntry
    {
        uint32_t Id;                                                // 0
        //char* name[16];                                           // 1-16 name langs
        //uint32_t name_flags                                       // 17
        uint32_t maxAmount;                                         // 18
        uint32_t equippedFlag;                                      // 19 - equipped (bool?)
    };
#endif

#define MAX_ITEM_ENCHANTMENT_EFFECTS 3
    struct ItemRandomPropertiesEntry
    {
        uint32_t ID;                                                // 0
        //uint32_t name1;                                           // 1
        uint32_t spells[MAX_ITEM_ENCHANTMENT_EFFECTS];              // 2-4
        //uint32_t unk1;                                            // 5
        //uint32_t unk2;                                            // 6
        char* name_suffix[NAME_PATTERN];                            // 7-22
        //uint32_t name_suffix_flags;                               // 23
    };

    struct ItemRandomSuffixEntry
    {
        uint32_t id;                                                // 0
        char* name_suffix[NAME_PATTERN];                            // 1-16
        //uint32_t name_suffix_flags;                               // 17
        //uint32_t unk1;                                            // 18
#if VERSION_STRING <= WotLK
        uint32_t enchantments[3];                                   // 19-21
        uint32_t prefixes[3];                                       // 24-26
#else
        uint32_t enchantments[5];                                   // 3-7
        uint32_t prefixes[5];                                       // 8-12
#endif
    };

#if VERSION_STRING == Cata
    struct ItemReforgeEntry
    {
        uint32_t Id;
        uint32_t SourceStat;
        float SourceMultiplier;
        uint32_t FinalStat;
        float FinalMultiplier;
    };
#endif

    struct ItemSetEntry
    {
        uint32_t id;                                                // 1
        char* name[NAME_PATTERN];                                   // 1-16 name (lang)
        //uint32_t localeflag;                                      // 17 constant
        uint32_t itemid[10];                                        // 18-27 item set items
        //uint32_t unk[7];                                          // 28-34 all 0
        uint32_t SpellID[8];                                        // 35-42
        uint32_t itemscount[8];                                     // 43-50
        uint32_t RequiredSkillID;                                   // 51
        uint32_t RequiredSkillAmt;                                  // 52
    };

    struct LFGDungeonEntry
    {
        uint32_t ID;                                                // 0
        char* name[NAME_PATTERN];                                   // 1-17 Name lang
        uint32_t minlevel;                                          // 18
        uint32_t maxlevel;                                          // 19
        uint32_t reclevel;                                          // 20
        uint32_t recminlevel;                                       // 21
        uint32_t recmaxlevel;                                       // 22
        int32_t map;                                                // 23
        uint32_t difficulty;                                        // 24
        uint32_t flags;                                             // 25
        uint32_t type;                                              // 26
        //uint32_t  unk;                                            // 27
        //char* iconname;                                           // 28
        uint32_t expansion;                                         // 29
        //uint32_t unk4;                                            // 30
        uint32_t grouptype;                                         // 31
        //char* desc[16];                                           // 32-47 Description

        // Helpers
        uint32_t Entry() const { return ID + (type << 24); }
    };

    struct LiquidTypeEntry
    {
        uint32_t Id;                                                // 0
        //uint32_t liquid_id;                                       // 1
        uint32_t Type;                                              // 2
        uint32_t SpellId;                                           // 3
    };

#define LOCK_NUM_CASES 8
    struct LockEntry
    {
        uint32_t Id;                                                // 0
        uint32_t locktype[LOCK_NUM_CASES];                          // 1-8 If this is 1, then the next lockmisc is an item ID, if it's 2, then it's an iRef to LockTypes.dbc.
        uint32_t lockmisc[LOCK_NUM_CASES];                          // 9-16 Item to unlock or iRef to LockTypes.dbc depending on the locktype.
        uint32_t minlockskill[LOCK_NUM_CASES];                      // 17-24 Required skill needed for lockmisc (if locktype = 2).
        //uint32_t action[LOCK_NUM_CASES];                          // 25-32 Something to do with direction / opening / closing.
    };

    struct MailTemplateEntry
    {
        uint32_t ID;                                                // 0
#if VERSION_STRING >= TBC
        char* subject;                                              // 1
        //float unused1[15]                                         // 2-16
        //uint32_t flags1                                           // 17 name flags, unused
        char* content;                                              // 18
#endif
        //float unused2[15]                                         // 19-34
        //uint32_t flags2                                           // 35 name flags, unused
    };

    struct MapEntry
    {
        uint32_t id;                                                // 0
        //char* name_internal;                                      // 1
        uint32_t map_type;                                          // 2
        //uint32_t is_pvp_zone;                                     // 3 -0 false -1 true
        char* map_name[NAME_PATTERN];                               // 4-19
        //uint32_t name_flags;                                      // 20
        uint32_t linked_zone;                                       // 22 common zone for instance and continent map
        //char* horde_intro[16];                                    // 23-38 horde text for PvP Zones
        //uint32_t hordeIntro_flags;                                // 39
        //char* alliance_intro[16];                                 // 40-55 alliance text for PvP Zones
        //uint32_t allianceIntro_flags;                             // 56
        uint32_t multimap_id;                                       // 57
#if VERSION_STRING >= TBC
        int32_t parent_map;                                         // 59 map_id of parent map
        float start_x;                                              // 60 enter x coordinate (if exist single entry)
        float start_y;                                              // 61 enter y coordinate (if exist single entry)
        //uint32_t dayTime;                                         // 62
#if VERSION_STRING == TBC
        uint32_t reset_raid_time;
        uint32_t reset_heroic_tim;
#endif
        uint32_t addon;                                             // 63 0-original maps, 1-tbc addon, 2-wotlk addon
#endif
#if VERSION_STRING >= WotLK
        uint32_t unk_time;                                          // 64
        uint32_t max_players;                                       // 65
#endif
#if VERSION_STRING >= Cata
        uint32_t next_phase_map;                                    // 19
#endif

        bool isDungeon() const { return map_type == MAP_INSTANCE || map_type == MAP_RAID; }
        bool isNonRaidDungeon() const { return map_type == MAP_INSTANCE; }
        bool instanceable() const { return map_type == MAP_INSTANCE || map_type == MAP_RAID || map_type == MAP_BATTLEGROUND; }
        bool isRaid() const { return map_type == MAP_RAID; }
        bool isBattleground() const { return map_type == MAP_BATTLEGROUND; }
        bool isBattleArena() const { return map_type == MAP_ARENA; }
        bool isBattlegroundOrArena() const { return map_type == MAP_BATTLEGROUND; }
        bool isWorldMap() const { return map_type == MAP_COMMON; }

        uint32_t getAddon() const
        {
#ifdef AE_CLASSIC
            const uint32_t addon = 0;
#endif
            return addon;
        }

#if VERSION_STRING <= TBC
        uint32_t getResetTimeNormal() const
        {
#ifdef AE_CLASSIC
            const uint32_t reset_raid_time = 604800;
#endif
            return reset_raid_time;
        }

        uint32_t getResetTimeHeroic() const
        {
#ifdef AE_CLASSIC
            const uint32_t reset_heroic_tim = 0;
#endif
            return reset_heroic_tim;
        }
#endif

#if VERSION_STRING >= TBC
        bool getEntrancePos(int32_t& mapid, float& x, float& y) const
        {
            if (parent_map < 0)
                return false;
            mapid = parent_map;
            x = start_x;
            y = start_y;
            return true;
        }
#endif

        bool isContinent() const
        {
            return id == 0 || id == 1 || id == 530 || id == 571;
        }
    };

    struct MapDifficulty
    {
        MapDifficulty() : resetTime(0), maxPlayers(0), hasErrorMessage(false) { }
        MapDifficulty(uint32_t _resetTime, uint32_t _maxPlayers, bool _hasErrorMessage) : resetTime(_resetTime), maxPlayers(_maxPlayers), hasErrorMessage(_hasErrorMessage) { }

        uint32_t resetTime;
        uint32_t maxPlayers;
        bool hasErrorMessage;
    };

#if VERSION_STRING >= WotLK
    struct MapDifficultyEntry
    {
        //uint32_t ID;                                              // 0
        uint32_t MapID;                                             // 1
        uint32_t Difficulty;                                        // 2 (for arenas: arena slot)
        char const* Message;                                        // 3-18 text showed when transfer to map failed (missing requirements)
        //uint32_t Message_lang_mask;                               // 19
        uint32_t RaidDuration;                                      // 20
        uint32_t MaxPlayers;                                        // 21
        //char const* Difficultystring;                             // 22
    };
#endif

#if VERSION_STRING >= Cata
    struct MountCapabilityEntry
    {
        uint32_t  id;                                               // 0 index
        uint32_t  flag;                                             // 1 some flag
        uint32_t  reqRidingSkill;                                   // 2 skill level of riding required
        uint32_t  reqArea;                                          // 3 required Area
        uint32_t  reqAura;                                          // 4 required Aura
        uint32_t  reqSpell;                                         // 5 spell that has to be known to you
        uint32_t  speedModSpell;                                    // 6 spell to cast to apply mount speed effects
        uint32_t  reqMap;                                           // 7 map where this is applicable
    };

#define MAX_MOUNT_CAPABILITIES 24
    struct MountTypeEntry
    {
        uint32_t id;                                                // 0 index
        uint32_t capabilities[MAX_MOUNT_CAPABILITIES];              // 1-17 capability ids from MountCapability.dbc
        //uint32_t  empty[7];                                       // 18-24 empty. maybe continues capabilities
    };
#endif

    struct NameGenEntry
    {
        uint32_t ID;                                                // 0
        char* Name;                                                 // 1
        uint32_t unk1;                                              // 2
        uint32_t type;                                              // 3
    };

#if VERSION_STRING >= Cata
    struct NumTalentsAtLevel
    {
        //uint32_t level;                                           // 0
        float talentPoints;                                         // 1
    };

    struct PhaseEntry
    {
        uint32_t Id;                                                // 0
        uint32_t PhaseShift;                                        // 1
        uint32_t Flags;                                             // 2
    };

    struct QuestSortEntry
    {
        uint32_t id;                                                // 0
        //char* name;                                               // 1
    };
#endif

#if VERSION_STRING >= WotLK
    struct QuestXP
    {
        uint32_t questLevel;                                        // 0
        uint32_t xpIndex[10];                                       // 1-10
    };

    struct ScalingStatDistributionEntry
    {
        uint32_t id;                                                // 0
        int32_t stat[10];                                           // 1-10
        uint32_t statmodifier[10];                                  // 11-20
        uint32_t maxlevel;                                          // 21
    };

    struct ScalingStatValuesEntry
    {
        uint32_t id;                                                // 0
        uint32_t level;                                             // 1
#if VERSION_STRING == Cata
        uint32_t multiplier[20];
#elif VERSION_STRING == Mop
        uint32_t multiplier[47];
#else
        uint32_t shoulderBudget;                                    // 2
        uint32_t trinketBudget;                                     // 3
        uint32_t weaponBudget1H;                                    // 4
        uint32_t rangedBudget;                                      // 5
        uint32_t clothShoulderArmor;                                // 6
        uint32_t leatherShoulderArmor;                              // 7
        uint32_t mailShoulderArmor;                                 // 8
        uint32_t plateShoulderArmor;                                // 9
        uint32_t weaponDPS1H;                                       // 10
        uint32_t weaponDPS2H;                                       // 11
        uint32_t spellcasterDPS1H;                                  // 12
        uint32_t spellcasterDPS2H;                                  // 13
        uint32_t rangedDPS;                                         // 14
        uint32_t wandDPS;                                           // 15
        uint32_t spellPower;                                        // 16
        uint32_t primaryBudget;                                     // 17
        uint32_t tertiaryBudget;                                    // 18
        uint32_t clothCloakArmor;                                   // 19
        uint32_t clothChestArmor;                                   // 20
        uint32_t leatherChestArmor;                                 // 21
        uint32_t mailChestArmor;                                    // 22
        uint32_t plateChestArmor;                                   // 23

        uint32_t getScalingStatDistributionMultiplier(uint32_t mask) const
        {
            if (mask & 0x4001F)
            {
                if (mask & 0x00000001) return shoulderBudget;
                if (mask & 0x00000002) return trinketBudget;
                if (mask & 0x00000004) return weaponBudget1H;
                if (mask & 0x00000008) return primaryBudget;
                if (mask & 0x00000010) return rangedBudget;
                if (mask & 0x00040000) return tertiaryBudget;
            }
            return 0;
        }

        uint32_t getArmorMod(uint32_t mask) const
        {
            if (mask & 0x00F001E0)
            {
                if (mask & 0x00000020) return clothShoulderArmor;
                if (mask & 0x00000040) return leatherShoulderArmor;
                if (mask & 0x00000080) return mailShoulderArmor;
                if (mask & 0x00000100) return plateShoulderArmor;

                if (mask & 0x00080000) return clothCloakArmor;
                if (mask & 0x00100000) return clothChestArmor;
                if (mask & 0x00200000) return leatherChestArmor;
                if (mask & 0x00400000) return mailChestArmor;
                if (mask & 0x00800000) return plateChestArmor;
            }
            return 0;
        }

        uint32_t getDPSMod(uint32_t mask) const
        {
            if (mask & 0x7E00)
            {
                if (mask & 0x00000200) return weaponDPS1H;
                if (mask & 0x00000400) return weaponDPS2H;
                if (mask & 0x00000800) return spellcasterDPS1H;
                if (mask & 0x00001000) return spellcasterDPS2H;
                if (mask & 0x00002000) return rangedDPS;
                if (mask & 0x00004000) return wandDPS;
            }
            return 0;
        }

        uint32_t getSpellBonus(uint32_t mask) const
        {
            if (mask & 0x00008000) return spellPower;
            return 0;
        }
#endif
    };
#endif

    struct SkillLineEntry
    {
        uint32_t id;                                                // 0
        uint32_t type;                                              // 1
        //uint32_t skillCostsID;                                    // 2
        char* Name[NAME_PATTERN];                                   // 3-18
        //uint32_t NameFlags;                                       // 19
        //char* Description[16];                                    // 20-35
        //uint32_t DescriptionFlags;                                // 36
        uint32_t spell_icon;                                        // 37
#if VERSION_STRING >= WotLK
        uint32_t linkable;                                          // 55
#endif
    };

    struct SkillLineAbilityEntry
    {
        uint32_t Id;                                                // 0
        uint32_t skilline;                                          // 1 skill id
        uint32_t spell;                                             // 2
        uint32_t race_mask;                                         // 3
        uint32_t class_mask;                                        // 4
        //uint32_t excludeRace;                                     // 5
        //uint32_t excludeClass;                                    // 6
        uint32_t minSkillLineRank;                                  // 7 req skill value
        uint32_t next;                                              // 8
        uint32_t acquireMethod;                                     // 9 auto learn
        uint32_t grey;                                              // 10 max
        uint32_t green;                                             // 11 min
        //uint32_t abandonable;                                     // 12
        //uint32_t reqTP;                                           // 13
        //uint32_t reqtrainpoints;                                  // 14
    };

#if VERSION_STRING <= WotLK
    struct StableSlotPrices
    {
        uint32_t Id;                                                // 0
        uint32_t Price;                                             // 1
    };
#endif

#if VERSION_STRING >= Cata

    // SpellAuraOptions.dbc
    struct SpellAuraOptionsEntry
    {
#if VERSION_STRING == Mop
        uint32_t Id;                                                // 0
#endif
        uint32_t MaxStackAmount;                                    // 1
        uint32_t procChance;                                        // 2
        uint32_t procCharges;                                       // 3
        uint32_t procFlags;                                         // 4
    };

    // SpellAuraRestrictions.dbc
    struct SpellAuraRestrictionsEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t CasterAuraState;                                   // 1
        uint32_t TargetAuraState;                                   // 2
        uint32_t CasterAuraStateNot;                                // 3
        uint32_t TargetAuraStateNot;                                // 4
        uint32_t casterAuraSpell;                                   // 5
        uint32_t targetAuraSpell;                                   // 6
        uint32_t CasterAuraSpellNot;                                // 7
        uint32_t TargetAuraSpellNot;                                // 8
    };

    // SpellCastingRequirements.dbc
    struct SpellCastingRequirementsEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t FacingCasterFlags;                                 // 1
        //uint32_t MinFactionId;                                    // 2
        //uint32_t MinReputation;                                   // 3
        int32_t AreaGroupId;                                        // 4
        //uint32_t RequiredAuraVision;                              // 5
        uint32_t RequiresSpellFocus;                                // 6
    };

    // SpellCategories.dbc
    struct SpellCategoriesEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t Category;                                          // 1
        uint32_t DmgClass;                                          // 2
        uint32_t DispelType;                                        // 3
        uint32_t MechanicsType;                                     // 4
        uint32_t PreventionType;                                    // 5
        uint32_t StartRecoveryCategory;                             // 6
    };

    // SpellClassOptions.dbc
    struct SpellClassOptionsEntry
    {
        //uint32_t Id;                                              // 0
        //uint32_t modalNextSpell;                                  // 1
        uint32_t SpellFamilyFlags[MAX_SPELL_EFFECTS];               // 2 - 4
        uint32_t SpellFamilyName;                                   // 5
        //char* Description;                                        // 6
    };

    // SpellCooldowns.dbc
    struct SpellCooldownsEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t CategoryRecoveryTime;                              // 1
        uint32_t RecoveryTime;                                      // 2
        uint32_t StartRecoveryTime;                                 // 3
    };

    // SpellEffect.dbc
    struct SpellEffectEntry
    {
        uint32_t id;                                                // 0
        //uint32_t unk;                                             // 1
        uint32_t Effect;                                            // 2
        float EffectMultipleValue;                                  // 3
        uint32_t EffectApplyAuraName;                               // 4
        uint32_t EffectAmplitude;                                   // 5
        int32_t EffectBasePoints;                                   // 6
        float EffectBonusMultiplier;                                // 7
        float EffectDamageMultiplier;                               // 8
        uint32_t EffectChainTarget;                                 // 9
        int32_t EffectDieSides;                                     // 10
        uint32_t EffectItemType;                                    // 11
        uint32_t EffectMechanic;                                    // 12
        int32_t EffectMiscValue;                                    // 13
        int32_t EffectMiscValueB;                                   // 14
        float EffectPointsPerComboPoint;                            // 15
        uint32_t EffectRadiusIndex;                                 // 16
        uint32_t EffectRadiusMaxIndex;                              // 17
        float EffectRealPointsPerLevel;                             // 18
#if VERSION_STRING == Cata
        uint32_t EffectSpellClassMask[3];                           // 18-20
#else
        uint32_t EffectSpellClassMask[4];                           // 19-22
#endif
        //ClassFamilyMask EffectSpellClassMask;
        uint32_t EffectTriggerSpell;                                // 23
        uint32_t EffectImplicitTargetA;                             // 25
        uint32_t EffectImplicitTargetB;                             // 26
        uint32_t EffectSpellId;                                     // 27
        uint32_t EffectIndex;                                       // 28
        //uint32_t unk;                                             // 29

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
        //uint32_t Id;                                              // 0
        int32_t EquippedItemClass;                                  // 1
        int32_t EquippedItemInventoryTypeMask;                      // 2
        int32_t EquippedItemSubClassMask;                           // 3
    };

    // SpellFocusObject.dbc
    struct SpellFocusObjectEntry
    {
        uint32_t ID;                                                // 0
        //char* Name;                                               // 1
    };

    // SpellInterrupts.dbc
    struct SpellInterruptsEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t AuraInterruptFlags;                                // 1
        //uint32_t unk2                                             // 2
        uint32_t ChannelInterruptFlags;                             // 3
        //uint32_t unk4                                             // 4
        uint32_t InterruptFlags;                                    // 5
    };

    // SpellItemEnchantmentCondition.dbc
    struct SpellItemEnchantmentConditionEntry
    {
        uint32_t ID;                                                // 0
        uint8_t Color[5];                                           // 1-5
        //uint32_t LT_Operand[5];                                   // 6-10
        uint8_t Comparator[5];                                      // 11-15
        uint8_t CompareColor[5];                                    // 15-20
        uint32_t Value[5];                                          // 21-25
        //uint8_t Logic[5]                                          // 25-30
    };

    // SpellLevels.dbc
    struct SpellLevelsEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t baseLevel;                                         // 1
        uint32_t maxLevel;                                          // 2
        uint32_t spellLevel;                                        // 3
    };

    // SpellPower.dbc
    struct SpellPowerEntry
    {
        //uint32_t Id;                                              // 0
#if VERSION_STRING == Mop
        uint32_t spellId;
        //uint32_t RaidDifficulty;
        uint32_t powerType;
#endif
        uint32_t manaCost;                                          // 1
        uint32_t manaCostPerlevel;                                  // 2
#if VERSION_STRING == Cata
        uint32_t ManaCostPercentage;                                // 3
#endif
        uint32_t manaPerSecond;                                     // 4
        uint32_t manaPerSecondPerLevel;                             // 5
        //uint32_t PowerDisplayId;                                  // 6 
        float ManaCostPercentageFloat;                              // 7
#if VERSION_STRING == Mop
        float ChannelCostPercentageFloat;
        uint32_t ShapeShiftSpellId;
#endif
    };

    struct SpellReagentsEntry
    {
        //uint32_t Id;                                              // 0
        int32_t Reagent[MAX_SPELL_REAGENTS];                        // 54-61
#if VERSION_STRING == Cata
        uint32_t ReagentCount[MAX_SPELL_REAGENTS];                  // 62-69
#else
        uint32_t ReagentCount[10];                                  // 62-69
#endif
    };


    // SpellScaling.dbc
    struct SpellScalingEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t castTimeMin;                                       // 1
        uint32_t castTimeMax;                                       // 2
        uint32_t castScalingMaxLevel;                               // 3
        uint32_t playerClass;                                       // 4
#if VERSION_STRING == Cata
        float coeff1[3];                                            // 5-7
        float coeff2[3];                                            // 8-10
        float coeff3[3];                                            // 11-13
#endif
        float coefBase;                                             // 14
        int32_t coefLevelBase;                                      // 15

        bool IsScalableEffect(uint8_t i) const { return coefBase != 0.0f; };
    };

    // SpellShapeshift.dbc
    struct SpellShapeshiftEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t ShapeshiftsExcluded;                               // 1
        //uint32_t ShapeshiftsExcluded1;                            // 2 unused, all zeros
        uint32_t Shapeshifts;                                       // 3
        //uint32_t Shapeshifts1;                                    // 4 unused, all zeros
        //uint32_t StanceBarOrder;                                  // 5
    };

    // SpellTargetRestrictions.dbc
    struct SpellTargetRestrictionsEntry
    {
        uint32_t Id;                                                // 0
        float MaxTargetRadius;                                      // 1
        uint32_t MaxAffectedTargets;                                // 2
        uint32_t MaxTargetLevel;                                    // 3
        uint32_t TargetCreatureType;                                // 4
        uint32_t Targets;                                           // 5
    };

    // SpellTotems.dbc
    struct SpellTotemsEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t TotemCategory[MAX_SPELL_TOTEM_CATEGORIES];         // 1 2
        uint32_t Totem[MAX_SPELL_TOTEMS];                           // 3 4
    };

#if VERSION_STRING == Mop
    struct SpellMiscEntry
    {
        uint32_t Id;                                                // 0
        uint32_t SpellDifficultyId;
        uint32_t Attributes;                                        // 1
        uint32_t AttributesEx;                                      // 2
        uint32_t AttributesExB;                                     // 3
        uint32_t AttributesExC;                                     // 4
        uint32_t AttributesExD;                                     // 5
        uint32_t AttributesExE;                                     // 6
        uint32_t AttributesExF;                                     // 7
        uint32_t AttributesExG;                                     // 8
        uint32_t AttributesExH;                                     // 9
        uint32_t AttributesExI;                                     // 10
        uint32_t AttributesExJ;                                     // 11
        uint32_t AttributesExK;
        uint32_t AttributesExL;
        uint32_t AttributesExM;
        uint32_t CastingTimeIndex;                                  // 12
        uint32_t DurationIndex;                                     // 13
        //int32_t powerType;                                        // 14
        uint32_t rangeIndex;                                        // 15
        float speed;                                                // 16
        uint32_t SpellVisual;                                       // 17
        uint32_t SpellVisual1;                                      // 18
        uint32_t spellIconID;                                       // 19
        uint32_t activeIconID;                                      // 20
        uint32_t School;                                            // 25

        //uint32_t SpellPowerId;                                      // 42 SpellPower.dbc
    };
#endif

#endif

    struct SpellCastTimesEntry
    {
        uint32_t ID;                                                // 0
        uint32_t CastTime;                                          // 1
        float CastTimePerLevel;                                     // 2
        int32_t MinCastTime;                                        // 3
    };

#if VERSION_STRING >= WotLK
    struct SpellDifficultyEntry
    {
        uint32_t ID;                                                // 0
        int32_t SpellId[InstanceDifficulty::MAX_DIFFICULTY];        // 1-4
    };

    struct SpellRuneCostEntry
    {
        uint32_t ID;                                                // 0
        uint32_t bloodRuneCost;                                     // 1
        uint32_t frostRuneCost;                                     // 2
        uint32_t unholyRuneCost;                                    // 3
#if VERSION_STRING == Mop
        uint32_t deathRuneCost;
#endif
        uint32_t runePowerGain;                                     // 4
    };
#endif

    struct SpellDurationEntry
    {
        uint32_t ID;                                                // 0
        int32_t Duration1;                                          // 1
        int32_t Duration2;                                          // 2
        int32_t Duration3;                                          // 3
    };

    struct SpellRadiusEntry
    {
        uint32_t ID;                                                // 0
        float radius_min;                                           // 1 Radius
        float radius_per_level;                                     // 2
        float radius_max;                                           // 3 Radius2
    };

    struct SpellRangeEntry
    {
        uint32_t ID;                                                // 0
        float minRange;                                             // 1
#if VERSION_STRING >= WotLK
        float minRangeFriendly;                                     // 2
#endif
        float maxRange;                                             // 3
#if VERSION_STRING >= WotLK
        float maxRangeFriendly;                                     // 4
#endif
        uint32_t range_type;                                        // 4
        //char* name1[16]                                           // 6-21
        //uint32_t name1_falgs;                                     // 22
        //char* name2[16]                                           // 23-38
        //uint32_t name2_falgs;                                     // 39
    };

    struct SpellShapeshiftFormEntry
    {
        uint32_t id;                                                // 0
        //uint32_t button_pos;                                      // 1
        //char* name[16];                                           // 2-17
        //uint32_t name_flags;                                      // 18
        uint32_t Flags;                                             // 19
        uint32_t unit_type;                                         // 20
        //uint32_t unk1                                             // 21
#if VERSION_STRING >= TBC
        uint32_t AttackSpeed;                                       // 22
        uint32_t modelId;                                           // 23 alliance?
        uint32_t modelId2;                                          // 24 horde?
        //uint32_t unk2                                             // 25
        //uint32_t unk3                                             // 26
        uint32_t spells[8];                                         // 27-34
#endif
    };

    struct SpellItemEnchantmentEntry
    {
        uint32_t Id;                                                // 0
        uint32_t type[MAX_ITEM_ENCHANTMENT_EFFECTS];                // 1-3
        uint32_t min[MAX_ITEM_ENCHANTMENT_EFFECTS];                 // 4-6 for combat, in practice min==max
#if VERSION_STRING <= Cata
        uint32_t max[MAX_ITEM_ENCHANTMENT_EFFECTS];                 // 7-9
#endif
        uint32_t spell[MAX_ITEM_ENCHANTMENT_EFFECTS];               // 10-12
        char* Name[NAME_PATTERN];                                   // 13
        uint32_t visual;                                            // 30 aura
        uint32_t EnchantGroups;                                     // 31 slot
#if VERSION_STRING >= TBC
        uint32_t GemEntry;                                          // 32
        uint32_t ench_condition;                                    // 33
#endif
#if VERSION_STRING >= WotLK
        uint32_t req_skill;                                         // 35
        uint32_t req_skill_value;                                   // 36
        uint32_t req_level;                                         // 37
#endif
    };

    struct SummonPropertiesEntry
    {
        uint32_t ID;                                                // 0
        uint32_t ControlType;                                       // 1
        uint32_t FactionID;                                         // 2
        uint32_t Type;                                              // 3
        uint32_t Slot;                                              // 4
        uint32_t Flags;                                             // 5
    };

    struct TalentEntry
    {
        uint32_t TalentID;                                          // 0
#if VERSION_STRING < Mop
        uint32_t TalentTree;                                        // 1
#endif
        uint32_t Row;                                               // 2
        uint32_t Col;                                               // 3
#if VERSION_STRING < Mop
        uint32_t RankID[5];                                         // 4-8

        //uint32_t unk[4];                                          // 9-12
        uint32_t DependsOn;                                         // 13
        //uint32_t unk1[2];                                         // 14-15
        uint32_t DependsOnRank;                                     // 16
#else
        uint32_t SpellId;
#endif
        //uint32_t unk2[2];                                         // 17-18
        //uint32_t unk3;                                            // 19
        //uint32_t unk4;                                            // 20
        //uint32_t unk5;                                            // 21
#if VERSION_STRING == Mop
        uint32_t playerClass;
        uint32_t overrideSpellId;
#endif
    };

    struct TalentTabEntry
    {
        uint32_t TalentTabID;                                       // 0
        //char* Name[16];                                           // 1-16
        //uint32_t name_flags;                                      // 17
        //uint32_t unk4;                                            // 18
        //uint32_t unk5;                                            // 19
        uint32_t ClassMask;                                         // 20
#if VERSION_STRING >= WotLK
        uint32_t PetTalentMask;                                     // 21
#endif
        uint32_t TabPage;                                           // 21
        //char* InternalName;                                       // 22
#if VERSION_STRING >= Cata
        uint32_t rolesMask;                                         // 8
        uint32_t masterySpells[2];                                  // 9-10
#endif
    };

#if VERSION_STRING >= Cata
    struct TalentTreePrimarySpells
    {
        uint32_t ID;                                                // 0
        uint32_t tabID;                                             // 1
        uint32_t SpellID;                                           // 2
        //uint32_t unk                                              // 3
    };
#endif

    struct TaxiNodesEntry
    {
        uint32_t id;                                                // 0
        uint32_t mapid;                                             // 1
        float x;                                                    // 2
        float y;                                                    // 3
        float z;                                                    // 4
        char* name[NAME_PATTERN];                                   // 5-21
        //uint32_t nameflags;                                       // 22
        uint32_t mountCreatureID[2];                                // 23-24
#if VERSION_STRING >= Cata
        uint32_t flags;                                             // 8
#endif
    };

    struct TaxiPathEntry
    {
        uint32_t id;                                                // 0
        uint32_t from;                                              // 1
        uint32_t to;                                                // 2
        uint32_t price;                                             // 3
    };

    struct TaxiPathNodeEntry
    {
        //uint32_t id;                                              // 0
        uint32_t pathId;                                            // 1
        uint32_t NodeIndex;                                         // 2 nodeIndex
        uint32_t mapid;                                             // 3
        float x;                                                    // 4
        float y;                                                    // 5
        float z;                                                    // 6
        uint32_t flags;                                             // 7
        uint32_t waittime;                                          // 8
#if VERSION_STRING >= TBC
        uint32_t arivalEventID;                                     // 9
        uint32_t departureEventID;                                  // 10
#endif
    };

#if VERSION_STRING >= TBC
    struct TotemCategoryEntry
    {
        uint32_t id;                                                // 0
        //char* name[16];                                           // 1-16
        //uint32_t unk;                                             // 17
        uint32_t categoryType;                                      // 18
        uint32_t categoryMask;                                      // 19
    };
#endif

    struct TransportAnimationEntry
    {
        //uint32_t Id;                                              // 0
        uint32_t TransportID;                                       // 1
        uint32_t TimeIndex;                                         // 2
        DBCPosition3D Pos;                                          // 3
        //uint32_t SequenceID;                                      // 4
    };

#if VERSION_STRING >= WotLK
    struct TransportRotationEntry
    {
        //uint32_t ID;                                              // 0
        uint32_t GameObjectsID;                                     // 1
        uint32_t TimeIndex;                                         // 2
        float X;                                                    // 3
        float Y;                                                    // 4
        float Z;                                                    // 5
        float W;                                                    // 6
    };
#endif

#define MAX_VEHICLE_SEATS 8

    struct VehicleEntry
    {
        uint32_t ID;                                                // 0
        uint32_t flags;                                             // 1
        float turnSpeed;                                            // 2
        float pitchSpeed;                                           // 3
        float pitchMin;                                             // 4
        float pitchMax;                                             // 5
        uint32_t seatID[MAX_VEHICLE_SEATS];                         // 6-13
        float mouseLookOffsetPitch;                                 // 14
        float cameraFadeDistScalarMin;                              // 15
        float cameraFadeDistScalarMax;                              // 16
        float cameraPitchOffset;                                    // 17
        float facingLimitRight;                                     // 18
        float facingLimitLeft;                                      // 19
        float msslTrgtTurnLingering;                                // 20
        float msslTrgtPitchLingering;                               // 21
        float msslTrgtMouseLingering;                               // 22
        float msslTrgtEndOpacity;                                   // 23
        float msslTrgtArcSpeed;                                     // 24
        float msslTrgtArcRepeat;                                    // 25
        float msslTrgtArcWidth;                                     // 26
        float msslTrgtImpactRadius[2];                              // 27-28
        char* msslTrgtArcTexture;                                   // 29
        char* msslTrgtImpactTexture;                                // 30
        char* msslTrgtImpactModel[2];                               // 31-32
        float cameraYawOffset;                                      // 33
        uint32_t uiLocomotionType;                                  // 34
        float msslTrgtImpactTexRadius;                              // 35
        uint32_t uiSeatIndicatorType;                               // 36
        uint32_t powerType;                                         // 37, new in 3.1
        //uint32_t unk1;                                            // 38
        //uint32_t unk2;                                            // 39
    };

    enum VehicleSeatFlags
    {
        VEHICLE_SEAT_FLAG_HAS_LOWER_ANIM_FOR_ENTER = 0x00000001,
        VEHICLE_SEAT_FLAG_HAS_LOWER_ANIM_FOR_RIDE = 0x00000002,
        VEHICLE_SEAT_FLAG_UNK3 = 0x00000004,
        VEHICLE_SEAT_FLAG_SHOULD_USE_VEH_SEAT_EXIT_ANIM_ON_VOLUNTARY_EXIT = 0x00000008,
        VEHICLE_SEAT_FLAG_UNK5 = 0x00000010,
        VEHICLE_SEAT_FLAG_UNK6 = 0x00000020,
        VEHICLE_SEAT_FLAG_UNK7 = 0x00000040,
        VEHICLE_SEAT_FLAG_UNK8 = 0x00000080,
        VEHICLE_SEAT_FLAG_UNK9 = 0x00000100,
        VEHICLE_SEAT_FLAG_HIDE_PASSENGER = 0x00000200,              // Passenger is hidden
        VEHICLE_SEAT_FLAG_ALLOW_TURNING = 0x00000400,
        VEHICLE_SEAT_FLAG_CAN_CONTROL = 0x00000800,                 // Lua_UnitInVehicleControlSeat
        VEHICLE_SEAT_FLAG_CAN_CAST_MOUNT_SPELL = 0x00001000,        // Can cast spells with SPELL_AURA_MOUNTED from seat (possibly 4.x only, 0 seats on 3.3.5a)
        VEHICLE_SEAT_FLAG_UNCONTROLLED = 0x00002000,                // can override !& VEHICLE_SEAT_FLAG_CAN_ENTER_OR_EXIT
        VEHICLE_SEAT_FLAG_CAN_ATTACK = 0x00004000,                  // Can attack, cast spells and use items from vehicle?
        VEHICLE_SEAT_FLAG_SHOULD_USE_VEH_SEAT_EXIT_ANIM_ON_FORCED_EXIT = 0x00008000,
        VEHICLE_SEAT_FLAG_UNK17 = 0x00010000,
        VEHICLE_SEAT_FLAG_UNK18 = 0x00020000,                       // Needs research and support (28 vehicles): Allow entering vehicles while keeping specific permanent(?) auras that impose visuals (states like beeing under freeze/stun mechanic, emote state animations).
        VEHICLE_SEAT_FLAG_HAS_VEH_EXIT_ANIM_VOLUNTARY_EXIT = 0x00040000,
        VEHICLE_SEAT_FLAG_HAS_VEH_EXIT_ANIM_FORCED_EXIT = 0x00080000,
        VEHICLE_SEAT_FLAG_PASSENGER_NOT_SELECTABLE = 0x00100000,
        VEHICLE_SEAT_FLAG_UNK22 = 0x00200000,
        VEHICLE_SEAT_FLAG_REC_HAS_VEHICLE_ENTER_ANIM = 0x00400000,
        VEHICLE_SEAT_FLAG_IS_USING_VEHICLE_CONTROLS = 0x00800000,   // Lua_IsUsingVehicleControls
        VEHICLE_SEAT_FLAG_ENABLE_VEHICLE_ZOOM = 0x01000000,
        VEHICLE_SEAT_FLAG_USABLE = 0x02000000,                      // Lua_CanExitVehicle
        VEHICLE_SEAT_FLAG_CAN_SWITCH = 0x04000000,                  // Lua_CanSwitchVehicleSeats
        VEHICLE_SEAT_FLAG_HAS_START_WARITING_FOR_VEH_TRANSITION_ANIM_ENTER = 0x08000000,
        VEHICLE_SEAT_FLAG_HAS_START_WARITING_FOR_VEH_TRANSITION_ANIM_EXIT = 0x10000000,
        VEHICLE_SEAT_FLAG_CAN_CAST = 0x20000000,                    // Lua_UnitHasVehicleUI
        VEHICLE_SEAT_FLAG_UNK2 = 0x40000000,                        // checked in conjunction with 0x800 in CastSpell2
    };

    enum VehicleSeatFlagsB
    {
        VEHICLE_SEAT_FLAG_B_NONE = 0x00000000,
        VEHICLE_SEAT_FLAG_B_USABLE_FORCED = 0x00000002,
        VEHICLE_SEAT_FLAG_B_TARGETS_IN_RAIDUI = 0x00000008,         // Lua_UnitTargetsVehicleInRaidUI
        VEHICLE_SEAT_FLAG_B_EJECTABLE = 0x00000020,                 // ejectable
        VEHICLE_SEAT_FLAG_B_USABLE_FORCED_2 = 0x00000040,
        VEHICLE_SEAT_FLAG_B_USABLE_FORCED_3 = 0x00000100,
        VEHICLE_SEAT_FLAG_B_KEEP_PET = 0x00020000,
        VEHICLE_SEAT_FLAG_B_USABLE_FORCED_4 = 0x02000000,
        VEHICLE_SEAT_FLAG_B_CAN_SWITCH = 0x04000000,
        VEHICLE_SEAT_FLAG_B_VEHICLE_PLAYERFRAME_UI = 0x80000000     // Lua_UnitHasVehiclePlayerFrameUI - actually checked for flagsb &~ 0x80000000
    };

    struct VehicleSeatEntry
    {
        uint32_t ID;                                                // 0
        uint32_t flags;                                             // 1
        int32_t attachmentID;                                       // 2
        float attachmentOffsetX;                                    // 3
        float attachmentOffsetY;                                    // 4
        float attachmentOffsetZ;                                    // 5
        float enterPreDelay;                                        // 6
        float enterSpeed;                                           // 7
        float enterGravity;                                         // 8
        float enterMinDuration;                                     // 9
        float enterMaxDuration;                                     // 10
        float enterMinArcHeight;                                    // 11
        float enterMaxArcHeight;                                    // 12
        int32_t enterAnimStart;                                     // 13
        int32_t enterAnimLoop;                                      // 14
        int32_t rideAnimStart;                                      // 15
        int32_t rideAnimLoop;                                       // 16
        int32_t rideUpperAnimStart;                                 // 17
        int32_t rideUpperAnimLoop;                                  // 18
        float exitPreDelay;                                         // 19
        float exitSpeed;                                            // 20
        float exitGravity;                                          // 21
        float exitMinDuration;                                      // 22
        float exitMaxDuration;                                      // 23
        float exitMinArcHeight;                                     // 24
        float exitMaxArcHeight;                                     // 25
        int32_t exitAnimStart;                                      // 26
        int32_t exitAnimLoop;                                       // 27
        int32_t exitAnimEnd;                                        // 28
        float passengerYaw;                                         // 29
        float passengerPitch;                                       // 30
        float passengerRoll;                                        // 31
        int32_t passengerAttachmentID;                              // 32
        int32_t vehicleEnterAnim;                                   // 33
        int32_t vehicleExitAnim;                                    // 34
        int32_t vehicleRideAnimLoop;                                // 35
        int32_t vehicleEnterAnimBone;                               // 36
        int32_t vehicleExitAnimBone;                                // 37
        int32_t vehicleRideAnimLoopBone;                            // 38
        float vehicleEnterAnimDelay;                                // 39
        float vehicleExitAnimDelay;                                 // 40
        uint32_t vehicleAbilityDisplay;                             // 41
        uint32_t enterUISoundID;                                    // 42
        uint32_t exitUISoundID;                                     // 43
        int32_t uiSkin;                                             // 44
        uint32_t flagsB;                                            // 45

        bool hasFlag(VehicleSeatFlags flag) const { return (flags & flag) != 0; }
        bool hasFlag(VehicleSeatFlagsB flag) const { return (flagsB & flag) != 0; }

        bool IsUsable() const
        {
            if ((flags & VEHICLE_SEAT_FLAG_USABLE) != 0)
                return true;
            return false;
        }

        bool IsController() const
        {
            if ((flags & VEHICLE_SEAT_FLAG_CAN_CONTROL) != 0)
                return true;
            return false;
        }

        bool HidesPassenger() const
        {
            if ((flags & VEHICLE_SEAT_FLAG_HIDE_PASSENGER) != 0)
                return true;
            return false;
        }

        bool canEnterOrExit() const { return hasFlag(VehicleSeatFlags(VEHICLE_SEAT_FLAG_USABLE | VEHICLE_SEAT_FLAG_CAN_CONTROL | VEHICLE_SEAT_FLAG_SHOULD_USE_VEH_SEAT_EXIT_ANIM_ON_VOLUNTARY_EXIT)); }
        bool canSwitchFromSeat() const { return hasFlag(VEHICLE_SEAT_FLAG_CAN_SWITCH); }
        bool isUsableByOverride() const {
            return hasFlag(VehicleSeatFlags(VEHICLE_SEAT_FLAG_UNCONTROLLED | VEHICLE_SEAT_FLAG_UNK18))
                || hasFlag(VehicleSeatFlagsB(VEHICLE_SEAT_FLAG_B_USABLE_FORCED | VEHICLE_SEAT_FLAG_B_USABLE_FORCED_2 |
                    VEHICLE_SEAT_FLAG_B_USABLE_FORCED_3 | VEHICLE_SEAT_FLAG_B_USABLE_FORCED_4));
        }
        bool isEjectable() const { return hasFlag(VEHICLE_SEAT_FLAG_B_EJECTABLE); }
    };

    struct WMOAreaTableEntry
    {
        uint32_t id;                                                // 0
        int32_t rootId;                                             // 1
        int32_t adtId;                                              // 2
        int32_t groupId;                                            // 3
        //uint32_t field4;                                          // 4
        //uint32_t field5;                                          // 5
        //uint32_t field6;                                          // 6
        //uint32_t field7;                                          // 7
        //uint32_t field8;                                          // 8
        uint32_t flags;                                             // 9
        uint32_t areaId;                                            // 10 ref -> AreaTableEntry
        //char Name[16];                                            // 11-26
        //uint32_t nameflags;                                       // 27
    };

#if VERSION_STRING >= TBC
    struct WorldMapAreaEntry
    {
        //uint32_t id;                                              // 0
        uint32_t mapId;                                             // 1
        uint32_t zoneId;                                            // 2
        //char const* name;                                         // 3
        //float y1;                                                 // 4
        //float y2;                                                 // 5
        //float x1;                                                 // 6
        //float x2;                                                 // 7
        int32_t continentMapId;                                     // 8 Map id of the continent where the area actually exists (-1 value means that mapId already has the continent map id)
    };
#endif

    struct WorldMapOverlayEntry
    {
        uint32_t ID;                                                // 0
        //uint32_t worldMapID;                                      // 1
        uint32_t areaID;                                            // 2 - index to AreaTable
        uint32_t areaID_2;                                          // 3 - index to AreaTable
        uint32_t areaID_3;                                          // 4 - index to AreaTable
        uint32_t areaID_4;                                          // 5 - index to AreaTable
        //uint32_t unk1[2];                                         // 6-7
        //uint32_t unk2;                                            // 8
        //uint32_t unk3[7];                                         // 9-16
    };

#if VERSION_STRING == Classic
    struct SpellEntry
    {
        uint32_t Id;                                                // 0
        uint32_t School;                                            // 1 NOT in bitmask!
        uint32_t Category;                                          // 2
        //uint32_t castUI;                                          // 3 not used
        uint32_t DispelType;                                        // 4
        uint32_t MechanicsType;                                     // 5
        uint32_t Attributes;                                        // 6
        uint32_t AttributesEx;                                      // 7
        uint32_t AttributesExB;                                     // 8
        uint32_t AttributesExC;                                     // 9
        uint32_t AttributesExD;                                     // 10
        uint32_t Shapeshifts;                                       // 11
        uint32_t ShapeshiftsExcluded;                               // 12
        uint32_t Targets;                                           // 13
        uint32_t TargetCreatureType;                                // 14
        uint32_t RequiresSpellFocus;                                // 15
        uint32_t CasterAuraState;                                   // 16
        uint32_t TargetAuraState;                                   // 17
        uint32_t CastingTimeIndex;                                  // 18
        uint32_t RecoveryTime;                                      // 19
        uint32_t CategoryRecoveryTime;                              // 20
        uint32_t InterruptFlags;                                    // 21
        uint32_t AuraInterruptFlags;                                // 22
        uint32_t ChannelInterruptFlags;                             // 23
        uint32_t procFlags;                                         // 24
        uint32_t procChance;                                        // 25
        uint32_t procCharges;                                       // 26
        uint32_t maxLevel;                                          // 27
        uint32_t baseLevel;                                         // 28
        uint32_t spellLevel;                                        // 29
        uint32_t DurationIndex;                                     // 30
        int32_t powerType;                                          // 31
        uint32_t manaCost;                                          // 32
        uint32_t manaCostPerlevel;                                  // 33
        uint32_t manaPerSecond;                                     // 34
        uint32_t manaPerSecondPerLevel;                             // 35
        uint32_t rangeIndex;                                        // 36
        float speed;                                                // 37
        //uint32_t modalNextSpell;                                  // 38 not used
        uint32_t MaxStackAmount;                                    // 39
        uint32_t Totem[MAX_SPELL_TOTEMS];                           // 40 - 41
        int32_t Reagent[MAX_SPELL_REAGENTS];                        // 42 - 49
        uint32_t ReagentCount[MAX_SPELL_REAGENTS];                  // 50 - 57
        int32_t EquippedItemClass;                                  // 58
        int32_t EquippedItemSubClass;                               // 59
        int32_t EquippedItemInventoryTypeMask;                      // 60
        uint32_t Effect[MAX_SPELL_EFFECTS];                         // 61 - 63
        int32_t EffectDieSides[MAX_SPELL_EFFECTS];                  // 64 - 66
        uint32_t EffectBaseDice[MAX_SPELL_EFFECTS];                 // 67 - 69
        float EffectDicePerLevel[MAX_SPELL_EFFECTS];                // 70 - 72
        float EffectRealPointsPerLevel[MAX_SPELL_EFFECTS];          // 73 - 75
        int32_t EffectBasePoints[MAX_SPELL_EFFECTS];                // 76 - 78
        uint32_t EffectMechanic[MAX_SPELL_EFFECTS];                 // 79 - 81
        uint32_t EffectImplicitTargetA[MAX_SPELL_EFFECTS];          // 82 - 84
        uint32_t EffectImplicitTargetB[MAX_SPELL_EFFECTS];          // 85 - 87
        uint32_t EffectRadiusIndex[MAX_SPELL_EFFECTS];              // 88 - 90
        uint32_t EffectApplyAuraName[MAX_SPELL_EFFECTS];            // 91 - 93
        uint32_t EffectAmplitude[MAX_SPELL_EFFECTS];                // 94 - 96
        float EffectMultipleValue[MAX_SPELL_EFFECTS];               // 97 - 99
        uint32_t EffectChainTarget[MAX_SPELL_EFFECTS];              // 100 - 102
        uint32_t EffectItemType[MAX_SPELL_EFFECTS];                 // 107 - 105
        int32_t EffectMiscValue[MAX_SPELL_EFFECTS];                 // 106 - 108
        uint32_t EffectTriggerSpell[MAX_SPELL_EFFECTS];             // 109 - 111
        float EffectPointsPerComboPoint[MAX_SPELL_EFFECTS];         // 112 - 114
        uint32_t SpellVisual;                                       // 115
        uint32_t SpellVisual1;                                      // 116
        uint32_t spellIconID;                                       // 117
        uint32_t activeIconID;                                      // 118 activeIconID;
        uint32_t spellPriority;                                     // 119
        const char* Name[8];                                        // 120 - 127
        //uint32_t NameFlags;                                       // 128 not used
        const char* Rank[8];                                        // 129 - 136
        //uint32_t RankFlags;                                       // 137 not used
        //const char* Description[8];                               // 138 - 145 not used
        //uint32_t DescriptionFlags;                                // 146 not used
        //const char* BuffDescription[8];                           // 147 - 154 not used
        //uint32_t buffdescflags;                                   // 155 not used
        uint32_t ManaCostPercentage;                                // 156
        uint32_t StartRecoveryCategory;                             // 157
        uint32_t StartRecoveryTime;                                 // 158
        uint32_t MaxTargetLevel;                                    // 159
        uint32_t SpellFamilyName;                                   // 160
        uint32_t SpellFamilyFlags[2];                               // 161 - 162
        uint32_t MaxTargets;                                        // 163
        uint32_t DmgClass;                                          // 164
        uint32_t PreventionType;                                    // 165
        //int32_t StanceBarOrder;                                   // 166 not used
        float EffectDamageMultiplier[MAX_SPELL_EFFECTS];            // 167 - 169
        //uint32_t MinFactionID;                                    // 170 not used
        //uint32_t MinReputation;                                   // 171 not used
        //uint32_t RequiredAuraVision;                              // 172 not used
    };
#pragma pack(pop)
#endif

#if VERSION_STRING == TBC
    struct SpellEntry
    {
        uint32_t Id;                                                // 0
        uint32_t Category;                                          // 1
        //uint32_t castUI;                                          // 2 not used
        uint32_t DispelType;                                        // 3
        uint32_t MechanicsType;                                     // 4
        uint32_t Attributes;                                        // 5
        uint32_t AttributesEx;                                      // 6
        uint32_t AttributesExB;                                     // 7
        uint32_t AttributesExC;                                     // 8
        uint32_t AttributesExD;                                     // 9
        uint32_t AttributesExE;                                     // 10
        uint32_t AttributesExF;                                     // 11
        uint32_t Shapeshifts;                                       // 12
        uint32_t ShapeshiftsExcluded;                               // 13
        uint32_t Targets;                                           // 14
        uint32_t TargetCreatureType;                                // 15
        uint32_t RequiresSpellFocus;                                // 16
        uint32_t FacingCasterFlags;                                 // 17
        uint32_t CasterAuraState;                                   // 18
        uint32_t TargetAuraState;                                   // 19
        uint32_t CasterAuraStateNot;                                // 20
        uint32_t TargetAuraStateNot;                                // 21
        uint32_t CastingTimeIndex;                                  // 22
        uint32_t RecoveryTime;                                      // 23
        uint32_t CategoryRecoveryTime;                              // 24
        uint32_t InterruptFlags;                                    // 25
        uint32_t AuraInterruptFlags;                                // 26
        uint32_t ChannelInterruptFlags;                             // 27
        uint32_t procFlags;                                         // 28
        uint32_t procChance;                                        // 29
        uint32_t procCharges;                                       // 30
        uint32_t maxLevel;                                          // 31
        uint32_t baseLevel;                                         // 32
        uint32_t spellLevel;                                        // 33
        uint32_t DurationIndex;                                     // 34
        int32_t powerType;                                          // 35
        uint32_t manaCost;                                          // 36
        uint32_t manaCostPerlevel;                                  // 37
        uint32_t manaPerSecond;                                     // 38
        uint32_t manaPerSecondPerLevel;                             // 39
        uint32_t rangeIndex;                                        // 40
        float speed;                                                // 41
        //uint32_t modalNextSpell;                                  // 42 not used
        uint32_t MaxStackAmount;                                    // 43
        uint32_t Totem[MAX_SPELL_TOTEMS];                           // 44 - 45
        int32_t Reagent[MAX_SPELL_REAGENTS];                        // 46 - 53
        uint32_t ReagentCount[MAX_SPELL_REAGENTS];                  // 54 - 61
        int32_t EquippedItemClass;                                  // 62
        int32_t EquippedItemSubClass;                               // 63
        int32_t EquippedItemInventoryTypeMask;                      // 64
        uint32_t Effect[MAX_SPELL_EFFECTS];                         // 65 - 67
        int32_t EffectDieSides[MAX_SPELL_EFFECTS];                  // 68 - 70
        uint32_t EffectBaseDice[MAX_SPELL_EFFECTS];                 // 71 - 73
        float EffectDicePerLevel[MAX_SPELL_EFFECTS];                // 74 - 76
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
        uint32_t SpellVisual;                                       // 122
        uint32_t SpellVisual1;                                      // 123
        uint32_t spellIconID;                                       // 124
        uint32_t activeIconID;                                      // 125 activeIconID;
        uint32_t spellPriority;                                     // 126
        const char* Name[16];                                       // 127 - 142
        //uint32_t NameFlags;                                       // 143 not used
        const char* Rank[16];                                       // 144 - 159
        //uint32_t RankFlags;                                       // 160 not used
        //const char* Description[16];                              // 161 - 176 not used
        //uint32_t DescriptionFlags;                                // 177 not used
        //const char* BuffDescription[16];                          // 178 - 193 not used
        //uint32_t buffdescflags;                                   // 194 not used
        uint32_t ManaCostPercentage;                                // 195
        uint32_t StartRecoveryCategory;                             // 196
        uint32_t StartRecoveryTime;                                 // 197
        uint32_t MaxTargetLevel;                                    // 198
        uint32_t SpellFamilyName;                                   // 199
        uint32_t SpellFamilyFlags[2];                               // 200 - 201
        uint32_t MaxTargets;                                        // 202
        uint32_t DmgClass;                                          // 203
        uint32_t PreventionType;                                    // 204
        //int32_t StanceBarOrder;                                   // 205 not used
        float EffectDamageMultiplier[MAX_SPELL_EFFECTS];            // 206 - 208
        //uint32_t MinFactionID;                                    // 209 not used
        //uint32_t MinReputation;                                   // 210 not used
        //uint32_t RequiredAuraVision;                              // 211 not used
        uint32_t TotemCategory[MAX_SPELL_TOTEM_CATEGORIES];         // 212 - 213
        int32_t AreaGroupId;                                        // 214
        uint32_t School;                                            // 215
    };
#pragma pack(pop)
#endif

#if VERSION_STRING == WotLK
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
        uint32_t SpellVisual1;                                      // 132
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
#pragma pack(pop)

#endif
#if VERSION_STRING == Cata

    struct SERVER_DECL SpellEntry
    {
        uint32_t Id;                                                // 0
        uint32_t Attributes;                                        // 1
        uint32_t AttributesEx;                                      // 2
        uint32_t AttributesExB;                                     // 3
        uint32_t AttributesExC;                                     // 4
        uint32_t AttributesExD;                                     // 5
        uint32_t AttributesExE;                                     // 6
        uint32_t AttributesExF;                                     // 7
        uint32_t AttributesExG;                                     // 8
        uint32_t AttributesExH;                                     // 9
        uint32_t AttributesExI;                                     // 10
        uint32_t AttributesExJ;                                     // 11
        uint32_t CastingTimeIndex;                                  // 12
        uint32_t DurationIndex;                                     // 13
        int32_t powerType;                                          // 14
        uint32_t rangeIndex;                                        // 15
        float speed;                                                // 16
        uint32_t SpellVisual;                                       // 17
        uint32_t SpellVisual1;                                      // 18
        uint32_t spellIconID;                                       // 19
        uint32_t activeIconID;                                      // 20
        const char* Name;                                           // 21
        const char* Rank;                                           // 22
        //char* Description;                                        // 23 not used
        //char* BuffDescription;                                    // 24 not used
        uint32_t School;                                            // 25
        uint32_t RuneCostID;                                        // 26
        //uint32_t spellMissileID;                                  // 27
        //uint32_t spellDescriptionVariableID;                      // 28
        uint32_t SpellDifficultyId;                                 // 29
        //float unk_1;                                              // 30
        uint32_t SpellScalingId;                                    // 31 SpellScaling.dbc
        uint32_t SpellAuraOptionsId;                                // 32 SpellAuraOptions.dbc
        uint32_t SpellAuraRestrictionsId;                           // 33 SpellAuraRestrictions.dbc
        uint32_t SpellCastingRequirementsId;                        // 34 SpellCastingRequirements.dbc
        uint32_t SpellCategoriesId;                                 // 35 SpellCategories.dbc
        uint32_t SpellClassOptionsId;                               // 36 SpellClassOptions.dbc
        uint32_t SpellCooldownsId;                                  // 37 SpellCooldowns.dbc
        //uint32_t unk_2;                                           // 38 all zeros...
        uint32_t SpellEquippedItemsId;                              // 39 SpellEquippedItems.dbc
        uint32_t SpellInterruptsId;                                 // 40 SpellInterrupts.dbc
        uint32_t SpellLevelsId;                                     // 41 SpellLevels.dbc
        uint32_t SpellPowerId;                                      // 42 SpellPower.dbc
        uint32_t SpellReagentsId;                                   // 43 SpellReagents.dbc
        uint32_t SpellShapeshiftId;                                 // 44 SpellShapeshift.dbc
        uint32_t SpellTargetRestrictionsId;                         // 45 SpellTargetRestrictions.dbc
        uint32_t SpellTotemsId;                                     // 46 SpellTotems.dbc
        //uint32_t ResearchProject;                                 // 47 ResearchProject.dbc

        // struct access functions
        SpellAuraOptionsEntry const* GetSpellAuraOptions() const;
        SpellAuraRestrictionsEntry const* GetSpellAuraRestrictions() const;
        SpellCastingRequirementsEntry const* GetSpellCastingRequirements() const;
        SpellCategoriesEntry const* GetSpellCategories() const;
        SpellClassOptionsEntry const* GetSpellClassOptions() const;
        SpellCooldownsEntry const* GetSpellCooldowns() const;
        SpellEffectEntry const* GetSpellEffect(uint8_t eff) const;
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
        uint32_t GetSpellFamilyName() const;
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
        uint32_t GetSpellEffectIdByIndex(uint8_t index) const;
        uint32_t GetAuraInterruptFlags() const;
        uint32_t GetEffectImplicitTargetAByIndex(uint8_t index) const;
        int32_t GetAreaGroupId() const;
        uint32_t GetFacingCasterFlags() const;
        uint32_t GetBaseLevel() const;
        uint32_t GetInterruptFlags() const;
        uint32_t GetTargetCreatureType() const;
        int32_t GetEffectMiscValue(uint8_t index) const;
        uint32_t GetStances() const;
        uint32_t GetStancesNot() const;
        uint32_t GetProcFlags() const;
        uint32_t GetChannelInterruptFlags() const;
        uint32_t GetManaCostPerLevel() const;
        uint32_t GetCasterAuraState() const;
        uint32_t GetTargets() const;
        uint32_t GetEffectApplyAuraNameByIndex(uint8_t index) const;

    private:
        SpellEntry(SpellEntry const&);
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
#endif
#if VERSION_STRING == Mop

    struct SERVER_DECL SpellEntry
    {
        uint32_t Id;                                                // 0
        const char* Name;                                           // 21
        const char* Rank;                                           // 22
        //char* Description;                                        // 23 not used
        //char* BuffDescription;                                    // 24 not used
        uint32_t RuneCostID;                                        // 26
        //uint32_t spellMissileID;                                  // 27
        //uint32_t spellDescriptionVariableID;                      // 28
        float unk_1;                                                // 30
        uint32_t SpellScalingId;                                    // 31 SpellScaling.dbc
        uint32_t SpellAuraOptionsId;                                // 32 SpellAuraOptions.dbc
        uint32_t SpellAuraRestrictionsId;                           // 33 SpellAuraRestrictions.dbc
        uint32_t SpellCastingRequirementsId;                        // 34 SpellCastingRequirements.dbc
        uint32_t SpellCategoriesId;                                 // 35 SpellCategories.dbc
        uint32_t SpellClassOptionsId;                               // 36 SpellClassOptions.dbc
        uint32_t SpellCooldownsId;                                  // 37 SpellCooldowns.dbc
        uint32_t SpellEquippedItemsId;                              // 39 SpellEquippedItems.dbc
        uint32_t SpellInterruptsId;                                 // 40 SpellInterrupts.dbc
        uint32_t SpellLevelsId;                                     // 41 SpellLevels.dbc
        //uint32_t SpellPowerId;                                    // 42 SpellPower.dbc
        uint32_t SpellReagentsId;                                   // 43 SpellReagents.dbc
        uint32_t SpellShapeshiftId;                                 // 44 SpellShapeshift.dbc
        uint32_t SpellTargetRestrictionsId;                         // 45 SpellTargetRestrictions.dbc
        uint32_t SpellTotemsId;                                     // 46 SpellTotems.dbc
        uint32_t ResearchProject;                                   // 47 ResearchProject.dbc
        uint32_t SpellMiscId;                                       // 24 SpellMisc.dbc

        // struct access functions
        SpellAuraOptionsEntry const* GetSpellAuraOptions() const;
        SpellAuraRestrictionsEntry const* GetSpellAuraRestrictions() const;
        SpellCastingRequirementsEntry const* GetSpellCastingRequirements() const;
        SpellCategoriesEntry const* GetSpellCategories() const;
        SpellClassOptionsEntry const* GetSpellClassOptions() const;
        SpellCooldownsEntry const* GetSpellCooldowns() const;
        SpellEffectEntry const* GetSpellEffect(uint8_t eff) const;
        SpellEquippedItemsEntry const* GetSpellEquippedItems() const;
        SpellInterruptsEntry const* GetSpellInterrupts() const;
        SpellLevelsEntry const* GetSpellLevels() const;
        SpellPowerEntry const* GetSpellPower() const;
        //SpellReagentsEntry const* GetSpellReagents() const;
        SpellScalingEntry const* GetSpellScaling() const;
        SpellShapeshiftEntry const* GetSpellShapeshift() const;
        SpellTargetRestrictionsEntry const* GetSpellTargetRestrictions() const;
        SpellTotemsEntry const* GetSpellTotems() const;
        SpellMiscEntry const* GetSpellMisc() const;

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
        uint32_t GetSpellFamilyName() const;
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
        uint32_t GetSpellEffectIdByIndex(uint8_t index) const;
        uint32_t GetAuraInterruptFlags() const;
        uint32_t GetEffectImplicitTargetAByIndex(uint8_t index) const;
        int32_t GetAreaGroupId() const;
        uint32_t GetFacingCasterFlags() const;
        uint32_t GetBaseLevel() const;
        uint32_t GetInterruptFlags() const;
        uint32_t GetTargetCreatureType() const;
        int32_t GetEffectMiscValue(uint8_t index) const;
        uint32_t GetStances() const;
        uint32_t GetStancesNot() const;
        uint32_t GetProcFlags() const;
        uint32_t GetChannelInterruptFlags() const;
        uint32_t GetManaCostPerLevel() const;
        uint32_t GetCasterAuraState() const;
        uint32_t GetTargets() const;
        uint32_t GetEffectApplyAuraNameByIndex(uint8_t index) const;

    private:
        SpellEntry(SpellEntry const&);
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
        SpellEffectEntry const* effects[32];
    };
    typedef std::map<uint32_t, SpellEffect> SpellEffectMap;
#endif
}
