/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "Storage/WDB/WDBStores.hpp"
#include "Management/QuestLogEntry.hpp"
#include "MMapFactory.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Creatures/Summons/Summon.hpp"
#include "Objects/DynamicObject.hpp"
#include "Management/HonorHandler.h"
#include "Objects/Item.hpp"
#include "Objects/Container.hpp"
#include "Management/TaxiMgr.hpp"
#include "Management/ItemInterface.h"
#include "Management/ItemProperties.hpp"
#include "Management/Loot/LootMgr.hpp"
#include "Management/Skill.hpp"
#include "Objects/Units/Stats.h"
#include "Management/Battleground/Battleground.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Objects/Units/Players/PlayerClasses.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Map/Management/MapMgr.hpp"
#include "SpellMgr.hpp"
#include "SpellAura.hpp"
#include "Definitions/SpellCastTargetFlags.hpp"
#include "Definitions/SpellDamageType.hpp"
#include "Definitions/CastInterruptFlags.hpp"
#include "Definitions/AuraInterruptFlags.hpp"
#include "Definitions/ChannelInterruptFlags.hpp"
#include "Definitions/PreventionType.hpp"
#include "Definitions/LockTypes.hpp"
#include "Definitions/SpellIsFlags.hpp"
#include "Definitions/TeleportEffectCustomFlags.hpp"
#include "Definitions/SummonControlTypes.hpp"
#include "Definitions/SummonTypes.hpp"
#include "Definitions/SpellState.hpp"
#include "Definitions/DispelType.hpp"
#include "Definitions/SpellMechanics.hpp"
#include "Definitions/PowerType.hpp"
#include "Definitions/Spec.hpp"
#include "Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Definitions/SpellEffects.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Server/Packets/SmsgMoveKnockBack.h"
#include "Server/Packets/SmsgBindPointUpdate.h"
#include "Server/Packets/SmsgClearTarget.h"
#include "Server/Packets/SmsgSpellStealLog.h"
#include "Server/Packets/SmsgSpellDispellLog.h"
#include "Server/Packets/SmsgNewTaxiPath.h"
#include "Server/Packets/SmsgPlayerBound.h"
#include "Server/Packets/MsgCorpseQuery.h"
#include "Server/Packets/SmsgMessageChat.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "VMapFactory.h"
#include "VMapManager2.h"
#include "Logging/Logger.hpp"
#include "Management/ObjectMgr.hpp"
#include "Management/QuestMgr.h"
#include "Movement/MovementManager.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Objects/Units/Creatures/Summons/SummonDefines.hpp"
#include "Objects/Units/Creatures/Summons/SummonHandler.hpp"
#include "Server/Definitions.h"
#include "Server/EventMgr.h"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Server/Script/GameObjectAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Random.hpp"

using namespace AscEmu::Packets;

pSpellEffect SpellEffectsHandler[TOTAL_SPELL_EFFECTS] =
{
    &Spell::spellEffectNotImplemented,          //   0 SPELL_EFFECT_NULL_0
    &Spell::SpellEffectInstantKill,             //   1 SPELL_EFFECT_INSTANT_KILL
    &Spell::SpellEffectSchoolDMG,               //   2 SPELL_EFFECT_SCHOOL_DMG
    &Spell::spellEffectDummy,                   //   3 SPELL_EFFECT_DUMMY
    &Spell::spellEffectNotImplemented,          //   4 SPELL_EFFECT_NULL_4
    &Spell::SpellEffectTeleportUnits,           //   5 SPELL_EFFECT_TELEPORT_UNITS
    &Spell::SpellEffectApplyAura,               //   6 SPELL_EFFECT_APPLY_AURA
    &Spell::SpellEffectEnvironmentalDamage,     //   7 SPELL_EFFECT_ENVIRONMENTAL_DAMAGE
    &Spell::SpellEffectPowerDrain,              //   8 SPELL_EFFECT_POWER_DRAIN
    &Spell::spellEffectHealthLeech,             //   9 SPELL_EFFECT_HEALTH_LEECH
    &Spell::SpellEffectHeal,                    //  10 SPELL_EFFECT_NULL_10
    &Spell::SpellEffectBind,                    //  11 SPELL_EFFECT_NULL_11
    &Spell::spellEffectNotImplemented,          //  12 SPELL_EFFECT_NULL_12
    &Spell::spellEffectNotImplemented,          //  13 SPELL_EFFECT_NULL_13
    &Spell::spellEffectNotImplemented,          //  14 SPELL_EFFECT_NULL_14
    &Spell::spellEffectNotImplemented,          //  15 SPELL_EFFECT_NULL_15
    &Spell::SpellEffectQuestComplete,           //  16 SPELL_EFFECT_QUEST_COMPLETE
    &Spell::SpellEffectWeapondamageNoschool,    //  17 SPELL_EFFECT_WEAPONDAMAGE_NO_SCHOOL
    &Spell::SpellEffectResurrect,               //  18 SPELL_EFFECT_RESURRECT
    &Spell::SpellEffectAddExtraAttacks,         //  19 SPELL_EFFECT_ADD_EXTRA_ATTACKS
    &Spell::SpellEffectDodge,                   //  20 SPELL_EFFECT_DODGE
    &Spell::spellEffectNotImplemented,          //  21 SPELL_EFFECT_NULL_21
    &Spell::SpellEffectParry,                   //  22 SPELL_EFFECT_PARRY
    &Spell::SpellEffectBlock,                   //  23 SPELL_EFFECT_BLOCK
    &Spell::SpellEffectCreateItem,              //  24 SPELL_EFFECT_CREATE_ITEM
    &Spell::spellEffectWeapon,                  //  25 SPELL_EFFECT_WEAPON
    &Spell::spellEffectDefense,                 //  26 SPELL_EFFECT_DEFENSE
    &Spell::SpellEffectPersistentAA,            //  27 SPELL_EFFECT_PERSISTENT_AA
    &Spell::SpellEffectSummon,                  //  28 SPELL_EFFECT_SUMMON
    &Spell::SpellEffectLeap,                    //  29 SPELL_EFFECT_LEAP
    &Spell::SpellEffectEnergize,                //  30 SPELL_EFFECT_ENERGIZE
    &Spell::SpellEffectWeaponDmgPerc,           //  31 SPELL_EFFECT_WEAPON_DMG_PERC
    &Spell::SpellEffectTriggerMissile,          //  32 SPELL_EFFECT_TRIGGER_MISSILE
    &Spell::SpellEffectOpenLock,                //  33 SPELL_EFFECT_OPEN_LOCK
    &Spell::SpellEffectTransformItem,           //  34 SPELL_EFFECT_TRANSFORM_ITEM
    &Spell::SpellEffectApplyGroupAA,            //  35 SPELL_EFFECT_APPLY_GROUP_AA
    &Spell::SpellEffectLearnSpell,              //  36 SPELL_EFFECT_LEARN_SPELL
    &Spell::SpellEffectSpellDefense,            //  37 SPELL_EFFECT_SPELL_DEFENSE
    &Spell::SpellEffectDispel,                  //  38 SPELL_EFFECT_DISPEL
    &Spell::spellEffectNotUsed,                 //  39 SPELL_EFFECT_LANGUAGE
    &Spell::spellEffectDualWield,               //  40 SPELL_EFFECT_DUAL_WIELD
    &Spell::SpellEffectJumpTarget,              //  41 SPELL_EFFECT_JUMP_TARGET
    &Spell::SpellEffectJumpBehindTarget,        //  42 SPELL_EFFECT_JUMP_BEHIND_TARGET
    &Spell::spellEffectNotImplemented,          //  43 SPELL_EFFECT_NULL_43
    &Spell::spellEffectSkillStep,               //  44 SPELL_EFFECT_SKILL_STEP
    &Spell::SpellEffectAddHonor,                //  45 SPELL_EFFECT_ADD_HONOR
    &Spell::SpellEffectSpawn,                   //  46 SPELL_EFFECT_SPAWN
    &Spell::spellEffectNotImplemented,          //  47 SPELL_EFFECT_NULL_47
    &Spell::spellEffectNotImplemented,          //  48 SPELL_EFFECT_NULL_48
    &Spell::spellEffectNotImplemented,          //  49 SPELL_EFFECT_NULL_49
    &Spell::SpellEffectSummonObject,            //  50 SPELL_EFFECT_SUMMON_OBJECT
    &Spell::spellEffectNotImplemented,          //  51 SPELL_EFFECT_NULL_51
    &Spell::spellEffectNotImplemented,          //  52 SPELL_EFFECT_NULL_52
    &Spell::SpellEffectEnchantItem,             //  53 SPELL_EFFECT_ENCHANT_ITEM
    &Spell::SpellEffectEnchantItemTemporary,    //  54 SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY
    &Spell::SpellEffectTameCreature,            //  55 SPELL_EFFECT_TAME_CREATURE
    &Spell::SpellEffectSummonPet,               //  56 SPELL_EFFECT_SUMMON_PET
    &Spell::SpellEffectLearnPetSpell,           //  57 SPELL_EFFECT_LEARN_PET_SPELL
    &Spell::SpellEffectWeapondamage,            //  58 SPELL_EFFECT_WEAPONDAMAGE
    &Spell::SpellEffectOpenLockItem,            //  59 SPELL_EFFECT_OPEN_LOCK_ITEM
    &Spell::spellEffectProficiency,             //  60 SPELL_EFFECT_PROFICIENCY
    &Spell::SpellEffectSendEvent,               //  61 SPELL_EFFECT_SEND_EVENT
    &Spell::SpellEffectPowerBurn,               //  62 SPELL_EFFECT_POWER_BURN
    &Spell::SpellEffectThreat,                  //  63 SPELL_EFFECT_THREAT
    &Spell::spellEffectTriggerSpell,            //  64 SPELL_EFFECT_TRIGGER_SPELL
    &Spell::SpellEffectApplyRaidAA,             //  65 SPELL_EFFECT_APPLY_RAID_AA
    &Spell::SpellEffectPowerFunnel,             //  66 SPELL_EFFECT_POWER_FUNNEL
    &Spell::SpellEffectHealMaxHealth,           //  67 SPELL_EFFECT_HEALMAX_HEALTH
    &Spell::SpellEffectInterruptCast,           //  68 SPELL_EFFECT_INTERRUPT_CAST
    &Spell::SpellEffectDistract,                //  69 SPELL_EFFECT_DISTRACT
    &Spell::SpellEffectPlayerPull,              //  70 SPELL_EFFECT_PLAYER_PULL
    &Spell::SpellEffectPickpocket,              //  71 SPELL_EFFECT_PICKPOCKET
    &Spell::SpellEffectAddFarsight,             //  72 SPELL_EFFECT_ADD_FARSIGHT
    &Spell::spellEffectNotImplemented,          //  73 SPELL_EFFECT_NULL_73
    &Spell::SpellEffectUseGlyph,                //  74 SPELL_EFFECT_USE_GLYPH
    &Spell::SpellEffectHealMechanical,          //  75 SPELL_EFFECT_HEAL_MECHANICAL
    &Spell::SpellEffectSummonObjectWild,        //  76 SPELL_EFFECT_SUMMON_OBJECT_WILD
    &Spell::spellEffectScriptEffect,            //  77 SPELL_EFFECT_SCRIPT_EFFECT
    &Spell::spellEffectNotImplemented,          //  78 SPELL_EFFECT_NULL_78
    &Spell::SpellEffectSanctuary,               //  79 SPELL_EFFECT_SANCTUARY
    &Spell::SpellEffectAddComboPoints,          //  80 SPELL_EFFECT_ADD_COMBO_POINTS
    &Spell::SpellEffectCreateHouse,             //  81 SPELL_EFFECT_CREATE_HOUSE
    &Spell::spellEffectNotImplemented,          //  82 SPELL_EFFECT_NULL_82
    &Spell::SpellEffectDuel,                    //  83 SPELL_EFFECT_DUEL
    &Spell::SpellEffectStuck,                   //  84 SPELL_EFFECT_STUCK
    &Spell::SpellEffectSummonPlayer,            //  85 SPELL_EFFECT_SUMMON_PLAYER
    &Spell::SpellEffectActivateObject,          //  86 SPELL_EFFECT_ACTIVATE_OBJECT
    &Spell::SpellEffectBuildingDamage,          //  87 SPELL_EFFECT_BUILDING_DAMAGE
    &Spell::spellEffectNotImplemented,          //  88 SPELL_EFFECT_NULL_88
    &Spell::spellEffectNotImplemented,          //  89 SPELL_EFFECT_NULL_89
    &Spell::spellEffectNotImplemented,          //  90 SPELL_EFFECT_NULL_90
    &Spell::spellEffectNotImplemented,          //  91 SPELL_EFFECT_NULL_91
    &Spell::SpellEffectEnchantHeldItem,         //  92 SPELL_EFFECT_ENCHANT_HELD_ITEM
    &Spell::SpellEffectForceDeselect,           //  93 SPELL_EFFECT_SET_MIRROR_NAME
    &Spell::SpellEffectSelfResurrect,           //  94 SPELL_EFFECT_SELF_RESURRECT
    &Spell::SpellEffectSkinning,                //  95 SPELL_EFFECT_SKINNING
    &Spell::SpellEffectCharge,                  //  96 SPELL_EFFECT_CHARGE
    &Spell::spellEffectNotImplemented,          //  97 SPELL_EFFECT_NULL_97
    &Spell::SpellEffectKnockBack,               //  98 SPELL_EFFECT_KNOCK_BACK
    &Spell::SpellEffectDisenchant,              //  99 SPELL_EFFECT_DISENCHANT
    &Spell::SpellEffectInebriate,               // 100 SPELL_EFFECT_INEBRIATE
    &Spell::SpellEffectFeedPet,                 // 101 SPELL_EFFECT_FEED_PET
    &Spell::SpellEffectDismissPet,              // 102 SPELL_EFFECT_DISMISS_PET
    &Spell::SpellEffectReputation,              // 103 SPELL_EFFECT_REPUTATION
    &Spell::SpellEffectSummonObjectSlot,        // 104 SPELL_EFFECT_SUMMON_OBJECT_SLOT #1
    &Spell::SpellEffectSummonObjectSlot,        // 105 SPELL_EFFECT_SUMMON_OBJECT_SLOT #2
    &Spell::SpellEffectSummonObjectSlot,        // 106 SPELL_EFFECT_SUMMON_OBJECT_SLOT #3
    &Spell::SpellEffectSummonObjectSlot,        // 107 SPELL_EFFECT_SUMMON_OBJECT_SLOT #4
    &Spell::SpellEffectDispelMechanic,          // 108 SPELL_EFFECT_DISPEL_MECHANIC
    &Spell::SpellEffectSummonDeadPet,           // 109 SPELL_EFFECT_SUMMON_DEAD_PET
    &Spell::SpellEffectDestroyAllTotems,        // 110 SPELL_EFFECT_DESTROY_ALL_TOTEMS
    &Spell::SpellEffectDurabilityDamage,        // 111 SPELL_EFFECT_DURABILITY_DAMAGE
    &Spell::spellEffectNotImplemented,          // 112 SPELL_EFFECT_NULL_112
    &Spell::SpellEffectResurrectNew,            // 113 SPELL_EFFECT_RESURRECT_NEW
    &Spell::SpellEffectAttackMe,                // 114 SPELL_EFFECT_ATTACK_ME
    &Spell::SpellEffectDurabilityDamagePCT,     // 115 SPELL_EFFECT_DURABILITY_DAMAGE_PCT
    &Spell::SpellEffectSkinPlayerCorpse,        // 116 SPELL_EFFECT_SKIN_PLAYER_CORPSE
    &Spell::spellEffectNotImplemented,          // 117 SPELL_EFFECT_NULL_117
    &Spell::spellEffectSkill,                   // 118 SPELL_EFFECT_SKILL
    &Spell::SpellEffectApplyPetAA,              // 119 SPELL_EFFECT_APPLY_PET_AA
    &Spell::spellEffectNotImplemented,          // 120 SPELL_EFFECT_NULL_120
    &Spell::SpellEffectDummyMelee,              // 121 SPELL_EFFECT_DUMMY_MELEE
    &Spell::spellEffectNotImplemented,          // 122 SPELL_EFFECT_NULL_122
    &Spell::SpellEffectStartTaxi,               // 123 SPELL_EFFECT_START_TAXI
    &Spell::SpellEffectPlayerPull,              // 124 SPELL_EFFECT_PLAYER_PULL
    &Spell::SpellEffectReduceThreatPercent,     // 125 SPELL_EFFECT_REDUCE_THREAT_PERCENT
    &Spell::SpellEffectSpellSteal,              // 126 SPELL_EFFECT_SPELL_STEAL
    &Spell::SpellEffectProspecting,             // 127 SPELL_EFFECT_PROSPECTING
    &Spell::SpellEffectApplyFriendAA,           // 128 SPELL_EFFECT_APPLY_FRIEND_AA
    &Spell::SpellEffectApplyEnemyAA,            // 129 SPELL_EFFECT_APPLY_ENEMY_AA
#if VERSION_STRING >= TBC
    &Spell::SpellEffectRedirectThreat,          // 130 SPELL_EFFECT_REDIRECT_THREAT
    &Spell::spellEffectNotImplemented,          // 131 SPELL_EFFECT_NULL_131
    &Spell::SpellEffectPlayMusic,               // 132 SPELL_EFFECT_PLAY_MUSIC
    &Spell::SpellEffectForgetSpecialization,    // 133 SPELL_EFFECT_FORGET_SPECIALIZATION
    &Spell::SpellEffectKillCredit,              // 134 SPELL_EFFECT_KILL_CREDIT
    &Spell::spellEffectNotImplemented,          // 135 SPELL_EFFECT_NULL_135
    &Spell::SpellEffectRestoreHealthPct,        // 136 SPELL_EFFECT_RESTORE_HEALTH_PCT
    &Spell::SpellEffectRestorePowerPct,         // 137 SPELL_EFFECT_RESTORE_POWER_PCT
    &Spell::SpellEffectKnockBack2,              // 138 SPELL_EFFECT_KNOCK_BACK2
    &Spell::SpellEffectClearQuest,              // 139 SPELL_EFFECT_CLEAR_QUEST
    &Spell::spellEffectForceCast,               // 140 SPELL_EFFECT_TRIGGER_SPELL
    &Spell::spellEffectNotImplemented,          // 141 SPELL_EFFECT_NULL_141
    &Spell::SpellEffectTriggerSpellWithValue,   // 142 SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE
    &Spell::SpellEffectApplyOwnerAA,            // 143 SPELL_EFFECT_APPLY_OWNER_AA
    &Spell::SpellEffectKnockBack,               // 144 SPELL_EFFECT_KNOCK_BACK
    &Spell::SpellEffectPullTowardsDest,         // 145 SPELL_EFFECT_PULL_TOWARDS_DEST
    &Spell::SpellEffectActivateRunes,           // 146 SPELL_EFFECT_ACTIVATE_RUNES
    &Spell::spellEffectNotImplemented,          // 147 SPELL_EFFECT_NULL_147
    &Spell::spellEffectNotImplemented,          // 148 SPELL_EFFECT_NULL_148
    &Spell::spellEffectNotImplemented,          // 149 SPELL_EFFECT_NULL_149
    &Spell::spellEffectNotImplemented,          // 150 SPELL_EFFECT_NULL_150
    &Spell::spellEffectTriggerSpell,            // 151 SPELL_EFFECT_TRIGGER_SPELL
    &Spell::spellEffectNotImplemented,          // 152 SPELL_EFFECT_NULL_152
    &Spell::SpellEffectCreatePet,               // 153 SPELL_EFFECT_CREATE_PET
#endif
#if VERSION_STRING >= WotLK
    &Spell::SpellEffectTeachTaxiPath,           // 154 SPELL_EFFECT_TEACH_TAXI_PATH
    &Spell::SpellEffectDualWield2H,             // 155 SPELL_EFFECT_DUAL_WIELD_2H
    &Spell::SpellEffectEnchantItemPrismatic,    // 156 SPELL_EFFECT_ENCHANT_ITEM_PRISMATIC
    &Spell::SpellEffectCreateItem2,             // 157 SPELL_EFFECT_CREATE_ITEM2
    &Spell::SpellEffectMilling,                 // 158 SPELL_EFFECT_MILLING
    &Spell::SpellEffectRenamePet,               // 159 SPELL_EFFECT_RENAME_PET
    &Spell::spellEffectNotImplemented,          // 160 SPELL_EFFECT_NULL_160
    &Spell::SpellEffectLearnSpec,               // 161 SPELL_EFFECT_LEARN_SPEC
    &Spell::SpellEffectActivateSpec,            // 162 SPELL_EFFECT_ACTIVATE_SPEC
    &Spell::spellEffectNotImplemented,          // 163 SPELL_EFFECT_NULL_163
    &Spell::spellEffectNotImplemented,          // 164 SPELL_EFFECT_NULL_164
#endif
#if VERSION_STRING >= Cata
    &Spell::spellEffectNotImplemented,          // 165 SPELL_EFFECT_NULL_165
    &Spell::spellEffectNotImplemented,          // 166 SPELL_EFFECT_NULL_166
    &Spell::spellEffectNotImplemented,          // 167 SPELL_EFFECT_NULL_167
    &Spell::spellEffectNotImplemented,          // 168 SPELL_EFFECT_NULL_168
    &Spell::spellEffectNotImplemented,          // 169 SPELL_EFFECT_NULL_169
    &Spell::spellEffectNotImplemented,          // 170 SPELL_EFFECT_NULL_170
    &Spell::spellEffectNotImplemented,          // 171 SPELL_EFFECT_NULL_171
    &Spell::spellEffectNotImplemented,          // 172 SPELL_EFFECT_NULL_172
    &Spell::spellEffectNotImplemented,          // 173 SPELL_EFFECT_NULL_173
    &Spell::spellEffectNotImplemented,          // 174 SPELL_EFFECT_NULL_174
    &Spell::spellEffectNotImplemented,          // 175 SPELL_EFFECT_NULL_175
    &Spell::spellEffectNotImplemented,          // 176 SPELL_EFFECT_NULL_176
    &Spell::spellEffectNotImplemented,          // 177 SPELL_EFFECT_NULL_177
    &Spell::spellEffectNotImplemented,          // 178 SPELL_EFFECT_NULL_178
    &Spell::spellEffectNotImplemented,          // 179 SPELL_EFFECT_NULL_179
    &Spell::spellEffectNotImplemented,          // 180 SPELL_EFFECT_NULL_180
    &Spell::spellEffectNotImplemented,          // 181 SPELL_EFFECT_NULL_181
    &Spell::spellEffectNotImplemented           // 182 SPELL_EFFECT_NULL_182
#endif
#if VERSION_STRING >= Mop
    // TODO: mop
#endif
};

