/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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
 *
 */

#include "StdAfx.h"
#include "Management/QuestLogEntry.hpp"
#include "MMapFactory.h"
#include "Units/Creatures/Creature.h"
#include "Units/Summons/Summon.h"
#include "Objects/DynamicObject.h"
#include "Server/Packets/Handlers/HonorHandler.h"
#include "Management/Item.h"
#include "Management/Container.h"
#include "Management/TaxiMgr.h"
#include "Management/ItemInterface.h"
#include "Units/Stats.h"
#include "Management/Battleground/Battleground.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Players/PlayerClasses.hpp"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Objects/Faction.h"
#include "SpellMgr.h"
#include "SpellAuras.h"
#include "Definitions/SpellCastTargetFlags.h"
#include "Definitions/SpellDamageType.h"
#include "Definitions/ProcFlags.h"
#include "Definitions/CastInterruptFlags.h"
#include "Definitions/AuraInterruptFlags.h"
#include "Definitions/ChannelInterruptFlags.h"
#include "Definitions/PreventionType.h"
#include "Definitions/SpellRanged.h"
#include "Definitions/LockTypes.h"
#include "Definitions/SpellIsFlags.h"
#include "Definitions/TeleportEffectCustomFlags.h"
#include "Definitions/SummonControlTypes.h"
#include "Definitions/SummonTypes.h"
#include "Definitions/SpellState.h"
#include "Definitions/DispelType.h"
#include "Definitions/SpellMechanics.h"
#include "Definitions/PowerType.h"
#include "Definitions/Spec.h"
#include "SpellHelpers.h"
#include "Spell.h"
#include "Customization/SpellCustomizations.hpp"
#include "Units/Creatures/Pet.h"

using ascemu::World::Spell::Helpers::spellModFlatIntValue;
using ascemu::World::Spell::Helpers::spellModPercentageIntValue;
using ascemu::World::Spell::Helpers::spellModFlatFloatValue;
using ascemu::World::Spell::Helpers::spellModPercentageFloatValue;

pSpellEffect SpellEffectsHandler[TOTAL_SPELL_EFFECTS] =
{
    &Spell::SpellEffectNULL,                    //   0 SPELL_EFFECT_NULL
    &Spell::SpellEffectInstantKill,             //   1 SPELL_EFFECT_INSTAKILL
    &Spell::SpellEffectSchoolDMG,               //   2 SPELL_EFFECT_SCHOOL_DAMAGE
    &Spell::SpellEffectDummy,                   //   3 SPELL_EFFECT_DUMMY
    &Spell::SpellEffectNULL,                    //   4 SPELL_EFFECT_PORTAL_TELEPORT
    &Spell::SpellEffectTeleportUnits,           //   5 SPELL_EFFECT_TELEPORT_UNITS
    &Spell::SpellEffectApplyAura,               //   6 SPELL_EFFECT_APPLY_AURA
    &Spell::SpellEffectEnvironmentalDamage,     //   7 SPELL_EFFECT_ENVIRONMENTAL_DAMAGE
    &Spell::SpellEffectPowerDrain,              //   8 SPELL_EFFECT_POWER_DRAIN
    &Spell::SpellEffectHealthLeech,             //   9 SPELL_EFFECT_HEALTH_LEECH
    &Spell::SpellEffectHeal,                    //  10 SPELL_EFFECT_HEAL
    &Spell::SpellEffectBind,                    //  11 SPELL_EFFECT_BIND
    &Spell::SpellEffectNULL,                    //  12 SPELL_EFFECT_PORTAL
    &Spell::SpellEffectNULL,                    //  13 SPELL_EFFECT_RITUAL_BASE
    &Spell::SpellEffectNULL,                    //  14 SPELL_EFFECT_RITUAL_SPECIALIZE
    &Spell::SpellEffectNULL,                    //  15 SPELL_EFFECT_RITUAL_ACTIVATE_PORTAL
    &Spell::SpellEffectQuestComplete,           //  16 SPELL_EFFECT_QUEST_COMPLETE
    &Spell::SpellEffectWeapondamageNoschool,    //  17 SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL
    &Spell::SpellEffectResurrect,               //  18 SPELL_EFFECT_RESURRECT
    &Spell::SpellEffectAddExtraAttacks,         //  19 SPELL_EFFECT_ADD_EXTRA_ATTACKS
    &Spell::SpellEffectDodge,                   //  20 SPELL_EFFECT_DODGE
    &Spell::SpellEffectNULL,                    //  21 SPELL_EFFECT_EVADE
    &Spell::SpellEffectParry,                   //  22 SPELL_EFFECT_PARRY
    &Spell::SpellEffectBlock,                   //  23 SPELL_EFFECT_BLOCK
    &Spell::SpellEffectCreateItem,              //  24 SPELL_EFFECT_CREATE_ITEM
    &Spell::SpellEffectWeapon,                  //  25 SPELL_EFFECT_WEAPON
    &Spell::SpellEffectDefense,                 //  26 SPELL_EFFECT_DEFENSE
    &Spell::SpellEffectPersistentAA,            //  27 SPELL_EFFECT_PERSISTENT_AREA_AURA
    &Spell::SpellEffectSummon,                  //  28 SPELL_EFFECT_SUMMON
    &Spell::SpellEffectLeap,                    //  29 SPELL_EFFECT_LEAP
    &Spell::SpellEffectEnergize,                //  30 SPELL_EFFECT_ENERGIZE
    &Spell::SpellEffectWeaponDmgPerc,           //  31 SPELL_EFFECT_WEAPON_PERCENT_DAMAGE
    &Spell::SpellEffectTriggerMissile,          //  32 SPELL_EFFECT_TRIGGER_MISSILE
    &Spell::SpellEffectOpenLock,                //  33 SPELL_EFFECT_OPEN_LOCK
    &Spell::SpellEffectTransformItem,           //  34 SPELL_EFFECT_TRANSFORM_ITEM
    &Spell::SpellEffectApplyGroupAA,            //  35 SPELL_EFFECT_APPLY_GROUP_AREA_AURA
    &Spell::SpellEffectLearnSpell,              //  36 SPELL_EFFECT_LEARN_SPELL
    &Spell::SpellEffectSpellDefense,            //  37 SPELL_EFFECT_SPELL_DEFENSE
    &Spell::SpellEffectDispel,                  //  38 SPELL_EFFECT_DISPEL
    &Spell::SpellEffectUnused,                  //  39 SPELL_EFFECT_LANGUAGE
    &Spell::SpellEffectDualWield,               //  40 SPELL_EFFECT_DUAL_WIELD
    &Spell::SpellEffectJumpTarget,              //  41 SPELL_EFFECT_JUMP_TARGET
    &Spell::SpellEffectJumpBehindTarget,        //  42 SPELL_EFFECT_JUMP_BEHIND_TARGET
    &Spell::SpellEffectNULL,                    //  43 SPELL_EFFECT_TELEPORT_UNITS_FACE_CASTER
    &Spell::SpellEffectSkillStep,               //  44 SPELL_EFFECT_SKILL_STEP
    &Spell::SpellEffectAddHonor,                //  45 SPELL_ADD_HONOR
    &Spell::SpellEffectSpawn,                   //  46 SPELL_EFFECT_SPAWN
    &Spell::SpellEffectNULL,                    //  47 SPELL_EFFECT_TRADE_SKILL
    &Spell::SpellEffectNULL,                    //  48 SPELL_EFFECT_STEALTH
    &Spell::SpellEffectNULL,                    //  49 SPELL_EFFECT_DETECT
    &Spell::SpellEffectSummonObject,            //  50 SPELL_EFFECT_SUMMON_OBJECT
    &Spell::SpellEffectNULL,                    //  51 SPELL_EFFECT_FORCE_CRITICAL_HIT NA
    &Spell::SpellEffectNULL,                    //  52 SPELL_EFFECT_GUARANTEE_HIT NA
    &Spell::SpellEffectEnchantItem,             //  53 SPELL_EFFECT_ENCHANT_ITEM
    &Spell::SpellEffectEnchantItemTemporary,    //  54 SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
    &Spell::SpellEffectTameCreature,            //  55 SPELL_EFFECT_TAMECREATURE
    &Spell::SpellEffectSummonPet,               //  56 SPELL_EFFECT_SUMMON_PET
    &Spell::SpellEffectLearnPetSpell,           //  57 SPELL_EFFECT_LEARN_PET_SPELL
    &Spell::SpellEffectWeapondamage,            //  58 SPELL_EFFECT_WEAPON_DAMAGE
    &Spell::SpellEffectOpenLockItem,            //  59 SPELL_EFFECT_OPEN_LOCK_ITEM
    &Spell::SpellEffectProficiency,             //  60 SPELL_EFFECT_PROFICIENCY
    &Spell::SpellEffectSendEvent,               //  61 SPELL_EFFECT_SEND_EVENT
    &Spell::SpellEffectPowerBurn,               //  62 SPELL_EFFECT_POWER_BURN
    &Spell::SpellEffectThreat,                  //  63 SPELL_EFFECT_THREAT
    &Spell::SpellEffectTriggerSpell,            //  64 SPELL_EFFECT_TRIGGER_SPELL
    &Spell::SpellEffectApplyRaidAA,             //  65 SPELL_EFFECT_APPLY_RAID_AREA_AURA
    &Spell::SpellEffectPowerFunnel,             //  66 SPELL_EFFECT_POWER_FUNNEL
    &Spell::SpellEffectHealMaxHealth,           //  67 SPELL_EFFECT_HEAL_MAX_HEALTH
    &Spell::SpellEffectInterruptCast,           //  68 SPELL_EFFECT_INTERRUPT_CAST
    &Spell::SpellEffectDistract,                //  69 SPELL_EFFECT_DISTRACT
    &Spell::SpellEffectPlayerPull,              //  70 SPELL_EFFECT_PULL
    &Spell::SpellEffectPickpocket,              //  71 SPELL_EFFECT_PICKPOCKET
    &Spell::SpellEffectAddFarsight,             //  72 SPELL_EFFECT_ADD_FARSIGHT
    &Spell::SpellEffectNULL,                    //  73 SPELL_EFFECT_UNTRAIN_TALENTS?
    &Spell::SpellEffectUseGlyph,                //  74 SPELL_EFFECT_USE_GLYPH
    &Spell::SpellEffectHealMechanical,          //  75 SPELL_EFFECT_HEAL_MECHANICAL
    &Spell::SpellEffectSummonObjectWild,        //  76 SPELL_EFFECT_SUMMON_OBJECT_WILD
    &Spell::SpellEffectScriptEffect,            //  77 SPELL_EFFECT_SCRIPT_EFFECT
    &Spell::SpellEffectNULL,                    //  78 SPELL_EFFECT_ATTACK
    &Spell::SpellEffectSanctuary,               //  79 SPELL_EFFECT_SANCTUARY
    &Spell::SpellEffectAddComboPoints,          //  80 SPELL_EFFECT_ADD_COMBO_POINTS
    &Spell::SpellEffectCreateHouse,             //  81 SPELL_EFFECT_CREATE_HOUSE
    &Spell::SpellEffectNULL,                    //  82 SPELL_EFFECT_BIND_SIGHT
    &Spell::SpellEffectDuel,                    //  83 SPELL_EFFECT_DUEL
    &Spell::SpellEffectStuck,                   //  84 SPELL_EFFECT_STUCK
    &Spell::SpellEffectSummonPlayer,            //  85 SPELL_EFFECT_SUMMON_PLAYER
    &Spell::SpellEffectActivateObject,          //  86 SPELL_EFFECT_ACTIVATE_OBJECT
    &Spell::SpellEffectBuildingDamage,          //  87 SPELL_EFFECT_BUILDING_DAMAGE
    &Spell::SpellEffectNULL,                    //  88 SPELL_EFFECT_BUILDING_REPAIR
    &Spell::SpellEffectNULL,                    //  89 SPELL_EFFECT_BUILDING_SWITCH_STATE
    &Spell::SpellEffectNULL,                    //  90 SPELL_EFFECT_KILL_CREDIT_90
    &Spell::SpellEffectNULL,                    //  91 SPELL_EFFECT_THREAT_ALL  UNUSED
    &Spell::SpellEffectEnchantHeldItem,         //  92 SPELL_EFFECT_ENCHANT_HELD_ITEM
    &Spell::SpellEffectSetMirrorName,           //  93 SPELL_EFFECT_SUMMON_PHANTASM  OLD
    &Spell::SpellEffectSelfResurrect,           //  94 SPELL_EFFECT_SELF_RESURRECT
    &Spell::SpellEffectSkinning,                //  95 SPELL_EFFECT_SKINNING
    &Spell::SpellEffectCharge,                  //  96 SPELL_EFFECT_CHARGE
    &Spell::SpellEffectNULL,                    //  97 SPELL_EFFECT_SUMMON_MULTIPLE_TOTEMS // SpellEffectCastButtons
    &Spell::SpellEffectKnockBack,               //  98 SPELL_EFFECT_KNOCK_BACK
    &Spell::SpellEffectDisenchant,              //  99 SPELL_EFFECT_DISENCHANT
    &Spell::SpellEffectInebriate,               // 100 SPELL_EFFECT_INEBRIATE
    &Spell::SpellEffectFeedPet,                 // 101 SPELL_EFFECT_FEED_PET
    &Spell::SpellEffectDismissPet,              // 102 SPELL_EFFECT_DISMISS_PET
    &Spell::SpellEffectReputation,              // 103 SPELL_EFFECT_REPUTATION
    &Spell::SpellEffectSummonObjectSlot,        // 104 SPELL_EFFECT_SUMMON_OBJECT_SLOT1
    &Spell::SpellEffectSummonObjectSlot,        // 105 SPELL_EFFECT_SUMMON_OBJECT_SLOT2
    &Spell::SpellEffectSummonObjectSlot,        // 106 SPELL_EFFECT_SUMMON_OBJECT_SLOT3
    &Spell::SpellEffectSummonObjectSlot,        // 107 SPELL_EFFECT_SUMMON_OBJECT_SLOT4
    &Spell::SpellEffectDispelMechanic,          // 108 SPELL_EFFECT_DISPEL_MECHANIC
    &Spell::SpellEffectSummonDeadPet,           // 109 SPELL_EFFECT_SUMMON_DEAD_PET
    &Spell::SpellEffectDestroyAllTotems,        // 110 SPELL_EFFECT_DESTROY_ALL_TOTEMS
    &Spell::SpellEffectDurabilityDamage,        // 111 SPELL_EFFECT_DURABILITY_DAMAGE
    &Spell::SpellEffectNULL,                    // 112 not in 3.3.5a client
    &Spell::SpellEffectResurrectNew,            // 113 SPELL_EFFECT_RESURRECT_NEW
    &Spell::SpellEffectAttackMe,                // 114 SPELL_EFFECT_ATTACK_ME
    &Spell::SpellEffectDurabilityDamagePCT,     // 115 SPELL_EFFECT_DURABILITY_DAMAGE_PCT
    &Spell::SpellEffectSkinPlayerCorpse,        // 116 SPELL_EFFECT_SKIN_PLAYER_CORPSE
    &Spell::SpellEffectNULL,                    // 117 SPELL_EFFECT_SPIRIT_HEAL //Not used
    &Spell::SpellEffectSkill,                   // 118 SPELL_EFFECT_SKILL
    &Spell::SpellEffectApplyPetAA,              // 119 SPELL_EFFECT_APPLY_PET_AURA
    &Spell::SpellEffectNULL,                    // 120 SPELL_EFFECT_TELEPORT_GRAVEYARD //Not used
    &Spell::SpellEffectDummyMelee,              // 121 SPELL_EFFECT_DUMMYMELEE
    &Spell::SpellEffectNULL,                    // 122 unknown //not used
    &Spell::SpellEffectStartTaxi,               // 123 SPELL_EFFECT_START_TAXI// http://www.wowhead.com/?spell=54575
    &Spell::SpellEffectPlayerPull,              // 124 SPELL_EFFECT_PLAYER_PULL - http://thottbot.com/e2312
    &Spell::SpellEffectReduceThreatPercent,     // 125 SPELL_EFFECT_REDUCE_THREAT_PERCENT // Reduce Threat by % //http://www.thottbot.com/?sp=32835
    &Spell::SpellEffectSpellSteal,              // 126 SPELL_EFFECT_SPELL_STEAL // Steal Beneficial Buff (Magic) //http://www.thottbot.com/?sp=30449
    &Spell::SpellEffectProspecting,             // 127 SPELL_EFFECT_PROSPECTING // Search 5 ore of a base metal for precious gems.  This will destroy the ore in the process.
    &Spell::SpellEffectApplyFriendAA,           // 128 Apply Aura friendly
    &Spell::SpellEffectApplyEnemyAA,            // 129 Apply Aura enemy
    &Spell::SpellEffectRedirectThreat,          // 130 unknown // http://www.thottbot.com/s34477
    &Spell::SpellEffectNULL,                    // 131 unknown // test spell
    &Spell::SpellEffectPlayMusic,               // 132 Play Music // http://www.thottbot.com/s46852
    &Spell::SpellEffectForgetSpecialization,    // 133 SPELL_EFFECT_FORGET_SPECIALIZATION // http://www.thottbot.com/s36441 // I think this is a gm/npc spell
    &Spell::SpellEffectKillCredit,              // 134 Quest Credit (Player only, not party) // related to summoning objects and removing them, http://www.thottbot.com/s39161
    &Spell::SpellEffectNULL,                    // 135 Summon Pet: http://www.thottbot.com/s23498
    &Spell::SpellEffectRestoreHealthPct,        // 136 Restore Health % // http://www.thottbot.com/s41542 and http://www.thottbot.com/s39703
    &Spell::SpellEffectRestorePowerPct,         // 137 Restore Power % // http://www.thottbot.com/s41542
    &Spell::SpellEffectKnockBack2,              // 138 knockback2 // related to superjump or even "*jump" spells http://www.thottbot.com/?e=Unknown%20138
    &Spell::SpellEffectClearQuest,              // 139 Remove Quest
    &Spell::SpellEffectTriggerSpell,            // 140 triggers a spell from target back to caster - used at Malacrass f.e.
    &Spell::SpellEffectNULL,                    // 141 unknown // triggers spell, magic one,  (Mother spell) http://www.thottbot.com/s41065
    &Spell::SpellEffectTriggerSpellWithValue,   // 142 SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE // triggers some kind of "Put spell on target" thing... (dono for sure) http://www.thottbot.com/s40872 and http://www.thottbot.com/s33076
    &Spell::SpellEffectApplyOwnerAA,            // 143 Apply Aura on summon owner // Master -> demon effecting spell, http://www.thottbot.com/s25228 and http://www.thottbot.com/s35696
    &Spell::SpellEffectKnockBack,               // 144 unknown
    &Spell::SpellEffectPlayerPull,              // 145 unknown
    &Spell::SpellEffectActivateRunes,           // 146 Activate Rune
    &Spell::SpellEffectNULL,                    // 147 Quest Fail
    &Spell::SpellEffectNULL,                    // 148 SpellEffectTriggerMissileWithValue unknown
    &Spell::SpellEffectNULL,                    // 149 unknown
    &Spell::SpellEffectNULL,                    // 150 unknown
    &Spell::SpellEffectTriggerSpell,            // 151 SPELL_EFFECT_TRIGGER_SPELL_2
    &Spell::SpellEffectNULL,                    // 152 Summon Refer-a-Friend
    &Spell::SpellEffectCreatePet,               // 153 Create tamed pet
    &Spell::SpellEffectTeachTaxiPath,           // 154 "Teach" a taxi path
    &Spell::SpellEffectDualWield2H,             // 155 DualWield2H (ex: Titan's Grip)
    &Spell::SpellEffectEnchantItemPrismatic,    // 156 Add a socket to an armor/a weapon
    &Spell::SpellEffectCreateItem2,             // 157 SPELL_EFFECT_CREATE_ITEM_2
    &Spell::SpellEffectMilling,                 // 158 Milling
    &Spell::SpellEffectRenamePet,               // 159 Allow Pet Rename
    &Spell::SpellEffectNULL,                    // 160 unknown
    &Spell::SpellEffectLearnSpec,               // 161 Learn or unlearn a spec
    &Spell::SpellEffectActivateSpec,            // 162 Activate a spec
};

const char* SpellEffectNames[TOTAL_SPELL_EFFECTS] =
{
    "NULL",
    "INSTANT_KILL",              //    1
    "SCHOOL_DAMAGE",             //    2
    "DUMMY",                     //    3
    "PORTAL_TELEPORT",           //    4
    "TELEPORT_UNITS",            //    5
    "APPLY_AURA",                //    6
    "ENVIRONMENTAL_DAMAGE",      //    7
    "POWER_DRAIN",               //    8
    "HEALTH_LEECH",              //    9
    "HEAL",                      //    10
    "BIND",                      //    11
    "PORTAL",                    //    12
    "RITUAL_BASE",               //    13
    "RITUAL_SPECIALIZE",         //    14
    "RITUAL_ACTIVATE_PORTAL",    //    15
    "QUEST_COMPLETE",            //    16
    "WEAPON_DAMAGE_NOSCHOOL",    //    17
    "RESURRECT",                 //    18
    "ADD_EXTRA_ATTACKS",         //    19
    "DODGE",                     //    20
    "EVADE",                     //    21
    "PARRY",                     //    22
    "BLOCK",                     //    23
    "CREATE_ITEM",               //    24
    "WEAPON",                    //    25
    "DEFENSE",                   //    26
    "PERSISTENT_AREA_AURA_GROUP",//    27
    "SUMMON",                    //    28
    "LEAP",                      //    29
    "ENERGIZE",                  //    30
    "WEAPON_PERCENT_DAMAGE",     //    31
    "TRIGGER_MISSILE",           //    32
    "OPEN_LOCK",                 //    33
    "TRANSFORM_ITEM",            //    34
    "APPLY_GROUP_AREA_AURA",     //    35
    "LEARN_SPELL",               //    36
    "SPELL_DEFENSE",             //    37
    "DISPEL",                    //    38
    "LANGUAGE",                  //    39
    "DUAL_WIELD",                //    40
    "LEAP_41",                   //    41
    "JUMP_BEHIND_TARGET",        //    42
    "TELEPORT_UNITS_FACE_CASTER",//    43
    "SKILL_STEP",                //    44
    "UNDEFINED_45",              //    45
    "SPAWN",                     //    46
    "TRADE_SKILL",               //    47
    "STEALTH",                   //    48
    "DETECT",                    //    49
    "SUMMON_OBJECT",             //    50
    "FORCE_CRITICAL_HIT",        //    51
    "GUARANTEE_HIT",             //    52
    "ENCHANT_ITEM",              //    53
    "ENCHANT_ITEM_TEMPORARY",    //    54
    "TAMECREATURE",              //    55
    "SUMMON_PET",                //    56
    "LEARN_PET_SPELL",           //    57
    "WEAPON_DAMAGE",             //    58
    "OPEN_LOCK_ITEM",            //    59
    "PROFICIENCY",               //    60
    "SEND_EVENT",                //    61
    "POWER_BURN",                //    62
    "THREAT",                    //    63
    "TRIGGER_SPELL",             //    64
    "APPLY_RAID_AREA_AURA",      //    65
    "POWER_FUNNEL",              //    66
    "HEAL_MAX_HEALTH",           //    67
    "INTERRUPT_CAST",            //    68
    "DISTRACT",                  //    69
    "PULL",                      //    70
    "PICKPOCKET",                //    71
    "ADD_FARSIGHT",              //    72
    "UNTRAIN_TALENTS",           //    73
    "USE_GLYPH",                 //    74
    "HEAL_MECHANICAL",           //    75
    "SUMMON_OBJECT_WILD",        //    76
    "SCRIPT_EFFECT",             //    77
    "ATTACK",                    //    78
    "SANCTUARY",                 //    79
    "ADD_COMBO_POINTS",          //    80
    "CREATE_HOUSE",              //    81
    "BIND_SIGHT",                //    82
    "DUEL",                      //    83
    "STUCK",                     //    84
    "SUMMON_PLAYER",             //    85
    "ACTIVATE_OBJECT",           //    86
    "BUILDING_DAMAGE",           //    87
    "BUILDING_REPAIR",           //    88
    "BUILDING_SWITCH_STATE",     //    89
    "KILL_CREDIT_90",            //    90
    "THREAT_ALL",                //    91
    "ENCHANT_HELD_ITEM",         //    92
    "SET_MIRROR_NAME",           //    93
    "SELF_RESURRECT",            //    94
    "SKINNING",                  //    95
    "CHARGE",                    //    96
    "SUMMON_MULTIPLE_TOTEMS",    //    97
    "KNOCK_BACK",                //    98
    "DISENCHANT",                //    99
    "INEBRIATE",                 //    100
    "FEED_PET",                  //    101
    "DISMISS_PET",               //    102
    "REPUTATION",                //    103
    "SUMMON_OBJECT_SLOT1",       //    104
    "SUMMON_OBJECT_SLOT2",       //    105
    "SUMMON_OBJECT_SLOT3",       //    106
    "SUMMON_OBJECT_SLOT4",       //    107
    "DISPEL_MECHANIC",           //    108
    "SUMMON_DEAD_PET",           //    109
    "DESTROY_ALL_TOTEMS",        //    110
    "DURABILITY_DAMAGE",         //    111
    "NONE",                      //    112
    "RESURRECT_FLAT",            //    113
    "ATTACK_ME",                 //    114
    "DURABILITY_DAMAGE_PCT",     //    115
    "SKIN_PLAYER_CORPSE",        //    116
    "SPIRIT_HEAL",               //    117
    "SKILL",                     //    118
    "APPLY_PET_AREA_AURA",       //    119
    "TELEPORT_GRAVEYARD",        //    120
    "DUMMYMELEE",                //    121
    "UNKNOWN1",                  //    122
    "START_TAXI",                //    123
    "PLAYER_PULL",               //    124
    "UNKNOWN4",                  //    125
    "UNKNOWN5",                  //    126
    "PROSPECTING",               //    127
    "APPLY_FRIEND_AREA_AURA",    //    128
    "APPLY_RAID_AREA_AURA",      //    129
    "UNKNOWN10",                 //    130
    "UNKNOWN11",                 //    131
    "PLAY_MUSIC",                //    132
    "FORGET_SPECIALIZATION",     //    133
    "KILL_CREDIT",               //    134
    "UNKNOWN15",                 //    135
    "UNKNOWN16",                 //    136
    "RESTORE_POWER_PCT",         //    137
    "KNOCKBACK2",                //    138
    "CLEAR_QUEST",               //    139
    "UNKNOWN20",                 //    140
    "UNKNOWN21",                 //    141
    "TRIGGER_SPELL_WITH_VALUE",  //    142
    "APPLY_OWNER_AREA_AURA",     //    143
    "UNKNOWN23",                 //    144
    "UNKNOWN24",                 //    145
    "ACTIVATE_RUNES",            //    146
    "QUEST_FAIL",                //    147
    "UNKNOWN26",                 //    148
    "UNKNOWN27",                 //    149
    "UNKNOWN28",                 //    150
    "SUMMON_TARGET",             //    151
    "UNKNOWN30",                 //    152
    "TAME_CREATURE",             //    153
    "UNKNOWN32",                 //    154
    "UNKNOWN33",                 //    155
    "UNKNOWN34",                 //    156
    "UNKNOWN35",                 //    157
    "MILLING",                   //    158
    "ALLOW_PET_RENAME",          //    159
    "UNKNOWN36",                 //    160
    "UNKNOWN37"                  //    161 //used by spell 63624(dual talents)
};

void Spell::SpellEffectUnused(uint8_t /*effectIndex*/)
{
    // To prevent debugging. Useless or implemented in a different way.
}

