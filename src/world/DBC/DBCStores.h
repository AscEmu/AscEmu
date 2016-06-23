/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2011 <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DBC_STORES_H
#define _DBC_STORES_H

#include "DBCGlobals.hpp"
#include "Definitions.h"

class Player;

#pragma pack(push,1)

#ifdef ENABLE_ACHIEVEMENTS

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

#endif

#define MAX_SPELL_EFFECTS 3

// Struct for the entry in Spell.dbc
struct SpellEntry
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

    /// CUSTOM: these fields are used for the modifications made in the world.cpp
    uint32 custom_DiminishStatus;
    uint32 custom_proc_interval;
                                                                    /// Buff Groupin Rule -> caster can cast this spell only on 1 target. Value represents the group spell is part of. Can be part of only 1 group
                                                                    /// target can have only buff of this type on self. Value represents the group spell is part of. Can be part of only 1 group
    uint32 custom_BGR_one_buff_on_target;                           /// these are related to creating a item through a spell caster can have only 1 Aura per spell group, ex pal auras
    uint32 custom_BGR_one_buff_from_caster_on_self;                 /// these are related to creating a item through a spell
    uint32 custom_c_is_flags;                                       /// store spell checks in a static way : isdamageind,ishealing
    uint32 custom_RankNumber;                                       /// this protects players from having >1 rank of a spell
    uint32 custom_NameHash;                                         /// related to custom spells, summon spell quest related spells
    uint32 custom_ThreatForSpell;
    float custom_ThreatForSpellCoef;
    uint32 custom_ProcOnNameHash[MAX_SPELL_EFFECTS];
    uint32 custom_spell_coef_flags;                                 /// store flags for spell coefficient calculations

    float custom_base_range_or_radius_sqr;                                 /// needed for aoe spells most of the time
    /// love me or hate me, all "In a cone in front of the caster" spells don't necessarily mean "in front"
    float cone_width;
    float casttime_coef;                                    /// CUSTOM, faster spell bonus calculation
    float fixed_dddhcoef;                                   /// CUSTOM, fixed DD-DH coefficient for some spells
    float fixed_hotdotcoef;                                 /// CUSTOM, fixed HOT-DOT coefficient for some spells
    float Dspell_coef_override;                             /// CUSTOM, overrides any spell coefficient calculation and use this value in DD&DH
    float OTspell_coef_override;                            /// CUSTOM, overrides any spell coefficient calculation and use this value in HOT&DOT
    int ai_target_type;

    bool custom_self_cast_only;
    bool custom_apply_on_shapeshift_change;
    bool custom_always_apply;
    bool custom_is_melee_spell;
    bool custom_is_ranged_spell;

	bool CheckLocation(uint32 map_id, uint32 zone_id, uint32 area_id, Player* player = NULL);

    uint32 custom_SchoolMask;
    uint32 CustomFlags;                                     /// Custom
    uint32 EffectCustomFlag[MAX_SPELL_EFFECTS];             /// Custom

    /// Pointer to static method of a Spell subclass to create a new instance. If this is NULL, the generic Spell class will be created
    /// Its type is void because class Spell is not visible here, so it'll be casted accordingly when necessary
    void* (*SpellFactoryFunc);

    /// Same for Auras
    void* (*AuraFactoryFunc);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// \note bool HasEffect   - Tells if the Spell has a certain effect
    ///
    /// \param uint32 effect  -  Effect Identifier
    ///
    /// \returns true if Spell has this effect, false if Spell has not this effect.
    ///
    //////////////////////////////////////////////////////////////////////////////////////////
    bool HasEffect(uint32 effect)
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            if (Effect[i] == effect)
                return true;

        return false;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// \note bool HasCustomFlagForEffect - Tells if the Spell has this flag for this effect
    ///
    /// \param uint32 effect  -  The effect index
    /// \param uint32 flag    -  Flag that we are checking
    ///
    /// \returns true if we have the flag, false if we don't.
    ///
    //////////////////////////////////////////////////////////////////////////////////////////
    bool HasCustomFlagForEffect(uint32 effect, uint32 flag)
    {
        if (effect >= MAX_SPELL_EFFECTS)
            return false;

        if ((EffectCustomFlag[effect] & flag) != 0)
            return true;
        else
            return false;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// \note bool AppliesAura  - Tells if the Spell applies this Aura
    ///
    /// \param uint32 aura - Aura id
    ///
    /// \returns true if the Spell applies this Aura, false otherwise.
    ///
    //////////////////////////////////////////////////////////////////////////////////////////
    bool AppliesAura(uint32 aura)
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {

            if ((Effect[i] == 6 ||        /// SPELL_EFFECT_APPLY_GROUP_AREA_AURA
                    Effect[i] == 27 ||    /// SPELL_EFFECT_PERSISTENT_AREA_AURA
                    Effect[i] == 35 ||    /// SPELL_EFFECT_APPLY_GROUP_AREA_AURA
                    Effect[i] == 65 ||    /// SPELL_EFFECT_APPLY_RAID_AREA_AURA
                    Effect[i] == 119 ||   /// SPELL_EFFECT_APPLY_PET_AREA_AURA
                    Effect[i] == 128 ||   /// SPELL_EFFECT_APPLY_FRIEND_AREA_AURA
                    Effect[i] == 129 ||   /// SPELL_EFFECT_APPLY_ENEMY_AREA_AURA
                    Effect[i] == 143) &&  /// SPELL_EFFECT_APPLY_OWNER_AREA_AURA
                    EffectApplyAuraName[i] == aura)
                return true;
        }

        return false;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// \note uint32 GetAAEffectId()  - Returns the Effect Id of the Area Aura effect if the spell has one.
    ///
    /// \param none
    ///
    /// \returns the Effect Id of the Area Aura effect if the spell has one, 0 otherwise.
    ///
    //////////////////////////////////////////////////////////////////////////////////////////
    uint32 GetAAEffectId()
    {

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; i++)
        {

            if (Effect[i] == 35 ||        /// SPELL_EFFECT_APPLY_GROUP_AREA_AURA
                    Effect[i] == 65 ||    /// SPELL_EFFECT_APPLY_RAID_AREA_AURA
                    Effect[i] == 119 ||   /// SPELL_EFFECT_APPLY_PET_AREA_AURA
                    Effect[i] == 128 ||   /// SPELL_EFFECT_APPLY_FRIEND_AREA_AURA
                    Effect[i] == 129 ||   /// SPELL_EFFECT_APPLY_ENEMY_AREA_AURA
                    Effect[i] == 143)     /// SPELL_EFFECT_APPLY_OWNER_AREA_AURA
                return Effect[i];
        }

        return 0;
    }

    SpellEntry()
    {
        Id = 0;
        Category = 0;
        DispelType = 0;
        MechanicsType = 0;
        Attributes = 0;
        AttributesEx = 0;
        AttributesExB = 0;
        AttributesExC = 0;
        AttributesExD = 0;
        AttributesExE = 0;
        AttributesExF = 0;
        AttributesExG = 0;
        RequiredShapeShift = 0;
        ShapeshiftExclude = 0;
        Targets = 0;
        TargetCreatureType = 0;
        RequiresSpellFocus = 0;
        FacingCasterFlags = 0;
        CasterAuraState = 0;
        TargetAuraState = 0;
        CasterAuraStateNot = 0;
        TargetAuraStateNot = 0;
        casterAuraSpell = 0;
        targetAuraSpell = 0;
        casterAuraSpellNot = 0;

        CustomFlags = 0;

        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; i++)
            EffectCustomFlag[i] = 0;

        SpellFactoryFunc = NULL;
        AuraFactoryFunc = NULL;
        custom_proc_interval = 0;
        custom_BGR_one_buff_on_target = 0;
        custom_BGR_one_buff_from_caster_on_self = 0;
        custom_c_is_flags = 0;
        custom_RankNumber = 0;
        custom_NameHash = 0;
        custom_ThreatForSpell = 0;
        custom_ThreatForSpellCoef = 0;
        custom_spell_coef_flags = 0;
        custom_base_range_or_radius_sqr = 0;
        cone_width = 0;
        casttime_coef = 0;
        fixed_dddhcoef = 0;
        fixed_hotdotcoef = 0;
        Dspell_coef_override = 0;
        OTspell_coef_override = 0;
        ai_target_type = 0;
        custom_self_cast_only = false;
        custom_apply_on_shapeshift_change = false;
        custom_always_apply = false;
        custom_is_melee_spell = false;
        custom_is_ranged_spell = false;

        custom_SchoolMask = 0;
        SpellVisual = 0;
        field114 = 0;
        spellIconID = 0;
        activeIconID = 0;
        spellPriority = 0;
        Name = nullptr;
        Rank = nullptr;
        Description = 0;
        BuffDescription = 0;
        ManaCostPercentage = 0;
        StartRecoveryCategory = 0;
        StartRecoveryTime = 0;
        MaxTargetLevel = 0;
        SpellFamilyName = 0;
        MaxTargets = 0;
        Spell_Dmg_Type = 0;
        PreventionType = 0;
        StanceBarOrder = 0;
        MinFactionID = 0;
        MinReputation = 0;
        RequiredAuraVision = 0;
        RequiresAreaId = 0;
        School = 0;
        RuneCostID = 0;
        SpellDifficultyID = 0;
        custom_DiminishStatus = 0;
        targetAuraSpellNot = 0;
        CastingTimeIndex = 0;
        RecoveryTime = 0;
        CategoryRecoveryTime = 0;
        InterruptFlags = 0;
        AuraInterruptFlags = 0;
        ChannelInterruptFlags = 0;
        procFlags = 0;
        procChance = 0;
        procCharges = 0;
        maxLevel = 0;
        baseLevel = 0;
        spellLevel = 0;
        DurationIndex = 0;
        powerType = 0;
        manaCost = 0;
        manaCostPerlevel = 0;
        manaPerSecond = 0;
        manaPerSecondPerLevel = 0;
        rangeIndex = 0;
        speed = 0;
        modalNextSpell = 0;
        maxstack = 0;
        EquippedItemClass = 0;
        EquippedItemSubClass = 0;
        RequiredItemFlags = 0;
    }
};