const char* SpellEffectNames[TOTAL_SPELL_EFFECTS] =
{
    "SPELL_EFFECT_NULL_0",
    "SPELL_EFFECT_INSTANT_KILL",                //    1
    "SPELL_EFFECT_SCHOOL_DMG",                  //    2
    "SPELL_EFFECT_DUMMY",                       //    3
    "SPELL_EFFECT_NULL_4",                      //    4
    "SPELL_EFFECT_TELEPORT_UNITS",              //    5
    "SPELL_EFFECT_APPLY_AURA",                  //    6
    "SPELL_EFFECT_ENVIRONMENTAL_DAMAGE",        //    7
    "SPELL_EFFECT_POWER_DRAIN",                 //    8
    "SPELL_EFFECT_HEALTH_LEECH",                //    9
    "SPELL_EFFECT_NULL_10",                     //    10
    "SPELL_EFFECT_NULL_11",                     //    11
    "SPELL_EFFECT_NULL_12",                     //    12
    "SPELL_EFFECT_NULL_13",                     //    13
    "SPELL_EFFECT_NULL_14",                     //    14
    "SPELL_EFFECT_NULL_15",                     //    15
    "SPELL_EFFECT_QUEST_COMPLETE",              //    16
    "SPELL_EFFECT_WEAPONDAMAGE_NO_SCHOOL",      //    17
    "SPELL_EFFECT_RESURRECT",                   //    18
    "SPELL_EFFECT_ADD_EXTRA_ATTACKS",           //    19
    "SPELL_EFFECT_DODGE",                       //    20
    "SPELL_EFFECT_NULL_21",                     //    21
    "SPELL_EFFECT_PARRY",                       //    22
    "SPELL_EFFECT_BLOCK",                       //    23
    "SPELL_EFFECT_CREATE_ITEM",                 //    24
    "SPELL_EFFECT_WEAPON",                      //    25
    "SPELL_EFFECT_DEFENSE",                     //    26
    "SPELL_EFFECT_PERSISTENT_AA",               //    27
    "SPELL_EFFECT_SUMMON",                      //    28
    "SPELL_EFFECT_LEAP",                        //    29
    "SPELL_EFFECT_ENERGIZE",                    //    30
    "SPELL_EFFECT_WEAPON_DMG_PERC",             //    31
    "SPELL_EFFECT_TRIGGER_MISSILE",             //    32
    "SPELL_EFFECT_OPEN_LOCK",                   //    33
    "SPELL_EFFECT_TRANSFORM_ITEM",              //    34
    "SPELL_EFFECT_APPLY_GROUP_AA",              //    35
    "SPELL_EFFECT_LEARN_SPELL",                 //    36
    "SPELL_EFFECT_SPELL_DEFENSE",               //    37
    "SPELL_EFFECT_DISPEL",                      //    38
    "SPELL_EFFECT_LANGUAGE",                    //    39
    "SPELL_EFFECT_DUAL_WIELD",                  //    40
    "SPELL_EFFECT_JUMP_TARGET",                 //    41
    "SPELL_EFFECT_JUMP_BEHIND_TARGET",          //    42
    "SPELL_EFFECT_NULL_43",                     //    43
    "SPELL_EFFECT_SKILL_STEP",                  //    44
    "SPELL_EFFECT_ADD_HONOR",                   //    45
    "SPELL_EFFECT_SPAWN",                       //    46
    "SPELL_EFFECT_NULL_47",                     //    47
    "SPELL_EFFECT_NULL_48",                     //    48
    "SPELL_EFFECT_NULL_49",                     //    49
    "SPELL_EFFECT_SUMMON_OBJECT",               //    50
    "SPELL_EFFECT_NULL_51",                     //    51
    "SPELL_EFFECT_NULL_52",                     //    52
    "SPELL_EFFECT_ENCHANT_ITEM",                //    53
    "SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY",      //    54
    "SPELL_EFFECT_TAME_CREATURE",               //    55
    "SPELL_EFFECT_SUMMON_PET",                  //    56
    "SPELL_EFFECT_LEARN_PET_SPELL",             //    57
    "SPELL_EFFECT_WEAPONDAMAGE",                //    58
    "SPELL_EFFECT_OPEN_LOCK_ITEM",              //    59
    "SPELL_EFFECT_PROFICIENCY",                 //    60
    "SPELL_EFFECT_SEND_EVENT",                  //    61
    "SPELL_EFFECT_POWER_BURN",                  //    62
    "SPELL_EFFECT_THREAT",                      //    63
    "SPELL_EFFECT_TRIGGER_SPELL",               //    64
    "SPELL_EFFECT_APPLY_RAID_AA",               //    65
    "SPELL_EFFECT_POWER_FUNNEL",                //    66
    "SPELL_EFFECT_HEALMAX_HEALTH",              //    67
    "SPELL_EFFECT_INTERRUPT_CAST",              //    68
    "SPELL_EFFECT_DISTRACT",                    //    69
    "SPELL_EFFECT_PLAYER_PULL",                 //    70
    "SPELL_EFFECT_PICKPOCKET",                  //    71
    "SPELL_EFFECT_ADD_FARSIGHT",                //    72
    "SPELL_EFFECT_NULL_73",                     //    73
    "SPELL_EFFECT_USE_GLYPH",                   //    74
    "SPELL_EFFECT_HEAL_MECHANICAL",             //    75
    "SPELL_EFFECT_SUMMON_OBJECT_WILD",          //    76
    "SPELL_EFFECT_SCRIPT_EFFECT",               //    77
    "SPELL_EFFECT_NULL_78",                     //    78
    "SPELL_EFFECT_SANCTUARY",                   //    79
    "SPELL_EFFECT_ADD_COMBO_POINTS",            //    80
    "SPELL_EFFECT_CREATE_HOUSE",                //    81
    "SPELL_EFFECT_NULL_82",                     //    82
    "SPELL_EFFECT_DUEL",                        //    83
    "SPELL_EFFECT_STUCK",                       //    84
    "SPELL_EFFECT_SUMMON_PLAYER",               //    85
    "SPELL_EFFECT_ACTIVATE_OBJECT",             //    86
    "SPELL_EFFECT_BUILDING_DAMAGE",             //    87
    "SPELL_EFFECT_NULL_88",                     //    88
    "SPELL_EFFECT_NULL_89",                     //    89
    "SPELL_EFFECT_NULL_90",                     //    90
    "SPELL_EFFECT_NULL_91",                     //    91
    "SPELL_EFFECT_ENCHANT_HELD_ITEM",           //    92
    "SPELL_EFFECT_FORCE_DESELECT",              //    93
    "SPELL_EFFECT_SELF_RESURRECT",              //    94
    "SPELL_EFFECT_SKINNING",                    //    95
    "SPELL_EFFECT_CHARGE",                      //    96
    "SPELL_EFFECT_NULL_97",                     //    97
    "SPELL_EFFECT_KNOCK_BACK",                  //    98
    "SPELL_EFFECT_DISENCHANT",                  //    99
    "SPELL_EFFECT_INEBRIATE",                   //    100
    "SPELL_EFFECT_FEED_PET",                    //    101
    "SPELL_EFFECT_DISMISS_PET",                 //    102
    "SPELL_EFFECT_REPUTATION",                  //    103
    "SPELL_EFFECT_SUMMON_OBJECT_SLOT1",         //    104
    "SPELL_EFFECT_SUMMON_OBJECT_SLOT2",         //    105
    "SPELL_EFFECT_SUMMON_OBJECT_SLOT3",         //    106
    "SPELL_EFFECT_SUMMON_OBJECT_SLOT4",         //    107
    "SPELL_EFFECT_DISPEL_MECHANIC",             //    108
    "SPELL_EFFECT_SUMMON_DEAD_PET",             //    109
    "SPELL_EFFECT_DESTROY_ALL_TOTEMS",          //    110
    "SPELL_EFFECT_DURABILITY_DAMAGE",           //    111
    "SPELL_EFFECT_NULL_112",                    //    112 Not in 3.3.5a client
    "SPELL_EFFECT_RESURRECT_NEW",               //    113
    "SPELL_EFFECT_ATTACK_ME",                   //    114
    "SPELL_EFFECT_DURABILITY_DAMAGE_PCT",       //    115
    "SPELL_EFFECT_SKIN_PLAYER_CORPSE",          //    116
    "SPELL_EFFECT_NULL_117",                    //    117 Not used
    "SPELL_EFFECT_SKILL",                       //    118
    "SPELL_EFFECT_APPLY_PET_AA",                //    119
    "SPELL_EFFECT_NULL_120",                    //    120 Not used (SPELL_EFFECT_TELEPORT_GRAVEYARD)
    "SPELL_EFFECT_DUMMY_MELEE",                 //    121
    "SPELL_EFFECT_NULL_122",                    //    122 unknown // not used
    "SPELL_EFFECT_START_TAXI",                  //    123 http://www.wowhead.com/?spell=54575
    "SPELL_EFFECT_PLAYER_PULL",                 //    124
    "SPELL_EFFECT_REDUCE_THREAT_PERCENT",       //    125 Reduce Threat by % // https://www.wowhead.com/spell=32835
    "SPELL_EFFECT_SPELL_STEAL",                 //    126 Steal Beneficial Buff (Magic) // https://www.wowhead.com/spell=30449
    "SPELL_EFFECT_PROSPECTING",                 //    127 Search 5 ore of a base metal for precious gems.  This will destroy the ore in the process.
    "SPELL_EFFECT_APPLY_FRIEND_AA",             //    128 Apply Aura friendly
    "SPELL_EFFECT_APPLY_ENEMY_AA",              //    129 Apply Aura enemy
#if VERSION_STRING >= TBC
    "SPELL_EFFECT_REDIRECT_THREAT",             //    130 unknown // https://www.wowhead.com/spell=34477
    "SPELL_EFFECT_NULL_131",                    //    131 unknown // test spell
    "SPELL_EFFECT_PLAY_MUSIC",                  //    132 Play Music // https://www.wowhead.com/spell=46852
    "SPELL_EFFECT_FORGET_SPECIALIZATION",       //    133 https://www.wowhead.com/spell=36441 // I think this is a gm/npc spell
    "SPELL_EFFECT_KILL_CREDIT",                 //    134 Quest Credit (Player only, not party) // related to summoning objects and removing them, https://www.wowhead.com/spell=39161
    "SPELL_EFFECT_NULL_135",                    //    135 Summon Pet: https://classic.wowhead.com/spell=23498
    "SPELL_EFFECT_RESTORE_HEALTH_PCT",          //    136 Restore Health % // https://www.wowhead.com/spell=41542 and https://www.wowhead.com/spell=39703
    "SPELL_EFFECT_RESTORE_POWER_PCT",           //    137 Restore Power % // https://www.wowhead.com/spell=41542
    "SPELL_EFFECT_KNOCK_BACK2",                 //    138 knockback2 // related to superjump or even "*jump" spells http://www.thottbot.com/?e=Unknown%
    "SPELL_EFFECT_CLEAR_QUEST",                 //    139 Remove Quest
    "SPELL_EFFECT_TRIGGER_SPELL",               //    140 triggers a spell from target back to caster - used at Malacrass f.e.
    "SPELL_EFFECT_NULL_141",                    //    141 unknown // triggers spell, magic one, (Mother spell) https://www.wowhead.com/spell=41065
    "SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE",    //    142 triggers some kind of "Put spell on target" thing... (dono for sure) https://www.wowhead.com/spell=40872/ and https://www.wowhead.com/spell=33076
    "SPELL_EFFECT_APPLY_OWNER_AA",              //    143 Apply Aura on summon owner // Master -> demon effecting spell, http://www.thottbot.com/s25228 and http://www.thottbot.com/s35696
    "SPELL_EFFECT_KNOCK_BACK",                  //    144 unknown
    "SPELL_EFFECT_PLAYER_PULL",                 //    145 unknown
    "SPELL_EFFECT_ACTIVATE_RUNES",              //    146 Activate Rune
    "SPELL_EFFECT_NULL_147",                    //    147 Quest Fail
    "SPELL_EFFECT_NULL_148",                    //    148 unknown
    "SPELL_EFFECT_NULL_149",                    //    149 unknown
    "SPELL_EFFECT_NULL_150",                    //    150 unknown
    "SPELL_EFFECT_TRIGGER_SPELL",               //    151
    "SPELL_EFFECT_NULL_152",                    //    152 Summon Refer-a-Friend
    "SPELL_EFFECT_CREATE_PET",                  //    153 Create tamed pet
#endif
#if VERSION_STRING >= WotLK
    "SPELL_EFFECT_TEACH_TAXI_PATH",             //    154 "Teach" a taxi path
    "SPELL_EFFECT_DUAL_WIELD_2H",               //    155
    "SPELL_EFFECT_ENCHANT_ITEM_PRISMATIC",      //    156
    "SPELL_EFFECT_CREATE_ITEM2",                //    157
    "SPELL_EFFECT_MILLING",                     //    158
    "SPELL_EFFECT_RENAME_PET",                  //    159
    "SPELL_EFFECT_NULL_160",                    //    160
    "SPELL_EFFECT_LEARN_SPEC"                   //    161 // used by spell 63624(dual talents)
    "SPELL_EFFECT_ACTIVATE_SPEC"                //    162
    "SPELL_EFFECT_NULL_163"                     //    163
    "SPELL_EFFECT_NULL_154"                     //    164
#endif
#if VERSION_STRING >= Cata
    "SPELL_EFFECT_NULL_165"                     //    165
    "SPELL_EFFECT_NULL_166"                     //    166
    "SPELL_EFFECT_NULL_167"                     //    167
    "SPELL_EFFECT_NULL_168"                     //    168
    "SPELL_EFFECT_NULL_169"                     //    169
    "SPELL_EFFECT_NULL_170"                     //    170
    "SPELL_EFFECT_NULL_171"                     //    171
    "SPELL_EFFECT_NULL_172"                     //    172
    "SPELL_EFFECT_NULL_173"                     //    173
    "SPELL_EFFECT_NULL_174"                     //    174
    "SPELL_EFFECT_NULL_175"                     //    175
    "SPELL_EFFECT_NULL_176"                     //    176
    "SPELL_EFFECT_NULL_177"                     //    177
    "SPELL_EFFECT_NULL_178"                     //    178
    "SPELL_EFFECT_NULL_179"                     //    179
    "SPELL_EFFECT_NULL_180"                     //    180
    "SPELL_EFFECT_NULL_181"                     //    181
    "SPELL_EFFECT_NULL_182"                     //    182
#endif
};

// APGL End
// MIT Start

void Spell::spellEffectNotImplemented(uint8_t effIndex)
{
    sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "Spells: Unhandled spell effect {} in spell {}.", getSpellInfo()->getEffect(effIndex), getSpellInfo()->getId());
}

void Spell::spellEffectNotUsed(uint8_t /*effIndex*/)
{
    // Handled elsewhere or not used, so do nothing
}

void Spell::spellEffectDummy(uint8_t effectIndex)
{
    // Check that the dummy effect is handled properly in spell script
    // In case it's not, generate warning to debug log
    const auto scriptResult = sScriptMgr.callScriptedSpellOnDummyOrScriptedEffect(this, effectIndex);
    if (scriptResult == SpellScriptCheckDummy::DUMMY_OK)
        return;

    if (sObjectMgr.checkForDummySpellScripts(static_cast<Player*>(u_caster), m_spellInfo->getId()))
        return;

    if (sScriptMgr.CallScriptedDummySpell(m_spellInfo->getId(), effectIndex, this))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "Spell::spellEffectDummy : Spell {} ({}) has a dummy effect index (%hhu), but no handler for it.", m_spellInfo->getId(), m_spellInfo->getName(), effectIndex);
}

void Spell::spellEffectHealthLeech(uint8_t effIndex)
{
    if (m_unitTarget == nullptr || !m_unitTarget->isAlive())
        return;

    m_targetDamageInfo = m_caster->doSpellDamage(m_unitTarget, getSpellInfo()->getId(), static_cast<float_t>(damage), effIndex, m_triggeredSpell, false, true, isForcedCrit, this);
    isTargetDamageInfoSet = true;
}

void Spell::spellEffectSummonTotem(uint8_t /*effIndex*/, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties, LocationVector& v)
{
    if (u_caster == nullptr)
        return;

    const auto totemSlot = static_cast<SummonSlot>(spe->Slot);

    // Generate spawn point
    const float_t angle = totemSlot > SUMMON_SLOT_NONE && totemSlot < SUMMON_SLOT_MINIPET
        ? M_PI_FLOAT / static_cast<float>(SUMMON_SLOT_TOTEM_AIR) - (totemSlot * 2 * M_PI_FLOAT / static_cast<float>(SUMMON_SLOT_TOTEM_AIR))
        : 0.0f;
    u_caster->GetPoint(u_caster->GetOrientation() + angle, 3.5f, v.x, v.y, v.z, false);

    // Correct Z position
    //\ todo: this probably should be inside Object::GetPoint()
    const auto landHeight = u_caster->getWorldMap()->getHeight(LocationVector(v.x, v.y, v.z + 2));
    const auto landDiff = landHeight - v.z;
    if (fabs(landDiff) <= 15)
        v.z = landHeight;

    auto summonDuration = static_cast<uint32_t>(getDuration());
    if (summonDuration == 0)
        summonDuration = 10 * 60 * 1000; // 10 min if duration does not exist

    // Create totem
    const auto totem = u_caster->getWorldMap()->summonCreature(properties->Id, v, spe, summonDuration, u_caster, getSpellInfo()->getId());
    if (totem == nullptr)
        return;

    totem->setMaxHealth(damage);
    totem->setHealth(damage);
}

void Spell::spellEffectWeapon(uint8_t /*effectIndex*/)
{
    if (m_playerTarget == nullptr)
        return;

    uint16_t skillId = 0;
    const auto skillLineAbility = sSpellMgr.getFirstSkillEntryForSpell(getSpellInfo()->getId());
    if (skillLineAbility != nullptr)
        skillId = static_cast<uint16_t>(skillLineAbility->skilline);

    const auto skillLine = sSkillLineStore.lookupEntry(skillId);
    if (skillLine == nullptr)
        return;

    if (!m_playerTarget->hasSkillLine(skillId))
        m_playerTarget->addSkillLine(skillId, 1, 0);
}

void Spell::spellEffectDefense(uint8_t /*effectIndex*/)
{
    if (m_playerTarget == nullptr)
        return;

    if (!m_playerTarget->hasSkillLine(SKILL_DEFENSE))
        m_playerTarget->addSkillLine(SKILL_DEFENSE, 1, 0);
}

void Spell::spellEffectDualWield(uint8_t /*effectIndex*/)
{
    if (m_unitTarget == nullptr)
        return;

    m_unitTarget->setDualWield(true);
}

void Spell::spellEffectSkillStep(uint8_t effectIndex)
{
    if (m_playerTarget == nullptr)
        return;

    auto skillId = static_cast<uint16_t>(getSpellInfo()->getEffectMiscValue(effectIndex));
#if VERSION_STRING <= WotLK
    // TODO: check if this is needed anymore
    // Legacy comment: somehow for lockpicking misc is different than the skill :s
    if (skillId == 242)
        skillId = SKILL_LOCKPICKING;
#endif

    const auto skillLine = sSkillLineStore.lookupEntry(skillId);
    if (skillLine == nullptr)
        return;

    // Set new skill maximum value for professions only
    // Other types will be handled on skill apply
    uint16_t max = 0;
    if (skillLine->type == SKILL_TYPE_PROFESSION || skillLine->type == SKILL_TYPE_SECONDARY)
        max = static_cast<uint16_t>(damage * 75);

    if (m_playerTarget->hasSkillLine(skillId))
        m_playerTarget->modifySkillMaximum(skillId, max);
    else
        m_playerTarget->addSkillLine(skillId, 1, max);
}

void Spell::spellEffectProficiency(uint8_t /*effectIndex*/)
{
    if (m_playerTarget == nullptr)
        return;

    m_playerTarget->applyItemProficienciesFromSpell(getSpellInfo(), true);
}

void Spell::spellEffectTriggerSpell(uint8_t effectIndex)
{
    const auto triggerInfo = sSpellMgr.getSpellInfo(getSpellInfo()->getEffectTriggerSpell(effectIndex));
    if (triggerInfo == nullptr)
        return;

    SpellCastTargets targets = m_targets;
    Spell* triggerSpell = sSpellMgr.newSpell(m_caster, triggerInfo, true, nullptr);
    triggerSpell->ProcedOnSpell = getSpellInfo();
    triggerSpell->prepare(&targets);
}

void Spell::spellEffectScriptEffect(uint8_t effectIndex)
{
    // Check that the scripted effect is handled properly in spell script
    // In case it's not, send error log
    const auto scriptResult = sScriptMgr.callScriptedSpellOnDummyOrScriptedEffect(this, effectIndex);
    if (scriptResult == SpellScriptCheckDummy::DUMMY_OK)
        return;

    // Legacy scripts
    if (sScriptMgr.CallScriptedDummySpell(m_spellInfo->getId(), effectIndex, this))
        return;

    // Legacy scripts
    if (sScriptMgr.HandleScriptedSpellEffect(m_spellInfo->getId(), effectIndex, this))
        return;

    sLogger.failure("Spell::spellEffectScriptEffect : Spell {} ({}) has a scripted effect index (%hhu), but no handler for it.", m_spellInfo->getId(), m_spellInfo->getName(), effectIndex);
}

void Spell::spellEffectSkill(uint8_t effectIndex)
{
    if (m_playerTarget == nullptr)
        return;

    const auto skillLine = static_cast<uint16_t>(getSpellInfo()->getEffectMiscValue(effectIndex));
    const auto amount = static_cast<uint16_t>(damage * 75);

    if (m_playerTarget->hasSkillLine(skillLine))
    {
        if (amount > m_playerTarget->getSkillLineMax(skillLine))
            m_playerTarget->modifySkillMaximum(skillLine, amount);
    }
    else
    {
        m_playerTarget->addSkillLine(skillLine, 1, amount);
    }
}

void Spell::spellEffectForceCast(uint8_t effectIndex)
{
    if (m_unitTarget == nullptr || getUnitCaster() == nullptr)
        return;

    const auto triggerInfo = sSpellMgr.getSpellInfo(getSpellInfo()->getEffectTriggerSpell(effectIndex));
    if (triggerInfo == nullptr)
        return;

    // TODO: figure the logic here, sometimes target should be original caster and sometimes effect target
    // hackfixing for now -Appled
    Unit* target = nullptr;
    switch (getSpellInfo()->getId())
    {
        case 72378: // Deathbringer Saurfang - Blood Nova 10m
        case 73058: // Deathbringer Saurfang - Blood Nova 25m
            target = m_unitTarget;
            break;
        default:
            target = getUnitCaster();
            break;
    }

    // TODO: original caster can also be gameobject
    m_unitTarget->castSpell(target, triggerInfo, true);
}

// MIT End
// APGL Start

void Spell::ApplyAreaAura(uint8_t effectIndex)
{
    if (!m_unitTarget || !m_unitTarget->isAlive()) return;
    if (u_caster != m_unitTarget) return;

    Aura* pAura = nullptr;
    auto itr = m_pendingAuras.find(m_unitTarget->getGuid());
    if (itr == m_pendingAuras.end())
    {
        auto auraHolder = sSpellMgr.newAura(getSpellInfo(), getDuration(), m_caster, m_unitTarget);

        float r = getEffectRadius(effectIndex);

        uint32_t eventtype = 0;

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
#if VERSION_STRING >= TBC
            case SPELL_EFFECT_APPLY_OWNER_AREA_AURA:
                eventtype = EVENT_ENEMY_AREA_AURA_UPDATE; //Zyres: The same event as SPELL_EFFECT_APPLY_ENEMY_AREA_AURA? @Appled o.O
#endif
                break;
        }

        auraHolder->m_castedItemId = castedItemId;

        if (!sEventMgr.HasEvent(auraHolder.get(), eventtype))      /* only add it once */
            sEventMgr.AddEvent(auraHolder.get(), &Aura::EventUpdateAreaAura, effectIndex, r * r, eventtype, 1000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        const auto [itr, _] = m_pendingAuras.try_emplace(m_unitTarget->getGuid(), 0, std::move(auraHolder));
        pAura = itr->second.aur.get();
    }
    else
    {
        pAura = itr->second.aur.get();
    }

    pAura->addAuraEffect(static_cast<AuraEffect>(getSpellInfo()->getEffectApplyAuraName(effectIndex)), damage, getSpellInfo()->getEffectMiscValue(effectIndex), effectPctModifier[effectIndex], isEffectDamageStatic[effectIndex], effectIndex);
}

void Spell::SpellEffectInstantKill(uint8_t /*effectIndex*/)
{
    if (!m_unitTarget || !m_unitTarget->isAlive())
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
    uint32_t spellId = getSpellInfo()->getId();

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
            uint32_t DemonicSacEffectSpellId = 0;
            switch (m_unitTarget->getEntry())
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
                SpellInfo const* se = sSpellMgr.getSpellInfo(DemonicSacEffectSpellId);
                if (se && u_caster)
                    u_caster->castSpell(u_caster, se, true);
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
            if (!u_caster || !u_caster->isPet())
                return;

            //TO< Pet* >(u_caster)->Dismiss(true);

            SpellInfo const* se = sSpellMgr.getSpellInfo(5);
            if (static_cast< Pet* >(u_caster)->getUnitOwner() == nullptr)
                return;

            SpellCastTargets targets(u_caster->getGuid());
            Spell* sp = sSpellMgr.newSpell(static_cast< Pet* >(u_caster)->getUnitOwner(), se, true, nullptr);
            sp->prepare(&targets);
            return;
        } break;

    }

    //SPELL_HASH_DEMONIC_SACRIFICE
    if (getSpellInfo()->getId() == 18788)
    {
        if (!p_caster || !m_unitTarget || !m_unitTarget->isPet())
            return;

        //TO< Pet* >(m_unitTarget)->Dismiss(true);

        SpellInfo const* se = sSpellMgr.getSpellInfo(5);

        SpellCastTargets targets(m_unitTarget->getGuid());
        Spell* sp = sSpellMgr.newSpell(p_caster, se, true, nullptr);
        sp->prepare(&targets);
        return;
    }
    else
    {
        // moar cheaters
        if (!p_caster || (u_caster && u_caster->isPet()))
            return;

        if (!p_caster->getSession()->hasPermissions())
            return;
    }

    //instant kill effects don't have a log
    //m_caster->SpellNonMeleeDamageLog(m_unitTarget, GetProto()->getId(), m_unitTarget->getHealth(), true);
    // cebernic: the value of instant kill must be higher than normal health,cuz anti health regenerated.
    if (u_caster != nullptr)
        u_caster->dealDamage(m_unitTarget, m_unitTarget->getHealth() << 1, 0);
    else
        m_unitTarget->dealDamage(m_unitTarget, m_unitTarget->getHealth() << 1, 0);
}