void Spell::ApplyAreaAura(uint8_t effectIndex)
{
    if (!unitTarget || !unitTarget->isAlive()) return;
    if (u_caster != unitTarget) return;

    Aura* pAura = nullptr;
    std::map<uint64, Aura*>::iterator itr = m_pendingAuras.find(unitTarget->GetGUID());
    if (itr == m_pendingAuras.end())
    {
        pAura = sSpellFactoryMgr.NewAura(GetSpellInfo(), GetDuration(), m_caster, unitTarget);

        float r = GetRadius(effectIndex);

        uint32 eventtype = 0;

        switch (m_spellInfo->getEffect(effectIndex))
        {
            case SPELL_EFFECT_APPLY_GROUP_AREA_AURA:
                eventtype = EVENT_GROUP_AREA_AURA_UPDATE;
                break;
            case SPELL_EFFECT_APPLY_RAID_AREA_AURA:
                eventtype = EVENT_RAID_AREA_AURA_UPDATE;
                break;
            case SPELL_EFFECT_APPLY_PET_AREA_AURA:
                eventtype = EVENT_PET_AREA_AURA_UPDATE;
                break;
            case SPELL_EFFECT_APPLY_FRIEND_AREA_AURA:
                eventtype = EVENT_FRIEND_AREA_AURA_UPDATE;
                break;
            case SPELL_EFFECT_APPLY_ENEMY_AREA_AURA:
                eventtype = EVENT_ENEMY_AREA_AURA_UPDATE;
                break;
            case SPELL_EFFECT_APPLY_OWNER_AREA_AURA:
                eventtype = EVENT_ENEMY_AREA_AURA_UPDATE;
                break;
        }

        if (!sEventMgr.HasEvent(pAura, eventtype))      /* only add it once */
            sEventMgr.AddEvent(pAura, &Aura::EventUpdateAreaAura, r * r, eventtype, 1000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        m_pendingAuras.insert(std::make_pair(unitTarget->GetGUID(), pAura));
        AddRef();
        sEventMgr.AddEvent(this, &Spell::HandleAddAura, unitTarget->GetGUID(), EVENT_SPELL_HIT, 100, 1, 0);
    }
    else
    {
        pAura = itr->second;
    }

    pAura->AddMod(GetSpellInfo()->getEffectApplyAuraName(effectIndex), damage, GetSpellInfo()->getEffectMiscValue(effectIndex), effectIndex);
}


void Spell::SpellEffectNULL(uint8_t effectIndex)
{
    LogDebugFlag(LF_SPELL_EFF, "Unhandled spell effect %u in spell %u.", GetSpellInfo()->getEffect(effectIndex), GetSpellInfo()->getId());
}

void Spell::SpellEffectInstantKill(uint8_t /*effectIndex*/)
{
    if (!unitTarget || !unitTarget->isAlive())
        return;

    //Sacrifice: if spell caster has "void walker" pet, pet dies and spell caster gets a
    /*Sacrifices the Voidwalker, giving its owner a shield that will absorb
    305 damage for 30 sec. While the shield holds, spellcasting will not be \
    interrupted by damage.*/

    /*
    Demonic Sacrifice

    When activated, sacrifices your summoned demon to grant you an effect that lasts
    30 minutes. The effect is canceled if any Demon is summoned.
    Imp: Increases your Fire damage by 15%.
    Voidwalker: Restores 3% of total Health every 4 sec.
    Succubus: Increases your Shadow damage by 15%.
    Felhunter: Restores 2% of total Mana every 4 sec.

    When activated, sacrifices your summoned demon to grant you an effect that lasts $18789d.  The effect is canceled if any Demon is summoned.

    Imp: Increases your Fire damage by $18789s1%.

    Voidwalker: Restores $18790s1% of total Health every $18790t1 sec.

    Succubus: Increases your Shadow damage by $18791s1%.

    Felhunter: Restores $18792s1% of total Mana every $18792t1 sec.

    */
    uint32 spellId = GetSpellInfo()->getId();

    switch (spellId)
    {
        case 48743:
        {
            // retarget? some one test this spell.
            return;
        }
        break;
        case 18788: //Demonic Sacrifice (508745)
        {
            uint32 DemonicSacEffectSpellId = 0;
            switch (unitTarget->GetEntry())
            {
                case 416:
                    DemonicSacEffectSpellId = 18789;
                    break; //Imp
                case 417:
                    DemonicSacEffectSpellId = 18792;
                    break; //Felhunter
                case 1860:
                    DemonicSacEffectSpellId = 18790;
                    break; //VoidWalker
                case 1863:
                    DemonicSacEffectSpellId = 18791;
                    break; //Succubus
                case 17252:
                    DemonicSacEffectSpellId = 35701;
                    break; //felguard
            }
            if (DemonicSacEffectSpellId)
            {
                SpellInfo* se = sSpellCustomizations.GetSpellInfo(DemonicSacEffectSpellId);
                if (se && u_caster)
                    u_caster->CastSpell(u_caster, se, true);
            }
        } break;

        //SPELL_HASH_SACRIFICE
        case 1050:
        case 7812:
        case 7885:
        case 19438:
        case 19439:
        case 19440:
        case 19441:
        case 19442:
        case 19443:
        case 19444:
        case 19445:
        case 19446:
        case 19447:
        case 20381:
        case 20382:
        case 20383:
        case 20384:
        case 20385:
        case 20386:
        case 22651:
        case 27273:
        case 27492:
        case 30115:
        case 33587:
        case 34661:
        case 47985:
        case 47986:
        case 48001:
        case 48002:
        {
            if (!u_caster || !u_caster->IsPet())
                return;

            //TO< Pet* >(u_caster)->Dismiss(true);

            SpellInfo* se = sSpellCustomizations.GetSpellInfo(5);
            if (static_cast< Pet* >(u_caster)->GetPetOwner() == nullptr)
                return;

            SpellCastTargets targets(u_caster->GetGUID());
            Spell* sp = sSpellFactoryMgr.NewSpell(static_cast< Pet* >(u_caster)->GetPetOwner(), se, true, nullptr);
            sp->prepare(&targets);
            return;
        } break;

    }

    //SPELL_HASH_DEMONIC_SACRIFICE
    if (GetSpellInfo()->getId() == 18788)
    {
        if (!p_caster || !unitTarget || !unitTarget->IsPet())
            return;

        //TO< Pet* >(unitTarget)->Dismiss(true);

        SpellInfo* se = sSpellCustomizations.GetSpellInfo(5);

        SpellCastTargets targets(unitTarget->GetGUID());
        Spell* sp = sSpellFactoryMgr.NewSpell(p_caster, se, true, nullptr);
        sp->prepare(&targets);
        return;
    }
    else
    {
        // moar cheaters
        if (!p_caster || (u_caster && u_caster->IsPet()))
            return;

        if (p_caster->GetSession()->GetPermissionCount() == 0)
            return;
    }

    //instant kill effects don't have a log
    //m_caster->SpellNonMeleeDamageLog(unitTarget, GetProto()->getId(), unitTarget->GetHealth(), true);
    // cebernic: the value of instant kill must be higher than normal health,cuz anti health regenerated.
    m_caster->DealDamage(unitTarget, unitTarget->getUInt32Value(UNIT_FIELD_HEALTH) << 1, 0, 0, 0);
}

void Spell::SpellEffectSchoolDMG(uint8_t effectIndex) // dmg school
{
    if (!unitTarget || !unitTarget->isAlive())
        return;

    if (unitTarget->SchoolImmunityList[GetSpellInfo()->getSchool()])
    {
        SendCastResult(SPELL_FAILED_IMMUNE);
        return;
    }

    uint32 dmg = 0;
    bool static_damage = false;
    bool force_crit = false;

    if (GetSpellInfo()->getEffectChainTarget(effectIndex))    //chain
    {
        if (GetSpellInfo()->getId() == 32445 || GetSpellInfo()->getId() == 28883)
        {
            int32 reduce = (int32)(GetSpellInfo()->getDmg_multiplier(effectIndex) * 100.0f);
            reduce -= 100;

            if (reduce && chaindamage)
            {
                if (u_caster != nullptr)
                {
                    spellModFlatIntValue(u_caster->SM_PJumpReduce, &reduce, GetSpellInfo()->getSpellGroupType());
                }
                chaindamage += ((GetSpellInfo()->getEffectBasePoints(effectIndex) + 51) * reduce / 100);
            }
            else
            {
                chaindamage = damage;
            }
            dmg = chaindamage;
        }
        else
        {
            int32 reduce = (int32)(GetSpellInfo()->getDmg_multiplier(effectIndex) * 100.0f);

            if (reduce && chaindamage)
            {
                if (u_caster != nullptr)
                {
                    spellModFlatIntValue(u_caster->SM_PJumpReduce, &reduce, GetSpellInfo()->getSpellGroupType());
                }
                chaindamage = chaindamage * reduce / 100;
            }
            else
            {
                chaindamage = damage;
            }
            dmg = chaindamage;
        }
    }
    else
    {
        dmg = damage;

        switch (GetSpellInfo()->getId())
        {
            // SPELL_HASH_METEOR_SLASH:
            case 45150:
            {
                uint32 splitCount = 0;
                for (std::set<Object*>::iterator itr = u_caster->GetInRangeOppFactsSetBegin(); itr != u_caster->GetInRangeOppFactsSetEnd(); ++itr)
                {
                    if ((*itr)->isInFront(u_caster) && u_caster->CalcDistance((*itr)) <= 65)
                        splitCount++;
                }

                if (splitCount > 1)
                    dmg = dmg / splitCount;
            } break;

            // SPELL_HASH_PULSING_SHOCKWAVE: // loken Pulsing shockwave
            case 52942:
            case 52961:
            case 59836:
            case 59837:
            {
                float _distance = u_caster->CalcDistance(unitTarget);
                if (_distance >= 2.0f)
                    dmg = static_cast<uint32>(dmg * _distance);
            } break;

            // SPELL_HASH_INCINERATE: // Incinerate -> Deals x-x extra damage if the target is affected by immolate
            case 19397:
            case 23308:
            case 23309:
            case 29722:
            case 32231:
            case 32707:
            case 36832:
            case 38401:
            case 38918:
            case 39083:
            case 40239:
            case 41960:
            case 43971:
            case 44519:
            case 46043:
            case 47837:
            case 47838:
            case 53493:
            case 69973:
            case 71135:
            {
                if (unitTarget->HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_IMMOLATE))
                {
                    // random extra damage
                    uint32 extra_dmg = 111 + (GetSpellInfo()->custom_RankNumber * 11) + RandomUInt(GetSpellInfo()->custom_RankNumber * 11);
                    dmg += extra_dmg;
                }
            } break;

            // SPELL_HASH_ARCANE_SHOT: //hunter - arcane shot
            case 3044:
            case 14281:
            case 14282:
            case 14283:
            case 14284:
            case 14285:
            case 14286:
            case 14287:
            case 27019:
            case 34829:
            case 35401:
            case 36293:
            case 36609:
            case 36623:
            case 38807:
            case 49044:
            case 49045:
            case 51742:
            case 55624:
            case 58973:
            case 69989:
            case 71116:
            {
                if (u_caster)
                    dmg += float2int32(u_caster->GetRAP() * 0.15f);
                dmg = float2int32(dmg * (0.9f + RandomFloat(0.2f)));      // randomized damage

                if (p_caster != nullptr)
                {
                    dmg = static_cast<uint32>(std::round((p_caster->GetRAP() * 0.15) + m_spellInfo->getEffectBasePoints(effectIndex)));
                }
            } break;

            // SPELL_HASH_GORE: // boar/ravager: Gore (50% chance of double damage)
            case 4102:
            case 32019:
            case 35290:
            case 35291:
            case 35292:
            case 35293:
            case 35294:
            case 35295:
            case 35299:
            case 35300:
            case 35302:
            case 35303:
            case 35304:
            case 35305:
            case 35306:
            case 35307:
            case 35308:
            case 48130:
            case 51751:
            case 59264:
            {
                dmg *= Rand(50) ? 2 : 1;
            } break;

            // SPELL_HASH_THUNDER_CLAP: // Thunderclap
            case 6343:
            case 8198:
            case 8204:
            case 8205:
            case 11580:
            case 11581:
            case 13532:
            case 25264:
            case 47501:
            case 47502:
            case 57832:
            case 60019:
            {
                if (u_caster)
                    dmg = (GetSpellInfo()->getEffectBasePoints(0) + 1) + float2int32(u_caster->GetAP() * 0.12f);
            } break;

            // SPELL_HASH_SHOCKWAVE:      // Shockwave
            case 25425:
            case 33686:
            case 46968:
            case 55636:
            case 55918:
            case 57728:
            case 57741:
            case 58947:
            case 58977:
            case 63783:
            case 63982:
            case 72149:
            case 73499:
            case 73794:
            case 73795:
            case 73796:
            case 75343:
            case 75417:
            case 75418:
            {
                if (u_caster)
                    dmg = u_caster->GetAP() * (GetSpellInfo()->getEffectBasePoints(2) + 1) / 100;
            } break;

            // SPELL_HASH_JUDGEMENT_OF_COMMAND:
            case 20425:
            case 20467:
            {
                if (p_caster != nullptr)
                {
                    if (!unitTarget->IsStunned())
                        dmg = dmg >> 1;
                    if (p_caster->HasAura(34258))
                        p_caster->CastSpell(static_cast<Unit*>(p_caster), 34260, true);
                    if ((p_caster->HasAura(53696) || p_caster->HasAura(53695)))
                        p_caster->CastSpell(static_cast<Unit*>(p_caster), 68055, true);
                    if (p_caster->HasAura(37186))
                        dmg = 33;
                }
            } break;
            case 29386:
            case 32778:
            case 33554:
            case 41368:
            case 41470:
            case 66005:
            case 68017:
            case 68018:
            case 68019:
            case 71551:
            {
                if (!unitTarget->IsStunned())
                    dmg = dmg >> 1;
            } break;

            // SPELL_HASH_FIRE_STRIKE:
            case 7712:
            case 7714:
            case 7715:
            case 7716:
            case 7717:
            case 7718:
            case 7719:
            // SPELL_HASH_LIGHTNING_STRIKE:
            case 16614:
            case 23686:
            case 23687:
            case 27983:
            case 37841:
            case 52944:
            case 53062:
            // SPELL_HASH_MOLTEN_ARMOR:       // fire armor, is static damage
            case 30482:
            case 34913:
            case 35915:
            case 35916:
            case 43043:
            case 43044:
            case 43045:
            case 43046:
            {
                static_damage = true;
            } break;

            // SPELL_HASH_EXORCISM:
            case 879:
            case 5614:
            case 5615:
            case 10312:
            case 10313:
            case 10314:
            case 17147:
            case 17149:
            case 27138:
            case 33632:
            case 48800:
            case 48801:
            case 52445:
            case 58822:
            {
                if (p_caster != nullptr)
                {
                    uint32 sph = p_caster->getUInt32Value(PLAYER_FIELD_MOD_DAMAGE_DONE_POS + 1);
                    int32 ap = p_caster->GetAP();
                    dmg += float2int32((0.15f * sph) + (0.15f * ap));
                    if (unitTarget && unitTarget->IsCreature())
                    {
                        uint32 type = static_cast<Creature*>(unitTarget)->GetCreatureProperties()->Type;
                        if (type == UNIT_TYPE_UNDEAD || type == UNIT_TYPE_DEMON)
                            force_crit = true;
                    }
                }
            } break;

            // SPELL_HASH_SHIELD_OF_RIGHTEOUSNESS: // Shield of Righteousness - a bit like "shield slam", OK for both ranks
            case 53600:
            case 61411:
            {
                if (p_caster != nullptr)
                {
#if VERSION_STRING != Classic
                    Item* it = static_cast<Item*>(p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND));
                    if (it && it->GetItemProperties() && it->GetItemProperties()->InventoryType == INVTYPE_SHIELD)
                        dmg = float2int32(1.3f * p_caster->getUInt32Value(PLAYER_SHIELD_BLOCK));
#else
                    dmg += float2int32(1.30f * p_caster->getUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_BLOCK) + GetSpellInfo()->EffectBasePoints[0]);
#endif
                }
            } break;

            //SPELL_HASH_CONFLAGRATE
            case 17962:
                unitTarget->RemoveFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_IMMOLATE);
                break;

            //SPELL_HASH_ICE_LANCE
            case 30455:
            case 31766:
            case 42913:
            case 42914:
            case 43427:
            case 43571:
            case 44176:
            case 45906:
            case 46194:
            case 49906:
            case 54261:
            {
                // Deal triple damage to frozen targets or to those in Deep Freeze
                if (unitTarget->HasFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_FROZEN) || unitTarget->HasAura(44572))
                    dmg *= 3;
            } break;

            //SPELL_HASH_HEROIC_THROW
            case 57755:
            {
                if (u_caster)
                    dmg = u_caster->GetAP() / 2 + 12;
                if (p_caster != nullptr)
                    dmg = static_cast<uint32>(std::round(p_caster->GetAP() * 0.5));
            } break;

            //SPELL_HASH_SHIELD_SLAM
            case 8242:
            case 15655:
            case 23922:
            case 23923:
            case 23924:
            case 23925:
            case 25258:
            case 29684:
            case 30356:
            case 30688:
            case 46762:
            case 47487:
            case 47488:
            case 49863:
            case 59142:
            case 69903:
            case 72645:
            {
                if (p_caster != nullptr)
                {
                    Item* it = p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                    if (it && it->GetItemProperties()->InventoryType == INVTYPE_SHIELD)
                    {
                        float block_multiplier = (100.0f + p_caster->m_modblockabsorbvalue) / 100.0f;
                        if (block_multiplier < 1.0f)block_multiplier = 1.0f;

                        int32 blockable_damage = float2int32((it->GetItemProperties()->Block + p_caster->m_modblockvaluefromspells + p_caster->getUInt32Value(PLAYER_FIELD_COMBAT_RATING_1 + PCR_BLOCK) + ((p_caster->GetStat(STAT_STRENGTH) / 2.0f) - 1.0f)) * block_multiplier);

                        /*
                        3.2.0:
                        The benefit from additional block value this ability gains is now subject
                        to diminishing returns. Diminishing returns occur once block value exceeds
                        30 times the player's level and caps the maximum damage benefit from shield
                        block value at 34.5 times the player's level.
                        */
                        int32 max_blockable_damage = static_cast<int32>(p_caster->getLevel() * 34.5f);
                        if (blockable_damage > max_blockable_damage)
                        {
                            blockable_damage = max_blockable_damage;
                        }

                        dmg += blockable_damage;

                    }
                }
            } break;

            //SPELL_HASH_BLOODTHIRST
            case 23880:
            case 23881:
            {
                if (p_caster != nullptr)
                {
                    dmg = static_cast<uint32>(std::round(p_caster->GetAP() * 0.5));
                    break;
                }
            }
            case 23885:
            case 23892:
            case 23893:
            case 23894:
            case 25251:
            case 30335:
            case 30474:
            case 30475:
            case 30476:
            case 31996:
            case 31997:
            case 31998:
            case 33964:
            case 35123:
            case 35125:
            case 35947:
            case 35948:
            case 35949:
            case 39070:
            case 39071:
            case 39072:
            case 40423:
            case 55968:
            case 55969:
            case 55970:
            case 57790:
            case 57791:
            case 57792:
            case 60017:
            case 71938:
            {
                dmg = u_caster->GetAP() * (GetSpellInfo()->getEffectBasePoints(0) + 1) / 100;
            } break;

            //SPELL_HASH_CONCUSSION_BLOW
            case 12809:
            case 22427:
            case 32588:
            case 52719:
            case 54132:
            {
                //3.2.2
                //[Concussion Blow]: The damage done by this ability has been reduced by 50%,
                //but its threat generation will remain approximately the same.
                dmg = u_caster->GetAP() * (GetSpellInfo()->getEffectBasePoints(2) + 1) / 100;
            } break;

            // SPELL_HASH_INTERCEPT
            case 20252:
            {
                if (p_caster != nullptr)
                {
                    dmg = static_cast<uint32>(std::round(p_caster->GetAP() * 0.12));
                    break;
                }
            }
            case 20253:
            case 20614:
            case 20615:
            case 20616:
            case 20617:
            case 25272:
            case 25273:
            case 25274:
            case 25275:
            case 27577:
            case 27826:
            case 30151:
            case 30153:
            case 30154:
            case 30194:
            case 30195:
            case 30197:
            case 30198:
            case 30199:
            case 30200:
            case 47995:
            case 47996:
            case 50823:
            case 58743:
            case 58747:
            case 58769:
            case 61490:
            case 61491:
            case 67540:
            case 67573:
            {
                if (u_caster)
                    dmg = float2int32(u_caster->GetAP() * 0.12f);
            } break;

            case 5308:
            case 20658:
            case 20660:
            case 20661:
            case 20662:
            case 25234:
            case 25236:
            case 47470:
            case 47471:
            {
                if (p_caster != nullptr)
                    dmg = p_caster->GetAP() * ((m_spellInfo->getEffectBasePoints(0) + 1) / 100);
            }break;

            //SPELL_HASH_SHIELD_SLAM
            /* Zyres 10/06/2017: Already defined!
            case 23922:
            case 23923:
            case 23924:
            case 23925:
            case 25258:
            case 30356:
            case 47487:
            case 47488:
            {
                if (p_caster != NULL)
                {
#if VERSION_STRING != Classic
                    Item* it = static_cast<Item*>(p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND));
                    if (it && it->GetItemProperties() && it->GetItemProperties()->InventoryType == INVTYPE_SHIELD)
                        dmg = p_caster->getUInt32Value(PLAYER_SHIELD_BLOCK);
#endif
                }
            }break;*/
            case 34428:
            {
                if (p_caster != nullptr)
                {
                    p_caster->RemoveFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_REJUVENATE);
                    dmg = (p_caster->GetAP()*(m_spellInfo->getEffectBasePoints(effectIndex) + 1)) / 100;
                }
            }break;
            case 6572:
            case 6574:
            case 7379:
            case 11600:
            case 11601:
            case 25288:
            case 25269:
            case 30357:
            case 57823:
            {
                if (p_caster != nullptr)
                    dmg = static_cast<uint32>(std::round(p_caster->GetAP() * 0.207));
            }break;

            case 64382:
                if (p_caster != nullptr)
                    dmg = static_cast<uint32>(std::round(p_caster->GetAP() * 0.5));
                break;

            case 29707:
            case 30324:
            case 47449:
            case 47450:
            {
                /* Possibly broken (infinity loop) -- ask Zyres
                if (p_caster != nullptr)
                {
                    if (unitTarget->IsDazed())
                        for (uint32 i = UNIT_FIELD_AURASTATE; i < AURASTATE_FLAG_REJUVENATE; i)
                        {
                            switch (m_spellInfo->getId())
                            { // This info isn't in the dbc files.....
                                case 29707:
                                    dmg = static_cast<uint32>(std::round(81.9));
                                    break;
                                case 30324:
                                    dmg = static_cast<uint32>(std::round(110.95));
                                    break;
                                case 47449:
                                    dmg = static_cast<uint32>(std::round(151.2));
                                    break;
                                case 47450:
                                    dmg = static_cast<uint32>(std::round(173.25));
                                    break;
                            }
                        }
                }
                */
            }break;
            case 845:
            case 7369:
            case 11608:
            case 11609:
            case 20569:
            case 25231:
            case 47519:
            case 47520:
            {
                if (p_caster != nullptr)
                {
                    auto item = p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                    if (item != nullptr)
                    {
                        if (p_caster->HasAura(12329))
                            dmg = static_cast<uint32>(std::round((((item->GetItemProperties()->Damage[0].Min + item->GetItemProperties()->Damage[0].Max) * 0.2f) + m_spellInfo->getEffectBasePoints(effectIndex)) * 0.4));
                        else if (p_caster->HasAura(12950))
                            dmg = static_cast<uint32>(std::round((((item->GetItemProperties()->Damage[0].Min + item->GetItemProperties()->Damage[0].Max) * 0.2f) + m_spellInfo->getEffectBasePoints(effectIndex)) * 0.8));
                        else if (p_caster->HasAura(20496))
                            dmg = static_cast<uint32>(std::round((((item->GetItemProperties()->Damage[0].Min + item->GetItemProperties()->Damage[0].Max) * 0.2f) + m_spellInfo->getEffectBasePoints(effectIndex)) * 1.2));
                        else
                            dmg = static_cast<uint32>(std::round(((item->GetItemProperties()->Damage[0].Min + item->GetItemProperties()->Damage[0].Max) * 0.2f) + m_spellInfo->getEffectBasePoints(effectIndex)));
                    }
                }
            }break;
            // Slam
            case 1464:
            case 8820:
            case 11604:
            case 11605:
            case 25241:
            case 25242:
            case 47474:
            case 47475:
            {
                if (p_caster != nullptr)
                {
                    auto item = p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                    if (item != nullptr)
                        dmg = static_cast<uint32>(std::round(((item->GetItemProperties()->Damage[0].Min + item->GetItemProperties()->Damage[0].Max) * 0.2f) + m_spellInfo->getEffectBasePoints(effectIndex)));
                }
            }break;

            case 31898:
            case 31804:
            case 20187:
            case 53733:
            case 57774:
            case 20268:
            case 53726:
            {
                if (p_caster != nullptr)
                {
                    if (p_caster->HasAura(34258))
                        p_caster->CastSpell(static_cast<Unit*>(p_caster), 34260, true);
                    if ((p_caster->HasAura(53696) || p_caster->HasAura(53695)))
                        p_caster->CastSpell(static_cast<Unit*>(p_caster), 68055, true);
                    if (p_caster->HasAura(37186))
                        dmg = 33;
                }
            }break;

            case 25742:
            {
                if (p_caster != nullptr)
                    dmg = static_cast<uint32>(std::round(p_caster->getUInt32Value(UNIT_FIELD_BASEATTACKTIME) / 1000 * ((0.022 * (p_caster->GetAP()) + (0.044 * (p_caster->GetDamageDoneMod(1))))) + m_spellInfo->getEffectBasePoints(effectIndex)));
            }break;
            case 9799:
            case 25988:
            {
                if (p_caster != nullptr)
                {
                    if (dmg > (p_caster->getUInt32Value(UNIT_FIELD_MAXHEALTH) / 2))
                        dmg = (p_caster->getUInt32Value(UNIT_FIELD_MAXHEALTH) / 2);
                }
            }break;

            case 19434:
            case 20900:
            case 20901:
            case 20902:
            case 20903:
            case 20904:
            case 27065:
            case 49049:
            case 49050:
            {
                if (p_caster != nullptr)
                {
                    auto item = p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                    if (item != nullptr)
                        dmg = static_cast<uint32>(std::round(((item->GetItemProperties()->Damage[0].Min + item->GetItemProperties()->Damage[0].Max) * 0.2f) + m_spellInfo->getEffectBasePoints(effectIndex)));

                }
            }break;
            case 53209:
            {
                if (p_caster != nullptr)
                {
                    auto item = p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                    if (item != nullptr)
                        dmg = static_cast<uint32>(std::round(((item->GetItemProperties()->Damage[0].Min + item->GetItemProperties()->Damage[0].Max) * 0.2f) * 1.25));
                }
            }break;
            case 56641:
            case 34120:
            case 49051:
            case 49052:
            {
#if VERSION_STRING != Cata
                if (p_caster != nullptr)
                {
                    Item* pItem = p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                    ItemProperties const* pItemProto = sMySQLStore.getItemProperties(p_caster->getUInt32Value(PLAYER_AMMO_ID));
                    uint32 stundmg;
                    float bowdmg;
                    float ammodmg;
                    if (unitTarget->IsDazed())
                        stundmg = p_caster->GetRAP() / 10 + m_spellInfo->getEffectBasePoints(effectIndex) + m_spellInfo->getEffectBasePoints(effectIndex + 1);
                    else
                        stundmg = p_caster->GetRAP() / 10 + m_spellInfo->getEffectBasePoints(effectIndex);
                    if (pItem)
                        bowdmg = (pItem->GetItemProperties()->Damage[0].Min + pItem->GetItemProperties()->Damage[0].Max) * 0.2f;
                    else
                        bowdmg = 0;
                    if (pItemProto)
                        ammodmg = (pItemProto->Damage[0].Min + pItemProto->Damage[0].Max) * 0.2f;
                    else
                        ammodmg = 0;

                    dmg = float2int32(ammodmg + bowdmg) + stundmg;
                }
#endif
            }break;
            case 64422: // Sonic Screech, Auriaya encounter
            case 64688:
            {
                if (u_caster != nullptr)
                {
                    int splitCount = 0;
                    for (std::set<Object*>::iterator itr = u_caster->GetInRangeOppFactsSetBegin(); itr != u_caster->GetInRangeOppFactsSetEnd(); ++itr)
                    {
                        if ((*itr)->isInFront(u_caster))
                            splitCount++;
                    };
                    if (splitCount > 1)
                        dmg /= splitCount;
                }
            }
            break;

            default:
                break;
        };
    }

    if (p_caster && !static_damage)   //this is wrong but with current spell coef system it has to be here...
    {
        switch (p_caster->getClass())
        {
            case WARRIOR:
            case ROGUE:
            case HUNTER:
            case DEATHKNIGHT:
                static_damage = true; //No spells from these classes benefit from spell damage. Prevents Arc hunters, frost DKs, etc.
                break;
            default:
                break;
        }
    }


    // check for no more damage left (chains)
    if (!dmg)
        dmg = GetSpellInfo()->getEffectBasePoints(effectIndex);

    if (!dmg)
        return;

    if (GetSpellInfo()->getSpeed() > 0 && m_triggeredSpell == false)
    {
        m_caster->SpellNonMeleeDamageLog(unitTarget, GetSpellInfo()->getId(), dmg, pSpellId == 0);
    }
    else
    {
        if (GetType() == SPELL_DMG_TYPE_MAGIC)
        {
            m_caster->SpellNonMeleeDamageLog(unitTarget, GetSpellInfo()->getId(), dmg, !m_triggeredSpell, static_damage);
        }
        else
        {
            if (u_caster != nullptr)
            {
                uint32 _type;
                if (GetType() == SPELL_DMG_TYPE_RANGED)
                    _type = RANGED;
                else
                {
                    if (GetSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_TYPE_OFFHAND)
                        _type = OFFHAND;
                    else
                        _type = MELEE;
                }

                u_caster->Strike(unitTarget, _type, GetSpellInfo(), 0, 0, dmg, !m_triggeredSpell, true, force_crit);
            }
        }
    }
}

void Spell::SpellEffectDummy(uint8_t effectIndex) // Dummy(Scripted events)
{
    if (objmgr.CheckforDummySpellScripts(static_cast<Player*>(u_caster), m_spellInfo->getId()))
        return;

    if (sScriptMgr.CallScriptedDummySpell(m_spellInfo->getId(), effectIndex, this))
        return;

    LogDebugFlag(LF_SPELL_EFF, "Spell ID: %u (%s) has a dummy effect index (%hhu) but no handler for it.", m_spellInfo->getId(), m_spellInfo->getName().c_str(), effectIndex);
}