struct Trainerspell
{
    uint32 Id;
    uint32 skilline1;
    uint32 skilline2;
    uint32 skilline3;
    uint32 maxlvl;
    uint32 charclass;
};

#pragma pack(pop)

inline float GetRadius(DBC::Structures::SpellRadiusEntry const* radius)
{
    if (radius == nullptr)
        return 0;

    return radius->radius_min;
}
inline uint32 GetCastTime(DBC::Structures::SpellCastTimesEntry const* time)
{
    if (time == nullptr)
        return 0;

    return time->CastTime;
}
inline float GetMaxRange(DBC::Structures::SpellRangeEntry const* range)
{
    if (range == nullptr)
        return 0;

    return range->maxRange;
}
inline float GetMinRange(DBC::Structures::SpellRangeEntry const* range)
{
    if (range == nullptr)
        return 0;

    return range->minRange;
}
inline uint32 GetDuration(DBC::Structures::SpellDurationEntry const* dur)
{
    if (dur == nullptr)
        return 0;
    return dur->Duration1;
}

#define SAFE_DBC_CODE_RETURNS       /// undefine this to make out of range/nulls return null. */

template<class T>
class SERVER_DECL DBCStorage
{
        T* m_heapBlock;
        T* m_firstEntry;

        T** m_entries;
        uint32 m_max;
        uint32 m_numrows;
        uint32 m_stringlength;
        char* m_stringData;