void Spell::SpellEffectSchoolDMG(uint8_t effectIndex) // dmg school
{
    if (!m_unitTarget || !m_unitTarget->isAlive())
        return;

    if (m_unitTarget->m_schoolImmunityList[getSpellInfo()->getFirstSchoolFromSchoolMask()])
    {
        sendCastResult(SPELL_FAILED_IMMUNE);
        return;
    }

    uint32_t dmg = damage;
    bool static_damage = false;
    bool force_crit = false;

    if (getSpellInfo()->getEffectChainTarget(effectIndex))    //chain
    {
        if (getSpellInfo()->getId() == 32445 || getSpellInfo()->getId() == 28883)
        {
            int32_t reduce = (int32_t)(getSpellInfo()->getEffectDamageMultiplier(effectIndex) * 100.0f);
            reduce -= 100;

            if (reduce && chaindamage)
            {
                if (u_caster != nullptr)
                {
                    u_caster->applySpellModifiers(SPELLMOD_JUMP_REDUCE, &reduce, getSpellInfo(), this);
                }
                chaindamage += ((getSpellInfo()->getEffectBasePoints(effectIndex) + 51) * reduce / 100);
            }
            else
            {
                chaindamage = dmg;
            }
            dmg = chaindamage;
        }
        else
        {
            int32_t reduce = (int32_t)(getSpellInfo()->getEffectDamageMultiplier(effectIndex) * 100.0f);

            if (reduce && chaindamage)
            {
                if (u_caster != nullptr)
                {
                    u_caster->applySpellModifiers(SPELLMOD_JUMP_REDUCE, &reduce, getSpellInfo(), this);
                }
                chaindamage = chaindamage * reduce / 100;
            }
            else
            {
                chaindamage = dmg;
            }
            dmg = chaindamage;
        }
    }
    else
    {
        switch (getSpellInfo()->getId())
        {
            // SPELL_HASH_METEOR_SLASH:
            case 45150:
            {
                uint32_t splitCount = 0;
                for (const auto& itr : u_caster->getInRangeOppositeFactionSet())
                {
                    if (itr && itr->isInFront(u_caster) && u_caster->CalcDistance(itr) <= 65)
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
                float _distance = u_caster->CalcDistance(m_unitTarget);
                if (_distance >= 2.0f)
                    dmg = static_cast<uint32_t>(dmg * _distance);
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
                if (m_unitTarget->hasAuraState(AURASTATE_FLAG_CONFLAGRATE, getSpellInfo(), u_caster))
                {
                    // random extra damage
                    const uint8_t spellRank = getSpellInfo()->hasSpellRanks() ? getSpellInfo()->getRankInfo()->getRank() : 1;
                    uint32_t extra_dmg = 111 + (spellRank * 11) + Util::getRandomUInt(spellRank * 11);
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
                    dmg += Util::float2int32(u_caster->getCalculatedRangedAttackPower() * 0.15f);
                dmg = Util::float2int32(dmg * (0.9f + Util::getRandomFloat(0.2f)));      // randomized damage

                if (p_caster != nullptr)
                {
                    dmg = static_cast<uint32_t>(std::round((p_caster->getCalculatedRangedAttackPower() * 0.15) + m_spellInfo->getEffectBasePoints(effectIndex)));
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
                dmg *= Util::checkChance(50) ? 2 : 1;
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
                    dmg = (getSpellInfo()->calculateEffectValue(0)) + Util::float2int32(u_caster->getCalculatedAttackPower() * 0.12f);
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
                    dmg = u_caster->getCalculatedAttackPower() * (getSpellInfo()->calculateEffectValue(2)) / 100;
            } break;

            // SPELL_HASH_JUDGEMENT_OF_COMMAND:
            case 20425:
            case 20467:
            {
                if (p_caster != nullptr)
                {
                    if (!m_unitTarget->isStunned())
                        dmg = dmg >> 1;
                    if (p_caster->hasAurasWithId(34258))
                        p_caster->castSpell(p_caster, 34260, true);
                    if ((p_caster->hasAurasWithId(53696) || p_caster->hasAurasWithId(53695)))
                        p_caster->castSpell(p_caster, 68055, true);
                    if (p_caster->hasAurasWithId(37186))
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
                if (!m_unitTarget->isStunned())
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
                    uint32_t sph = p_caster->getModDamageDonePositive(SCHOOL_HOLY);
                    int32_t ap = p_caster->getCalculatedAttackPower();
                    dmg += Util::float2int32((0.15f * sph) + (0.15f * ap));
                    if (m_unitTarget && m_unitTarget->isCreature())
                    {
                        uint32_t type = static_cast<Creature*>(m_unitTarget)->GetCreatureProperties()->Type;
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
                    Item* it = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                    if (it && it->getItemProperties() && it->getItemProperties()->InventoryType == INVTYPE_SHIELD)
                        dmg = Util::float2int32(1.3f * p_caster->getShieldBlock());
#else
                    dmg += Util::float2int32(1.30f * p_caster->getCombatRating(CR_BLOCK) + getSpellInfo()->getEffectBasePoints(0));
#endif
                }
            } break;

            //SPELL_HASH_CONFLAGRATE
            case 17962:
                m_unitTarget->removeAuraStateAndAuras(AURASTATE_FLAG_CONFLAGRATE);
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
                if (m_unitTarget->hasAuraState(AURASTATE_FLAG_FROZEN, getSpellInfo(), u_caster) || m_unitTarget->hasAurasWithId(44572))
                    dmg *= 3;
            } break;

            //SPELL_HASH_HEROIC_THROW
            case 57755:
            {
                if (u_caster)
                    dmg = u_caster->getCalculatedAttackPower() / 2 + 12;
                if (p_caster != nullptr)
                    dmg = static_cast<uint32_t>(std::round(p_caster->getCalculatedAttackPower() * 0.5));
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
                    Item* it = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                    if (it && it->getItemProperties()->InventoryType == INVTYPE_SHIELD)
                    {
                        float block_multiplier = (100.0f + p_caster->m_modBlockAbsorbValue) / 100.0f;
                        if (block_multiplier < 1.0f)block_multiplier = 1.0f;

                        int32_t blockable_damage = Util::float2int32((it->getItemProperties()->Block + p_caster->m_modBlockValueFromSpells + p_caster->getCombatRating(CR_BLOCK) + ((p_caster->getStat(STAT_STRENGTH) / 2.0f) - 1.0f)) * block_multiplier);

                        /*
                        3.2.0:
                        The benefit from additional block value this ability gains is now subject
                        to diminishing returns. Diminishing returns occur once block value exceeds
                        30 times the player's level and caps the maximum damage benefit from shield
                        block value at 34.5 times the player's level.
                        */
                        int32_t max_blockable_damage = static_cast<int32_t>(p_caster->getLevel() * 34.5f);
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
                    dmg = static_cast<uint32_t>(std::round(p_caster->getCalculatedAttackPower() * 0.5));
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
                dmg = u_caster->getCalculatedAttackPower() * (getSpellInfo()->calculateEffectValue(0)) / 100;
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
                dmg = u_caster->getCalculatedAttackPower() * (getSpellInfo()->calculateEffectValue(2)) / 100;
            } break;

            // SPELL_HASH_INTERCEPT
            case 20252:
            {
                if (p_caster != nullptr)
                {
                    dmg = static_cast<uint32_t>(std::round(p_caster->getCalculatedAttackPower() * 0.12));
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
                    dmg = Util::float2int32(u_caster->getCalculatedAttackPower() * 0.12f);
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
                    dmg = p_caster->getCalculatedAttackPower() * ((m_spellInfo->calculateEffectValue(0)) / 100);
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
                    Item* it = static_cast<Item*>(p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND));
                    if (it && it->getItemProperties() && it->getItemProperties()->InventoryType == INVTYPE_SHIELD)
                        dmg = p_caster->getUInt32Value(PLAYER_SHIELD_BLOCK);
#endif
                }
            }break;*/
            case 34428:
            {
                if (p_caster != nullptr)
                {
                    p_caster->removeAuraStateAndAuras(AURASTATE_FLAG_SWIFTMEND);
                    dmg = (p_caster->getCalculatedAttackPower()*(m_spellInfo->getEffectBasePoints(effectIndex) + 1)) / 100;
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
                    dmg = static_cast<uint32_t>(std::round(p_caster->getCalculatedAttackPower() * 0.207));
            }break;

            case 64382:
                if (p_caster != nullptr)
                    dmg = static_cast<uint32_t>(std::round(p_caster->getCalculatedAttackPower() * 0.5));
                break;

            case 29707:
            case 30324:
            case 47449:
            case 47450:
            {
                /* Possibly broken (infinity loop) -- ask Zyres
                if (p_caster != nullptr)
                {
                    if (m_unitTarget->isDazed())
                        for (uint32_t i = UNIT_FIELD_AURASTATE; i < AURASTATE_FLAG_SWIFTMEND; i)
                        {
                            switch (m_spellInfo->getId())
                            { // This info isn't in the dbc files.....
                                case 29707:
                                    dmg = static_cast<uint32_t>(std::round(81.9));
                                    break;
                                case 30324:
                                    dmg = static_cast<uint32_t>(std::round(110.95));
                                    break;
                                case 47449:
                                    dmg = static_cast<uint32_t>(std::round(151.2));
                                    break;
                                case 47450:
                                    dmg = static_cast<uint32_t>(std::round(173.25));
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
                    auto item = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                    if (item != nullptr)
                    {
                        if (p_caster->hasAurasWithId(12329))
                            dmg = static_cast<uint32_t>(std::round((((item->getItemProperties()->Damage[0].Min + item->getItemProperties()->Damage[0].Max) * 0.2f) + m_spellInfo->getEffectBasePoints(effectIndex)) * 0.4));
                        else if (p_caster->hasAurasWithId(12950))
                            dmg = static_cast<uint32_t>(std::round((((item->getItemProperties()->Damage[0].Min + item->getItemProperties()->Damage[0].Max) * 0.2f) + m_spellInfo->getEffectBasePoints(effectIndex)) * 0.8));
                        else if (p_caster->hasAurasWithId(20496))
                            dmg = static_cast<uint32_t>(std::round((((item->getItemProperties()->Damage[0].Min + item->getItemProperties()->Damage[0].Max) * 0.2f) + m_spellInfo->getEffectBasePoints(effectIndex)) * 1.2));
                        else
                            dmg = static_cast<uint32_t>(std::round(((item->getItemProperties()->Damage[0].Min + item->getItemProperties()->Damage[0].Max) * 0.2f) + m_spellInfo->getEffectBasePoints(effectIndex)));
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
                    auto item = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                    if (item != nullptr)
                        dmg = static_cast<uint32_t>(std::round(((item->getItemProperties()->Damage[0].Min + item->getItemProperties()->Damage[0].Max) * 0.2f) + m_spellInfo->getEffectBasePoints(effectIndex)));
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
                    if (p_caster->hasAurasWithId(34258))
                        p_caster->castSpell(p_caster, 34260, true);
                    if ((p_caster->hasAurasWithId(53696) || p_caster->hasAurasWithId(53695)))
                        p_caster->castSpell(p_caster, 68055, true);
                    if (p_caster->hasAurasWithId(37186))
                        dmg = 33;
                }
            }break;

            case 25742:
            {
                if (p_caster != nullptr)
                    dmg = static_cast<uint32_t>(std::round(p_caster->getBaseAttackTime(MELEE) / 1000 * ((0.022 * (p_caster->getCalculatedAttackPower()) + (0.044 * (p_caster->GetDamageDoneMod(1))))) + m_spellInfo->getEffectBasePoints(effectIndex)));
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
                    auto item = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                    if (item != nullptr)
                        dmg = static_cast<uint32_t>(std::round(((item->getItemProperties()->Damage[0].Min + item->getItemProperties()->Damage[0].Max) * 0.2f) + m_spellInfo->getEffectBasePoints(effectIndex)));

                }
            }break;
            case 53209:
            {
                if (p_caster != nullptr)
                {
                    auto item = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                    if (item != nullptr)
                        dmg = static_cast<uint32_t>(std::round(((item->getItemProperties()->Damage[0].Min + item->getItemProperties()->Damage[0].Max) * 0.2f) * 1.25));
                }
            }break;
            case 56641:
            case 34120:
            case 49051:
            case 49052:
            {
#if VERSION_STRING < Cata
                if (p_caster != nullptr)
                {
                    Item* pItem = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_RANGED);
                    ItemProperties const* pItemProto = sMySQLStore.getItemProperties(p_caster->getAmmoId());
                    uint32_t stundmg;
                    float bowdmg;
                    float ammodmg;
                    if (m_unitTarget->isDazed())
                        stundmg = p_caster->getCalculatedRangedAttackPower() / 10 + m_spellInfo->getEffectBasePoints(effectIndex) + m_spellInfo->getEffectBasePoints(effectIndex + 1);
                    else
                        stundmg = p_caster->getCalculatedRangedAttackPower() / 10 + m_spellInfo->getEffectBasePoints(effectIndex);
                    if (pItem)
                        bowdmg = (pItem->getItemProperties()->Damage[0].Min + pItem->getItemProperties()->Damage[0].Max) * 0.2f;
                    else
                        bowdmg = 0;
                    if (pItemProto)
                        ammodmg = (pItemProto->Damage[0].Min + pItemProto->Damage[0].Max) * 0.2f;
                    else
                        ammodmg = 0;

                    dmg = Util::float2int32(ammodmg + bowdmg) + stundmg;
                }
#endif
            }break;
            case 64422: // Sonic Screech, Auriaya encounter
            case 64688:
            {
                if (u_caster != nullptr)
                {
                    int splitCount = 0;
                    for (const auto& itr : u_caster->getInRangeOppositeFactionSet())
                    {
                        if (itr && itr->isInFront(u_caster))
                            splitCount++;
                    }

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
#if VERSION_STRING > TBC
            case DEATHKNIGHT:
#endif
                static_damage = true; //No spells from these classes benefit from spell damage. Prevents Arc hunters, frost DKs, etc.
                break;
            default:
                break;
        }
    }


    // check for no more damage left (chains)
    if (!dmg)
        dmg = getSpellInfo()->getEffectBasePoints(effectIndex);

    if (!dmg)
        return;

    if (getSpellInfo()->getSpeed() > 0 && m_triggeredSpell == false)
    {
        m_targetDamageInfo = m_caster->doSpellDamage(m_unitTarget, getSpellInfo()->getId(), static_cast<float_t>(dmg), effectIndex, pSpellId != 0, false, false, isForcedCrit, this);
        isTargetDamageInfoSet = true;
    }
    else
    {
        if (GetType() == SPELL_DMG_TYPE_MAGIC)
        {
            m_targetDamageInfo = m_caster->doSpellDamage(m_unitTarget, getSpellInfo()->getId(), static_cast<float_t>(dmg), effectIndex, m_triggeredSpell, false, false, isForcedCrit, this);
            isTargetDamageInfoSet = true;
        }
        else
        {
            if (u_caster != nullptr)
            {
                WeaponDamageType _type;
                if (GetType() == SPELL_DMG_TYPE_RANGED)
                    _type = RANGED;
                else
                {
                    if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_REQUIRES_OFFHAND_WEAPON)
                        _type = OFFHAND;
                    else
                        _type = MELEE;
                }

                m_targetDamageInfo = u_caster->strike(m_unitTarget, _type, getSpellInfo(), 0, 0, dmg, m_triggeredSpell, true, (force_crit || isForcedCrit), this);
                isTargetDamageInfoSet = true;
            }
        }
    }
}

void Spell::SpellEffectTeleportUnits(uint8_t effectIndex)    // Teleport Units
{
    if (m_unitTarget == nullptr || m_caster == nullptr)
        return;

    uint32_t spellId = getSpellInfo()->getId();

    // Portals
    if (m_spellInfo->hasCustomFlagForEffect(effectIndex, TELEPORT_TO_COORDINATES))
    {
        TeleportCoords const* teleport_coord = sMySQLStore.getTeleportCoord(spellId);
        if (teleport_coord == nullptr)
        {
            sLogger.failure("Spell {} ({}) has a TELEPORT TO COORDINATES effect, but has no coordinates to teleport to. ", spellId, m_spellInfo->getName());
            return;
        }

        HandleTeleport(LocationVector(teleport_coord->x, teleport_coord->y, teleport_coord->z), teleport_coord->mapId, m_unitTarget);
        return;
    }

    // Hearthstone and co.
    if (m_spellInfo->hasCustomFlagForEffect(effectIndex, TELEPORT_TO_BINDPOINT))
    {
        if (m_unitTarget->isPlayer())
        {
            Player* p = static_cast<Player*>(m_unitTarget);

            HandleTeleport(p->getBindPosition(), p->getBindMapId(), p);
        }
        return;
    }

    // Summon
    if (m_spellInfo->hasCustomFlagForEffect(effectIndex, TELEPORT_TO_CASTER))
    {
        if (u_caster == nullptr)
            return;

        HandleTeleport(m_caster->GetPosition(), m_caster->GetMapId(), m_unitTarget);
        return;
    }

    // Shadowstep for example
    if (m_spellInfo->hasCustomFlagForEffect(effectIndex, TELEPORT_BEHIND_TARGET))
    {
        if (p_caster == nullptr)
            return;

        ///////////////////////////////////////////////// Code taken from the Shadowstep dummy script /////////////////////////////////////////////////////////////////////

        /* this is rather tricky actually. we have to calculate the orientation of the creature/player, and then calculate a little bit of distance behind that. */
        float ang;

        if (m_unitTarget == m_caster)
        {
            /* try to get a selection */
            m_unitTarget = m_caster->getWorldMap()->getUnit(m_targets.getUnitTargetGuid());
            if ((!m_unitTarget) || !p_caster->isValidTarget(m_unitTarget, getSpellInfo()) || (m_unitTarget->CalcDistance(p_caster) > 28.0f))
            {
                return;
            }
        }

        if (m_unitTarget->isCreature())
        {
            if (m_unitTarget->getTargetGuid() != 0)
            {
                /* We're chasing a target. We have to calculate the angle to this target, this is our orientation. */
                ang = m_caster->calcAngle(m_caster->GetPositionX(), m_caster->GetPositionY(), m_unitTarget->GetPositionX(), m_unitTarget->GetPositionY());
                /* convert degree angle to radians */
                ang = ang * M_PI_FLOAT / 180.0f;
            }
            else
            {
                /* Our orientation has already been set. */
                ang = m_unitTarget->GetOrientation();
            }
        }
        else
        {
            /* Players orientation is sent in movement packets */
            ang = m_unitTarget->GetOrientation();
        }
        // avoid teleporting into the model on scaled models
        const static float shadowstep_distance = 1.6f * m_unitTarget->getScale();
        float new_x = m_unitTarget->GetPositionX() - (shadowstep_distance * cosf(ang));
        float new_y = m_unitTarget->GetPositionY() - (shadowstep_distance * sinf(ang));
        /* Send a movement packet to "charge" at this target. Similar to warrior charge. */
        p_caster->m_zAxisPosition = 0.0f;
        p_caster->safeTeleport(p_caster->GetMapId(), p_caster->GetInstanceID(), LocationVector(new_x, new_y, (m_unitTarget->GetPositionZ() + 0.1f), ang));


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        return;
    }

    // For those special teleport spells
    if (sScriptMgr.CallScriptedDummySpell(m_spellInfo->getId(), effectIndex, this))
        return;

    sLogger.failure("Unhandled Teleport effect Index %hhu for Spell {} ({}).", effectIndex, m_spellInfo->getId(), m_spellInfo->getName());
}

void Spell::SpellEffectApplyAura(uint8_t effectIndex)  // Apply Aura
{
    if (m_unitTarget == nullptr)
        return;

#ifdef GM_Z_DEBUG_DIRECTLY
    else
    {
        if (m_unitTarget->isPlayer() && m_unitTarget->IsInWorld() && TO< Player* >(m_unitTarget)->getSession() && TO< Player* >(m_unitTarget)->getSession()->CanUseCommand('z'))
        {
            sChatHandler.BlueSystemMessage(TO< Player* >(m_unitTarget)->getSession(), "[%sSystem%s] |rSpell::SpellEffectApplyAura: %s EffectApplyAuraName [%u] .", MSG_COLOR_WHITE, MSG_COLOR_LIGHTBLUE, MSG_COLOR_SUBWHITE,
                                           i);
        }
    }
#endif

    // avoid map corruption.
    if (m_unitTarget->GetInstanceID() != m_caster->GetInstanceID())
        return;

    //check if we already have stronger aura
    Aura* pAura;

    auto itr = m_pendingAuras.find(m_unitTarget->getGuid());
    //if we do not make a check to see if the aura owner is the same as the caster then we will stack the 2 auras and they will not be visible client sided
    if (itr == m_pendingAuras.end())
    {
        uint32_t Duration = getDuration();

        if (ProcedOnSpell)  //Warrior's Blood Frenzy
        {
            switch (getSpellInfo()->getId())
            {
                //SPELL_HASH_BLOOD_FRENZY
                case 16952:
                case 16954:
                case 29836:
                case 29859:
                case 30069:
                case 30070:
                {
                    const auto motherSpellDuration = sSpellDurationStore.lookupEntry(ProcedOnSpell->getDurationIndex());
                    if (motherSpellDuration != nullptr)
                    {
                        if (motherSpellDuration->Duration3 > 0)
                            Duration = motherSpellDuration->Duration3;
                        else if (motherSpellDuration->Duration2 > 0)
                            Duration = motherSpellDuration->Duration2;
                        else
                            Duration = motherSpellDuration->Duration1;
                    }
                } break;
            }
        }

        // Handle diminishing returns, if it should be resisted, it'll make duration 0 here.
        if (!(getSpellInfo()->isPassive())) // Passive
        {
            m_unitTarget->applyDiminishingReturnTimer(&Duration, getSpellInfo());
        }

        if (!Duration)
        {
            sendCastResult(SPELL_FAILED_IMMUNE);
            return;
        }

        std::unique_ptr<Aura> auraHolder;
        if (g_caster && g_caster->getCreatedByGuid() && g_caster->getUnitOwner())
            auraHolder = sSpellMgr.newAura(getSpellInfo(), Duration, g_caster->getUnitOwner(), m_unitTarget, m_triggeredSpell, i_caster);
        else
            auraHolder = sSpellMgr.newAura(getSpellInfo(), Duration, m_caster, m_unitTarget, m_triggeredSpell, i_caster);

        auraHolder->pSpellId = pSpellId; //this is required for triggered spells
        auraHolder->m_castedItemId = castedItemId;

        const auto [itr, _] = m_pendingAuras.try_emplace(m_unitTarget->getGuid(), 0, std::move(auraHolder));
        pAura = itr->second.aur.get();
    }
    else
    {
        pAura = itr->second.aur.get();
    }
    switch (m_spellInfo->getId())
    {
        case 27907:
        {
            if (m_unitTarget->getEntry() == 15941)
            {
                m_unitTarget->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "What? Oh, not this again!");
            }
            else if (m_unitTarget->getEntry() == 15945)
            {
                m_unitTarget->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "You can't do this to me! We had a deal!");
            }
            else
            {
                sendCastResult(SPELL_FAILED_BAD_TARGETS);
                return;
            }
        }break;
        case 28880:
        {
            if (!p_caster)
                break;

            if (m_unitTarget->getEntry() == 16483)
            {
                m_unitTarget->removeAllAurasById(29152);
                m_unitTarget->setStandState(STANDSTATE_STAND);
                static const char* testo[12] = { "None", "Warrior", "Paladin", "Hunter", "Rogue", "Priest", "Death Knight", "Shaman", "Mage", "Warlock", "None", "Druid" };
                char msg[150];
                snprintf(msg, 150, "Many thanks to you %s. I'd best get to the crash site and see how I can help out. Until we meet again...", testo[p_caster->getClass()]);
                m_unitTarget->sendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, msg);
                ((Creature*)m_unitTarget)->Despawn(900000, 300000);
            }
        }break;
        case 38177:
        {
            if (!p_caster)
                break;

            if (m_unitTarget->getEntry() == 21387)
            {
                ((Creature*)m_unitTarget)->Despawn(5000, 360000);
                p_caster->castSpell(p_caster, 38178, true);
            }
            else
            {
                sendCastResult(SPELL_FAILED_BAD_TARGETS);
                return;
            }
        }break;
    }

    if (!pAura)
    {
        sLogger.failure("Warning tried to add Aura Effect for nonexistant Aura returning!");
        return;
    }

    pAura->addAuraEffect(static_cast<AuraEffect>(getSpellInfo()->getEffectApplyAuraName(effectIndex)), damage, getSpellInfo()->getEffectMiscValue(effectIndex), effectPctModifier[effectIndex], isEffectDamageStatic[effectIndex], effectIndex);
}

void Spell::SpellEffectEnvironmentalDamage(uint8_t effectIndex)
{
    if (!m_playerTarget) return;

    if (m_playerTarget->m_schoolImmunityList[getSpellInfo()->getFirstSchoolFromSchoolMask()])
    {
        sendCastResult(SPELL_FAILED_IMMUNE);
        return;
    }
    //this is GO, not unit
    m_targetDamageInfo = m_caster->doSpellDamage(m_playerTarget, getSpellInfo()->getId(), static_cast<float_t>(damage), effectIndex, m_triggeredSpell, false, false, isForcedCrit, this);
    isTargetDamageInfoSet = true;

    m_playerTarget->sendEnvironmentalDamageLogPacket(m_playerTarget->getGuid(), DAMAGE_FIRE, damage);
}

void Spell::SpellEffectPowerDrain(uint8_t effectIndex)  // Power Drain
{
    if (!m_unitTarget || !m_unitTarget->isAlive())
        return;

    auto powerField = static_cast<PowerType>(getSpellInfo()->getEffectMiscValue(effectIndex));
    auto curPower = m_unitTarget->getPower(powerField);
    if (powerField == POWER_TYPE_MANA && m_unitTarget->isPlayer())
    {
        Player* mPlayer = static_cast< Player* >(m_unitTarget);
        if (mPlayer->isInFeralForm())
            return;

        // Resilience - reduces the effect of mana drains by (CalcRating*2)%.
        damage = Util::float2int32(damage * (1 - ((static_cast< Player* >(m_unitTarget)->calcRating(CR_CRIT_TAKEN_SPELL) * 2) / 100.0f)));
    }
    uint32_t amt = damage + ((u_caster->GetDamageDoneMod(getSpellInfo()->getFirstSchoolFromSchoolMask()) * 80) / 100);
    if (amt > curPower)
        amt = curPower;
    m_unitTarget->setPower(powerField, curPower - amt);
    u_caster->energize(u_caster, getSpellInfo()->getId(), amt, powerField);
}

void Spell::SpellEffectHeal(uint8_t effectIndex) // Heal
{
    if (p_caster != nullptr)
    {
        // HACKY but with SM_FEffect2_bonus it doesnt work
        uint32_t fireResistanceAura[] =
        {
            //SPELL_HASH_FIRE_RESISTANCE_AURA
            19891,
            19899,
            19900,
            27153,
            48947,
            0
        };

        uint32_t frostResistanceAura[] =
        {
            //SPELL_HASH_FROST_RESISTANCE_AURA
            19888,
            19897,
            19898,
            27152,
            48945,
            0
        };

        uint32_t shadowResistanceAura[] =
        {
            //SPELL_HASH_SHADOW_RESISTANCE_AURA
            19876,
            19895,
            19896,
            27151,
            48943,
            0
        };

        uint32_t retributionAura[] =
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

        uint32_t devotionAura[] =
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
        if (m_unitTarget && (m_unitTarget->hasAurasWithId(devotionAura) || m_unitTarget->hasAurasWithId(retributionAura) ||
            m_unitTarget->hasAurasWithId(19746) || m_unitTarget->hasAurasWithId(32223) || m_unitTarget->hasAurasWithId(fireResistanceAura) ||
            m_unitTarget->hasAurasWithId(frostResistanceAura) || m_unitTarget->hasAurasWithId(shadowResistanceAura)))
        {
            if (p_caster->hasSpell(20140))     // Improved Devotion Aura Rank 3
                damage = (int32_t)(damage * 1.06);
            else if (p_caster->hasSpell(20139))     // Improved Devotion Aura Rank 2
                damage = (int32_t)(damage * 1.04);
            else if (p_caster->hasSpell(20138))     // Improved Devotion Aura Rank 1
                damage = (int32_t)(damage * 1.02);
        }

        if (p_caster->hasSpell(54943) && p_caster->hasAurasWithId(20165))       // Glyph of Seal of Light
            damage = (int32_t)(damage * 1.05);
    }

    auto heal = damage;

    if (getSpellInfo()->getEffectChainTarget(effectIndex))    //chain
    {
        if (!chaindamage)
        {
            chaindamage = heal;
            m_targetDamageInfo = m_caster->doSpellHealing(m_unitTarget, getSpellInfo()->getId(), static_cast<float_t>(chaindamage), m_triggeredSpell, false, false, isForcedCrit, this);
            isTargetDamageInfoSet = true;
        }
        else
        {
            int32_t reduce = getSpellInfo()->getEffectDieSides(effectIndex) + 1;
            if (u_caster != nullptr)
            {
                u_caster->applySpellModifiers(SPELLMOD_JUMP_REDUCE, &reduce, getSpellInfo(), this);
            }
            chaindamage -= (reduce * chaindamage) / 100;
            m_targetDamageInfo = m_caster->doSpellHealing(m_unitTarget, getSpellInfo()->getId(), static_cast<float_t>(chaindamage), m_triggeredSpell, false, false, isForcedCrit, this);
            isTargetDamageInfoSet = true;
        }
    }
    else
    {
        //yep, the usual special case. This one is shaman talent : Nature's guardian
        //health is below 30%, we have a mother spell to get value from
        switch (getSpellInfo()->getId())
        {
            case 31616:
            {
                if (m_unitTarget && m_unitTarget->isPlayer() && pSpellId && m_unitTarget->getHealthPct() < 30)
                {
                    //check for that 10 second cooldown
                    const auto spellInfo = sSpellMgr.getSpellInfo(pSpellId);
                    if (spellInfo)
                    {
                        //heal value is received by the level of current active talent :s
                        //maybe we should use CalculateEffect(uint32_t i) to gain SM benefits
                        int32_t value = 0;
                        int32_t basePoints = spellInfo->getEffectBasePoints(effectIndex) + 1; //+(m_caster->getLevel()*basePointsPerLevel);
                        uint32_t randomPoints = spellInfo->getEffectDieSides(effectIndex);
                        if (randomPoints <= 1)
                            value = basePoints;
                        else
                            value = basePoints + Util::getRandomUInt(randomPoints);
                        //the value is in percent. Until now it's a fixed 10%
                        const auto amt = m_unitTarget->getMaxHealth()*value / 100.0f;
                        m_targetDamageInfo = m_caster->doSpellHealing(m_unitTarget, getSpellInfo()->getId(), amt, m_triggeredSpell, false, false, isForcedCrit, this);
                        isTargetDamageInfoSet = true;
                    }
                }
            }
            break;
            //Bloodthirst
            case 23880:
            {
                if (m_unitTarget)
                {
                    m_targetDamageInfo = m_caster->doSpellHealing(m_unitTarget, getSpellInfo()->getId(), m_unitTarget->getMaxHealth() / 100.0f, m_triggeredSpell, false, false, isForcedCrit, this);
                    isTargetDamageInfoSet = true;
                }
            }
            break;
            case 22845: // Druid: Frenzied Regeneration
            {
                if (m_unitTarget == nullptr || !m_unitTarget->isPlayer() || !m_unitTarget->isAlive())
                    break;

                Player* mPlayer = static_cast< Player* >(m_unitTarget);
                if (!mPlayer->isInFeralForm() ||
                    (mPlayer->getShapeShiftForm() != FORM_BEAR &&
                    mPlayer->getShapeShiftForm() != FORM_DIREBEAR))
                    break;
                uint32_t val = mPlayer->getPower(POWER_TYPE_RAGE);
                if (val > 100)
                    val = 100;
                uint32_t HpPerPoint = Util::float2int32((mPlayer->getMaxHealth() * 0.003f));   //0.3% of hp per point of rage
                uint32_t healAmt = HpPerPoint * (val / 10); //1 point of rage = 0.3% of max hp
                mPlayer->modPower(POWER_TYPE_RAGE, -1 * val);

                if (val)
                    mPlayer->addSimpleHealingBatchEvent(healAmt, mPlayer, sSpellMgr.getSpellInfo(22845));
            }
            break;
            case 18562: //druid - swiftmend
            {
                if (m_unitTarget)
                {
                    float_t new_dmg = 0.0f;
                    //consume rejuvenetaion and regrowth

                    uint32_t regrowth[] =
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
                    Aura* taura = m_unitTarget->getAuraWithId(regrowth);    //Regrowth
                    if (taura && taura->getSpellInfo())
                    {
                        uint32_t amplitude = taura->getSpellInfo()->getEffectAmplitude(1) / 1000;
                        if (!amplitude)
                            amplitude = 3;

                        //our hapiness is that we did not store the aura mod amount so we have to recalc it
                        Spell* spell = sSpellMgr.newSpell(m_caster, taura->getSpellInfo(), false, nullptr);
                        uint32_t healamount = spell->calculateEffect(1);
                        delete spell;
                        spell = nullptr;
                        new_dmg = healamount * 18.0f / amplitude;

                        taura->removeAura();

                        //do not remove flag if we still can cast it again
                        uint32_t rejuvenation[] =
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

                        if (!m_unitTarget->hasAurasWithId(rejuvenation))
                        {
                            m_unitTarget->removeAuraStateAndAuras(AURASTATE_FLAG_SWIFTMEND);
                            sEventMgr.RemoveEvents(m_unitTarget, EVENT_REJUVENATION_FLAG_EXPIRE);
                        }
                    }
                    else
                    {
                        uint32_t rejuvenation[] =
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

                        taura = m_unitTarget->getAuraWithId(rejuvenation);  //Rejuvenation
                        if (taura  && taura->getSpellInfo())
                        {
                            uint32_t amplitude = taura->getSpellInfo()->getEffectAmplitude(0) / 1000;
                            if (!amplitude) amplitude = 3;

                            //our happiness is that we did not store the aura mod amount so we have to recalc it
                            Spell* spell = sSpellMgr.newSpell(m_caster, taura->getSpellInfo(), false, nullptr);
                            uint32_t healamount = spell->calculateEffect(0);
                            delete spell;
                            spell = nullptr;
                            new_dmg = healamount * 12.0f / amplitude;

                            taura->removeAura();

                            m_unitTarget->removeAuraStateAndAuras(AURASTATE_FLAG_SWIFTMEND);
                            sEventMgr.RemoveEvents(m_unitTarget, EVENT_REJUVENATION_FLAG_EXPIRE);
                        }
                    }

                    if (new_dmg > 0.0f)
                    {
                        const auto spellInfo = sSpellMgr.getSpellInfo(18562);
                        Spell* spell = sSpellMgr.newSpell(m_unitTarget, spellInfo, true, nullptr);
                        spell->setUnitTarget(m_unitTarget);
                        m_targetDamageInfo = m_caster->doSpellHealing(m_unitTarget, spellInfo->getId(), new_dmg, m_triggeredSpell, false, false, isForcedCrit, this);
                        isTargetDamageInfoSet = true;
                        delete spell;
                    }
                }
            }
            break;
            default:
                m_targetDamageInfo = m_caster->doSpellHealing(m_unitTarget, getSpellInfo()->getId(), static_cast<float_t>(heal), m_triggeredSpell, false, false, isForcedCrit, this);
                isTargetDamageInfoSet = true;
                break;
        }
    }
}

void Spell::SpellEffectBind(uint8_t effectIndex)
{
    if (!m_playerTarget || !m_playerTarget->isAlive() || !m_caster)
        return;

    WorldPacket data(45);
    uint32_t areaid = m_playerTarget->getZoneId();
    uint32_t mapid = m_playerTarget->GetMapId();
    if (getSpellInfo()->getEffectMiscValue(effectIndex))
    {
        areaid = getSpellInfo()->getEffectMiscValue(effectIndex);
        auto at = MapManagement::AreaManagement::AreaStorage::GetAreaById(areaid);
        if (!at)
            return;
        mapid = at->map_id;
    }

    m_playerTarget->setBindPoint(m_playerTarget->GetPositionX(), m_playerTarget->GetPositionY(), m_playerTarget->GetPositionZ(), m_playerTarget->GetOrientation(), mapid, areaid);

    m_playerTarget->getSession()->SendPacket(SmsgBindPointUpdate(m_playerTarget->getBindPosition(), m_playerTarget->getBindMapId(), m_playerTarget->getBindZoneId()).serialise().get());

    m_playerTarget->getSession()->SendPacket(SmsgPlayerBound(m_caster->getGuid(), m_playerTarget->getBindZoneId()).serialise().get());
}

void Spell::SpellEffectQuestComplete(uint8_t effectIndex) // Quest Complete
{
    if (!p_caster)
        return;

    if (auto* questLog = p_caster->getQuestLogByQuestId(getSpellInfo()->getEffectMiscValue(effectIndex)))
    {
        questLog->setStateComplete();
        questLog->updatePlayerFields();
        questLog->sendQuestComplete();
    }
}

//wand->
void Spell::SpellEffectWeapondamageNoschool(uint8_t /*effectIndex*/) // Weapon damage + (no School)
{
    if (!m_unitTarget || !u_caster)
        return;

    m_targetDamageInfo = u_caster->strike(m_unitTarget, (GetType() == SPELL_DMG_TYPE_RANGED ? RANGED : MELEE), getSpellInfo(), damage, 0, 0, m_triggeredSpell, true, isForcedCrit, this);
    isTargetDamageInfoSet = true;
}

void Spell::SpellEffectResurrect(uint8_t effectIndex) // Resurrect (Flat)
{
    if (!m_playerTarget)
    {
        if (!m_corpseTarget)
        {
            // unit resurrection handler
            if (m_unitTarget)
            {
                if (m_unitTarget->isCreature() && m_unitTarget->isPet() && m_unitTarget->isDead())
                {
                    uint32_t hlth = ((uint32_t)getSpellInfo()->getEffectBasePoints(effectIndex) > m_unitTarget->getMaxHealth()) ? m_unitTarget->getMaxHealth() : (uint32_t)getSpellInfo()->getEffectBasePoints(effectIndex);
                    uint32_t mana = ((uint32_t)getSpellInfo()->getEffectBasePoints(effectIndex) > m_unitTarget->getMaxPower(POWER_TYPE_MANA)) ? m_unitTarget->getMaxPower(POWER_TYPE_MANA) : (uint32_t)getSpellInfo()->getEffectBasePoints(effectIndex);

                    if (!m_unitTarget->isPet())
                    {
                        sEventMgr.RemoveEvents(m_unitTarget, EVENT_CREATURE_REMOVE_CORPSE);
                    }
                    else
                    {
                        sEventMgr.RemoveEvents(m_unitTarget, EVENT_PET_DELAYED_REMOVE);
                        sEventMgr.RemoveEvents(m_unitTarget, EVENT_CREATURE_REMOVE_CORPSE);
                    }
                    m_unitTarget->setHealth(hlth);
                    m_unitTarget->setPower(POWER_TYPE_MANA, mana);
                    m_unitTarget->setTaggerGuid(nullptr);
                    m_unitTarget->setDeathState(ALIVE);
                    m_unitTarget->loot.clear();
                    static_cast< Creature* >(m_unitTarget)->SetLimboState(false); // we can regenerate health now
                }
            }

            return;
        }

        WoWGuid wowGuid;
        wowGuid.Init(m_corpseTarget->getOwnerGuid());

        m_playerTarget = sObjectMgr.getPlayer(wowGuid.getGuidLowPart());
        if (!m_playerTarget) return;
    }

    if (m_playerTarget->isAlive() || !m_playerTarget->IsInWorld())
        return;

    uint32_t health = getSpellInfo()->getEffectBasePoints(effectIndex);
    uint32_t mana = getSpellInfo()->getEffectMiscValue(effectIndex);

    m_playerTarget->setResurrectHealth(health);
    m_playerTarget->setResurrectMana(mana);

    SendResurrectRequest(m_playerTarget);
    m_playerTarget->setMoveRoot(false);
}

void Spell::SpellEffectAddExtraAttacks(uint8_t /*effectIndex*/) // Add Extra Attacks
{
    if (!u_caster)
        return;
    u_caster->m_extraAttacks += damage;
}

void Spell::SpellEffectDodge(uint8_t /*effectIndex*/)
{
    //i think this actually enables the skill to be able to dodge melee+ranged attacks
    //value is static and sets value directly which will be modified by other factors
    //this is only basic value and will be overwritten elsewhere !!!
    //  if (m_unitTarget->isPlayer())
    //      m_unitTarget->SetFloatValue(PLAYER_DODGE_PERCENTAGE,damage);
}

void Spell::SpellEffectParry(uint8_t /*effectIndex*/)
{
    if (m_unitTarget)
        m_unitTarget->setcanparry(true);
}

void Spell::SpellEffectBlock(uint8_t /*effectIndex*/)
{
    //i think this actually enables the skill to be able to block melee+ranged attacks
    //value is static and sets value directly which will be modified by other factors
    //  if (m_unitTarget->isPlayer())
    //      m_unitTarget->SetFloatValue(PLAYER_BLOCK_PERCENTAGE,damage);
}

void Spell::SpellEffectCreateItem(uint8_t effectIndex)
{
    uint32_t spellid = m_spellInfo->getId();

    if (m_playerTarget == nullptr)
    {
        sLogger.failure("Spell {} ({}) has a create item effect but no player target!", spellid, m_spellInfo->getName());
        return;
    }


    uint32_t itemid = m_spellInfo->getEffectItemType(effectIndex);
    uint32_t count = 0;
    uint32_t basecount = m_spellInfo->getEffectDieSides(effectIndex);
    uint32_t difference = m_spellInfo->getEffectBasePoints(effectIndex);

    if (itemid == 0)
    {
        sLogger.failure("Spell {} ({}) has a create item effect but no itemid to add, Spell needs to be fixed!", spellid, m_spellInfo->getName());
        return;
    }

    ItemProperties const* m_itemProto = sMySQLStore.getItemProperties(itemid);
    if (m_itemProto == nullptr)
    {
        sLogger.failure("Spell {} ({}) has a create item effect but the itemid is invalid!", spellid, m_spellInfo->getName());
        return;
    }

    if (difference > basecount)
    {
        count = basecount + difference;

    }
    else
    {
        uint32_t mincount = basecount - difference;
        uint32_t maxcount = basecount + difference;
        uint32_t variablecount = maxcount - mincount;
        uint32_t randcount = Util::getRandomUInt(variablecount);

        count = mincount + randcount;
    }

    uint32_t countperlevel = static_cast<uint32_t>(std::round(m_spellInfo->getEffectRealPointsPerLevel(effectIndex)));

    if (countperlevel != 0)
    {
        uint32_t leveldiff = m_spellInfo->getMaxLevel() - m_spellInfo->getBaseLevel();
        uint32_t countforlevel = leveldiff * countperlevel;

        count += countforlevel;
    }

    // Make sure the count is at least 1 (no need to generate warning from this)
    count = std::max(1u, count);

    if (p_caster != nullptr)
    {
        auto skill_line_ability = sSpellMgr.getFirstSkillEntryForSpell(spellid);

        // potions learned by discovery variables
        uint32_t cast_chance = 5;
        uint32_t learn_spell = 0;

        // tailoring specializations get +1 cloth bonus
        switch (spellid)
        {
            case 36686: //Shadowcloth
                if (p_caster->hasSpell(26801)) count++;
                break;

            case 26751: // Primal Mooncloth
                if (p_caster->hasSpell(26798)) count++;
                break;

            case 31373: //Spellcloth
                if (p_caster->hasSpell(26797)) count++;
                break;
        }

        if ((skill_line_ability != nullptr) && (skill_line_ability->skilline == SKILL_ALCHEMY))
        {
            //Potion Master
            if (m_itemProto->Name.compare("Potion"))
            {
                if (p_caster->hasSpell(28675))
                    while (Util::checkChance(20) && (count < 5))
                        count++;

                // Super Rejuvenation Potion
                cast_chance = 2;
                learn_spell = 28586;
            }

            //Elixir Master
            if (m_itemProto->Name.compare("Elixir") || m_itemProto->Name.compare("Flask"))
            {
                if (p_caster->hasSpell(28677))
                    while (Util::checkChance(20) && (count < 5))
                        count++;

                uint32_t spList[] = { 28590, 28587, 28588, 28591, 28589 };
                cast_chance = 2;
                learn_spell = spList[Util::getRandomUInt(4)];
            }

            //Transmutation Master
            if (m_spellInfo->getCategory() == 310)
            {

                //rate for primal might is lower than for anything else
                if (m_spellInfo->getId() == 29688)
                {
                    if (p_caster->hasSpell(28672))
                        while (Util::checkChance(40) && (count < 5))
                            count++;
                }
                else
                {
                    if (p_caster->hasSpell(28672))
                        while (Util::checkChance(20) && (count < 5))
                            count++;
                }

                uint32_t spList[] = { 28581, 28585, 28585, 28584, 28582, 28580 };
                cast_chance = 5;
                learn_spell = spList[Util::getRandomUInt(5)];
            }
        }

        if (!m_playerTarget->getItemInterface()->AddItemById(itemid, count, 0))
        {
            sendCastResult(SPELL_FAILED_TOO_MANY_OF_ITEM);
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

            if ((learn_spell != 0) && (p_caster->getLevel() > 60) && !p_caster->hasSpell(learn_spell) && Util::checkChance(cast_chance))
            {
                SpellInfo const* dspellproto = sSpellMgr.getSpellInfo(learn_spell);

                if (dspellproto != nullptr)
                {
                    p_caster->broadcastMessage("%sDISCOVERY! You discovered the %s !|r", MSG_COLOR_YELLOW, dspellproto->getName().c_str());
                    p_caster->addSpell(learn_spell);
                }
                else
                {
                    sLogger.failure("Spell {} ({}) Effect Index %hhu tried to teach a non-existing Spell {} in {}:{}", spellid, m_spellInfo->getName(), effectIndex, learn_spell, __FILE__, __LINE__);
                }
            }
        }

        if (skill_line_ability != nullptr)
        {
            const auto skillLine = static_cast<uint16_t>(skill_line_ability->skilline);
            DetermineSkillUp(skillLine);

            uint32_t discovered_recipe = 0;

            for (auto itr = sMySQLStore._professionDiscoveryStore.begin(); itr != sMySQLStore._professionDiscoveryStore.end(); ++itr)
            {
                const auto& pf = *itr;
                if (spellid == pf->SpellId && p_caster->getSkillLineCurrent(skillLine) >= pf->SkillValue && !p_caster->hasSpell(pf->SpellToDiscover) && Util::checkChance(pf->Chance))
                {
                    discovered_recipe = pf->SpellToDiscover;
                    break;
                }
            }

            // if something was discovered teach player that recipe and broadcast message
            if (discovered_recipe != 0)
            {
                SpellInfo const* se = sSpellMgr.getSpellInfo(discovered_recipe);

                if (se != nullptr)
                {
                    p_caster->addSpell(discovered_recipe);

                    char msg[256];
                    sprintf(msg, "%sDISCOVERY! %s has discovered how to create %s.|r", MSG_COLOR_GOLD, p_caster->getName().c_str(), se->getName().c_str());

                    p_caster->getWorldMap()->sendChatMessageToCellPlayers(p_caster, SmsgMessageChat(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, 0, msg, p_caster->getGuid()).serialise().get(), 2, 1, LANG_UNIVERSAL, p_caster->getSession());
                }
                else
                {
                    sLogger.failure("Spell {} ({}) Effect index %hhu tried to teach a non-existing Spell {} in {}:{}", spellid, m_spellInfo->getName(), effectIndex, learn_spell, __FILE__, __LINE__);
                }
            }
        }
    }
    else
    {
        if (!m_playerTarget->getItemInterface()->AddItemById(itemid, count, 0))
            sendCastResult(SPELL_FAILED_TOO_MANY_OF_ITEM);
    }
}

void Spell::SpellEffectPersistentAA(uint8_t effectIndex) // Persistent Area Aura
{
    if (m_AreaAura || !m_caster->IsInWorld())
        return;
    //create only 1 dyn object
    uint32_t dur = getDuration();
    float r = getEffectRadius(effectIndex);

    //Note: this code seems to be useless
    //this must be only source point or dest point
    //this AREA aura it's applied on area
    //it can'be on unit or self or item or object
    //uncomment it if I'm wrong
    //We are thinking in general so it might be useful later DK

    // grep: this is a hack!
    // our shitty dynobj system doesn't support GO casters, so we gotta
    // kinda have 2 summoners for traps that apply AA.
    DynamicObject* dynObj = m_caster->getWorldMap()->createDynamicObject();

    if (g_caster != nullptr && g_caster->getUnitOwner() && !m_unitTarget)
    {
        Unit* caster = g_caster->getUnitOwner();
        dynObj->create(caster, this, g_caster->GetPosition(), dur, r, DYNAMIC_OBJECT_AREA_SPELL);
        m_AreaAura = true;
        return;
    }

    switch (m_targets.getTargetMask())
    {
        case TARGET_FLAG_SELF:
        {
            dynObj->create(u_caster, this, m_caster->GetPosition(), dur, r, DYNAMIC_OBJECT_AREA_SPELL);
        }
        break;
        case TARGET_FLAG_UNIT:
        {
            if (!m_unitTarget || !m_unitTarget->isAlive())
            {
                dynObj->remove();
                return;
            }

            dynObj->create(u_caster, this, m_unitTarget->GetPosition(), dur, r, DYNAMIC_OBJECT_AREA_SPELL);
        }
        break;
        case TARGET_FLAG_OBJECT:
        {
            if (!m_unitTarget || !m_unitTarget->isAlive())
            {
                dynObj->remove();
                return;
            }

            dynObj->create(u_caster, this, m_unitTarget->GetPosition(), dur, r, DYNAMIC_OBJECT_AREA_SPELL);
        }
        break;
        case TARGET_FLAG_SOURCE_LOCATION:
        {
            auto source = m_targets.getSource();
            dynObj->create(u_caster, this, source, dur, r, DYNAMIC_OBJECT_AREA_SPELL);
        }
        break;
        case TARGET_FLAG_DEST_LOCATION:
        {
            auto destination = m_targets.getDestination();
            if (u_caster != nullptr)
                dynObj->create(u_caster, this, destination, dur, r, DYNAMIC_OBJECT_AREA_SPELL);
            else if (g_caster != nullptr)
                dynObj->create(g_caster->getUnitOwner(), this, destination, dur, r, DYNAMIC_OBJECT_AREA_SPELL);
        }
        break;
        default:
            dynObj->remove();
            return;
    }

    if (u_caster)
        if (getSpellInfo()->getChannelInterruptFlags() > 0)
        {
            u_caster->setChannelObjectGuid(dynObj->getGuid());
            u_caster->setChannelSpellId(getSpellInfo()->getId());
        }

    m_AreaAura = true;
}

void Spell::SpellEffectSummon(uint8_t effectIndex)
{
    uint32_t summonpropid = m_spellInfo->getEffectMiscValueB(effectIndex);

    auto summon_properties = sSummonPropertiesStore.lookupEntry(summonpropid);
    if (summon_properties == nullptr)
    {
        sLogger.failure("No SummonPropertiesEntry for Spell {} ({})", m_spellInfo->getId(), m_spellInfo->getName());
        return;
    }

    uint32_t entry = m_spellInfo->getEffectMiscValue(effectIndex);

    CreatureProperties const* cp = sMySQLStore.getCreatureProperties(entry);

    if (cp == nullptr)
    {
        sLogger.failure("Spell {} ({}) tried to summon creature {} without database data", m_spellInfo->getId(), m_spellInfo->getName(), entry);
        return;
    }

    LocationVector v(0.0f, 0.0f, 0.0f, 0.0f);

    if ((m_targets.hasDestination()) != 0)
        v = m_targets.getDestination();
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
                spellEffectSummonTotem(effectIndex, summon_properties, cp, v);
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
        case SUMMON_CATEGORY_UNK:
            if (summon_properties->Flags & 512)
            {
                SpellEffectSummonGuardian(effectIndex, summon_properties, cp, v);
                return;
            }
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
            spellEffectSummonTotem(effectIndex, summon_properties, cp, v);
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

    sLogger.failure("Unknown summon type in summon property {} in spell {} {}", summonpropid, m_spellInfo->getId(), m_spellInfo->getName());
}

void Spell::SpellEffectSummonWild(uint8_t effectIndex)  // Summon Wild
{
    //these are some creatures that have your faction and do not respawn
    //number of creatures is actually dmg (the usual formula), sometimes =3 sometimes =1
    //if( u_caster == NULL || !u_caster->IsInWorld() )
    //  return;

    if ((!m_caster->isGameObject() && !m_caster->isCreatureOrPlayer()) || !m_caster->IsInWorld())
        return;

    uint32_t cr_entry = getSpellInfo()->getEffectMiscValue(effectIndex);
    CreatureProperties const* properties = sMySQLStore.getCreatureProperties(cr_entry);
    if (properties == nullptr)
    {
        sLogger.failure("Warning : Missing summon creature template {} used by spell {}!", cr_entry, getSpellInfo()->getId());
        return;
    }
    float x, y, z;
    if (m_targets.hasDestination() && m_targets.getDestination().isSet())
    {
        auto destination = m_targets.getDestination();
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
        float tempx = x + (getEffectRadius(effectIndex) * (cosf(m_fallowAngle + m_caster->GetOrientation())));
        float tempy = y + (getEffectRadius(effectIndex) * (sinf(m_fallowAngle + m_caster->GetOrientation())));

        if (Creature* p = m_caster->getWorldMap()->createCreature(cr_entry))
        {
            p->Load(properties, tempx, tempy, z);
            p->setZoneId(m_caster->getZoneId());

            if (p->GetCreatureProperties()->Faction == 35)
            {
                if (m_caster->isGameObject())
                    p->setFaction(static_cast<GameObject*>(m_caster)->getFactionTemplate());
                else
                    p->setFaction(static_cast<Unit*>(m_caster)->getFactionTemplate());
            }
            else
            {
                p->setFaction(properties->Faction);
            }

            p->setSummonedByGuid(m_caster->getGuid());
            p->setCreatedByGuid(m_caster->getGuid());

            p->PushToWorld(m_caster->getWorldMap());

            // Delay this a bit to make sure its Spawned
            sEventMgr.AddEvent(p, &Creature::InitSummon, m_caster, EVENT_UNK, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

            //make sure they will be desummoned (roxor)
            sEventMgr.AddEvent(p, &Creature::SummonExpire, EVENT_SUMMON_EXPIRE, static_cast<uint32_t>(getDuration()), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }
        else
        {
            sLogger.failure("Spell::SpellEffectSummonWild tried to summon invalid creature {}", cr_entry);
        }
    }
}

void Spell::SpellEffectSummonGuardian(uint32_t /*i*/, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v)
{
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

        Summon* s = u_caster->getWorldMap()->summonCreature(properties_->Id, v, spe, static_cast<uint32_t>(getDuration()), u_caster, getSpellInfo()->getId());
        if (s == nullptr)
            return;

        // Lightwell
        if (spe->Type == SUMMON_TYPE_LIGHTWELL)
        {
            s->setMoveRoot(true);
            s->addNpcFlags(UNIT_NPC_FLAG_SPELLCLICK);
        }
    }
}

void Spell::SpellEffectSummonTemporaryPet(uint32_t i, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v)
{
    if (p_caster == nullptr)
        return;

    if (p_caster->getPet() != nullptr)
        p_caster->getPet()->unSummon();

    int32_t count = 0;

    // Only Inferno uses this SummonProperty ID, and somehow it has the wrong count
    if (spe->ID == 711)
        count = 1;
    else
        count = damage;

    // We know for sure that this will suceed because we checked in Spell::SpellEffectSummon
    CreatureProperties const* ci = sMySQLStore.getCreatureProperties(properties_->Id);

    float angle_for_each_spawn = -M_PI_FLOAT * 2 / damage;

    for (int32_t i = 0; i < count; i++)
    {
        float followangle = angle_for_each_spawn * i;

        float x = 3 * (cosf(followangle + u_caster->GetOrientation()));
        float y = 3 * (sinf(followangle + u_caster->GetOrientation()));

        v.x += x;
        v.y += y;

        const auto pet = sObjectMgr.createPet(properties_->Id, spe);
        if (!pet->createAsSummon(ci, nullptr, p_caster, v, static_cast<uint32_t>(getDuration()), m_spellInfo, i, PET_TYPE_SUMMON))
        {
            pet->DeleteMe();
            break;
        }

        // Delay this a bit to make sure its Spawned
        sEventMgr.AddEvent(pet->ToCreature(), &Creature::InitSummon, m_caster, EVENT_UNK, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Spell::SpellEffectSummonPossessed(uint32_t /*i*/, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v)
{
    if (p_caster == nullptr)
        return;

    if (p_caster->getPet() != nullptr)
        p_caster->getPet()->unSummon();

    v.x += (3 * cos(M_PI_FLOAT / 2 + v.o));
    v.y += (3 * cos(M_PI_FLOAT / 2 + v.o));

    Summon* s = p_caster->getWorldMap()->summonCreature(properties_->Id, v, spe, static_cast<uint32_t>(getDuration()), p_caster, m_spellInfo->getId());
    if (s == nullptr)
        return;

    p_caster->possess(s, 1000);
}

void Spell::SpellEffectSummonCompanion(uint32_t /*i*/, WDB::Structures::SummonPropertiesEntry const* spe, CreatureProperties const* properties_, LocationVector & v)
{
    if (u_caster == nullptr)
        return;

#if VERSION_STRING > TBC
    if (u_caster->getCritterGuid() != 0)
    {
        auto critter = u_caster->getWorldMap()->getUnit(u_caster->getCritterGuid());
        if (critter == nullptr)
            return;

        auto creature = static_cast< Creature* >(critter);

        uint32_t currententry = creature->GetCreatureProperties()->Id;

        creature->RemoveFromWorld(false, true);
        u_caster->setCritterGuid(0);

        // Before WOTLK when you casted the companion summon spell the second time it removed the companion
        // Customized servers or old databases could still use this method
        if (properties_->Id == currententry)
            return;
    }

    auto summon = u_caster->getWorldMap()->summonCreature(properties_->Id, v, spe, static_cast<uint32_t>(getDuration()), u_caster, m_spellInfo->getId());
    if (summon == nullptr)
        return;

    u_caster->setCritterGuid(summon->getGuid());
#endif
}

void Spell::SpellEffectSummonVehicle(uint32_t /*i*/, WDB::Structures::SummonPropertiesEntry const* /*spe*/, CreatureProperties const* properties_, LocationVector& v)
{
    if (u_caster == nullptr)
        return;

    // If it has no vehicle id, then we can't really do anything with it as a vehicle :/
    if ((properties_->vehicleid == 0) && (p_caster == nullptr))
        return;

    Creature* c = u_caster->getWorldMap()->createCreature(properties_->Id);
    c->Load(properties_, v.x, v.y, v.z, v.o);
    c->setPhase(PHASE_SET, u_caster->GetPhase());
    c->setCreatedBySpellId(m_spellInfo->getId());
    c->setCreatedByGuid(u_caster->getGuid());
    c->setSummonedByGuid(u_caster->getGuid());
    c->removeNpcFlags(UNIT_NPC_FLAG_SPELLCLICK);
    c->PushToWorld(u_caster->getWorldMap());

    // Delay this a bit to make sure its Spawned
    sEventMgr.AddEvent(c->ToCreature(), &Creature::InitSummon, m_caster, EVENT_UNK, 100, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

#ifdef FT_VEHICLES
    // Need to delay this a bit since first the client needs to see the vehicle
    u_caster->callEnterVehicle(c);
#endif
}

void Spell::SpellEffectLeap(uint8_t effectIndex) // Leap
{
    if (m_unitTarget == nullptr)
        return;

    float radius = getEffectRadius(effectIndex);
    m_unitTarget->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_ANY_DAMAGE_TAKEN);

    MMAP::MMapManager* mmap = MMAP::MMapFactory::createOrGetMMapManager();
    dtNavMesh* nav = const_cast<dtNavMesh*>(mmap->GetNavMesh(m_caster->GetMapId()));
    // dtNavMeshQuery* nav_query = const_cast<dtNavMeshQuery*>(mmap->GetNavMeshQuery(m_caster->GetMapId(), m_caster->GetInstanceID()));
    //NavMeshData* nav = CollideInterface.GetNavMesh(m_caster->GetMapId());

    if (nav != nullptr)
    {
        float destx, desty, destz;
        m_unitTarget->GetPoint(m_unitTarget->GetOrientation(), radius, destx, desty, destz);
        if (m_playerTarget != nullptr)
            m_playerTarget->safeTeleport(m_playerTarget->GetMapId(), m_playerTarget->GetInstanceID(), LocationVector(destx, desty, destz, m_playerTarget->GetOrientation()));
        else
        {
            float speedXY, speedZ;
            calculateJumpSpeeds(u_caster, getSpellInfo(), effectIndex, u_caster->getExactDist2d(m_unitTarget->GetPositionX(), m_unitTarget->GetPositionY()), speedXY, speedZ);
            u_caster->getMovementManager()->moveJump(destx, desty, destz, m_unitTarget->GetOrientation(), speedXY, speedZ, EVENT_JUMP, !m_unitTarget);
        }
    }
    else
    {
        if (m_playerTarget == nullptr)  //let client handle this for players
            return;

        m_playerTarget->getSession()->SendPacket(SmsgMoveKnockBack(m_playerTarget->GetNewGUID(), Util::getMSTime(), cosf(m_playerTarget->GetOrientation()), sinf(m_playerTarget->GetOrientation()), radius, -10.0f).serialise().get());
    }
}

void Spell::SpellEffectEnergize(uint8_t effectIndex) // Energize
{
    if (!m_unitTarget || !m_unitTarget->isAlive())
        return;

    uint32_t modEnergy = 0;
    switch (getSpellInfo()->getId())
    {
        case 30824: // Shamanistic Rage
            modEnergy = damage * getUnitTarget()->getCalculatedAttackPower() / 100;
            break;
        case 31786: // Paladin - Spiritual Attunement
            if (ProcedOnSpell)
            {
                SpellInfo const* motherspell = sSpellMgr.getSpellInfo(pSpellId);
                if (motherspell)
                {
                    //heal amount from procspell (we only proceed on a heal spell)
                    uint32_t healamt = 0;
                    if (ProcedOnSpell->getEffect(0) == SPELL_EFFECT_HEAL || ProcedOnSpell->getEffect(0) == SPELL_EFFECT_SCRIPT_EFFECT)
                        healamt = ProcedOnSpell->calculateEffectValue(0);
                    else if (ProcedOnSpell->getEffect(1) == SPELL_EFFECT_HEAL || ProcedOnSpell->getEffect(1) == SPELL_EFFECT_SCRIPT_EFFECT)
                        healamt = ProcedOnSpell->calculateEffectValue(1);
                    else if (ProcedOnSpell->getEffect(2) == SPELL_EFFECT_HEAL || ProcedOnSpell->getEffect(2) == SPELL_EFFECT_SCRIPT_EFFECT)
                        healamt = ProcedOnSpell->calculateEffectValue(2);
                    modEnergy = (motherspell->calculateEffectValue(0)) * (healamt) / 100;
                }
            }
            break;
        case 57669:
        {
            modEnergy = Util::float2int32(0.01f * m_unitTarget->getBaseMana());
        }
        break;
        case 31930:
        {
            if (u_caster)
                modEnergy = Util::float2int32(0.25f * m_unitTarget->getBaseMana());
        }
        break;
        case 2687: // Improved Bloodrage, dirty fix
        {
            modEnergy = damage;
            if (p_caster)
            {
                if (p_caster->hasSpell(12818))
                    modEnergy += 110; //60
                if (p_caster->hasSpell(12301))
                    modEnergy += 60; //30
            }
        }
        break;
        default:
            modEnergy = damage;
            break;
    }

    if (m_unitTarget->hasAurasWithId(17619))
        modEnergy = uint32_t(modEnergy * 1.4f);

    if (u_caster)
        u_caster->energize(m_unitTarget, getSpellInfo()->getId(), modEnergy, static_cast<PowerType>(getSpellInfo()->getEffectMiscValue(effectIndex)));
}

void Spell::SpellEffectWeaponDmgPerc(uint8_t effectIndex) // Weapon Percent damage
{
    if (!m_unitTarget || !u_caster) return;

    if (GetType() == SPELL_DMG_TYPE_MAGIC)
    {
        auto dmg = CalculateDamage(u_caster, m_unitTarget, MELEE, nullptr, getSpellInfo()) * damage / 100.0f;

        // Get bonus damage from spell power and attack power
        if (!isEffectDamageStatic[effectIndex])
            dmg = getUnitCaster()->applySpellDamageBonus(getSpellInfo(), static_cast<int32_t>(dmg), effectPctModifier[effectIndex], false, this);

        m_targetDamageInfo = u_caster->doSpellDamage(m_unitTarget, getSpellInfo()->getId(), dmg, effectIndex, m_triggeredSpell, false, false, isForcedCrit, this);
        isTargetDamageInfoSet = true;
    }
    else
    {
        WeaponDamageType _type;
        if (GetType() == SPELL_DMG_TYPE_RANGED)
            _type = RANGED;
        else
        {
            if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_REQUIRES_OFFHAND_WEAPON)
                _type = OFFHAND;
            else
                _type = MELEE;
        }

        m_targetDamageInfo = u_caster->strike(m_unitTarget, _type, getSpellInfo(), add_damage, damage, 0, m_triggeredSpell, true, isForcedCrit, this);
        isTargetDamageInfoSet = true;

        if (p_caster != nullptr)   // rogue - fan of knives
        {
            switch (getSpellInfo()->getId())
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
                    Item* oit = p_caster->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
                    if (oit != nullptr)
                    {
                        if (oit->getDurability() != 0)
                        {
                            if (oit->getItemProperties()->Class == 2 && oit->getItemProperties()->SubClass == 15)   // daggers
                                damage = 105; //causing 105% weapon damage with daggers
                            else
                                damage = getSpellInfo()->getEffectBasePoints(effectIndex) + 1;// and 70% weapon damage with all other weapons.

                            u_caster->strike(m_unitTarget, OFFHAND, getSpellInfo(), add_damage, damage, 0, m_triggeredSpell, true, isForcedCrit, this);
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

    uint32_t spellid = getSpellInfo()->getEffectTriggerSpell(effectIndex);
    if (spellid == 0)
    {
        sLogger.failure("Spell {} ({}) has a trigger missle effect index (%hhu) but no trigger spell ID. Spell needs fixing.", m_spellInfo->getId(), m_spellInfo->getName(), effectIndex);
        return;
    }

    SpellInfo const* spInfo = sSpellMgr.getSpellInfo(spellid);
    if (spInfo == nullptr)
    {
        sLogger.failure("Spell {} ({}) has a trigger missle effect index (%hhu) but has an invalid trigger spell ID. Spell needs fixing.", m_spellInfo->getId(), m_spellInfo->getName(), effectIndex);
        return;
    }

    // Cast the triggered spell on the destination location, spells like Freezing Arrow use it
    if ((u_caster != nullptr) && (m_targets.hasDestination()))
    {
        u_caster->castSpellLoc(m_targets.getDestination(), spInfo, true);
        return;
    }

    float spellRadius = getEffectRadius(effectIndex);

    //\todo Following should be / is probably in SpellTarget code
    for (const auto& itr : m_caster->getInRangeObjectsSet())
    {
        if (!itr || !itr->isCreatureOrPlayer() || !static_cast<Unit*>(itr)->isAlive())
            continue;

        Unit* t = static_cast<Unit*>(itr);

        float r;
        auto destination = m_targets.getDestination();
        float d = destination.x - t->GetPositionX();
        r = d * d;
        d = destination.y - t->GetPositionY();
        r += d * d;
        d = destination.z - t->GetPositionZ();
        r += d * d;

        if (std::sqrt(r) > spellRadius) continue;

        if (!m_caster->isValidTarget(itr))   //Fix Me: only enemy targets?
            continue;

        Spell* sp = sSpellMgr.newSpell(m_caster, spInfo, true, nullptr);
        SpellCastTargets tgt(itr->getGuid());
        sp->prepare(&tgt);
    }
}

void Spell::SpellEffectOpenLock(uint8_t effectIndex)
{
    if (!p_caster)
        return;

    uint8_t loottype = 0;

    uint32_t locktype = getSpellInfo()->getEffectMiscValue(effectIndex);
    switch (locktype)
    {
#if VERSION_STRING <= WotLK
        case LOCKTYPE_PICKLOCK:
        {
            uint32_t v = 0;
            uint32_t lockskill = p_caster->getSkillLineCurrent(SKILL_LOCKPICKING);

            if (m_itemTarget)
            {
                if (!m_itemTarget->m_isLocked)
                    return;

                auto lock = sLockStore.lookupEntry(m_itemTarget->getItemProperties()->LockId);
                if (!lock)
                    return;

                for (uint8_t j = 0; j < LOCK_NUM_CASES; ++j)
                {
                    if (lock->locktype[j] == 2 && lock->minlockskill[j] && lockskill >= lock->minlockskill[j])
                    {
                        v = lock->minlockskill[j];
                        m_itemTarget->m_isLocked = false;
                        m_itemTarget->addFlags(ITEM_FLAG_LOOTABLE);
                        DetermineSkillUp(SKILL_LOCKPICKING, v / 5);
                        break;
                    }
                }
            }
            else if (m_gameObjTarget)
            {
                auto gameobject_info = m_gameObjTarget->GetGameObjectProperties();
                if (m_gameObjTarget->getState() == 0)
                    return;

                auto lock = sLockStore.lookupEntry(gameobject_info->raw.parameter_0);
                if (lock == nullptr)
                    return;

                for (uint8_t j = 0; j < LOCK_NUM_CASES; ++j)
                {
                    if (lock->locktype[j] == 2 && lock->minlockskill[j] && lockskill >= lock->minlockskill[j])
                    {
                        v = lock->minlockskill[j];
                        m_gameObjTarget->setFlags(GO_FLAG_NONE);
                        m_gameObjTarget->setState(GO_STATE_CLOSED);
                        //Add Fill GO loot here
                        if (m_gameObjTarget->IsLootable())
                        {
                            GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(m_gameObjTarget);
                            if (pLGO->loot.items.size() == 0)
                            {
                                if (m_gameObjTarget->getWorldMap() != nullptr)
                                    sLootMgr.fillGOLoot(p_caster, &pLGO->loot, m_gameObjTarget->GetGameObjectProperties()->raw.parameter_1, m_gameObjTarget->getWorldMap()->getDifficulty());
                                else
                                    sLootMgr.fillGOLoot(p_caster, &pLGO->loot, m_gameObjTarget->GetGameObjectProperties()->raw.parameter_1, 0);

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
#endif
        case LOCKTYPE_HERBALISM:
        {
            if (!m_gameObjTarget) return;

            uint32_t v = m_gameObjTarget->GetGOReqSkill();
            bool bAlreadyUsed = false;

            if (static_cast<Player*>(m_caster)->getSkillLineCurrent(SKILL_HERBALISM) < v)
            {
                //sendCastResult(SPELL_FAILED_LOW_CASTLEVEL);
                return;
            }
            else
            {
                if (m_gameObjTarget->IsLootable())
                {
                    GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(m_gameObjTarget);

                    if (pLGO->loot.items.size() == 0)
                    {
                        if (m_gameObjTarget->getWorldMap() != nullptr)
                            sLootMgr.fillGOLoot(p_caster, &pLGO->loot, m_gameObjTarget->GetGameObjectProperties()->raw.parameter_1, m_gameObjTarget->getWorldMap()->getDifficulty());
                        else
                            sLootMgr.fillGOLoot(p_caster, &pLGO->loot, m_gameObjTarget->GetGameObjectProperties()->raw.parameter_1, 0);
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
            if (!m_gameObjTarget)
                return;

            uint32_t v = m_gameObjTarget->GetGOReqSkill();
            bool bAlreadyUsed = false;

            if (static_cast<Player*>(m_caster)->getSkillLineCurrent(SKILL_MINING) < v)
            {
                //sendCastResult(SPELL_FAILED_LOW_CASTLEVEL);
                return;
            }
            else if (m_gameObjTarget->IsLootable())
            {
                GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(m_gameObjTarget);
                if (pLGO->loot.items.size() == 0)
                {
                    if (m_gameObjTarget->getWorldMap() != nullptr)
                        sLootMgr.fillGOLoot(p_caster, &pLGO->loot, m_gameObjTarget->GetGameObjectProperties()->raw.parameter_1, m_gameObjTarget->getWorldMap()->getDifficulty());
                    else
                        sLootMgr.fillGOLoot(p_caster, &pLGO->loot, m_gameObjTarget->GetGameObjectProperties()->raw.parameter_1, 0);
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
            if (!m_gameObjTarget) return;
            if (p_caster && p_caster->getBattleground())
                if (p_caster->getBattleground()->HookSlowLockOpen(m_gameObjTarget, p_caster, this))
                    return;

            uint32_t spellid = !m_gameObjTarget->GetGameObjectProperties()->raw.parameter_10 ? 23932 : m_gameObjTarget->GetGameObjectProperties()->raw.parameter_10;
            SpellInfo const* en = sSpellMgr.getSpellInfo(spellid);
            Spell* sp = sSpellMgr.newSpell(p_caster, en, true, nullptr);
            SpellCastTargets tgt;
            tgt.setGameObjectTarget(m_gameObjTarget->getGuid());
            sp->prepare(&tgt);
            return;
        }
        break;
        case LOCKTYPE_QUICK_CLOSE:
        {
            if (m_gameObjTarget == nullptr)
                return;

            m_gameObjTarget->Use(m_caster->getGuid());
        }
        break;

        case LOCKTYPE_QUICK_OPEN:
            if (m_gameObjTarget == nullptr)
                return;

            if ((p_caster != nullptr) && (p_caster->getBattleground() != nullptr))
                p_caster->getBattleground()->HookQuickLockOpen(m_gameObjTarget, p_caster, this);

            // there is no break here on purpose

        default://not profession
        {
            if (m_gameObjTarget == nullptr)
                return;

            m_gameObjTarget->onUse(m_caster->ToPlayer());
            m_gameObjTarget->Use(m_caster->getGuid());

            if (m_gameObjTarget->GetScript())
                m_gameObjTarget->GetScript()->OnActivate(p_caster);

            if (m_gameObjTarget->getWorldMap() && m_gameObjTarget->getWorldMap()->getScript())
                m_gameObjTarget->getWorldMap()->getScript()->OnGameObjectActivate(m_gameObjTarget, p_caster);

            if (sQuestMgr.OnActivateQuestGiver(m_gameObjTarget, p_caster))
                return;

            if (sQuestMgr.OnGameObjectActivate(p_caster, m_gameObjTarget))
            {
                p_caster->updateNearbyQuestGameObjects();
                return;
            }

            if (m_gameObjTarget->IsLootable())
            {
                GameObject_Lootable* pLGO = static_cast<GameObject_Lootable*>(m_gameObjTarget);

                if (pLGO->loot.items.size() == 0)
                {
                    if (m_gameObjTarget->getWorldMap() != nullptr)
                        sLootMgr.fillGOLoot(p_caster, &pLGO->loot, m_gameObjTarget->GetGameObjectProperties()->raw.parameter_1, m_gameObjTarget->getWorldMap()->getDifficulty());
                    else
                        sLootMgr.fillGOLoot(p_caster, &pLGO->loot, m_gameObjTarget->GetGameObjectProperties()->raw.parameter_1, 0);
                }
            }
            loottype = LOOT_CORPSE;
        }
        break;
    };
    if (m_gameObjTarget && m_gameObjTarget->getGoType() == GAMEOBJECT_TYPE_CHEST)
        static_cast< Player* >(m_caster)->sendLoot(m_gameObjTarget->getGuid(), loottype, m_gameObjTarget->GetMapId());
}

void Spell::SpellEffectTransformItem(uint8_t effectIndex)
{
    bool result;

    if (!i_caster) return;
    uint32_t itemid = getSpellInfo()->getEffectItemType(effectIndex);
    if (!itemid) return;

    //Save durability of the old item
    Player* owner = i_caster->getOwner();
    uint32_t dur = i_caster->getDurability();

    result = owner->getItemInterface()->SafeFullRemoveItemByGuid(i_caster->getGuid());
    if (!result)
    {
        //something went wrong if this happen, item doesn't exist, so it wasn't destroyed.
        return;
    }

    i_caster = nullptr;

    auto it = sObjectMgr.createItem(itemid, owner);
    if (!it) return;

    it->setDurability(dur);
    //additem

    //additem
    const auto [result2, _] = owner->getItemInterface()->AddItemToFreeSlot(std::move(it));
    if (!result2) //should never get here
    {
        owner->getItemInterface()->buildInventoryChangeError(nullptr, nullptr, INV_ERR_BAG_FULL);
    }
}

void Spell::SpellEffectApplyGroupAA(uint8_t effectIndex)
{
    ApplyAreaAura(effectIndex);
}

void Spell::SpellEffectLearnSpell(uint8_t effectIndex) // Learn Spell
{
    if (m_playerTarget == nullptr && m_unitTarget && m_unitTarget->isPet()) // something's wrong with this logic here.
    {
        // bug in target map fill?
        //playerTarget = m_caster->getWorldMap()->GetPlayer((uint32_t)m_targets.getUnitTarget());
        SpellEffectLearnPetSpell(effectIndex);
        return;
    }

    if (getSpellInfo()->getId() == 483 || getSpellInfo()->getId() == 55884)          // "Learning"
    {
        if (!p_caster) return;

        uint32_t spellid = damage;
        if (!spellid || !sSpellMgr.getSpellInfo(spellid)) return;

        // learn me!
        p_caster->addSpell(spellid);

        // no normal handler
        return;
    }

    if (m_playerTarget)
    {
        /*if (u_caster && playerTarget->isHostileTo(u_caster))
        return;*/

        uint32_t spellToLearn = getSpellInfo()->getEffectTriggerSpell(effectIndex);
        m_playerTarget->addSpell(spellToLearn);

        if (spellToLearn == 2575)   //hacky fix for mining from creatures
            m_playerTarget->addSpell(32606);

        if (spellToLearn == 2366)   //hacky fix for herbalism from creatures
            m_playerTarget->addSpell(32605);

        //smth is wrong here, first we add this spell to player then we may cast it on player...
        SpellInfo const* spellinfo = sSpellMgr.getSpellInfo(spellToLearn);
        //remove specializations
        switch (spellinfo->getId())
        {
            case 26801: //Shadoweave Tailoring
                m_playerTarget->removeSpell(26798, false); //Mooncloth Tailoring
                m_playerTarget->removeSpell(26797, false); //Spellfire Tailoring
                break;
            case 26798: // Mooncloth Tailoring
                m_playerTarget->removeSpell(26801, false); //Shadoweave Tailoring
                m_playerTarget->removeSpell(26797, false); //Spellfire Tailoring
                break;
            case 26797: //Spellfire Tailoring
                m_playerTarget->removeSpell(26801, false); //Shadoweave Tailoring
                m_playerTarget->removeSpell(26798, false); //Mooncloth Tailoring
                break;
            case 10656: //Dragonscale Leatherworking
                m_playerTarget->removeSpell(10658, false); //Elemental Leatherworking
                m_playerTarget->removeSpell(10660, false); //Tribal Leatherworking
                break;
            case 10658: //Elemental Leatherworking
                m_playerTarget->removeSpell(10656, false); //Dragonscale Leatherworking
                m_playerTarget->removeSpell(10660, false); //Tribal Leatherworking
                break;
            case 10660: //Tribal Leatherworking
                m_playerTarget->removeSpell(10656, false); //Dragonscale Leatherworking
                m_playerTarget->removeSpell(10658, false); //Elemental Leatherworking
                break;
            case 28677: //Elixir Master
                m_playerTarget->removeSpell(28675, false); //Potion Master
                m_playerTarget->removeSpell(28672, false); //Transmutation Maste
                break;
            case 28675: //Potion Master
                m_playerTarget->removeSpell(28677, false); //Elixir Master
                m_playerTarget->removeSpell(28672, false); //Transmutation Maste
                break;
            case 28672: //Transmutation Master
                m_playerTarget->removeSpell(28675, false); //Potion Master
                m_playerTarget->removeSpell(28677, false); //Elixir Master
                break;
            case 20219: //Gnomish Engineer
                m_playerTarget->removeSpell(20222, false); //Goblin Engineer
                break;
            case 20222: //Goblin Engineer
                m_playerTarget->removeSpell(20219, false); //Gnomish Engineer
                break;
            case 9788: //Armorsmith
                m_playerTarget->removeSpell(9787, false); //Weaponsmith
                m_playerTarget->removeSpell(17039, false); //Master Swordsmith
                m_playerTarget->removeSpell(17040, false); //Master Hammersmith
                m_playerTarget->removeSpell(17041, false); //Master Axesmith
                break;
            case 9787: //Weaponsmith
                m_playerTarget->removeSpell(9788, false); //Armorsmith
                break;
            case 17041: //Master Axesmith
                m_playerTarget->removeSpell(9788, false); //Armorsmith
                m_playerTarget->removeSpell(17040, false); //Master Hammersmith
                m_playerTarget->removeSpell(17039, false); //Master Swordsmith
                break;
            case 17040: //Master Hammersmith
                m_playerTarget->removeSpell(9788, false); //Armorsmith
                m_playerTarget->removeSpell(17039, false); //Master Swordsmith
                m_playerTarget->removeSpell(17041, false); //Master Axesmith
                break;
            case 17039: //Master Swordsmith
                m_playerTarget->removeSpell(9788, false); //Armorsmith
                m_playerTarget->removeSpell(17040, false); //Master Hammersmith
                m_playerTarget->removeSpell(17041, false); //Master Axesmith
                break;
        }
        for (uint8_t j = 0; j < 3; j++)
            if (spellinfo->getEffect(j) == SPELL_EFFECT_WEAPON ||
                spellinfo->getEffect(j) == SPELL_EFFECT_PROFICIENCY ||
                spellinfo->getEffect(j) == SPELL_EFFECT_DUAL_WIELD)
            {
                Spell* sp = sSpellMgr.newSpell(m_unitTarget, spellinfo, true, nullptr);
                SpellCastTargets targets(m_unitTarget->getGuid());
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
    if (u_caster == nullptr || m_unitTarget == nullptr)
        return;

    uint16_t start, end;

    if (u_caster->isValidTarget(m_unitTarget) || getSpellInfo()->getEffectMiscValue(effectIndex) == DISPEL_STEALTH)    // IsAttackable returns false for stealthed
    {
        start = AuraSlots::POSITIVE_SLOT_START;
        end = AuraSlots::POSITIVE_SLOT_END;
        if (m_unitTarget->m_schoolImmunityList[getSpellInfo()->getFirstSchoolFromSchoolMask()])
            return;
    }
    else
    {
        start = AuraSlots::NEGATIVE_SLOT_START;
        end = AuraSlots::NEGATIVE_SLOT_END;
    }

    SpellInfo const* aursp;
    std::list< uint32_t > dispelledSpells;
    bool finish = false;

    for (uint16_t x = start; x < end; x++)
    {
        if (auto* const aur = m_unitTarget->getAuraWithAuraSlot(x))
        {
            bool AuraRemoved = false;
            aursp = aur->getSpellInfo();

            //Nothing can dispel resurrection sickness;
            if (!aur->IsPassive() && !(aursp->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY))
            {
                if (getSpellInfo()->getDispelType() == DISPEL_ALL)
                {
                    dispelledSpells.push_back(aursp->getId());

                    aur->removeAura(AURA_REMOVE_ON_DISPEL);
                    AuraRemoved = true;

                    if (--damage <= 0)
                        finish = true;
                }
                else if (static_cast<int32_t>(aursp->getDispelType()) == getSpellInfo()->getEffectMiscValue(effectIndex))
                {
                    dispelledSpells.push_back(aursp->getId());

                    aur->removeAura(AURA_REMOVE_ON_DISPEL);
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
                            const auto spellInfo = sSpellMgr.getSpellInfo(31117);
                            Spell* spell = sSpellMgr.newSpell(u_caster, spellInfo, true, nullptr);
                            spell->forced_basepoints->set(0, (aursp->calculateEffectValue(0)) * 9);   //damage effect
                            spell->ProcedOnSpell = getSpellInfo();
                            spell->pSpellId = aursp->getId();
                            SpellCastTargets targets(u_caster->getGuid());
                            spell->prepare(&targets);
                        } break;
                    }
                }
            }
            if (finish)
                break;
        }
    }

    // send spell dispell log packet
    if (!dispelledSpells.empty())
    {
        m_caster->sendMessageToSet(SmsgSpellDispellLog(m_caster->getGuid(), m_unitTarget->getGuid(), getSpellInfo()->getId(), dispelledSpells).serialise().get(), true);
    }
}

void Spell::SpellEffectAddHonor(uint8_t effectIndex)
{
    if (!m_playerTarget) return;

    uint32_t val = getSpellInfo()->getEffectBasePoints(effectIndex);

    // TODO: is this correct? -Appled
    if (getSpellInfo()->getAttributesExB() & ATTRIBUTESEXB_IGNORE_LINE_OF_SIGHT) val /= 10;

    val += 1;

    HonorHandler::AddHonorPointsToPlayer(m_playerTarget, val);

    m_playerTarget->sendPvpCredit(val, 0, 5);
}

void Spell::SpellEffectSpawn(uint8_t /*effectIndex*/)
{
    // this effect is mostly for custom teleporting
    switch (getSpellInfo()->getId())
    {
        case 10418: // Arugal spawn-in spell , teleports it to 3 locations randomly sneaking players (bastard ;P)
        {
            //only Creature can cast this spell
            if (u_caster == nullptr || p_caster != nullptr)
                return;

            //static float coord[3][3] = { { -108.9034f, 2129.5678f, 144.9210f }, { -108.9034f, 2155.5678f, 155.678f }, { -77.9034f, 2155.5678f, 155.678f } };

            // uint8_t j = RandomUInt(3);
            //u_caster->getAIInterface()->SendMoveToPacket(coord[j][0],coord[j][1],coord[j][2],0.0f,0,u_caster->getAIInterface()->getMoveFlags());
        }
    }
}

void Spell::SpellEffectSummonObject(uint8_t effectIndex)
{
    if (!u_caster)
        return;

    uint32_t entry = getSpellInfo()->getEffectMiscValue(effectIndex);

    GameObjectProperties const* info = sMySQLStore.getGameObjectProperties(entry);
    if (info == nullptr)
    {
        sLogger.failure("Spell {} ( {} ) Effect Index {} tried to summon a GameObject with ID {}. GameObject is not in the database.", m_spellInfo->getId(), m_spellInfo->getName(), effectIndex, entry);
        return;
    }

    WorldMap* map = m_caster->getWorldMap();
    float px = u_caster->GetPositionX();
    float py = u_caster->GetPositionY();
    float pz = u_caster->GetPositionZ();

    int32_t duration = getDuration();
    GameObject* go = nullptr;

    if (info->type == GAMEOBJECT_TYPE_FISHINGNODE)
    {
        if (p_caster == nullptr)
            return;

        float minDist = m_spellInfo->getMinRange(true);
        float maxDist = m_spellInfo->getMaxRange(true);
        float posx = 0, posy = 0, posz = 0;
        float dist = Util::getRandomFloat(minDist, maxDist);

        float angle = Util::getRandomFloat(0.0f, 1.0f) * static_cast<float>(M_PI * 35.0f / 180.0f) - static_cast<float>(M_PI * 17.5f / 180.0f);
        m_caster->getClosePoint(posx, posy, posz, 0.388999998569489f, dist, angle);

        float liquidLevel = VMAP_INVALID_HEIGHT_VALUE;

        LiquidData liquidData;
        if (map->getLiquidStatus(m_caster->GetPhase(), LocationVector(posx, posy, posz), MAP_ALL_LIQUIDS, &liquidData, m_caster->getCollisionHeight()))
            liquidLevel = liquidData.level;

        go = u_caster->getWorldMap()->createGameObject(entry);

        LocationVector pos = { posx, posy, liquidLevel, u_caster->GetOrientation() };
        QuaternionData rot = QuaternionData::fromEulerAnglesZYX(u_caster->GetOrientation(), 0.f, 0.f);

        if (!go->create(entry, map, p_caster->GetPhase(), pos, rot, GO_STATE_CLOSED))
        {
            delete go;
            return;
        }

        go->setGoType(GAMEOBJECT_TYPE_FISHINGNODE);
        go->setCreatedByGuid(m_caster->getGuid());
        u_caster->addGameObject(go);

        go->PushToWorld(m_caster->getWorldMap());

        u_caster->setChannelObjectGuid(go->getGuid());
    }
    else
    {
        float posx = px;
        float posy = py;
        auto destination = m_targets.getDestination();

        if ((m_targets.hasDestination()) && destination.isSet())
        {
            posx = destination.x;
            posy = destination.y;
            pz = destination.z;
        }

        LocationVector pos = { posx, posy, pz, u_caster->GetOrientation() };
        QuaternionData rot = QuaternionData::fromEulerAnglesZYX(u_caster->GetOrientation(), 0.f, 0.f);
        go = u_caster->getWorldMap()->createGameObject(entry);
        if (!go->create(entry, map, u_caster->GetPhase(), pos, rot, GO_STATE_CLOSED))
        {
            delete go;
            return;
        }

        go->setCreatedByGuid(m_caster->getGuid());
        u_caster->addGameObject(go);
        go->PushToWorld(m_caster->getWorldMap());
    }

    switch (info->type)
    {
        case GAMEOBJECT_TYPE_FISHINGNODE:
        {
            go->setCreatedByGuid(m_caster->getGuid());
            u_caster->addGameObject(go);
            u_caster->setChannelObjectGuid(go->getGuid());

            int32_t lastSec = 0;
            switch (Util::getRandomUInt(0, 2))
            {
                case 0: lastSec = 3; break;
                case 1: lastSec = 7; break;
                case 2: lastSec = 13; break;
            }

            // Duration of the fishing bobber can't be higher than the Fishing channeling duration
            duration = std::min(duration, (duration - lastSec * IN_MILLISECONDS + 5 * IN_MILLISECONDS));
        } break;
        case GAMEOBJECT_TYPE_RITUAL:
        {
            if (u_caster->isPlayer())
            {
                go->setCreatedByGuid(m_caster->getGuid());
                u_caster->addGameObject(go);

                GameObject_Ritual* go_ritual = static_cast<GameObject_Ritual*>(go);

                go_ritual->GetRitual()->Setup(p_caster->getGuidLow(), 0, m_spellInfo->getId());
                go_ritual->GetRitual()->Setup(p_caster->getGuidLow(), static_cast<uint32_t>(p_caster->getTargetGuid()), m_spellInfo->getId());
            }
        } break;
        case GAMEOBJECT_TYPE_DUEL_ARBITER: // 52991
        {
            go->setCreatedByGuid(m_caster->getGuid());
            u_caster->addGameObject(go);
        } break;
        case GAMEOBJECT_TYPE_FISHINGHOLE:
        case GAMEOBJECT_TYPE_CHEST:
        default:
            break;
    }

    go->setRespawnTime(duration > 0 ? duration / IN_MILLISECONDS : 0);
    go->setSpellId(m_spellInfo->getId());

    if (GameObject* linkedTrap = go->getLinkedTrap())
    {
        linkedTrap->setRespawnTime(duration > 0 ? duration / IN_MILLISECONDS : 0);
        linkedTrap->setSpellId(m_spellInfo->getId());
    }

    if (p_caster != nullptr)
        p_caster->setSummonedObject(go);
}

void Spell::SpellEffectEnchantItem(uint8_t effectIndex) // Enchant Item Permanent
{
    if (!m_itemTarget || !p_caster)
        return;

    // Vellums
    if (getSpellInfo()->getEffectItemType(effectIndex) && (m_itemTarget->getEntry() == 39349 ||
        m_itemTarget->getEntry() == 39350 || m_itemTarget->getEntry() == 43146 ||
        m_itemTarget->getEntry() == 38682 || m_itemTarget->getEntry() == 37602 ||
        m_itemTarget->getEntry() == 43145))
    {
        uint32_t itemid = getSpellInfo()->getEffectItemType(effectIndex);
        ItemProperties const* it = sMySQLStore.getItemProperties(itemid);
        if (it == nullptr)
        {
            p_caster->getSession()->SystemMessage("Item is missing, report this to devs. Entry: %u", itemid);
            return;
        }

        auto pItem = sObjectMgr.createItem(itemid, p_caster);
        if (pItem == nullptr)
            return;

        p_caster->getItemInterface()->RemoveItemAmt(m_itemTarget->getEntry(), 1);
        p_caster->getItemInterface()->AddItemToFreeSlot(std::move(pItem));

        return;
    }

    auto spell_item_enchant = sSpellItemEnchantmentStore.lookupEntry(getSpellInfo()->getEffectMiscValue(effectIndex));

    if (!spell_item_enchant)
    {
        sLogger.failure("Invalid enchantment entry {} for Spell {}", getSpellInfo()->getEffectMiscValue(effectIndex), getSpellInfo()->getId());
        return;
    }

    if (p_caster->getSession()->hasPermissions())
        sGMLog.writefromsession(p_caster->getSession(), "enchanted item for %s", m_itemTarget->getOwner()->getName().c_str());

    //remove other perm enchantment that was enchanted by profession
    m_itemTarget->removeEnchantment(PERM_ENCHANTMENT_SLOT);
    const auto addedEnchantment = m_itemTarget->addEnchantment(getSpellInfo()->getEffectMiscValue(effectIndex), PERM_ENCHANTMENT_SLOT, 0);
    if (!addedEnchantment)
        return; // Apply failed

    if (!i_caster)
        DetermineSkillUp();
}

void Spell::SpellEffectEnchantItemTemporary(uint8_t effectIndex)  // Enchant Item Temporary
{
    if ((m_itemTarget == nullptr) || (p_caster == nullptr))
        return;

    uint32_t Duration = m_spellInfo->getEffectBasePoints(effectIndex);
    uint32_t EnchantmentID = m_spellInfo->getEffectMiscValue(effectIndex);

    // don't allow temporary enchants unless we're the owner of the item
    if (m_itemTarget->getOwner() != p_caster)
        return;

    if (Duration == 0)
    {
        sLogger.failure("Spell {} ({}) has no enchantment duration. Spell needs to be fixed!", m_spellInfo->getId(), m_spellInfo->getName());
        return;
    }

    if (EnchantmentID == 0)
    {
        sLogger.failure("Spell {} ({}) has no enchantment ID. Spell needs to be fixed!", m_spellInfo->getId(), m_spellInfo->getName());
        return;
    }

    auto spell_item_enchant = sSpellItemEnchantmentStore.lookupEntry(EnchantmentID);
    if (spell_item_enchant == nullptr)
    {
        sLogger.failure("Invalid enchantment entry {} for Spell {}", EnchantmentID, getSpellInfo()->getId());
        return;
    }

    m_itemTarget->removeEnchantment(TEMP_ENCHANTMENT_SLOT);

    const auto addedEnchantment = m_itemTarget->addEnchantment(EnchantmentID, TEMP_ENCHANTMENT_SLOT, Duration * 1000);
    if (!addedEnchantment)
        return; // Apply failed

    auto skill_line_ability = sSpellMgr.getFirstSkillEntryForSpell(getSpellInfo()->getId());
    if (skill_line_ability != nullptr)
        DetermineSkillUp(static_cast<uint16_t>(skill_line_ability->skilline), m_itemTarget->getItemProperties()->ItemLevel);
}

void Spell::SpellEffectTameCreature(uint8_t effectIndex)
{
    if (m_unitTarget == nullptr || !m_unitTarget->isCreature())
        return;
    if (p_caster == nullptr || p_caster->getPet() != nullptr)
        return;
    Creature* tame = static_cast<Creature*>(m_unitTarget);

    // Remove target
    tame->getAIInterface()->handleEvent(EVENT_LEAVECOMBAT, p_caster, 0);
    const auto pet = sObjectMgr.createPet(tame->getEntry(), nullptr);
    if (!pet->createAsSummon(tame->GetCreatureProperties(), tame, p_caster, p_caster->GetPosition(), 0, nullptr, effectIndex, PET_TYPE_HUNTER))
    {
        pet->DeleteMe();//CreateAsSummon() returns false if an error occurred.
    }
    tame->Despawn(0, tame->GetCreatureProperties()->RespawnTime);
}

void Spell::SpellEffectSummonPet(uint8_t effectIndex) //summon - pet
{
    if (!p_caster) return;

    if (getSpellInfo()->getId() == 883)  // "Call Pet" spell
    {
        if (p_caster->getPet() != nullptr)
        {
            sendCastResult(SPELL_FAILED_ALREADY_HAVE_SUMMON);
            return;
        }

        const auto foundPetId = p_caster->getPetIdFromSlot(PET_SLOT_FIRST_ACTIVE_SLOT);
        if (foundPetId.has_value())
        {
            const auto petno = foundPetId.value();
            if (p_caster->isPetRequiringTemporaryUnsummon())
            {
#if VERSION_STRING < WotLK
                sendCastResult(SPELL_FAILED_TRY_AGAIN);
#else
                sendCastResult(SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW);
#endif
                return;
            }

            if (p_caster->getPetCache(petno) == nullptr)
            {
                sendCastResult(SPELL_FAILED_ALREADY_HAVE_SUMMON);
                return;
            }

            if (p_caster->getPetCache(petno)->alive)
            {
                p_caster->spawnPet(petno);
            }
            else
            {
                SendTameFailure(PETTAME_DEAD);
            }
        }
        else
        {
            sendCastResult(SPELL_FAILED_NO_PET);
        }
        return;
    }

    //uint32_t entryId = GetProto()->EffectMiscValue[i];

    //VoidWalker:torment, sacrifice, suffering, consume shadows
    //Succubus:lash of pain, soothing kiss, seduce , lesser invisibility
    //felhunter:     Devour Magic,Paranoia,Spell Lock,  Tainted Blood

    // remove old pet
    Pet* old = p_caster->getPet();
    if (old)
        old->unSummon();

    CreatureProperties const* ci = sMySQLStore.getCreatureProperties(getSpellInfo()->getEffectMiscValue(effectIndex));
    if (ci)
    {
        if (p_caster->getClass() == WARLOCK)
        {
            //if demonic sacrifice auras are still active, remove them
            p_caster->removeAllAurasById(18789);
            p_caster->removeAllAurasById(18790);
            p_caster->removeAllAurasById(18791);
            p_caster->removeAllAurasById(18792);
            p_caster->removeAllAurasById(35701);
        }

        const auto pet = sObjectMgr.createPet(getSpellInfo()->getEffectMiscValue(effectIndex), nullptr);
        if (!pet->createAsSummon(ci, nullptr, u_caster, u_caster->GetPosition(), 0, m_spellInfo, effectIndex, PET_TYPE_SUMMON))
        {
            pet->DeleteMe();//CreateAsSummon() returns false if an error occurred.
        }
    }
}

void Spell::SpellEffectLearnPetSpell(uint8_t effectIndex)
{
    /*if (m_unitTarget && m_caster->getObjectTypeId() == TYPEID_PLAYER)
    {
    if (m_unitTarget->isPet() && m_unitTarget->getObjectTypeId() == TYPEID_UNIT)
    {
    TO< Player* >(m_caster)->AddPetSpell(GetProto()->EffectTriggerSpell[i], m_unitTarget->getEntry());
    }
    }*/

    if (m_unitTarget && m_unitTarget->isPet() && p_caster)
    {
        Pet* pPet = static_cast< Pet* >(m_unitTarget);
        if (!pPet->isHunterPet())
            p_caster->addSummonSpell(m_unitTarget->getEntry(), getSpellInfo()->getEffectTriggerSpell(effectIndex));

        pPet->addSpell(sSpellMgr.getSpellInfo(getSpellInfo()->getEffectTriggerSpell(effectIndex)));

        // Send Packet
        /*      WorldPacket data(SMSG_SET_EXTRA_AURA_INFO_OBSOLETE, 22);
        data << pPet->getGuid() << uint8_t(0) << uint32_t(GetProto()->EffectTriggerSpell[i]) << uint32_t(-1) << uint32_t(0);
        p_caster->getSession()->sendPacket(&data);*/
    }
}

void Spell::SpellEffectWeapondamage(uint8_t /*effectIndex*/)   // Weapon damage +
{
    if (!m_unitTarget || !u_caster)
        return;

    //Hackfix for Mangle
    if (p_caster != nullptr)
    {
        switch (getSpellInfo()->getId())
        {
            case 33876:
            case 33982:
            case 33983:
            case 48565:
            case 48566:
                p_caster->addComboPoints(m_unitTarget->getGuid(), 1);
                break;
        }
    }

    // Hacky fix for druid spells where it would "double attack".
    if (getSpellInfo()->getEffect(2) == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE || getSpellInfo()->getEffect(1) == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE)
    {
        add_damage += damage;
        return;
    }

    WeaponDamageType _type;
    if (GetType() == SPELL_DMG_TYPE_RANGED)
        _type = RANGED;
    else
    {
        if (hasAttributeExC(ATTRIBUTESEXC_REQUIRES_OFFHAND_WEAPON))
            _type = OFFHAND;
        else
            _type = MELEE;
    }
    m_targetDamageInfo = u_caster->strike(m_unitTarget, _type, getSpellInfo(), damage, 0, 0, m_triggeredSpell, true, isForcedCrit, this);
    isTargetDamageInfoSet = true;
}

void Spell::SpellEffectOpenLockItem(uint8_t /*effectIndex*/)
{
    if (p_caster == nullptr || i_caster == nullptr)
        return;

    p_caster->handleSpellLoot(i_caster->getItemProperties()->ItemId);
}

void Spell::SpellEffectSendEvent(uint8_t effectIndex) //Send Event
{
    //This is mostly used to trigger events on quests or some places

    if (sScriptMgr.CallScriptedDummySpell(m_spellInfo->getId(), effectIndex, this))
        return;

    if (sScriptMgr.HandleScriptedSpellEffect(m_spellInfo->getId(), effectIndex, this))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "Spell ID: {} ({}) has a scripted effect index ({}) but no handler for it.", m_spellInfo->getId(), m_spellInfo->getName(), effectIndex);

}

void Spell::SpellEffectPowerBurn(uint8_t effectIndex) // power burn
{
    if (m_unitTarget == nullptr || !m_unitTarget->isAlive() || m_unitTarget->getPowerType() != POWER_TYPE_MANA)
        return;

    if (m_unitTarget->isPlayer())
    {
        Player* mPlayer = static_cast< Player* >(m_unitTarget);
        if (mPlayer->isInFeralForm())
            return;

        // Resilience - reduces the effect of mana drains by (CalcRating*2)%.
        damage = Util::float2int32(damage * (1 - ((static_cast< Player* >(m_unitTarget)->calcRating(CR_CRIT_TAKEN_SPELL) * 2) / 100.0f)));
    }
    int32_t mult = damage;
    damage = mult * m_unitTarget->getMaxPower(POWER_TYPE_MANA) / 100;
    if (m_caster->isCreatureOrPlayer())  //Spell ctor has ASSERT(m_caster != NULL) so there's no need to add NULL checks, even if static analysis reports them.
    {
        Unit* caster = static_cast< Unit* >(m_caster);
        if ((uint32_t)damage > caster->getMaxPower(POWER_TYPE_MANA) * (mult * 2) / 100)
            damage = caster->getMaxPower(POWER_TYPE_MANA) * (mult * 2) / 100;
    }

    int32_t mana = std::min((int32_t)m_unitTarget->getPower(POWER_TYPE_MANA), damage);

    m_unitTarget->modPower(POWER_TYPE_MANA, -mana);

    m_targetDamageInfo = m_caster->doSpellDamage(m_unitTarget, getSpellInfo()->getId(), mana * getSpellInfo()->getEffectMultipleValue(effectIndex), effectIndex, m_triggeredSpell, false, false, isForcedCrit, this);
    isTargetDamageInfoSet = true;
}

void Spell::SpellEffectThreat(uint8_t effectIndex) // Threat
{
    if (!m_unitTarget || !m_unitTarget->isAlive() || !m_unitTarget->isCreature())
        return;

    int32_t amount = getSpellInfo()->getEffectBasePoints(effectIndex);

    bool chck = m_unitTarget->getThreatManager().getThreat(u_caster);

    if (!chck)
    {
        m_unitTarget->getAIInterface()->onHostileAction(u_caster);
        m_unitTarget->getThreatManager().addThreat(u_caster, 0.0f);
    }
    else
        m_unitTarget->getThreatManager().modifyThreatByPercent(u_caster, (int32_t)m_unitTarget->getThreatManager().getThreat(u_caster) * amount / 100);
}

void Spell::SpellEffectClearQuest(uint8_t effectIndex)
{
    if (m_playerTarget == nullptr)
    {
        sLogger.failure("Spell {} ({}) was not casted on Player, but Spell requires Player to be a target.", m_spellInfo->getId(), m_spellInfo->getName());
        return;
    }

    uint32_t questid1 = m_spellInfo->getEffectBasePoints(effectIndex);
    uint32_t questid2 = m_spellInfo->getEffectMiscValue(effectIndex);

    m_playerTarget->clearQuest(questid1);
    m_playerTarget->clearQuest(questid2);
}

void Spell::SpellEffectApplyRaidAA(uint8_t effectIndex)
{
    ApplyAreaAura(effectIndex);
}

void Spell::SpellEffectPowerFunnel(uint8_t /*effectIndex*/) // Power Funnel
{
    if (!m_unitTarget || !m_unitTarget->isAlive() || !m_unitTarget->isPet())
        return;

    //does not exist
}

void Spell::SpellEffectHealMaxHealth(uint8_t /*effectIndex*/)   // Heal Max Health
{
    if (!m_unitTarget || !m_unitTarget->isAlive())
        return;

    uint32_t dif = m_unitTarget->getMaxHealth() - m_unitTarget->getHealth();
    if (!dif)
    {
        sendCastResult(SPELL_FAILED_ALREADY_AT_FULL_HEALTH);
        return;
    }

    m_unitTarget->addSimpleHealingBatchEvent(dif, u_caster, pSpellId != 0 ? sSpellMgr.getSpellInfo(pSpellId) : getSpellInfo());

    if (u_caster != nullptr)
    {
        switch (this->getSpellInfo()->getId())
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
                u_caster->castSpell(m_unitTarget, 25771, true);
                break;
            default:
                break;
        }
    }
}

void Spell::SpellEffectInterruptCast(uint8_t /*effectIndex*/) // Interrupt Cast
{
    if (!m_unitTarget || !m_unitTarget->isAlive())
        return;

    if (getSpellInfo()->getAttributesExG() & ATTRIBUTESEXG_INTERRUPT_NPC && m_unitTarget->isPlayer())
        return;

    // Get target's current spell (either channeled or generic spell with cast time)
    if (m_unitTarget->isCastingSpell(false, true))
    {
        Spell* TargetSpell = nullptr;
        if (m_unitTarget->getCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr && m_unitTarget->getCurrentSpell(CURRENT_CHANNELED_SPELL)->getCastTimeLeft() > 0)
        {
            TargetSpell = m_unitTarget->getCurrentSpell(CURRENT_CHANNELED_SPELL);
        }
        // No need to check cast time for generic spells, checked already in Object::isCastingSpell()
        else if (m_unitTarget->getCurrentSpell(CURRENT_GENERIC_SPELL) != nullptr)
        {
            TargetSpell = m_unitTarget->getCurrentSpell(CURRENT_GENERIC_SPELL);
        }

        if (TargetSpell != nullptr)
        {
            uint32_t school = TargetSpell->getSpellInfo()->getFirstSchoolFromSchoolMask(); // Get target's casting spell school
            int32_t duration = getDuration(); // Duration of school lockout

            // Check for CastingTime (to prevent interrupting instant casts), PreventionType
            // and InterruptFlags of target's casting spell
            if (school
                && (TargetSpell->getState() == SPELL_STATE_CHANNELING
                || (TargetSpell->getState() == SPELL_STATE_CASTING && TargetSpell->getSpellInfo()->getCastingTimeIndex() > 0))
                && TargetSpell->getSpellInfo()->getPreventionType() == PREVENTION_TYPE_SILENCE
                && ((TargetSpell->getSpellInfo()->getInterruptFlags() & CAST_INTERRUPT_ON_AUTOATTACK)
                || (TargetSpell->getSpellInfo()->getChannelInterruptFlags() & CHANNEL_INTERRUPT_ON_MOVEMENT)))
            {
                if (m_unitTarget->isPlayer())
                {
                    // Check for interruption reducing talents
                    int32_t DurationModifier = m_unitTarget->m_mechanicDurationPctMod[MECHANIC_INTERRUPTED];
                    if (DurationModifier >= -100)
                        duration = (duration * (100 + DurationModifier)) / 100;

                    // Prevent player from casting in that school
                    static_cast<Player*>(m_unitTarget)->sendPreventSchoolCast(school, duration);
                }
                else
                {
                    // Prevent unit from casting in that school
                    m_unitTarget->m_schoolCastPrevent[school] = duration + Util::getMSTime();
                }

                // Interrupt the spell cast
                m_unitTarget->interruptSpell(TargetSpell->getSpellInfo()->getId(), false);
            }
        }
    }
}

void Spell::SpellEffectDistract(uint8_t /*effectIndex*/) // Distract
{
    //spellId 1725 Distract:Throws a distraction attracting the all monsters for ten sec's
    if (!m_unitTarget || !m_unitTarget->isAlive())
        return;

    if (m_targets.getDestination().isSet())
    {
        //      m_unitTarget->getAIInterface()->MoveTo(m_targets.m_destX, m_targets.m_destY, m_targets.m_destZ, 0);
        uint32_t Stare_duration = getDuration();
        if (Stare_duration > 30 * 60 * 1000)
            Stare_duration = 10000;//if we try to stare for more then a half an hour then better not stare at all :P (bug)

        auto destination = m_targets.getDestination();
        float newo = m_unitTarget->calcRadAngle(m_unitTarget->GetPositionX(), m_unitTarget->GetPositionY(), destination.x, destination.y);

        m_unitTarget->pauseMovement(Stare_duration);
        m_unitTarget->setFacing(newo);
    }

    //Smoke Emitter 164870
    //Smoke Emitter Big 179066
    //Unit Field Target of
}

void Spell::SpellEffectPickpocket(uint8_t /*effectIndex*/) // pickpocket
{
    //Show random loot based on roll,
    if (!m_unitTarget || !p_caster || !m_unitTarget->isCreature())
        return;

    Creature* target = static_cast< Creature* >(m_unitTarget);
    if (target->IsPickPocketed() || (target->GetCreatureProperties()->Type != UNIT_TYPE_HUMANOID))
    {
        sendCastResult(SPELL_FAILED_TARGET_NO_POCKETS);
        return;
    }

    sLootMgr.fillPickpocketingLoot(p_caster, &m_unitTarget->loot, m_unitTarget->getEntry(), 0);

    uint32_t _rank = static_cast< Creature* >(m_unitTarget)->GetCreatureProperties()->Rank;
    m_unitTarget->loot.gold = Util::float2int32((_rank + 1) * m_unitTarget->getLevel() * (Util::getRandomUInt(5) + 1) * worldConfig.getFloatRate(RATE_MONEY));

    p_caster->sendLoot(m_unitTarget->getGuid(), LOOT_PICKPOCKETING, m_unitTarget->GetMapId());
    target->SetPickPocketed(true);
}

void Spell::SpellEffectAddFarsight(uint8_t effectIndex) // Add Farsight
{
    if (p_caster == nullptr)
        return;

    auto lv = m_targets.getDestination();
    if (!lv.isSet())
    {
        lv = m_targets.getSource();
    }

    DynamicObject* dynObj = p_caster->getWorldMap()->createDynamicObject();
    dynObj->create(u_caster, this, lv, static_cast<uint32_t>(getDuration()), getEffectRadius(effectIndex), DYNAMIC_OBJECT_FARSIGHT_FOCUS);
    dynObj->SetInstanceID(p_caster->GetInstanceID());
    p_caster->setFarsightGuid(dynObj->getGuid());

    p_caster->getWorldMap()->changeFarsightLocation(p_caster, dynObj);
}

void Spell::SpellEffectUseGlyph(uint8_t effectIndex)
{
#if VERSION_STRING > TBC
    if (!p_caster)
        return;

    if (m_glyphslot >= GLYPHS_COUNT)
        return;

    const auto glyphSlot = static_cast<uint16_t>(m_glyphslot);
    uint32_t glyph_new = m_spellInfo->getEffectMiscValue(effectIndex);
    auto glyph_prop_new = sGlyphPropertiesStore.lookupEntry(glyph_new);
    if (!glyph_prop_new)
        return;

    // check if glyph is locked (obviously)
    if (!(p_caster->getGlyphsEnabled() & (1 << glyphSlot)))
    {
        sendCastResult(SPELL_FAILED_GLYPH_SOCKET_LOCKED);
        return;
    }

    uint32_t glyph_old = p_caster->getGlyph(glyphSlot);
    if (glyph_old)
    {
        if (glyph_old == glyph_new)
        {
            return;
        }
        else
        {
            auto glyph_prop_old = sGlyphPropertiesStore.lookupEntry(glyph_old);
            if (glyph_prop_old)
                p_caster->removeAllAurasById(glyph_prop_old->SpellID);
        }
    }

    auto glyph_slot = sGlyphSlotStore.lookupEntry(p_caster->getGlyphSlot(glyphSlot));
    if (glyph_slot)
    {
        if (glyph_slot->Type != glyph_prop_new->Type)
        {
            sendCastResult(SPELL_FAILED_INVALID_GLYPH);
            return;
        }
        p_caster->setGlyph(glyphSlot, glyph_new);
        p_caster->castSpell(p_caster, glyph_prop_new->SpellID, true);
        p_caster->m_specs[p_caster->m_talentActiveSpec].setGlyph(static_cast<uint16_t>(glyph_new), glyphSlot);
        p_caster->smsg_TalentsInfo(false);
    }
#endif
}

void Spell::SpellEffectHealMechanical(uint8_t /*effectIndex*/)
{
    if (!m_unitTarget || !m_unitTarget->isCreature() || static_cast< Creature* >(m_unitTarget)->GetCreatureProperties()->Type != UNIT_TYPE_MECHANICAL)
        return;

    m_targetDamageInfo = m_caster->doSpellHealing(m_unitTarget, getSpellInfo()->getId(), static_cast<float_t>(damage), m_triggeredSpell, false, false, isForcedCrit, this);
    isTargetDamageInfoSet = true;
}

void Spell::SpellEffectSummonObjectWild(uint8_t effectIndex)
{
    if (!u_caster)
        return;

    uint32_t gameobject_id = getSpellInfo()->getEffectMiscValue(effectIndex);

    float x, y, z;
    if (m_targets.hasDestination())
    {
        m_targets.getDestination().getPosition(x, y, z);
    }
    else
    {
        u_caster->getClosePoint(x, y, z, 0.388999998569489f);
    }

    WorldMap* map = u_caster->getWorldMap();

    QuaternionData rot = QuaternionData::fromEulerAnglesZYX(m_caster->GetOrientation(), 0.f, 0.f);

    // spawn a new one
    GameObject* GoSummon = u_caster->getWorldMap()->createGameObject(gameobject_id);
    if (!GoSummon->create(gameobject_id, map, m_caster->GetPhase(), LocationVector(x, y, z, m_caster->GetOrientation()), rot, GO_STATE_CLOSED))
    {
        delete GoSummon;
        return;
    }

    int32_t duration = getDuration();

    GoSummon->setRespawnTime(duration > 0 ? duration / IN_MILLISECONDS : 0);

    GoSummon->PushToWorld(u_caster->getWorldMap());
    u_caster->addGameObject(GoSummon);
    GoSummon->setSpellId(m_spellInfo->getId());

    if (GameObject* linkedTrap = GoSummon->getLinkedTrap())
    {
        linkedTrap->setRespawnTime(duration > 0 ? duration / IN_MILLISECONDS : 0);
        linkedTrap->setSpellId(m_spellInfo->getId());
    }
}

void Spell::SpellEffectSanctuary(uint8_t /*effectIndex*/) // Stop all attacks made to you
{
    if (!u_caster)
        return;

    if (p_caster != nullptr)
        p_caster->removeAllAurasByAuraEffect(SPELL_AURA_MOD_ROOT);

    for (const auto& itr : u_caster->getInRangeObjectsSet())
    {
        if (itr && itr->isCreature())
            static_cast<Creature*>(itr)->getThreatManager().clearThreat(m_unitTarget);
    }
}

void Spell::SpellEffectAddComboPoints(uint8_t /*effectIndex*/) // Add Combo Points
{
    if (!p_caster)
        return;

    p_caster->addComboPoints(p_caster->getTargetGuid(), static_cast<uint8_t>(damage));
}

void Spell::SpellEffectCreateHouse(uint8_t /*effectIndex*/) // Create House
{
}

void Spell::SpellEffectDuel(uint8_t /*effectIndex*/) // Duel
{
    if (!p_caster || !p_caster->isAlive())
        return;

    if (p_caster->isStealthed())
    {
        sendCastResult(SPELL_FAILED_CANT_DUEL_WHILE_STEALTHED);
        return; // Player is stealth
    }
    if (!m_playerTarget || m_playerTarget == p_caster)
        return;

    /* not implemented yet
    \todo dueling zones ? (SPELL_FAILED_NO_DUELING)
    if (player->IsInvisible())
    {
    sendCastResult(SPELL_FAILED_CANT_DUEL_WHILE_INVISIBLE);
    return;
    }
    */

    //Player* pTarget = sObjHolder.GetObject<Player>(player->getTargetGuid());      //  hacky.. and will screw up if plr is deselected..
    if (!m_playerTarget)
    {
        sendCastResult(SPELL_FAILED_BAD_TARGETS);
        return; // invalid Target
    }
    if (!m_playerTarget->isAlive())
    {
        sendCastResult(SPELL_FAILED_TARGETS_DEAD);
        return; // Target not alive
    }
    // todo
    /*if (playerTarget->hasUnitStateFlag(UNIT_STATE_ATTACKING))
    {
        sendCastResult(SPELL_FAILED_TARGET_IN_COMBAT);
        return; // Target in combat with another unit
    }*/
    if (m_playerTarget->getDuelPlayer())
    {
        sendCastResult(SPELL_FAILED_TARGET_DUELING);
        return; // Already Dueling
    }

    p_caster->requestDuel(m_playerTarget);
}

void Spell::SpellEffectStuck(uint8_t /*effectIndex*/)
{
    if (!m_playerTarget || m_playerTarget != p_caster)
        return;

    sEventMgr.AddEvent(m_playerTarget, &Player::eventTeleport, m_playerTarget->getBindMapId(), LocationVector(m_playerTarget->getBindPosition().x, m_playerTarget->getBindPosition().y,
                       m_playerTarget->getBindPosition().z, m_playerTarget->getBindPosition().o), uint32_t(0), EVENT_PLAYER_TELEPORT, 50, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    /*
    playerTarget->SafeTeleport(playerTarget->getBindMapId(), 0, playerTarget->GetBindPositionX(), playerTarget->GetBindPositionY(), playerTarget->GetBindPositionZ(), 3.14f);*/
}

void Spell::SpellEffectSummonPlayer(uint8_t /*effectIndex*/)
{
    if (!m_playerTarget)
        return;

    // vojta: from 2.4 players can be summoned on another map
    //if (m_caster->getWorldMap()->GetMapInfo() && m_caster->getWorldMap()->GetMapInfo()->type != INSTANCE_NULL && m_caster->GetMapId() != playerTarget->GetMapId())
    //  return;
    if (m_caster->getWorldMap()->getBaseMap()->getMapInfo() && m_playerTarget->getLevel() < m_caster->getWorldMap()->getBaseMap()->getMapInfo()->minlevel)    // we need some blizzlike message that player needs level xx - feel free to add it ;)
        return;

    m_playerTarget->sendSummonRequest(m_caster->getGuidLow(), m_caster->getZoneId(), m_caster->GetMapId(), m_caster->GetInstanceID(), m_caster->GetPosition());
}

void Spell::SpellEffectActivateObject(uint8_t effectIndex) // Activate Object
{
    if (!p_caster)
        return;

    if (!m_gameObjTarget)
    {
        sLogger.failure("Spell {} ({}) effect %hhu not handled because no target was found. ", m_spellInfo->getId(), m_spellInfo->getName(), effectIndex);
        return;
    }

    if (m_gameObjTarget->GetScript())
        m_gameObjTarget->GetScript()->OnActivate(p_caster);

    m_gameObjTarget->setDynamicFlags(GO_DYN_FLAG_INTERACTABLE);

#if VERSION_STRING < WotLK
    sEventMgr.AddEvent(m_gameObjTarget, &GameObject::setDynamicFlags, static_cast<uint32_t>(0), 0, static_cast<uint32_t>(getDuration()), 1, 0);
#elif VERSION_STRING < Mop
    sEventMgr.AddEvent(m_gameObjTarget, &GameObject::setDynamicFlags, static_cast<uint16_t>(0), 0, static_cast<uint32_t>(getDuration()), 1, 0);
#else
    sEventMgr.AddEvent(dynamic_cast<Object*>(m_gameObjTarget), &Object::setDynamicFlags, static_cast<uint16_t>(0), 0, static_cast<uint32_t>(getDuration()), 1, 0);
#endif
}

void Spell::SpellEffectBuildingDamage(uint8_t effectIndex)
{
    if (m_gameObjTarget == nullptr)
    {
        sLogger.failure("Spell {} ({}) effect %hhu not handled because no target was found. ", m_spellInfo->getId(), m_spellInfo->getName(), effectIndex);
        return;
    }

    if (m_gameObjTarget->GetGameObjectProperties()->type != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
        return;

    if (u_caster == nullptr)
        return;

    uint32_t spellDamage = m_spellInfo->getEffectBasePoints(effectIndex) + 1;
    Unit* controller = nullptr;
#ifdef FT_VEHICLES
    if (u_caster->getVehicle() != nullptr)
        controller = u_caster->getWorldMap()->getUnit(u_caster->getCharmedByGuid());
#endif

    if (controller == nullptr)
        controller = u_caster;

    // Baaaam
    GameObject_Destructible* dgo = static_cast<GameObject_Destructible*>(m_gameObjTarget);
    dgo->Damage(spellDamage, u_caster->getGuid(), controller->getGuid(), m_spellInfo->getId());
}

void Spell::SpellEffectEnchantHeldItem(uint8_t effectIndex)
{
    if (!m_playerTarget) return;

    Item* item = m_playerTarget->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
    if (!item)
        return;

    uint32_t Duration = 0; // Needs to be found in dbc.. I guess?

    switch (getSpellInfo()->getId())
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

    auto spell_item_enchant = sSpellItemEnchantmentStore.lookupEntry(getSpellInfo()->getEffectMiscValue(effectIndex));

    if (!spell_item_enchant)
    {
        sLogger.failure("Invalid enchantment entry {} for Spell {}", getSpellInfo()->getEffectMiscValue(effectIndex), getSpellInfo()->getId());
        return;
    }

    item->removeEnchantment(TEMP_ENCHANTMENT_SLOT);
    item->addEnchantment(getSpellInfo()->getEffectMiscValue(effectIndex), TEMP_ENCHANTMENT_SLOT, Duration * 1000);
}

void Spell::SpellEffectSelfResurrect(uint8_t effectIndex)
{
    if (!p_caster || !m_unitTarget || m_playerTarget->isAlive()) return;

    uint32_t mana;
    uint32_t health;
    uint32_t class_ = m_unitTarget->getClass();

    switch (getSpellInfo()->getId())
    {
        case 3026:
        case 20758:
        case 20759:
        case 20760:
        case 20761:
        case 27240:
        case 47882:
        {
            health = getSpellInfo()->getEffectMiscValue(effectIndex);
            mana = -damage;
        }
        break;
        case 21169: //Reincarnation. Resurrect with 20% health and mana
        {
            int32_t amt = damage;
            health = uint32_t((m_unitTarget->getMaxHealth() * amt) / 100);
            mana = uint32_t((m_unitTarget->getMaxPower(POWER_TYPE_MANA) * amt) / 100);
        }
        break;
        default:
        {
            if (damage < 0) return;
            health = uint32_t(m_unitTarget->getMaxHealth() * damage / 100);
            mana = uint32_t(m_unitTarget->getMaxPower(POWER_TYPE_MANA) * damage / 100);
        }
        break;
    }

    if (class_ == WARRIOR || class_ == ROGUE)
        mana = 0;

    m_playerTarget->setResurrectHealth(health);
    m_playerTarget->setResurrectMana(mana);

    m_playerTarget->resurrect();
    m_playerTarget->setMoveRoot(false);

    m_playerTarget->setSelfResurrectSpell(0);

    if (getSpellInfo()->getId() == 21169)
        p_caster->addSpellCooldown(getSpellInfo(), i_caster, this);
}

void Spell::SpellEffectSkinning(uint8_t /*effectIndex*/)
{
    if (!m_unitTarget || !m_unitTarget->isCreature())
        return;

    Creature* cr = static_cast<Creature*>(m_unitTarget);
    auto skill = cr->GetRequiredLootSkill();
    auto sk = static_cast<Player*>(m_caster)->getSkillLineCurrent(skill);
    uint32_t lvl = cr->getLevel();

    if ((sk >= lvl * 5) || ((sk + 100U) >= lvl * 10))
    {
        //Fill loot for Skinning
        sLootMgr.fillSkinningLoot(p_caster, &cr->loot, m_unitTarget->getEntry(), 0);
        static_cast<Player*>(m_caster)->sendLoot(m_unitTarget->getGuid(), LOOT_SKINNING, m_unitTarget->GetMapId());

        //Not skinable again
        cr->removeUnitFlags(UNIT_FLAG_SKINNABLE);
        cr->Skinned = true;

        if (cr->GetCreatureProperties()->Rank > 0)
            DetermineSkillUp(skill, sk < lvl * 5 ? sk / 5 : lvl, 2);
        else
            DetermineSkillUp(skill, sk < lvl * 5 ? sk / 5 : lvl, 1);
    }
    else
    {
        sendCastResult(SPELL_FAILED_TARGET_UNSKINNABLE);
    }
}

void Spell::SpellEffectCharge(uint8_t /*effectIndex*/)
{
    if (m_unitTarget == nullptr || !m_unitTarget->isAlive())
        return;

    float speed = G3D::fuzzyGt(getSpellInfo()->getSpeed(), 0.0f) ? getSpellInfo()->getSpeed() : SPEED_CHARGE;

    LocationVector pos = m_unitTarget->getFirstCollisionPosition(m_unitTarget->getCombatReach(), m_unitTarget->getRelativeAngle(m_caster));
    u_caster->getMovementManager()->moveCharge(pos, speed);
}

void Spell::SpellEffectKnockBack(uint8_t effectIndex)
{
    if (m_unitTarget == nullptr || !m_unitTarget->isAlive())
        return;

    // Spells with SPELL_EFFECT_KNOCK_BACK (like Thunderstorm) can't knockback target if target has ROOT/STUN
    if (m_unitTarget->hasUnitStateFlag(UNIT_STATE_ROOTED | UNIT_STATE_STUNNED))
        return;

    float ratio = 0.1f;
    float speedxy = float(m_spellInfo->getEffectMiscValue(effectIndex)) * ratio;
    float speedz = float(damage) * ratio;
    if (speedxy < 0.01f && speedz < 0.01f)
        return;

    float x, y;
#if VERSION_STRING >= TBC
    if (m_spellInfo->getEffect(effectIndex) == SPELL_EFFECT_KNOCK_BACK_DEST)
    {
        if (m_targets.hasDestination())
        {
            auto destination = m_targets.getDestination();
            x = destination.x;
            y = destination.y;
        }
        else
            return;
    }
    else
#endif
    {
        m_caster->getPosition(x, y);
    }

    m_unitTarget->knockbackFrom(x, y, speedxy, speedz);
}

void Spell::SpellEffectKnockBack2(uint8_t effectIndex)
{
    if (m_unitTarget == nullptr || !m_unitTarget->isAlive())
        return;

    m_unitTarget->handleKnockback(m_caster, getSpellInfo()->getEffectMiscValue(effectIndex) / 10.0f, damage / 10.0f);
}

void Spell::SpellEffectPullTowardsDest(uint8_t effIndex)
{
    if (!m_unitTarget)
        return;

    if (!m_targets.hasDestination())
    {
        sLogger.failure("Spell {} with SPELL_EFFECT_PULL_TOWARDS_DEST has no dest target", m_spellInfo->getId());
        return;
    }

    LocationVector pos = m_targets.getDestination();
    // This is a blizzlike mistake: this should be 2D distance according to projectile motion formulas, but Blizzard erroneously used 3D distance
    float distXY = m_unitTarget->getExactDist(pos);

    // Avoid division by 0
    if (distXY < 0.001)
        return;

    float distZ = pos.getPositionZ() - m_unitTarget->GetPositionZ();

    float speedXY = m_spellInfo->getEffectMiscValue(effIndex) ? m_spellInfo->getEffectMiscValue(effIndex) / 10.0f : 30.0f;
    float speedZ = (2 * speedXY * speedXY * distZ + MovementMgr::gravity * distXY * distXY) / (2 * speedXY * distXY);

    if (!std::isfinite(speedZ))
    {
        sLogger.failure("Spell {} with SPELL_EFFECT_PULL_TOWARDS_DEST called with invalid speedZ.", m_spellInfo->getId());
        return;
    }

    m_unitTarget->jumpTo(speedXY, speedZ, true, pos);
}

void Spell::SpellEffectDisenchant(uint8_t /*effectIndex*/)
{
    if (!p_caster)
        return;

    Item* it = p_caster->getItemInterface()->GetItemByGUID(m_targets.getItemTargetGuid());
    if (!it)
    {
        sendCastResult(SPELL_FAILED_CANT_BE_DISENCHANTED);
        return;
    }

    //Fill disenchanting loot
    p_caster->setLootGuid(it->getGuid());
    if (!it->m_loot)
    {
        it->m_loot = std::make_unique<Loot>();
        sLootMgr.fillItemLoot(p_caster, it->m_loot.get(), it->getEntry(), 0);
    }

    sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "Successfully disenchanted item {}", uint32_t(it->getEntry()));
    p_caster->sendLoot(it->getGuid(), LOOT_DISENCHANTING, p_caster->GetMapId());

    //We can increase Enchanting skill up to 60
    auto skill = p_caster->getSkillLineCurrent(SKILL_ENCHANTING);
    if (skill && skill < 60)
    {
        if (Util::checkChance(100.0f - skill * 0.75f))
        {
            auto SkillUp = static_cast<uint16_t>(Util::float2int32(1.0f * worldConfig.getFloatRate(RATE_SKILLRATE)));
            if (skill + SkillUp > 60)
                SkillUp = 60 - skill;

            p_caster->advanceSkillLine(SKILL_ENCHANTING, SkillUp);
        }
    }
    if (it == i_caster)
        i_caster = nullptr;
}

void Spell::SpellEffectInebriate(uint8_t /*effectIndex*/) // lets get drunk!
{
    if (m_playerTarget == nullptr)
        return;

    // Drunkee!
    uint16_t currentDrunk = m_playerTarget->getServersideDrunkValue();
    uint16_t drunkMod = static_cast<uint16_t>(damage)* 256;
    if (currentDrunk + drunkMod > 0xFFFF)
        currentDrunk = 0xFFFF;
    else
        currentDrunk += drunkMod;
    m_playerTarget->setServersideDrunkValue(currentDrunk, i_caster ? i_caster->getEntry() : 0);
}

void Spell::SpellEffectFeedPet(uint8_t effectIndex)  // Feed Pet
{
    // food flags and food level are checked in Spell::CanCast()
    if (!m_itemTarget || !p_caster)
        return;

    Pet* pPet = p_caster->getPet();
    if (!pPet)
        return;

    /** Cast feed pet effect
    - effect is item level and pet level dependent, aura ticks are 35, 17, 8 (*1000) happiness*/
    int8_t deltaLvl = static_cast<int8_t>(pPet->getLevel() - m_itemTarget->getItemProperties()->ItemLevel);
    damage /= 1000; //damage of Feed pet spell is 35000
    if (deltaLvl > 10) damage = damage >> 1;//divide by 2
    if (deltaLvl > 20) damage = damage >> 1;
    damage *= 1000;

    const auto spellInfo = sSpellMgr.getSpellInfo(getSpellInfo()->getEffectTriggerSpell(effectIndex));
    Spell* sp = sSpellMgr.newSpell(p_caster, spellInfo, true, nullptr);
    sp->forced_basepoints->set(0, damage);
    SpellCastTargets tgt(pPet->getGuid());
    sp->prepare(&tgt);

    if (m_itemTarget->getStackCount() > 1)
    {
        m_itemTarget->modStackCount(-1);
        m_itemTarget->m_isDirty = true;
    }
    else
    {
        p_caster->getItemInterface()->SafeFullRemoveItemByGuid(m_itemTarget->getGuid());
        m_itemTarget = nullptr;
    }
}

void Spell::SpellEffectDismissPet(uint8_t /*effectIndex*/)
{
    // remove pet.. but don't delete so it can be called later
    if (!p_caster) return;

    Pet* pPet = p_caster->getPet();
    if (!pPet) return;
    pPet->unSummon();
}

void Spell::SpellEffectReputation(uint8_t effectIndex)
{
    if (!m_playerTarget)
        return;

    m_playerTarget->modFactionStanding(getSpellInfo()->getEffectMiscValue(effectIndex), damage);
}

void Spell::SpellEffectSummonObjectSlot(uint8_t effectIndex)
{
    if (!u_caster || !u_caster->IsInWorld())
        return;

    GameObject* GoSummon = nullptr;

    uint32_t slot = getSpellInfo()->getEffect(effectIndex) - SPELL_EFFECT_SUMMON_OBJECT_SLOT1;
    GoSummon = u_caster->m_objectSlots[slot] ? u_caster->getWorldMap()->getGameObject(u_caster->m_objectSlots[slot]) : 0;
    u_caster->m_objectSlots[slot] = 0;

    if (uint32_t guid = u_caster->m_objectSlots[slot])
    {
        if (GameObject* obj = u_caster->getWorldMapGameObject(guid))
        {
            // Recast case - null spell id to make auras not be removed on object remove from world
            if (m_spellInfo->getId() == obj->getSpellId())
                obj->setSpellId(0);
            u_caster->removeGameObject(obj, true);
        }
        u_caster->m_objectSlots[slot] = 0;
    }

    // spawn a new one
    GoSummon = u_caster->getWorldMap()->createGameObject(getSpellInfo()->getEffectMiscValue(effectIndex));

    float dx = 0.0f;
    float dy = 0.0f;
    float dz = 0.0f;

    if (m_targets.hasDestination())
    {
        auto destination = m_targets.getDestination();
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

    QuaternionData rot = QuaternionData::fromEulerAnglesZYX(m_caster->GetOrientation(), 0.f, 0.f);
    if (!GoSummon->create(getSpellInfo()->getEffectMiscValue(effectIndex), m_caster->getWorldMap(), m_caster->GetPhase(), LocationVector(dx, dy, dz, m_caster->GetOrientation()), rot, GO_STATE_CLOSED))
    {
        delete GoSummon;
        return;
    }

    GoSummon->setLevel(u_caster->getLevel());
    GoSummon->setCreatedByGuid(m_caster->getGuid());
    GoSummon->Phase(PHASE_SET, u_caster->GetPhase());

    GoSummon->PushToWorld(m_caster->getWorldMap());

    int32_t duration = getDuration();

    GoSummon->setRespawnTime(duration > 0 ? duration / IN_MILLISECONDS : 0);
    GoSummon->setSpellId(m_spellInfo->getId());
    u_caster->addGameObject(GoSummon);
    u_caster->m_objectSlots[slot] = GoSummon->GetUIdFromGUID();
}

void Spell::SpellEffectDispelMechanic(uint8_t effectIndex)
{
    if (!m_unitTarget || !m_unitTarget->isAlive())
        return;

    m_unitTarget->removeAllAurasBySpellMechanic(static_cast<SpellMechanic>(getSpellInfo()->getEffectMiscValue(effectIndex)), false);
}

void Spell::SpellEffectSummonDeadPet(uint8_t /*effectIndex*/)
{
    //this is pet resurrect
    if (!p_caster)
        return;
    Pet* pPet = p_caster->getPet();
    if (pPet)
    {
        // Pet should teleport to owner's position
        HandleTeleport(p_caster->GetPosition(), p_caster->GetMapId(), pPet);
        //\note remove all dynamic flags
        pPet->setDynamicFlags(0);
        pPet->setHealth(pPet->getMaxHealth() * damage / 100);
        pPet->setDeathState(ALIVE);
        pPet->getAIInterface()->handleEvent(EVENT_FOLLOWOWNER, pPet, 0);
        pPet->sendSpellsToController(p_caster, pPet->getTimeLeft());

        // Restore unit and pvp flags that were resetted on death
        if (p_caster->isPvpFlagSet())
            pPet->setPvpFlag();
        else
            pPet->removePvpFlag();

        if (p_caster->isFfaPvpFlagSet())
            pPet->setFfaPvpFlag();
        else
            pPet->removeFfaPvpFlag();

        if (p_caster->isSanctuaryFlagSet())
            pPet->setSanctuaryFlag();
        else
            pPet->removeSanctuaryFlag();

        if (p_caster->hasUnitFlags(UNIT_FLAG_PVP_ATTACKABLE))
            pPet->addUnitFlags(UNIT_FLAG_PVP_ATTACKABLE);

#if VERSION_STRING == TBC
        if (p_caster->hasUnitFlags(UNIT_FLAG_PVP_ATTACKABLE))
            pPet->setPositiveAuraLimit(POS_AURA_LIMIT_PVP_ATTACKABLE);
        else
            pPet->setPositiveAuraLimit(POS_AURA_LIMIT_CREATURE);
#endif
    }
    else
    {
        // This was set in canCast so it should exist at this point
        if (add_damage == 0)
            return;

        if (!p_caster->isPetRequiringTemporaryUnsummon())
            p_caster->spawnPet(static_cast<uint8_t>(add_damage));

        pPet = p_caster->getPet();
        if (pPet == nullptr)//no pets to Revive
            return;

        pPet->setHealth(pPet->getMaxHealth() * damage / 100);
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

    uint32_t RetreivedMana = 0;
    uint32_t refundpercent = m_spellInfo->getEffectBasePoints(effectIndex) + 1;

    for (uint8_t i = SUMMON_SLOT_TOTEM_FIRE; i <= SUMMON_SLOT_TOTEM_AIR; ++i)
    {
        const auto* totem = p_caster->getSummonInterface()->getSummonInSlot(static_cast<SummonSlot>(i));
        if (totem == nullptr || !totem->isTotem())
            continue;

        const auto spellid = totem->getCreatedBySpellId();
        SpellInfo const* sp = sSpellMgr.getSpellInfo(spellid);

        if (sp != nullptr)
        {
            uint32_t cost = 0;

            if (sp->getManaCostPercentage() != 0)
                cost = (p_caster->getBaseMana() * sp->getManaCostPercentage()) / 100;
            else
                cost = sp->getManaCost();

            RetreivedMana += static_cast<uint32_t>((cost * refundpercent) / 100.0f);
        }
    }

    p_caster->getSummonInterface()->killAllTotems();
    p_caster->energize(p_caster, getSpellInfo()->getId(), RetreivedMana, POWER_TYPE_MANA);
}

void Spell::SpellEffectResurrectNew(uint8_t effectIndex)
{
    //base p =hp,misc mana
    if (!m_playerTarget)
    {
        if (!m_corpseTarget)
        {
            // unit resurrection handler
            if (m_unitTarget)
            {
                if (m_unitTarget->isCreature() && m_unitTarget->isPet() && m_unitTarget->isDead())
                {
                    uint32_t hlth = ((uint32_t)getSpellInfo()->getEffectBasePoints(effectIndex) > m_unitTarget->getMaxHealth()) ? m_unitTarget->getMaxHealth() : (uint32_t)getSpellInfo()->getEffectBasePoints(effectIndex);
                    uint32_t mana = ((uint32_t)getSpellInfo()->getEffectBasePoints(effectIndex) > m_unitTarget->getMaxPower(POWER_TYPE_MANA)) ? m_unitTarget->getMaxPower(POWER_TYPE_MANA) : (uint32_t)getSpellInfo()->getEffectBasePoints(effectIndex);

                    if (!m_unitTarget->isPet())
                    {
                        sEventMgr.RemoveEvents(m_unitTarget, EVENT_CREATURE_REMOVE_CORPSE);
                    }
                    else
                    {
                        sEventMgr.RemoveEvents(m_unitTarget, EVENT_PET_DELAYED_REMOVE);
                        sEventMgr.RemoveEvents(m_unitTarget, EVENT_CREATURE_REMOVE_CORPSE);
                    }
                    m_unitTarget->setHealth(hlth);
                    m_unitTarget->setPower(POWER_TYPE_MANA, mana);
                    m_unitTarget->setTaggerGuid(nullptr);
                    m_unitTarget->setDeathState(ALIVE);
                    m_unitTarget->loot.clear();
                }
            }

            return;
        }

        WoWGuid wowGuid;
        wowGuid.Init(m_corpseTarget->getOwnerGuid());

        m_playerTarget = sObjectMgr.getPlayer(wowGuid.getGuidLowPart());
        if (!m_playerTarget) return;
    }

    if (m_playerTarget->isAlive() || !m_playerTarget->IsInWorld())
        return;
    //resurrect
    m_playerTarget->setResurrectMapId(p_caster->GetMapId());
    m_playerTarget->setResurrectInstanceId(p_caster->GetInstanceID());
    m_playerTarget->setResurrectPosition(p_caster->GetPosition());
    m_playerTarget->setResurrectHealth(damage);
    m_playerTarget->setResurrectMana(getSpellInfo()->getEffectMiscValue(effectIndex));

    SendResurrectRequest(m_playerTarget);
}

void Spell::SpellEffectAttackMe(uint8_t /*effectIndex*/)
{
    if (!m_unitTarget || !m_unitTarget->isAlive())
        return;

    if (!m_unitTarget->getThreatManager().canHaveThreatList())
        return;

    ThreatManager& mgr = m_unitTarget->getThreatManager();
    if (mgr.getCurrentVictim() == u_caster)
        return;

    if (!mgr.isThreatListEmpty())
    {
        mgr.addThreat(u_caster, 20.0f, nullptr, false, false);
        // Set threat equal to highest threat currently on target
        mgr.matchUnitThreatToHighestThreat(u_caster);

        // Call on AIInterface
        if (m_unitTarget->getAIInterface())
            m_unitTarget->getAIInterface()->eventOnTaunt(u_caster);
    }
}

void Spell::SpellEffectSkinPlayerCorpse(uint8_t /*effectIndex*/)
{
    Corpse* corpse = nullptr;
    if (!m_playerTarget)
    {
        // means we're "skinning" a corpse
        corpse = sObjectMgr.getCorpseByGuid((uint32_t)m_targets.getUnitTargetGuid());  // hacky
    }
    else if (m_playerTarget->getDeathState() == CORPSE)   // repopped while we were casting
    {
        corpse = sObjectMgr.getCorpseByGuid(m_playerTarget->getGuidLow());
    }

    if (p_caster == nullptr)
        return;

    if (m_playerTarget && !corpse)
    {
        if (!m_playerTarget->getBattleground() || !m_playerTarget->isDead())
            return;

        // Set all the lootable stuff on the player. If he repops before we've looted, we'll set the flags
        // on corpse then :p

        m_playerTarget->setLootableOnCorpse(false);
        m_playerTarget->removeUnitFlags(UNIT_FLAG_SKINNABLE);
        m_playerTarget->addDynamicFlags(U_DYN_FLAG_LOOTABLE);

        // Send the loot.
        p_caster->sendLoot(m_playerTarget->getGuid(), LOOT_SKINNING, m_playerTarget->GetMapId());

        // Send a message to the died player, telling him he has to resurrect at the graveyard.
        // Send an empty corpse location too, :P

        m_playerTarget->sendPacket(MsgCorspeQuery(0).serialise().get());

        // don't allow him to spawn a corpse
        m_playerTarget->setAllowedToCreateCorpse(false);

        // and.. force him to the graveyard and repop him.
        m_playerTarget->repopRequest();

    }
    else if (corpse)
    {
        // find the corpses' owner
        WoWGuid wowGuid;
        wowGuid.Init(corpse->getOwnerGuid());

        Player* owner = sObjectMgr.getPlayer(wowGuid.getGuidLowPart());
        if (owner)
        {
            if (!owner->getBattleground())
                return;

            owner->sendPacket(MsgCorspeQuery(0).serialise().get());
        }

        if (corpse->getDynamicFlags() != 1)
            corpse->setDynamicFlags(1); // sets it so you can loot the plyr

        // remove skinnable flag
        corpse->setFlags(CORPSE_FLAG_BONE | CORPSE_FLAG_UNK1);

        // remove owner association
        corpse->setOwnerNotifyMap(0);
        corpse->setCorpseState(CORPSE_STATE_BONES);

        // send loot
        p_caster->sendLoot(corpse->getGuid(), LOOT_SKINNING, corpse->GetMapId());

        corpse->deleteFromDB();
        sObjectMgr.addCorpseDespawnTime(corpse);
    }
}

void Spell::SpellEffectApplyPetAA(uint8_t effectIndex)
{
    ApplyAreaAura(effectIndex);
}

void Spell::SpellEffectDummyMelee(uint8_t /*effectIndex*/)   // Normalized Weapon damage +
{

    if (!m_unitTarget || !u_caster)
        return;

    switch (getSpellInfo()->getId())
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
                p_caster->clearComboPoints(); //some say that we should only remove 1 point per dodge. Due to cooldown you can't cast it twice anyway..
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
            uint32_t sunder_count = 0;
            SpellInfo const* spellInfo = nullptr;
            for (uint16_t x = AuraSlots::NEGATIVE_SLOT_START; x < AuraSlots::NEGATIVE_SLOT_END; ++x)
            {
                if (const auto* aur = m_unitTarget->getAuraWithAuraSlot(x))
                {
                    switch (aur->getSpellInfo()->getId())
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
                            spellInfo = aur->getSpellInfo();
                        } break;
                        default:
                            spellInfo = sSpellMgr.getSpellInfo(7386);
                            break;
                    }
                }
            }

            if (spellInfo == nullptr)
                return; //omg how did this happen ?
                        //we should also cast sunder armor effect on target with or without dmg
            Spell* spell = sSpellMgr.newSpell(u_caster, spellInfo, true, nullptr);
            spell->ProcedOnSpell = getSpellInfo();
            spell->pSpellId = getSpellInfo()->getId();
            SpellCastTargets targets(m_unitTarget->getGuid());
            spell->prepare(&targets);
            if (!sunder_count)
                return; //no damage = no joy
            damage = damage * sunder_count;
        } break;
        default:
            break;
    }

    switch (getSpellInfo()->getId())
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
                p_caster->addComboPoints(p_caster->getTargetGuid(), 1);
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
        if (getSpellInfo()->getEffect(x) == SPELL_EFFECT_WEAPON_PERCENT_DAMAGE)
        {
            add_damage = damage * (getSpellInfo()->getEffectBasePoints(x) + 1) / 100;
            return;
        }

    //rogue - mutilate ads dmg if target is poisoned
    uint32_t pct_dmg_mod = 100;
    if (m_unitTarget->isPoisoned())
    {
        switch (getSpellInfo()->getId())
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

    WeaponDamageType _type;
    if (GetType() == SPELL_DMG_TYPE_RANGED)
        _type = RANGED;
    else
    {
        if (getSpellInfo()->getAttributesExC() & ATTRIBUTESEXC_REQUIRES_OFFHAND_WEAPON)
            _type = OFFHAND;
        else
            _type = MELEE;
    }
    m_targetDamageInfo = u_caster->strike(m_unitTarget, _type, getSpellInfo(), damage, pct_dmg_mod, 0, m_triggeredSpell, true, isForcedCrit, this);
    isTargetDamageInfoSet = true;
}

void Spell::SpellEffectStartTaxi(uint8_t effectIndex)
{
    if (!m_playerTarget || !m_playerTarget->isAlive() || !u_caster)
        return;

    if (!m_unitTarget || !m_unitTarget->isPlayer())
        return;

    m_unitTarget->ToPlayer()->activateTaxiPathTo(m_spellInfo->getEffectMiscValue(effectIndex), m_spellInfo->getId());
}

void Spell::SpellEffectPlayerPull(uint8_t /*effectIndex*/)
{
    if (!m_unitTarget || !m_unitTarget->isAlive() || !m_unitTarget->isPlayer())
        return;

    Player* p_target = static_cast< Player* >(m_unitTarget);

    // calculate destination
    float pullD = p_target->CalcDistance(m_caster) - p_target->getBoundingRadius() - (u_caster ? u_caster->getBoundingRadius() : 0) - 1.0f;
    float pullO = p_target->calcRadAngle(p_target->GetPositionX(), p_target->GetPositionY(), m_caster->GetPositionX(), m_caster->GetPositionY());
    float pullX = p_target->GetPositionX() + pullD * cosf(pullO);
    float pullY = p_target->GetPositionY() + pullD * sinf(pullO);
    float pullZ = m_caster->GetPositionZ() + 0.3f;
    uint32_t time = uint32_t(pullD * 42.0f);

    p_target->SetOrientation(pullO);

    WorldPacket data(SMSG_MONSTER_MOVE, 60);
    data << p_target->GetNewGUID();
    data << uint8_t(0);
    data << p_target->GetPositionX();
    data << p_target->GetPositionY();
    data << p_target->GetPositionZ();
    data << Util::getMSTime();
    data << uint8_t(4);
    data << pullO;
    data << uint32_t(0x00001000);
    data << time;
    data << uint32_t(1);
    data << pullX;
    data << pullY;
    data << pullZ;

    p_target->sendMessageToSet(&data, true);
}

void Spell::SpellEffectReduceThreatPercent(uint8_t /*effectIndex*/)
{
    if (!m_unitTarget || !m_unitTarget->isCreature() || !u_caster || m_unitTarget->getThreatManager().getThreat(u_caster) == 0)
        return;

    m_unitTarget->getThreatManager().modifyThreatByPercent(u_caster, (int32_t)m_unitTarget->getThreatManager().getThreat(u_caster) * damage / 100);
}

void Spell::SpellEffectSpellSteal(uint8_t /*effectIndex*/)
{
    if (m_unitTarget == nullptr || u_caster == nullptr || !m_unitTarget->isAlive())
        return;

    if (m_playerTarget != nullptr && p_caster != nullptr && p_caster != m_playerTarget)
    {
        if (m_playerTarget->isPvpFlagSet())
            p_caster->togglePvP();
    }

    uint16_t start, end;
    if (u_caster->isValidTarget(m_unitTarget))
    {
        start = AuraSlots::POSITIVE_SLOT_START;
        end = AuraSlots::POSITIVE_SLOT_END;
    }
    else
        return;

    std::list< uint32_t > stealedSpells;

    for (auto x = start; x < end; x++)
    {
        if (auto* const aur = m_unitTarget->getAuraWithAuraSlot(x))
        {
            SpellInfo const* aursp = aur->getSpellInfo();

            if (aursp->getId() != 15007 && !aur->IsPassive()
                //              && aur->IsPositive()    // Zack : We are only checking positive auras. There is no meaning to check again
               ) //Nothing can dispel resurrection sickness
            {
                if (aursp->getDispelType() == DISPEL_MAGIC)
                {
                    stealedSpells.push_back(aursp->getId());

                    uint32_t aurdur = (aur->getTimeLeft() > 120000 ? 120000 : aur->getTimeLeft());
                    auto aura = sSpellMgr.newAura(aursp, aurdur, u_caster, u_caster);
                    m_unitTarget->removeAllAurasByIdReturnCount(aursp->getId());
                    for (uint8_t j = 0; j < 3; j++)
                    {
                        if (aura->getSpellInfo()->getEffect(j))
                        {
                            aura->addAuraEffect(static_cast<AuraEffect>(aura->getSpellInfo()->getEffectApplyAuraName(j)), aura->getSpellInfo()->getEffectBasePoints(j) + 1, aura->getSpellInfo()->getEffectMiscValue(j), aur->getAuraEffect(j)->getEffectPercentModifier(), true, j);
                        }
                    }
                    u_caster->addAura(std::move(aura));
                    break;
                }
            }
        }
    }

    if (!stealedSpells.empty())
    {
        m_caster->sendMessageToSet(SmsgSpellStealLog(m_caster->getGuid(), m_unitTarget->getGuid(), getSpellInfo()->getId(), stealedSpells).serialise().get(), true);
    }
}

void Spell::SpellEffectProspecting(uint8_t /*effectIndex*/)
{
    if (!p_caster) return;

    if (!m_itemTarget) // this should never happen
    {
        sendCastResult(SPELL_FAILED_CANT_BE_PROSPECTED);
        return;
    }

    //Fill Prospecting loot
    p_caster->setLootGuid(m_itemTarget->getGuid());
    if (!m_itemTarget->m_loot)
    {
        m_itemTarget->m_loot = std::make_unique<Loot>();
        sLootMgr.fillItemLoot(p_caster, m_itemTarget->m_loot.get(), m_itemTarget->getEntry(), 0);
    }

    if (m_itemTarget->m_loot->items.size() > 0)
    {
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "Successfully prospected item {}", uint32_t(m_itemTarget->getEntry()));
        p_caster->sendLoot(m_itemTarget->getGuid(), LOOT_PROSPECTING, p_caster->GetMapId());
    }
    else // this should never happen either
    {
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "Prospecting failed, item {} has no loot", uint32_t(m_itemTarget->getEntry()));
        sendCastResult(SPELL_FAILED_CANT_BE_PROSPECTED);
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
    if (!p_caster || !m_unitTarget)
        return;

    if ((m_unitTarget->isPlayer() && p_caster->getGroup() != static_cast< Player* >(m_unitTarget)->getGroup()) || (m_unitTarget->isCreature() && !m_unitTarget->isPet()))
        return;

    p_caster->setMisdirectionTarget(m_unitTarget->getGuid());

    // Threat Management
    p_caster->getThreatManager().registerRedirectThreat(m_spellInfo->getId(), m_unitTarget->getGuid(), uint32_t(damage));
}

void Spell::SpellEffectPlayMusic(uint8_t effectIndex)
{
    uint32_t soundid = m_spellInfo->getEffectMiscValue(effectIndex);

    if (soundid == 0)
    {
        sLogger.failure("Spell {} ({}) has no sound ID to play. Spell needs fixing!", m_spellInfo->getId(), m_spellInfo->getName());
        return;
    }

    m_caster->PlaySoundToSet(soundid);
}

void Spell::SpellEffectForgetSpecialization(uint8_t effectIndex)
{
    if (!m_playerTarget) return;

    uint32_t spellid = getSpellInfo()->getEffectTriggerSpell(effectIndex);
    m_playerTarget->removeSpell(spellid, false);

    sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "Player {} have forgot spell {} from spell {} (caster: {})", m_playerTarget->getGuidLow(), spellid, getSpellInfo()->getId(), m_caster->getGuidLow());
}

void Spell::SpellEffectKillCredit(uint8_t effectIndex)
{
    CreatureProperties const* ci = sMySQLStore.getCreatureProperties(getSpellInfo()->getEffectMiscValue(effectIndex));
    if (m_playerTarget != nullptr && ci != nullptr)
        sQuestMgr._OnPlayerKill(m_playerTarget, getSpellInfo()->getEffectMiscValue(effectIndex), false);
}

void Spell::SpellEffectRestorePowerPct(uint8_t effectIndex)
{
    if (u_caster == nullptr || m_unitTarget == nullptr || !m_unitTarget->isAlive())
        return;

    auto power_type = static_cast<PowerType>(getSpellInfo()->getEffectMiscValue(effectIndex));
    if (power_type >= TOTAL_PLAYER_POWER_TYPES)
    {
        sLogger.failure("Unhandled power type {} in {}, report this line to devs.", power_type, __FUNCTION__);
        return;
    }

    uint32_t amount = damage * m_unitTarget->getMaxPower(power_type) / 100;
    u_caster->energize(m_unitTarget, getSpellInfo()->getId(), amount, power_type);
}

void Spell::SpellEffectTriggerSpellWithValue(uint8_t effectIndex)
{
    if (!m_unitTarget) return;

    SpellInfo const* TriggeredSpell = sSpellMgr.getSpellInfo(getSpellInfo()->getEffectTriggerSpell(effectIndex));
    if (TriggeredSpell == nullptr)
        return;

    Spell* sp = sSpellMgr.newSpell(m_caster, TriggeredSpell, true, nullptr);

    for (uint8_t x = 0; x < 3; x++)
    {
        if (effectIndex == x)
            sp->forced_basepoints->set(x, damage);  //prayer of mending should inherit heal bonus ?
        else
            sp->forced_basepoints->set(x, TriggeredSpell->getEffectBasePoints(effectIndex));

    }

    SpellCastTargets tgt(m_unitTarget->getGuid());
    sp->prepare(&tgt);
}

void Spell::SpellEffectApplyOwnerAA(uint8_t effectIndex)
{
    ApplyAreaAura(effectIndex);
}

void Spell::SpellEffectCreatePet(uint8_t effectIndex)
{
    if (!m_playerTarget)
        return;

    if (m_playerTarget->getPet())
        m_playerTarget->getPet()->unSummon();

    CreatureProperties const* ci = sMySQLStore.getCreatureProperties(getSpellInfo()->getEffectMiscValue(effectIndex));
    if (ci)
    {
        const auto pet = sObjectMgr.createPet(getSpellInfo()->getEffectMiscValue(effectIndex), nullptr);
        if (!pet->createAsSummon(ci, nullptr, m_playerTarget, m_playerTarget->GetPosition(), 0, m_spellInfo, effectIndex, PET_TYPE_HUNTER))
        {
            pet->DeleteMe();//CreateAsSummon() returns false if an error occurred.
        }
    }
}

void Spell::SpellEffectTeachTaxiPath(uint8_t effectIndex)
{
    if (!m_playerTarget || !getSpellInfo()->getEffectTriggerSpell(effectIndex))
        return;

    uint32_t nodeid = m_spellInfo->getEffectMiscValue(effectIndex);
    if (sTaxiNodesStore.lookupEntry(nodeid))
    {
        m_playerTarget->getSession()->sendDiscoverNewTaxiNode(nodeid);
    }
}

void Spell::SpellEffectDualWield2H(uint8_t /*effectIndex*/)
{
    if (!m_playerTarget)
        return;

    m_playerTarget->setDualWield2H(true);
}

void Spell::SpellEffectEnchantItemPrismatic(uint8_t effectIndex)
{
#if VERSION_STRING < WotLK
    return;
#else
    if (!m_itemTarget || !p_caster)
        return;

    auto spell_item_enchant = sSpellItemEnchantmentStore.lookupEntry(m_spellInfo->getEffectMiscValue(effectIndex));

    if (!spell_item_enchant)
    {
        sLogger.failure("Invalid enchantment entry {} for Spell {}", getSpellInfo()->getEffectMiscValue(effectIndex), getSpellInfo()->getId());
        return;
    }

    if (p_caster->getSession()->hasPermissions())
        sGMLog.writefromsession(p_caster->getSession(), "enchanted item for %s", m_itemTarget->getOwner()->getName().c_str());

    //remove other socket enchant
    m_itemTarget->removeEnchantment(PRISMATIC_ENCHANTMENT_SLOT);
    const auto addedEnchantment = m_itemTarget->addEnchantment(m_spellInfo->getEffectMiscValue(effectIndex), PRISMATIC_ENCHANTMENT_SLOT, 0);

    if (!addedEnchantment)
        return; // Apply failed

    m_itemTarget->m_isDirty = true;
#endif
}

void Spell::SpellEffectCreateItem2(uint8_t effectIndex) // Create item
{
    ///\todo This spell effect has also a misc value - meaning is unknown yet
    if (p_caster == nullptr)
        return;

    uint32_t new_item_id = getSpellInfo()->getEffectItemType(effectIndex);

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

    if (!m_itemTarget) // this should never happen
    {
        sendCastResult(SPELL_FAILED_CANT_BE_PROSPECTED);
        return;
    }

    //Fill Prospecting loot
    p_caster->setLootGuid(m_itemTarget->getGuid());
    if (!m_itemTarget->m_loot)
    {
        m_itemTarget->m_loot = std::make_unique<Loot>();
        sLootMgr.fillItemLoot(p_caster, m_itemTarget->m_loot.get(), m_itemTarget->getEntry(), 0);
    }

    if (m_itemTarget->m_loot->items.size() > 0)
    {
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "Successfully milled item {}", uint32_t(m_itemTarget->getEntry()));
        p_caster->sendLoot(m_itemTarget->getGuid(), LOOT_MILLING, p_caster->GetMapId());
    }
    else // this should never happen either
    {
        sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "Milling failed, item {} has no loot", uint32_t(m_itemTarget->getEntry()));
        sendCastResult(SPELL_FAILED_CANT_BE_PROSPECTED);
    }
}

void Spell::SpellEffectRenamePet(uint8_t /*effectIndex*/)
{
    if (!m_unitTarget || !m_unitTarget->isPet() ||
        !static_cast< Pet* >(m_unitTarget)->getPlayerOwner() || static_cast< Pet* >(m_unitTarget)->getPlayerOwner()->getClass() != HUNTER)
        return;

#if VERSION_STRING == Classic
    m_unitTarget->addUnitFlags(UNIT_FLAG_PET_CAN_BE_RENAMED);
#else
    m_unitTarget->addPetFlags(PET_FLAG_CAN_BE_RENAMED);
#endif
}

void Spell::SpellEffectRestoreHealthPct(uint8_t /*effectIndex*/)
{
    if (m_unitTarget == nullptr || !m_unitTarget->isAlive())
        return;

    m_unitTarget->addSimpleHealingBatchEvent(Util::float2int32(damage * m_unitTarget->getMaxHealth() / 100.0f), u_caster, pSpellId != 0 ? sSpellMgr.getSpellInfo(pSpellId) : getSpellInfo());
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
#ifndef FT_DUAL_SPEC
    return;
#else
    if (p_caster == nullptr)
        return;

    if (p_caster->getCombatHandler().isInCombat())
    {
        sendCastResult(SPELL_FAILED_AFFECTING_COMBAT);
        return;
    }
    else if (p_caster->getBattleground())
    {
        if (p_caster->getBattleground()->isArena())
        {
            sendCastResult(SPELL_FAILED_AFFECTING_COMBAT); // does the job
            return;
        }
        else
        {
            if (p_caster->getBattleground()->hasStarted())
                sendCastResult(SPELL_FAILED_AFFECTING_COMBAT); // does the job
        }
    }

    // TODO: check if player even have learnt secondary spec
    uint8_t NewSpec = p_caster->m_talentActiveSpec == SPEC_PRIMARY ? SPEC_SECONDARY : SPEC_PRIMARY; // Check if primary spec is on or not
    p_caster->activateTalentSpec(NewSpec);
#endif
}

void Spell::SpellEffectDurabilityDamage(uint8_t effectIndex)
{
    if (!m_unitTarget || !m_unitTarget->isPlayer())
        return;

    int16_t slot = int16_t(getSpellInfo()->getEffectMiscValue(effectIndex));

    Item* pItem;
    Container* pContainer;
    uint32_t j, k;

    // FIXME: some spells effects have value -1/-2
    // Possibly its mean -1 all player equipped items and -2 all items
    if (slot < 0)
    {
        for (k = 0; k < MAX_INVENTORY_SLOT; k++)
        {
            pItem = p_caster->getItemInterface()->GetInventoryItem(static_cast<uint16_t>(k));
            if (pItem != nullptr)
            {
                if (pItem->isContainer())
                {
                    pContainer = static_cast< Container* >(pItem);
                    for (j = 0; j < pContainer->getItemProperties()->ContainerSlots; ++j)
                    {
                        pItem = pContainer->getItem(static_cast<uint16_t>(j));
                        if (pItem != nullptr)
                        {
                            uint32_t maxdur = pItem->getMaxDurability();
                            uint32_t olddur = pItem->getDurability();
                            uint32_t newdur = (olddur)-(damage);

                            if (static_cast<int32_t>(newdur) < 0)
                                newdur = 0;

                            if (newdur > maxdur)
                                newdur = maxdur;

                            pItem->setDurability(newdur);
                        }
                    }
                }
                else
                {
                    uint32_t maxdur = pItem->getMaxDurability();
                    uint32_t olddur = pItem->getDurability();
                    uint32_t newdur = (olddur)-(damage);

                    if (static_cast<int32_t>(newdur) < 0)
                        newdur = 0;

                    if (newdur > maxdur)
                        newdur = maxdur;

                    // Apply / Disapply enchantements from this item
                    pItem->setDurability(newdur);
                    if (newdur == 0 && olddur > 0)
                        p_caster->applyItemMods(pItem, static_cast<uint16_t>(k), false);
                    else if (newdur > 0 && olddur == 0)
                        p_caster->applyItemMods(pItem, static_cast<uint16_t>(k), true);
                }
            }
        }
        return;
    }

    // invalid slot value
    if (slot >= INVENTORY_SLOT_BAG_END)
        return;

    pItem = p_caster->getItemInterface()->GetInventoryItem(slot);
    if (pItem)
    {
        uint32_t maxdur = pItem->getMaxDurability();
        uint32_t olddur = pItem->getDurability();
        uint32_t newdur = (olddur)-(damage);

        if (static_cast<int32_t>(newdur) < 0)
            newdur = 0;

        if (newdur > maxdur)
            newdur = maxdur;

        pItem->setDurability(newdur);

        // Apply / Disapply enchantements from this item
        if (newdur == 0 && olddur > 0)
            p_caster->applyItemMods(pItem, slot, false);
        else if (newdur > 0 && olddur == 0)
            p_caster->applyItemMods(pItem, slot, true);
    }
}

void Spell::SpellEffectDurabilityDamagePCT(uint8_t effectIndex)
{
    if (!m_unitTarget || !m_unitTarget->isPlayer())
        return;

    int16_t slot = int16_t(getSpellInfo()->getEffectMiscValue(effectIndex));

    Item* pItem;
    Container* pContainer;
    uint32_t j, k;

    // FIXME: some spells effects have value -1/-2
    // Possibly its mean -1 all player equipped items and -2 all items
    if (slot < 0)
    {
        for (k = 0; k < MAX_INVENTORY_SLOT; ++k)
        {
            pItem = p_caster->getItemInterface()->GetInventoryItem(static_cast<uint16_t>(k));
            if (pItem != nullptr)
            {
                if (pItem->isContainer())
                {
                    pContainer = static_cast< Container* >(pItem);
                    for (j = 0; j < pContainer->getItemProperties()->ContainerSlots; ++j)
                    {
                        pItem = pContainer->getItem(static_cast<uint16_t>(j));
                        if (pItem != nullptr)
                        {
                            uint32_t maxdur = pItem->getMaxDurability();
                            uint32_t olddur = pItem->getDurability();
                            uint32_t newdur = (olddur - (uint32_t)(maxdur * (damage / 100.0)));

                            if (static_cast<int32_t>(newdur) < 0)
                                newdur = 0;

                            if (newdur > maxdur)
                                newdur = maxdur;

                            pItem->setDurability(newdur);
                        }
                    }
                }
                else
                {
                    uint32_t maxdur = pItem->getMaxDurability();
                    uint32_t olddur = pItem->getDurability();
                    uint32_t newdur = (olddur - (uint32_t)(maxdur * (damage / 100.0)));

                    if (static_cast<int32_t>(newdur) < 0)
                        newdur = 0;

                    if (newdur > maxdur)
                        newdur = maxdur;

                    // Apply / Disapply enchantements from this item
                    pItem->setDurability(newdur);
                    if (newdur == 0 && olddur > 0)
                        p_caster->applyItemMods(pItem, static_cast<uint16_t>(k), false);
                    else if (newdur > 0 && olddur == 0)
                        p_caster->applyItemMods(pItem, static_cast<uint16_t>(k), true);
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

    pItem = p_caster->getItemInterface()->GetInventoryItem(slot);
    if (pItem)
    {
        uint32_t maxdur = pItem->getMaxDurability();
        uint32_t olddur = pItem->getDurability();
        uint32_t newdur = (olddur - (uint32_t)(maxdur * (damage / 100.0)));

        if (static_cast<int32_t>(newdur) < 0)
            newdur = 0;

        if (newdur > maxdur)
            newdur = maxdur;

        pItem->setDurability(newdur);

        // Apply / Disapply enchantements from this item
        if (newdur == 0 && olddur > 0)
            p_caster->applyItemMods(pItem, slot, false);
        else if (newdur > 0 && olddur == 0)
            p_caster->applyItemMods(pItem, slot, true);
    }
}

void Spell::SpellEffectActivateRunes(uint8_t effectIndex)
{
    if (p_caster == nullptr || !p_caster->isClassDeathKnight())
        return;

    DeathKnight* dk = static_cast<DeathKnight*>(p_caster);

    uint32_t count = damage;
    if (!count)
        count = 1;

    for (uint8_t x = 0; x < MAX_RUNES && count; ++x)
    {
        if (dk->GetRuneType(x) == getSpellInfo()->getEffectMiscValue(effectIndex) && dk->GetRuneIsUsed(x))
        {
            dk->ResetRune(x);
            --count;
        }
    }
}

void Spell::SpellEffectForceDeselect(uint8_t /*effectIndex*/)
{
    //clear focus SMSG_BREAK_TARGET

    //clear target
    m_caster->sendMessageToSet(SmsgClearTarget(m_caster->getGuid()).serialise().get(), true);

    //stop attacking and pet target
}