void Spell::SpellEffectTeleportUnits(uint8_t effectIndex)    // Teleport Units
{
    if (unitTarget == nullptr || m_caster == nullptr)
        return;

    uint32 spellId = GetSpellInfo()->getId();

    // Portals
    if (m_spellInfo->HasCustomFlagForEffect(effectIndex, TELEPORT_TO_COORDINATES))
    {
        TeleportCoords const* teleport_coord = sMySQLStore.getTeleportCoord(spellId);
        if (teleport_coord == nullptr)
        {
            LOG_ERROR("Spell %u (%s) has a TELEPORT TO COORDINATES effect, but has no coordinates to teleport to. ", spellId, m_spellInfo->getName().c_str());
            return;
        }

        HandleTeleport(teleport_coord->x, teleport_coord->y, teleport_coord->z, teleport_coord->mapId, unitTarget);
        return;
    }

    // Hearthstone and co.
    if (m_spellInfo->HasCustomFlagForEffect(effectIndex, TELEPORT_TO_BINDPOINT))
    {
        if (unitTarget->IsPlayer())
        {
            Player* p = static_cast<Player*>(unitTarget);

            HandleTeleport(p->GetBindPositionX(), p->GetBindPositionY(), p->GetBindPositionZ(), p->GetBindMapId(), p);
        }
        return;
    }

    // Summon
    if (m_spellInfo->HasCustomFlagForEffect(effectIndex, TELEPORT_TO_CASTER))
    {
        if (u_caster == nullptr)
            return;

        HandleTeleport(m_caster->GetPositionX(), m_caster->GetPositionY(), m_caster->GetPositionZ(), m_caster->GetMapId(), unitTarget);
        return;
    }

    // Shadowstep for example
    if (m_spellInfo->HasCustomFlagForEffect(effectIndex, TELEPORT_BEHIND_TARGET))
    {
        if (p_caster == nullptr)
            return;

        ///////////////////////////////////////////////// Code taken from the Shadowstep dummy script /////////////////////////////////////////////////////////////////////

        /* this is rather tricky actually. we have to calculate the orientation of the creature/player, and then calculate a little bit of distance behind that. */
        float ang;

        if (unitTarget == m_caster)
        {
            /* try to get a selection */
            unitTarget = m_caster->GetMapMgr()->GetUnit(m_targets.m_unitTarget);
            if ((!unitTarget) || !isAttackable(p_caster, unitTarget, !(GetSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED)) || (unitTarget->CalcDistance(p_caster) > 28.0f))
            {
                return;
            }
        }

        if (unitTarget->IsCreature())
        {
            if (unitTarget->GetTargetGUID() != 0)
            {
                /* We're chasing a target. We have to calculate the angle to this target, this is our orientation. */
                ang = m_caster->calcAngle(m_caster->GetPositionX(), m_caster->GetPositionY(), unitTarget->GetPositionX(), unitTarget->GetPositionY());
                /* convert degree angle to radians */
                ang = ang * M_PI_FLOAT / 180.0f;
            }
            else
            {
                /* Our orientation has already been set. */
                ang = unitTarget->GetOrientation();
            }
        }
        else
        {
            /* Players orientation is sent in movement packets */
            ang = unitTarget->GetOrientation();
        }
        // avoid teleporting into the model on scaled models
        const static float shadowstep_distance = 1.6f * unitTarget->getFloatValue(OBJECT_FIELD_SCALE_X);
        float new_x = unitTarget->GetPositionX() - (shadowstep_distance * cosf(ang));
        float new_y = unitTarget->GetPositionY() - (shadowstep_distance * sinf(ang));
        /* Send a movement packet to "charge" at this target. Similar to warrior charge. */
        p_caster->z_axisposition = 0.0f;
        p_caster->SafeTeleport(p_caster->GetMapId(), p_caster->GetInstanceID(), LocationVector(new_x, new_y, (unitTarget->GetPositionZ() + 0.1f), ang));


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        return;
    }

    // For those special teleport spells
    if (sScriptMgr.CallScriptedDummySpell(m_spellInfo->getId(), effectIndex, this))
        return;

    LOG_ERROR("Unhandled Teleport effect Index %hhu for Spell %u (%s).", effectIndex, m_spellInfo->getId(), m_spellInfo->getName().c_str());
}

void Spell::SpellEffectApplyAura(uint8_t effectIndex)  // Apply Aura
{
    if (unitTarget == nullptr)
        return;
    // can't apply stuns/fear/polymorph/root etc on boss
    if (unitTarget->IsCreature())
    {
        if (u_caster != nullptr && (u_caster != unitTarget))
        {
            Creature* c = static_cast< Creature* >(unitTarget);
            /*
            Charm (Mind Control, enslave demon): 1
            Confuse (Blind etc): 2
            Fear: 4
            Root: 8
            Silence : 16
            Stun: 32
            Sheep: 64
            Banish: 128
            Sap: 256
            Frozen : 512
            Ensnared 1024
            Sleep 2048
            Taunt (aura): 4096
            Decrease Speed (Hamstring) (aura): 8192
            Spell Haste (Curse of Tongues) (aura): 16384
            Interrupt Cast: 32768
            Mod Healing % (Mortal Strike) (aura): 65536
            Total Stats % (Vindication) (aura): 131072
            */

            //Spells with Mechanic also add other ugly auras, but if the main aura is the effect --> immune to whole spell
            if (c->GetCreatureProperties()->modImmunities)
            {
                bool immune = false;
                if (m_spellInfo->getMechanicsType())
                {
                    switch (m_spellInfo->getMechanicsType())
                    {
                        case MECHANIC_CHARMED:
                            if (c->GetCreatureProperties()->modImmunities & 1)
                                immune = true;
                            break;
                        case MECHANIC_DISORIENTED:
                            if (c->GetCreatureProperties()->modImmunities & 2)
                                immune = true;
                            break;
                        case MECHANIC_FLEEING:
                            if (c->GetCreatureProperties()->modImmunities & 4)
                                immune = true;
                            break;
                        case MECHANIC_ROOTED:
                            if (c->GetCreatureProperties()->modImmunities & 8)
                                immune = true;
                            break;
                        case MECHANIC_SILENCED:
                            if (c->GetCreatureProperties()->modImmunities & 16)
                                immune = true;
                            break;
                        case MECHANIC_STUNNED:
                            if (c->GetCreatureProperties()->modImmunities & 32)
                                immune = true;
                            break;
                        case MECHANIC_POLYMORPHED:
                            if (c->GetCreatureProperties()->modImmunities & 64)
                                immune = true;
                            break;
                        case MECHANIC_BANISHED:
                            if (c->GetCreatureProperties()->modImmunities & 128)
                                immune = true;
                            break;
                        case MECHANIC_SAPPED:
                            if (c->GetCreatureProperties()->modImmunities & 256)
                                immune = true;
                            break;
                        case MECHANIC_FROZEN:
                            if (c->GetCreatureProperties()->modImmunities & 512)
                                immune = true;
                            break;
                        case MECHANIC_ENSNARED:
                            if (c->GetCreatureProperties()->modImmunities & 1024)
                                immune = true;
                            break;
                        case MECHANIC_ASLEEP:
                            if (c->GetCreatureProperties()->modImmunities & 2048)
                                immune = true;
                            break;
                    }
                }
                if (!immune)
                {
                    // Spells that do more than just one thing (damage and the effect) don't have a mechanic and we should only cancel the aura to be placed
                    switch (m_spellInfo->getEffectApplyAuraName(effectIndex))
                    {
                        case SPELL_AURA_MOD_CONFUSE:
                            if (c->GetCreatureProperties()->modImmunities & 2)
                                immune = true;
                            break;
                        case SPELL_AURA_MOD_FEAR:
                            if (c->GetCreatureProperties()->modImmunities & 4)
                                immune = true;
                            break;
                        case SPELL_AURA_MOD_TAUNT:
                            if (c->GetCreatureProperties()->modImmunities & 4096)
                                immune = true;
                            break;
                        case SPELL_AURA_MOD_STUN:
                            if (c->GetCreatureProperties()->modImmunities & 32)
                                immune = true;
                            break;
                        case SPELL_AURA_MOD_SILENCE:
                            if ((c->GetCreatureProperties()->modImmunities & 32768) || (c->GetCreatureProperties()->modImmunities & 16))
                                immune = true;
                            break;
                        case SPELL_AURA_MOD_DECREASE_SPEED:
                            if (c->GetCreatureProperties()->modImmunities & 8192)
                                immune = true;
                            break;
                        case SPELL_AURA_INCREASE_CASTING_TIME_PCT:
                            if (c->GetCreatureProperties()->modImmunities & 16384)
                                immune = true;
                            break;
                        case SPELL_AURA_MOD_LANGUAGE: //hacky way to prefer that the COT icon is set to mob
                            if (c->GetCreatureProperties()->modImmunities & 16384)
                                immune = true;
                            break;
                        case SPELL_AURA_MOD_HEALING_DONE_PERCENT:
                            if (c->GetCreatureProperties()->modImmunities & 65536)
                                immune = true;
                            break;
                        case SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE:
                            if (c->GetCreatureProperties()->modImmunities & 131072)
                                immune = true;
                            break;
                    }
                }
                if (immune)
                    return;
            }
        }
    }

#ifdef GM_Z_DEBUG_DIRECTLY
    else
    {
        if (unitTarget->IsPlayer() && unitTarget->IsInWorld() && TO< Player* >(unitTarget)->GetSession() && TO< Player* >(unitTarget)->GetSession()->CanUseCommand('z'))
        {
            sChatHandler.BlueSystemMessage(TO< Player* >(unitTarget)->GetSession(), "[%sSystem%s] |rSpell::SpellEffectApplyAura: %s EffectApplyAuraName [%u] .", MSG_COLOR_WHITE, MSG_COLOR_LIGHTBLUE, MSG_COLOR_SUBWHITE,
                                           i);
        }
    }
#endif

    // avoid map corruption.
    if (unitTarget->GetInstanceID() != m_caster->GetInstanceID())
        return;

    //check if we already have stronger aura
    Aura* pAura;

    std::map<uint64, Aura*>::iterator itr = m_pendingAuras.find(unitTarget->GetGUID());
    //if we do not make a check to see if the aura owner is the same as the caster then we will stack the 2 auras and they will not be visible client sided
    if (itr == m_pendingAuras.end())
    {
        if (ProcedOnSpell)  //Warrior's Blood Frenzy
        {
            switch (GetSpellInfo()->getId())
            {
                //SPELL_HASH_BLOOD_FRENZY
                case 16952:
                case 16954:
                case 29836:
                case 29859:
                case 30069:
                case 30070:
                    GetSpellInfo()->setDurationIndex(ProcedOnSpell->getDurationIndex());
                    break;
            }
        }

        uint32 Duration = GetDuration();

        // Handle diminishing returns, if it should be resisted, it'll make duration 0 here.
        if (!(GetSpellInfo()->IsPassive())) // Passive
        {
            unitTarget->applyDiminishingReturnTimer(&Duration, GetSpellInfo());
        }

        if (!Duration)
        {
            SendCastResult(SPELL_FAILED_IMMUNE);
            return;
        }

        if (g_caster && g_caster->getUInt32Value(OBJECT_FIELD_CREATED_BY) && g_caster->m_summoner)
            pAura = sSpellFactoryMgr.NewAura(GetSpellInfo(), Duration, g_caster->m_summoner, unitTarget, m_triggeredSpell, i_caster);
        else
            pAura = sSpellFactoryMgr.NewAura(GetSpellInfo(), Duration, m_caster, unitTarget, m_triggeredSpell, i_caster);

        pAura->pSpellId = pSpellId; //this is required for triggered spells

        m_pendingAuras.insert(std::make_pair(unitTarget->GetGUID(), pAura));
        AddRef();
        sEventMgr.AddEvent(this, &Spell::HandleAddAura, unitTarget->GetGUID(), EVENT_SPELL_HIT, 100, 1, 0);
    }
    else
    {
        pAura = itr->second;
    }
    switch (m_spellInfo->getId())
    {
        case 27907:
        {
            if (unitTarget->GetEntry() == 15941)
            {
                unitTarget->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "What? Oh, not this again!");
            }
            else if (unitTarget->GetEntry() == 15945)
            {
                unitTarget->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "You can't do this to me! We had a deal!");
            }
            else
            {
                SendCastResult(SPELL_FAILED_BAD_TARGETS);
                return;
            }
        }break;
        case 28880:
        {
            if (!p_caster)
                break;

            if (unitTarget->GetEntry() == 16483)
            {
                unitTarget->RemoveAura(29152);
                unitTarget->SetStandState(STANDSTATE_STAND);
                static const char* testo[12] = { "None", "Warrior", "Paladin", "Hunter", "Rogue", "Priest", "Death Knight", "Shaman", "Mage", "Warlock", "None", "Druid" };
                char msg[150];
                snprintf(msg, 150, "Many thanks to you %s. I'd best get to the crash site and see how I can help out. Until we meet again...", testo[p_caster->getClass()]);
                unitTarget->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg);
                ((Creature*)unitTarget)->Despawn(900000, 300000);
            }
        }break;
        case 38177:
        {
            if (!p_caster)
                break;

            if (unitTarget->GetEntry() == 21387)
            {
                ((Creature*)unitTarget)->Despawn(5000, 360000);
                p_caster->CastSpell(p_caster, 38178, true);
            }
            else
            {
                SendCastResult(SPELL_FAILED_BAD_TARGETS);
                return;
            }
        }break;
    }
    pAura->AddMod(GetSpellInfo()->getEffectApplyAuraName(effectIndex), damage, GetSpellInfo()->getEffectMiscValue(effectIndex), effectIndex);
}

void Spell::SpellEffectEnvironmentalDamage(uint8_t /*effectIndex*/)
{
    if (!playerTarget) return;

    if (playerTarget->SchoolImmunityList[GetSpellInfo()->getSchool()])
    {
        SendCastResult(SPELL_FAILED_IMMUNE);
        return;
    }
    //this is GO, not unit
    m_caster->SpellNonMeleeDamageLog(playerTarget, GetSpellInfo()->getId(), damage, pSpellId == 0);
    playerTarget->SendEnvironmentalDamageLog(playerTarget->GetGUID(), DAMAGE_FIRE, damage);
}

void Spell::SpellEffectPowerDrain(uint8_t effectIndex)  // Power Drain
{
    if (!unitTarget || !unitTarget->isAlive())
        return;

    uint32 powerField = UNIT_FIELD_POWER1 + GetSpellInfo()->getEffectMiscValue(effectIndex);
    uint32_t curPower = unitTarget->getUInt32Value(static_cast<uint16_t>(powerField));
    if (powerField == UNIT_FIELD_POWER1 && unitTarget->IsPlayer())
    {
        Player* mPlayer = static_cast< Player* >(unitTarget);
        if (mPlayer->IsInFeralForm())
            return;

        // Resilience - reduces the effect of mana drains by (CalcRating*2)%.
        damage = float2int32(damage * (1 - ((static_cast< Player* >(unitTarget)->CalcRating(PCR_SPELL_CRIT_RESILIENCE) * 2) / 100.0f)));
    }
    uint32 amt = damage + ((u_caster->GetDamageDoneMod(GetSpellInfo()->getSchool()) * 80) / 100);
    if (amt > curPower)
        amt = curPower;
    unitTarget->setUInt32Value(static_cast<uint16_t>(powerField), curPower - amt);
    u_caster->Energize(u_caster, GetSpellInfo()->getId(), amt, GetSpellInfo()->getEffectMiscValue(effectIndex));
}

void Spell::SpellEffectHealthLeech(uint8_t /*effectIndex*/) // Health Leech
{
    if (!unitTarget || !unitTarget->isAlive())
        return;

    uint32 curHealth = unitTarget->GetHealth();
    uint32 amt = damage;
    if (amt > curHealth)
    {
        amt = curHealth;
    }
    m_caster->DealDamage(unitTarget, damage, 0, 0, GetSpellInfo()->getId());

    if (!u_caster)
        return;
    uint32 playerCurHealth = u_caster->getUInt32Value(UNIT_FIELD_HEALTH);
    uint32 playerMaxHealth = u_caster->getUInt32Value(UNIT_FIELD_MAXHEALTH);

    if (playerCurHealth + amt > playerMaxHealth)
    {
        u_caster->SetHealth(playerMaxHealth);
    }
    else
    {
        u_caster->ModHealth(amt);
    }
}

void Spell::SpellEffectHeal(uint8_t effectIndex) // Heal
{
    if (p_caster != nullptr)
    {
        // HACKY but with SM_FEffect2_bonus it doesnt work
        uint32 fireResistanceAura[] =
        {
            //SPELL_HASH_FIRE_RESISTANCE_AURA
            19891,
            19899,
            19900,
            27153,
            48947,
            0
        };

        uint32 frostResistanceAura[] =
        {
            //SPELL_HASH_FROST_RESISTANCE_AURA
            19888,
            19897,
            19898,
            27152,
            48945,
            0
        };

        uint32 shadowResistanceAura[] =
        {
            //SPELL_HASH_SHADOW_RESISTANCE_AURA
            19876,
            19895,
            19896,
            27151,
            48943,
            0
        };

        uint32 retributionAura[] =
        {
            //SPELL_HASH_RETRIBUTION_AURA
            7294,
            8990,
            10298,
            10299,
            10300,
            10301,
            13008,
            27150,
            54043,
            0
        };

        uint32 devotionAura[] =
        {
            //SPELL_HASH_DEVOTION_AURA
            465,
            643,
            1032,
            8258,
            10290,
            10291,
            10292,
            10293,
            17232,
            27149,
            41452,
            48941,
            48942,
            52442,
            57740,
            58944,
            0
        };

        // Apply this only on targets, which have one of paladins auras
        if (unitTarget && (unitTarget->hasAurasWithId(devotionAura) || unitTarget->hasAurasWithId(retributionAura) ||
            unitTarget->hasAurasWithId(19746) || unitTarget->hasAurasWithId(32223) || unitTarget->hasAurasWithId(fireResistanceAura) ||
            unitTarget->hasAurasWithId(frostResistanceAura) || unitTarget->hasAurasWithId(shadowResistanceAura)))
        {
            if (p_caster->HasSpell(20140))     // Improved Devotion Aura Rank 3
                damage = (int32)(damage * 1.06);
            else if (p_caster->HasSpell(20139))     // Improved Devotion Aura Rank 2
                damage = (int32)(damage * 1.04);
            else if (p_caster->HasSpell(20138))     // Improved Devotion Aura Rank 1
                damage = (int32)(damage * 1.02);
        }

        if (p_caster->HasSpell(54943) && p_caster->HasAura(20165))       // Glyph of Seal of Light
            damage = (int32)(damage * 1.05);
    }

    if (GetSpellInfo()->getEffectChainTarget(effectIndex))    //chain
    {
        if (!chaindamage)
        {
            chaindamage = damage;
            Heal((int32)chaindamage);
        }
        else
        {
            int32 reduce = GetSpellInfo()->getEffectDieSides(effectIndex) + 1;
            if (u_caster != nullptr)
            {
                spellModFlatIntValue(u_caster->SM_PJumpReduce, &reduce, GetSpellInfo()->getSpellGroupType());
            }
            chaindamage -= (reduce * chaindamage) / 100;
            Heal((int32)chaindamage);
        }
    }
    else
    {
        //yep, the usual special case. This one is shaman talent : Nature's guardian
        //health is below 30%, we have a mother spell to get value from
        switch (GetSpellInfo()->getId())
        {
            case 31616:
            {
                if (unitTarget && unitTarget->IsPlayer() && pSpellId && unitTarget->GetHealthPct() < 30)
                {
                    //check for that 10 second cooldown
                    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(pSpellId);
                    if (spellInfo)
                    {
                        //heal value is received by the level of current active talent :s
                        //maybe we should use CalculateEffect(uint32 i) to gain SM benefits
                        int32 value = 0;
                        int32 basePoints = spellInfo->getEffectBasePoints(effectIndex) + 1; //+(m_caster->getLevel()*basePointsPerLevel);
                        uint32 randomPoints = spellInfo->getEffectDieSides(effectIndex);
                        if (randomPoints <= 1)
                            value = basePoints;
                        else
                            value = basePoints + RandomUInt(randomPoints);
                        //the value is in percent. Until now it's a fixed 10%
                        Heal(unitTarget->GetMaxHealth()*value / 100);
                    }
                }
            }
            break;
            //Bloodthirst
            case 23880:
            {
                if (unitTarget)
                {
                    Heal(unitTarget->GetMaxHealth() / 100);
                }
            }
            break;

            case 34299: //Druid: Improved Leader of the PAck
            {
                if (unitTarget == nullptr || !unitTarget->IsPlayer() || !unitTarget->isAlive())
                    break;

                Player* mPlayer = static_cast< Player* >(unitTarget);
                if (!mPlayer->IsInFeralForm() || (
                    mPlayer->GetShapeShift() != FORM_CAT &&
                    mPlayer->GetShapeShift() != FORM_BEAR &&
                    mPlayer->GetShapeShift() != FORM_DIREBEAR))
                    break;
                uint32 max = mPlayer->getUInt32Value(UNIT_FIELD_MAXHEALTH);
                uint32 val = float2int32(((mPlayer->getAuraWithId(34300)) ? 0.04f : 0.02f) * max);
                if (val)
                    mPlayer->Heal(mPlayer, 34299, (uint32)(val));
            }
            break;
            case 22845: // Druid: Frenzied Regeneration
            {
                if (unitTarget == nullptr || !unitTarget->IsPlayer() || !unitTarget->isAlive())
                    break;

                Player* mPlayer = static_cast< Player* >(unitTarget);
                if (!mPlayer->IsInFeralForm() ||
                    (mPlayer->GetShapeShift() != FORM_BEAR &&
                    mPlayer->GetShapeShift() != FORM_DIREBEAR))
                    break;
                uint32 val = mPlayer->GetPower(POWER_TYPE_RAGE);
                if (val > 100)
                    val = 100;
                uint32 HpPerPoint = float2int32((mPlayer->getUInt32Value(UNIT_FIELD_MAXHEALTH) * 0.003f));   //0.3% of hp per point of rage
                uint32 heal = HpPerPoint * (val / 10); //1 point of rage = 0.3% of max hp
                mPlayer->ModPower(POWER_TYPE_RAGE, -1 * val);

                if (val)
                    mPlayer->Heal(mPlayer, 22845, heal);
            }
            break;
            case 18562: //druid - swiftmend
            {
                if (unitTarget)
                {
                    uint32 new_dmg = 0;
                    //consume rejuvenetaion and regrowth

                    uint32 regrowth[] =
                    {
                        //SPELL_HASH_REGROWTH
                        8936,
                        8938,
                        8939,
                        8940,
                        8941,
                        9750,
                        9856,
                        9857,
                        9858,
                        16561,
                        20665,
                        22373,
                        22695,
                        26980,
                        27637,
                        28744,
                        34361,
                        39000,
                        39125,
                        48442,
                        48443,
                        66067,
                        67968,
                        67969,
                        67970,
                        69882,
                        71141,
                        0
                    };
                    Aura* taura = unitTarget->getAuraWithId(regrowth);    //Regrowth
                    if (taura && taura->GetSpellInfo())
                    {
                        uint32 amplitude = taura->GetSpellInfo()->getEffectAmplitude(1) / 1000;
                        if (!amplitude)
                            amplitude = 3;

                        //our hapiness is that we did not store the aura mod amount so we have to recalc it
                        Spell* spell = sSpellFactoryMgr.NewSpell(m_caster, taura->GetSpellInfo(), false, nullptr);
                        uint32 healamount = spell->CalculateEffect(1, unitTarget);
                        delete spell;
                        spell = nullptr;
                        new_dmg = healamount * 18 / amplitude;

                        unitTarget->RemoveAura(taura);

                        //do not remove flag if we still can cast it again
                        uint32 rejuvenation[] =
                        {
                            //SPELL_HASH_REJUVENATION
                            774,
                            1058,
                            1430,
                            2090,
                            2091,
                            3627,
                            8070,
                            8910,
                            9839,
                            9840,
                            9841,
                            12160,
                            15981,
                            20664,
                            20701,
                            25299,
                            26981,
                            26982,
                            27532,
                            28716,
                            28722,
                            28723,
                            28724,
                            31782,
                            32131,
                            38657,
                            42544,
                            48440,
                            48441,
                            53607,
                            64801,
                            66065,
                            67971,
                            67972,
                            67973,
                            69898,
                            70691,
                            71142,
                            0
                        };

                        if (!unitTarget->hasAurasWithId(rejuvenation))
                        {
                            unitTarget->RemoveFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_REJUVENATE);
                            sEventMgr.RemoveEvents(unitTarget, EVENT_REJUVENATION_FLAG_EXPIRE);
                        }
                    }
                    else
                    {
                        uint32 rejuvenation[] =
                        {
                            //SPELL_HASH_REJUVENATION
                            774,
                            1058,
                            1430,
                            2090,
                            2091,
                            3627,
                            8070,
                            8910,
                            9839,
                            9840,
                            9841,
                            12160,
                            15981,
                            20664,
                            20701,
                            25299,
                            26981,
                            26982,
                            27532,
                            28716,
                            28722,
                            28723,
                            28724,
                            31782,
                            32131,
                            38657,
                            42544,
                            48440,
                            48441,
                            53607,
                            64801,
                            66065,
                            67971,
                            67972,
                            67973,
                            69898,
                            70691,
                            71142,
                            0
                        };

                        taura = unitTarget->getAuraWithId(rejuvenation);  //Rejuvenation
                        if (taura  && taura->GetSpellInfo())
                        {
                            uint32 amplitude = taura->GetSpellInfo()->getEffectAmplitude(0) / 1000;
                            if (!amplitude) amplitude = 3;

                            //our happiness is that we did not store the aura mod amount so we have to recalc it
                            Spell* spell = sSpellFactoryMgr.NewSpell(m_caster, taura->GetSpellInfo(), false, nullptr);
                            uint32 healamount = spell->CalculateEffect(0, unitTarget);
                            delete spell;
                            spell = nullptr;
                            new_dmg = healamount * 12 / amplitude;

                            unitTarget->RemoveAura(taura);

                            unitTarget->RemoveFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_REJUVENATE);
                            sEventMgr.RemoveEvents(unitTarget, EVENT_REJUVENATION_FLAG_EXPIRE);
                        }
                    }

                    if (new_dmg > 0)
                    {
                        SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(18562);
                        Spell* spell = sSpellFactoryMgr.NewSpell(unitTarget, spellInfo, true, nullptr);
                        spell->SetUnitTarget(unitTarget);
                        spell->Heal((int32)new_dmg);
                        delete spell;
                    }
                }
            }
            break;
            default:
                Heal(damage);
                break;
        }
    }
}

void Spell::SpellEffectBind(uint8_t effectIndex)
{
    if (!playerTarget || !playerTarget->isAlive() || !m_caster)
        return;

    WorldPacket data(45);
    uint32 areaid = playerTarget->GetZoneId();
    uint32 mapid = playerTarget->GetMapId();
    if (GetSpellInfo()->getEffectMiscValue(effectIndex))
    {
        areaid = GetSpellInfo()->getEffectMiscValue(effectIndex);
        auto at = MapManagement::AreaManagement::AreaStorage::GetAreaById(areaid);
        if (!at)
            return;
        mapid = at->map_id;
    }

    playerTarget->SetBindPoint(playerTarget->GetPositionX(), playerTarget->GetPositionY(), playerTarget->GetPositionZ(), mapid, areaid);

    data.Initialize(SMSG_BINDPOINTUPDATE);
    data << playerTarget->GetBindPositionX();
    data << playerTarget->GetBindPositionY();
    data << playerTarget->GetBindPositionZ();
    data << playerTarget->GetBindMapId();
    data << playerTarget->GetBindZoneId();
    playerTarget->GetSession()->SendPacket(&data);

    data.Initialize(SMSG_PLAYERBOUND);
    data << m_caster->GetGUID();
    data << playerTarget->GetBindZoneId();
    playerTarget->GetSession()->SendPacket(&data);
}

void Spell::SpellEffectQuestComplete(uint8_t effectIndex) // Quest Complete
{
    if (!p_caster) return;
    QuestLogEntry* en = p_caster->GetQuestLogForEntry(GetSpellInfo()->getEffectMiscValue(effectIndex));
    if (en)
    {
        en->Complete();
        en->UpdatePlayerFields();
        en->SendQuestComplete();
    }
}

//wand->
void Spell::SpellEffectWeapondamageNoschool(uint8_t /*effectIndex*/) // Weapon damage + (no School)
{
    if (!unitTarget || !u_caster)
        return;

    u_caster->Strike(unitTarget, (GetType() == SPELL_DMG_TYPE_RANGED ? RANGED : MELEE), GetSpellInfo(), damage, 0, 0, false, true);
}

void Spell::SpellEffectResurrect(uint8_t effectIndex) // Resurrect (Flat)
{
    if (!playerTarget)
    {
        if (!corpseTarget)
        {
            // unit resurrection handler
            if (unitTarget)
            {
                if (unitTarget->IsCreature() && unitTarget->IsPet() && unitTarget->IsDead())
                {
                    uint32 hlth = ((uint32)GetSpellInfo()->getEffectBasePoints(effectIndex) > unitTarget->GetMaxHealth()) ? unitTarget->GetMaxHealth() : (uint32)GetSpellInfo()->getEffectBasePoints(effectIndex);
                    uint32 mana = ((uint32)GetSpellInfo()->getEffectBasePoints(effectIndex) > unitTarget->GetMaxPower(POWER_TYPE_MANA)) ? unitTarget->GetMaxPower(POWER_TYPE_MANA) : (uint32)GetSpellInfo()->getEffectBasePoints(effectIndex);

                    if (!unitTarget->IsPet())
                    {
                        sEventMgr.RemoveEvents(unitTarget, EVENT_CREATURE_REMOVE_CORPSE);
                    }
                    else
                    {
                        sEventMgr.RemoveEvents(unitTarget, EVENT_PET_DELAYED_REMOVE);
                        sEventMgr.RemoveEvents(unitTarget, EVENT_CREATURE_REMOVE_CORPSE);
                    }
                    unitTarget->SetHealth(hlth);
                    unitTarget->SetPower(POWER_TYPE_MANA, mana);
                    unitTarget->setUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
                    unitTarget->setDeathState(ALIVE);
                    static_cast< Creature* >(unitTarget)->UnTag();
                    static_cast< Creature* >(unitTarget)->loot.gold = 0;
                    static_cast< Creature* >(unitTarget)->loot.looters.clear();
                    static_cast< Creature* >(unitTarget)->loot.items.clear();
                    static_cast< Creature* >(unitTarget)->SetLimboState(false); // we can regenerate health now
                }
            }

            return;
        }
        playerTarget = objmgr.GetPlayer(GET_LOWGUID_PART(corpseTarget->GetOwner()));
        if (!playerTarget) return;
    }

    if (playerTarget->isAlive() || !playerTarget->IsInWorld())
        return;

    uint32 health = GetSpellInfo()->getEffectBasePoints(effectIndex);
    uint32 mana = GetSpellInfo()->getEffectMiscValue(effectIndex);

    playerTarget->m_resurrectHealth = health;
    playerTarget->m_resurrectMana = mana;

    SendResurrectRequest(playerTarget);
    playerTarget->setMoveRoot(false);
}

void Spell::SpellEffectAddExtraAttacks(uint8_t /*effectIndex*/) // Add Extra Attacks
{
    if (!u_caster)
        return;
    u_caster->m_extraattacks += damage;
}