        uint32 rows;
        uint32 cols;
        uint32 useless_shit;
        uint32 header;

    public:

        class iterator
        {
            private:
                T* p;
            public:
                iterator(T* ip = 0) : p(ip) { }
                iterator & operator++() { ++p; return *this; }
                iterator & operator--() { --p; return *this; }
                bool operator!=(const iterator & i) { return (p != i.p); }
                T* operator*() { return p; }
        };

        iterator begin()
        {
            return iterator(&m_heapBlock[0]);
        }
        iterator end()
        {
            return iterator(&m_heapBlock[m_numrows]);
        }

        DBCStorage()
        {
            m_heapBlock = NULL;
            m_entries = NULL;
            m_firstEntry = NULL;
            m_max = 0;
            m_numrows = 0;
            m_stringlength = 0;
            m_stringData = NULL;
            rows = 0;
            cols = 0;
            useless_shit = 0;
            header = 0;
        }

        ~DBCStorage()
        {
            Cleanup();
        }

        void Cleanup()
        {
            if (m_heapBlock)
            {
                free(m_heapBlock);
                m_heapBlock = NULL;
            }
            if (m_entries)
            {
                free(m_entries);
                m_entries = NULL;
            }
            if (m_stringData != NULL)
            {
                free(m_stringData);
                m_stringData = NULL;
            }
        }