void Spell::SpellEffectDodge(uint8_t /*effectIndex*/)
{
    //i think this actually enables the skill to be able to dodge melee+ranged attacks
    //value is static and sets value directly which will be modified by other factors
    //this is only basic value and will be overwritten elsewhere !!!
    //  if (unitTarget->IsPlayer())
    //      unitTarget->SetFloatValue(PLAYER_DODGE_PERCENTAGE,damage);
}

void Spell::SpellEffectParry(uint8_t /*effectIndex*/)
{
    if (unitTarget)
        unitTarget->setcanparry(true);
}

void Spell::SpellEffectBlock(uint8_t /*effectIndex*/)
{
    //i think this actually enables the skill to be able to block melee+ranged attacks
    //value is static and sets value directly which will be modified by other factors
    //  if (unitTarget->IsPlayer())
    //      unitTarget->SetFloatValue(PLAYER_BLOCK_PERCENTAGE,damage);
}

void Spell::SpellEffectCreateItem(uint8_t effectIndex)
{
    uint32 spellid = m_spellInfo->getId();

    if (playerTarget == nullptr)
    {
        LOG_ERROR("Spell %u (%s) has a create item effect but no player target!", spellid, m_spellInfo->getName().c_str());
        return;
    }


    uint32 itemid = m_spellInfo->getEffectItemType(effectIndex);
    uint32 count = 0;
    uint32 basecount = m_spellInfo->getEffectDieSides(effectIndex);
    uint32 difference = m_spellInfo->getEffectBasePoints(effectIndex);

    if (itemid == 0)
    {
        LOG_ERROR("Spell %u (%s) has a create item effect but no itemid to add, Spell needs to be fixed!", spellid, m_spellInfo->getName().c_str());
        return;
    }

    ItemProperties const* m_itemProto = sMySQLStore.getItemProperties(itemid);
    if (m_itemProto == nullptr)
    {
        LOG_ERROR("Spell %u (%s) has a create item effect but the itemid is invalid!", spellid, m_spellInfo->getName().c_str());
        return;
    }

    if (difference > basecount)
    {
        count = basecount + difference;

    }
    else
    {
        uint32 mincount = basecount - difference;
        uint32 maxcount = basecount + difference;
        uint32 variablecount = maxcount - mincount;
        uint32 randcount = RandomUInt(variablecount);

        count = mincount + randcount;
    }

    uint32 countperlevel = static_cast<uint32>(std::round(m_spellInfo->getEffectRealPointsPerLevel(effectIndex)));

    if (countperlevel != 0)
    {
        uint32 leveldiff = m_spellInfo->getMaxLevel() - m_spellInfo->getBaseLevel();
        uint32 countforlevel = leveldiff * countperlevel;

        count += countforlevel;
    }


    if (count <= 0)
    {
        LOG_ERROR("Spell %u (%s) has a create item effect but no item count to add, Spell needs to be fixed! Count overriden to 1.", spellid, m_spellInfo->getName().c_str());
        count = 1;
    }

    if (p_caster != nullptr)
    {
        auto skill_line_ability = objmgr.GetSpellSkill(spellid);

        // potions learned by discovery variables
        uint32 cast_chance = 5;
        uint32 learn_spell = 0;

        // tailoring specializations get +1 cloth bonus
        switch (spellid)
        {

            case 36686: //Shadowcloth
                if (p_caster->HasSpell(26801)) count++;
                break;

            case 26751: // Primal Mooncloth
                if (p_caster->HasSpell(26798)) count++;
                break;

            case 31373: //Spellcloth
                if (p_caster->HasSpell(26797)) count++;
                break;
        }

        if ((skill_line_ability != nullptr) && (skill_line_ability->skilline == SKILL_ALCHEMY))
        {
            //Potion Master
            if (m_itemProto->Name.compare("Potion"))
            {
                if (p_caster->HasSpell(28675))
                    while (Rand(20) && (count < 5))
                        count++;

                // Super Rejuvenation Potion
                cast_chance = 2;
                learn_spell = 28586;
            }

            //Elixir Master
            if (m_itemProto->Name.compare("Elixir") || m_itemProto->Name.compare("Flask"))
            {
                if (p_caster->HasSpell(28677))
                    while (Rand(20) && (count < 5))
                        count++;

                uint32 spList[] = { 28590, 28587, 28588, 28591, 28589 };
                cast_chance = 2;
                learn_spell = spList[RandomUInt(4)];
            }

            //Transmutation Master
            if (m_spellInfo->getCategory() == 310)
            {

                //rate for primal might is lower than for anything else
                if (m_spellInfo->getId() == 29688)
                {
                    if (p_caster->HasSpell(28672))
                        while (Rand(40) && (count < 5))
                            count++;
                }
                else
                {
                    if (p_caster->HasSpell(28672))
                        while (Rand(20) && (count < 5))
                            count++;
                }

                uint32 spList[] = { 28581, 28585, 28585, 28584, 28582, 28580 };
                cast_chance = 5;
                learn_spell = spList[RandomUInt(5)];
            }
        }

        if (!playerTarget->GetItemInterface()->AddItemById(itemid, count, 0))
        {
            SendCastResult(SPELL_FAILED_TOO_MANY_OF_ITEM);
            return;
        }

        if (p_caster != nullptr)
        {

            //random discovery by crafter item id
            switch (itemid)
            {

                case 22845: //Major Arcane Protection Potion
                    cast_chance = 20;
                    learn_spell = 41458;
                    break;

                case 22841: //Major Fire Protection Potion
                    cast_chance = 20;
                    learn_spell = 41500;
                    break;

                case 22842: //Major Frost Protection Potion
                    cast_chance = 20;
                    learn_spell = 41501;
                    break;

                case 22847: //Major Holy Protection Potion
                    // there is none
                    break;

                case 22844: //Major Nature Protection Potion
                    cast_chance = 20;
                    learn_spell = 41502;
                    break;

                case 22846: //Major Shadow Protection Potion
                    cast_chance = 20;
                    learn_spell = 41503;
                    break;
            }

            if ((learn_spell != 0) && (p_caster->getLevel() > 60) && !p_caster->HasSpell(learn_spell) && Rand(cast_chance))
            {
                SpellInfo* dspellproto = sSpellCustomizations.GetSpellInfo(learn_spell);

                if (dspellproto != nullptr)
                {
                    p_caster->BroadcastMessage("%sDISCOVERY! You discovered the %s !|r", MSG_COLOR_YELLOW, dspellproto->getName().c_str());
                    p_caster->addSpell(learn_spell);
                }
                else
                {
                    LOG_ERROR("Spell %u (%s) Effect Index %hhu tried to teach a non-existing Spell %u in %s:%u", spellid, m_spellInfo->getName().c_str(), effectIndex, learn_spell, __FILE__, __LINE__);
                }
            }
        }

        if (skill_line_ability != nullptr)
        {
            DetermineSkillUp(skill_line_ability->skilline);

            uint32 discovered_recipe = 0;

            for (std::set<MySQLStructure::ProfessionDiscovery*>::iterator itr = sMySQLStore._professionDiscoveryStore.begin(); itr != sMySQLStore._professionDiscoveryStore.end(); ++itr)
            {
                MySQLStructure::ProfessionDiscovery* pf = *itr;
                if (spellid == pf->SpellId && p_caster->_GetSkillLineCurrent(skill_line_ability->skilline) >= pf->SkillValue && !p_caster->HasSpell(pf->SpellToDiscover) && Rand(pf->Chance))
                {
                    discovered_recipe = pf->SpellToDiscover;
                    break;
                }
            }

            // if something was discovered teach player that recipe and broadcast message
            if (discovered_recipe != 0)
            {
                SpellInfo* se = sSpellCustomizations.GetSpellInfo(discovered_recipe);

                if (se != nullptr)
                {
                    p_caster->addSpell(discovered_recipe);

                    WorldPacket* data;
                    char msg[256];
                    sprintf(msg, "%sDISCOVERY! %s has discovered how to create %s.|r", MSG_COLOR_GOLD, p_caster->GetName(), se->getName().c_str());
                    data = sChatHandler.FillMessageData(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, msg, p_caster->GetGUID(), 0);
                    p_caster->GetMapMgr()->SendChatMessageToCellPlayers(p_caster, data, 2, 1, LANG_UNIVERSAL, p_caster->GetSession());
                    delete data;
                }
                else
                {
                    LOG_ERROR("Spell %u (%s) Effect index %hhu tried to teach a non-existing Spell %u in %s:%u", spellid, m_spellInfo->getName().c_str(), effectIndex, learn_spell, __FILE__, __LINE__);
                }
            }
        }
    }
    else
    {
        if (!playerTarget->GetItemInterface()->AddItemById(itemid, count, 0))
            SendCastResult(SPELL_FAILED_TOO_MANY_OF_ITEM);
    }
}

void Spell::SpellEffectWeapon(uint8_t /*effectIndex*/)
{
    if (!playerTarget)
        return;

    uint32 skill = 0;
    uint32 spell = 0;

    switch (this->GetSpellInfo()->getId())
    {
        case 201:    // one-handed swords
        {
            skill = SKILL_SWORDS;
        }
        break;
        case 202:   // two-handed swords
        {
            skill = SKILL_2H_SWORDS;
        }
        break;
        case 203:   // Unarmed
        {
            skill = SKILL_UNARMED;
        }
        break;
        case 199:   // two-handed maces
        {
            skill = SKILL_2H_MACES;
        }
        break;
        case 198:   // one-handed maces
        {
            skill = SKILL_MACES;
        }
        break;
        case 197:   // two-handed axes
        {
            skill = SKILL_2H_AXES;
        }
        break;
        case 196:   // one-handed axes
        {
            skill = SKILL_AXES;
        }
        break;
        case 5011: // crossbows
        {
            skill = SKILL_CROSSBOWS;
            spell = SPELL_RANGED_GENERAL;
        }
        break;
        case 227:   // staves
        {
            skill = SKILL_STAVES;
        }
        break;
        case 1180:  // daggers
        {
            skill = SKILL_DAGGERS;
        }
        break;
        case 200:   // polearms
        {
            skill = SKILL_POLEARMS;
        }
        break;
        case 15590: // fist weapons
        {
            skill = SKILL_UNARMED;
        }
        break;
        case 264:   // bows
        {
            skill = SKILL_BOWS;
            spell = SPELL_RANGED_GENERAL;
        }
        break;
        case 266: // guns
        {
            skill = SKILL_GUNS;
            spell = SPELL_RANGED_GENERAL;
        }
        break;
        case 2567:  // thrown
        {
            skill = SKILL_THROWN;
        }
        break;
        case 5009:  // wands
        {
            skill = SKILL_WANDS;
        }
        break;
        case 2382:  // Generic
        {
            // Passiv Spell, Aura hidden
        }
        break;
        default:
        {
            skill = 0;
            LogDebugFlag(LF_SPELL_EFF, "WARNING: Could not determine skill for spell id %d (SPELL_EFFECT_WEAPON)", this->GetSpellInfo()->getId());
        }
        break;
    }

    if (skill)
    {
        if (spell)
            playerTarget->addSpell(spell);

        // if we do not have the skill line
        if (!playerTarget->_HasSkillLine(skill))
        {
            playerTarget->_AddSkillLine(skill, 1, playerTarget->getLevel() * 5);
        }
        else // unhandled.... if we have the skill line
        {
        }
    }
}

void Spell::SpellEffectDefense(uint8_t /*effectIndex*/)
{
    //i think this actually enables the skill to be able to use defense
    //value is static and sets value directly which will be modified by other factors
    //this is only basic value and will be overwritten elsewhere !!!
    //  if (unitTarget->IsPlayer())
    //      unitTarget->SetFloatValue(UNIT_FIELD_RESISTANCES,damage);
}

void Spell::SpellEffectPersistentAA(uint8_t effectIndex) // Persistent Area Aura
{
    if (m_AreaAura || !m_caster->IsInWorld())
        return;
    //create only 1 dyn object
    uint32 dur = GetDuration();
    float r = GetRadius(effectIndex);

    //Note: this code seems to be useless
    //this must be only source point or dest point
    //this AREA aura it's applied on area
    //it can'be on unit or self or item or object
    //uncomment it if I'm wrong
    //We are thinking in general so it might be useful later DK

    // grep: this is a hack!
    // our shitty dynobj system doesn't support GO casters, so we gotta
    // kinda have 2 summoners for traps that apply AA.
    DynamicObject* dynObj = m_caster->GetMapMgr()->CreateDynamicObject();

    if (g_caster != nullptr && g_caster->m_summoner && !unitTarget)
    {
        Unit* caster = g_caster->m_summoner;
        dynObj->Create(caster, this, g_caster->GetPositionX(), g_caster->GetPositionY(),
                       g_caster->GetPositionZ(), dur, r, DYNAMIC_OBJECT_AREA_SPELL);
        m_AreaAura = true;
        return;
    }

    switch (m_targets.m_targetMask)
    {
        case TARGET_FLAG_SELF:
        {
            dynObj->Create(u_caster, this, m_caster->GetPositionX(),
                           m_caster->GetPositionY(), m_caster->GetPositionZ(), dur, r, DYNAMIC_OBJECT_AREA_SPELL);
        }
        break;
        case TARGET_FLAG_UNIT:
        {
            if (!unitTarget || !unitTarget->isAlive())
            {
                dynObj->Remove();
                return;
            }

            dynObj->Create(u_caster, this, unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(),
                           dur, r, DYNAMIC_OBJECT_AREA_SPELL);
        }
        break;
        case TARGET_FLAG_OBJECT:
        {
            if (!unitTarget || !unitTarget->isAlive())
            {
                dynObj->Remove();
                return;
            }

            dynObj->Create(u_caster, this, unitTarget->GetPositionX(), unitTarget->GetPositionY(), unitTarget->GetPositionZ(),
                           dur, r, DYNAMIC_OBJECT_AREA_SPELL);
        }
        break;
        case TARGET_FLAG_SOURCE_LOCATION:
        {
            auto source = m_targets.source();
            dynObj->Create(u_caster, this, source.x, source.y, source.z, dur, r, DYNAMIC_OBJECT_AREA_SPELL);
        }
        break;
        case TARGET_FLAG_DEST_LOCATION:
        {
            auto destination = m_targets.destination();
            if (u_caster != nullptr)
                dynObj->Create(u_caster, this, destination.x, destination.y, destination.z, dur, r, DYNAMIC_OBJECT_AREA_SPELL);
            else if (g_caster != nullptr)
                dynObj->Create(g_caster->m_summoner, this, destination.x, destination.y, destination.z, dur, r, DYNAMIC_OBJECT_AREA_SPELL);
        }
        break;
        default:
            dynObj->Remove();
            return;
    }

    if (u_caster)
        if (GetSpellInfo()->getChannelInterruptFlags() > 0)
        {
            u_caster->SetChannelSpellTargetGUID(dynObj->GetGUID());
            u_caster->SetChannelSpellId(GetSpellInfo()->getId());
        }

    m_AreaAura = true;
}

void Spell::SpellEffectSummon(uint8_t effectIndex)
{
    uint32 summonpropid = m_spellInfo->getEffectMiscValueB(effectIndex);

    auto summon_properties = sSummonPropertiesStore.LookupEntry(summonpropid);
    if (summon_properties == nullptr)
    {
        LOG_ERROR("No SummonPropertiesEntry for Spell %u (%s)", m_spellInfo->getId(), m_spellInfo->getName().c_str());
        return;
    }

    uint32 entry = m_spellInfo->getEffectMiscValue(effectIndex);

    CreatureProperties const* cp = sMySQLStore.getCreatureProperties(entry);

    if (cp == nullptr)
    {
        LOG_ERROR("Spell %u (%s) tried to summon creature %u without database data", m_spellInfo->getId(), m_spellInfo->getName().c_str(), entry);
        return;
    }

    LocationVector v(0.0f, 0.0f, 0.0f, 0.0f);

    if ((m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION) != 0)
        v = m_targets.destination();
    else
        v = m_caster->GetPosition();

    // Ugly hack to make sure we always summon at least 1
    if (damage == 0)
        damage = 1;

    // Client adds these spells to the companion window, it's weird but then it happens anyways
    if (summon_properties->Slot == 5)
    {
        SpellEffectSummonCompanion(effectIndex, summon_properties, cp, v);
        return;
    }

    switch (summon_properties->ControlType)
    {
        case SUMMON_CONTROL_TYPE_GUARDIAN:
            if (summon_properties->ID == 121)
            {
                SpellEffectSummonTotem(effectIndex, summon_properties, cp, v);
                return;
            }
            break;

        case SUMMON_CONTROL_TYPE_PET:
            SpellEffectSummonTemporaryPet(effectIndex, summon_properties, cp, v);
            return;

        case SUMMON_CONTROL_TYPE_POSSESSED:
            SpellEffectSummonPossessed(effectIndex, summon_properties, cp, v);
            return;

        case SUMMON_CONTROL_TYPE_VEHICLE:
            SpellEffectSummonVehicle(effectIndex, summon_properties, cp, v);
            return;
    }

    switch (summon_properties->Type)
    {
        case SUMMON_TYPE_NONE:
        case SUMMON_TYPE_CONSTRUCT:
        case SUMMON_TYPE_OPPONENT:

            if (summon_properties->ControlType == SUMMON_CONTROL_TYPE_GUARDIAN)
                SpellEffectSummonGuardian(effectIndex, summon_properties, cp, v);
            else
                SpellEffectSummonWild(effectIndex);

            return;

        case SUMMON_TYPE_PET:
            SpellEffectSummonTemporaryPet(effectIndex, summon_properties, cp, v);
            return;

        case SUMMON_TYPE_GUARDIAN:
        case SUMMON_TYPE_MINION:
        case SUMMON_TYPE_RUNEBLADE:
            SpellEffectSummonGuardian(effectIndex, summon_properties, cp, v);
            return;

        case SUMMON_TYPE_TOTEM:
            SpellEffectSummonTotem(effectIndex, summon_properties, cp, v);
            return;

        case SUMMON_TYPE_COMPANION:
            // These are used as guardians in some quests
            if (summon_properties->Slot == 6)
                SpellEffectSummonGuardian(effectIndex, summon_properties, cp, v);
            else
                SpellEffectSummonCompanion(effectIndex, summon_properties, cp, v);
            return;

        case SUMMON_TYPE_VEHICLE:
        case SUMMON_TYPE_MOUNT:
            SpellEffectSummonVehicle(effectIndex, summon_properties, cp, v);
            return;

        case SUMMON_TYPE_LIGHTWELL:
            SpellEffectSummonGuardian(effectIndex, summon_properties, cp, v);
            return;
    }

    LOG_ERROR("Unknown summon type in summon property %u in spell %u %s", summonpropid, m_spellInfo->getId(), m_spellInfo->getName().c_str());
}