        bool Load(const char* filename, const char* format, bool load_indexed, bool load_strings)
        {
            uint32 i;
            uint32 string_length;
            int pos;

            FILE* f = fopen(filename, "rb");
            if (f == NULL)
                return false;

            // read the number of rows, and allocate our block on the heap
            if (fread(&header, 4, 1, f) != 1)
            {
                fclose(f);
                return false;
            }
            if (fread(&rows, 4, 1, f) != 1)
            {
                fclose(f);
                return false;
            }
            if (fread(&cols, 4, 1, f) != 1)
            {
                fclose(f);
                return false;
            }
            if (fread(&useless_shit, 4, 1, f) != 1)
            {
                fclose(f);
                return false;
            }
            if (fread(&string_length, 4, 1, f) != 1)
            {
                fclose(f);
                return false;
            }
            pos = ftell(f);

            if (load_strings)
            {
                if (fseek(f, 20 + (rows * cols * 4), SEEK_SET) != 0)
                {
                    fclose(f);
                    return false;
                }

                m_stringData = (char*)malloc(string_length);
                m_stringlength = string_length;
                if (m_stringData)
                {
                    if (fread(m_stringData, string_length, 1, f) != 1)
                    {
                        fclose(f);
                        return false;
                    }
                }
            }

            if (fseek(f, pos, SEEK_SET) != 0)
            {
                fclose(f);
                return false;
            }

            m_heapBlock = (T*)malloc(rows * sizeof(T));
            ASSERT(m_heapBlock);

            // read the data for each row
            for (i = 0; i < rows; ++i)
            {
                memset(&m_heapBlock[i], 0, sizeof(T));
                ReadEntry(f, &m_heapBlock[i], format, cols, filename);

                if (load_indexed)
                {
                    // all the time the first field in the dbc is our unique entry
                    if (*(uint32*)&m_heapBlock[i] > m_max)
                        m_max = *(uint32*)&m_heapBlock[i];
                }
            }

            if (load_indexed)
            {
                m_entries = (T**)malloc(sizeof(T*) * (m_max + 1));
                ASSERT(m_entries);

                memset(m_entries, 0, (sizeof(T*) * (m_max + 1)));
                for (i = 0; i < rows; ++i)
                {
                    if (m_firstEntry == NULL)
                        m_firstEntry = &m_heapBlock[i];

                    m_entries[*(uint32*)&m_heapBlock[i]] = &m_heapBlock[i];
                }
            }

            m_numrows = rows;

            fclose(f);
            return true;
        }

        void ReadEntry(FILE* f, T* dest, const char* format, uint32 cols, const char* filename)
        {
            const char* t = format;
            uint32* dest_ptr = (uint32*)dest;
            uint32 c = 0;
            uint32 val;
            size_t len = strlen(format);
            if (len != cols)
                sLog.outError("!!! possible invalid format in file %s (us: %u, them: %u)", filename, len, cols);

            while (*t != 0)
            {
                if ((++c) > cols)
                {
                    ++t;
                    sLog.outError("!!! Read buffer overflow in DBC reading of file %s", filename);
                    continue;
                }

                if (fread(&val, 4, 1, f) != 1)
                {
                    ++t;
                    continue;
                }

                if (*t == 'x')
                {
                    ++t;
                    continue;        // skip!
                }

                if ((*t == 's') || (*t == 'l'))
                {
                    char** new_ptr = (char**)dest_ptr;
                    static const char* null_str = "";
                    char* ptr;
                    // if t == 'lxxxxxxxxxxxxxxxx' use localized strings in case
                    // the english one is empty. *t ends at most on the locale flag
                    for (int count = (*t == 'l') ? 16 : 0 ;
                            val == 0 && count > 0 && *(t + 1) == 'x'; t++, count--)
                    {
                        fread(&val, 4, 1, f);

                    }
                    if (val < m_stringlength)
                        ptr = m_stringData + val;
                    else
                        ptr = (char*)null_str;

                    *new_ptr = ptr;
                    new_ptr++;
                    dest_ptr = (uint32*)new_ptr;
                }
                else
                {
                    *dest_ptr = val;
                    dest_ptr++;
                }

                ++t;
            }
        }

        inline uint32 GetNumRows()
        {
            return m_numrows;
        }

        T* LookupEntryForced(uint32 i)
        {
#if 0
            if (m_entries)
            {
                if (i > m_max || m_entries[i] == NULL)
                {
                    printf("LookupEntryForced failed for entry %u\n", i);
                    return NULL;
                }
                else
                    return m_entries[i];
            }
            else
            {
                if (i >= m_numrows)
                    return NULL;
                else
                    return &m_heapBlock[i];
            }
#else
            if (m_entries)
            {
                if (i > m_max || m_entries[i] == NULL)
                    return NULL;
                else
                    return m_entries[i];
            }
            else
            {
                if (i >= m_numrows)
                    return NULL;
                else
                    return &m_heapBlock[i];
            }
#endif
        }

        T* LookupRowForced(uint32 i)
        {
            if (i >= m_numrows)
                return NULL;
            else
                return &m_heapBlock[i];
        }

        T* CreateCopy(T* obj)
        {
            T* oCopy = (T*)malloc(sizeof(T));
            ASSERT(oCopy);
            memcpy(oCopy, obj, sizeof(T));
            return oCopy;
        }
        void SetRow(uint32 i, T* t)
        {
            if (i < m_max && m_entries)
                m_entries[i] = t;
        }

        T* LookupEntry(uint32 i)
        {
            if (m_entries)
            {
                if (i > m_max || m_entries[i] == NULL)
                    return m_firstEntry;
                else
                    return m_entries[i];
            }
            else
            {
                if (i >= m_numrows)
                    return &m_heapBlock[0];
                else
                    return &m_heapBlock[i];
            }
        }

        T* LookupRow(uint32 i)
        {
            if (i >= m_numrows)
                return &m_heapBlock[0];
            else
                return &m_heapBlock[i];
        }
};

extern SERVER_DECL DBC::DBCStorage<DBC::Structures::WorldMapOverlayEntry> sWorldMapOverlayStore;

#ifdef ENABLE_ACHIEVEMENTS
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AchievementEntry> sAchievementStore;
extern SERVER_DECL DBC::DBCStorage<AchievementCriteriaEntry> sAchievementCriteriaStore;
#endif

extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CharTitlesEntry> sCharTitlesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CurrencyTypesEntry> sCurrencyTypesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::BarberShopStyleEntry> sBarberShopStyleStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GemPropertiesEntry> sGemPropertiesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GlyphPropertiesEntry> sGlyphPropertiesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GlyphSlotEntry> sGlyphSlotStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemEntry> sItemStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemSetEntry> sItemSetStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::LockEntry> sLockStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellEntry_New> sSpellStore;
///\todo remove the old spell loader
extern SERVER_DECL DBCStorage<SpellEntry> dbcSpell;

extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellDifficultyEntry> sSpellDifficultyStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellDurationEntry> sSpellDurationStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRangeEntry> sSpellRangeStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellShapeshiftFormEntry> sSpellShapeshiftFormStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::EmotesTextEntry> sEmotesTextStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRadiusEntry> sSpellRadiusStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellCastTimesEntry> sSpellCastTimesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaGroupEntry> sAreaGroupStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTableEntry> sAreaStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::FactionTemplateEntry> sFactionTemplateStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::FactionEntry> sFactionStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GameObjectDisplayInfoEntry> sGameObjectDisplayInfoStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellItemEnchantmentEntry> sSpellItemEnchantmentStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemRandomPropertiesEntry> sItemRandomPropertiesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SkillLineAbilityEntry> sSkillLineAbilityStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SkillLineEntry> sSkillLineStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiNodesEntry> sTaxiNodesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiPathEntry> sTaxiPathStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiPathNodeEntry> sTaxiPathNodeStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AuctionHouseEntry> sAuctionHouseStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentEntry> sTalentStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentTabEntry> sTalentTabStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureSpellDataEntry> sCreatureSpellDataStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureFamilyEntry> sCreatureFamilyStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrClassesEntry> sChrClassesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrRacesEntry> sChrRacesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::MapEntry> sMapStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::HolidaysEntry> sHolidaysStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRuneCostEntry> sSpellRuneCostStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemExtendedCostEntry> sItemExtendedCostStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemRandomSuffixEntry> sItemRandomSuffixStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtCombatRatingsEntry> sGtCombatRatingsStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ChatChannelsEntry> sChatChannelsStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::DurabilityCostsEntry> sDurabilityCostsStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::DurabilityQualityEntry> sDurabilityQualityStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::BankBagSlotPrices> sBankBagSlotPricesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::StableSlotPrices> sStableSlotPricesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtBarberShopCostBaseEntry> sBarberShopCostBaseStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToMeleeCritEntry> sGtChanceToMeleeCritStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToSpellCritEntry> sGtChanceToSpellCritStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTRegenMPEntry> sGtOCTRegenMPStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtRegenMPPerSptEntry> sGtRegenMPPerSptStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTRegenHPEntry> sGtOCTRegenHPStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtRegenHPPerSptEntry> sGtRegenHPPerSptStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTriggerEntry> sAreaTriggerStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ScalingStatDistributionEntry> sScalingStatDistributionStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ScalingStatValuesEntry> sScalingStatValuesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemLimitCategoryEntry> sItemLimitCategoryStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::QuestXP> sQuestXPStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::MailTemplateEntry> sMailTemplateStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::WMOAreaTableEntry> sWMOAreaTableStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SummonPropertiesEntry> sSummonPropertiesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::NameGenEntry> sNameGenStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::LFGDungeonEntry> sLFGDungeonStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::LiquidTypeEntry> sLiquidTypeStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::VehicleEntry> sVehicleStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::VehicleSeatEntry> sVehicleSeatStore;

bool LoadDBCs();

DBC::Structures::WMOAreaTableEntry const* GetWMOAreaTableEntryByTriple(int32 root_id, int32 adt_id, int32 group_id);

#endif // _DBC_STORES_H