void Spell::SpellEffectSummonWild(uint8_t effectIndex)  // Summon Wild
{
    //these are some creatures that have your faction and do not respawn
    //number of creatures is actually dmg (the usual formula), sometimes =3 sometimes =1
    //if( u_caster == NULL || !u_caster->IsInWorld() )
    //	return;

    if ((!m_caster->IsGameObject() && !m_caster->IsUnit()) || !m_caster->IsInWorld())
        return;

    uint32 cr_entry = GetSpellInfo()->getEffectMiscValue(effectIndex);
    CreatureProperties const* properties = sMySQLStore.getCreatureProperties(cr_entry);
    if (properties == nullptr)
    {
        LogError("Warning : Missing summon creature template %u used by spell %u!", cr_entry, GetSpellInfo()->getId());
        return;
    }
    float x, y, z;
    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION && m_targets.destination().isSet())
    {
        auto destination = m_targets.destination();
        x = destination.x;
        y = destination.y;
        z = destination.z;
    }
    else
    {
        x = m_caster->GetPositionX();
        y = m_caster->GetPositionY();
        z = m_caster->GetPositionZ();
    }
    for (int j = 0; j<damage; j++)
    {
        float m_fallowAngle = -((float(M_PI) / 2) * j);
        float tempx = x + (GetRadius(effectIndex) * (cosf(m_fallowAngle + m_caster->GetOrientation())));
        float tempy = y + (GetRadius(effectIndex) * (sinf(m_fallowAngle + m_caster->GetOrientation())));
        Creature* p = m_caster->GetMapMgr()->CreateCreature(cr_entry);

        ASSERT(p != NULL);

        p->Load(properties, tempx, tempy, z);
        p->SetZoneId(m_caster->GetZoneId());

        if (p->GetCreatureProperties()->Faction == 35)
        {
            p->SetSummonedByGUID(m_caster->GetGUID());
            p->SetCreatedByGUID(m_caster->GetGUID());

            if (m_caster->IsGameObject())
                p->SetFaction(static_cast<GameObject*>(m_caster)->GetFaction());
            else
                p->SetFaction(static_cast<Unit*>(m_caster)->GetFaction());
        }
        else
        {
            p->SetFaction(properties->Faction);
        }
        p->PushToWorld(m_caster->GetMapMgr());
        //make sure they will be desummoned (roxor)
        sEventMgr.AddEvent(p, &Creature::SummonExpire, EVENT_SUMMON_EXPIRE, GetDuration(), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Spell::SpellEffectSummonGuardian(uint32 /*i*/, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v)
{

    if (g_caster != nullptr)
        u_caster = g_caster->m_summoner;

    if (u_caster == nullptr)
        return;

    float angle_for_each_spawn = -M_PI_FLOAT * 2 / damage;

    for (int j = 0; j < damage; j++)
    {
        float followangle = angle_for_each_spawn * j;

        float x = 3 * (cosf(followangle + u_caster->GetOrientation()));
        float y = 3 * (sinf(followangle + u_caster->GetOrientation()));

        v.x += x;
        v.y += y;

        Summon* s = u_caster->GetMapMgr()->CreateSummon(properties_->Id, SUMMONTYPE_GUARDIAN);
        if (s == nullptr)
            return;

        s->Load(properties_, u_caster, v, m_spellInfo->getId(), spe->Slot - 1);
        s->GetAIInterface()->SetUnitToFollowAngle(followangle);
        s->PushToWorld(u_caster->GetMapMgr());

        if ((p_caster != nullptr) && (spe->Slot != 0))
            p_caster->SendTotemCreated(static_cast<uint8_t>(spe->Slot - 1), s->GetGUID(), GetDuration(), m_spellInfo->getId());

        // Lightwell
        if (spe->Type == SUMMON_TYPE_LIGHTWELL)
        {
            s->setMoveRoot(true);
            s->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        }

        int32 duration = static_cast<int32>(GetDuration());
        if (duration > 0)
            sEventMgr.AddEvent(static_cast< Object* >(s), &Object::Delete, EVENT_SUMMON_EXPIRE, GetDuration(), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    }
}

void Spell::SpellEffectSummonTemporaryPet(uint32 /*i*/, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v)
{
    if (p_caster == nullptr)
        return;

    p_caster->DismissActivePets();
    p_caster->RemoveFieldSummon();

    int32 count = 0;

    // Only Inferno uses this SummonProperty ID, and somehow it has the wrong count
    if (spe->ID == 711)
        count = 1;
    else
        count = damage;

    // We know for sure that this will suceed because we checked in Spell::SpellEffectSummon
    CreatureProperties const* ci = sMySQLStore.getCreatureProperties(properties_->Id);

    float angle_for_each_spawn = -M_PI_FLOAT * 2 / damage;

    for (int32 i = 0; i < count; i++)
    {
        float followangle = angle_for_each_spawn * i;

        float x = 3 * (cosf(followangle + u_caster->GetOrientation()));
        float y = 3 * (sinf(followangle + u_caster->GetOrientation()));

        v.x += x;
        v.y += y;

        Pet* pet = objmgr.CreatePet(properties_->Id);

        if (!pet->CreateAsSummon(properties_->Id, ci, nullptr, p_caster, m_spellInfo, 1, GetDuration(), &v, false))
        {
            pet->DeleteMe();
            pet = nullptr;
            break;
        }

        pet->GetAIInterface()->SetUnitToFollowAngle(followangle);
    }
}

void Spell::SpellEffectSummonTotem(uint32 /*i*/, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v)
{
    if (u_caster == nullptr)
        return;

    int slot = spe->Slot;
    v.x += (slot < 3) ? -1.5f : 1.5f;
    v.y += (slot % 2) ? -1.5f : 1.5f;

    float landh = u_caster->GetMapMgr()->GetLandHeight(v.x, v.y, v.z + 2);
    float landdiff = landh - v.z;

    if (fabs(landdiff) <= 15)
        v.z = landh;

    Summon* s = u_caster->GetMapMgr()->CreateSummon(properties_->Id, SUMMONTYPE_TOTEM);
    if (s == nullptr)
        return;
    s->Load(properties_, u_caster, v, m_spellInfo->getId(), spe->Slot - 1);
    s->SetMaxHealth(damage);
    s->SetHealth(damage);
    s->PushToWorld(u_caster->GetMapMgr());

    if (p_caster != nullptr)
        p_caster->SendTotemCreated(static_cast<uint8_t>(spe->Slot - 1), s->GetGUID(), GetDuration(), m_spellInfo->getId());

    int32 duration = static_cast<int32>(GetDuration());
    if (duration > 0)
        sEventMgr.AddEvent(static_cast< Object* >(s), &Object::Delete, EVENT_SUMMON_EXPIRE, GetDuration(), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    else
        sEventMgr.AddEvent(static_cast< Object* >(s), &Object::Delete, EVENT_SUMMON_EXPIRE, 60 * 60 * 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Spell::SpellEffectSummonPossessed(uint32 /*i*/, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v)
{
    if (p_caster == nullptr)
        return;

    p_caster->DismissActivePets();
    p_caster->RemoveFieldSummon();

    Summon* s = p_caster->GetMapMgr()->CreateSummon(properties_->Id, SUMMONTYPE_POSSESSED);
    if (s == nullptr)
        return;

    v.x += (3 * cos(M_PI_FLOAT / 2 + v.o));
    v.y += (3 * cos(M_PI_FLOAT / 2 + v.o));

    s->Load(properties_, p_caster, v, m_spellInfo->getId(), spe->Slot - 1);
    s->SetCreatedBySpell(m_spellInfo->getId());
    s->PushToWorld(p_caster->GetMapMgr());

    p_caster->Possess(s, 1000);
}

void Spell::SpellEffectSummonCompanion(uint32 i, DBC::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v)
{
    if (u_caster == nullptr)
        return;

    if (u_caster->GetSummonedCritterGUID() != 0)
    {
        auto critter = u_caster->GetMapMgr()->GetUnit(u_caster->GetSummonedCritterGUID());
        if (critter == nullptr)
            return;

        auto creature = static_cast< Creature* >(critter);

        uint32 currententry = creature->GetCreatureProperties()->Id;

        creature->RemoveFromWorld(false, true);
        u_caster->SetSummonedCritterGUID(0);

        // Before WOTLK when you casted the companion summon spell the second time it removed the companion
        // Customized servers or old databases could still use this method
        if (properties_->Id == currententry)
            return;
    }

    auto summon = u_caster->GetMapMgr()->CreateSummon(properties_->Id, SUMMONTYPE_COMPANION);
    if (summon == nullptr)
        return;

    summon->Load(properties_, u_caster, v, m_spellInfo->getId(), spe->Slot - 1);
    summon->SetCreatedBySpell(m_spellInfo->getId());
    summon->GetAIInterface()->SetFollowDistance(GetRadius(i));
    summon->PushToWorld(u_caster->GetMapMgr());
    u_caster->SetSummonedCritterGUID(summon->GetGUID());
}

void Spell::SpellEffectSummonVehicle(uint32 /*i*/, DBC::Structures::SummonPropertiesEntry const* /*spe*/, CreatureProperties const* properties_, LocationVector& v)
{
    if (u_caster == nullptr)
        return;

    // If it has no vehicle id, then we can't really do anything with it as a vehicle :/
    if ((properties_->vehicleid == 0) && (p_caster == nullptr))
        return;

    Creature* c = u_caster->GetMapMgr()->CreateCreature(properties_->Id);
    c->Load(properties_, v.x, v.y, v.z, v.o);
    c->Phase(PHASE_SET, u_caster->GetPhase());
    c->SetCreatedBySpell(m_spellInfo->getId());
    c->SetCreatedByGUID(u_caster->GetGUID());
    c->SetSummonedByGUID(u_caster->GetGUID());
    c->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
    c->PushToWorld(u_caster->GetMapMgr());

    // Need to delay this a bit since first the client needs to see the vehicle
    u_caster->EnterVehicle(c->GetGUID(), 1 * 1000);
}

void Spell::SpellEffectLeap(uint8_t effectIndex) // Leap
{
    if (unitTarget == nullptr)
        return;

    float radius = GetRadius(effectIndex);
    unitTarget->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN);

    MMAP::MMapManager* mmap = MMAP::MMapFactory::createOrGetMMapManager();
    dtNavMesh* nav = const_cast<dtNavMesh*>(mmap->GetNavMesh(m_caster->GetMapId()));
    // dtNavMeshQuery* nav_query = const_cast<dtNavMeshQuery*>(mmap->GetNavMeshQuery(m_caster->GetMapId(), m_caster->GetInstanceID()));
    //NavMeshData* nav = CollideInterface.GetNavMesh(m_caster->GetMapId());

    if (nav != nullptr)
    {
        float destx, desty, destz;
        unitTarget->GetPoint(unitTarget->GetOrientation(), radius, destx, desty, destz);
        if (playerTarget != nullptr)
            playerTarget->SafeTeleport(playerTarget->GetMapId(), playerTarget->GetInstanceID(), LocationVector(destx, desty, destz, playerTarget->GetOrientation()));
        else if (unitTarget != nullptr)
            unitTarget->GetAIInterface()->splineMoveJump(destx, desty, destz, unitTarget->GetOrientation());
    }
    else
    {
        if (playerTarget == nullptr)  //let client handle this for players
            return;

        WorldPacket data(SMSG_MOVE_KNOCK_BACK, 50);
        data << playerTarget->GetNewGUID();
        data <<Util::getMSTime();
        data << cosf(playerTarget->GetOrientation());
        data << sinf(playerTarget->GetOrientation());
        data << radius;
        data << float(-10.0f);
        playerTarget->GetSession()->SendPacket(&data);
    }
}

void Spell::SpellEffectEnergize(uint8_t effectIndex) // Energize
{
    if (!unitTarget || !unitTarget->isAlive())
        return;

    uint32 modEnergy = 0;
    switch (GetSpellInfo()->getId())
    {
        case 30824: // Shamanistic Rage
            modEnergy = damage * GetUnitTarget()->GetAP() / 100;
            break;
        case 31786: // Paladin - Spiritual Attunement
            if (ProcedOnSpell)
            {
                SpellInfo* motherspell = sSpellCustomizations.GetSpellInfo(pSpellId);
                if (motherspell)
                {
                    //heal amount from procspell (we only proceed on a heal spell)
                    uint32 healamt = 0;
                    if (ProcedOnSpell->getEffect(0) == SPELL_EFFECT_HEAL || ProcedOnSpell->getEffect(0) == SPELL_EFFECT_SCRIPT_EFFECT)
                        healamt = ProcedOnSpell->getEffectBasePoints(0) + 1;
                    else if (ProcedOnSpell->getEffect(1) == SPELL_EFFECT_HEAL || ProcedOnSpell->getEffect(1) == SPELL_EFFECT_SCRIPT_EFFECT)
                        healamt = ProcedOnSpell->getEffectBasePoints(1) + 1;
                    else if (ProcedOnSpell->getEffect(2) == SPELL_EFFECT_HEAL || ProcedOnSpell->getEffect(2) == SPELL_EFFECT_SCRIPT_EFFECT)
                        healamt = ProcedOnSpell->getEffectBasePoints(2) + 1;
                    modEnergy = (motherspell->getEffectBasePoints(0) + 1) * (healamt) / 100;
                }
            }
            break;
        case 57669:
        {
            modEnergy = float2int32(0.01f * unitTarget->getUInt32Value(UNIT_FIELD_BASE_MANA));
        }
        break;
        case 31930:
        {
            if (u_caster)
                modEnergy = float2int32(0.25f * unitTarget->getUInt32Value(UNIT_FIELD_BASE_MANA));
        }
        break;
        case 2687: // Improved Bloodrage, dirty fix
        {
            modEnergy = damage;
            if (p_caster)
            {
                if (p_caster->mSpells.find(12818) != p_caster->mSpells.end())
                    modEnergy += 110; //60
                if (p_caster->mSpells.find(12301) != p_caster->mSpells.end())
                    modEnergy += 60; //30
            }
        }
        break;
        default:
            modEnergy = damage;
            break;
    }

    if (unitTarget->HasAura(17619))
    {
        modEnergy = uint32(modEnergy * 1.4f);
    }

    u_caster->Energize(unitTarget, GetSpellInfo()->getId(), modEnergy, GetSpellInfo()->getEffectMiscValue(effectIndex));
}

void Spell::SpellEffectWeaponDmgPerc(uint8_t effectIndex) // Weapon Percent damage
{
    if (!unitTarget || !u_caster) return;

    if (GetType() == SPELL_DMG_TYPE_MAGIC)
    {
        uint32 dmg = CalculateDamage(u_caster, unitTarget, MELEE, nullptr, GetSpellInfo()) * damage / 100;
        u_caster->SpellNonMeleeDamageLog(unitTarget, GetSpellInfo()->getId(), dmg, false, false, false);
    }
    else
    {
        uint32 _type;
        if (GetType() == SPELL_DMG_TYPE_RANGED)
            _type = RANGED;
        else
        {
            if (GetSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_TYPE_OFFHAND)
                _type = OFFHAND;
            else
                _type = MELEE;
        }

        u_caster->Strike(unitTarget, _type, GetSpellInfo(), add_damage, damage, 0, false, true);

        if (p_caster != nullptr)   // rogue - fan of knives
        {
            switch (GetSpellInfo()->getId())
            {
                // SPELL_HASH_FAN_OF_KNIVES
                case 51723:
                case 52874:
                case 61739:
                case 61740:
                case 61741:
                case 61742:
                case 61743:
                case 61744:
                case 61745:
                case 61746:
                case 63753:
                case 65955:
                case 67706:
                case 68097:
                case 68098:
                case 68099:
                case 69921:
                case 71128:
                {
                    Item* oit = p_caster->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                    if (oit != nullptr)
                    {
                        if (oit->GetDurability() != 0)
                        {
                            if (oit->GetItemProperties()->Class == 2 && oit->GetItemProperties()->SubClass == 15)   // daggers
                                damage = 105; //causing 105% weapon damage with daggers
                            else
                                damage = GetSpellInfo()->getEffectBasePoints(effectIndex) + 1;// and 70% weapon damage with all other weapons.

                            u_caster->Strike(unitTarget, OFFHAND, GetSpellInfo(), add_damage, damage, 0, false, true);
                        }
                    }
                } break;
            }
        }
    }
}

void Spell::SpellEffectTriggerMissile(uint8_t effectIndex) // Trigger Missile
{
    //Used by mortar team
    //Triggers area effect spell at destinatiom

    uint32 spellid = GetSpellInfo()->getEffectTriggerSpell(effectIndex);
    if (spellid == 0)
    {
        LOG_ERROR("Spell %u (%s) has a trigger missle effect index (%hhu) but no trigger spell ID. Spell needs fixing.", m_spellInfo->getId(), m_spellInfo->getName().c_str(), effectIndex);
        return;
    }

    SpellInfo* spInfo = sSpellCustomizations.GetSpellInfo(spellid);
    if (spInfo == nullptr)
    {
        LOG_ERROR("Spell %u (%s) has a trigger missle effect index (%hhu) but has an invalid trigger spell ID. Spell needs fixing.", m_spellInfo->getId(), m_spellInfo->getName().c_str(), effectIndex);
        return;
    }

    // Cast the triggered spell on the destination location, spells like Freezing Arrow use it
    if ((u_caster != nullptr) && (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION))
    {
        u_caster->CastSpellAoF(m_targets.destination(), spInfo, true);
        return;
    }

    float spellRadius = GetRadius(effectIndex);

    ///\todo Following should be / is probably in SpellTarget code
    for (std::set<Object*>::iterator itr = m_caster->GetInRangeSetBegin(); itr != m_caster->GetInRangeSetEnd(); ++itr)
    {
        if (!((*itr)->IsUnit()) || !static_cast< Unit* >((*itr))->isAlive())
            continue;
        Unit* t = static_cast< Unit* >((*itr));

        float r;
        auto destination = m_targets.destination();
        float d = destination.x - t->GetPositionX();
        r = d * d;
        d = destination.y - t->GetPositionY();
        r += d * d;
        d = destination.z - t->GetPositionZ();
        r += d * d;

        if (sqrt(r) > spellRadius) continue;

        if (!isAttackable(m_caster, *itr))//Fix Me: only enemy targets?
            continue;

        Spell* sp = sSpellFactoryMgr.NewSpell(m_caster, spInfo, true, nullptr);
        SpellCastTargets tgt;
        tgt.m_unitTarget = (*itr)->GetGUID();
        sp->prepare(&tgt);
    }
}

void Spell::SpellEffectOpenLock(uint8_t effectIndex)
{
    if (!p_caster)
        return;

    uint8 loottype = 0;

    uint32 locktype = GetSpellInfo()->getEffectMiscValue(effectIndex);
    switch (locktype)
    {
        case LOCKTYPE_PICKLOCK:
        {
            uint32 v = 0;
            uint32 lockskill = p_caster->_GetSkillLineCurrent(SKILL_LOCKPICKING);

            if (itemTarget)
            {
                if (!itemTarget->locked)
                    return;

                auto lock = sLockStore.LookupEntry(itemTarget->GetItemProperties()->LockId);
                if (!lock)
                    return;

                for (uint8 j = 0; j < LOCK_NUM_CASES; ++j)
                {
                    if (lock->locktype[j] == 2 && lock->minlockskill[j] && lockskill >= lock->minlockskill[j])
                    {
                        v = lock->minlockskill[j];
                        itemTarget->locked = false;
                        itemTarget->UnLock();
                        DetermineSkillUp(SKILL_LOCKPICKING, v / 5);
                        break;
                    }
                }
            }
            else if (gameObjTarget)
            {
                auto gameobject_info = gameObjTarget->GetGameObjectProperties();
                if (gameObjTarget->GetState() == 0)
                    return;

                auto lock = sLockStore.LookupEntry(gameobject_info->raw.parameter_0);
                if (lock == nullptr)
                    return;

                for (uint8 j = 0; j < LOCK_NUM_CASES; ++j)
                {
                    if (lock->locktype[j] == 2 && lock->minlockskill[j] && lockskill >= lock->minlockskill[j])
                    {
                        v = lock->minlockskill[j];
                        gameObjTarget->SetFlags(GO_FLAG_NONE);
                        gameObjTarget->SetState(GO_STATE_CLOSED);
                        //Add Fill GO loot here
                        if (gameObjTarget->IsLootable())
                        {
                            GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(gameObjTarget);
                            if (pLGO->loot.items.size() == 0)
                            {
                                if (gameObjTarget->GetMapMgr() != nullptr)
                                    lootmgr.FillGOLoot(&pLGO->loot, gameObjTarget->GetGameObjectProperties()->raw.parameter_1, gameObjTarget->GetMapMgr()->iInstanceMode);
                                else
                                    lootmgr.FillGOLoot(&pLGO->loot, gameObjTarget->GetGameObjectProperties()->raw.parameter_1, 0);

                                DetermineSkillUp(SKILL_LOCKPICKING, v / 5);     //to prevent free skill up
                            }
                        }
                        loottype = LOOT_CORPSE;
                        //End of it
                        break;
                    }
                }
            }
        }
        break;
        case LOCKTYPE_HERBALISM:
        {
            if (!gameObjTarget) return;

            uint32 v = gameObjTarget->GetGOReqSkill();
            bool bAlreadyUsed = false;

            if (static_cast<Player*>(m_caster)->_GetSkillLineCurrent(SKILL_HERBALISM) < v)
            {
                //SendCastResult(SPELL_FAILED_LOW_CASTLEVEL);
                return;
            }
            else
            {
                if (gameObjTarget->IsLootable())
                {
                    GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(gameObjTarget);

                    if (pLGO->loot.items.size() == 0)
                    {
                        if (gameObjTarget->GetMapMgr() != nullptr)
                            lootmgr.FillGOLoot(&pLGO->loot, gameObjTarget->GetGameObjectProperties()->raw.parameter_1, gameObjTarget->GetMapMgr()->iInstanceMode);
                        else
                            lootmgr.FillGOLoot(&pLGO->loot, gameObjTarget->GetGameObjectProperties()->raw.parameter_1, 0);
                    }
                    else
                    {
                        bAlreadyUsed = true;
                    }
                }
            }
            loottype = LOOT_SKINNING;

            //Skill up
            if (!bAlreadyUsed) //Avoid cheats with opening/closing without taking the loot
                DetermineSkillUp(SKILL_HERBALISM, v / 5);
        }
        break;
        case LOCKTYPE_MINING:
        {
            if (!gameObjTarget)
                return;

            uint32 v = gameObjTarget->GetGOReqSkill();
            bool bAlreadyUsed = false;

            if (static_cast<Player*>(m_caster)->_GetSkillLineCurrent(SKILL_MINING) < v)
            {
                //SendCastResult(SPELL_FAILED_LOW_CASTLEVEL);
                return;
            }
            else if (gameObjTarget->IsLootable())
            {
                GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(gameObjTarget);
                if (pLGO->loot.items.size() == 0)
                {
                    if (gameObjTarget->GetMapMgr() != nullptr)
                        lootmgr.FillGOLoot(&pLGO->loot, gameObjTarget->GetGameObjectProperties()->raw.parameter_1, gameObjTarget->GetMapMgr()->iInstanceMode);
                    else
                        lootmgr.FillGOLoot(&pLGO->loot, gameObjTarget->GetGameObjectProperties()->raw.parameter_1, 0);
                }
            }
            else
            {
                bAlreadyUsed = true;
            }
            loottype = LOOT_SKINNING;

            //Skill up
            if (!bAlreadyUsed) //Avoid cheats with opening/closing without taking the loot
                DetermineSkillUp(SKILL_MINING, v / 5);
        }
        break;
        case LOCKTYPE_SLOW_OPEN: // used for BG go's
        {
            if (!gameObjTarget) return;
            if (p_caster && p_caster->m_bg)
                if (p_caster->m_bg->HookSlowLockOpen(gameObjTarget, p_caster, this))
                    return;

            uint32 spellid = !gameObjTarget->GetGameObjectProperties()->raw.parameter_10 ? 23932 : gameObjTarget->GetGameObjectProperties()->raw.parameter_10;
            SpellInfo* en = sSpellCustomizations.GetSpellInfo(spellid);
            Spell* sp = sSpellFactoryMgr.NewSpell(p_caster, en, true, nullptr);
            SpellCastTargets tgt;
            tgt.m_unitTarget = gameObjTarget->GetGUID();
            sp->prepare(&tgt);
            return;
        }
        break;
        case LOCKTYPE_QUICK_CLOSE:
        {
            if (gameObjTarget == nullptr)
                return;

            gameObjTarget->Use(m_caster->GetGUID());
        }
        break;

        case LOCKTYPE_QUICK_OPEN:
            if (gameObjTarget == nullptr)
                return;

            if ((p_caster != nullptr) && (p_caster->m_bg != nullptr))
                p_caster->m_bg->HookQuickLockOpen(gameObjTarget, p_caster, this);

            // there is no break here on purpose

        default://not profession
        {
            if (gameObjTarget == nullptr)
                return;

            gameObjTarget->Use(m_caster->GetGUID());

            CALL_GO_SCRIPT_EVENT(gameObjTarget, OnActivate)(p_caster);
            CALL_INSTANCE_SCRIPT_EVENT(gameObjTarget->GetMapMgr(), OnGameObjectActivate)(gameObjTarget, p_caster);

            if (sQuestMgr.OnActivateQuestGiver(gameObjTarget, p_caster))
                return;

            if (sQuestMgr.OnGameObjectActivate(p_caster, gameObjTarget))
            {
                p_caster->UpdateNearbyGameObjects();
                return;
            }

            if (gameObjTarget->IsLootable())
            {
                GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(gameObjTarget);

                if (pLGO->loot.items.size() == 0)
                {
                    if (gameObjTarget->GetMapMgr() != nullptr)
                        lootmgr.FillGOLoot(&pLGO->loot, gameObjTarget->GetGameObjectProperties()->raw.parameter_1, gameObjTarget->GetMapMgr()->iInstanceMode);
                    else
                        lootmgr.FillGOLoot(&pLGO->loot, gameObjTarget->GetGameObjectProperties()->raw.parameter_1, 0);
                }
            }
            loottype = LOOT_CORPSE;
        }
        break;
    };
    if (gameObjTarget && gameObjTarget->GetType() == GAMEOBJECT_TYPE_CHEST)
        static_cast< Player* >(m_caster)->SendLoot(gameObjTarget->GetGUID(), loottype, gameObjTarget->GetMapId());
}

void Spell::SpellEffectTransformItem(uint8_t effectIndex)
{
    bool result;
    AddItemResult result2;

    if (!i_caster) return;
    uint32 itemid = GetSpellInfo()->getEffectItemType(effectIndex);
    if (!itemid) return;

    //Save durability of the old item
    Player* owner = i_caster->GetOwner();
    uint32 dur = i_caster->GetDurability();

    result = owner->GetItemInterface()->SafeFullRemoveItemByGuid(i_caster->GetGUID());
    if (!result)
    {
        //something went wrong if this happen, item doesn't exist, so it wasn't destroyed.
        return;
    }

    i_caster = nullptr;

    Item* it = objmgr.CreateItem(itemid, owner);
    if (!it) return;

    it->SetDurability(dur);
    //additem

    //additem
    result2 = owner->GetItemInterface()->AddItemToFreeSlot(it);
    if (!result2) //should never get here
    {
        owner->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_BAG_FULL);
        it->DeleteMe();
    }
}

void Spell::SpellEffectApplyGroupAA(uint8_t effectIndex)
{
    ApplyAreaAura(effectIndex);
}

void Spell::SpellEffectLearnSpell(uint8_t effectIndex) // Learn Spell
{
    if (playerTarget == nullptr && unitTarget && unitTarget->IsPet()) // something's wrong with this logic here.
    {
        // bug in target map fill?
        //playerTarget = m_caster->GetMapMgr()->GetPlayer((uint32)m_targets.m_unitTarget);
        SpellEffectLearnPetSpell(effectIndex);
        return;
    }

    if (GetSpellInfo()->getId() == 483 || GetSpellInfo()->getId() == 55884)          // "Learning"
    {
        if (!i_caster || !p_caster) return;

        uint32 spellid = 0;
        for (uint8 j = 0; j < 5; ++j)
        {
            if (i_caster->GetItemProperties()->Spells[j].Trigger == LEARNING && i_caster->GetItemProperties()->Spells[j].Id != 0)
            {
                spellid = i_caster->GetItemProperties()->Spells[j].Id;
                break;
            }
        }

        if (!spellid || !sSpellCustomizations.GetSpellInfo(spellid)) return;

        // learn me!
        p_caster->addSpell(spellid);

        // no normal handler
        return;
    }

    if (playerTarget)
    {
        /*if (u_caster && isHostile(playerTarget, u_caster))
        return;*/

        uint32 spellToLearn = GetSpellInfo()->getEffectTriggerSpell(effectIndex);
        playerTarget->addSpell(spellToLearn);

        if (spellToLearn == 2575)   //hacky fix for mining from creatures
            playerTarget->addSpell(32606);

        if (spellToLearn == 2366)   //hacky fix for herbalism from creatures
            playerTarget->addSpell(32605);

        //smth is wrong here, first we add this spell to player then we may cast it on player...
        SpellInfo* spellinfo = sSpellCustomizations.GetSpellInfo(spellToLearn);
        //remove specializations
        switch (spellinfo->getId())
        {
            case 26801: //Shadoweave Tailoring
                playerTarget->removeSpell(26798, false, false, 0); //Mooncloth Tailoring
                playerTarget->removeSpell(26797, false, false, 0); //Spellfire Tailoring
                break;
            case 26798: // Mooncloth Tailoring
                playerTarget->removeSpell(26801, false, false, 0); //Shadoweave Tailoring
                playerTarget->removeSpell(26797, false, false, 0); //Spellfire Tailoring
                break;
            case 26797: //Spellfire Tailoring
                playerTarget->removeSpell(26801, false, false, 0); //Shadoweave Tailoring
                playerTarget->removeSpell(26798, false, false, 0); //Mooncloth Tailoring
                break;
            case 10656: //Dragonscale Leatherworking
                playerTarget->removeSpell(10658, false, false, 0); //Elemental Leatherworking
                playerTarget->removeSpell(10660, false, false, 0); //Tribal Leatherworking
                break;
            case 10658: //Elemental Leatherworking
                playerTarget->removeSpell(10656, false, false, 0); //Dragonscale Leatherworking
                playerTarget->removeSpell(10660, false, false, 0); //Tribal Leatherworking
                break;
            case 10660: //Tribal Leatherworking
                playerTarget->removeSpell(10656, false, false, 0); //Dragonscale Leatherworking
                playerTarget->removeSpell(10658, false, false, 0); //Elemental Leatherworking
                break;
            case 28677: //Elixir Master
                playerTarget->removeSpell(28675, false, false, 0); //Potion Master
                playerTarget->removeSpell(28672, false, false, 0); //Transmutation Maste
                break;
            case 28675: //Potion Master
                playerTarget->removeSpell(28677, false, false, 0); //Elixir Master
                playerTarget->removeSpell(28672, false, false, 0); //Transmutation Maste
                break;
            case 28672: //Transmutation Master
                playerTarget->removeSpell(28675, false, false, 0); //Potion Master
                playerTarget->removeSpell(28677, false, false, 0); //Elixir Master
                break;
            case 20219: //Gnomish Engineer
                playerTarget->removeSpell(20222, false, false, 0); //Goblin Engineer
                break;
            case 20222: //Goblin Engineer
                playerTarget->removeSpell(20219, false, false, 0); //Gnomish Engineer
                break;
            case 9788: //Armorsmith
                playerTarget->removeSpell(9787, false, false, 0); //Weaponsmith
                playerTarget->removeSpell(17039, false, false, 0); //Master Swordsmith
                playerTarget->removeSpell(17040, false, false, 0); //Master Hammersmith
                playerTarget->removeSpell(17041, false, false, 0); //Master Axesmith
                break;
            case 9787: //Weaponsmith
                playerTarget->removeSpell(9788, false, false, 0); //Armorsmith
                break;
            case 17041: //Master Axesmith
                playerTarget->removeSpell(9788, false, false, 0); //Armorsmith
                playerTarget->removeSpell(17040, false, false, 0); //Master Hammersmith
                playerTarget->removeSpell(17039, false, false, 0); //Master Swordsmith
                break;
            case 17040: //Master Hammersmith
                playerTarget->removeSpell(9788, false, false, 0); //Armorsmith
                playerTarget->removeSpell(17039, false, false, 0); //Master Swordsmith
                playerTarget->removeSpell(17041, false, false, 0); //Master Axesmith
                break;
            case 17039: //Master Swordsmith
                playerTarget->removeSpell(9788, false, false, 0); //Armorsmith
                playerTarget->removeSpell(17040, false, false, 0); //Master Hammersmith
                playerTarget->removeSpell(17041, false, false, 0); //Master Axesmith
                break;
        }
        for (uint8_t j = 0; j < 3; j++)
            if (spellinfo->getEffect(j) == SPELL_EFFECT_WEAPON ||
                spellinfo->getEffect(j) == SPELL_EFFECT_PROFICIENCY ||
                spellinfo->getEffect(j) == SPELL_EFFECT_DUAL_WIELD)
            {
                Spell* sp = sSpellFactoryMgr.NewSpell(unitTarget, spellinfo, true, nullptr);
                SpellCastTargets targets;
                targets.m_unitTarget = unitTarget->GetGUID();
                targets.m_targetMask = TARGET_FLAG_UNIT;
                sp->prepare(&targets);
                break;
            }
        return;
    }

    // if we got here... try via pet spells..
    SpellEffectLearnPetSpell(effectIndex);
}

void Spell::SpellEffectSpellDefense(uint8_t /*effectIndex*/)
{
    //used to enable this ability. We use it all the time ...
}

void Spell::SpellEffectDispel(uint8_t effectIndex) // Dispel
{
    if (u_caster == nullptr || unitTarget == nullptr)
        return;

    uint32 start, end;

    if (isAttackable(u_caster, unitTarget) || GetSpellInfo()->getEffectMiscValue(effectIndex) == DISPEL_STEALTH)    // IsAttackable returns false for stealthed
    {
        start = MAX_POSITIVE_AURAS_EXTEDED_START;
        end = MAX_POSITIVE_AURAS_EXTEDED_END;
        if (unitTarget->SchoolImmunityList[GetSpellInfo()->getSchool()])
            return;
    }
    else
    {
        start = MAX_NEGATIVE_AURAS_EXTEDED_START;
        end = MAX_NEGATIVE_AURAS_EXTEDED_END;
    }

    Aura* aur;
    SpellInfo* aursp;
    std::list< uint32 > dispelledSpells;
    bool finish = false;

    for (uint32 x = start; x < end; x++)
        if (unitTarget->m_auras[x] != nullptr)
        {
            bool AuraRemoved = false;
            aur = unitTarget->m_auras[x];
            aursp = aur->GetSpellInfo();

            //Nothing can dispel resurrection sickness;
            if (!aur->IsPassive() && !(aursp->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY))
            {
                if (GetSpellInfo()->getDispelType() == DISPEL_ALL)
                {
                    unitTarget->HandleProc(PROC_ON_PRE_DISPELL_AURA_VICTIM, u_caster, GetSpellInfo(), m_triggeredSpell, aursp->getId());

                    dispelledSpells.push_back(aursp->getId());

                    unitTarget->RemoveAura(aur);
                    AuraRemoved = true;

                    if (--damage <= 0)
                        finish = true;
                }
                else if (static_cast<int32_t>(aursp->getDispelType()) == GetSpellInfo()->getEffectMiscValue(effectIndex))
                {
                    unitTarget->HandleProc(PROC_ON_PRE_DISPELL_AURA_VICTIM, u_caster, GetSpellInfo(), m_triggeredSpell, aursp->getId());

                    dispelledSpells.push_back(aursp->getId());

                    unitTarget->removeAllAurasByIdForGuid(aursp->getId(), aur->GetCasterGUID());
                    AuraRemoved = true;

                    if (--damage <= 0)
                        finish = true;
                }

                if (AuraRemoved)
                {
                    switch (aursp->getId())
                    {
                        //SPELL_HASH_UNSTABLE_AFFLICTION
                        case 30108:
                        case 30404:
                        case 30405:
                        case 31117:
                        case 34438:
                        case 34439:
                        case 35183:
                        case 43522:
                        case 43523:
                        case 47841:
                        case 47843:
                        case 65812:
                        case 65813:
                        case 68154:
                        case 68155:
                        case 68156:
                        case 68157:
                        case 68158:
                        case 68159:
                        {
                            SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(31117);
                            Spell* spell = sSpellFactoryMgr.NewSpell(u_caster, spellInfo, true, nullptr);
                            spell->forced_basepoints[0] = (aursp->getEffectBasePoints(0) + 1) * 9;   //damage effect
                            spell->ProcedOnSpell = GetSpellInfo();
                            spell->pSpellId = aursp->getId();
                            SpellCastTargets targets;
                            targets.m_unitTarget = u_caster->GetGUID();
                            spell->prepare(&targets);
                        } break;
                    }
                }
            }
            if (finish)
                break;
        }

    // send spell dispell log packet
    if (!dispelledSpells.empty())
    {
        WorldPacket data(SMSG_SPELLDISPELLOG, 25 + dispelledSpells.size() * 5);
        data << m_caster->GetNewGUID();
        data << unitTarget->GetNewGUID();
        data << GetSpellInfo()->getId();
        data << uint8(0);               // unused
        data << uint32(dispelledSpells.size());
        for (std::list< uint32 >::iterator itr = dispelledSpells.begin(); itr != dispelledSpells.end(); ++itr)
        {
            data << uint32(*itr);       // dispelled spell id
            data << uint8(0);           // 0 = dispelled, else cleansed
        }

        m_caster->SendMessageToSet(&data, true);
    }
}

void Spell::SpellEffectLanguage(uint8_t effectIndex)
{
    if (!playerTarget || !GetSpellInfo()->getEffectMiscValue(effectIndex))
        return;

#if VERSION_STRING != Cata
    uint32 skills[15][2] =
#else
    uint32 skills[17][2] =
#endif
    {
        { 0, 0 },
        { SKILL_LANG_ORCISH, 669 },
        { SKILL_LANG_DARNASSIAN, 671 },
        { SKILL_LANG_TAURAHE, 670 },
        { SKILL_LANG_DWARVEN, 672 },
        { SKILL_LANG_COMMON, 668 },
        { SKILL_LANG_DEMON_TONGUE, 815 },
        { SKILL_LANG_TITAN, 816 },
        { SKILL_LANG_THALASSIAN, 813 },
        { SKILL_LANG_DRACONIC, 814 },
        { 0, 0 },
        { SKILL_LANG_GNOMISH, 7340 },
        { SKILL_LANG_TROLL, 7341 },
        { SKILL_LANG_GUTTERSPEAK, 17737 },
        { SKILL_LANG_DRAENEI, 29932 },
#if VERSION_STRING == Cata
        { SKILL_LANG_GOBLIN, 69269 },
        { SKILL_LANG_GILNEAN, 69270 },
#endif
    };

#if VERSION_STRING != Cata
    if (skills[GetSpellInfo()->getEffectMiscValue(effectIndex)][0])
    {
        playerTarget->_AddSkillLine(skills[GetSpellInfo()->getEffectMiscValue(effectIndex)][0], 300, 300);
        playerTarget->addSpell(skills[GetSpellInfo()->getEffectMiscValue(effectIndex)][1]);
    }
#endif
}

void Spell::SpellEffectDualWield(uint8_t /*effectIndex*/)
{
    if (p_caster == nullptr)
        return;

    if (!p_caster->_HasSkillLine(SKILL_DUAL_WIELD))
        p_caster->_AddSkillLine(SKILL_DUAL_WIELD, 1, 1);

    // Increase it by one
    //dual wield is 1/1 , it never increases it's not even displayed in skills tab
}

void Spell::SpellEffectSkillStep(uint8_t effectIndex) // Skill Step
{
    Player* target;
    if (p_caster == nullptr)
    {
        // Check targets
        if (m_targets.m_unitTarget)
        {
            target = objmgr.GetPlayer((uint32)m_targets.m_unitTarget);
            if (!target)
                return;
        }
        else return;
    }
    else
    {
        target = p_caster;
    }

    uint32 skill = GetSpellInfo()->getEffectMiscValue(effectIndex);
    if (skill == 242)
        skill = SKILL_LOCKPICKING; // somehow for lockpicking misc is different than the skill :s

    auto skill_line = sSkillLineStore.LookupEntry(skill);

    if (!skill_line)
        return;

    uint32 max = 1;
    switch (skill_line->type)
    {
        case SKILL_TYPE_PROFESSION:
        case SKILL_TYPE_SECONDARY:
            max = damage * 75;
            break;
        case SKILL_TYPE_WEAPON:
            max = 5 * target->getLevel();
            break;
        case SKILL_TYPE_CLASS:
        case SKILL_TYPE_ARMOR:
            if (skill == SKILL_LOCKPICKING) { max = damage * 75; }
            else { max = 1; }
            break;
        default: //u cant learn other types in game
            return;
    };

    if (target->_HasSkillLine(skill))
    {
        target->_ModifySkillMaximum(skill, max);
    }
    else
    {
        // Don't add skills to players logging in.
        /*if ((GetProto()->Attributes & 64) && playerTarget->m_TeleportState == 1)
        return;*/

        if (skill_line->type == SKILL_TYPE_PROFESSION)
            target->ModPrimaryProfessionPoints(-1);

        if (skill == SKILL_RIDING)
            target->_AddSkillLine(skill, max, max);
        else
            target->_AddSkillLine(skill, 1, max);
    }
}

void Spell::SpellEffectAddHonor(uint8_t effectIndex)
{
    if (!playerTarget) return;

    uint32 val = GetSpellInfo()->getEffectBasePoints(effectIndex);

    if (GetSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_UNK4) val /= 10;

    val += 1;

    HonorHandler::AddHonorPointsToPlayer(playerTarget, val);

    WorldPacket data(SMSG_PVP_CREDIT, 16);
    data << val;
    data << uint64(0);
    data << uint32(5);
    playerTarget->GetSession()->SendPacket(&data);
}

void Spell::SpellEffectSpawn(uint8_t /*effectIndex*/)
{
    // this effect is mostly for custom teleporting
    switch (GetSpellInfo()->getId())
    {
        case 10418: // Arugal spawn-in spell , teleports it to 3 locations randomly sneaking players (bastard ;P)
        {
            //only Creature can cast this spell
            if (u_caster == nullptr || p_caster != nullptr)
                return;

            static float coord[3][3] = { { -108.9034f, 2129.5678f, 144.9210f }, { -108.9034f, 2155.5678f, 155.678f }, { -77.9034f, 2155.5678f, 155.678f } };

            // uint8 j = RandomUInt(3);
            //u_caster->GetAIInterface()->SendMoveToPacket(coord[j][0],coord[j][1],coord[j][2],0.0f,0,u_caster->GetAIInterface()->getMoveFlags());
        }
    }
}

void Spell::SpellEffectSummonObject(uint8_t effectIndex)
{
    if (!u_caster)
        return;

    uint32 entry = GetSpellInfo()->getEffectMiscValue(effectIndex);

    GameObjectProperties const* info = sMySQLStore.getGameObjectProperties(entry);
    if (info == nullptr)
    {
        LogError("Spell %u ( %s ) Effect Index %u tried to summon a GameObject with ID %u. GameObject is not in the database.", m_spellInfo->getId(), m_spellInfo->getName().c_str(), effectIndex, entry);
        return;
    }

    uint32 mapid = u_caster->GetMapId();
    float px = u_caster->GetPositionX();
    float py = u_caster->GetPositionY();
    float pz = u_caster->GetPositionZ();
    float orient = m_caster->GetOrientation();
    float posx = 0, posy = 0, posz = 0;

    GameObject* go = nullptr;

    if (info->type == GAMEOBJECT_TYPE_FISHINGNODE)
    {
        if (p_caster == nullptr)
            return;

        float co = cos(orient);
        float si = sin(orient);
        MapMgr* map = m_caster->GetMapMgr();

        float r;
        for (r = 20; r > 10; r--)
        {
            posx = px + r * co;
            posy = py + r * si;
            uint32 liquidtype;
            map->GetLiquidInfo(posx, posy, pz + 2, posz, liquidtype);
            if (!(liquidtype & 1))//water
                continue;
            if (posz > map->GetLandHeight(posx, posy, pz + 2))  //water
                break;
        }

        posx = px + r * co;
        posy = py + r * si;

        go = u_caster->GetMapMgr()->CreateGameObject(entry);

        go->CreateFromProto(entry, mapid, posx, posy, posz, orient);
        go->SetFlags(GO_FLAG_NONE);
        go->SetState(GO_STATE_OPEN);
        go->setUInt64Value(OBJECT_FIELD_CREATED_BY, m_caster->GetGUID());
        go->SetFaction(u_caster->GetFaction());
        go->Phase(PHASE_SET, u_caster->GetPhase());

        go->PushToWorld(m_caster->GetMapMgr());

        u_caster->SetChannelSpellTargetGUID(go->GetGUID());
    }
    else
    {
        posx = px;
        posy = py;
        auto destination = m_targets.destination();
        if ((m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION) && destination.isSet())
        {
            posx = destination.x;
            posy = destination.y;
            pz = destination.z;
        }

        go = m_caster->GetMapMgr()->CreateGameObject(entry);

        go->CreateFromProto(entry, mapid, posx, posy, pz, orient);
        go->setUInt64Value(OBJECT_FIELD_CREATED_BY, m_caster->GetGUID());
        go->Phase(PHASE_SET, u_caster->GetPhase());
        go->PushToWorld(m_caster->GetMapMgr());
        sEventMgr.AddEvent(go, &GameObject::ExpireAndDelete, EVENT_GAMEOBJECT_EXPIRE, GetDuration(), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        if (info->type == GAMEOBJECT_TYPE_RITUAL)
        {
            if (p_caster == nullptr)
                return;

            GameObject_Ritual* go_ritual = static_cast<GameObject_Ritual*>(go);

            go_ritual->GetRitual()->Setup(p_caster->GetLowGUID(), 0, m_spellInfo->getId());
            go_ritual->GetRitual()->Setup(p_caster->GetLowGUID(), static_cast< uint32 >(p_caster->GetSelection()), m_spellInfo->getId());
        }
    }

    if (p_caster != nullptr)
        p_caster->SetSummonedObject(go);
}

void Spell::SpellEffectEnchantItem(uint8_t effectIndex) // Enchant Item Permanent
{
    if (!itemTarget || !p_caster)
        return;

    // Vellums
    if (GetSpellInfo()->getEffectItemType(effectIndex) && (itemTarget->GetEntry() == 39349 ||
        itemTarget->GetEntry() == 39350 || itemTarget->GetEntry() == 43146 ||
        itemTarget->GetEntry() == 38682 || itemTarget->GetEntry() == 37602 ||
        itemTarget->GetEntry() == 43145))
    {
        uint32 itemid = GetSpellInfo()->getEffectItemType(effectIndex);
        ItemProperties const* it = sMySQLStore.getItemProperties(itemid);
        if (it == nullptr)
        {
            p_caster->GetSession()->SystemMessage("Item is missing, report this to devs. Entry: %u", itemid);
            return;
        }

        Item* pItem = objmgr.CreateItem(itemid, p_caster);
        if (pItem == nullptr)
            return;

        p_caster->GetItemInterface()->RemoveItemAmt(itemTarget->GetEntry(), 1);
        if (!p_caster->GetItemInterface()->AddItemToFreeSlot(pItem))
            pItem->DeleteMe();

        return;
    }

    auto spell_item_enchant = sSpellItemEnchantmentStore.LookupEntry(GetSpellInfo()->getEffectMiscValue(effectIndex));

    if (!spell_item_enchant)
    {
        LOG_ERROR("Invalid enchantment entry %u for Spell %u", GetSpellInfo()->getEffectMiscValue(effectIndex), GetSpellInfo()->getId());
        return;
    }

    if (p_caster->GetSession()->GetPermissionCount() > 0)
        sGMLog.writefromsession(p_caster->GetSession(), "enchanted item for %s", itemTarget->GetOwner()->GetName());

    //remove other perm enchantment that was enchanted by profession
    itemTarget->RemoveProfessionEnchant();
    int32 Slot = itemTarget->AddEnchantment(spell_item_enchant, 0, true, true, false, 0);
    if (Slot < 0)
        return; // Apply failed

    if (!i_caster)
        DetermineSkillUp();
}

void Spell::SpellEffectEnchantItemTemporary(uint8_t effectIndex)  // Enchant Item Temporary
{
    if ((itemTarget == nullptr) || (p_caster == nullptr))
        return;

    uint32 Duration = m_spellInfo->getEffectBasePoints(effectIndex);
    uint32 EnchantmentID = m_spellInfo->getEffectMiscValue(effectIndex);

    // don't allow temporary enchants unless we're the owner of the item
    if (itemTarget->GetOwner() != p_caster)
        return;

    if (Duration == 0)
    {
        LOG_ERROR("Spell %u (%s) has no enchantment duration. Spell needs to be fixed!", m_spellInfo->getId(), m_spellInfo->getName().c_str());
        return;
    }

    if (EnchantmentID == 0)
    {
        LOG_ERROR("Spell %u (%s) has no enchantment ID. Spell needs to be fixed!", m_spellInfo->getId(), m_spellInfo->getName().c_str());
        return;
    }

    auto spell_item_enchant = sSpellItemEnchantmentStore.LookupEntry(EnchantmentID);
    if (spell_item_enchant == nullptr)
    {
        LOG_ERROR("Invalid enchantment entry %u for Spell %u", EnchantmentID, GetSpellInfo()->getId());
        return;
    }

    itemTarget->RemoveEnchantment(TEMP_ENCHANTMENT_SLOT);

    int32 Slot = itemTarget->AddEnchantment(spell_item_enchant, Duration, false, true, false, TEMP_ENCHANTMENT_SLOT);
    if (Slot < 0)
        return; // Apply failed

    auto skill_line_ability = objmgr.GetSpellSkill(GetSpellInfo()->getId());
    if (skill_line_ability != nullptr)
        DetermineSkillUp(skill_line_ability->skilline, itemTarget->GetItemProperties()->ItemLevel);
}

void Spell::SpellEffectTameCreature(uint8_t /*effectIndex*/)
{
    if (unitTarget == nullptr || !unitTarget->IsCreature())
        return;
    Creature* tame = static_cast<Creature*>(unitTarget);

    // Remove target
    tame->GetAIInterface()->HandleEvent(EVENT_LEAVECOMBAT, p_caster, 0);
    Pet* pPet = objmgr.CreatePet(tame->GetEntry());
    if (!pPet->CreateAsSummon(tame->GetEntry(), tame->GetCreatureProperties(), tame, p_caster, nullptr, 2, 0))
    {
        pPet->DeleteMe();//CreateAsSummon() returns false if an error occurred.
        pPet = nullptr;
    }
    tame->Despawn(0, tame->GetCreatureProperties()->RespawnTime);
}

void Spell::SpellEffectSummonPet(uint8_t effectIndex) //summon - pet
{
    if (!p_caster) return;

    if (GetSpellInfo()->getId() == 883)  // "Call Pet" spell
    {
        if (p_caster->GetSummon())
        {
            SendCastResult(SPELL_FAILED_ALREADY_HAVE_SUMMON);
            return;
        }

        uint32 petno = p_caster->GetUnstabledPetNumber();
        if (petno)
        {
            if (p_caster->GetPlayerPet(petno) == nullptr)
            {
                SendCastResult(SPELL_FAILED_ALREADY_HAVE_SUMMON);
                return;
            }

            if (p_caster->GetPlayerPet(petno)->alive)
            {
                p_caster->SpawnPet(petno);
            }
            else
            {
                SendTameFailure(PETTAME_DEAD);
            }
        }
        else
        {
            SendCastResult(SPELL_FAILED_NO_PET);
        }
        return;
    }

    //uint32 entryId = GetProto()->EffectMiscValue[i];

    //VoidWalker:torment, sacrifice, suffering, consume shadows
    //Succubus:lash of pain, soothing kiss, seduce , lesser invisibility
    //felhunter:     Devour Magic,Paranoia,Spell Lock,  Tainted Blood

    // remove old pet
    Pet* old = static_cast< Player* >(m_caster)->GetSummon();
    if (old)
        old->Dismiss();

    CreatureProperties const* ci = sMySQLStore.getCreatureProperties(GetSpellInfo()->getEffectMiscValue(effectIndex));
    if (ci)
    {
        if (p_caster->getClass() == WARLOCK)
        {
            //if demonic sacrifice auras are still active, remove them
            p_caster->RemoveAura(18789);
            p_caster->RemoveAura(18790);
            p_caster->RemoveAura(18791);
            p_caster->RemoveAura(18792);
            p_caster->RemoveAura(35701);
        }

        Pet* summon = objmgr.CreatePet(GetSpellInfo()->getEffectMiscValue(effectIndex));
        if (!summon->CreateAsSummon(GetSpellInfo()->getEffectMiscValue(effectIndex), ci, nullptr, p_caster, GetSpellInfo(), 2, 0))
        {
            summon->DeleteMe();//CreateAsSummon() returns false if an error occurred.
            summon = nullptr;
        }
    }
}

void Spell::SpellEffectLearnPetSpell(uint8_t effectIndex)
{
    /*if (unitTarget && m_caster->GetTypeId() == TYPEID_PLAYER)
    {
    if (unitTarget->IsPet() && unitTarget->GetTypeId() == TYPEID_UNIT)
    {
    TO< Player* >(m_caster)->AddPetSpell(GetProto()->EffectTriggerSpell[i], unitTarget->GetEntry());
    }
    }*/

    if (unitTarget && unitTarget->IsPet() && p_caster)
    {
        Pet* pPet = static_cast< Pet* >(unitTarget);
        if (pPet->IsSummonedPet())
            p_caster->AddSummonSpell(unitTarget->GetEntry(), GetSpellInfo()->getEffectTriggerSpell(effectIndex));

        pPet->AddSpell(sSpellCustomizations.GetSpellInfo(GetSpellInfo()->getEffectTriggerSpell(effectIndex)), true);

        // Send Packet
        /*      WorldPacket data(SMSG_SET_EXTRA_AURA_INFO_OBSOLETE, 22);
        data << pPet->GetGUID() << uint8(0) << uint32(GetProto()->EffectTriggerSpell[i]) << uint32(-1) << uint32(0);
        p_caster->GetSession()->SendPacket(&data);*/
    }
}

void Spell::SpellEffectWeapondamage(uint8_t /*effectIndex*/)   // Weapon damage +
{
    if (!unitTarget || !u_caster)
        return;

    //Hackfix for Mangle
    if (p_caster != nullptr)
    {
        switch (GetSpellInfo()->getId())
        {
            case 33876:
            case 33982:
            case 33983:
            case 48565:
            case 48566:
                p_caster->AddComboPoints(unitTarget->GetGUID(), 1);
                break;
        }
    }

    // Hacky fix for druid spells where it would "double attack".
    if (GetSpellInfo()->getEffect(2) == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE || GetSpellInfo()->getEffect(1) == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE)
    {
        add_damage += damage;
        return;
    }

    uint32 _type;
    if (GetType() == SPELL_DMG_TYPE_RANGED)
        _type = RANGED;
    else
    {
        if (hasAttributeExC(ATTRIBUTESEXC_TYPE_OFFHAND))
            _type = OFFHAND;
        else
            _type = MELEE;
    }
    u_caster->Strike(unitTarget, _type, GetSpellInfo(), damage, 0, 0, false, true);
}

void Spell::SpellEffectOpenLockItem(uint8_t /*effectIndex*/)
{
    if (p_caster == nullptr || i_caster == nullptr)
        return;

    p_caster->HandleSpellLoot(i_caster->GetItemProperties()->ItemId);
}

void Spell::SpellEffectProficiency(uint8_t /*effectIndex*/)
{
    uint32 skill = 0;

    auto skill_line_ability = objmgr.GetSpellSkill(GetSpellInfo()->getId());
    if (skill_line_ability != nullptr)
        skill = skill_line_ability->skilline;

    auto skill_line = sSkillLineStore.LookupEntry(skill);
    if (skill_line)
    {
        if (playerTarget)
        {
            if (playerTarget->_HasSkillLine(skill))
            {
                // Increase it by one
                // playerTarget->AdvanceSkillLine(skill);
            }
            else
            {
                // Don't add skills to players logging in.
                /*if ((GetProto()->Attributes & 64) && playerTarget->m_TeleportState == 1)
                return;*/

                if (skill_line->type == SKILL_TYPE_WEAPON)
                    playerTarget->_AddSkillLine(skill, 1, 5 * playerTarget->getLevel());
                else
                    playerTarget->_AddSkillLine(skill, 1, 1);
            }
        }
    }
}

void Spell::SpellEffectSendEvent(uint8_t effectIndex) //Send Event
{
    //This is mostly used to trigger events on quests or some places

    if (sScriptMgr.CallScriptedDummySpell(m_spellInfo->getId(), effectIndex, this))
        return;

    if (sScriptMgr.HandleScriptedSpellEffect(m_spellInfo->getId(), effectIndex, this))
        return;

    LogDebugFlag(LF_SPELL_EFF, "Spell ID: %u (%s) has a scripted effect index (%u) but no handler for it.", m_spellInfo->getId(), m_spellInfo->getName().c_str(), effectIndex);

}

void Spell::SpellEffectPowerBurn(uint8_t effectIndex) // power burn
{
    if (unitTarget == nullptr || !unitTarget->isAlive() || unitTarget->GetPowerType() != POWER_TYPE_MANA)
        return;

    if (unitTarget->IsPlayer())
    {
        Player* mPlayer = static_cast< Player* >(unitTarget);
        if (mPlayer->IsInFeralForm())
            return;

        // Resilience - reduces the effect of mana drains by (CalcRating*2)%.
        damage = float2int32(damage * (1 - ((static_cast< Player* >(unitTarget)->CalcRating(PCR_SPELL_CRIT_RESILIENCE) * 2) / 100.0f)));
    }
    int32 mult = damage;
    damage = mult * unitTarget->GetMaxPower(POWER_TYPE_MANA) / 100;
    if (m_caster->IsUnit())  //Spell ctor has ASSERT(m_caster != NULL) so there's no need to add NULL checks, even if static analysis reports them.
    {
        Unit* caster = static_cast< Unit* >(m_caster);
        if ((uint32)damage > caster->GetMaxPower(POWER_TYPE_MANA) * (mult * 2) / 100)
            damage = caster->GetMaxPower(POWER_TYPE_MANA) * (mult * 2) / 100;
    }

    int32 mana = (int32)std::min((int32)unitTarget->GetPower(POWER_TYPE_MANA), damage);

    unitTarget->ModPower(POWER_TYPE_MANA, -mana);

    m_caster->SpellNonMeleeDamageLog(unitTarget, GetSpellInfo()->getId(), (uint32)(mana * GetSpellInfo()->getEffectMultipleValue(effectIndex)), pSpellId == 0, true);
}

void Spell::SpellEffectThreat(uint8_t effectIndex) // Threat
{
    if (!unitTarget || !unitTarget->isAlive() || !unitTarget->IsCreature())
        return;

    int32 amount = GetSpellInfo()->getEffectBasePoints(effectIndex);

    spellModFlatIntValue(u_caster->SM_FMiscEffect, &amount, GetSpellInfo()->getSpellGroupType());
    spellModPercentageIntValue(u_caster->SM_PMiscEffect, &amount, GetSpellInfo()->getSpellGroupType());


    bool chck = unitTarget->GetAIInterface()->modThreatByPtr(u_caster, amount);
    if (!chck)
        unitTarget->GetAIInterface()->AttackReaction(u_caster, 1, 0);
}

void Spell::SpellEffectClearQuest(uint8_t effectIndex)
{
    if (playerTarget == nullptr)
    {
        LOG_ERROR("Spell %u (%s) was not casted on Player, but Spell requires Player to be a target.", m_spellInfo->getId(), m_spellInfo->getName().c_str());
        return;
    }

    uint32 questid1 = m_spellInfo->getEffectBasePoints(effectIndex);
    uint32 questid2 = m_spellInfo->getEffectMiscValue(effectIndex);

    playerTarget->ClearQuest(questid1);
    playerTarget->ClearQuest(questid2);
}

void Spell::SpellEffectTriggerSpell(uint8_t effectIndex) // Trigger Spell
{
    SpellInfo* entry = sSpellCustomizations.GetSpellInfo(GetSpellInfo()->getEffectTriggerSpell(effectIndex));
    if (entry == nullptr)
        return;

    SpellCastTargets targets = m_targets;
    Spell* sp = sSpellFactoryMgr.NewSpell(m_caster, entry, m_triggeredSpell, nullptr);
    sp->ProcedOnSpell = GetSpellInfo();
    sp->prepare(&targets);
}

void Spell::SpellEffectApplyRaidAA(uint8_t effectIndex)
{
    ApplyAreaAura(effectIndex);
}

void Spell::SpellEffectPowerFunnel(uint8_t /*effectIndex*/) // Power Funnel
{
    if (!unitTarget || !unitTarget->isAlive() || !unitTarget->IsPet())
        return;

    //does not exist
}

void Spell::SpellEffectHealMaxHealth(uint8_t /*effectIndex*/)   // Heal Max Health
{
    if (!unitTarget || !unitTarget->isAlive())
        return;

    uint32 dif = unitTarget->getUInt32Value(UNIT_FIELD_MAXHEALTH) - unitTarget->getUInt32Value(UNIT_FIELD_HEALTH);
    if (!dif)
    {
        SendCastResult(SPELL_FAILED_ALREADY_AT_FULL_HEALTH);
        return;
    }

    SendHealSpellOnPlayer(m_caster, unitTarget, dif, false, 0, pSpellId ? pSpellId : GetSpellInfo()->getId());

    unitTarget->ModHealth(dif);

    if (u_caster != nullptr)
    {
        switch (this->GetSpellInfo()->getId())
        {
            //SPELL_HASH_LAY_ON_HANDS
            case 633:
            case 2800:
            case 9257:
            case 10310:
            case 17233:
            case 20233:
            case 20236:
            case 27154:
            case 48788:
            case 53778:
                u_caster->CastSpell(unitTarget, 25771, true);
                break;
            default:
                break;
        }
    }
}

void Spell::SpellEffectInterruptCast(uint8_t /*effectIndex*/) // Interrupt Cast
{
    if (!unitTarget || !unitTarget->isAlive())
        return;

    if (GetSpellInfo()->getAttributesExG() & ATTRIBUTESEXG_INTERRUPT_NPC && unitTarget->IsPlayer())
        return;

    if (!playerTarget)
    {
        if (u_caster && (u_caster != unitTarget))
        {
            unitTarget->GetAIInterface()->AttackReaction(u_caster, 1, m_spellInfo->getId());
            Creature* c = static_cast< Creature* >(unitTarget);
            if (c->GetCreatureProperties()->modImmunities & 32768)
                return;
        }
    }

    // Get target's current spell (either channeled or generic spell with cast time)
    if (unitTarget->isCastingNonMeleeSpell(true, false, true))
    {
        Spell* TargetSpell = nullptr;
        if (unitTarget->getCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr && unitTarget->getCurrentSpell(CURRENT_CHANNELED_SPELL)->getCastTimeLeft() > 0)
        {
            TargetSpell = unitTarget->getCurrentSpell(CURRENT_CHANNELED_SPELL);
        }
        // No need to check cast time for generic spells, checked already in Object::isCastingNonMeleeSpell()
        else if (unitTarget->getCurrentSpell(CURRENT_GENERIC_SPELL) != nullptr)
        {
            TargetSpell = unitTarget->getCurrentSpell(CURRENT_GENERIC_SPELL);
        }

        if (TargetSpell != nullptr)
        {
            uint32 school = TargetSpell->GetSpellInfo()->getSchool(); // Get target's casting spell school
            int32 duration = GetDuration(); // Duration of school lockout

            // Check for CastingTime (to prevent interrupting instant casts), PreventionType
            // and InterruptFlags of target's casting spell
            if (school
                && (TargetSpell->getState() == SPELL_STATE_CASTING
                || (TargetSpell->getState() == SPELL_STATE_PREPARING && TargetSpell->GetSpellInfo()->getCastingTimeIndex() > 0))
                && TargetSpell->GetSpellInfo()->getPreventionType() == PREVENTION_TYPE_SILENCE
                && ((TargetSpell->GetSpellInfo()->getInterruptFlags() & CAST_INTERRUPT_ON_INTERRUPT_SCHOOL)
                || (TargetSpell->GetSpellInfo()->getChannelInterruptFlags() & CHANNEL_INTERRUPT_ON_4)))
            {
                if (unitTarget->IsPlayer())
                {
                    // Check for interruption reducing talents
                    int32 DurationModifier = static_cast< Player* >(unitTarget)->MechanicDurationPctMod[MECHANIC_INTERRUPTED];
                    if (DurationModifier >= -100)
                        duration = (duration * (100 + DurationModifier)) / 100;

                    // Prevent player from casting in that school
                    static_cast<Player*>(unitTarget)->SendPreventSchoolCast(school, duration);
                }
                else
                    // Prevent unit from casting in that school
                    unitTarget->SchoolCastPrevent[school] = duration + Util::getMSTime();

                // Interrupt the spell cast
                unitTarget->interruptSpell(TargetSpell->GetSpellInfo()->getId(), false);
            }
        }
    }
}

void Spell::SpellEffectDistract(uint8_t /*effectIndex*/) // Distract
{
    //spellId 1725 Distract:Throws a distraction attracting the all monsters for ten sec's
    if (!unitTarget || !unitTarget->isAlive())
        return;

    if (m_targets.destination().isSet())
    {
        //      unitTarget->GetAIInterface()->MoveTo(m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, 0);
        uint32 Stare_duration = GetDuration();
        if (Stare_duration > 30 * 60 * 1000)
            Stare_duration = 10000;//if we try to stare for more then a half an hour then better not stare at all :P (bug)

        auto destination = m_targets.destination();
        float newo = unitTarget->calcRadAngle(unitTarget->GetPositionX(), unitTarget->GetPositionY(), destination.x, destination.y);
        unitTarget->GetAIInterface()->StopMovement(Stare_duration);
        unitTarget->SetFacing(newo);
    }

    //Smoke Emitter 164870
    //Smoke Emitter Big 179066
    //Unit Field Target of
}

void Spell::SpellEffectPickpocket(uint8_t /*effectIndex*/) // pickpocket
{
    //Show random loot based on roll,
    if (!unitTarget || !p_caster || !unitTarget->IsCreature())
        return;

    Creature* target = static_cast< Creature* >(unitTarget);
    if (target->IsPickPocketed() || (target->GetCreatureProperties()->Type != UNIT_TYPE_HUMANOID))
    {
        SendCastResult(SPELL_FAILED_TARGET_NO_POCKETS);
        return;
    }

    lootmgr.FillPickpocketingLoot(&static_cast< Creature* >(unitTarget)->loot, unitTarget->GetEntry());

    uint32 _rank = static_cast< Creature* >(unitTarget)->GetCreatureProperties()->Rank;
    unitTarget->loot.gold = float2int32((_rank + 1) * unitTarget->getLevel() * (RandomUInt(5) + 1) * worldConfig.getFloatRate(RATE_MONEY));

    p_caster->SendLoot(unitTarget->GetGUID(), LOOT_PICKPOCKETING, unitTarget->GetMapId());
    target->SetPickPocketed(true);
}

void Spell::SpellEffectAddFarsight(uint8_t effectIndex) // Add Farsight
{
    if (p_caster == nullptr)
        return;

    auto lv = m_targets.destination();
    if (!lv.isSet())
    {
        lv = m_targets.source();
    }

    DynamicObject* dynObj = p_caster->GetMapMgr()->CreateDynamicObject();
    dynObj->Create(u_caster, this, lv, GetDuration(), GetRadius(effectIndex), DYNAMIC_OBJECT_FARSIGHT_FOCUS);
    dynObj->SetInstanceID(p_caster->GetInstanceID());
    p_caster->SetFarsightTarget(dynObj->GetGUID());

    p_caster->GetMapMgr()->ChangeFarsightLocation(p_caster, dynObj);
}

void Spell::SpellEffectUseGlyph(uint8_t effectIndex)
{
#if VERSION_STRING > TBC
    if (!p_caster)
        return;

    uint32 glyph_new = m_spellInfo->getEffectMiscValue(effectIndex);
    auto glyph_prop_new = sGlyphPropertiesStore.LookupEntry(glyph_new);
    if (!glyph_prop_new)
        return;

    // check if glyph is locked (obviously)
    if (!(p_caster->getUInt32Value(PLAYER_GLYPHS_ENABLED) & (1 << m_glyphslot)))
    {
        SendCastResult(SPELL_FAILED_GLYPH_SOCKET_LOCKED);
        return;
    }

    uint32 glyph_old = p_caster->GetGlyph(static_cast<uint16_t>(m_glyphslot));
    if (glyph_old)
    {
        if (glyph_old == glyph_new)
        {
            return;
        }
        else
        {
            auto glyph_prop_old = sGlyphPropertiesStore.LookupEntry(glyph_old);
            if (glyph_prop_old)
                p_caster->RemoveAura(glyph_prop_old->SpellID);
        }
    }

    auto glyph_slot = sGlyphSlotStore.LookupEntry(p_caster->getUInt32Value(static_cast<uint16_t>(PLAYER_FIELD_GLYPH_SLOTS_1 + m_glyphslot)));
    if (glyph_slot)
    {
        if (glyph_slot->Type != glyph_prop_new->Type)
        {
            SendCastResult(SPELL_FAILED_INVALID_GLYPH);
            return;
        }
        p_caster->SetGlyph(static_cast<uint8_t>(m_glyphslot), glyph_new);
        p_caster->CastSpell(p_caster, glyph_prop_new->SpellID, true);
        p_caster->m_specs[p_caster->m_talentActiveSpec].glyphs[m_glyphslot] = static_cast<uint16>(glyph_new);
        p_caster->smsg_TalentsInfo(false);
    }
#endif
}

void Spell::SpellEffectHealMechanical(uint8_t /*effectIndex*/)
{
    if (!unitTarget || !unitTarget->IsCreature() || static_cast< Creature* >(unitTarget)->GetCreatureProperties()->Type != UNIT_TYPE_MECHANICAL)
        return;

    Heal(damage);
}

void Spell::SpellEffectSummonObjectWild(uint8_t effectIndex)
{
    if (!u_caster) return;

    // spawn a new one
    GameObject* GoSummon = u_caster->GetMapMgr()->CreateGameObject(GetSpellInfo()->getEffectMiscValue(effectIndex));
    if (!GoSummon->CreateFromProto(GetSpellInfo()->getEffectMiscValue(effectIndex),
        m_caster->GetMapId(), m_caster->GetPositionX() + 1, m_caster->GetPositionY() + 1, m_caster->GetPositionZ(), m_caster->GetOrientation()))
    {
        delete GoSummon;
        return;
    }

    GoSummon->Phase(PHASE_SET, u_caster->GetPhase());
    GoSummon->PushToWorld(u_caster->GetMapMgr());
    GoSummon->SetSummoned(u_caster);

    sEventMgr.AddEvent(GoSummon, &GameObject::ExpireAndDelete, EVENT_GAMEOBJECT_EXPIRE, GetDuration(), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Spell::SpellEffectScriptEffect(uint8_t effectIndex) // Script Effect
{
    // Try a dummy SpellHandler
    if (sScriptMgr.CallScriptedDummySpell(m_spellInfo->getId(), effectIndex, this))
        return;

    if (sScriptMgr.HandleScriptedSpellEffect(m_spellInfo->getId(), effectIndex, this))
        return;

    LOG_ERROR("Spell ID: %u (%s) has a scripted effect index (%hhu) but no handler for it.", m_spellInfo->getId(), m_spellInfo->getName().c_str(), effectIndex);
}

void Spell::SpellEffectSanctuary(uint8_t /*effectIndex*/) // Stop all attacks made to you
{
    if (!u_caster)
        return;

    //warning this causes crashes !
    //  Object::InRangeSet::iterator itr = u_caster->GetInRangeOppFactsSetBegin();
    //  Object::InRangeSet::iterator itr_end = u_caster->GetInRangeOppFactsSetEnd();
    //use these instead
    Object::InRangeSet::iterator itr = u_caster->GetInRangeSetBegin();
    Object::InRangeSet::iterator itr_end = u_caster->GetInRangeSetEnd();

    if (p_caster != nullptr)
        p_caster->RemoveAllAuraType(SPELL_AURA_MOD_ROOT);

    for (; itr != itr_end; ++itr)
    {
        if ((*itr)->IsCreature())
            static_cast<Creature*>(*itr)->GetAIInterface()->RemoveThreatByPtr(unitTarget);
    }
}

void Spell::SpellEffectAddComboPoints(uint8_t /*effectIndex*/) // Add Combo Points
{
    if (!p_caster)
        return;

    //if this is a procspell Ruthlessness (maybe others later)
    if (pSpellId && (GetSpellInfo()->getId() == 14157 || GetSpellInfo()->getId() == 70802 || GetSpellInfo()->getId() == 14181))
    {
        //it seems this combo adding procspell is going to change combopoint count before they will get reset. We add it after the reset
        /* burlex - this is wrong, and exploitable.. :/ if someone uses this they will have unlimited combo points */
        //re-enabled this by Zack. Explained why it is used + rechecked to make sure initialization is good ...
        // while casting a spell talent will trigger upon the spell prepare faze
        // the effect of the talent is to add 1 combo point but when triggering spell finishes it will clear the extra combo point
        p_caster->m_spellcomboPoints += static_cast<int8>(damage);
        return;
    }
    p_caster->AddComboPoints(p_caster->GetSelection(), static_cast<uint8>(damage));
}

void Spell::SpellEffectCreateHouse(uint8_t /*effectIndex*/) // Create House
{
}

void Spell::SpellEffectDuel(uint8_t /*effectIndex*/) // Duel
{
    if (!p_caster || !p_caster->isAlive())
        return;

    if (p_caster->IsStealth())
    {
        SendCastResult(SPELL_FAILED_CANT_DUEL_WHILE_STEALTHED);
        return; // Player is stealth
    }
    if (!playerTarget || playerTarget == p_caster)
        return;

    /* not implemented yet
    \todo dueling zones ? (SPELL_FAILED_NO_DUELING)
    if (player->IsInvisible())
    {
    SendCastResult(SPELL_FAILED_CANT_DUEL_WHILE_INVISIBLE);
    return;
    }
    */

    //Player* pTarget = sObjHolder.GetObject<Player>(player->GetSelection());      //  hacky.. and will screw up if plr is deselected..
    if (!playerTarget)
    {
        SendCastResult(SPELL_FAILED_BAD_TARGETS);
        return; // invalid Target
    }
    if (!playerTarget->isAlive())
    {
        SendCastResult(SPELL_FAILED_TARGETS_DEAD);
        return; // Target not alive
    }
    if (playerTarget->hasUnitStateFlag(UNIT_STATE_ATTACKING))
    {
        SendCastResult(SPELL_FAILED_TARGET_IN_COMBAT);
        return; // Target in combat with another unit
    }
    if (playerTarget->DuelingWith)
    {
        SendCastResult(SPELL_FAILED_TARGET_DUELING);
        return; // Already Dueling
    }

    p_caster->RequestDuel(playerTarget);
}

void Spell::SpellEffectStuck(uint8_t /*effectIndex*/)
{
    if (!playerTarget || playerTarget != p_caster)
        return;

    sEventMgr.AddEvent(playerTarget, &Player::EventTeleport, playerTarget->GetBindMapId(), playerTarget->GetBindPositionX(), playerTarget->GetBindPositionY(),
                       playerTarget->GetBindPositionZ(), EVENT_PLAYER_TELEPORT, 50, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    /*
    playerTarget->SafeTeleport(playerTarget->GetBindMapId(), 0, playerTarget->GetBindPositionX(), playerTarget->GetBindPositionY(), playerTarget->GetBindPositionZ(), 3.14f);*/
}

void Spell::SpellEffectSummonPlayer(uint8_t /*effectIndex*/)
{
    if (!playerTarget)
        return;

    // vojta: from 2.4 players can be summoned on another map
    //if (m_caster->GetMapMgr()->GetMapInfo() && m_caster->GetMapMgr()->GetMapInfo()->type != INSTANCE_NULL && m_caster->GetMapId() != playerTarget->GetMapId())
    //  return;
    if (m_caster->GetMapMgr()->GetMapInfo() && playerTarget->getLevel() < m_caster->GetMapMgr()->GetMapInfo()->minlevel)    // we need some blizzlike message that player needs level xx - feel free to add it ;)
        return;

    playerTarget->SummonRequest(m_caster->GetLowGUID(), m_caster->GetZoneId(), m_caster->GetMapId(), m_caster->GetInstanceID(), m_caster->GetPosition());
}

void Spell::SpellEffectActivateObject(uint8_t effectIndex) // Activate Object
{
    if (!p_caster)
        return;

    if (!gameObjTarget)
    {
        LOG_ERROR("Spell %u (%s) effect %hhu not handled because no target was found. ", m_spellInfo->getId(), m_spellInfo->getName().c_str(), effectIndex);
        return;
    }

    CALL_GO_SCRIPT_EVENT(gameObjTarget, OnActivate)(p_caster);
    gameObjTarget->Activate();

    sEventMgr.AddEvent(gameObjTarget, &GameObject::Deactivate, 0, GetDuration(), 1, 0);
}

void Spell::SpellEffectBuildingDamage(uint8_t effectIndex)
{
    if (gameObjTarget == nullptr)
    {
        LOG_ERROR("Spell %u (%s) effect %hhu not handled because no target was found. ", m_spellInfo->getId(), m_spellInfo->getName().c_str(), effectIndex);
        return;
    }

    if (gameObjTarget->GetGameObjectProperties()->type != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
        return;

    if (u_caster == nullptr)
        return;

    uint32 spellDamage = m_spellInfo->getEffectBasePoints(effectIndex) + 1;
    Unit* controller = nullptr;

    if (u_caster->GetVehicleComponent() != nullptr)
        controller = u_caster->GetMapMgr()->GetUnit(u_caster->GetCharmedByGUID());

    if (controller == nullptr)
        controller = u_caster;

    // Baaaam
    GameObject_Destructible* dgo = static_cast<GameObject_Destructible*>(gameObjTarget);
    dgo->Damage(spellDamage, u_caster->GetGUID(), controller->GetGUID(), m_spellInfo->getId());
}

void Spell::SpellEffectEnchantHeldItem(uint8_t effectIndex)
{
    if (!playerTarget) return;

    Item* item = playerTarget->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    if (!item)
        return;

    uint32 Duration = 0; // Needs to be found in dbc.. I guess?

    switch (GetSpellInfo()->getId())
    {
        //SPELL_HASH_FLAMETONGUE_WEAPON
        case 8024:
        case 8027:
        case 8030:
        case 16339:
        case 16341:
        case 16342:
        case 25489:
        case 58785:
        case 58789:
        case 58790:
        case 65979:
        //SPELL_HASH_WINDFURY_WEAPON
        case 8232:
        case 8235:
        case 10486:
        case 16362:
        case 25505:
        case 32911:
        case 35886:
        case 58801:
        case 58803:
        case 58804:
            Duration = 10;
            break;
        default:
            Duration = 1800;
            break;
    }

    auto spell_item_enchant = sSpellItemEnchantmentStore.LookupEntry(GetSpellInfo()->getEffectMiscValue(effectIndex));

    if (!spell_item_enchant)
    {
        LOG_ERROR("Invalid enchantment entry %u for Spell %u", GetSpellInfo()->getEffectMiscValue(effectIndex), GetSpellInfo()->getId());
        return;
    }

    item->RemoveEnchantment(1);
    item->AddEnchantment(spell_item_enchant, Duration, false, true, false, 1);
}

void Spell::SpellEffectSelfResurrect(uint8_t effectIndex)
{
    if (!p_caster || !unitTarget || playerTarget->isAlive()) return;

    uint32 mana;
    uint32 health;
    uint32 class_ = unitTarget->getClass();

    switch (GetSpellInfo()->getId())
    {
        case 3026:
        case 20758:
        case 20759:
        case 20760:
        case 20761:
        case 27240:
        case 47882:
        {
            health = GetSpellInfo()->getEffectMiscValue(effectIndex);
            mana = -damage;
        }
        break;
        case 21169: //Reincarnation. Resurrect with 20% health and mana
        {
            int32 amt = 20;
            spellModFlatIntValue(unitTarget->SM_FMiscEffect, &amt, GetSpellInfo()->getSpellGroupType());
            spellModPercentageIntValue(unitTarget->SM_PMiscEffect, &amt, GetSpellInfo()->getSpellGroupType());
            health = uint32((unitTarget->GetMaxHealth() * amt) / 100);
            mana = uint32((unitTarget->GetMaxPower(POWER_TYPE_MANA) * amt) / 100);
        }
        break;
        default:
        {
            if (damage < 0) return;
            health = uint32(unitTarget->GetMaxHealth() * damage / 100);
            mana = uint32(unitTarget->GetMaxPower(POWER_TYPE_MANA) * damage / 100);
        }
        break;
    }

    if (class_ == WARRIOR || class_ == ROGUE)
        mana = 0;

    playerTarget->m_resurrectHealth = health;
    playerTarget->m_resurrectMana = mana;

    playerTarget->ResurrectPlayer();
    playerTarget->setMoveRoot(false);

    playerTarget->setUInt32Value(PLAYER_SELF_RES_SPELL, 0);

    if (GetSpellInfo()->getId() == 21169)
        AddCooldown();
}

void Spell::SpellEffectSkinning(uint8_t /*effectIndex*/)
{
    if (!unitTarget || !unitTarget->IsCreature())
        return;

    Creature* cr = static_cast<Creature*>(unitTarget);
    uint32 skill = cr->GetRequiredLootSkill();
    uint32 sk = static_cast<Player*>(m_caster)->_GetSkillLineCurrent(skill);
    uint32 lvl = cr->getLevel();

    if ((sk >= lvl * 5) || ((sk + 100) >= lvl * 10))
    {
        //Fill loot for Skinning
        lootmgr.FillSkinningLoot(&cr->loot, unitTarget->GetEntry());
        static_cast<Player*>(m_caster)->SendLoot(unitTarget->GetGUID(), LOOT_SKINNING, unitTarget->GetMapId());

        //Not skinable again
        cr->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
        cr->Skinned = true;

        if (cr->GetCreatureProperties()->Rank > 0)
            DetermineSkillUp(skill, sk < lvl * 5 ? sk / 5 : lvl, 2);
        else
            DetermineSkillUp(skill, sk < lvl * 5 ? sk / 5 : lvl, 1);
    }
    else
    {
        SendCastResult(SPELL_FAILED_TARGET_UNSKINNABLE);
    }
}

void Spell::SpellEffectCharge(uint8_t /*effectIndex*/)
{
    if (unitTarget == nullptr || !unitTarget->isAlive())
        return;

    u_caster->GetAIInterface()->splineMoveCharge(unitTarget, u_caster->GetBoundingRadius());
}

void Spell::SpellEffectKnockBack(uint8_t effectIndex)
{
    if (unitTarget == nullptr || !unitTarget->isAlive())
        return;

    unitTarget->HandleKnockback(m_caster, GetSpellInfo()->getEffectMiscValue(effectIndex) / 10.0f, damage / 10.0f);
}

void Spell::SpellEffectKnockBack2(uint8_t effectIndex)
{
    if (unitTarget == nullptr || !unitTarget->isAlive())
        return;

    unitTarget->HandleKnockback(m_caster, GetSpellInfo()->getEffectMiscValue(effectIndex) / 10.0f, damage / 10.0f);
}

void Spell::SpellEffectDisenchant(uint8_t /*effectIndex*/)
{
    if (!p_caster)
        return;

    Item* it = p_caster->GetItemInterface()->GetItemByGUID(m_targets.m_itemTarget);
    if (!it)
    {
        SendCastResult(SPELL_FAILED_CANT_BE_DISENCHANTED);
        return;
    }

    //Fill disenchanting loot
    p_caster->SetLootGUID(it->GetGUID());
    if (!it->loot)
    {
        it->loot = new Loot;
        lootmgr.FillItemLoot(it->loot, it->GetEntry());
    }

    LogDebugFlag(LF_SPELL_EFF, "Successfully disenchanted item %d", uint32(it->GetEntry()));
    p_caster->SendLoot(it->GetGUID(), LOOT_DISENCHANTING, p_caster->GetMapId());

    //We can increase Enchanting skill up to 60
    uint32 skill = p_caster->_GetSkillLineCurrent(SKILL_ENCHANTING);
    if (skill && skill < 60)
    {
        if (Rand(100.0f - skill * 0.75f))
        {
            uint32 SkillUp = float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE));
            if (skill + SkillUp > 60)
                SkillUp = 60 - skill;

            p_caster->_AdvanceSkillLine(SKILL_ENCHANTING, SkillUp);
        }
    }
    if (it == i_caster)
        i_caster = nullptr;
}

void Spell::SpellEffectInebriate(uint8_t /*effectIndex*/) // lets get drunk!
{
    if (playerTarget == nullptr)
        return;

    // Drunkee!
    uint16 currentDrunk = playerTarget->GetDrunkValue();
    uint16 drunkMod = static_cast<uint16>(damage)* 256;
    if (currentDrunk + drunkMod > 0xFFFF)
        currentDrunk = 0xFFFF;
    else
        currentDrunk += drunkMod;
    playerTarget->SetDrunkValue(currentDrunk, i_caster ? i_caster->GetEntry() : 0);
}

void Spell::SpellEffectFeedPet(uint8_t effectIndex)  // Feed Pet
{
    // food flags and food level are checked in Spell::CanCast()
    if (!itemTarget || !p_caster)
        return;

    Pet* pPet = p_caster->GetSummon();
    if (!pPet)
        return;

    /** Cast feed pet effect
    - effect is item level and pet level dependent, aura ticks are 35, 17, 8 (*1000) happiness*/
    int8 deltaLvl = static_cast<int8>(pPet->getLevel() - itemTarget->GetItemProperties()->ItemLevel);
    damage /= 1000; //damage of Feed pet spell is 35000
    if (deltaLvl > 10) damage = damage >> 1;//divide by 2
    if (deltaLvl > 20) damage = damage >> 1;
    damage *= 1000;

    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(GetSpellInfo()->getEffectTriggerSpell(effectIndex));
    Spell* sp = sSpellFactoryMgr.NewSpell(p_caster, spellInfo, true, nullptr);
    sp->forced_basepoints[0] = damage;
    SpellCastTargets tgt;
    tgt.m_unitTarget = pPet->GetGUID();
    sp->prepare(&tgt);

    if (itemTarget->GetStackCount() > 1)
    {
        itemTarget->ModStackCount(-1);
        itemTarget->m_isDirty = true;
    }
    else
    {
        p_caster->GetItemInterface()->SafeFullRemoveItemByGuid(itemTarget->GetGUID());
        itemTarget = nullptr;
    }
}

void Spell::SpellEffectDismissPet(uint8_t /*effectIndex*/)
{
    // remove pet.. but don't delete so it can be called later
    if (!p_caster) return;

    Pet* pPet = p_caster->GetSummon();
    if (!pPet) return;
    pPet->Remove(true, true);
}

void Spell::SpellEffectReputation(uint8_t effectIndex)
{
    if (!playerTarget)
        return;

    playerTarget->ModStanding(GetSpellInfo()->getEffectMiscValue(effectIndex), damage);
}

void Spell::SpellEffectSummonObjectSlot(uint8_t effectIndex)
{
    if (!u_caster || !u_caster->IsInWorld())
        return;

    GameObject* GoSummon = nullptr;

    uint32 slot = GetSpellInfo()->getEffect(effectIndex) - SPELL_EFFECT_SUMMON_OBJECT_SLOT1;
    GoSummon = u_caster->m_ObjectSlots[slot] ? u_caster->GetMapMgr()->GetGameObject(u_caster->m_ObjectSlots[slot]) : 0;
    u_caster->m_ObjectSlots[slot] = 0;

    if (GoSummon)
    {
        if (GoSummon->GetInstanceID() != u_caster->GetInstanceID())
            GoSummon->ExpireAndDelete();
        else
        {
            if (GoSummon->IsInWorld())
                GoSummon->RemoveFromWorld(true);
            delete GoSummon;
        }
    }


    // spawn a new one
    GoSummon = u_caster->GetMapMgr()->CreateGameObject(GetSpellInfo()->getEffectMiscValue(effectIndex));

    float dx = 0.0f;
    float dy = 0.0f;
    float dz = 0.0f;

    if (m_targets.m_targetMask & TARGET_FLAG_DEST_LOCATION)
    {
        auto destination = m_targets.destination();
        dx = destination.x;
        dy = destination.y;
        dz = destination.z;
    }
    else
    {
        dx = m_caster->GetPositionX();
        dy = m_caster->GetPositionY();
        dz = m_caster->GetPositionZ();
    }

    if (!GoSummon->CreateFromProto(GetSpellInfo()->getEffectMiscValue(effectIndex), m_caster->GetMapId(), dx, dy, dz, m_caster->GetOrientation()))
    {
        delete GoSummon;
        return;
    }

    GoSummon->SetLevel(u_caster->getLevel());
    GoSummon->setUInt64Value(OBJECT_FIELD_CREATED_BY, m_caster->GetGUID());
    GoSummon->Phase(PHASE_SET, u_caster->GetPhase());

    GoSummon->PushToWorld(m_caster->GetMapMgr());

    sEventMgr.AddEvent(GoSummon, &GameObject::ExpireAndDelete, EVENT_GAMEOBJECT_EXPIRE, GetDuration(), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    GoSummon->SetSummoned(u_caster);
    u_caster->m_ObjectSlots[slot] = GoSummon->GetUIdFromGUID();
}

void Spell::SpellEffectDispelMechanic(uint8_t effectIndex)
{
    if (!unitTarget || !unitTarget->isAlive())
        return;

    unitTarget->RemoveAllAurasByMechanic(GetSpellInfo()->getEffectMiscValue(effectIndex), GetSpellInfo()->getEffectBasePoints(effectIndex), false);
}

void Spell::SpellEffectSummonDeadPet(uint8_t /*effectIndex*/)
{
    //this is pet resurrect
    if (!p_caster)
        return;
    Pet* pPet = p_caster->GetSummon();
    if (pPet)
    {
        spellModFlatIntValue(p_caster->SM_FMiscEffect, &damage, GetSpellInfo()->getSpellGroupType());
        spellModPercentageIntValue(p_caster->SM_PMiscEffect, &damage, GetSpellInfo()->getSpellGroupType());

        pPet->setUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
        pPet->SetHealth((uint32)((pPet->GetMaxHealth() * damage) / 100));
        pPet->setDeathState(ALIVE);
        pPet->GetAIInterface()->HandleEvent(EVENT_FOLLOWOWNER, pPet, 0);
        sEventMgr.RemoveEvents(pPet, EVENT_PET_DELAYED_REMOVE);
        pPet->SendSpellsToOwner();
    }
    else
    {

        p_caster->SpawnPet(p_caster->GetUnstabledPetNumber());
        pPet = p_caster->GetSummon();
        if (pPet == nullptr)//no pets to Revive
            return;

        spellModFlatIntValue(p_caster->SM_FMiscEffect, &damage, GetSpellInfo()->getSpellGroupType());
        spellModPercentageIntValue(p_caster->SM_PMiscEffect, &damage, GetSpellInfo()->getSpellGroupType());

        pPet->SetHealth((uint32)((pPet->GetMaxHealth() * damage) / 100));
    }
}

/* This effect has 2 functions
* 1. It delete's all current totems from the player
* 2. It returns a percentage of the mana back to the player
*
* Bur kick my ass if this is not safe:P
*/

void Spell::SpellEffectDestroyAllTotems(uint8_t effectIndex)
{

    if (p_caster == nullptr || !p_caster->IsInWorld())
        return;

    uint32 RetreivedMana = 0;
    uint32 refundpercent = m_spellInfo->getEffectBasePoints(effectIndex) + 1;

    std::vector< uint32 > spellids;

    p_caster->summonhandler.GetSummonSlotSpellIDs(spellids);

    for (std::vector< uint32 >::iterator itr = spellids.begin(); itr != spellids.end(); ++itr)
    {
        uint32 spellid = *itr;
        SpellInfo* sp = sSpellCustomizations.GetSpellInfo(spellid);

        if (sp != nullptr)
        {
            uint32 cost = 0;

            if (sp->getManaCostPercentage() != 0)
                cost = (p_caster->GetBaseMana() * sp->getManaCostPercentage()) / 100;
            else
                cost = sp->getManaCost();

            RetreivedMana += static_cast<uint32>((cost * refundpercent) / 100.0f);
        }
    }

    p_caster->summonhandler.ExpireSummonsInSlot();
    p_caster->Energize(p_caster, GetSpellInfo()->getId(), RetreivedMana, POWER_TYPE_MANA);
}

void Spell::SpellEffectResurrectNew(uint8_t effectIndex)
{
    //base p =hp,misc mana
    if (!playerTarget)
    {
        if (!corpseTarget)
        {
            // unit resurrection handler
            if (unitTarget)
            {
                if (unitTarget->IsCreature() && unitTarget->IsPet() && unitTarget->IsDead())
                {
                    uint32 hlth = ((uint32)GetSpellInfo()->getEffectBasePoints(effectIndex) > unitTarget->GetMaxHealth()) ? unitTarget->GetMaxHealth() : (uint32)GetSpellInfo()->getEffectBasePoints(effectIndex);
                    uint32 mana = ((uint32)GetSpellInfo()->getEffectBasePoints(effectIndex) > unitTarget->GetMaxPower(POWER_TYPE_MANA)) ? unitTarget->GetMaxPower(POWER_TYPE_MANA) : (uint32)GetSpellInfo()->getEffectBasePoints(effectIndex);

                    if (!unitTarget->IsPet())
                    {
                        sEventMgr.RemoveEvents(unitTarget, EVENT_CREATURE_REMOVE_CORPSE);
                    }
                    else
                    {
                        sEventMgr.RemoveEvents(unitTarget, EVENT_PET_DELAYED_REMOVE);
                        sEventMgr.RemoveEvents(unitTarget, EVENT_CREATURE_REMOVE_CORPSE);
                    }
                    unitTarget->SetHealth(hlth);
                    unitTarget->SetPower(POWER_TYPE_MANA, mana);
                    unitTarget->setUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
                    unitTarget->setDeathState(ALIVE);
                    static_cast< Creature* >(unitTarget)->UnTag();
                    static_cast< Creature* >(unitTarget)->loot.gold = 0;
                    static_cast< Creature* >(unitTarget)->loot.looters.clear();
                    static_cast< Creature* >(unitTarget)->loot.items.clear();
                }
            }

            return;
        }
        playerTarget = objmgr.GetPlayer(GET_LOWGUID_PART(corpseTarget->GetOwner()));
        if (!playerTarget) return;
    }

    if (playerTarget->isAlive() || !playerTarget->IsInWorld())
        return;
    //resurrect
    playerTarget->m_resurrectMapId = p_caster->GetMapId();
    playerTarget->m_resurrectInstanceID = p_caster->GetInstanceID();
    playerTarget->m_resurrectPosition = p_caster->GetPosition();
    playerTarget->m_resurrectHealth = damage;
    playerTarget->m_resurrectMana = GetSpellInfo()->getEffectMiscValue(effectIndex);

    SendResurrectRequest(playerTarget);
}

void Spell::SpellEffectAttackMe(uint8_t /*effectIndex*/)
{
    if (!unitTarget || !unitTarget->isAlive())
        return;

    unitTarget->GetAIInterface()->AttackReaction(u_caster, 1, 0);
}

void Spell::SpellEffectSkinPlayerCorpse(uint8_t /*effectIndex*/)
{
    Corpse* corpse = nullptr;
    if (!playerTarget)
    {
        // means we're "skinning" a corpse
        corpse = objmgr.GetCorpse((uint32)m_targets.m_unitTarget);  // hacky
    }
    else if (playerTarget->getDeathState() == CORPSE)   // repopped while we were casting
    {
        corpse = objmgr.GetCorpse(playerTarget->GetLowGUID());
    }

    if (p_caster == nullptr)
        return;

    if (playerTarget && !corpse)
    {
        if (!playerTarget->m_bg || !playerTarget->IsDead())
            return;

        // Set all the lootable stuff on the player. If he repops before we've looted, we'll set the flags
        // on corpse then :p

        playerTarget->bShouldHaveLootableOnCorpse = false;
        playerTarget->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
        playerTarget->SetFlag(UNIT_DYNAMIC_FLAGS, U_DYN_FLAG_LOOTABLE);

        // Send the loot.
        p_caster->SendLoot(playerTarget->GetGUID(), LOOT_SKINNING, playerTarget->GetMapId());

        // Send a message to the died player, telling him he has to resurrect at the graveyard.
        // Send an empty corpse location too, :P

        playerTarget->GetSession()->OutPacket(SMSG_PLAYER_SKINNED, 1, "\x00");
        playerTarget->GetSession()->OutPacket(MSG_CORPSE_QUERY, 1, "\x00");

        // don't allow him to spawn a corpse
        playerTarget->bCorpseCreateable = false;

        // and.. force him to the graveyard and repop him.
        playerTarget->RepopRequestedPlayer();

    }
    else if (corpse)
    {
        // find the corpses' owner
        Player* owner = objmgr.GetPlayer(GET_LOWGUID_PART(corpse->GetOwner()));
        if (owner)
        {
            if (!owner->m_bg) return;

            owner->GetSession()->OutPacket(SMSG_PLAYER_SKINNED, 1, "\x00");
            owner->GetSession()->OutPacket(MSG_CORPSE_QUERY, 1, "\x00");
        }

        if (corpse->getUInt32Value(CORPSE_FIELD_DYNAMIC_FLAGS) != 1)
            corpse->setUInt32Value(CORPSE_FIELD_DYNAMIC_FLAGS, 1); // sets it so you can loot the plyr

        // remove skinnable flag
        corpse->setUInt32Value(CORPSE_FIELD_FLAGS, 5);

        // remove owner association
        corpse->SetOwner(0);
        corpse->SetCorpseState(CORPSE_STATE_BONES);

        // send loot
        p_caster->SendLoot(corpse->GetGUID(), LOOT_SKINNING, corpse->GetMapId());

        corpse->DeleteFromDB();
        objmgr.CorpseAddEventDespawn(corpse);
    }
}

void Spell::SpellEffectSkill(uint8_t effectIndex)
{
    // Used by professions only
    // Effect should be renamed in RequireSkill

    if (!p_caster || p_caster->_GetSkillLineMax(GetSpellInfo()->getEffectMiscValue(effectIndex)) >= uint32(damage * 75))
        return;

    if (p_caster->_HasSkillLine(GetSpellInfo()->getEffectMiscValue(effectIndex)))
        p_caster->_ModifySkillMaximum(GetSpellInfo()->getEffectMiscValue(effectIndex), uint32(damage * 75));
    else
        p_caster->_AddSkillLine(GetSpellInfo()->getEffectMiscValue(effectIndex), 1, uint32(damage * 75));
}

void Spell::SpellEffectApplyPetAA(uint8_t effectIndex)
{
    ApplyAreaAura(effectIndex);
}

void Spell::SpellEffectDummyMelee(uint8_t /*effectIndex*/)   // Normalized Weapon damage +
{

    if (!unitTarget || !u_caster)
        return;

    switch (GetSpellInfo()->getId())
    {
        //SPELL_HASH_OVERPOWER
        case 7384:
        case 7887:
        case 11584:
        case 11585:
        case 14895:
        case 17198:
        case 24407:
        case 32154:
        case 37321:
        case 37529:
        case 43456:
        case 58516:
        case 65924:
        {
            if (p_caster)   //warrior : overpower - let us clear the event and the combopoint count
            {
                p_caster->NullComboPoints(); //some say that we should only remove 1 point per dodge. Due to cooldown you can't cast it twice anyway..
                sEventMgr.RemoveEvents(p_caster, EVENT_COMBO_POINT_CLEAR_FOR_TARGET);
            }
        } break;
        
        case 20243:
        case 30016:
        case 30017:
        case 30022:
        case 36891:
        case 36894:
        case 38849:
        case 38967:
        case 44452:
        case 47497:
        case 47498:
        case 57795:
        case 60018:
        case 62317:
        case 69902:
        {
            //count the number of sunder armors on target
            uint32 sunder_count = 0;
            SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(7386);
            for (uint32 x = MAX_NEGATIVE_AURAS_EXTEDED_START; x < MAX_NEGATIVE_AURAS_EXTEDED_END; ++x)
            {
                if (unitTarget->m_auras[x])
                {
                    switch (unitTarget->m_auras[x]->GetSpellInfo()->getId())
                    {
                        //SPELL_HASH_SUNDER_ARMOR
                        case 7386:
                        case 7405:
                        case 8380:
                        case 11596:
                        case 11597:
                        case 11971:
                        case 13444:
                        case 15502:
                        case 15572:
                        case 16145:
                        case 21081:
                        case 24317:
                        case 25051:
                        case 25225:
                        case 27991:
                        case 30901:
                        case 47467:
                        case 48893:
                        case 50370:
                        case 53618:
                        case 54188:
                        case 57807:
                        case 58461:
                        case 58567:
                        case 59350:
                        case 59608:
                        case 64978:
                        case 65936:
                        case 71554:
                        {
                            sunder_count++;
                            spellInfo = unitTarget->m_auras[x]->GetSpellInfo();
                        } break;
                    }
                }
            }

            if (spellInfo == nullptr)
                return; //omg how did this happen ?
                        //we should also cast sunder armor effect on target with or without dmg
            Spell* spell = sSpellFactoryMgr.NewSpell(u_caster, spellInfo, true, nullptr);
            spell->ProcedOnSpell = GetSpellInfo();
            spell->pSpellId = GetSpellInfo()->getId();
            SpellCastTargets targets(unitTarget->GetGUID());
            spell->prepare(&targets);
            if (!sunder_count)
                return; //no damage = no joy
            damage = damage * sunder_count;
        } break;
        default:
            break;
    }

    switch (GetSpellInfo()->getId())
    {
        //SPELL_HASH_HEMORRHAGE
        case 16511:
        case 17347:
        case 17348:
        case 26864:
        case 30478:
        case 37331:
        case 45897:
        case 48660:
        case 65954:
        {
            if (p_caster)
                p_caster->AddComboPoints(p_caster->GetSelection(), 1);
        } break;

        // AMBUSH
        case 8676:
            add_damage = 77;
            return;
            break;          // r1
        case 8724:
            add_damage = 110;
            return;
            break;          // r2
        case 8725:
            add_damage = 138;
            return;
            break;          // r3
        case 11267:
            add_damage = 204;
            return;
            break;      // r4
        case 11268:
            add_damage = 253;
            return;
            break;      // r5
        case 11269:
            add_damage = 319;
            return;
            break;      // r6
        case 27441:
            add_damage = 369;
            return;
            break;      // r7
        case 48689:
            add_damage = 509;
            return;
            break;      // r8
        case 48690:
            add_damage = 770;
            return;
            break;      // r9
        case 48691:
            add_damage = 908;
            return;
            break;      // r10

            // BACKSTAB
        case 53:
            add_damage = 15;
            return;
            break;          // r1
        case 2589:
            add_damage = 30;
            return;
            break;          // r2
        case 2590:
            add_damage = 48;
            return;
            break;          // r3
        case 2591:
            add_damage = 69;
            return;
            break;          // r4
        case 8721:
            add_damage = 90;
            return;
            break;          // r5
        case 11279:
            add_damage = 135;
            return;
            break;      // r6
        case 11280:
            add_damage = 165;
            return;
            break;      // r7
        case 11281:
            add_damage = 210;
            return;
            break;      // r8
        case 25300:
            add_damage = 225;
            return;
            break;      // r9
        case 26863:
            add_damage = 255;
            return;
            break;      // r10
        case 48656:
            add_damage = 383;
            return;
            break;      // r11
        case 48657:
            add_damage = 465;
            return;
            break;      // r12
    }

    // rogue ambush etc
    for (uint8_t x = 0; x < 3; x++)
        if (GetSpellInfo()->getEffect(x) == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE)
        {
            add_damage = damage * (GetSpellInfo()->getEffectBasePoints(x) + 1) / 100;
            return;
        }

    //rogue - mutilate ads dmg if target is poisoned
    uint32 pct_dmg_mod = 100;
    if (unitTarget->IsPoisoned())
    {
        switch (GetSpellInfo()->getId())
        {
            //SPELL_HASH_MUTILATE
            case 1329:
            case 5374:
            case 27576:
            case 32319:
            case 32320:
            case 32321:
            case 34411:
            case 34412:
            case 34413:
            case 34414:
            case 34415:
            case 34416:
            case 34417:
            case 34418:
            case 34419:
            case 41103:
            case 48661:
            case 48662:
            case 48663:
            case 48664:
            case 48665:
            case 48666:
            case 60850:
                pct_dmg_mod = 120;
                break;
        }
    }

    uint32 _type;
    if (GetType() == SPELL_DMG_TYPE_RANGED)
        _type = RANGED;
    else
    {
        if (GetSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_TYPE_OFFHAND)
            _type = OFFHAND;
        else
            _type = MELEE;
    }
    u_caster->Strike(unitTarget, _type, GetSpellInfo(), damage, pct_dmg_mod, 0, false, true);
}

void Spell::SpellEffectStartTaxi(uint8_t /*effectIndex*/)
{
    if (!playerTarget || !playerTarget->isAlive() || !u_caster)
        return;

    if (playerTarget->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER))
        return;

    TaxiPath* taxipath = sTaxiMgr.GetTaxiPath(GetSpellInfo()->getEffectMiscValue(0));

    if (!taxipath)
        return;

    TaxiNode* taxinode = sTaxiMgr.GetTaxiNode(taxipath->GetSourceNode());

    if (!taxinode)
        return;

    uint32 modelid = 0;

    if (playerTarget->IsTeamHorde())
    {
        CreatureProperties const* ci = sMySQLStore.getCreatureProperties(taxinode->horde_mount);
        if (!ci)
            return;
        modelid = ci->Male_DisplayID;
        if (!modelid)
            return;
    }
    else
    {
        CreatureProperties const* ci = sMySQLStore.getCreatureProperties(taxinode->alliance_mount);
        if (!ci)
            return;
        modelid = ci->Male_DisplayID;
        if (!modelid)
            return;
    }

    playerTarget->TaxiStart(taxipath, modelid, 0);
}

void Spell::SpellEffectPlayerPull(uint8_t /*effectIndex*/)
{
    if (!unitTarget || !unitTarget->isAlive() || !unitTarget->IsPlayer())
        return;

    Player* p_target = static_cast< Player* >(unitTarget);

    // calculate destination
    float pullD = p_target->CalcDistance(m_caster) - p_target->GetBoundingRadius() - (u_caster ? u_caster->GetBoundingRadius() : 0) - 1.0f;
    float pullO = p_target->calcRadAngle(p_target->GetPositionX(), p_target->GetPositionY(), m_caster->GetPositionX(), m_caster->GetPositionY());
    float pullX = p_target->GetPositionX() + pullD * cosf(pullO);
    float pullY = p_target->GetPositionY() + pullD * sinf(pullO);
    float pullZ = m_caster->GetPositionZ() + 0.3f;
    uint32 time = uint32(pullD * 42.0f);

    p_target->SetOrientation(pullO);

    WorldPacket data(SMSG_MONSTER_MOVE, 60);
    data << p_target->GetNewGUID();
    data << uint8(0);
    data << p_target->GetPositionX();
    data << p_target->GetPositionY();
    data << p_target->GetPositionZ();
    data <<Util::getMSTime();
    data << uint8(4);
    data << pullO;
    data << uint32(0x00001000);
    data << time;
    data << uint32(1);
    data << pullX;
    data << pullY;
    data << pullZ;

    p_target->SendMessageToSet(&data, true);
}

void Spell::SpellEffectReduceThreatPercent(uint8_t /*effectIndex*/)
{
    if (!unitTarget || !unitTarget->IsCreature() || !u_caster || unitTarget->GetAIInterface()->getThreatByPtr(u_caster) == 0)
        return;

    unitTarget->GetAIInterface()->modThreatByPtr(u_caster, (int32)unitTarget->GetAIInterface()->getThreatByPtr(u_caster) * damage / 100);
}

void Spell::SpellEffectSpellSteal(uint8_t /*effectIndex*/)
{
    if (unitTarget == nullptr || u_caster == nullptr || !unitTarget->isAlive())
        return;

    if (playerTarget != nullptr && p_caster != nullptr && p_caster != playerTarget)
    {
        if (playerTarget->IsPvPFlagged())
            p_caster->PvPToggle();
    }

    uint32 start, end;
    if (isAttackable(u_caster, unitTarget))
    {
        start = MAX_POSITIVE_AURAS_EXTEDED_START;
        end = MAX_POSITIVE_AURAS_EXTEDED_END;
    }
    else
        return;

    Aura* aur;
    SpellInfo* aursp;
    std::list< uint32 > stealedSpells;

    for (uint32 x = start; x < end; x++)
    {
        if (unitTarget->m_auras[x] != nullptr)
        {
            aur = unitTarget->m_auras[x];
            aursp = aur->GetSpellInfo();

            if (aursp->getId() != 15007 && !aur->IsPassive()
                //              && aur->IsPositive()    // Zack : We are only checking positive auras. There is no meaning to check again
               ) //Nothing can dispel resurrection sickness
            {
                if (aursp->getDispelType() == DISPEL_MAGIC)
                {
                    stealedSpells.push_back(aursp->getId());

                    uint32 aurdur = (aur->GetDuration() > 120000 ? 120000 : aur->GetDuration());
                    Aura* aura = sSpellFactoryMgr.NewAura(aursp, aurdur, u_caster, u_caster);
                    uint32 aur_removed = unitTarget->removeAllAurasByIdReturnCount(aursp->getId());
                    for (uint8 j = 0; j < 3; j++)
                    {
                        if (aura->GetSpellInfo()->getEffect(j))
                        {
                            aura->AddMod(aura->GetSpellInfo()->getEffectApplyAuraName(j), aura->GetSpellInfo()->getEffectBasePoints(j) + 1, aura->GetSpellInfo()->getEffectMiscValue(j), j);
                        }
                    }
                    if (aura->GetSpellInfo()->getProcChance() > 0)
                    {
                        Aura* aur2;
                        for (uint32 j = 0; j < aur_removed - 1; j++)
                        {
                            aur2 = sSpellFactoryMgr.NewAura(aura->GetSpellInfo(), aurdur, u_caster, u_caster);
                            u_caster->AddAura(aur2);
                        }
                        if (!(aura->GetSpellInfo()->getProcFlags() & PROC_REMOVEONUSE))
                        {
                            SpellCharge charge;
                            charge.count = aur_removed;
                            charge.spellId = aura->GetSpellId();
                            charge.ProcFlag = aura->GetSpellInfo()->getProcFlags();
                            charge.lastproc = 0;
                            charge.procdiff = 0;
                            u_caster->m_chargeSpells.insert(std::make_pair(aura->GetSpellId(), charge));
                        }
                    }
                    u_caster->AddAura(aura);
                    break;
                }
            }
        }
    }

    if (!stealedSpells.empty())
    {
        WorldPacket data(SMSG_SPELLSTEALLOG, 25 + stealedSpells.size() * 5);
        data << m_caster->GetNewGUID();
        data << unitTarget->GetNewGUID();
        data << GetSpellInfo()->getId();
        data << uint8(0);               // unused
        data << uint32(stealedSpells.size());
        for (std::list< uint32 >::iterator itr = stealedSpells.begin(); itr != stealedSpells.end(); ++itr)
        {
            data << uint32(*itr);       // stealed spell id
            data << uint8(1);           // 0 = dispelled, else cleansed
        }

        m_caster->SendMessageToSet(&data, true);
    }
}

void Spell::SpellEffectProspecting(uint8_t /*effectIndex*/)
{
    if (!p_caster) return;

    if (!itemTarget) // this should never happen
    {
        SendCastResult(SPELL_FAILED_CANT_BE_PROSPECTED);
        return;
    }

    //Fill Prospecting loot
    p_caster->SetLootGUID(itemTarget->GetGUID());
    if (!itemTarget->loot)
    {
        itemTarget->loot = new Loot;
        lootmgr.FillItemLoot(itemTarget->loot, itemTarget->GetEntry());
    }

    if (itemTarget->loot->items.size() > 0)
    {
        LogDebugFlag(LF_SPELL_EFF, "Successfully prospected item %d", uint32(itemTarget->GetEntry()));
        p_caster->SendLoot(itemTarget->GetGUID(), LOOT_PROSPECTING, p_caster->GetMapId());
    }
    else // this should never happen either
    {
        LogDebugFlag(LF_SPELL_EFF, "Prospecting failed, item %d has no loot", uint32(itemTarget->GetEntry()));
        SendCastResult(SPELL_FAILED_CANT_BE_PROSPECTED);
    }
}

void Spell::SpellEffectApplyFriendAA(uint8_t effectIndex)
{
    ApplyAreaAura(effectIndex);
}

void Spell::SpellEffectApplyEnemyAA(uint8_t effectIndex)
{
    ApplyAreaAura(effectIndex);
}

void Spell::SpellEffectRedirectThreat(uint8_t /*effectIndex*/)
{
    if (!p_caster || !unitTarget)
        return;

    if ((unitTarget->IsPlayer() && p_caster->GetGroup() != static_cast< Player* >(unitTarget)->GetGroup()) || (unitTarget->IsCreature() && !unitTarget->IsPet()))
        return;

    p_caster->SetMisdirectionTarget(unitTarget->GetGUID());
}

void Spell::SpellEffectPlayMusic(uint8_t effectIndex)
{
    uint32 soundid = m_spellInfo->getEffectMiscValue(effectIndex);

    if (soundid == 0)
    {
        LOG_ERROR("Spell %u (%s) has no sound ID to play. Spell needs fixing!", m_spellInfo->getId(), m_spellInfo->getName().c_str());
        return;
    }

    m_caster->PlaySoundToSet(soundid);
}

void Spell::SpellEffectForgetSpecialization(uint8_t effectIndex)
{
    if (!playerTarget) return;

    uint32 spellid = GetSpellInfo()->getEffectTriggerSpell(effectIndex);
    playerTarget->removeSpell(spellid, false, false, 0);

    LogDebugFlag(LF_SPELL_EFF, "Player %u have forgot spell %u from spell %u (caster: %u)", playerTarget->GetLowGUID(), spellid, GetSpellInfo()->getId(), m_caster->GetLowGUID());
}

void Spell::SpellEffectKillCredit(uint8_t effectIndex)
{
    CreatureProperties const* ci = sMySQLStore.getCreatureProperties(GetSpellInfo()->getEffectMiscValue(effectIndex));
    if (playerTarget != nullptr && ci != nullptr)
        sQuestMgr._OnPlayerKill(playerTarget, GetSpellInfo()->getEffectMiscValue(effectIndex), false);
}

void Spell::SpellEffectRestorePowerPct(uint8_t effectIndex)
{
    if (u_caster == nullptr || unitTarget == nullptr || !unitTarget->isAlive())
        return;

    uint32 power_type = GetSpellInfo()->getEffectMiscValue(effectIndex);
    if (power_type > POWER_TYPE_HAPPINESS)
    {
        LOG_ERROR("Unhandled power type %u in %s, report this line to devs.", power_type, __FUNCTION__);
        return;
    }

    uint32 amount = damage * unitTarget->GetMaxPower(static_cast<uint16_t>(power_type)) / 100;
    u_caster->Energize(unitTarget, GetSpellInfo()->getId(), amount, power_type);
}

void Spell::SpellEffectTriggerSpellWithValue(uint8_t effectIndex)
{
    if (!unitTarget) return;

    SpellInfo* TriggeredSpell = sSpellCustomizations.GetSpellInfo(GetSpellInfo()->getEffectTriggerSpell(effectIndex));
    if (TriggeredSpell == nullptr)
        return;

    Spell* sp = sSpellFactoryMgr.NewSpell(m_caster, TriggeredSpell, true, nullptr);

    for (uint32 x = 0; x < 3; x++)
    {
        if (effectIndex == x)
            sp->forced_basepoints[x] = damage;  //prayer of mending should inherit heal bonus ?
        else
            sp->forced_basepoints[x] = TriggeredSpell->getEffectBasePoints(effectIndex);

    }

    SpellCastTargets tgt(unitTarget->GetGUID());
    sp->prepare(&tgt);
}

void Spell::SpellEffectApplyOwnerAA(uint8_t effectIndex)
{
    ApplyAreaAura(effectIndex);
}

void Spell::SpellEffectCreatePet(uint8_t effectIndex)
{
    if (!playerTarget)
        return;

    if (playerTarget->GetSummon())
        playerTarget->GetSummon()->Remove(true, true);

    CreatureProperties const* ci = sMySQLStore.getCreatureProperties(GetSpellInfo()->getEffectMiscValue(effectIndex));
    if (ci)
    {
        Pet* pPet = objmgr.CreatePet(GetSpellInfo()->getEffectMiscValue(effectIndex));
        if (!pPet->CreateAsSummon(GetSpellInfo()->getEffectMiscValue(effectIndex), ci, nullptr, playerTarget, GetSpellInfo(), 1, 0))
        {
            pPet->DeleteMe();//CreateAsSummon() returns false if an error occurred.
            pPet = nullptr;
        }
    }
}

void Spell::SpellEffectTeachTaxiPath(uint8_t effectIndex)
{
    if (!playerTarget || !GetSpellInfo()->getEffectTriggerSpell(effectIndex))
        return;

    uint8 field = (uint8)((GetSpellInfo()->getEffectTriggerSpell(effectIndex) - 1) / 32);
    uint32 submask = 1 << ((GetSpellInfo()->getEffectTriggerSpell(effectIndex) - 1) % 32);

    // Check for known nodes
    if (!(playerTarget->GetTaximask(field) & submask))
    {
        playerTarget->SetTaximask(field, (submask | playerTarget->GetTaximask(field)));

        playerTarget->GetSession()->OutPacket(SMSG_NEW_TAXI_PATH);

        //Send packet
        WorldPacket update(SMSG_TAXINODE_STATUS, 9);
        update << uint64(0) << uint8(1);
        playerTarget->GetSession()->SendPacket(&update);
    }
}

void Spell::SpellEffectDualWield2H(uint8_t /*effectIndex*/)
{
    if (!playerTarget)
        return;

    playerTarget->DualWield2H = true;
}

void Spell::SpellEffectEnchantItemPrismatic(uint8_t effectIndex)
{
    if (!itemTarget || !p_caster)
        return;

    auto spell_item_enchant = sSpellItemEnchantmentStore.LookupEntry(m_spellInfo->getEffectMiscValue(effectIndex));

    if (!spell_item_enchant)
    {
        LOG_ERROR("Invalid enchantment entry %u for Spell %u", GetSpellInfo()->getEffectMiscValue(effectIndex), GetSpellInfo()->getId());
        return;
    }

    if (p_caster->GetSession()->GetPermissionCount() > 0)
        sGMLog.writefromsession(p_caster->GetSession(), "enchanted item for %s", itemTarget->GetOwner()->GetName());

    //remove other socket enchant
    itemTarget->RemoveEnchantment(6);
    int32 Slot = itemTarget->AddEnchantment(spell_item_enchant, 0, true, true, false, 6);

    if (Slot < 6)
        return; // Apply failed

    itemTarget->m_isDirty = true;

}

void Spell::SpellEffectCreateItem2(uint8_t effectIndex) // Create item
{
    ///\todo This spell effect has also a misc value - meaning is unknown yet
    if (p_caster == nullptr)
        return;

    uint32 new_item_id = GetSpellInfo()->getEffectItemType(effectIndex);

    if (new_item_id != 0)
    {
        // create item
        CreateItem(new_item_id);
    }
    else if (i_caster)
    {
        // provide player with item loot (clams)
        ///\todo Finish this
    }
}

void Spell::SpellEffectMilling(uint8_t /*effectIndex*/)
{
    if (!p_caster) return;

    if (!itemTarget) // this should never happen
    {
        SendCastResult(SPELL_FAILED_CANT_BE_PROSPECTED);
        return;
    }

    //Fill Prospecting loot
    p_caster->SetLootGUID(itemTarget->GetGUID());
    if (!itemTarget->loot)
    {
        itemTarget->loot = new Loot;
        lootmgr.FillItemLoot(itemTarget->loot, itemTarget->GetEntry());
    }

    if (itemTarget->loot->items.size() > 0)
    {
        LogDebugFlag(LF_SPELL_EFF, "Successfully milled item %d", uint32(itemTarget->GetEntry()));
        p_caster->SendLoot(itemTarget->GetGUID(), LOOT_MILLING, p_caster->GetMapId());
    }
    else // this should never happen either
    {
        LogDebugFlag(LF_SPELL_EFF, "Milling failed, item %d has no loot", uint32(itemTarget->GetEntry()));
        SendCastResult(SPELL_FAILED_CANT_BE_PROSPECTED);
    }
}

void Spell::SpellEffectRenamePet(uint8_t /*effectIndex*/)
{
    if (!unitTarget || !unitTarget->IsPet() ||
        !static_cast< Pet* >(unitTarget)->GetPetOwner() || static_cast< Pet* >(unitTarget)->GetPetOwner()->getClass() != HUNTER)
        return;

    unitTarget->setByteValue(UNIT_FIELD_BYTES_2, 2, PET_RENAME_ALLOWED);
}

void Spell::SpellEffectRestoreHealthPct(uint8_t /*effectIndex*/)
{
    if (unitTarget == nullptr || !unitTarget->isAlive())
        return;

    uint32 currentHealth = unitTarget->GetHealth();
    uint32 maxHealth = unitTarget->GetMaxHealth();
    uint32 modHealth = damage * maxHealth / 100;
    uint32 newHealth = modHealth + currentHealth;

    uint32 overheal = 0;
    if (newHealth >= maxHealth)
    {
        unitTarget->SetHealth(maxHealth);
        overheal = newHealth - maxHealth;
    }
    else
        unitTarget->ModHealth(modHealth);

    SendHealSpellOnPlayer(m_caster, unitTarget, modHealth, false, overheal, pSpellId ? pSpellId : GetSpellInfo()->getId());
}

void Spell::SpellEffectLearnSpec(uint8_t /*effectIndex*/)
{
    if (p_caster == nullptr)
        return;

    p_caster->m_talentSpecsCount = 2;
    p_caster->smsg_TalentsInfo(false);
}

void Spell::SpellEffectActivateSpec(uint8_t /*effectIndex*/)
{
    if (p_caster == nullptr)
        return;

    if (p_caster->CombatStatus.IsInCombat())
    {
        SendCastResult(SPELL_FAILED_AFFECTING_COMBAT);
        return;
    }
    else if (p_caster->m_bg)
    {
        uint32 Type = p_caster->m_bg->GetType();
        if (isArena(Type))
        {
            SendCastResult(SPELL_FAILED_AFFECTING_COMBAT); // does the job
            return;
        }
        else
        {
            if (p_caster->m_bg->HasStarted())
            {
                SendCastResult(SPELL_FAILED_AFFECTING_COMBAT); // does the job
            }
        }
    }

    uint8 NewSpec = p_caster->m_talentActiveSpec == SPEC_PRIMARY ? SPEC_SECONDARY : SPEC_PRIMARY; // Check if primary spec is on or not
    p_caster->ActivateSpec(NewSpec);

    WorldPacket data(SMSG_ACTION_BUTTONS, PLAYER_ACTION_BUTTON_SIZE + 1);

    data << uint8(1); // Force the client to reset the actionbar and use new values

    for (uint32 j = 0; j < PLAYER_ACTION_BUTTON_COUNT; ++j)
    {
        data << p_caster->m_specs[NewSpec].mActions[j].Action;
        data << p_caster->m_specs[NewSpec].mActions[j].Type;
        data << p_caster->m_specs[NewSpec].mActions[j].Misc;
    }

    p_caster->GetSession()->SendPacket(&data);
    p_caster->SetPower(p_caster->GetPowerType(), 0);
    p_caster->SendPowerUpdate(false);
}

void Spell::SpellEffectDurabilityDamage(uint8_t effectIndex)
{
    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    int16 slot = int16(GetSpellInfo()->getEffectMiscValue(effectIndex));

    Item* pItem;
    Container* pContainer;
    uint32 j, k;

    // FIXME: some spells effects have value -1/-2
    // Possibly its mean -1 all player equipped items and -2 all items
    if (slot < 0)
    {
        for (k = 0; k < MAX_INVENTORY_SLOT; k++)
        {
            pItem = p_caster->GetItemInterface()->GetInventoryItem(static_cast<uint16>(k));
            if (pItem != nullptr)
            {
                if (pItem->IsContainer())
                {
                    pContainer = static_cast< Container* >(pItem);
                    for (j = 0; j < pContainer->GetItemProperties()->ContainerSlots; ++j)
                    {
                        pItem = pContainer->GetItem(static_cast<uint16>(j));
                        if (pItem != nullptr)
                        {
                            uint32 maxdur = pItem->GetDurabilityMax();
                            uint32 olddur = pItem->GetDurability();
                            uint32 newdur = (olddur)-(damage);

                            if (static_cast<int32>(newdur) < 0)
                                newdur = 0;

                            if (newdur > maxdur)
                                newdur = maxdur;

                            pItem->SetDurability(newdur);
                        }
                    }
                }
                else
                {
                    uint32 maxdur = pItem->GetDurabilityMax();
                    uint32 olddur = pItem->GetDurability();
                    uint32 newdur = (olddur)-(damage);

                    if (static_cast<int32>(newdur) < 0)
                        newdur = 0;

                    if (newdur > maxdur)
                        newdur = maxdur;

                    // Apply / Disapply enchantements from this item
                    pItem->SetDurability(newdur);
                    if (newdur == 0 && olddur > 0)
                        p_caster->ApplyItemMods(pItem, static_cast<uint16>(k), false);
                    else if (newdur > 0 && olddur == 0)
                        p_caster->ApplyItemMods(pItem, static_cast<uint16>(k), true);
                }
            }
        }
        return;
    }

    // invalid slot value
    if (slot >= INVENTORY_SLOT_BAG_END)
        return;

    pItem = p_caster->GetItemInterface()->GetInventoryItem(slot);
    if (pItem)
    {
        uint32 maxdur = pItem->GetDurabilityMax();
        uint32 olddur = pItem->GetDurability();
        uint32 newdur = (olddur)-(damage);

        if (static_cast<int32>(newdur) < 0)
            newdur = 0;

        if (newdur > maxdur)
            newdur = maxdur;

        pItem->SetDurability(newdur);

        // Apply / Disapply enchantements from this item
        if (newdur == 0 && olddur > 0)
            p_caster->ApplyItemMods(pItem, slot, false);
        else if (newdur > 0 && olddur == 0)
            p_caster->ApplyItemMods(pItem, slot, true);
    }
}

void Spell::SpellEffectDurabilityDamagePCT(uint8_t effectIndex)
{
    if (!unitTarget || !unitTarget->IsPlayer())
        return;

    int16 slot = int16(GetSpellInfo()->getEffectMiscValue(effectIndex));

    Item* pItem;
    Container* pContainer;
    uint32 j, k;

    // FIXME: some spells effects have value -1/-2
    // Possibly its mean -1 all player equipped items and -2 all items
    if (slot < 0)
    {
        for (k = 0; k < MAX_INVENTORY_SLOT; ++k)
        {
            pItem = p_caster->GetItemInterface()->GetInventoryItem(static_cast<uint16>(k));
            if (pItem != nullptr)
            {
                if (pItem->IsContainer())
                {
                    pContainer = static_cast< Container* >(pItem);
                    for (j = 0; j < pContainer->GetItemProperties()->ContainerSlots; ++j)
                    {
                        pItem = pContainer->GetItem(static_cast<uint16>(j));
                        if (pItem != nullptr)
                        {
                            uint32 maxdur = pItem->GetDurabilityMax();
                            uint32 olddur = pItem->GetDurability();
                            uint32 newdur = (olddur - (uint32)(maxdur * (damage / 100.0)));

                            if (static_cast<int32>(newdur) < 0)
                                newdur = 0;

                            if (newdur > maxdur)
                                newdur = maxdur;

                            pItem->SetDurability(newdur);
                        }
                    }
                }
                else
                {
                    uint32 maxdur = pItem->GetDurabilityMax();
                    uint32 olddur = pItem->GetDurability();
                    uint32 newdur = (olddur - (uint32)(maxdur * (damage / 100.0)));

                    if (static_cast<int32>(newdur) < 0)
                        newdur = 0;

                    if (newdur > maxdur)
                        newdur = maxdur;

                    // Apply / Disapply enchantements from this item
                    pItem->SetDurability(newdur);
                    if (newdur == 0 && olddur > 0)
                        p_caster->ApplyItemMods(pItem, static_cast<uint16>(k), false);
                    else if (newdur > 0 && olddur == 0)
                        p_caster->ApplyItemMods(pItem, static_cast<uint16>(k), true);
                }
            }
        }
        return;
    }

    // invalid slot value
    if (slot >= INVENTORY_SLOT_BAG_END)
        return;

    if (damage <= 0)
        return;

    pItem = p_caster->GetItemInterface()->GetInventoryItem(slot);
    if (pItem)
    {
        uint32 maxdur = pItem->GetDurabilityMax();
        uint32 olddur = pItem->GetDurability();
        uint32 newdur = (olddur - (uint32)(maxdur * (damage / 100.0)));

        if (static_cast<int32>(newdur) < 0)
            newdur = 0;

        if (newdur > maxdur)
            newdur = maxdur;

        pItem->SetDurability(newdur);

        // Apply / Disapply enchantements from this item
        if (newdur == 0 && olddur > 0)
            p_caster->ApplyItemMods(pItem, slot, false);
        else if (newdur > 0 && olddur == 0)
            p_caster->ApplyItemMods(pItem, slot, true);
    }
}

void Spell::SpellEffectActivateRunes(uint8_t effectIndex)
{
    if (p_caster == nullptr || !p_caster->IsDeathKnight())
        return;

    DeathKnight* dk = static_cast<DeathKnight*>(p_caster);

    uint32 count = damage;
    if (!count)
        count = 1;

    for (uint8 x = 0; x < MAX_RUNES && count; ++x)
    {
        if (dk->GetRuneType(x) == GetSpellInfo()->getEffectMiscValue(effectIndex) && dk->GetRuneIsUsed(x))
        {
            dk->ResetRune(x);
            --count;
        }
    }
}

void Spell::SpellEffectSetMirrorName(uint8_t /*effectIndex*/)
{
    WorldPacket data(SMSG_CLEAR_TARGET, 8);
    data << uint64(m_caster->GetGUID());
    m_caster->SendMessageToSet(&data, true);
}
