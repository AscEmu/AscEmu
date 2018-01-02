/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
#include "Units/Creatures/Creature.h"
#include "Units/Summons/Summon.h"
#include "Management/Item.h"
#include "Management/ItemInterface.h"
#include "Units/Stats.h"
#include "Management/Battleground/Battleground.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Players/PlayerClasses.hpp"
#include "Map/MapMgr.h"
#include "Objects/Faction.h"
#include "SpellAuras.h"
#include "Definitions/SpellModifierType.h"
#include "SpellHelpers.h"
#include "Definitions/ProcFlags.h"
#include "Definitions/AuraInterruptFlags.h"
#include "Definitions/SpellSchoolConversionTable.h"
#include "Definitions/SpellTypes.h"
#include "Definitions/SpellIsFlags.h"
#include "Definitions/SpellState.h"
#include "Definitions/SpellMechanics.h"
#include "Definitions/PowerType.h"
#include "Customization/SpellCustomizations.hpp"
#include "Units/Creatures/Pet.h"

using ascemu::World::Spell::Helpers::decimalToMask;
using ascemu::World::Spell::Helpers::spellModFlatFloatValue;
using ascemu::World::Spell::Helpers::spellModFlatIntValue;
using ascemu::World::Spell::Helpers::spellModPercentageFloatValue;
using ascemu::World::Spell::Helpers::spellModPercentageIntValue;

pSpellAura SpellAuraHandler[TOTAL_SPELL_AURAS] =
{
    &Aura::SpellAuraNULL,                                                   //   0 SPELL_AURA_NONE
    &Aura::SpellAuraBindSight,                                              //   1 SPELL_AURA_BIND_SIGHT
    &Aura::SpellAuraModPossess,                                             //   2 SPELL_AURA_MOD_POSSESS
    &Aura::SpellAuraPeriodicDamage,                                         //   3 SPELL_AURA_PERIODIC_DAMAGE
    &Aura::SpellAuraDummy,                                                  //   4 SPELL_AURA_DUMMY
    &Aura::SpellAuraModConfuse,                                             //   5 SPELL_AURA_MOD_CONFUSE
    &Aura::SpellAuraModCharm,                                               //   6 SPELL_AURA_MOD_CHARM
    &Aura::SpellAuraModFear,                                                //   7 SPELL_AURA_MOD_FEAR
    &Aura::SpellAuraPeriodicHeal,                                           //   8 SPELL_AURA_PERIODIC_HEAL
    &Aura::SpellAuraModAttackSpeed,                                         //   9 SPELL_AURA_MOD_ATTACKSPEED
    &Aura::SpellAuraModThreatGenerated,                                     //  10 SPELL_AURA_MOD_THREAT
    &Aura::SpellAuraModTaunt,                                               //  11 SPELL_AURA_MOD_TAUNT
    &Aura::SpellAuraModStun,                                                //  12 SPELL_AURA_MOD_STUN
    &Aura::SpellAuraModDamageDone,                                          //  13 SPELL_AURA_MOD_DAMAGE_DONE
    &Aura::SpellAuraModDamageTaken,                                         //  14 SPELL_AURA_MOD_DAMAGE_TAKEN
    &Aura::SpellAuraDamageShield,                                           //  15 SPELL_AURA_DAMAGE_SHIELD
    &Aura::SpellAuraModStealth,                                             //  16 SPELL_AURA_MOD_STEALTH
    &Aura::SpellAuraModDetect,                                              //  17 SPELL_AURA_MOD_DETECT
    &Aura::SpellAuraModInvisibility,                                        //  18 SPELL_AURA_MOD_INVISIBILITY
    &Aura::SpellAuraModInvisibilityDetection,                               //  19 SPELL_AURA_MOD_INVISIBILITY_DETECTION
    &Aura::SpellAuraModTotalHealthRegenPct,                                 //  20 SPELL_AURA_MOD_TOTAL_HEALTH_REGEN_PCT
    &Aura::SpellAuraModTotalManaRegenPct,                                   //  21 SPELL_AURA_MOD_TOTAL_MANA_REGEN_PCT
    &Aura::SpellAuraModResistance,                                          //  22 SPELL_AURA_MOD_RESISTANCE
    &Aura::SpellAuraPeriodicTriggerSpell,                                   //  23 SPELL_AURA_PERIODIC_TRIGGER_SPELL
    &Aura::SpellAuraPeriodicEnergize,                                       //  24 SPELL_AURA_PERIODIC_ENERGIZE
    &Aura::SpellAuraModPacify,                                              //  25 SPELL_AURA_MOD_PACIFY
    &Aura::SpellAuraModRoot,                                                //  26 SPELL_AURA_MOD_ROOT
    &Aura::SpellAuraModSilence,                                             //  27 SPELL_AURA_MOD_SILENCE
    &Aura::SpellAuraReflectSpells,                                          //  28 SPELL_AURA_REFLECT_SPELLS
    &Aura::SpellAuraModStat,                                                //  29 SPELL_AURA_MOD_STAT
    &Aura::SpellAuraModSkill,                                               //  30 SPELL_AURA_MOD_SKILL
    &Aura::SpellAuraModIncreaseSpeed,                                       //  31 SPELL_AURA_MOD_INCREASE_SPEED
    &Aura::SpellAuraModIncreaseMountedSpeed,                                //  32 SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED
    &Aura::SpellAuraModDecreaseSpeed,                                       //  33 SPELL_AURA_MOD_DECREASE_SPEED
    &Aura::SpellAuraModIncreaseHealth,                                      //  34 SPELL_AURA_MOD_INCREASE_HEALTH
    &Aura::SpellAuraModIncreaseEnergy,                                      //  35 SPELL_AURA_MOD_INCREASE_ENERGY
    &Aura::SpellAuraModShapeshift,                                          //  36 SPELL_AURA_MOD_SHAPESHIFT
    &Aura::SpellAuraModEffectImmunity,                                      //  37 SPELL_AURA_EFFECT_IMMUNITY
    &Aura::SpellAuraModStateImmunity,                                       //  38 SPELL_AURA_STATE_IMMUNITY
    &Aura::SpellAuraModSchoolImmunity,                                      //  39 SPELL_AURA_SCHOOL_IMMUNITY
    &Aura::SpellAuraModDmgImmunity,                                         //  40 SPELL_AURA_DAMAGE_IMMUNITY
    &Aura::SpellAuraModDispelImmunity,                                      //  41 SPELL_AURA_DISPEL_IMMUNITY
    &Aura::SpellAuraProcTriggerSpell,                                       //  42 SPELL_AURA_PROC_TRIGGER_SPELL
    &Aura::SpellAuraProcTriggerDamage,                                      //  43 SPELL_AURA_PROC_TRIGGER_DAMAGE
    &Aura::SpellAuraTrackCreatures,                                         //  44 SPELL_AURA_TRACK_CREATURES
    &Aura::SpellAuraTrackResources,                                         //  45 SPELL_AURA_TRACK_RESOURCES
    &Aura::SpellAuraNULL,                                                   //  46 SPELL_AURA_MOD_PARRY_SKILL, obsolete? not used in 1.12.1 spell.dbc
    &Aura::SpellAuraModParryPerc,                                           //  47 SPELL_AURA_MOD_PARRY_PERCENT
    &Aura::SpellAuraNULL,                                                   //  48 SPELL_AURA_MOD_DODGE_SKILL obsolete?
    &Aura::SpellAuraModDodgePerc,                                           //  49 SPELL_AURA_MOD_DODGE_PERCENT
    &Aura::SpellAuraNULL,                                                   //  50 SPELL_AURA_MOD_BLOCK_SKILL obsolete?,
    &Aura::SpellAuraModBlockPerc,                                           //  51 SPELL_AURA_MOD_BLOCK_PERCENT
    &Aura::SpellAuraModCritPerc,                                            //  52 SPELL_AURA_MOD_CRIT_PERCENT
    &Aura::SpellAuraPeriodicLeech,                                          //  53 SPELL_AURA_PERIODIC_LEECH
    &Aura::SpellAuraModHitChance,                                           //  54 SPELL_AURA_MOD_HIT_CHANCE
    &Aura::SpellAuraModSpellHitChance,                                      //  55 SPELL_AURA_MOD_SPELL_HIT_CHANCE
    &Aura::SpellAuraTransform,                                              //  56 SPELL_AURA_TRANSFORM
    &Aura::SpellAuraModSpellCritChance,                                     //  57 SPELL_AURA_MOD_SPELL_CRIT_CHANCE
    &Aura::SpellAuraIncreaseSwimSpeed,                                      //  58 SPELL_AURA_MOD_INCREASE_SWIM_SPEED
    &Aura::SpellAuraModCratureDmgDone,                                      //  59 SPELL_AURA_MOD_DAMAGE_DONE_CREATURE
    &Aura::SpellAuraPacifySilence,                                          //  60 SPELL_AURA_MOD_PACIFY_SILENCE
    &Aura::SpellAuraModScale,                                               //  61 SPELL_AURA_MOD_SCALE
    &Aura::SpellAuraPeriodicHealthFunnel,                                   //  62 SPELL_AURA_PERIODIC_HEALTH_FUNNEL
    &Aura::SpellAuraNULL,                                                   //  63 SPELL_AURA_PERIODIC_MANA_FUNNEL obsolete?
    &Aura::SpellAuraPeriodicManaLeech,                                      //  64 SPELL_AURA_PERIODIC_MANA_LEECH
    &Aura::SpellAuraModCastingSpeed,                                        //  65 SPELL_AURA_MOD_CASTING_SPEED
    &Aura::SpellAuraFeignDeath,                                             //  66 SPELL_AURA_FEIGN_DEATH
    &Aura::SpellAuraModDisarm,                                              //  67 SPELL_AURA_MOD_DISARM
    &Aura::SpellAuraModStalked,                                             //  68 SPELL_AURA_MOD_STALKED
    &Aura::SpellAuraSchoolAbsorb,                                           //  69 SPELL_AURA_SCHOOL_ABSORB
    &Aura::SpellAuraNULL,                                                   //  70 SPELL_AURA_EXTRA_ATTACKS obsolete?
    &Aura::SpellAuraModSpellCritChanceSchool,                               //  71 SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
    &Aura::SpellAuraModPowerCost,                                           //  72 SPELL_AURA_MOD_POWER_COST
    &Aura::SpellAuraModPowerCostSchool,                                     //  73 SPELL_AURA_MOD_POWER_COST_SCHOOL
    &Aura::SpellAuraReflectSpellsSchool,                                    //  74 SPELL_AURA_REFLECT_SPELLS_SCHOOL
    &Aura::SpellAuraModLanguage,                                            //  75 SPELL_AURA_MOD_LANGUAGE
    &Aura::SpellAuraAddFarSight,                                            //  76 SPELL_AURA_FAR_SIGHT
    &Aura::SpellAuraMechanicImmunity,                                       //  77 SPELL_AURA_MECHANIC_IMMUNITY
    &Aura::SpellAuraMounted,                                                //  78 SPELL_AURA_MOUNTED
    &Aura::SpellAuraModDamagePercDone,                                      //  79 SPELL_AURA_MOD_DAMAGE_PERCENT_DONE
    &Aura::SpellAuraModPercStat,                                            //  80 SPELL_AURA_MOD_PERCENT_STAT
    &Aura::SpellAuraSplitDamage,                                            //  81 SPELL_AURA_SPLIT_DAMAGE
    &Aura::SpellAuraWaterBreathing,                                         //  82 SPELL_AURA_WATER_BREATHING
    &Aura::SpellAuraModBaseResistance,                                      //  83 SPELL_AURA_MOD_BASE_RESISTANCE
    &Aura::SpellAuraModRegen,                                               //  84 SPELL_AURA_MOD_REGEN
    &Aura::SpellAuraModPowerRegen,                                          //  85 SPELL_AURA_MOD_POWER_REGEN
    &Aura::SpellAuraChannelDeathItem,                                       //  86 SPELL_AURA_CHANNEL_DEATH_ITEM
    &Aura::SpellAuraModDamagePercTaken,                                     //  87 SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN
    &Aura::SpellAuraModRegenPercent,                                        //  88 SPELL_AURA_MOD_PERCENT_REGEN
    &Aura::SpellAuraPeriodicDamagePercent,                                  //  89 SPELL_AURA_PERIODIC_DAMAGE_PERCENT
    &Aura::SpellAuraModResistChance,                                        //  90 SPELL_AURA_MOD_RESIST_CHANCE
    &Aura::SpellAuraModDetectRange,                                         //  91 SPELL_AURA_MOD_DETECT_RANGE
    &Aura::SpellAuraPreventsFleeing,                                        //  92 SPELL_AURA_PREVENTS_FLEEING
    &Aura::SpellAuraModUnattackable,                                        //  93 SPELL_AURA_MOD_UNATTACKABLE
    &Aura::SpellAuraInterruptRegen,                                         //  94 SPELL_AURA_INTERRUPT_REGEN
    &Aura::SpellAuraGhost,                                                  //  95 SPELL_AURA_GHOST
    &Aura::SpellAuraMagnet,                                                 //  96 SPELL_AURA_SPELL_MAGNET
    &Aura::SpellAuraManaShield,                                             //  97 SPELL_AURA_MANA_SHIELD
    &Aura::SpellAuraSkillTalent,                                            //  98 SPELL_AURA_MOD_SKILL_TALENT
    &Aura::SpellAuraModAttackPower,                                         //  99 SPELL_AURA_MOD_ATTACK_POWER
    &Aura::SpellAuraVisible,                                                // 100 SPELL_AURA_AURAS_VISIBLE
    &Aura::SpellAuraModResistancePCT,                                       // 101 SPELL_AURA_MOD_RESISTANCE_PCT
    &Aura::SpellAuraModCreatureAttackPower,                                 // 102 SPELL_AURA_MOD_CREATURE_ATTACK_POWER
    &Aura::SpellAuraModTotalThreat,                                         // 103 SPELL_AURA_MOD_TOTAL_THREAT
    &Aura::SpellAuraWaterWalk,                                              // 104 SPELL_AURA_WATER_WALK
    &Aura::SpellAuraFeatherFall,                                            // 105 SPELL_AURA_FEATHER_FALL
    &Aura::SpellAuraHover,                                                  // 106 SPELL_AURA_HOVER
    &Aura::SpellAuraAddFlatModifier,                                        // 107 SPELL_AURA_ADD_FLAT_MODIFIER
    &Aura::SpellAuraAddPctMod,                                              // 108 SPELL_AURA_ADD_PCT_MODIFIER
    &Aura::SpellAuraAddClassTargetTrigger,                                  // 109 SPELL_AURA_ADD_CLASS_TARGET_TRIGGER
    &Aura::SpellAuraModPowerRegPerc,                                        // 110 SPELL_AURA_MOD_POWER_REGEN_PERCENT
    &Aura::SpellAuraNULL,                                                   // 111 SPELL_AURA_ADD_CASTER_HIT_TRIGGER
    &Aura::SpellAuraOverrideClassScripts,                                   // 112 SPELL_AURA_OVERRIDE_CLASS_SCRIPTS
    &Aura::SpellAuraModRangedDamageTaken,                                   // 113 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN
    &Aura::SpellAuraNULL,                                                   // 114 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT obadac na mangosie
    &Aura::SpellAuraModHealing,                                             // 115 SPELL_AURA_MOD_HEALING
    &Aura::SpellAuraIgnoreRegenInterrupt,                                   // 116 SPELL_AURA_IGNORE_REGEN_INTERRUPT
    &Aura::SpellAuraModMechanicResistance,                                  // 117 SPELL_AURA_MOD_MECHANIC_RESISTANCE
    &Aura::SpellAuraModHealingPCT,                                          // 118 SPELL_AURA_MOD_HEALING_PCT
    &Aura::SpellAuraNULL,                                                   // 119 SPELL_AURA_SHARE_PET_TRACKING obsolete
    &Aura::SpellAuraUntrackable,                                            // 120 SPELL_AURA_UNTRACKABLE
    &Aura::SpellAuraEmphaty,                                                // 121 SPELL_AURA_EMPATHY
    &Aura::SpellAuraModOffhandDamagePCT,                                    // 122 SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT
    &Aura::SpellAuraModPenetration,                                         // 123 SPELL_AURA_MOD_POWER_COST_PCT --> armor penetration & spell penetration, NOT POWER COST!
    &Aura::SpellAuraModRangedAttackPower,                                   // 124 SPELL_AURA_MOD_RANGED_ATTACK_POWER
    &Aura::SpellAuraModMeleeDamageTaken,                                    // 125 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN
    &Aura::SpellAuraModMeleeDamageTakenPct,                                 // 126 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT
    &Aura::SpellAuraRAPAttackerBonus,                                       // 127 SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS
    &Aura::SpellAuraModPossessPet,                                          // 128 SPELL_AURA_MOD_POSSESS_PET
    &Aura::SpellAuraModIncreaseSpeedAlways,                                 // 129 SPELL_AURA_MOD_INCREASE_SPEED_ALWAYS
    &Aura::SpellAuraModIncreaseMountedSpeed,                                // 130 SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS
    &Aura::SpellAuraModCreatureRangedAttackPower,                           // 131 SPELL_AURA_MOD_CREATURE_RANGED_ATTACK_POWER
    &Aura::SpellAuraModIncreaseEnergyPerc,                                  // 132 SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT
    &Aura::SpellAuraModIncreaseHealthPerc,                                  // 133 SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT
    &Aura::SpellAuraModManaRegInterrupt,                                    // 134 SPELL_AURA_MOD_MANA_REGEN_INTERRUPT
    &Aura::SpellAuraModHealingDone,                                         // 135 SPELL_AURA_MOD_HEALING_DONE
    &Aura::SpellAuraModHealingDonePct,                                      // 136 SPELL_AURA_MOD_HEALING_DONE_PERCENT
    &Aura::SpellAuraModTotalStatPerc,                                       // 137 SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE
    &Aura::SpellAuraModHaste,                                               // 138 SPELL_AURA_MOD_HASTE
    &Aura::SpellAuraForceReaction,                                          // 139 SPELL_AURA_FORCE_REACTION
    &Aura::SpellAuraModRangedHaste,                                         // 140 SPELL_AURA_MOD_RANGED_HASTE
    &Aura::SpellAuraModRangedAmmoHaste,                                     // 141 SPELL_AURA_MOD_RANGED_AMMO_HASTE
    &Aura::SpellAuraModBaseResistancePerc,                                  // 142 SPELL_AURA_MOD_BASE_RESISTANCE_PCT
    &Aura::SpellAuraModResistanceExclusive,                                 // 143 SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE
    &Aura::SpellAuraSafeFall,                                               // 144 SPELL_AURA_SAFE_FALL
    &Aura::SpellAuraNULL,                                                   // 145 SPELL_AURA_CHARISMA obsolete?
    &Aura::SpellAuraNULL,                                                   // 146 SPELL_AURA_PERSUADED obsolete
    &Aura::SpellAuraNULL,                                                   // 147 SPELL_AURA_ADD_CREATURE_IMMUNITY http://wow.allakhazam.com/db/spell.html?wspell=36798
    &Aura::SpellAuraRetainComboPoints,                                      // 148 SPELL_AURA_RETAIN_COMBO_POINTS
    &Aura::SpellAuraResistPushback,                                         // 149 SPELL_AURA_RESIST_PUSHBACK Resist Pushback //Simply resist spell casting delay
    &Aura::SpellAuraModShieldBlockPCT,                                      // 150 SPELL_AURA_MOD_SHIELD_BLOCK_PCT Mod Shield Absorbed dmg %
    &Aura::SpellAuraTrackStealthed,                                         // 151 SPELL_AURA_TRACK_STEALTHED Track Stealthed
    &Aura::SpellAuraModDetectedRange,                                       // 152 SPELL_AURA_MOD_DETECTED_RANGE Mod Detected Range
    &Aura::SpellAuraSplitDamageFlat,                                        // 153 SPELL_AURA_SPLIT_DAMAGE_FLAT Split Damage Flat
    &Aura::SpellAuraModStealthLevel,                                        // 154 SPELL_AURA_MOD_STEALTH_LEVEL Stealth Level Modifier
    &Aura::SpellAuraModUnderwaterBreathing,                                 // 155 SPELL_AURA_MOD_WATER_BREATHING Mod Water Breathing
    &Aura::SpellAuraModReputationAdjust,                                    // 156 SPELL_AURA_MOD_REPUTATION_ADJUST Mod Reputation Gain
    &Aura::SpellAuraNULL,                                                   // 157 SPELL_AURA_PET_DAMAGE_MULTI Mod Pet Damage
    &Aura::SpellAuraModBlockValue,                                          // 158 SPELL_AURA_MOD_SHIELD_BLOCKVALUE used Apply Aura: Mod Shield Block //http://www.thottbot.com/?sp=25036
    &Aura::SpellAuraNoPVPCredit,                                            // 159 SPELL_AURA_NO_PVP_CREDIT used Apply Aura: No PVP Credit http://www.thottbot.com/?sp=2479
    &Aura::SpellAuraNULL,                                                   // 160 SPELL_AURA_MOD_AOE_AVOIDANCE Apply Aura: Mod Side/Rear PBAE Damage Taken %//used http://www.thottbot.com/?sp=23198
    &Aura::SpellAuraModHealthRegInCombat,                                   // 161 SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT
    &Aura::SpellAuraPowerBurn,                                              // 162 missing used //Apply Aura: Power Burn (Mana) //http://www.thottbot.com/?sp=19659
    &Aura::SpellAuraModCritDmgPhysical,                                     // 163 missing Apply Aura: Mod Crit Damage Bonus (Physical)
    &Aura::SpellAuraNULL,                                                   // 164 missing used //test spell
    &Aura::SpellAuraAPAttackerBonus,                                        // 165 SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS	// Melee AP Attacker Bonus
    &Aura::SpellAuraModPAttackPower,                                        // 166 missing used //Apply Aura: Mod Attack Power % // http://www.thottbot.com/?sp=30803
    &Aura::SpellAuraModRangedAttackPowerPct,                                // 167 missing http://www.thottbot.com/s34485
    &Aura::SpellAuraIncreaseDamageTypePCT,                                  // 168 missing used //Apply Aura: Increase Damage % *type* //http://www.thottbot.com/?sp=24991
    &Aura::SpellAuraIncreaseCricticalTypePCT,                               // 169 missing used //Apply Aura: Increase Critical % *type* //http://www.thottbot.com/?sp=24293
    &Aura::SpellAuraNULL,                                                   // 170 SPELL_AURA_DETECT_AMORE //used //Apply Aura: Detect Amore //http://www.thottbot.com/?sp=26802
    &Aura::SpellAuraIncreasePartySpeed,                                     // 171 SPELL_AURA_MOD_SPEED_NOT_STACK
    &Aura::SpellAuraIncreaseMovementAndMountedSpeed,                        // 172 SPELL_AURA_MOD_MOUNTED_SPEED_NOT_STACK //used //Apply Aura: Increase Movement and Mounted Speed (Non-Stacking) //http://www.thottbot.com/?sp=26022 2e effect
    &Aura::SpellAuraNULL,                                                   // 173 SPELL_AURA_ALLOW_CHAMPION_SPELLS // Apply Aura: Allow Champion Spells
    &Aura::SpellAuraIncreaseSpellDamageByAttribute,                         // 174 SPELL_AURA_MOD_SPELL_DAMAGE_OF_STAT_PERCENT //used //Apply Aura: Increase Spell Damage by % Spirit (Spells) //http://www.thottbot.com/?sp=15031
    &Aura::SpellAuraIncreaseHealingByAttribute,                             // 175 SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT //used //Apply Aura: Increase Spell Healing by % Spirit //http://www.thottbot.com/?sp=15031
    &Aura::SpellAuraSpiritOfRedemption,                                     // 176 SPELL_AURA_SPIRIT_OF_REDEMPTION //used // Apply Aura: Spirit of Redemption
    &Aura::SpellAuraNULL,                                                   // 177 SPELL_AURA_AOE_CHARM //used //Apply Aura: Area Charm // http://www.thottbot.com/?sp=26740
    &Aura::SpellAuraNULL,                                                   // 178 missing //Apply Aura: Increase Debuff Resistance mangos
    &Aura::SpellAuraIncreaseAttackerSpellCrit,                              // 179 SPELL_AURA_INCREASE_ATTACKER_SPELL_CRIT//Apply Aura: Increase Attacker Spell Crit % *type* //http://www.thottbot.com/?sp=12579
    &Aura::SpellAuraNULL,                                                   // 180 missing //used //Apply Aura: Increase Spell Damage *type* //http://www.thottbot.com/?sp=29113 mangos
    &Aura::SpellAuraNULL,                                                   // 181 missing
    &Aura::SpellAuraIncreaseArmorByPctInt,                                  // 182 missing //used //Apply Aura: Increase Armor by % of Intellect //http://www.thottbot.com/?sp=28574  SPELL_AURA_INC_ARMOR_BY_PCT_INT
    &Aura::SpellAuraNULL,                                                   // 183 SPELL_AURA_MOD_CRITICAL_THREAT = 183 //used //Apply Aura: Decrease Critical Threat by % (Spells) //http://www.thottbot.com/?sp=28746
    &Aura::SpellAuraReduceAttackerMHitChance,                               // 184 //Apply Aura: Reduces Attacker Chance to Hit with Melee //http://www.thottbot.com/s31678
    &Aura::SpellAuraReduceAttackerRHitChance,                               // 185 //Apply Aura: Reduces Attacker Chance to Hit with Ranged //http://www.thottbot.com/?sp=30895
    &Aura::SpellAuraReduceAttackerSHitChance,                               // 186 //Apply Aura: Reduces Attacker Chance to Hit with Spells (Spells) //http://www.thottbot.com/?sp=30895
    &Aura::SpellAuraReduceEnemyMCritChance,                                 // 187 missing //used //Apply Aura: Reduces Attacker Chance to Crit with Melee (Ranged?) //http://www.thottbot.com/?sp=30893
    &Aura::SpellAuraReduceEnemyRCritChance,                                 // 188 missing //used //Apply Aura: Reduces Attacker Chance to Crit with Ranged (Melee?) //http://www.thottbot.com/?sp=30893
    &Aura::SpellAuraIncreaseRating,                                         // 189 missing //Apply Aura: Increases Rating
    &Aura::SpellAuraIncreaseRepGainPct,                                     // 190 SPELL_AURA_MOD_FACTION_REPUTATION_GAIN //used // Apply Aura: Increases Reputation Gained by % //http://www.thottbot.com/?sp=30754
    &Aura::SpellAuraLimitSpeed,                                             // 191 missing //used // noname //http://www.thottbot.com/?sp=29894
    &Aura::SpellAuraMeleeHaste,                                             // 192 Apply Aura: Melee Slow %
    &Aura::SpellAuraIncreaseTimeBetweenAttacksPCT,                          // 193 Apply Aura: Increase Time Between Attacks (Melee, Ranged and Spell) by %
    &Aura::SpellAuraNULL,                                                   // 194 &Aura::SpellAuraIncreaseSpellDamageByInt,// Apply Aura: Increase Spell Damage by % of Intellect (All)
    &Aura::SpellAuraNULL,                                                   // 195 &Aura::SpellAuraIncreaseHealingByInt,// Apply Aura: Increase Healing by % of Intellect
    &Aura::SpellAuraNULL,                                                   // 196 Apply Aura: Mod All Weapon Skills (6)
    &Aura::SpellAuraModAttackerCritChance,                                  // 197 Apply Aura: Reduce Attacker Critical Hit Chance by %
    &Aura::SpellAuraIncreaseAllWeaponSkill,                                 // 198
    &Aura::SpellAuraIncreaseHitRate,                                        // 199 Apply Aura: Increases Spell % To Hit (Fire, Nature, Frost)
    &Aura::SpellAuraNULL,                                                   // 200 Increases experience earned by $s1%.  Lasts $d.
    &Aura::SpellAuraAllowFlight,                                            // 201 SPELL_AURA_FLY 201 isn't it same like 206 and 207?
    &Aura::SpellAuraFinishingMovesCannotBeDodged,                           // 202 Finishing moves cannot be dodged - 32601, 44452
    &Aura::SpellAuraReduceCritMeleeAttackDmg,                               // 203 Apply Aura: Reduces Attacker Critical Hit Damage with Melee by %
    &Aura::SpellAuraReduceCritRangedAttackDmg,                              // 204 Apply Aura: Reduces Attacker Critical Hit Damage with Ranged by %
    &Aura::SpellAuraNULL,                                                   // 205 "School" Vulnerability
    &Aura::SpellAuraEnableFlight,                                           // 206 Take flight on a worn old carpet. - Spell 43343
    &Aura::SpellAuraEnableFlight,                                           // 207 set fly mod flight speed?
    &Aura::SpellAuraEnableFlightWithUnmountedSpeed,                         // 208 mod flight speed?
    &Aura::SpellAuraNULL,                                                   // 209 mod flight speed?
    &Aura::SpellAuraNULL,                                                   // 210 commentator's command - spell 42009
    &Aura::SpellAuraIncreaseFlightSpeed,                                    // 211
    &Aura::SpellAuraIncreaseRAPbyStatPct,                                   // SPELL_AURA_MOD_RANGED_ATTACK_POWER_BY_STAT_PCT //212 Apply Aura: Increase Ranged Atk Power by % of stat
    &Aura::SpellAuraIncreaseRageFromDamageDealtPCT,                         // 213 Apply Aura: Increase Rage from Damage Dealt by %
    &Aura::SpellAuraNULL,                                                   // 214 // Tamed Pet Passive (DND)
    &Aura::SpellAuraRemoveReagentCost,                                      // 215 // arena preparation buff - cancel soul shard requirement?
    &Aura::SpellAuraModCastingSpeed,                                        // 216 Increases casting time %, reuse existing handler...
    &Aura::SpellAuraNULL,                                                   // 217 // not used
    &Aura::SpellAuraNULL,                                                   // 218 // increases time between ranged attacks
    &Aura::SpellAuraRegenManaStatPCT,                                       // 219 Regenerate mana equal to $s1% of your Intellect every 5 sec, even while casting
    &Aura::SpellAuraSpellHealingStatPCT,                                    // 220 Increases your healing spells  by up to $s1% of your Strength // increases your critical strike rating by 35% of your spirit // Molten Armor only?
    &Aura::SpellAuraNULL,                                                   // 221 Detaunt "Ignores an enemy, forcing the caster to not attack it unless there is no other target nearby. When the effect wears off, the creature will attack the most threatening target."
    &Aura::SpellAuraNULL,                                                   // 222 // not used
    &Aura::SpellAuraNULL,                                                   // 223 // used in one spell, cold stare 43593
    &Aura::SpellAuraNULL,                                                   // 224 // not used
    &Aura::SpellAuraNULL,                                                   // 225 // Prayer of Mending "Places a spell on the target that heals them for $s1 the next time they take damage.  When the heal occurs, Prayer of Mending jumps to a raid member within $a1 yards.  Jumps up to $n times and lasts $d after each jump.  This spell can only be placed on one target at a time."
    &Aura::SpellAuraPeriodicTriggerDummy,                                   // 226 // used in brewfest spells, headless horseman, Aspect of the Viper
    &Aura::SpellAuraPeriodicTriggerSpellWithValue,                          // 227 // Used by Mind Flay, Siege Turrets 'Machine gun' and a few other spells.
    &Aura::SpellAuraModStealthDetection,                                    // 228 Stealth Detection. http://www.thottbot.com/s34709
    &Aura::SpellAuraReduceAOEDamageTaken,                                   // 229 Apply Aura:Reduces the damage your pet takes from area of effect attacks http://www.thottbot.com/s35694
    &Aura::SpellAuraIncreaseMaxHealth,                                      // 230 Increase Max Health (commanding shout);
    &Aura::SpellAuraProcTriggerSpell,                                       // 231 curse a target http://www.thottbot.com/s40303
    &Aura::SpellAuraReduceEffectDuration,                                   // 232 // Reduces duration of Magic effects by $s2%. SPELL_AURA_MECHANIC_DURATION_MOD
    &Aura::SpellAuraNULL,                                                   // 233 // Beer Goggles
    &Aura::SpellAuraReduceEffectDuration,                                   // 234 Apply Aura: Reduces Silence or Interrupt effects, Item spell magic http://www.thottbot.com/s42184
    &Aura::SpellAuraNULL,                                                   // 235 33206 Instantly reduces a friendly target's threat by $44416s1%, reduces all damage taken by $s1% and increases resistance to Dispel mechanics by $s2% for $d.
    &Aura::HandleAuraControlVehicle,                                        // 236
    &Aura::SpellAuraModHealingByAP,                                         // 237   //increase spell healing by X pct from attack power
    &Aura::SpellAuraModSpellDamageByAP,                                     // 238   //increase spell dmg by X pct from attack power
    &Aura::SpellAuraModScale,                                               // 239
    &Aura::SpellAuraExpertise,                                              // 240 SPELL_AURA_EXPERTISE
    &Aura::SpellAuraForceMoveForward,                                       // 241 makes target to run forward
    &Aura::SpellAuraNULL,                                                   // 242
    &Aura::SpellAuraNULL,                                                   // 243
    &Aura::SpellAuraComprehendLang,                                         // 244 allows target to understand itself while talking in different language
    &Aura::SpellAuraNULL,                                                   // 245
    &Aura::SpellAuraNULL,                                                   // 246
    &Aura::SpellAuraMirrorImage,                                            // 247
    &Aura::SpellAuraModCombatResultChance,                                  // 248 SPELL_AURA_MOD_COMBAT_RESULT_CHANCE
    &Aura::SpellAuraConvertRune,                                            // 249 SPELL_AURA_CONVERT_RUNE
    &Aura::SpellAuraAddHealth,                                              // 250
    &Aura::SpellAuraNULL,                                                   // 251  Mod Enemy Dodge
    &Aura::SpellAuraNULL,                                                   // 252 Reduces the target's ranged, melee attack, and casting speed by X pct for Y sec.
    &Aura::SpellAuraBlockMultipleDamage,                                    // 253 SPELL_AURA_BLOCK_MULTIPLE_DAMAGE
    &Aura::SpellAuraModDisarm,                                              // 254 SPELL_AURA_MOD_DISARM_OFFHAND
    &Aura::SpellAuraModMechanicDmgTakenPct,                                 // 255 SPELL_AURA_MOD_MECHANIC_DAMAGE_TAKEN_PERCENT
    &Aura::SpellAuraRemoveReagentCost,                                      // 256 Remove reagent cost
    &Aura::SpellAuraNULL,                                                   // 257 Mod Target Resist By Spell Class (does damage in the form of X damage, ignoring all resistances, absorption, and immunity mechanics. - http://thottbot.com/s47271)
    &Aura::SpellAuraNULL,                                                   // 258 Mod Spell Visual
    &Aura::SpellAuraNULL,                                                   // 259 Mod Periodic Damage Taken Pct - Periodic Shadow damage taken increased by 3% (http://thottbot.com/s60448)
    &Aura::SpellAuraNULL,                                                   // 260 Screen Effect
    &Aura::SpellAuraPhase,                                                  // 261
    &Aura::SpellAuraIgnoreTargetAuraState,                                  // 262 SPELL_AURA_IGNORE_TARGET_AURA_STATE
    &Aura::SpellAuraAllowOnlyAbility,                                       // 263 SPELL_AURA_ALLOW_ONLY_ABILITY
    &Aura::SpellAuraNULL,                                                   // 264
    &Aura::SpellAuraNULL,                                                   // 265
    &Aura::SpellAuraNULL,                                                   // 266
    &Aura::SpellAuraNULL,                                                   // 267 Prevent the application of harmful magical effects. used only by Dk's Anti Magic Shell
    &Aura::SpellAuraIncreaseAPbyStatPct,                                    // 268 Increase attack power by % of stat
    &Aura::SpellAuraNULL,                                                   // 269 Damage reduction effects ignored. (?) - http://thottbot.com/s57318
    &Aura::SpellAuraNULL,                                                   // 270 Ignore target resist
    &Aura::SpellAuraModSpellDamageDOTPct,                                   // 271
    &Aura::SpellAuraNULL,                                                   // 272 Unknown
    &Aura::SpellAuraNULL,                                                   // 273 Some sort of dummy aura? (http://thottbot.com/s54844 + http://thottbot.com/s26659)
    &Aura::SpellAuraConsumeNoAmmo,                                          // 274 Consumes no ammo
    &Aura::SpellAuraIgnoreShapeshift,                                       // 275 Ignore unit states
    &Aura::SpellAuraNULL,                                                   // 276 Mod Damage % Mechanic
    &Aura::SpellAuraNULL,                                                   // 277 SPELL_AURA_REDIRECT_THREAT or SPELL_AURA_MOD_MAX_AFFECTED_TARGETS ?
    &Aura::SpellAuraModDisarm,                                              // 278 SPELL_AURA_MOD_DISARM_RANGED
    &Aura::SpellAuraMirrorImage2,                                           // 279 Modify models(?)
    &Aura::SpellAuraModIgnoreArmorPct,                                      // 280 SPELL_AURA_IGNORE_ARMOR_PCT
    &Aura::SpellAuraNULL,                                                   // 281 Mod Honor gain increased by X pct. Final Reward Honor increased by X pct for Y Rank and above. (http://thottbot.com/s58560 && http://thottbot.com/s58557)
    &Aura::SpellAuraModBaseHealth,                                          // 282 SPELL_AURA_MOD_BASE_HEALTH
    &Aura::SpellAuraModHealingPCT,                                          // 283 Increases all healing received by X pct
    &Aura::SpellAuraNULL,                                                   // 284 not used by any spells (3.08a)
    &Aura::SpellAuraModAttackPowerOfArmor,                                  // 285 SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR
    &Aura::SpellAuraNULL,                                                   // 286 SPELL_AURA_ALLOW_DOT_TO_CRIT
    &Aura::SpellAuraDeflectSpells,                                          // 287 SPELL_AURA_DEFLECT_SPELLS
    &Aura::SpellAuraNULL,                                                   // 288 not used by any spells (3.09) except 1 test spell.
    &Aura::SpellAuraNULL,                                                   // 289 unused
    &Aura::SpellAuraNULL,                                                   // 290 unused
    &Aura::SpellAuraNULL,                                                   // 291 unused
    &Aura::SpellAuraCallStabledPet,                                         // 292 call stabled pet
    &Aura::SpellAuraNULL,                                                   // 293 2 test spells
    &Aura::SpellAuraNULL,                                                   // 294 2 spells, possible prevent mana regen
    &Aura::SpellAuraNULL,                                                   // 295
    &Aura::SpellAuraNULL,                                                   // 296
    &Aura::SpellAuraNULL,                                                   // 297
    &Aura::SpellAuraNULL,                                                   // 298
    &Aura::SpellAuraNULL,                                                   // 299
    &Aura::SpellAuraNULL,                                                   // 300
    &Aura::SpellAuraNULL,                                                   // 301
    &Aura::SpellAuraNULL,                                                   // 302
    &Aura::SpellAuraNULL,                                                   // 303
    &Aura::SpellAuraNULL,                                                   // 304
    &Aura::SpellAuraNULL,                                                   // 305
    &Aura::SpellAuraNULL,                                                   // 306
    &Aura::SpellAuraNULL,                                                   // 307
    &Aura::SpellAuraNULL,                                                   // 308
    &Aura::SpellAuraNULL,                                                   // 309
    &Aura::SpellAuraNULL,                                                   // 310
    &Aura::SpellAuraNULL,                                                   // 311
    &Aura::SpellAuraNULL,                                                   // 312
    &Aura::SpellAuraNULL,                                                   // 313
    &Aura::SpellAuraNULL,                                                   // 314
    &Aura::SpellAuraNULL,                                                   // 315
    &Aura::SpellAuraNULL                                                    // 316

};

const char* SpellAuraNames[TOTAL_SPELL_AURAS] =
{
    "NONE",                                             //   0 None
    "BIND_SIGHT",                                       //   1 Bind Sight
    "MOD_POSSESS",                                      //   2 Mod Possess
    "PERIODIC_DAMAGE",                                  //   3 Periodic Damage
    "DUMMY",                                            //   4 Script Aura
    "MOD_CONFUSE",                                      //   5 Mod Confuse
    "MOD_CHARM",                                        //   6 Mod Charm
    "MOD_FEAR",                                         //   7 Mod Fear
    "PERIODIC_HEAL",                                    //   8 Periodic Heal
    "MOD_ATTACKSPEED",                                  //   9 Mod Attack Speed
    "MOD_THREAT",                                       //  10 Mod Threat
    "MOD_TAUNT",                                        //  11 Taunt
    "MOD_STUN",                                         //  12 Stun
    "MOD_DAMAGE_DONE",                                  //  13 Mod Damage Done
    "MOD_DAMAGE_TAKEN",                                 //  14 Mod Damage Taken
    "DAMAGE_SHIELD",                                    //  15 Damage Shield
    "MOD_STEALTH",                                      //  16 Mod Stealth
    "MOD_DETECT",                                       //  17 Mod Detect
    "MOD_INVISIBILITY",                                 //  18 Mod Invisibility
    "MOD_INVISIBILITY_DETECTION",                       //  19 Mod Invisibility Detection
    "MOD_TOTAL_HEALTH_REGEN_PCT",                       //  20
    "MOD_TOTAL_MANA_REGEN_PCT",                         //  21
    "MOD_RESISTANCE",                                   //  22 Mod Resistance
    "PERIODIC_TRIGGER_SPELL",                           //  23 Periodic Trigger
    "PERIODIC_ENERGIZE",                                //  24 Periodic Energize
    "MOD_PACIFY",                                       //  25 Pacify
    "MOD_ROOT",                                         //  26 Root
    "MOD_SILENCE",                                      //  27 Silence
    "REFLECT_SPELLS",                                   //  28 Reflect Spells %
    "MOD_STAT",                                         //  29 Mod Stat
    "MOD_SKILL",                                        //  30 Mod Skill
    "MOD_INCREASE_SPEED",                               //  31 Mod Speed
    "MOD_INCREASE_MOUNTED_SPEED",                       //  32 Mod Speed Mounted
    "MOD_DECREASE_SPEED",                               //  33 Mod Speed Slow
    "MOD_INCREASE_HEALTH",                              //  34 Mod Increase Health
    "MOD_INCREASE_ENERGY",                              //  35 Mod Increase Energy
    "MOD_SHAPESHIFT",                                   //  36 Shapeshift
    "EFFECT_IMMUNITY",                                  //  37 Immune Effect
    "STATE_IMMUNITY",                                   //  38 Immune State
    "SCHOOL_IMMUNITY",                                  //  39 Immune School
    "DAMAGE_IMMUNITY",                                  //  40 Immune Damage
    "DISPEL_IMMUNITY",                                  //  41 Immune Dispel Type
    "PROC_TRIGGER_SPELL",                               //  42 Proc Trigger Spell
    "PROC_TRIGGER_DAMAGE",                              //  43 Proc Trigger Damage
    "TRACK_CREATURES",                                  //  44 Track Creatures
    "TRACK_RESOURCES",                                  //  45 Track Resources
    "MOD_PARRY_SKILL",                                  //  46 Mod Parry Skill
    "MOD_PARRY_PERCENT",                                //  47 Mod Parry Percent
    "MOD_DODGE_SKILL",                                  //  48 Mod Dodge Skill
    "MOD_DODGE_PERCENT",                                //  49 Mod Dodge Percent
    "MOD_BLOCK_SKILL",                                  //  50 Mod Block Skill
    "MOD_BLOCK_PERCENT",                                //  51 Mod Block Percent
    "MOD_CRIT_PERCENT",                                 //  52 Mod Crit Percent
    "PERIODIC_LEECH",                                   //  53 Periodic Leech
    "MOD_HIT_CHANCE",                                   //  54 Mod Hit Chance
    "MOD_SPELL_HIT_CHANCE",                             //  55 Mod Spell Hit Chance
    "TRANSFORM",                                        //  56 Transform
    "MOD_SPELL_CRIT_CHANCE",                            //  57 Mod Spell Crit Chance
    "MOD_INCREASE_SWIM_SPEED",                          //  58 Mod Speed Swim
    "MOD_DAMAGE_DONE_CREATURE",                         //  59 Mod Creature Dmg Done
    "MOD_PACIFY_SILENCE",                               //  60 Pacify & Silence
    "MOD_SCALE",                                        //  61 Mod Scale
    "PERIODIC_HEALTH_FUNNEL",                           //  62 Periodic Health Funnel
    "PERIODIC_MANA_FUNNEL",                             //  63 Periodic Mana Funnel
    "PERIODIC_MANA_LEECH",                              //  64 Periodic Mana Leech
    "MOD_CASTING_SPEED",                                //  65 Haste - Spells
    "FEIGN_DEATH",                                      //  66 Feign Death
    "MOD_DISARM",                                       //  67 Disarm
    "MOD_STALKED",                                      //  68 Mod Stalked
    "SCHOOL_ABSORB",                                    //  69 School Absorb
    "EXTRA_ATTACKS",                                    //  70 Extra Attacks
    "MOD_SPELL_CRIT_CHANCE_SCHOOL",                     //  71 Mod School Spell Crit Chance
    "MOD_POWER_COST",                                   //  72 Mod Power Cost
    "MOD_POWER_COST_SCHOOL",                            //  73 Mod School Power Cost
    "REFLECT_SPELLS_SCHOOL",                            //  74 Reflect School Spells %
    "MOD_LANGUAGE",                                     //  75 Mod Language
    "FAR_SIGHT",                                        //  76 Far Sight
    "MECHANIC_IMMUNITY",                                //  77 Immune Mechanic
    "MOUNTED",                                          //  78 Mounted
    "MOD_DAMAGE_PERCENT_DONE",                          //  79 Mod Dmg %
    "MOD_PERCENT_STAT",                                 //  80 Mod Stat %
    "SPLIT_DAMAGE",                                     //  81 Split Damage
    "WATER_BREATHING",                                  //  82 Water Breathing
    "MOD_BASE_RESISTANCE",                              //  83 Mod Base Resistance
    "MOD_REGEN",                                        //  84 Mod Health Regen
    "MOD_POWER_REGEN",                                  //  85 Mod Power Regen
    "CHANNEL_DEATH_ITEM",                               //  86 Create Death Item
    "MOD_DAMAGE_PERCENT_TAKEN",                         //  87 Mod Dmg % Taken
    "MOD_PERCENT_REGEN",                                //  88 Mod Health Regen Percent
    "PERIODIC_DAMAGE_PERCENT",                          //  89 Periodic Damage Percent
    "MOD_RESIST_CHANCE",                                //  90 Mod Resist Chance
    "MOD_DETECT_RANGE",                                 //  91 Mod Detect Range
    "PREVENTS_FLEEING",                                 //  92 Prevent Fleeing
    "MOD_UNATTACKABLE",                                 //  93 Mod Uninteractible
    "INTERRUPT_REGEN",                                  //  94 Interrupt Regen
    "GHOST",                                            //  95 Ghost
    "SPELL_MAGNET",                                     //  96 Spell Magnet
    "MANA_SHIELD",                                      //  97 Mana Shield
    "MOD_SKILL_TALENT",                                 //  98 Mod Skill Talent
    "MOD_ATTACK_POWER",                                 //  99 Mod Attack Power
    "AURAS_VISIBLE",                                    // 100 Auras Visible
    "MOD_RESISTANCE_PCT",                               // 101 Mod Resistance %
    "MOD_CREATURE_ATTACK_POWER",                        // 102 Mod Creature Attack Power
    "MOD_TOTAL_THREAT",                                 // 103 Mod Total Threat (Fade)
    "WATER_WALK",                                       // 104 Water Walk
    "FEATHER_FALL",                                     // 105 Feather Fall
    "HOVER",                                            // 106 Hover
    "ADD_FLAT_MODIFIER",                                // 107 Add Flat Modifier
    "ADD_PCT_MODIFIER",                                 // 108 Add % Modifier
    "ADD_TARGET_TRIGGER",                               // 109 Add Class Target Trigger
    "MOD_POWER_REGEN_PERCENT",                          // 110 Mod Power Regen %
    "ADD_CASTER_HIT_TRIGGER",                           // 111 Add Class Caster Hit Trigger
    "OVERRIDE_CLASS_SCRIPTS",                           // 112 Override Class Scripts
    "MOD_RANGED_DAMAGE_TAKEN",                          // 113 Mod Ranged Dmg Taken
    "MOD_RANGED_DAMAGE_TAKEN_PCT",                      // 114 Mod Ranged % Dmg Taken
    "MOD_HEALING",                                      // 115 Mod Healing
    "IGNORE_REGEN_INTERRUPT",                           // 116 Regen During Combat
    "MOD_MECHANIC_RESISTANCE",                          // 117 Mod Mechanic Resistance
    "MOD_HEALING_PCT",                                  // 118 Mod Healing %
    "SHARE_PET_TRACKING",                               // 119 Share Pet Tracking
    "UNTRACKABLE",                                      // 120 Untrackable
    "EMPATHY",                                          // 121 Empathy (Lore, whatever)
    "MOD_OFFHAND_DAMAGE_PCT",                           // 122 Mod Offhand Dmg %
    "MOD_POWER_COST_PCT",                               // 123 Mod Power Cost % --> armor penetration & spell penetration
    "MOD_RANGED_ATTACK_POWER",                          // 124 Mod Ranged Attack Power
    "MOD_MELEE_DAMAGE_TAKEN",                           // 125 Mod Melee Dmg Taken
    "MOD_MELEE_DAMAGE_TAKEN_PCT",                       // 126 Mod Melee % Dmg Taken
    "RANGED_ATTACK_POWER_ATTACKER_BONUS",               // 127 Rngd Atk Pwr Attckr Bonus
    "MOD_POSSESS_PET",                                  // 128 Mod Possess Pet
    "MOD_INCREASE_SPEED_ALWAYS",                        // 129 Mod Speed Always
    "MOD_MOUNTED_SPEED_ALWAYS",                         // 130 Mod Mounted Speed Always
    "MOD_CREATURE_RANGED_ATTACK_POWER",                 // 131 Mod Creature Ranged Attack Power
    "MOD_INCREASE_ENERGY_PERCENT",                      // 132 Mod Increase Energy %
    "MOD_INCREASE_HEALTH_PERCENT",                      // 133 Mod Max Health %
    "MOD_MANA_REGEN_INTERRUPT",                         // 134 Mod Interrupted Mana Regen
    "MOD_HEALING_DONE",                                 // 135 Mod Healing Done
    "MOD_HEALING_DONE_PERCENT",                         // 136 Mod Healing Done %
    "MOD_TOTAL_STAT_PERCENTAGE",                        // 137 Mod Total Stat %
    "MOD_HASTE",                                        // 138 Haste - Melee
    "FORCE_REACTION",                                   // 139 Force Reaction
    "MOD_RANGED_HASTE",                                 // 140 Haste - Ranged
    "MOD_RANGED_AMMO_HASTE",                            // 141 Haste - Ranged (Ammo Only)
    "MOD_BASE_RESISTANCE_PCT",                          // 142 Mod Base Resistance %
    "MOD_RESISTANCE_EXCLUSIVE",                         // 143 Mod Resistance Exclusive
    "SAFE_FALL",                                        // 144 Safe Fall
    "CHARISMA",                                         // 145 Charisma
    "PERSUADED",                                        // 146 Persuaded
    "ADD_CREATURE_IMMUNITY",                            // 147 Add Creature Immunity
    "RETAIN_COMBO_POINTS",                              // 148 Retain Combo Points
    "RESIST_PUSHBACK",                                  // 149 Resist Pushback
    "MOD_SHIELD_BLOCK_PCT",                             // 150 Mod Shield Block %
    "TRACK_STEALTHED",                                  // 151 Track Stealthed
    "MOD_DETECTED_RANGE",                               // 152 Mod Detected Range
    "SPLIT_DAMAGE_FLAT",                                // 153 Split Damage Flat
    "MOD_STEALTH_LEVEL",                                // 154 Stealth Level Modifier
    "MOD_WATER_BREATHING",                              // 155 Mod Water Breathing
    "MOD_REPUTATION_ADJUST",                            // 156 Mod Reputation Gain
    "PET_DAMAGE_MULTI",                                 // 157 Mod Pet Damage
    "MOD_SHIELD_BLOCK",                                 // 158 Mod Shield Block
    "NO_PVP_CREDIT",                                    // 159 No PVP Credit
    "MOD_SIDE_REAR_PDAE_DAMAGE_TAKEN",                  // 160 Mod Side/Rear PBAE Damage Taken
    "MOD_HEALTH_REGEN_IN_COMBAT",                       // 161 Mod Health Regen In Combat
    "POWER_BURN",                                       // 162 Power Burn
    "MOD_CRIT_DAMAGE_BONUS_MELEE",                      // 163 Mod Critical Damage Bonus (Physical)
    "",                                                 // 164
    "MELEE_ATTACK_POWER_ATTACKER_BONUS",                // 165 Melee AP Attacker Bonus
    "MOD_ATTACK_POWER_PCT",                             // 166 Mod Attack Power
    "MOD_RANGED_ATTACK_POWER_PCT",                      // 167 Mod Ranged Attack Power %
    "INCREASE_DAMAGE",                                  // 168 Increase Damage Type
    "INCREASE_CRITICAL",                                // 169 Increase Critical Type
    "DETECT_AMORE",                                     // 170 Detect Amore
    "",                                                 // 171
    "INCREASE_MOVEMENT_AND_MOUNTED_SPEED",              // 172 Increase Movement and Mounted Speed (Non-Stacking)
    "",                                                 // 173
    "INCREASE_SPELL_DAMAGE_PCT",                        // 174 Increase Spell Damage by % status
    "INCREASE_SPELL_HEALING_PCT",                       // 175 Increase Spell Healing by % status
    "SPIRIT_OF_REDEMPTION",                             // 176 Spirit of Redemption Auras
    "AREA_CHARM",                                       // 177 Area Charm
    "",                                                 // 178
    "INCREASE_ATTACKER_SPELL_CRIT",                     // 179 Increase Attacker Spell Crit Type
    "INCREASE_SPELL_DAMAGE_VS_TYPE",                    // 180 Increase Spell Damage Type
    "",                                                 // 181
    "INCREASE_ARMOR_BASED_ON_INTELLECT_PCT",            // 182 Increase Armor based on Intellect
    "DECREASE_CRIT_THREAT",                             // 183 Decrease Critical Threat by
    "DECREASE_ATTACKER_CHANCE_TO_HIT_MELEE",            // 184 Reduces Attacker Chance to Hit with Melee
    "DECREASE_ATTACKER_CHANGE_TO_HIT_RANGED",           // 185 Reduces Attacker Chance to Hit with Ranged
    "DECREASE_ATTACKER_CHANGE_TO_HIT_SPELLS",           // 186 Reduces Attacker Chance to Hit with Spells
    "DECREASE_ATTACKER_CHANGE_TO_CRIT_MELEE",           // 187 Reduces Attacker Chance to Crit with Melee (Ranged?)
    "DECREASE_ATTACKER_CHANGE_TO_CRIT_RANGED",          // 188 Reduces Attacker Chance to Crit with Ranged (Melee?)
    "",                                                 // 189
    "INCREASE_REPUTATION",                              // 190 Increases reputation from killed creatures
    "SPEED_LIMIT",                                      // 191 speed limit
    "MELEE_SLOW_PCT",                                   // 192
    "INCREASE_TIME_BETWEEN_ATTACKS",                    // 193
    "INCREASE_SPELL_DAMAGE_PCT_OF_INTELLECT",           // 194 NOT USED ANYMORE - 174 used instead
    "INCREASE_HEALING_PCT_OF_INTELLECT",                // 195 NOT USED ANYMORE - 175 used instead
    "MOD_ALL_WEAPON_SKILLS",                            // 196
    "REDUCE_ATTACKER_CRICTICAL_HIT_CHANCE_PCT",         // 197
    "",                                                 // 198
    "INCREASE_SPELL_HIT_PCT",                           // 199
    "",                                                 // 200
    "CANNOT_BE_DODGED",                                 // 201
    "FINISHING_MOVES_CANNOT_BE_DODGED",                 // 202
    "REDUCE_ATTACKER_CRICTICAL_HIT_DAMAGE_MELEE_PCT",   // 203
    "REDUCE_ATTACKER_CRICTICAL_HIT_DAMAGE_RANGED_PCT",  // 204
    "",                                                 // 205
    "SPELL_AURA_ENABLE_FLIGHT",                         // 206
    "SPELL_AURA_ENABLE_FLIGHT2",                        // 207
    "SPELL_AURA_ENABLE_FLIGHT_WITH_UNMOUNTED_SPEED",    // 208
    "",                                                 // 209
    "",                                                 // 210
    "",                                                 // 211
    "INCREASE_RANGED_ATTACK_POWER_PCT_OF_INTELLECT",    // 212
    "INCREASE_RAGE_FROM_DAMAGE_DEALT_PCT",              // 213
    "",                                                 // 214
    "",                                                 // 215
    "INCREASE_CASTING_TIME_PCT",                        // 216
    "",                                                 // 217
    "",                                                 // 218
    "REGEN_MANA_STAT_PCT",                              // 219
    "HEALING_STAT_PCT",                                 // 220
    "",                                                 // 221
    "",                                                 // 222
    "",                                                 // 223
    "",                                                 // 224
    "",                                                 // 225
    "PERIODIC_TRIGGER_DUMMY",                           // 226
    "TRIGGER_SPELL_WITH_VALUE",                         // 227 Used by Mind Flay and some other spells.
    "",                                                 // 228
    "SPELL_AURA_REDUCE_AOE_DAMAGE_TAKEN",               // 229
    "INCREASE_MAX_HEALTH",                              // 230 Used by Commanding Shout
    "SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE",         // 231
    "SPELL_AURA_MECHANIC_DURATION_MOD",                 // 232
    "",                                                 // 233
    "",                                                 // 234
    "",                                                 // 235
    "CONTROL_VEHICLE",                                  // 236
    "SPELL_AURA_MOD_HEALING_FROM_AP",                   // 237
    "SPELL_AURA_MOD_SPELL_DAMAGE_FROM_AP",              // 238
    "",                                                 // 239
    "EXPERTISE",                                        // 240
    "241",                                              // 241
    "MOD_SPELL_DAMAGE_FROM_HEALING",                    // 242
    "243",                                              // 243
    "244",                                              // 244
    "MOD_DURATION_OF_MAGIC_EFFECTS",                    // 245
    "246",                                              // 246
    "247",                                              // 247
    "MOD_COMBAT_RESULT_CHANCE",                         // 248
    "CONVERT_RUNE",                                     // 249 Convert rune
    "MOD_INCREASE_HEALTH_2",                            // 250
    "MOD_ENEMY_DODGE",                                  // 251
    "",                                                 // 252
    "SPELL_AURA_BLOCK_MULTIPLE_DAMAGE",                 // 253
    "SPELL_AURA_MOD_DISARM_OFFHAND",                    // 254
    "SPELL_AURA_MOD_MECHANIC_DAMAGE_TAKEN_PERCENT",     // 255
    "",                                                 // 256
    "",                                                 // 257
    "",                                                 // 258
    "",                                                 // 259
    "",                                                 // 260
    "SPELL_AURA_SET_PHASE",                             // 261
    "SPELL_AURA_IGNORE_TARGET_AURA_STATE",              // 262
    "SPELL_AURA_ALLOW_ONLY_ABILITY",                    // 263
    "",                                                 // 264
    "",                                                 // 265
    "",                                                 // 266
    "",                                                 // 267
    "MOD_SHAMMY_ATTACK_POWER",                          // 268 Mental Dexterity (increases ap by x% of intellect)
    "",                                                 // 269
    "",                                                 // 270
    "SPELL_AURA_INCREASE_SPELL_DOT_DAMAGE_PCT",         // 271
    "",                                                 // 272
    "",                                                 // 273
    "SPELL_AURA_CONSUMES_NO_AMMO",                      // 274
    "",                                                 // 275
    "",                                                 // 276
    "",                                                 // 277
    "SPELL_AURA_MOD_DISARM_RANGED",                     // 278
    "",                                                 // 279
    "SPELL_AURA_IGNORE_ARMOR_PCT",                      // 280
    "",                                                 // 281
    "SPELL_AURA_MOD_BASE_HEALTH",                       // 282
    "",                                                 // 283
    "",                                                 // 284
    "SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR",             // 285
    "SPELL_AURA_ALLOW_CRIT_PERIODIC_DAMAGE",            // 286
    "SPELL_AURA_DEFLECT_SPELLS",                        // 287
    "",                                                 // 288
    "",                                                 // 289
    "",                                                 // 290
    "",                                                 // 291
    "SPELL_AURA_CALL_STABLED_PET",                      // 292
    "",                                                 // 293
    "",                                                 // 294
    "",                                                 // 295
    "",                                                 // 296
    "",                                                 // 297
    "",                                                 // 298
    "",                                                 // 299
    "",                                                 // 300
    "",                                                 // 301
    "",                                                 // 302
    "",                                                 // 303
    "",                                                 // 304
    "",                                                 // 305
    "",                                                 // 306
    "",                                                 // 307
    "",                                                 // 308
    "",                                                 // 309
    "",                                                 // 310
    "",                                                 // 311
    "",                                                 // 312
    "",                                                 // 313
    "",                                                 // 314
    "",                                                 // 315
    ""                                                  // 316
};


Player* Aura::GetPlayerCaster()
{
    //caster and target are the same
    if (m_casterGuid == m_target->GetGUID())
    {
        if (m_target->IsPlayer())
        {
            return static_cast<Player*>(m_target);
        }
        else //caster is not a player
        {
            return nullptr;
        }
    }

    if (m_target->GetMapMgr())
    {
        return m_target->GetMapMgr()->GetPlayer(Arcemu::Util::GUID_LOPART(m_casterGuid));
    }
    else
    {
        return nullptr;
    }
}

Unit* Aura::GetUnitCaster()
{
    if (m_casterGuid == m_target->GetGUID())
        return m_target;

    if (m_target->GetMapMgr())
        return m_target->GetMapMgr()->GetUnit(m_casterGuid);
    else
        return nullptr;
}

Object* Aura::GetCaster()
{
    if (m_target == nullptr)
        return nullptr;
    if (m_casterGuid == m_target->GetGUID())
        return m_target;
    if (m_target->GetMapMgr())
        return m_target->GetMapMgr()->_GetObject(m_casterGuid);
    else
        return nullptr;
}

Aura::Aura(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary, Item* i_caster)
{
    m_castInDuel = false;
    m_spellInfo = proto;
    m_duration = duration;
    m_positive = 0; //we suppose spell will have positive impact on target
    m_temporary = temporary; // Aura saving related
    m_deleted = false;
    m_ignoreunapply = false;
    m_casterGuid = caster->GetGUID();
    ARCEMU_ASSERT(target != NULL);
    m_target = target;

    if (m_target->IsPlayer())
        p_target = static_cast< Player* >(m_target);
    else
        p_target = nullptr;

    if (i_caster != nullptr)
    {
        m_castedItemId = i_caster->GetItemProperties()->ItemId;
        itemCasterGUID = i_caster->GetGUID();
    }
    else
    {
        m_castedItemId = 0;
        itemCasterGUID = 0;
    }

    // Modifies current aura duration based on its mechanic type
    if (p_target && GetDuration() > 0)
    {
        int32 DurationModifier = p_target->MechanicDurationPctMod[Spell::GetMechanic(proto)];
        if (DurationModifier < -100)
            DurationModifier = -100; // Can't reduce by more than 100%
        SetDuration((GetDuration() * (100 + DurationModifier)) / 100);
    }

    if (GetDuration() > 0 && m_spellInfo->getChannelInterruptFlags() != 0 && caster->IsUnit())
        SetDuration(GetDuration() * float2int32(static_cast<Unit*>(caster)->GetCastSpeedMod()));

    // SetCasterFaction(caster->_getFaction());

    // m_auraSlot = 0;
    m_modcount = 0;
    m_dynamicValue = 0;
    m_areaAura = false;

    if (m_spellInfo->custom_c_is_flags & SPELL_FLAG_IS_FORCEDDEBUFF)
        SetNegative(100);
    if (m_spellInfo->custom_c_is_flags & SPELL_FLAG_IS_FORCEDBUFF)
        SetPositive(100);
    if (m_spellInfo->getAttributes() & ATTRIBUTES_NEGATIVE)
        SetNegative(100);

    if (caster->IsUnit())
    {
        if (isAttackable(caster, target))
        {
            SetNegative();
        }
        else
            SetPositive();

        if (p_target && caster->IsPlayer())
        {
            if (p_target->DuelingWith == static_cast<Player*>(caster))
            {
                m_castInDuel = true;
            }
        }
    }

    if (!IsPassive())
    {
        expirytime = (uint32)UNIXTIME;
    }

    m_visualSlot = 0xFF;
    pSpellId = 0;
    // LOG_DETAIL("Aura::Constructor %u (%s) from %u.", m_spellProto->getId(), m_spellProto->Name, m_target->GetLowGUID());
    m_auraSlot = 0xffff;
    m_interrupted = -1;
    m_flags = 0;
    // fixed_amount = 0;//used only por percent values to be able to recover value correctly.No need to init this if we are not using it

    m_casterfaction = 0;
    mod = nullptr;
    for (uint8 i = 0; i < 3; ++i)
    {
        m_modList[i].m_type = 0;
        m_modList[i].m_amount = 0;
        m_modList[i].m_miscValue = 0;
        m_modList[i].m_effectIndex = 0;
        m_modList[i].realamount = 0;
    }
}

Aura::~Aura()
{
    sEventMgr.RemoveEvents(this);
}

void Aura::Remove()
{
    sEventMgr.RemoveEvents(this);

    ///\todo Check this condition - consider there are 3 aura modifiers and m_deleted can be set to true by first one, other two mods are normally applied, but cant un-apply (?)
    if (m_deleted)
        return;

    sHookInterface.OnAuraRemove(this);

    LogDebugFlag(LF_AURA, "Removing aura %u from unit %u", m_spellInfo->getId(), m_target->GetGUID());

    m_deleted = true;

    if (!IsPassive() || m_spellInfo->getAttributesEx() & ATTRIBUTES_ON_NEXT_SWING_2)
        m_target->ModVisualAuraStackCount(this, -1);

    ApplyModifiers(false);

    for (uint8_t x = 0; x < 3; x++)
    {
        if (!m_spellInfo->getEffect(x))
            continue;

        if (m_spellInfo->getEffect(x) == SPELL_EFFECT_TRIGGER_SPELL && sSpellCustomizations.isAlwaysApply(m_spellInfo) == false)
        {
            // I'm not sure about this! FIX ME!!
            auto spell_entry = sSpellCustomizations.GetSpellInfo(GetSpellInfo()->getEffectTriggerSpell(x));
            if (spell_entry != nullptr)
                if (spell_entry->getDurationIndex() < m_spellInfo->getDurationIndex())
                    m_target->RemoveAura(GetSpellInfo()->getEffectTriggerSpell(x));
        }
        else if (IsAreaAura() && m_casterGuid == m_target->GetGUID())
            ClearAATargets();
    }

    if (m_spellInfo->getProcCharges() > 0 && m_spellInfo->custom_proc_interval == 0)
    {
        if (m_target->m_chargeSpellsInUse)
        {
            m_target->m_chargeSpellRemoveQueue.push_back(GetSpellId());
        }
        else
        {
            std::map< uint32, struct SpellCharge >::iterator iter;
            iter = m_target->m_chargeSpells.find(GetSpellId());
            if (iter != m_target->m_chargeSpells.end())
            {
                if (iter->second.count > 1)
                    --iter->second.count;
                else
                    m_target->m_chargeSpells.erase(iter);
            }
        }
    }

    // maybe we are removing it without even assigning it. Example when we are refreshing an aura
    if (m_auraSlot != 0xFFFF)
        m_target->m_auras[m_auraSlot] = nullptr;

    // reset diminishing return timer if needed
    m_target->removeDiminishingReturnTimer(m_spellInfo);

    // remove attacker
    Unit* caster = GetUnitCaster();
    if (caster != nullptr)
    {
        if (caster != m_target)
        {
            caster->CombatStatus.RemoveAttackTarget(m_target);
            m_target->CombatStatus.RemoveAttacker(caster, caster->GetGUID());
        }
    }
    else
        m_target->CombatStatus.RemoveAttacker(nullptr, m_casterGuid);

    /**********************Cooldown**************************
    * this is only needed for some spells
    * for now only spells that have:
    * (m_spellInfo->Attributes == 0x2050000) && !(m_spellInfo->AttributesEx) ||
    * m_spellProto->Attributes == 0x2040100
    * are handled. Its possible there are more spells like this
    *************************************************************/
    if (caster != nullptr && caster->IsPlayer() && caster->IsInWorld() && m_spellInfo->custom_c_is_flags & SPELL_FLAG_IS_REQUIRECOOLDOWNUPDATE)
    {
        Player* p = static_cast< Player* >(caster);

        p->SendSpellCooldownEvent(m_spellInfo->getId());
    }

    if (caster != nullptr && caster->IsPlayer() && caster->IsInWorld() && caster->getUInt32Value(PLAYER_FARSIGHT) != 0)
    {
        uint8 j;
        for (j = 0; j < 3; ++j)
            if (m_spellInfo->getEffect(j) == SPELL_EFFECT_ADD_FARSIGHT)
                break;

        if (j != 3)
        {
            static_cast<Player*>(caster)->SetFarsightTarget(0);
        }
    }

    // If this aura can affect one target at a time, remove this target from the caster map
    if (caster != nullptr && GetSpellInfo()->getAttributesExE() & ATTRIBUTESEXE_SINGLE_TARGET_AURA && m_target->GetAuraStackCount(GetSpellId()) == 1)
        caster->removeSingleTargetGuidForAura(GetSpellInfo()->getId());

    /* Remove aurastates */
    uint32 flag = 0;
    if (m_spellInfo->getMechanicsType() == MECHANIC_ENRAGED && !--m_target->asc_enraged)
        flag |= AURASTATE_FLAG_ENRAGED;
    else if (m_spellInfo->getMechanicsType() == MECHANIC_BLEEDING && !--m_target->asc_bleed)
        flag |= AURASTATE_FLAG_BLEED;
    if (m_spellInfo->custom_BGR_one_buff_on_target & SPELL_TYPE_SEAL && !--m_target->asc_seal)
        flag |= AURASTATE_FLAG_JUDGEMENT;
    if (flag != 0)
        m_target->RemoveFlag(UNIT_FIELD_AURASTATE, flag);

    // We will delete this on the next update, eluding some spell crashes :|
    m_target->AddGarbageAura(this);
    m_target->UpdateAuraForGroup(static_cast<uint8>(m_auraSlot));

    // maybe we are removing it without even assigning it. Example when we are refreshing an aura
    if (m_auraSlot != 0xFFFF)
        m_target->m_auras[m_auraSlot] = nullptr;

    // only remove channel stuff if caster == target, then it's not removed twice, for example, arcane missiles applies a dummy aura to target
    if (caster != nullptr && caster == m_target && m_spellInfo->getChannelInterruptFlags() != 0)
    {
        caster->SetChannelSpellTargetGUID(0);
        caster->SetChannelSpellId(0);
    }

    if ((caster != nullptr) && caster->IsPlayer() && m_spellInfo->HasEffect(SPELL_EFFECT_SUMMON))
    {
        Unit* charm = caster->GetMapMgr()->GetUnit(caster->GetCharmedUnitGUID());
        if ((charm != nullptr) && (charm->GetCreatedBySpell() == m_spellInfo->getId()))
            static_cast< Player* >(caster)->UnPossess();
    }
}

void Aura::AddMod(uint32 t, int32 a, uint32 miscValue, uint8_t effectIndex)
{
    // this is fix, when u have the same unit in target list several times
    // for (uint32 x= 0;x<m_modcount;x++)
    //	if (m_modList[x].i==i)return;

    /*if (m_modList[0].m_type == t ||
        m_modList[1].m_type == t ||
        m_modList[2].m_type == t)
        return; // don't duplicate mods // some spells apply duplicate mods, like some seals*/

    if (m_modcount >= 3)
    {
        LOG_ERROR("Tried to add >3 (%u) mods to spellid %u [%u:%u, %u:%u, %u:%u]", m_modcount + 1, this->m_spellInfo->getId(), m_modList[0].m_type, m_modList[0].m_amount, m_modList[1].m_type, m_modList[1].m_amount, m_modList[2].m_type, m_modList[2].m_amount);
        return;
    }
    m_modList[m_modcount].m_type = t;
    m_modList[m_modcount].m_amount = a;
    m_modList[m_modcount].m_miscValue = static_cast<uint16_t>(miscValue);
    m_modList[m_modcount].m_effectIndex = effectIndex;
    m_modcount++;
    // ARCEMU_ASSERT(  m_modcount<=3);
}

void Aura::ApplyModifiers(bool apply)
{
    for (uint32 x = 0; x < m_modcount; x++)
    {
        if (m_modList[x].m_type < TOTAL_SPELL_AURAS)
        {
            mod = &m_modList[x];
            LogDebugFlag(LF_AURA, "WORLD: target=%u, Spell Aura id=%u (%s), SpellId=%u, EffectIndex=%u, apply=%s, duration=%u, miscValue=%d, damage=%d",
                      m_target->GetLowGUID(), mod->m_type, SpellAuraNames[mod->m_type], m_spellInfo->getId(), mod->m_effectIndex, apply ? "true" : "false", GetDuration(), mod->m_miscValue, mod->m_amount);
            (*this.*SpellAuraHandler[mod->m_type])(apply);
            if (apply)
            {
                if (m_spellInfo->custom_c_is_flags & SPELL_FLAG_IS_FORCEDDEBUFF)
                    SetNegative(100);
                if (m_spellInfo->custom_c_is_flags & SPELL_FLAG_IS_FORCEDBUFF)
                    SetPositive(100);
                if (m_spellInfo->getAttributes() & ATTRIBUTES_NEGATIVE)
                    SetNegative(100);
            }

        }
        else
            LOG_ERROR("Unknown Aura id %d", m_modList[x].m_type);
    }
}

void Aura::UpdateModifiers()
{
    for (uint8 x = 0; x < m_modcount; x++)
    {
        mod = &m_modList[x];

        if (mod->m_type < TOTAL_SPELL_AURAS)
        {
            LogDebugFlag(LF_AURA, "WORLD: Update Aura mods : target = %u , Spell Aura id = %u (%s), SpellId  = %u, EffectIndex = %u, duration = %u, damage = %d",
                      m_target->GetLowGUID(), mod->m_type, SpellAuraNames[mod->m_type], m_spellInfo->getId(), mod->m_effectIndex, GetDuration(), mod->m_amount);
            switch (mod->m_type)
            {
                case SPELL_AURA_MOD_DECREASE_SPEED:
                    UpdateAuraModDecreaseSpeed();
                    break;
                case SPELL_AURA_MOD_ATTACK_POWER_BY_STAT_PCT:
                case SPELL_AURA_MOD_RANGED_ATTACK_POWER_BY_STAT_PCT:
                    (*this.*SpellAuraHandler[mod->m_type])(false);
                    (*this.*SpellAuraHandler[mod->m_type])(true);
                    break;
            }
        }
        else
            LOG_ERROR("Unknown Aura id %d", (uint32)mod->m_type);
    }
}

void Aura::EventUpdateGroupAA(float r)
{
    Player* owner = nullptr;

    owner = static_cast< Player* >(m_target->GetPlayerOwner());

    if (owner == nullptr)
    {
        targets.clear();
        return;
    }

    if (!owner->InGroup())
    {
        if (m_target->GetGUID() != owner->GetGUID())
        {
            if ((m_target->getDistanceSq(owner) <= r))
            {
                if (!owner->HasAura(m_spellInfo->getId()))
                    targets.insert(owner->GetGUID());
            }
            else
            {
                if (owner->HasAura(m_spellInfo->getId()))
                {
                    targets.erase(owner->GetLowGUID());
                    owner->RemoveAura(m_spellInfo->getId());
                }
            }
        }
    }
    else
    {
        owner->GetGroup()->Lock();

        SubGroup* sg = owner->GetGroup()->GetSubGroup(owner->GetSubGroup());
        for (GroupMembersSet::iterator itr = sg->GetGroupMembersBegin(); itr != sg->GetGroupMembersEnd(); ++itr)
        {
            Player* op = (*itr)->m_loggedInPlayer;

            if (op == nullptr)
                continue;

            if (m_target->getDistanceSq(op) > r)
                continue;

            if (m_target->GetInstanceID() != op->GetInstanceID())
                continue;

            if ((m_target->GetPhase() & op->GetPhase()) == 0)
                continue;

            if (!op->isAlive())
                continue;

            if (op->HasAura(m_spellInfo->getId()))
                continue;

            targets.insert(op->GetGUID());
        }

        owner->GetGroup()->Unlock();
    }

    for (AreaAuraList::iterator itr = targets.begin(); itr != targets.end();)
    {
        AreaAuraList::iterator itr2 = itr;
        ++itr;

        Player* tp = m_target->GetMapMgr()->GetPlayer(Arcemu::Util::GUID_LOPART(*itr2));

        bool removable = false;
        if (tp == nullptr)
        {
            targets.erase(itr2);
            continue;
        }

        if (m_target->getDistanceSq(tp) > r)
            removable = true;

        if ((m_target->GetPhase() & tp->GetPhase()) == 0)
            removable = true;

        if ((tp->GetGUID() != owner->GetGUID()) && !tp->InGroup())
            removable = true;
        else
        {
            if (owner->InGroup())
            {
                if (owner->GetGroup()->GetID() != tp->GetGroup()->GetID())
                    removable = true;

                if (owner->GetSubGroup() != tp->GetSubGroup())
                    removable = true;
            }
        }

        if (removable)
        {
            targets.erase(itr2);
            tp->RemoveAura(m_spellInfo->getId());
        }
    }
}

void Aura::EventUpdateRaidAA(float r)
{
    Player* owner = nullptr;

    if (m_target->IsPlayer())
        owner = static_cast< Player* >(m_target);
    else
        owner = static_cast< Player* >(m_target->GetPlayerOwner());

    if (owner == nullptr)
    {
        targets.clear();
        return;
    }

    if (!owner->InGroup())
    {
        if (m_target->GetGUID() != owner->GetGUID())
        {
            if ((m_target->getDistanceSq(owner) <= r))
            {
                if (!owner->HasAura(m_spellInfo->getId()))
                    targets.insert(owner->GetGUID());
            }
            else
            {
                if (owner->HasAura(m_spellInfo->getId()))
                {
                    targets.erase(owner->GetLowGUID());
                    owner->RemoveAura(m_spellInfo->getId());
                }
            }
        }

    }
    else
    {
        Group* g = owner->GetGroup();

        g->Lock();
        uint32 subgroups = g->GetSubGroupCount();

        for (uint32 i = 0; i < subgroups; i++)
        {
            SubGroup* sg = g->GetSubGroup(i);

            for (GroupMembersSet::iterator itr = sg->GetGroupMembersBegin(); itr != sg->GetGroupMembersEnd(); ++itr)
            {
                PlayerInfo* pi = *itr;
                Player* op = pi->m_loggedInPlayer;

                if (op == nullptr)
                    continue;

                if (op->GetInstanceID() != m_target->GetInstanceID())
                    continue;

                if (m_target->getDistanceSq(op) > r)
                    continue;

                if ((m_target->GetPhase() & op->GetPhase()) == 0)
                    continue;

                if (!op->isAlive())
                    continue;

                if (op->HasAura(m_spellInfo->getId()))
                    continue;

                targets.insert(op->GetGUID());
            }
        }

        g->Unlock();
    }

    // Check for targets that should be no longer affected
    for (AreaAuraList::iterator itr = targets.begin(); itr != targets.end();)
    {
        AreaAuraList::iterator itr2 = itr;
        ++itr;

        Player* tp = m_target->GetMapMgr()->GetPlayer(Arcemu::Util::GUID_LOPART(*itr2));
        bool removable = false;

        if (tp == nullptr)
        {
            targets.erase(itr2);
            continue;
        }

        if (m_target->getDistanceSq(tp) > r)
            removable = true;

        if ((m_target->GetPhase() & tp->GetPhase()) == 0)
            removable = true;

        if ((tp->GetGUID() != owner->GetGUID()) && !tp->InGroup())
            removable = true;

        if (removable)
        {
            targets.erase(itr2);
            tp->RemoveAura(m_spellInfo->getId());
        }
    }
}

void Aura::EventUpdatePetAA(float r)
{
    Player* p = nullptr;

    if (m_target->IsPlayer())
        p = static_cast<Player*>(m_target);
    else
        return;

    std::list< Pet* > pl = p->GetSummons();
    for (std::list< Pet* >::iterator itr = pl.begin(); itr != pl.end(); ++itr)
    {
        Pet* pet = *itr;

        if (p->getDistanceSq(pet) > r)
            continue;

        if (!pet->isAlive())
            continue;

        if (pet->HasAura(m_spellInfo->getId()))
            continue;

        {
            Aura* a = sSpellFactoryMgr.NewAura(m_spellInfo, GetDuration(), p, pet, true);
            a->m_areaAura = true;
            a->AddMod(mod->m_type, mod->m_amount, mod->m_miscValue, mod->m_effectIndex);
            pet->AddAura(a);
        }
    }

    for (std::list< Pet* >::iterator itr = pl.begin(); itr != pl.end();)
    {
        std::list< Pet* >::iterator itr2 = itr;

        Pet* pet = *itr2;
        ++itr;

        if (p->getDistanceSq(pet) <= r)
            continue;

        pet->RemoveAura(m_spellInfo->getId());
    }
}

void Aura::EventUpdateFriendAA(float r)
{
    Unit* u = m_target;
    if (u == nullptr)
        return;

    for (const auto& itr : u->getInRangeObjectsSet())
    {
        Object* o = itr;

        if (!o || !o->IsUnit())
            continue;

        Unit* ou = static_cast<Unit*>(o);

        if (u->getDistanceSq(ou) > r)
            continue;

        if ((u->GetPhase() & ou->GetPhase()) == 0)
            continue;

        if (!ou->isAlive())
            continue;

        if (isHostile(u, ou))
            continue;

        if (isNeutral(u, ou))
            continue;

        if (ou->HasAura(m_spellInfo->getId()))
            continue;

        targets.insert(ou->GetGUID());
    }

    for (AreaAuraList::iterator itr = targets.begin(); itr != targets.end();)
    {
        AreaAuraList::iterator itr2 = itr;
        ++itr;

        Unit* tu = u->GetMapMgr()->GetUnit(*itr2);
        bool removable = false;

        if (tu == nullptr)
        {
            targets.erase(itr2);
            continue;
        }

        if (u->getDistanceSq(tu) > r)
            removable = true;

        if (isHostile(u, tu))
            removable = true;

        if (isNeutral(u, tu))
            removable = true;

        if ((u->GetPhase() & tu->GetPhase()) == 0)
            removable = true;

        if (removable)
        {
            tu->RemoveAura(m_spellInfo->getId());
            targets.erase(itr2);
        }
    }
}

void Aura::EventUpdateEnemyAA(float r)
{
    Unit* u = m_target;
    if (u == nullptr)
        return;

    for (const auto& itr : u->getInRangeObjectsSet())
    {
        Object* o = itr;

        if (!o || !o->IsUnit())
            continue;

        Unit* ou = static_cast<Unit*>(o);

        if (u->getDistanceSq(ou) > r)
            continue;

        if ((u->GetPhase() & ou->GetPhase()) == 0)
            continue;

        if (!ou->isAlive())
            continue;

        if (!isHostile(u, ou))
            continue;

        if (ou->HasAura(m_spellInfo->getId()))
            continue;

        targets.insert(ou->GetGUID());
    }

    for (AreaAuraList::iterator itr = targets.begin(); itr != targets.end();)
    {
        AreaAuraList::iterator itr2 = itr;
        ++itr;

        Unit* tu = u->GetMapMgr()->GetUnit(*itr2);
        bool removable = false;

        if (tu == nullptr)
        {
            targets.erase(itr2);
            continue;
        }

        if (u->getDistanceSq(tu) > r)
            removable = true;

        if (!isHostile(u, tu))
            removable = true;

        if (isNeutral(u, tu))
            removable = true;

        if ((u->GetPhase() & tu->GetPhase()) == 0)
            removable = true;

        if (removable)
        {
            tu->RemoveAura(m_spellInfo->getId());
            targets.erase(itr2);
        }
    }
}

void Aura::EventUpdateOwnerAA(float r)
{
    Object* o = GetTarget();
    if (!o->IsCreature())
        return;

    Creature* c = static_cast<Creature*>(o);
    if (!c->IsSummon())
        return;

    Unit* ou = nullptr;
    ou = static_cast< Summon* >(c)->GetOwner();

    if (ou == nullptr)
        return;

    if (ou->isAlive() &&
        !ou->HasAura(m_spellInfo->getId()) &&
        (c->getDistanceSq(ou) <= r))
    {

        Aura* a = sSpellFactoryMgr.NewAura(m_spellInfo, GetDuration(), c, ou, true);
        a->m_areaAura = true;
        a->AddMod(mod->m_type, mod->m_amount, mod->m_miscValue, mod->m_effectIndex);
        ou->AddAura(a);
    }


    if (!ou->isAlive() || (c->getDistanceSq(ou) > r))
        ou->RemoveAura(m_spellInfo->getId());
}

void Aura::EventUpdateAreaAura(float r)
{
    /* burlex: cheap hack to get this to execute in the correct context always */
    if (event_GetCurrentInstanceId() == -1)
    {
        event_Relocate();
        return;
    }

    Unit* u_caster = GetUnitCaster();

    // if the caster is no longer valid->remove the aura
    if (u_caster == nullptr)
    {
        Remove();
        //since we lost the caster we cannot do anything more
        return;
    }

    uint32 AreaAuraEffectId = m_spellInfo->GetAreaAuraEffectId();
    if (AreaAuraEffectId == 0)
    {
        LOG_ERROR("Spell %u (%s) has tried to update Area Aura targets but Spell has no Area Aura effect.", m_spellInfo->getId(), m_spellInfo->getName().c_str());
        ARCEMU_ASSERT(false);
    }

    switch (AreaAuraEffectId)
    {

        case SPELL_EFFECT_APPLY_GROUP_AREA_AURA:
            EventUpdateGroupAA(r);
            break;

        case SPELL_EFFECT_APPLY_RAID_AREA_AURA:
            EventUpdateRaidAA(r);
            break;

        case SPELL_EFFECT_APPLY_PET_AREA_AURA:
            EventUpdatePetAA(r);
            break;

        case SPELL_EFFECT_APPLY_FRIEND_AREA_AURA:
            EventUpdateFriendAA(r);
            break;

        case SPELL_EFFECT_APPLY_ENEMY_AREA_AURA:
            EventUpdateEnemyAA(r);
            break;

        case SPELL_EFFECT_APPLY_OWNER_AREA_AURA:
            EventUpdateOwnerAA(r);
            break;

        default:
            ARCEMU_ASSERT(false);
            break;
    }


    for (AreaAuraList::iterator itr = targets.begin(); itr != targets.end(); ++itr)
    {
        auto unit = m_target->GetMapMgr()->GetUnit(*itr);
        if (unit == nullptr)
            return;

        if (unit->HasAura(m_spellInfo->getId()))
            continue;

        Aura* a = sSpellFactoryMgr.NewAura(m_spellInfo, GetDuration(), m_target, unit, true);
        a->m_areaAura = true;
        a->AddMod(mod->m_type, mod->m_amount, mod->m_miscValue, mod->m_effectIndex);
        unit->AddAura(a);
    }
}

void Aura::ClearAATargets()
{
    uint32 spellid = m_spellInfo->getId();

    for (AreaAuraList::iterator itr = targets.begin(); itr != targets.end(); ++itr)
    {
        Unit* tu = m_target->GetMapMgr()->GetUnit(*itr);

        if (tu == nullptr)
            continue;

        tu->RemoveAura(spellid);
    }
    targets.clear();

    if (m_target->IsPlayer() && m_spellInfo->HasEffect(SPELL_EFFECT_APPLY_PET_AREA_AURA))
    {
        Player* p = static_cast<Player*>(m_target);

        std::list< Pet* > pl = p->GetSummons();
        for (std::list< Pet* >::iterator itr = pl.begin(); itr != pl.end(); ++itr)
        {
            Pet* pet = *itr;

            pet->RemoveAura(spellid);
        }
    }

    if (m_spellInfo->HasEffect(SPELL_EFFECT_APPLY_OWNER_AREA_AURA))
    {
        Unit* u = m_target->GetMapMgr()->GetUnit(m_target->GetCreatedByGUID());

        if (u != nullptr)
            u->RemoveAura(spellid);

    }
}

//------------------------- Aura Effects -----------------------------

void Aura::SpellAuraModBaseResistance(bool apply)
{
    SpellAuraModResistance(apply);
    //both add/decrease some resistance difference is unknown
}

void Aura::SpellAuraModBaseResistancePerc(bool apply)
{
    uint32 Flag = mod->m_miscValue;
    int32 amt;
    if (apply)
    {
        amt = mod->m_amount;
        if (amt > 0)
            SetPositive();
        else
            SetNegative();
    }
    else
        amt = -mod->m_amount;

    for (uint16 x = 0; x < 7; x++)
    {
        if (Flag & (((uint32)1) << x))
        {
            if (m_target->IsPlayer())
            {
                if (mod->m_amount > 0)
                {
                    static_cast< Player* >(m_target)->BaseResistanceModPctPos[x] += amt;
                }
                else
                {
                    static_cast< Player* >(m_target)->BaseResistanceModPctNeg[x] -= amt;
                }
                static_cast< Player* >(m_target)->CalcResistance(x);

            }
            else if (m_target->IsCreature())
            {
                static_cast< Creature* >(m_target)->BaseResistanceModPct[x] += amt;
                static_cast< Creature* >(m_target)->CalcResistance(x);
            }
        }
    }
}

void Aura::SpellAuraNULL(bool /*apply*/)
{
    LogDebugFlag(LF_AURA, "Unknown Aura id %d", (uint32)mod->m_type);
}

void Aura::SpellAuraBindSight(bool apply)
{
    SetPositive();
    // MindVision
    Player* caster = GetPlayerCaster();
    if (caster == nullptr)
        return;

    if (apply)
        caster->SetFarsightTarget(m_target->GetGUID());
    else
        caster->SetFarsightTarget(0);
}

void Aura::SpellAuraModPossess(bool apply)
{
    Unit* caster = GetPlayerCaster();

    if (apply)
    {
        if (caster != nullptr && caster->IsInWorld())
            caster->Possess(m_target);
    }
    else
    {
        if (caster != nullptr && caster->IsInWorld())
        {
            caster->UnPossess();
            m_target->RemoveAura(GetSpellId());
        }

        // make sure Player::UnPossess() didn't fail, if it did we will just free the target here
        if (m_target->GetCharmedByGUID() != 0)
        {
            if (m_target->IsCreature())
            {
                m_target->setAItoUse(true);
                m_target->m_redirectSpellPackets = nullptr;
            }

            m_target->SetCharmedByGUID(0);
            m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED_CREATURE | UNIT_FLAG_PVP_ATTACKABLE);
            m_target->SetFaction(m_target->GetCharmTempVal());
            m_target->updateInRangeOppositeFactionSet();
        }
        else
        {
            //mob woke up and realized he was controlled. He will turn to controller and also notify the other mobs he is fighting that they should attack the caster
            //sadly i got only 3 test cases about this so i might be wrong :(
            //zack : disabled until tested
            m_target->GetAIInterface()->EventChangeFaction(caster);
        }
    }
}

void Aura::SpellAuraPeriodicDamage(bool apply)
{
    if (apply)
    {
        if (m_spellInfo->getMechanicsType() == MECHANIC_BLEEDING && m_target->MechanicsDispels[MECHANIC_BLEEDING])
        {
            m_flags |= 1 << mod->m_effectIndex;
            return;
        }
        int32 dmg = mod->m_amount;
        Unit* c = GetUnitCaster();
        switch (m_spellInfo->getId())
        {
            case 703:
            case 8631:
            case 8632:
            case 8633:
            case 8818:
            case 11289:
            case 11290:
                if (c != nullptr)
                    c->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_START_ATTACK);  // remove stealth
                break;
                //mage talent ignite
            case 12654:
            {
                if (!pSpellId) //we need a parent spell and should always have one since it procs on it
                    break;
                SpellInfo* parentsp = sSpellCustomizations.GetSpellInfo(pSpellId);
                if (!parentsp)
                    return;
                if (c != nullptr && c->IsPlayer())
                {
                    dmg = float2int32(static_cast< Player* >(c)->m_casted_amount[SCHOOL_FIRE] * parentsp->getEffectBasePoints(0) / 100.0f);
                }
                else if (c != nullptr)
                {
                    if (!dmg)
                        return;
                    Spell* spell = sSpellFactoryMgr.NewSpell(c, parentsp, false, nullptr);
                    SpellCastTargets castTargets(m_target->GetGUID());

                    //this is so not good, maybe parent spell has more then dmg effect and we use it to calc our new dmg :(
                    dmg = 0;
                    for (uint8 i = 0; i < 3; ++i)
                    {
                        dmg += spell->CalculateEffect(i, m_target) * parentsp->getEffectBasePoints(0) / 100;
                    }
                    delete spell;
                    spell = nullptr;
                }
            }
        };
        //this is warrior : Deep Wounds
        if (c != nullptr && c->IsPlayer() && pSpellId)
        {
            uint32 multiplyer = 0;
            if (pSpellId == 12834)
                multiplyer = 16; //level 1 of the talent should apply 16 of average melee weapon dmg
            else if (pSpellId == 12849)
                multiplyer = 32;
            else if (pSpellId == 12867)
                multiplyer = 48;
            if (multiplyer)
            {
                Player* pr = static_cast< Player* >(c);
                Item* it;
                it = pr->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                if (it)
                {
                    dmg = 0;
                    for (uint8 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
                        if (it->GetItemProperties()->Damage[i].Type == SCHOOL_NORMAL)
                            dmg += int32((it->GetItemProperties()->Damage[i].Min + it->GetItemProperties()->Damage[i].Max) / 2);
                    dmg = multiplyer * dmg / 100;
                }
            }
        }
        uint32* gr = GetSpellInfo()->getSpellGroupType();
        if (gr)
        {
            if (c != nullptr)
            {
                spellModFlatIntValue(c->SM_FDOT, (int32*)&dmg, gr);
                spellModPercentageIntValue(c->SM_PDOT, (int32*)&dmg, gr);
            }
        }

        if (dmg <= 0)
            return; //who would want a negative dmg here ?

        LogDebugFlag(LF_AURA, "Adding periodic dmg aura, spellid: %lu", this->GetSpellId());
        sEventMgr.AddEvent(this, &Aura::EventPeriodicDamage, (uint32)dmg,
                           EVENT_AURA_PERIODIC_DAMAGE, GetSpellInfo()->getEffectAmplitude(mod->m_effectIndex), 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

        /*TO< Player* >(c)->GetSession()->SystemMessage("dot will do %u damage every %u seconds (total of %u)", dmg,m_spellProto->EffectAmplitude[mod->i],(GetDuration()/m_spellProto->EffectAmplitude[mod->i])*dmg);
        printf("dot will do %u damage every %u seconds (total of %u)\n", dmg,m_spellProto->EffectAmplitude[mod->i],(GetDuration()/m_spellProto->EffectAmplitude[mod->i])*dmg);*/
        SetNegative();
        if (m_spellInfo->custom_BGR_one_buff_on_target & SPELL_TYPE_WARLOCK_IMMOLATE)
        {
            m_target->SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_IMMOLATE);
        }
        //maybe poison aurastate should get triggered on other spells too ?
        else if (m_spellInfo->custom_c_is_flags & SPELL_FLAG_IS_POISON)  //deadly poison
        {
            m_target->SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_POISON);
        }
    }
    else if ((m_flags & (1 << mod->m_effectIndex)) == 0)   //add these checks to mods where immunity can cancel only 1 mod and not whole spell
    {
        if (m_spellInfo->custom_BGR_one_buff_on_target & SPELL_TYPE_WARLOCK_IMMOLATE)
            m_target->RemoveFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_IMMOLATE);
        //maybe poison aurastate should get triggered on other spells too ?
        else if (m_spellInfo->custom_c_is_flags & SPELL_FLAG_IS_POISON)  //deadly poison
        {
            m_target->RemoveFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_POISON);
        }
    }
}

void Aura::EventPeriodicDamage(uint32 amount)
{
    //DOT
    if (!m_target->isAlive())
        return;

    if (m_target->SchoolImmunityList[GetSpellInfo()->getSchool()])
    {
        if (GetUnitCaster() != nullptr)
            SendTickImmune(m_target, GetUnitCaster());
        return;
    }

    float res = static_cast<float>(amount);
    uint32 abs_dmg = 0;
    int32 bonus = 0;
    uint32 school = GetSpellInfo()->getSchool();
    Unit* c = GetUnitCaster();
    uint32 aproc = PROC_ON_ANY_HOSTILE_ACTION;
    uint32 vproc = PROC_ON_ANY_HOSTILE_ACTION | PROC_ON_ANY_DAMAGE_VICTIM;
    bool is_critical = false;

    if (m_target->GetGUID() != m_casterGuid)	//don't use resist when cast on self-- this is some internal stuff
    {
        if (c != nullptr)
        {
            uint32 amp = m_spellInfo->getEffectAmplitude(mod->m_effectIndex);
            if (!amp)
                amp = event_GetEventPeriod(EVENT_AURA_PERIODIC_DAMAGE);

            if (GetDuration())
            {
                switch (GetSpellInfo()->getId())
                {
                    //SPELL_HASH_IGNITE //static damage for Ignite. Need to be reworked when "static DoTs" will be implemented
                    case 3261:
                    case 11119:
                    case 11120:
                    case 12654:
                    case 12846:
                    case 12847:
                    case 12848:
                    case 52210:
                    case 58438:
                        break;
                    default:
                    {
                        int32 spbon = c->GetSpellDmgBonus(m_target, m_spellInfo, amount, true);
                        bonus += (spbon * static_cast<int32>(amp) / GetDuration());
                        res += static_cast<float>(bonus);

                        // damage taken is reduced after bonus damage is calculated and added
                        res += c->CalcSpellDamageReduction(m_target, m_spellInfo, res);
                    } break;
                }
            }

            if (res < 0.0f)
                res = 0.0f;
            else
            {
                float summaryPCTmod = 1.0f;
                if (p_target != nullptr)   //resilience
                {
                    float dmg_reduction_pct = p_target->CalcRating(PCR_MELEE_CRIT_RESILIENCE) / 100.0f;
                    if (dmg_reduction_pct > 1.0f)
                        dmg_reduction_pct = 1.0f;
                    summaryPCTmod -= dmg_reduction_pct;
                }
                res *= summaryPCTmod;
                if (res < 0.0f)
                    res = 0.0f;
            }

            if (DotCanCrit())
            {
                is_critical = c->IsCriticalDamageForSpell(m_target, GetSpellInfo());

                if (is_critical)
                {
                    res = c->GetCriticalDamageBonusForSpell(m_target, GetSpellInfo(), res);

                    aproc |= PROC_ON_SPELL_CRIT_HIT;
                    vproc |= PROC_ON_SPELL_CRIT_HIT_VICTIM;
                }
            }
        }

        uint32 ress = static_cast<uint32>(res);
        abs_dmg = m_target->AbsorbDamage(school, &ress);
        uint32 ms_abs_dmg = m_target->ManaShieldAbsorb(ress);
        if (ms_abs_dmg)
        {
            if (ms_abs_dmg > ress)
                ress = 0;
            else
                ress -= ms_abs_dmg;

            abs_dmg += ms_abs_dmg;
        }

        res = static_cast<float>(ress);
        dealdamage dmg;
        dmg.school_type = school;
        dmg.full_damage = ress;
        dmg.resisted_damage = 0;

        if (res <= 0.0f)
            dmg.resisted_damage = dmg.full_damage;

        if (res > 0.0f && c && m_spellInfo->getMechanicsType() != MECHANIC_BLEEDING)
        {
            c->CalculateResistanceReduction(m_target, &dmg, m_spellInfo, 0);
            if (static_cast<int32>(dmg.resisted_damage) > dmg.full_damage)
                res = 0.0f;
            else
                res = static_cast<float>(dmg.full_damage - dmg.resisted_damage);
        }

        m_target->SendPeriodicAuraLog(m_casterGuid, m_target->GetNewGUID(), GetSpellInfo()->getId(), school, static_cast<int32>(res), abs_dmg, dmg.resisted_damage, FLAG_PERIODIC_DAMAGE, is_critical);
    }

    // grep: this is hack.. some auras seem to delete this shit.
    SpellInfo* sp = m_spellInfo;

    if (m_target->m_damageSplitTarget)
        res = static_cast<float>(m_target->DoDamageSplitTarget(static_cast<uint32>(res), GetSpellInfo()->getSchool(), false));

    if (c != nullptr)
        c->DealDamage(m_target, static_cast<int32>(res), 2, 0, GetSpellId());
    else
        m_target->DealDamage(m_target, static_cast<int32>(res), 2, 0, GetSpellId());

    if (m_target->GetGUID() != m_casterGuid && c != nullptr)	//don't use resist when cast on self-- this is some internal stuff
    {
        int32 dmg = static_cast<int32>(res);

        if (abs_dmg)
            vproc |= PROC_ON_ABSORB;

        c->HandleProc(aproc, m_target, sp, false, dmg, abs_dmg);
        c->m_procCounter = 0;

        m_target->HandleProc(vproc, c, sp, false, dmg, abs_dmg);
        m_target->m_procCounter = 0;
    }
}

void Aura::SpellAuraDummy(bool apply)
{
    if (sScriptMgr.CallScriptedDummyAura(GetSpellId(), mod->m_effectIndex, this, apply))
        return;

    LogDebugFlag(LF_AURA_EFF, "Aura::SpellAuraDummy : Spell %u (%s) has an apply dummy aura effect, but no handler for it. ", m_spellInfo->getId(), m_spellInfo->getName().c_str());
}

void Aura::SpellAuraModConfuse(bool apply)
{
    Unit* u_caster = GetUnitCaster();

    if (m_target->IsTotem())
        return;

    if (apply)
    {
        if (u_caster == nullptr) return;

        // Check Mechanic Immunity
        if (m_target->MechanicsDispels[MECHANIC_DISORIENTED]
            || (m_spellInfo->getMechanicsType() == MECHANIC_POLYMORPHED && m_target->MechanicsDispels[MECHANIC_POLYMORPHED])
            )
        {
            m_flags |= 1 << mod->m_effectIndex;
            return;
        }
        SetNegative();

        m_target->addUnitStateFlag(UNIT_STATE_CONFUSE);
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);

        m_target->setAItoUse(true);
        m_target->GetAIInterface()->HandleEvent(EVENT_WANDER, u_caster, 0);

        if (p_target)
        {
            // this is a hackfix to stop player from moving -> see AIInterface::_UpdateMovement() Wander AI for more info
            WorldPacket data1(9);
            data1.Initialize(SMSG_CLIENT_CONTROL_UPDATE);
            data1 << m_target->GetNewGUID() << uint8(0x00);
            p_target->GetSession()->SendPacket(&data1);
            p_target->SpeedCheatDelay(GetDuration());
        }
    }
    else if ((m_flags & (1 << mod->m_effectIndex)) == 0)   //add these checks to mods where immunity can cancel only 1 mod and not whole spell
    {
        m_target->removeUnitStateFlag(UNIT_STATE_CONFUSE);
        m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
        if (p_target)
            p_target->SpeedCheatReset();

        m_target->GetAIInterface()->HandleEvent(EVENT_UNWANDER, nullptr, 0);

        if (p_target)
        {
            // re-enable movement
            WorldPacket data1(9);
            data1.Initialize(SMSG_CLIENT_CONTROL_UPDATE);
            data1 << m_target->GetNewGUID() << uint8(0x01);
            p_target->GetSession()->SendPacket(&data1);

            m_target->setAItoUse(false);

            if (u_caster != nullptr)
                sHookInterface.OnEnterCombat(p_target, u_caster);
        }
        else
            m_target->GetAIInterface()->AttackReaction(u_caster, 1, 0);
    }
}

void Aura::SpellAuraModCharm(bool apply)
{
    Player* caster = GetPlayerCaster();
    if (caster == nullptr)
        return;
    if (!m_target->IsCreature())
        return;

    Creature* target = static_cast< Creature* >(m_target);

    if (target->IsTotem())
        return;

    SetPositive(3); //we ignore the other 2 effect of this spell and force it to be a positive spell

    if (apply)
    {
        if ((int32)m_target->getLevel() > mod->m_amount || m_target->IsPet())
            return;

        // this should be done properly
        if (target->GetEnslaveCount() >= 10)
            return;

        if (caster->GetCharmedUnitGUID() != 0)
            return;

        m_target->addUnitStateFlag(UNIT_STATE_CHARM);
        m_target->SetCharmTempVal(m_target->GetFaction());
        m_target->SetFaction(caster->GetFaction());
        m_target->updateInRangeOppositeFactionSet();
        m_target->GetAIInterface()->Init(m_target, AI_SCRIPT_PET, Movement::WP_MOVEMENT_SCRIPT_NONE, caster);
        m_target->SetCharmedByGUID(caster->GetGUID());
        caster->SetCharmedUnitGUID(target->GetGUID());
        //damn it, the other effects of enslave demon will agro him on us anyway :S
        m_target->GetAIInterface()->WipeHateList();
        m_target->GetAIInterface()->WipeTargetList();
        m_target->GetAIInterface()->resetNextTarget();

        target->SetEnslaveCount(target->GetEnslaveCount() + 1);

        if (caster->GetSession())   // crashfix
        {
            WorldPacket data(SMSG_PET_SPELLS, 500);
            data << target->GetGUID();
            data << uint16(0);
            data << uint32(0x1000);
            data << uint32(0x100);
            data << uint32(PET_SPELL_ATTACK);
            data << uint32(PET_SPELL_FOLLOW);
            data << uint32(PET_SPELL_STAY);
            for (uint8 i = 0; i < 4; i++)
                data << uint32(0);
            data << uint32(PET_SPELL_AGRESSIVE);
            data << uint32(PET_SPELL_DEFENSIVE);
            data << uint32(PET_SPELL_PASSIVE);
            caster->GetSession()->SendPacket(&data);
            target->SetEnslaveSpell(m_spellInfo->getId());
        }
    }
    else
    {
        m_target->removeUnitStateFlag(UNIT_STATE_CHARM);
        m_target->SetFaction(m_target->GetCharmTempVal());
        m_target->GetAIInterface()->WipeHateList();
        m_target->GetAIInterface()->WipeTargetList();
        m_target->updateInRangeOppositeFactionSet();
        m_target->GetAIInterface()->Init(m_target, AI_SCRIPT_AGRO, Movement::WP_MOVEMENT_SCRIPT_NONE);
        m_target->SetCharmedByGUID(0);

        if (caster->GetSession() != nullptr)   // crashfix
        {
            caster->SetCharmedUnitGUID(0);
            WorldPacket data(SMSG_PET_SPELLS, 8);
            data << uint64(0);
            caster->GetSession()->SendPacket(&data);
            target->SetEnslaveSpell(0);
        }
    }
}

void Aura::SpellAuraModFear(bool apply)
{
    Unit* u_caster = GetUnitCaster();

    if (m_target->IsCreature() &&
        (m_target->IsTotem() || static_cast< Creature* >(m_target)->isRooted()))
        return;

    if (apply)
    {
        if (u_caster == nullptr) return;
        // Check Mechanic Immunity
        if (m_target->MechanicsDispels[MECHANIC_FLEEING])
        {
            m_flags |= 1 << mod->m_effectIndex;
            return;
        }

        SetNegative();

        m_target->addUnitStateFlag(UNIT_STATE_FEAR);
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING);

        m_target->setAItoUse(true);
        m_target->GetAIInterface()->HandleEvent(EVENT_FEAR, u_caster, 0);
        m_target->m_fearmodifiers++;
        if (p_target)
        {
            // this is a hackfix to stop player from moving -> see AIInterface::_UpdateMovement() Fear AI for more info
            WorldPacket data1(9);
            data1.Initialize(SMSG_CLIENT_CONTROL_UPDATE);
            data1 << m_target->GetNewGUID() << uint8(0x00);
            p_target->GetSession()->SendPacket(&data1);
            p_target->SpeedCheatDelay(GetDuration());
        }
    }
    else if ((m_flags & (1 << mod->m_effectIndex)) == 0)   //add these checks to mods where immunity can cancel only 1 mod and not whole spell
    {
        m_target->m_fearmodifiers--;

        if (m_target->m_fearmodifiers <= 0)
        {
            m_target->removeUnitStateFlag(UNIT_STATE_FEAR);
            m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FLEEING);

            m_target->GetAIInterface()->HandleEvent(EVENT_UNFEAR, nullptr, 0);

            if (p_target)
            {
                // re-enable movement
                WorldPacket data1(9);
                data1.Initialize(SMSG_CLIENT_CONTROL_UPDATE);
                data1 << m_target->GetNewGUID() << uint8(0x01);
                p_target->GetSession()->SendPacket(&data1);

                m_target->setAItoUse(false);

                if (u_caster != nullptr)
                    sHookInterface.OnEnterCombat(p_target, u_caster);
                p_target->SpeedCheatReset();
            }
            else
                m_target->GetAIInterface()->AttackReaction(u_caster, 1, 0);
        }
    }
}

void Aura::SpellAuraPeriodicHeal(bool apply)
{
    if (apply)
    {
        SetPositive();

        int32 val = mod->m_amount;
        Unit* c = GetUnitCaster();
        if (c != nullptr)
        {
            spellModFlatIntValue(c->SM_FMiscEffect, &val, GetSpellInfo()->getSpellGroupType());
            spellModPercentageIntValue(c->SM_PMiscEffect, &val, GetSpellInfo()->getSpellGroupType());
        }

        if (val > 0)
        {
            sEventMgr.AddEvent(this, &Aura::EventPeriodicHeal, (uint32)val, EVENT_AURA_PERIODIC_HEAL, GetSpellInfo()->getEffectAmplitude(mod->m_effectIndex), 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

            switch (GetSpellInfo()->getId())
            {
                //SPELL_HASH_REJUVENATION
                case 774:
                case 1058:
                case 1430:
                case 2090:
                case 2091:
                case 3627:
                case 8070:
                case 8910:
                case 9839:
                case 9840:
                case 9841:
                case 12160:
                case 15981:
                case 20664:
                case 20701:
                case 25299:
                case 26981:
                case 26982:
                case 27532:
                case 28716:
                case 28722:
                case 28723:
                case 28724:
                case 31782:
                case 32131:
                case 38657:
                case 42544:
                case 48440:
                case 48441:
                case 53607:
                case 64801:
                case 66065:
                case 67971:
                case 67972:
                case 67973:
                case 69898:
                case 70691:
                case 71142:
                //SPELL_HASH_REGROWTH
                case 8936:
                case 8938:
                case 8939:
                case 8940:
                case 8941:
                case 9750:
                case 9856:
                case 9857:
                case 9858:
                case 16561:
                case 20665:
                case 22373:
                case 22695:
                case 26980:
                case 27637:
                case 28744:
                case 34361:
                case 39000:
                case 39125:
                case 48442:
                case 48443:
                case 66067:
                case 67968:
                case 67969:
                case 67970:
                case 69882:
                case 71141:
                {
                    m_target->SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_REJUVENATE);
                    if (!sEventMgr.HasEvent(m_target, EVENT_REJUVENATION_FLAG_EXPIRE))
                    {
                        sEventMgr.AddEvent(m_target, &Unit::EventAurastateExpire, (uint32)AURASTATE_FLAG_REJUVENATE, EVENT_REJUVENATION_FLAG_EXPIRE, GetDuration(), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                    }
                    else
                    {
                        sEventMgr.ModifyEventTimeLeft(m_target, EVENT_REJUVENATION_FLAG_EXPIRE, GetDuration(), 0);
                    }
                } break;
            }
        }
    }
}

void Aura::EventPeriodicHeal(uint32 amount)
{
    if (!m_target->isAlive())
        return;

    Unit* c = GetUnitCaster();

    int32 bonus = 0;
    bool is_critical = false;

    if (c != nullptr)
    {
        bonus += c->HealDoneMod[m_spellInfo->getSchool()] + m_target->HealTakenMod[m_spellInfo->getSchool()];
        if (c->IsPlayer())
        {
            for (uint16_t a = 0; a < 5; a++)
            {
                bonus += float2int32(static_cast<Player*>(c)->SpellHealDoneByAttribute[a][m_spellInfo->getSchool()] * static_cast<Player*>(c)->GetStat(a));
            }
        }
        //Spell Coefficient
        if (m_spellInfo->OTspell_coef_override >= 0)   //In case we have forced coefficients
            bonus = float2int32(bonus * m_spellInfo->OTspell_coef_override);
        else
        {
            //Bonus to HoT part
            if (m_spellInfo->fixed_hotdotcoef >= 0)
            {
                bonus = float2int32(bonus * m_spellInfo->fixed_hotdotcoef);
                //we did most calculations in world.cpp, but talents that increase DoT spells duration
                //must be included now.
                if (c->IsPlayer())
                {
                    int durmod = 0;
                    spellModFlatIntValue(c->SM_FDur, &durmod, m_spellInfo->getSpellGroupType());
                    bonus += bonus * durmod / 15000;
                }
            }
        }

        /*
        int penalty_pct = 0;
        int penalty_flt = 0;
        spellModFlatIntValue(c->SM_PPenalty, &penalty_pct, GetSpellProto()->SpellGroupType);
        bonus += bonus * (penalty_pct / 100);
        spellModFlatIntValue(c->SM_FPenalty, &penalty_flt, GetSpellProto()->SpellGroupType);
        bonus += penalty_flt;
        */
        spellModPercentageIntValue(c->SM_PPenalty, &bonus, m_spellInfo->getSpellGroupType());
    }

    int amp = m_spellInfo->getEffectAmplitude(mod->m_effectIndex);
    if (!amp)
        amp = event_GetEventPeriod(EVENT_AURA_PERIODIC_HEAL);
    // Healing Stream is not a HOT
    {
        int32 dur = GetDuration();
        //example : Citrine Pendant of Golden Healing is in AA aura that does not have duration. In this case he would have full healbonus benefit
        if ((dur == 0 || dur == -1) && GetSpellInfo()->getDurationIndex())
        {
            auto spell_duration = sSpellDurationStore.LookupEntry(GetSpellInfo()->getDurationIndex());
            if (spell_duration != nullptr)
                dur = ::GetDuration(spell_duration);
        }
        if (dur && dur != -1)
        {
            int ticks = (amp > 0) ? dur / amp : 0;
            bonus = (ticks > 0) ? bonus / ticks : 0;
        }
        //removed by Zack : Why is this directly setting bonus to 0 ? It's not logical
        //		else bonus = 0;
    }
    /*Downranking
    if (c != NULL && c->IsPlayer())
    {
    if (m_spellProto->baseLevel > 0 && m_spellProto->maxLevel > 0)
    {
    float downrank1 = 1.0f;
    if (m_spellProto->baseLevel < 20)
    downrank1 = 1.0f - (20.0f - float(m_spellProto->baseLevel)) * 0.0375f;

    float downrank2 = (float(m_spellProto->maxLevel + 5.0f) / float(c->getLevel()));
    if (downrank2 >= 1 || downrank2 < 0)
    downrank2 = 1.0f;

    bonus = float2int32(float(bonus) * downrank1 * downrank2);
    }
    }*/

    int add = (bonus + amount > 0) ? bonus + amount : 0;
    if (c != nullptr)
    {
        add += float2int32(add * (m_target->HealTakenPctMod[m_spellInfo->getSchool()] + c->HealDonePctMod[GetSpellInfo()->getSchool()]));
        spellModPercentageIntValue(c->SM_PDOT, &add, m_spellInfo->getSpellGroupType());

        if (this->DotCanCrit())
        {
            is_critical = c->IsCriticalHealForSpell(m_target, GetSpellInfo());
            if (is_critical)
                add = float2int32(c->GetCriticalHealBonusForSpell(m_target, GetSpellInfo(), (float)add));
        }
    }

    uint32 curHealth = m_target->GetHealth();
    uint32 maxHealth = m_target->GetMaxHealth();
    uint32 over_heal = 0;

    if ((curHealth + add) >= maxHealth)
    {
        m_target->SetHealth(maxHealth);
        over_heal = curHealth + add - maxHealth;
    }
    else
        m_target->ModHealth(add);

    m_target->SendPeriodicHealAuraLog(m_casterGuid, m_target->GetNewGUID(), GetSpellId(), add, over_heal, is_critical);

    m_target->RemoveAurasByHeal();

    if (GetSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
    {
        m_target->Emote(EMOTE_ONESHOT_EAT);
    }

    // add threat
    Unit* u_caster = GetUnitCaster();
    if (u_caster != nullptr)
    {
        if (add > 0)
        {
            switch (GetSpellInfo()->getId())
            {
                //SPELL_HASH_HEALTH_FUNNEL
                case 755:
                case 3698:
                case 3699:
                case 3700:
                case 11693:
                case 11694:
                case 11695:
                case 16569:
                case 27259:
                case 40671:
                case 46467:
                case 47856:
                case 60829:
                {
                    dealdamage sdmg;

                    sdmg.full_damage = add;
                    sdmg.resisted_damage = 0;
                    sdmg.school_type = 0;
                    u_caster->DealDamage(u_caster, add, 0, 0, 0);
                    u_caster->SendAttackerStateUpdate(u_caster, u_caster, &sdmg, add, 0, 0, 0, ATTACK);
                } break;
                default:
                    break;
            }
        }

        std::vector<Unit*> target_threat;
        int count = 0;
        Creature* tmp_creature = nullptr;
        for (const auto& itr : u_caster->getInRangeObjectsSet())
        {
            if (!itr || !itr->IsCreature())
                continue;
            tmp_creature = static_cast<Creature*>(itr);
            if (!tmp_creature->CombatStatus.IsInCombat() || (tmp_creature->GetAIInterface()->getThreatByPtr(u_caster) == 0 && tmp_creature->GetAIInterface()->getThreatByPtr(m_target) == 0))
                continue;

            if (!(u_caster->GetPhase() & tmp_creature->GetPhase()))   //Can't see, no threat
                continue;

            target_threat.push_back(tmp_creature);
            count++;
        }
        if (count == 0)
            return;

        add = add / count;

        for (std::vector<Unit*>::iterator itr = target_threat.begin(); itr != target_threat.end(); ++itr)
        {
            static_cast< Unit* >(*itr)->GetAIInterface()->HealReaction(u_caster, m_target, m_spellInfo, add);
        }

        if (m_target->IsInWorld() && u_caster->IsInWorld())
            u_caster->CombatStatus.WeHealed(m_target);
    }
}

void Aura::SpellAuraModAttackSpeed(bool apply)
{
    if (mod->m_amount < 0)
        SetNegative();
    else
        SetPositive();

    if (m_target->IsPlayer())
    {
        if (apply)
        {
            static_cast< Player* >(m_target)->ModAttackSpeed(mod->m_amount, MOD_MELEE);
        }
        else
        {
            static_cast< Player* >(m_target)->ModAttackSpeed(-mod->m_amount, MOD_MELEE);
        }
        static_cast< Player* >(m_target)->UpdateStats();
    }
    else
    {
        if (apply)
        {
            mod->fixed_amount[0] = m_target->getPercentModUInt32Value(UNIT_FIELD_BASEATTACKTIME, mod->m_amount);
            mod->fixed_amount[1] = m_target->getPercentModUInt32Value(UNIT_FIELD_BASEATTACKTIME + 1, mod->m_amount);
            mod->fixed_amount[2] = m_target->getPercentModUInt32Value(UNIT_FIELD_RANGEDATTACKTIME, mod->m_amount);
            m_target->ModBaseAttackTime(MELEE, -mod->fixed_amount[0]);
            m_target->ModBaseAttackTime(OFFHAND, -mod->fixed_amount[1]);
            m_target->ModBaseAttackTime(RANGED, -mod->fixed_amount[2]);
        }
        else
        {
            m_target->ModBaseAttackTime(MELEE, mod->fixed_amount[0]);
            m_target->ModBaseAttackTime(OFFHAND, mod->fixed_amount[1]);
            m_target->ModBaseAttackTime(RANGED, mod->fixed_amount[2]);
        }
    }

}

void Aura::SpellAuraModThreatGenerated(bool apply)
{
    mod->m_amount < 0 ? SetPositive() : SetNegative();
    for (uint32 x = 0; x < 7; x++)
    {
        if (mod->m_miscValue & (((uint32)1) << x))
        {
            if (apply)
                m_target->ModGeneratedThreatModifyer(x, mod->m_amount);
            else
                m_target->ModGeneratedThreatModifyer(x, -(mod->m_amount));
        }
    }
}

void Aura::SpellAuraModTaunt(bool apply)
{
    Unit* m_caster = GetUnitCaster();
    if (!m_caster || !m_caster->isAlive())
        return;

    SetNegative();

    if (apply)
    {
        m_target->GetAIInterface()->AttackReaction(m_caster, 1, 0);
        m_target->GetAIInterface()->taunt(m_caster, true);
    }
    else
    {
        if (m_target->GetAIInterface()->getTauntedBy() == m_caster)
        {
            m_target->GetAIInterface()->taunt(m_caster, false);
        }
    }
}

void Aura::SpellAuraModStun(bool apply)
{
    if (apply)
    {
        // Check Mechanic Immunity
        // Stun is a tricky one... it's used for all different kinds of mechanics as a base Aura
        
        switch (m_spellInfo->getId())
        {
            //SPELL_HASH_ICE_BLOCK
            case 27619:
            case 36911:
            case 41590:
            case 45438:
            case 45776:
            case 46604:
            case 46882:
            case 56124:
            case 56644:
            case 62766:
            case 65802:
            case 69924:
                break;
            default:
            {
                if (!IsPositive())    // ice block stuns you, don't want our own spells to ignore stun effects
                {
                    if ((m_spellInfo->getMechanicsType() == MECHANIC_CHARMED &&  m_target->MechanicsDispels[MECHANIC_CHARMED])
                        || (m_spellInfo->getMechanicsType() == MECHANIC_INCAPACIPATED && m_target->MechanicsDispels[MECHANIC_INCAPACIPATED])

                        || (m_spellInfo->getMechanicsType() == MECHANIC_SAPPED && m_target->MechanicsDispels[MECHANIC_SAPPED])
                        || (m_target->MechanicsDispels[MECHANIC_STUNNED])
                        )
                    {
                        m_flags |= 1 << mod->m_effectIndex;
                        return;
                    }
                }
            } break;
        }
        SetNegative();

        //\todo Zyres: is tis relly the way this should work?
        m_target->m_rootCounter++;

        if (m_target->m_rootCounter == 1)
            m_target->setMoveRoot(true);

        if (m_target->IsStealth())
            m_target->RemoveStealth();

        m_target->m_stunned++;
        m_target->addUnitStateFlag(UNIT_STATE_STUN);
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);

        if (m_target->IsCreature())
            m_target->GetAIInterface()->resetNextTarget();

        // remove the current spell
        if (m_target->isCastingNonMeleeSpell())
        {
            m_target->interruptSpell();
        }

        //warrior talent - second wind triggers on stun and immobilize. This is not used as proc to be triggered always !
        Unit* caster = GetUnitCaster();
        if (caster != nullptr)
        {
            caster->EventStunOrImmobilize(m_target);
            m_target->EventStunOrImmobilize(caster, true);
        }
    }
    else if ((m_flags & (1 << mod->m_effectIndex)) == 0)   //add these checks to mods where immunity can cancel only 1 mod and not whole spell
    {
        //\todo Zyres: is tis relly the way this should work?
        m_target->m_rootCounter--;

        if (m_target->m_rootCounter == 0)
            m_target->setMoveRoot(false);

        m_target->m_stunned--;

        if (m_target->m_stunned == 0)
        {
            m_target->removeUnitStateFlag(UNIT_STATE_STUN);
            m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
        }

        // attack them back.. we seem to lose this sometimes for some reason
        if (m_target->IsCreature())
        {
            Unit* target = GetUnitCaster();
            if (m_target->GetAIInterface()->getNextTarget() != nullptr)
                target = m_target->GetAIInterface()->getNextTarget();

            if (target == nullptr)
                return;
            m_target->GetAIInterface()->AttackReaction(target, 1, 0);
        }
    }

    /*
        if (apply)
        {
        switch(this->m_spellProto->getId())
        {
        case 652:
        case 2070:
        case 6770:
        case 6771:
        case 11297:
        case 11298:
        {
        // sap
        Unit* c = GetUnitCaster();
        if (c)
        c->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_START_ATTACK);  // remove stealth
        }break;
        case 1776:
        case 1777:
        case 1780:
        case 1781:
        case 8629:
        case 8630:
        case 11285:
        case 11286:
        case 11287:
        case 11288:
        case 12540:
        case 13579:
        case 24698:
        case 28456:
        {
        // gouge
        Unit* c = GetUnitCaster();
        if (c && c->GetTypeId() == TYPEID_PLAYER)
        {
        //TO< Player* >(c)->CombatModeDelay = 10;
        TO< Player* >(c)->EventAttackStop();
        c->smsg_AttackStop(m_target);
        c->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_START_ATTACK);  // remove stealth
        }
        }
        }
        }*/
}

void Aura::SpellAuraModDamageDone(bool apply)
{
    int32 val;

    if (m_target->IsPlayer())
    {
        uint16_t index;

        if (mod->m_amount > 0)
        {
            if (apply)
            {
                SetPositive();
                val = mod->m_amount;
            }
            else
            {
                val = -mod->m_amount;
            }
            index = PLAYER_FIELD_MOD_DAMAGE_DONE_POS;

        }
        else
        {
            if (apply)
            {
                SetNegative();
                val = -mod->m_amount;
            }
            else
            {
                val = mod->m_amount;
            }
            index = PLAYER_FIELD_MOD_DAMAGE_DONE_NEG;
        }

        for (uint16_t x = 0; x < 7; x++)
        {
            if (mod->m_miscValue & (((uint32)1) << x))
            {
                m_target->modInt32Value(index + x, val);
            }
        }
    }
    else if (m_target->IsCreature())
    {
        if (mod->m_amount > 0)
        {
            if (apply)
            {
                SetPositive();
                val = mod->m_amount;
            }
            else
            {
                val = -mod->m_amount;
            }

        }
        else
        {
            if (apply)
            {
                SetNegative();
                val = mod->m_amount;
            }
            else
            {
                val = -mod->m_amount;
            }
        }

        for (uint32 x = 0; x < 7; x++)
        {
            if (mod->m_miscValue & (((uint32)1) << x))
            {
                static_cast< Creature* >(m_target)->ModDamageDone[x] += val;
            }
        }
    }

    if (mod->m_miscValue & 1)
        m_target->CalcDamage();
}

void Aura::SpellAuraModDamageTaken(bool apply)
{
    int32 val = (apply) ? mod->m_amount : -mod->m_amount;
    for (uint32 x = 0; x < 7; x++)
    {
        if (mod->m_miscValue & (((uint32)1) << x))
        {
            m_target->DamageTakenMod[x] += val;
        }
    }
}

void Aura::SpellAuraDamageShield(bool apply)
{
    if (apply)
    {
        SetPositive();
        DamageProc ds;// = new DamageShield();
        ds.m_damage = mod->m_amount;
        ds.m_spellId = GetSpellInfo()->getId();
        ds.m_school = GetSpellInfo()->getSchool();
        ds.m_flags = PROC_ON_MELEE_ATTACK_VICTIM | PROC_MISC; //maybe later we might want to add other flags too here
        ds.owner = (void*)this;
        m_target->m_damageShields.push_back(ds);
    }
    else
    {
        for (std::list<struct DamageProc>::iterator i = m_target->m_damageShields.begin(); i != m_target->m_damageShields.end(); ++i)
        {
            if (i->owner == this)
            {
                m_target->m_damageShields.erase(i);
                return;
            }
        }
    }
}

void Aura::SpellAuraModStealth(bool apply)
{
    if (apply)
    {
        //Overkill must proc only if we aren't already stealthed, also refreshing duration.
        if (!m_target->IsStealth() && m_target->HasAura(58426))
        {
            Aura *buff = m_target->getAuraWithId(58427);
            if (buff)
            {
                // Spell Overkill - in stealth and 20 seconds after stealth +30% energy regeneration - -1 duration => hacky infinity
                m_target->SetAurDuration(58427, static_cast<uint32_t>(-1));
                m_target->ModVisualAuraStackCount(buff, 0);
            }
            else
                m_target->CastSpell(m_target, 58427, true);
        }

        if (p_target && p_target->m_bgHasFlag)
        {
            if (p_target->m_bg && p_target->m_bg->GetType() == BATTLEGROUND_WARSONG_GULCH)
            {
                p_target->m_bg->HookOnFlagDrop(p_target);
            }
            if (p_target->m_bg && p_target->m_bg->GetType() == BATTLEGROUND_EYE_OF_THE_STORM)
            {
                p_target->m_bg->HookOnFlagDrop(p_target);
            }
        }

        SetPositive();
        switch (m_spellInfo->getId())
        {
            //SPELL_HASH_VANISH
            case 1856:
            case 1857:
            case 11327:
            case 11329:
            case 24223:
            case 24228:
            case 24229:
            case 24230:
            case 24231:
            case 24232:
            case 24233:
            case 24699:
            case 26888:
            case 26889:
            case 27617:
            case 29448:
            case 31619:
            case 35205:
            case 39667:
            case 41476:
            case 41479:
            case 44290:
            case 55964:
            case 71400:
                m_target->SetStealth(GetSpellId());
                break;
            //SPELL_HASH_STEALTH
            case 1784:
            case 1785:
            case 1786:
            case 1787:
            case 8822:
            case 30831:
            case 30991:
            case 31526:
            case 31621:
            case 32199:
            case 32615:
            case 34189:
            case 42347:
            case 42866:
            case 42943:
            case 52188:
            case 58506:
                m_target->SetFlag(UNIT_FIELD_BYTES_2, 0x1E000000); //sneak anim
                break;
        }

        m_target->SetFlag(UNIT_FIELD_BYTES_1, 0x020000);
        if (m_target->IsPlayer())
            m_target->SetFlag(PLAYER_FIELD_BYTES2, 0x2000);

        m_target->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_STEALTH | AURA_INTERRUPT_ON_INVINCIBLE);
        m_target->m_stealthLevel += mod->m_amount;

        // hack fix for vanish stuff
        if (m_target->IsPlayer())   // Vanish
        {
            switch (m_spellInfo->getId())
            {
                //SPELL_HASH_VANISH
                case 1856:
                case 1857:
                case 11327:
                case 11329:
                case 24223:
                case 24228:
                case 24229:
                case 24230:
                case 24231:
                case 24232:
                case 24233:
                case 24699:
                case 26888:
                case 26889:
                case 27617:
                case 29448:
                case 31619:
                case 35205:
                case 39667:
                case 41476:
                case 41479:
                case 44290:
                case 55964:
                case 71400:
                {
                    for (const auto& iter : m_target->getInRangeObjectsSet())
                    {
                        if (iter == nullptr || !iter->IsUnit())
                            continue;

                        Unit* _unit = static_cast<Unit*>(iter);
                        if (!_unit->isAlive())
                            continue;

                        if (_unit->isCastingNonMeleeSpell())
                        {
                            for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
                            {
                                Spell* curSpell = _unit->getCurrentSpell(CurrentSpellType(i));
                                if (curSpell != nullptr && curSpell->GetUnitTarget() == m_target)
                                {
                                    _unit->interruptSpellWithSpellType(CurrentSpellType(i));
                                }
                            }
                        }

                        if (_unit->GetAIInterface() != nullptr)
                            _unit->GetAIInterface()->RemoveThreatByPtr(m_target);
                    }

                    for (uint32 x = MAX_POSITIVE_AURAS_EXTEDED_START; x < MAX_POSITIVE_AURAS_EXTEDED_END; x++)
                    {
                        if (m_target->m_auras[x] != nullptr)
                        {
                            if (m_target->m_auras[x]->GetSpellInfo()->getMechanicsType() == MECHANIC_ROOTED || m_target->m_auras[x]->GetSpellInfo()->getMechanicsType() == MECHANIC_ENSNARED)   // Remove roots and slow spells
                            {
                                m_target->m_auras[x]->Remove();
                            }
                            else // if got immunity for slow, remove some that are not in the mechanics
                            {
                                for (uint8 i = 0; i < 3; i++)
                                {
                                    uint32 AuraEntry = m_target->m_auras[x]->GetSpellInfo()->getEffectApplyAuraName(i);
                                    if (AuraEntry == SPELL_AURA_MOD_DECREASE_SPEED || AuraEntry == SPELL_AURA_MOD_ROOT || AuraEntry == SPELL_AURA_MOD_STALKED)
                                    {
                                        m_target->m_auras[x]->Remove();
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    // Cast stealth spell/dismount/drop BG flag
                    if (p_target != nullptr)
                    {
                        p_target->CastSpell(p_target, 1784, true);

                        p_target->Dismount();

                        if (p_target->m_bg && p_target->m_bgHasFlag)
                        {
                            if (p_target->m_bg->GetType() == BATTLEGROUND_WARSONG_GULCH || p_target->m_bg->GetType() == BATTLEGROUND_EYE_OF_THE_STORM)
                            {
                                p_target->m_bg->HookOnFlagDrop(p_target);
                            }
                        }
                    }
                } break;
            }
        }
    }
    else
    {
        m_target->m_stealthLevel -= mod->m_amount;

        switch (m_spellInfo->getId())
        {
            //SPELL_HASH_VANISH
            case 1856:
            case 1857:
            case 11327:
            case 11329:
            case 24223:
            case 24228:
            case 24229:
            case 24230:
            case 24231:
            case 24232:
            case 24233:
            case 24699:
            case 26888:
            case 26889:
            case 27617:
            case 29448:
            case 31619:
            case 35205:
            case 39667:
            case 41476:
            case 41479:
            case 44290:
            case 55964:
            case 71400:
                break;
            default:
            {
                m_target->SetStealth(0);
                m_target->RemoveFlag(UNIT_FIELD_BYTES_2, 0x1E000000);

                m_target->RemoveFlag(UNIT_FIELD_BYTES_1, 0x020000);

                if (p_target != nullptr)
                {
                    p_target->RemoveFlag(PLAYER_FIELD_BYTES2, 0x2000);
                    p_target->SendSpellCooldownEvent(m_spellInfo->getId());

                    if (p_target->m_outStealthDamageBonusPeriod && p_target->m_outStealthDamageBonusPct)
                        p_target->m_outStealthDamageBonusTimer = (uint32)UNIXTIME + p_target->m_outStealthDamageBonusPeriod;
                }
            } break;
        }

        switch (m_spellInfo->getId())
        {
            //SPELL_HASH_STEALTH
            case 1784:
            case 1785:
            case 1786:
            case 1787:
            case 8822:
            case 30831:
            case 30991:
            case 31526:
            case 31621:
            case 32199:
            case 32615:
            case 34189:
            case 42347:
            case 42866:
            case 42943:
            case 52188:
            case 58506:
            {
                for (uint32 x = MAX_POSITIVE_AURAS_EXTEDED_START; x < MAX_POSITIVE_AURAS_EXTEDED_END; x++)
                {
                    if (m_target->m_auras[x] && m_target->m_auras[x]->GetSpellInfo()->getEffectApplyAuraName(0) != SPELL_AURA_DUMMY)
                    {
                        uint32 tmp_duration = 0;

                        switch (m_target->m_auras[x]->GetSpellInfo()->getId())
                        {
                            //SPELL_HASH_MASTER_OF_SUBTLETY
                            case 31221:
                            case 31222:
                            case 31223:
                            case 31665:
                            case 31666:
                            {
                                tmp_duration = MSTIME_6SECONDS;
                            } break;

                            //SPELL_HASH_OVERKILL
                            case 58426:
                            case 58427:
                            {
                                tmp_duration = MSTIME_SECOND * 20;
                            } break;
                        }

                        if (tmp_duration != 0)
                        {
                            m_target->m_auras[x]->SetDuration(tmp_duration);

                            sEventMgr.ModifyEventTimeLeft(m_target->m_auras[x], EVENT_AURA_REMOVE, tmp_duration);
                            m_target->ModVisualAuraStackCount(m_target->m_auras[x], 0);
                            sEventMgr.AddEvent(m_target->m_auras[x], &Aura::Remove, EVENT_AURA_REMOVE, tmp_duration, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT | EVENT_FLAG_DELETES_OBJECT);
                        }
                    }
                }
            } break;
        }
    }

    m_target->UpdateVisibility();
}

void Aura::SpellAuraModDetect(bool apply)
{
    if (apply)
    {
        //SetPositive();
        m_target->m_stealthDetectBonus += mod->m_amount;
    }
    else
        m_target->m_stealthDetectBonus -= mod->m_amount;
}

void Aura::SpellAuraModInvisibility(bool apply)
{
    SetPositive();
    if (m_spellInfo->getEffect(mod->m_effectIndex) == SPELL_EFFECT_APPLY_FRIEND_AREA_AURA)  ///\todo WTF is this crap? TODO clean this
        return;

    if (apply)
    {
        m_target->SetInvisibility(GetSpellId());
        m_target->m_invisFlag = static_cast<uint8>(mod->m_miscValue);
        if (m_target->IsPlayer())
        {
            if (GetSpellId() == 32612)
                static_cast< Player* >(m_target)->SetFlag(PLAYER_FIELD_BYTES2, 0x4000);   //Mage Invis self visual
        }

        m_target->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_INVINCIBLE);
    }
    else
    {
        m_target->m_invisFlag = INVIS_FLAG_NORMAL;
        if (m_target->IsPlayer())
        {
            if (GetSpellId() == 32612)
                static_cast< Player* >(m_target)->RemoveFlag(PLAYER_FIELD_BYTES2, 0x4000);
        }
    }

    m_target->m_invisible = apply;
    m_target->UpdateVisibility();
}

void Aura::SpellAuraModInvisibilityDetection(bool apply)
{
    //Always Positive

    ARCEMU_ASSERT(mod->m_miscValue < INVIS_FLAG_TOTAL);
    if (apply)
    {
        m_target->m_invisDetect[mod->m_miscValue] += mod->m_amount;
        SetPositive();
    }
    else
        m_target->m_invisDetect[mod->m_miscValue] -= mod->m_amount;

    if (m_target->IsPlayer())
        static_cast< Player* >(m_target)->UpdateVisibility();
}

void Aura::SpellAuraModTotalHealthRegenPct(bool apply)
{
    if (apply)
    {
        SetPositive();
        sEventMgr.AddEvent(this, &Aura::EventPeriodicHealPct, (float)mod->m_amount,
                           EVENT_AURA_PERIODIC_HEALPERC, GetSpellInfo()->getEffectAmplitude(mod->m_effectIndex), 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Aura::EventPeriodicHealPct(float RegenPct)
{
    if (!m_target->isAlive())
        return;

    uint32 add = float2int32(m_target->GetMaxHealth() * (RegenPct / 100.0f));

    uint32 newHealth = m_target->GetHealth() + add;

    if (newHealth <= m_target->GetMaxHealth())
        m_target->SetHealth(newHealth);
    else
        m_target->SetHealth(m_target->GetMaxHealth());

    m_target->SendPeriodicAuraLog(m_casterGuid, m_target->GetNewGUID(), m_spellInfo->getId(), m_spellInfo->getSchool(), add, 0, 0, FLAG_PERIODIC_HEAL, false);

    if (GetSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
    {
        m_target->Emote(EMOTE_ONESHOT_EAT);
    }

    m_target->RemoveAurasByHeal();
}

void Aura::SpellAuraModTotalManaRegenPct(bool apply)
{
    if (apply)
    {
        SetPositive();
        sEventMgr.AddEvent(this, &Aura::EventPeriodicManaPct, (float)mod->m_amount,
                           EVENT_AURA_PERIOCIC_MANA, GetSpellInfo()->getEffectAmplitude(mod->m_effectIndex), 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Aura::EventPeriodicManaPct(float RegenPct)
{
    if (!m_target->isAlive())
        return;

    uint32 add = static_cast<uint32>(m_target->GetMaxPower(POWER_TYPE_MANA) * (RegenPct / 100.0f));

    uint32 newPower = m_target->GetPower(POWER_TYPE_MANA) + add;

    if (newPower <= m_target->GetMaxPower(POWER_TYPE_MANA))
    {
        if (GetSpellInfo()->getId() != 60069)
            m_target->Energize(m_target, m_spellInfo->getId(), add, POWER_TYPE_MANA);
        else
            m_target->Energize(m_target, 49766, add, POWER_TYPE_MANA);
    }
    else
        m_target->SetPower(POWER_TYPE_MANA, m_target->GetMaxPower(POWER_TYPE_MANA));

    // CAPT
    ///\todo sniff it or disasm wow.exe to find the mana flag

    if (GetSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
    {
        m_target->Emote(EMOTE_ONESHOT_EAT);
    }
}

void Aura::EventPeriodicTriggerDummy()
{
    if (!sScriptMgr.CallScriptedDummyAura(m_spellInfo->getId(), mod->m_effectIndex, this, true))
        LOG_ERROR("Spell %u (%s) has an apply periodic trigger dummy aura effect, but no handler for it.", m_spellInfo->getId(), m_spellInfo->getName().c_str());
}

void Aura::SpellAuraModResistance(bool apply)
{
    uint32 Flag = mod->m_miscValue;
    int32 amt;
    if (apply)
    {
        amt = mod->m_amount;
        if (amt < 0)//don't change it
            SetNegative();
        else
            SetPositive();
    }
    else
        amt = -mod->m_amount;
    Unit* caster = GetUnitCaster();
    if (!IsPositive() && caster != nullptr && m_target->IsCreature())
        m_target->GetAIInterface()->AttackReaction(caster, 1, GetSpellId());

    switch (GetSpellInfo()->getId())
    {
        //SPELL_HASH_FAERIE_FIRE__FERAL_
        case 16857:
        case 60089:
        //SPELL_HASH_FAERIE_FIRE
        case 770:
        case 6950:
        case 13424:
        case 13752:
        case 16498:
        case 20656:
        case 21670:
        case 25602:
        case 32129:
        case 65863:
        {
            m_target->m_can_stealth = !apply;
        } break;
    }

    Player* plr = GetPlayerCaster();
    if (plr != nullptr)
    {
        switch (GetSpellInfo()->getId())
        {
            //SPELL_HASH_DEVOTION_AURA
            case 465:
            case 643:
            case 1032:
            case 8258:
            case 10290:
            case 10291:
            case 10292:
            case 10293:
            case 17232:
            case 27149:
            case 41452:
            case 48941:
            case 48942:
            case 52442:
            case 57740:
            case 58944:
            {
                // Increases the armor bonus of your Devotion Aura by %u - HACKY
                if (plr->HasSpell(20140))     // Improved Devotion Aura Rank 3
                    amt = (int32)(amt * 1.5);
                else if (plr->HasSpell(20139))     // Improved Devotion Aura Rank 2
                    amt = (int32)(amt * 1.34);
                else if (plr->HasSpell(20138))     // Improved Devotion Aura Rank 1
                    amt = (int32)(amt * 1.17);
            } break;
        }
    }

    if (m_target->IsPlayer())
    {
        for (uint16 x = 0; x < 7; x++)
        {
            if (Flag & (((uint32)1) << x))
            {
                if (mod->m_amount > 0)
                    static_cast< Player* >(m_target)->FlatResistanceModifierPos[x] += amt;
                else
                    static_cast< Player* >(m_target)->FlatResistanceModifierNeg[x] -= amt;
                static_cast< Player* >(m_target)->CalcResistance(x);
            }
        }
    }
    else if (m_target->IsCreature())
    {
        for (uint16 x = 0; x < 7; x++)
        {
            if (Flag & (((uint32)1) << (uint32)x))
            {
                static_cast< Creature* >(m_target)->FlatResistanceMod[x] += amt;
                static_cast< Creature* >(m_target)->CalcResistance(x);
            }
        }
    }
}

void Aura::SpellAuraPeriodicTriggerSpellWithValue(bool apply)
{
    if (apply)
    {
        SpellInfo* spe = sSpellCustomizations.GetSpellInfo(m_spellInfo->getEffectTriggerSpell(mod->m_effectIndex));
        if (spe == nullptr)
            return;

        float amptitude = static_cast<float>(GetSpellInfo()->getEffectAmplitude(mod->m_effectIndex));
        Unit* caster = GetUnitCaster();
        uint32 numticks = m_spellInfo->getSpellDuration(caster) / m_spellInfo->getEffectAmplitude(mod->m_effectIndex);
        if (caster != nullptr)
        {
            spellModFlatFloatValue(caster->SM_FAmptitude, &amptitude, m_spellInfo->getSpellGroupType());
            spellModPercentageFloatValue(caster->SM_PAmptitude, &amptitude, m_spellInfo->getSpellGroupType());
            if (m_spellInfo->getChannelInterruptFlags() != 0)
                amptitude *= caster->GetCastSpeedMod();
        }

        sEventMgr.AddEvent(this, &Aura::EventPeriodicTriggerSpell, spe, true, mod->m_amount,
                           EVENT_AURA_PERIODIC_TRIGGERSPELL, float2int32(amptitude), numticks, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Aura::SpellAuraPeriodicTriggerSpell(bool apply)
{
    switch (m_spellInfo->getId())
    {
        case 23493:
        case 24379:
        {
            Unit* caster = m_target;
            if (caster != nullptr)
            {
                sEventMgr.AddEvent(this, &Aura::EventPeriodicHealPct, 10.0f , EVENT_AURA_PERIODIC_HEALPERC, 1000, 10, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

                if (caster->GetMaxPower(POWER_TYPE_MANA))
                {
                    sEventMgr.AddEvent(this, &Aura::EventPeriodicManaPct, 10.0f, EVENT_AURA_PERIOCIC_MANA, 1000, 10, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                }
            }
            return;
        }
        case 57550: //Tirion Aggro
        {
            //Zyres: prevent server crash by invalid caster spells/amplitude
            return;
        }
    }

    if (m_spellInfo->getEffectTriggerSpell(mod->m_effectIndex) == 0)
        return;

    /*
    // This should be fixed in other way...
    if (IsPassive() &&
    m_spellProto->dummy != 2010 &&
    m_spellProto->dummy != 2020 &&
    m_spellProto->dummy != 2255 &&
    m_spellProto->getId() != 8145 &&
    m_spellProto->getId() != 8167 &&
    m_spellProto->getId() != 8172)
    {
    Unit* target = (m_target != 0) ? m_target : GetUnitCaster();
    if (target == 0 || !target->IsPlayer())
    return; //what about creatures ?

    SpellEntry *proto = sSpellCustomizations.GetSpellInfo(m_spellProto->EffectTriggerSpell[mod->i]);

    if (apply)
    TO< Player* >(target)->AddOnStrikeSpell(proto, m_spellProto->EffectAmplitude[mod->i]);
    else
    TO< Player* >(target)->RemoveOnStrikeSpell(proto);

    return;
    }
    */

    if (apply)
    {
        SpellInfo* trigger = sSpellCustomizations.GetSpellInfo(GetSpellInfo()->getEffectTriggerSpell(mod->m_effectIndex));

        if (trigger == nullptr)
            return;


        float amptitude = static_cast<float>(GetSpellInfo()->getEffectAmplitude(mod->m_effectIndex));
        Unit* caster = GetUnitCaster();
        uint32 numticks = m_spellInfo->getSpellDuration(caster) / m_spellInfo->getEffectAmplitude(mod->m_effectIndex);
        if (caster != nullptr)
        {
            spellModFlatFloatValue(caster->SM_FAmptitude, &amptitude, m_spellInfo->getSpellGroupType());
            spellModPercentageFloatValue(caster->SM_PAmptitude, &amptitude, m_spellInfo->getSpellGroupType());
            if (m_spellInfo->getChannelInterruptFlags() != 0)
                amptitude *= caster->GetCastSpeedMod();
        }

        sEventMgr.AddEvent(this, &Aura::EventPeriodicTriggerSpell, trigger, false, int32(0),
                           EVENT_AURA_PERIODIC_TRIGGERSPELL, float2int32(amptitude), numticks, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Aura::EventPeriodicTriggerSpell(SpellInfo* spellInfo, bool overridevalues, int32 overridevalue)
{
    Spell* spell = sSpellFactoryMgr.NewSpell(m_target, spellInfo, true, this);
    if (overridevalues)
    {
        spell->m_overrideBasePoints = true;
        for (uint8 i = 0; i < 3; ++i)
            spell->m_overridenBasePoints[i] = overridevalue;
    }
    SpellCastTargets spellTargets;
    spell->GenerateTargets(&spellTargets);
    spell->prepare(&spellTargets);
}

void Aura::SpellAuraPeriodicEnergize(bool apply)
{
    if (apply)
    {
        SetPositive();
        sEventMgr.AddEvent(this, &Aura::EventPeriodicEnergize, (uint32)mod->m_amount, (uint32)mod->m_miscValue,
                           EVENT_AURA_PERIODIC_ENERGIZE, GetSpellInfo()->getEffectAmplitude(mod->m_effectIndex), 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Aura::EventPeriodicEnergize(uint32 amount, uint32 type)
{
    uint32 POWER_TYPE = UNIT_FIELD_POWER1 + type;

    ARCEMU_ASSERT(POWER_TYPE <= UNIT_FIELD_POWER5);

    Unit* ucaster = GetUnitCaster();
    if (ucaster == nullptr)
        return;

    ucaster->Energize(m_target, m_spellInfo->getId(), amount, type);

    if ((GetSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP) && type == POWER_TYPE_MANA)
    {
        m_target->Emote(EMOTE_ONESHOT_EAT);
    }
}

void Aura::SpellAuraModPacify(bool apply)
{
    // Can't Attack
    if (apply)
    {
        if (m_spellInfo->getId() == 24937 || m_spellInfo->getId() == 41450) //SPELL_HASH_BLESSING_OF_PROTECTION
            SetPositive();
        else
            SetNegative();

        m_target->m_pacified++;
        m_target->addUnitStateFlag(UNIT_STATE_PACIFY);
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    }
    else
    {
        m_target->m_pacified--;

        if (m_target->m_pacified == 0)
        {
            m_target->removeUnitStateFlag(UNIT_STATE_PACIFY);
            m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
        }
    }
}

void Aura::SpellAuraModRoot(bool apply)
{
    if (apply)
    {
        // Check Mechanic Immunity
        if (m_target->MechanicsDispels[MECHANIC_ROOTED])
        {
            m_flags |= 1 << mod->m_effectIndex;
            return;
        }

        SetNegative();

        //\todo Zyres: is tis relly the way this should work?
        m_target->m_rootCounter++;

        if (m_target->m_rootCounter == 1)
            m_target->setMoveRoot(true);

        //warrior talent - second wind triggers on stun and immobilize. This is not used as proc to be triggered always !
        Unit* caster = GetUnitCaster();
        if (caster != nullptr)
        {
            caster->EventStunOrImmobilize(m_target);
            m_target->EventStunOrImmobilize(caster, true);
        }

        if (GetSpellInfo()->getSchool() == SCHOOL_FROST && !m_target->asc_frozen++)
            m_target->SetFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_FROZEN);

        ///\todo -Supalosa- TODO: Mobs will attack nearest enemy in range on aggro list when rooted. */
    }
    else if ((m_flags & (1 << mod->m_effectIndex)) == 0)   //add these checks to mods where immunity can cancel only 1 mod and not whole spell
    {
        //\todo Zyres: is tis relly the way this should work?
        m_target->m_rootCounter--;

        if (m_target->m_rootCounter == 0)
            m_target->setMoveRoot(false);

        if (m_target->IsCreature())
            m_target->GetAIInterface()->AttackReaction(GetUnitCaster(), 1, 0);

        if (GetSpellInfo()->getSchool() == SCHOOL_FROST && !--m_target->asc_frozen)
            m_target->RemoveFlag(UNIT_FIELD_AURASTATE, AURASTATE_FLAG_FROZEN);
    }
}

void Aura::SpellAuraModSilence(bool apply)
{
    if (apply)
    {
        m_target->m_silenced++;
        m_target->addUnitStateFlag(UNIT_STATE_SILENCE);
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);

        // Interrupt target's current casted spell (either channeled or generic spell with cast time)
        if (m_target->isCastingNonMeleeSpell(true, false, true))
        {
            if (m_target->getCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr && m_target->getCurrentSpell(CURRENT_CHANNELED_SPELL)->getCastTimeLeft() > 0)
            {
                m_target->interruptSpellWithSpellType(CURRENT_CHANNELED_SPELL);
            }
            // No need to check cast time for generic spells, checked already in Object::isCastingNonMeleeSpell()
            else if (m_target->getCurrentSpell(CURRENT_GENERIC_SPELL) != nullptr)
            {
                m_target->interruptSpellWithSpellType(CURRENT_GENERIC_SPELL);
            }
        }
    }
    else
    {
        m_target->m_silenced--;

        if (m_target->m_silenced == 0)
        {
            m_target->removeUnitStateFlag(UNIT_STATE_SILENCE);
            m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
        }
    }
}

void Aura::SpellAuraReflectSpells(bool apply)
{
    m_target->RemoveReflect(GetSpellId(), apply);

    if (apply)
    {
        ReflectSpellSchool* rss = new ReflectSpellSchool;
        rss->chance = mod->m_amount;
        rss->spellId = GetSpellId();
        rss->school = -1;
        rss->charges = m_spellInfo->getProcCharges();
        rss->infront = false;

        m_target->m_reflectSpellSchool.push_back(rss);
    }
}

void Aura::SpellAuraModStat(bool apply)
{
    int32 stat = (int32)mod->m_miscValue;
    int32 val;

    if (apply)
    {
        val = mod->m_amount;
        if (val < 0)
            SetNegative();
        else
            SetPositive();
    }
    else
    {
        val = -mod->m_amount;
    }

    if (stat == -1)   //all stats
    {
        if (m_target->IsPlayer())
        {
            for (uint8 x = 0; x < 5; x++)
            {
                if (mod->m_amount > 0)
                    static_cast< Player* >(m_target)->FlatStatModPos[x] += val;
                else
                    static_cast< Player* >(m_target)->FlatStatModNeg[x] -= val;

                static_cast< Player* >(m_target)->CalcStat(x);
            }

            static_cast< Player* >(m_target)->UpdateStats();
            static_cast< Player* >(m_target)->UpdateChances();
        }
        else if (m_target->IsCreature())
        {
            for (uint8 x = 0; x < 5; x++)
            {
                static_cast< Creature* >(m_target)->FlatStatMod[x] += val;
                static_cast< Creature* >(m_target)->CalcStat(x);
            }
        }
    }
    else if (stat >= 0)
    {
        ARCEMU_ASSERT(mod->m_miscValue < 5);

        uint16_t modValue = static_cast<uint16_t>(mod->m_miscValue);
        if (m_target->IsPlayer())
        {
            if (mod->m_amount > 0)
                static_cast< Player* >(m_target)->FlatStatModPos[modValue] += val;
            else
                static_cast< Player* >(m_target)->FlatStatModNeg[modValue] -= val;

            static_cast< Player* >(m_target)->CalcStat(modValue);

            static_cast< Player* >(m_target)->UpdateStats();
            static_cast< Player* >(m_target)->UpdateChances();
        }
        else if (m_target->IsCreature())
        {
            static_cast< Creature* >(m_target)->FlatStatMod[modValue] += val;
            static_cast< Creature* >(m_target)->CalcStat(modValue);
        }
    }
}

void Aura::SpellAuraModSkill(bool apply)
{
    if (m_target->IsPlayer())
    {
        if (apply)
        {
            SetPositive();
            static_cast< Player* >(m_target)->_ModifySkillBonus(mod->m_miscValue, mod->m_amount);
        }
        else
            static_cast< Player* >(m_target)->_ModifySkillBonus(mod->m_miscValue, -mod->m_amount);

        static_cast< Player* >(m_target)->UpdateStats();
    }
}

void Aura::SpellAuraModIncreaseSpeed(bool apply)
{
    if (apply)
        m_target->m_speedModifier += mod->m_amount;
    else
        m_target->m_speedModifier -= mod->m_amount;

    m_target->UpdateSpeed();
}

void Aura::SpellAuraModIncreaseMountedSpeed(bool apply)
{
    if ((GetSpellId() == 68768 || GetSpellId() == 68769) && p_target != nullptr)
    {
        int32 newspeed = 0;

        if (p_target->_GetSkillLineCurrent(SKILL_RIDING, true) >= 150)
            newspeed = 100;
        else if (p_target->_GetSkillLineCurrent(SKILL_RIDING, true) >= 75)
            newspeed = 60;

        mod->m_amount = newspeed; // EffectBasePoints + 1 (59+1 and 99+1)
    }

    if (apply)
    {
        SetPositive();
        m_target->m_mountedspeedModifier += mod->m_amount;
    }
    else
        m_target->m_mountedspeedModifier -= mod->m_amount;
    m_target->UpdateSpeed();
}

void Aura::SpellAuraModCreatureRangedAttackPower(bool apply)
{
    if (apply)
    {
        for (uint32 x = 0; x < 11; x++)
            if (mod->m_miscValue & (((uint32)1) << x))
                m_target->CreatureRangedAttackPowerMod[x + 1] += mod->m_amount;
        if (mod->m_amount < 0)
            SetNegative();
        else
            SetPositive();
    }
    else
    {
        for (uint32 x = 0; x < 11; x++)
        {
            if (mod->m_miscValue & (((uint32)1) << x))
            {
                m_target->CreatureRangedAttackPowerMod[x + 1] -= mod->m_amount;
            }
        }
    }
    m_target->CalcDamage();
}

void Aura::SpellAuraModDecreaseSpeed(bool apply)
{
    //there can not be 2 slow downs only most powerful is applied
    if (apply)
    {
        // Check Mechanic Immunity
        if (m_target->MechanicsDispels[MECHANIC_ENSNARED])
        {
            m_flags |= 1 << mod->m_effectIndex;
            return;
        }
        switch (m_spellInfo->getId())
        {
            // SPELL_HASH_STEALTH
            case 1784:
            case 1785:
            case 1786:
            case 1787:
            case 8822:
            case 30831:
            case 30991:
            case 31526:
            case 31621:
            case 32199:
            case 32615:
            case 34189:
            case 42347:
            case 42866:
            case 42943:
            case 52188:
            case 58506:
                SetPositive();
                break;

            // SPELL_HASH_DAZED
            case 1604:
            case 5101:
            case 13496:
            case 15571:
            case 29703:
            case 35955:
            case 50259:
            case 50411:
            case 51372:
                SetNegative();
                break;

            default:
                /* burlex: this would be better as a if (caster is hostile to target) then effect = negative) */
                if (m_casterGuid != m_target->GetGUID())
                    SetNegative();
                break;
        }

        //let's check Mage talents if we proc anything
        if (m_spellInfo->getSchool() == SCHOOL_FROST)
        {
            //yes we are freezing the bastard, so can we proc anything on this ?
            Unit* caster = GetUnitCaster();
            if (caster != nullptr && caster->IsPlayer())
                static_cast< Unit* >(caster)->EventChill(m_target);
            if (m_target->IsPlayer() && caster)
                static_cast< Unit* >(m_target)->EventChill(caster, true);
        }
        m_target->speedReductionMap.insert(std::make_pair(m_spellInfo->getId(), mod->m_amount));
        //m_target->m_slowdown=this;
        //m_target->m_speedModifier += mod->m_amount;
    }
    else if ((m_flags & (1 << mod->m_effectIndex)) == 0)   //add these checks to mods where immunity can cancel only 1 mod and not whole spell
    {
        std::map< uint32, int32 >::iterator itr = m_target->speedReductionMap.find(m_spellInfo->getId());
        if (itr != m_target->speedReductionMap.end())
            m_target->speedReductionMap.erase(itr);
        //m_target->m_speedModifier -= mod->m_amount;
        //m_target->m_slowdown= NULL;
    }
    if (m_target->GetSpeedDecrease())
        m_target->UpdateSpeed();
}

void Aura::UpdateAuraModDecreaseSpeed()
{
    if (m_target->MechanicsDispels[MECHANIC_ENSNARED])
    {
        m_flags |= 1 << mod->m_effectIndex;
        return;
    }

    //let's check Mage talents if we proc anything
    if (m_spellInfo->getSchool() == SCHOOL_FROST)
    {
        //yes we are freezing the bastard, so can we proc anything on this ?
        Unit* caster = GetUnitCaster();
        if (caster && caster->IsPlayer())
            static_cast< Unit* >(caster)->EventChill(m_target);
        if (m_target->IsPlayer() && caster)
            static_cast< Unit* >(m_target)->EventChill(caster, true);
    }
}

void Aura::SpellAuraModIncreaseHealth(bool apply)
{
    int32 amt;

    if (apply)
    {
        //threat special cases. We should move these to scripted spells maybe
        switch (m_spellInfo->getId())
        {
            case 23782:// Gift of Life
                mod->m_amount = 1500;
                break;
            case 12976:// Last Stand
                mod->m_amount = (uint32)(m_target->GetMaxHealth() * 0.3);
                break;
        }
        SetPositive();
        amt = mod->m_amount;
    }
    else
        amt = -mod->m_amount;

    if (m_target->IsPlayer())
    {
        //maybe we should not adjust hitpoints too but only maximum health
        static_cast< Player* >(m_target)->SetHealthFromSpell(static_cast< Player* >(m_target)->GetHealthFromSpell() + amt);
        static_cast< Player* >(m_target)->UpdateStats();
        if (apply)
            m_target->ModHealth(amt);
        else
        {
            if ((int32)m_target->GetHealth() > -amt) //watch it on remove value is negative
                m_target->ModHealth(amt);
            else m_target->SetHealth(1); //do not kill player but do strip him good
        }
    }
    else
        m_target->ModMaxHealth(amt);
}

void Aura::SpellAuraModIncreaseEnergy(bool apply)
{
    SetPositive();
    //uint32 powerField,maxField ;
    //uint8 powerType = m_target->GetPowerType();

    /*if (powerType == POWER_TYPE_MANA) // Mana
    {
    powerField = UNIT_FIELD_POWER1;
    maxField = UNIT_FIELD_MAXPOWER1;
    }
    else if (powerType == POWER_TYPE_RAGE) // Rage
    {
    powerField = UNIT_FIELD_POWER2;
    maxField = UNIT_FIELD_MAXPOWER2;
    }
    else if (powerType == POWER_TYPE_ENERGY) // Energy
    {
    powerField = UNIT_FIELD_POWER4;
    maxField = UNIT_FIELD_MAXPOWER4;
    }
    else // Capt: if we can not use identify the type: do nothing
    return; */

    int32 amount = apply ? mod->m_amount : -mod->m_amount;
    uint16_t modValue = static_cast<uint16_t>(mod->m_miscValue);
    m_target->ModMaxPower(modValue, amount);
    m_target->ModPower(modValue, amount);

    if (modValue == 0 && m_target->IsPlayer())
    {
        static_cast< Player* >(m_target)->SetManaFromSpell(static_cast< Player* >(m_target)->GetManaFromSpell() + amount);
    }
}

void Aura::SpellAuraModShapeshift(bool apply)
{
    if (p_target != nullptr)
    {
        if (p_target->m_MountSpellId != 0 && p_target->m_MountSpellId != m_spellInfo->getId())
        {
            switch (mod->m_miscValue)
            {
                case FORM_BATTLESTANCE:
                case FORM_DEFENSIVESTANCE:
                case FORM_BERSERKERSTANCE:
                case FORM_UNDEAD:
                    break;
                default:
                    p_target->Dismount();
            }
        }
    }

    auto shapeshift_form = sSpellShapeshiftFormStore.LookupEntry(mod->m_miscValue);
    if (!shapeshift_form)
        return;

    uint32 spellId = 0;
    // uint32 spellId2 = 0;
    uint32 modelId = (uint32)(apply ? shapeshift_form->modelId : 0);

    bool freeMovements = false;

    switch (shapeshift_form->id)
    {
        case FORM_CAT:
        {
            //druid
            freeMovements = true;
            spellId = 3025;
            if (apply)
            {
                m_target->SetPowerType(POWER_TYPE_ENERGY);
                m_target->SetMaxPower(POWER_TYPE_ENERGY, 100);  //100 Energy
                m_target->SetPower(POWER_TYPE_ENERGY, 0);  //0 Energy
                if (m_target->getRace() != RACE_NIGHTELF)//TAUREN
                    modelId = 8571;

            }
            else
            {
                //turn back to mana
                //m_target->SetBaseAttackTime(MELEE,oldap);
                m_target->SetPowerType(POWER_TYPE_MANA);
                if (m_target->m_stealth)
                {
                    uint32 sp = m_target->m_stealth;
                    m_target->m_stealth = 0;
                    m_target->RemoveAura(sp);//prowl
                }
            }
        }
        break;
        case FORM_TREE:
        {
            freeMovements = true;
            spellId = 34123; // this is area aura
            //spellId2 = 5420;
        }
        break;
        case FORM_TRAVEL:
        {
            //druid
            freeMovements = true;
            spellId = 5419;
        }
        break;
        case FORM_AQUA:
        {
            //druid aqua
            freeMovements = true;
            spellId = 5421;
        }
        break;
        case FORM_BEAR:
        {
            //druid only
            freeMovements = true;
            spellId = 1178;
            if (apply)
            {
                m_target->SetPowerType(POWER_TYPE_RAGE);
                m_target->SetMaxPower(POWER_TYPE_RAGE, 1000);
                m_target->SetPower(POWER_TYPE_RAGE, 0); //0 rage

                if (m_target->getRace() != RACE_NIGHTELF)   //TAUREN
                    modelId = 2289;

                //some say there is a second effect
                SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(21178);

                Spell* sp = sSpellFactoryMgr.NewSpell(m_target, spellInfo, true, nullptr);
                SpellCastTargets tgt;
                tgt.m_unitTarget = m_target->GetGUID();
                sp->prepare(&tgt);
            }
            else
            {
                //reset back to mana
                m_target->SetPowerType(POWER_TYPE_MANA);
                m_target->RemoveAura(21178);   // remove Bear Form (Passive2)
            }
        }
        break;
        case FORM_DIREBEAR:
        {
            //druid only
            freeMovements = true;
            spellId = 9635;
            if (apply)
            {
                m_target->SetPowerType(POWER_TYPE_RAGE);
                m_target->SetMaxPower(POWER_TYPE_RAGE, 1000);
                m_target->SetPower(POWER_TYPE_RAGE, 0); //0 rage
                if (m_target->getRace() != RACE_NIGHTELF)   //TAUREN
                    modelId = 2289;
            }
            else //reset back to mana
                m_target->SetPowerType(POWER_TYPE_MANA);
        }
        break;
        case FORM_BATTLESTANCE:
        {
            spellId = 21156;
        }
        break;
        case FORM_DEFENSIVESTANCE:
        {
            spellId = 7376;
        }
        break;
        case FORM_BERSERKERSTANCE:
        {
            spellId = 7381;
        }
        break;
        case FORM_SHADOW:
        {
            if (apply)
            {
                static_cast< Player* >(m_target)->SendSpellCooldownEvent(m_spellInfo->getId());
            }
            spellId = 49868;
        }
        break;
        case FORM_FLIGHT:
        {
            // druid
            freeMovements = true;
            spellId = 33948;
            if (apply)
            {
                if (m_target->getRace() != RACE_NIGHTELF)
                    modelId = 20872;
            }
        }
        break;
        case FORM_STEALTH:
        {
            // rogue
            if (!m_target->m_can_stealth)
                return;
            //m_target->UpdateVisibility();
        }
        break;
        case FORM_MOONKIN:
        {
            //druid
            freeMovements = true;
            spellId = 24905;
            if (apply)
            {
                if (m_target->getRace() != RACE_NIGHTELF)
                    modelId = shapeshift_form->modelId2; // Lol, why is this the only one that has it in ShapeShift DBC? =/ lameeee...
            }
        }
        break;
        case FORM_SWIFT: //not tested yet, right now going on trust
        {
            // druid
            freeMovements = true;
            spellId = 40121; //Swift Form Passive
            if (apply)
            {
                if (m_target->getRace() != RACE_NIGHTELF)//TAUREN
                    modelId = 21244;
            }
        }
        break;
        case FORM_SPIRITOFREDEMPTION:
        {
            spellId = 27795;
            modelId = 12824; // Smaller spirit healer, heehee :3
        }
        break;
        case FORM_GHOUL:
        case FORM_SKELETON:
        case FORM_ZOMBIE:
        {
            if (p_target != nullptr)
                p_target->SendAvailSpells(shapeshift_form, apply);
        }
        break;
        case FORM_METAMORPHOSIS:
        {
            spellId = 59673;
        }
        break;
    }

    if (apply)
    {
        if (p_target != nullptr)
        {
            if (p_target->getClass() == WARRIOR && p_target->GetPower(POWER_TYPE_RAGE) > p_target->m_retainedrage)
                p_target->SetPower(POWER_TYPE_RAGE, p_target->m_retainedrage);

            if (m_target->getClass() == DRUID)
            {
                if (Rand(p_target->m_furorChance))
                {
                    uint32 furorSpell;
                    if (mod->m_miscValue == FORM_CAT)
                        furorSpell = 17099;
                    else if (mod->m_miscValue == FORM_BEAR || mod->m_miscValue == FORM_DIREBEAR)
                        furorSpell = 17057;
                    else
                        furorSpell = 0;

                    if (furorSpell != 0)
                    {
                        SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(furorSpell);

                        Spell* sp = sSpellFactoryMgr.NewSpell(m_target, spellInfo, true, nullptr);
                        SpellCastTargets tgt;
                        tgt.m_unitTarget = m_target->GetGUID();
                        sp->prepare(&tgt);
                    }
                }
            }

            if (spellId != GetSpellId())
            {
                if (p_target->m_ShapeShifted)
                    p_target->RemoveAura(p_target->m_ShapeShifted);

                p_target->m_ShapeShifted = GetSpellId();
            }
        }

        if (modelId != 0)
        {
            m_target->SetDisplayId(modelId);
            m_target->EventModelChange();
        }

        m_target->SetShapeShift(static_cast<uint8_t>(mod->m_miscValue));

        // check for spell id
        if (spellId == 0)
            return;

        SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);

        Spell* sp = sSpellFactoryMgr.NewSpell(m_target, spellInfo, true, nullptr);
        SpellCastTargets tgt;
        tgt.m_unitTarget = m_target->GetGUID();
        sp->prepare(&tgt);

        /*if (spellId2 != 0) This cannot be true CID 52824
        {
            spellInfo = sSpellCustomizations.GetSpellInfo(spellId2);
            sp = sSpellFactoryMgr.NewSpell(m_target, spellInfo, true, NULL);
            sp->prepare(&tgt);
        }*/

        // remove the caster from impairing movements
        if (freeMovements)
        {
            for (uint32 x = MAX_POSITIVE_AURAS_EXTEDED_START; x < MAX_POSITIVE_AURAS_EXTEDED_END; x++)
            {
                if (m_target->m_auras[x] != nullptr)
                {
                    if (m_target->m_auras[x]->GetSpellInfo()->getMechanicsType() == MECHANIC_ROOTED || m_target->m_auras[x]->GetSpellInfo()->getMechanicsType() == MECHANIC_ENSNARED)   // Remove roots and slow spells
                    {
                        m_target->m_auras[x]->Remove();
                    }
                    else // if got immunity for slow, remove some that are not in the mechanics
                    {
                        for (uint8 i = 0; i < 3; i++)
                        {
                            if (m_target->m_auras[x]->GetSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_MOD_DECREASE_SPEED || m_target->m_auras[x]->GetSpellInfo()->getEffectApplyAuraName(i) == SPELL_AURA_MOD_ROOT)
                            {
                                m_target->m_auras[x]->Remove();
                                break;
                            }
                        }
                    }
                }
            }
        }

        //execute after we changed shape
        if (p_target != nullptr)
            p_target->EventTalentHearthOfWildChange(true);
    }
    else
    {
        if (shapeshift_form->id != FORM_STEALTH)
            m_target->RemoveAllAurasByRequiredShapeShift(ascemu::World::Spell::Helpers::decimalToMask(mod->m_miscValue));

        if (m_target->isCastingNonMeleeSpell())
        {
            for (uint8_t i = 0; i < CURRENT_SPELL_MAX; ++i)
            {
                Spell* curSpell = m_target->getCurrentSpell(CurrentSpellType(i));
                if (curSpell != nullptr && (curSpell->GetSpellInfo()->getRequiredShapeShift() & decimalToMask(mod->m_miscValue)))
                    m_target->interruptSpellWithSpellType(CurrentSpellType(i));
            }
        }

        //execute before changing shape back
        if (p_target != nullptr)
        {
            p_target->EventTalentHearthOfWildChange(false);
            p_target->m_ShapeShifted = 0;
        }
        m_target->SetDisplayId(m_target->GetNativeDisplayId());
        m_target->EventModelChange();
        if (spellId != GetSpellId())
        {
            if (spellId)
                m_target->RemoveAura(spellId);
        }

        m_target->SetShapeShift(0);
    }

    if (p_target != nullptr)
    {
        p_target->UpdateStats();
        p_target->UpdateAttackSpeed();
    }
}

void Aura::SpellAuraModEffectImmunity(bool apply)
{
    if (m_spellInfo->getId() == 24937)
        SetPositive();

    if (!apply)
    {
        if (m_spellInfo->getId() == 23333 || m_spellInfo->getId() == 23335 || m_spellInfo->getId() == 34976)
        {
            Player* plr = GetPlayerCaster();
            if (plr == nullptr || plr->m_bg == nullptr)
                return;

            plr->m_bg->HookOnFlagDrop(plr);

        }
    }
}

void Aura::SpellAuraModStateImmunity(bool /*apply*/)
{
    //%50 chance to dispel 1 magic effect on target
    //23922
}

void Aura::SpellAuraModSchoolImmunity(bool apply)
{
    switch (m_spellInfo->getId())
    {
        //SPELL_HASH_DIVINE_SHIELD
        case 642:
        case 13874:
        case 29382:
        case 33581:
        case 40733:
        case 41367:
        case 54322:
        case 63148:
        case 66010:
        case 67251:
        case 71550:
        //SPELL_HASH_ICE_BLOCK
        case 27619:
        case 36911:
        case 41590:
        case 45438:
        case 45776:
        case 46604:
        case 46882:
        case 56124:
        case 56644:
        case 62766:
        case 65802:
        case 69924:
        {
            if (apply)
            {
                if (!m_target->isAlive())
                    return;

                Aura* pAura;
                for (uint32 i = MAX_NEGATIVE_AURAS_EXTEDED_START; i < MAX_NEGATIVE_AURAS_EXTEDED_END; ++i)
                {
                    pAura = m_target->m_auras[i];
                    if (pAura != this &&
                        pAura != nullptr &&
                        !pAura->IsPassive() &&
                        !pAura->IsPositive() &&
                        !(pAura->GetSpellInfo()->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY))
                    {
                        pAura->Remove();
                    }
                }
            }
        } break;
    }

    switch (m_spellInfo->getId())
    {
        //SPELL_HASH_DIVINE_SHIELD
        case 642:
        case 13874:
        case 29382:
        case 33581:
        case 40733:
        case 41367:
        case 54322:
        case 63148:
        case 66010:
        case 67251:
        case 71550:
        //SPELL_HASH_BLESSING_OF_PROTECTION
        case 41450:
        //SPELL_HASH_ICE_BLOCK
        case 27619:
        case 36911:
        case 41590:
        case 45438:
        case 45776:
        case 46604:
        case 46882:
        case 56124:
        case 56644:
        case 62766:
        case 65802:
        case 69924:
        {
            if (apply)
                m_target->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_INVINCIBLE);
        } break;
    }

    if (apply)
    {
        //fix me may be negative
        Unit* c = GetUnitCaster();
        if (c)
        {
            if (isAttackable(c, m_target))
                SetNegative();
            else SetPositive();
        }
        else
            SetPositive();

        LogDebugFlag(LF_AURA, "SpellAuraModSchoolImmunity called with misValue = %x", mod->m_miscValue);
        for (uint8 i = 0; i < SCHOOL_COUNT; i++)
        {
            if (mod->m_miscValue & (1 << i))
            {
                m_target->SchoolImmunityList[i]++;
                m_target->RemoveAurasOfSchool(i, false, true);
            }
        }
    }
    else
    {
        for (uint8 i = 0; i < SCHOOL_COUNT; i++)
        {
            if (mod->m_miscValue & (1 << i) &&
                m_target->SchoolImmunityList[i] > 0)
            {
                m_target->SchoolImmunityList[i]--;
            }
        }
    }
}

void Aura::SpellAuraModDmgImmunity(bool /*apply*/)
{

}

void Aura::SpellAuraModDispelImmunity(bool apply)
{
    ARCEMU_ASSERT(mod->m_miscValue < 10);
    if (apply)
        m_target->dispels[mod->m_miscValue]++;
    else
        m_target->dispels[mod->m_miscValue]--;

    if (apply)
    {
        for (uint32 x = MAX_POSITIVE_AURAS_EXTEDED_START; x < MAX_POSITIVE_AURAS_EXTEDED_END; x++)
        {
            // HACK FIX FOR: 41425 and 25771
            if (m_target->m_auras[x] && m_target->m_auras[x]->GetSpellId() != 41425 && m_target->m_auras[x]->GetSpellId() != 25771)
                if (m_target->m_auras[x]->GetSpellInfo()->getDispelType() == (uint32)mod->m_miscValue)
                    m_target->m_auras[x]->Remove();
        }
    }
}

void Aura::SpellAuraProcTriggerSpell(bool apply)
{
    if (apply)
    {
        uint32 groupRelation[3];
        int charges;
        uint32 spellId;

        // Find spell of effect to be triggered
        spellId = GetSpellInfo()->getEffectTriggerSpell(mod->m_effectIndex);
        if (spellId == 0)
        {
            LogDebugFlag(LF_AURA, "Warning! trigger spell is null for spell %u", GetSpellInfo()->getId());
            return;
        }

#if VERSION_STRING != Cata
        // Initialize mask
        groupRelation[0] = GetSpellInfo()->getEffectSpellClassMask(mod->m_effectIndex, 0);
        groupRelation[1] = GetSpellInfo()->getEffectSpellClassMask(mod->m_effectIndex, 1);
        groupRelation[2] = GetSpellInfo()->getEffectSpellClassMask(mod->m_effectIndex, 2);
#else
        groupRelation[0] = GetSpellInfo()->EffectSpellClassMask[0];
        groupRelation[1] = GetSpellInfo()->EffectSpellClassMask[1];
        groupRelation[2] = GetSpellInfo()->EffectSpellClassMask[2];
#endif

        // Initialize charges
        charges = GetSpellInfo()->getProcCharges();
        Unit* ucaster = GetUnitCaster();
        if (ucaster != nullptr)
        {
            spellModFlatIntValue(ucaster->SM_FCharges, &charges, GetSpellInfo()->getSpellGroupType());
            spellModPercentageIntValue(ucaster->SM_PCharges, &charges, GetSpellInfo()->getSpellGroupType());
        }

        m_target->AddProcTriggerSpell(spellId, GetSpellInfo()->getId(), m_casterGuid, GetSpellInfo()->getProcChance(), GetSpellInfo()->getProcFlags(), charges, groupRelation, nullptr);

        LogDebugFlag(LF_AURA, "%u is registering %u chance %u flags %u charges %u triggeronself %u interval %u", GetSpellInfo()->getId(), spellId, GetSpellInfo()->getProcChance(), GetSpellInfo()->getProcFlags() & ~PROC_TARGET_SELF, charges, GetSpellInfo()->getProcFlags() & PROC_TARGET_SELF, GetSpellInfo()->custom_proc_interval);
    }
    else
    {
        // Find spell of effect to be triggered
        uint32 spellId = GetSpellInfo()->getEffectTriggerSpell(mod->m_effectIndex);
        if (spellId == 0)
        {
            LogDebugFlag(LF_AURA, "Warning! trigger spell is null for spell %u", GetSpellInfo()->getId());
            return;
        }

        m_target->RemoveProcTriggerSpell(spellId, m_casterGuid);
    }
}

void Aura::SpellAuraProcTriggerDamage(bool apply)
{
    if (apply)
    {
        DamageProc ds;
        ds.m_damage = mod->m_amount;
        ds.m_spellId = GetSpellInfo()->getId();
        ds.m_school = GetSpellInfo()->getSchool();
        ds.m_flags = m_spellInfo->getProcFlags();
        ds.owner = (void*)this;
        m_target->m_damageShields.push_back(ds);
        LogDebugFlag(LF_AURA, "registering dmg proc %u, school %u, flags %u, charges at least %u", ds.m_spellId, ds.m_school, ds.m_flags, m_spellInfo->getProcCharges());
    }
    else
    {
        for (std::list<struct DamageProc>::iterator i = m_target->m_damageShields.begin(); i != m_target->m_damageShields.end(); ++i)
        {
            if (i->owner == this)
            {
                m_target->m_damageShields.erase(i);
                break;
            }
        }
    }
}

void Aura::SpellAuraTrackCreatures(bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            if (p_target->TrackingSpell != 0)
                p_target->RemoveAura(p_target->TrackingSpell);

            p_target->setUInt32Value(PLAYER_TRACK_CREATURES, (uint32)1 << (mod->m_miscValue - 1));
            p_target->TrackingSpell = GetSpellId();
        }
        else
        {
            p_target->TrackingSpell = 0;
            p_target->setUInt32Value(PLAYER_TRACK_CREATURES, 0);
        }
    }
}

void Aura::SpellAuraTrackResources(bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            if (p_target->TrackingSpell != 0)
                p_target->RemoveAura(p_target->TrackingSpell);

            p_target->setUInt32Value(PLAYER_TRACK_RESOURCES, (uint32)1 << (mod->m_miscValue - 1));
            p_target->TrackingSpell = GetSpellId();
        }
        else
        {
            p_target->TrackingSpell = 0;
            p_target->setUInt32Value(PLAYER_TRACK_RESOURCES, 0);
        }
    }
}

void Aura::SpellAuraModParryPerc(bool apply)
{
    //if (m_target->GetTypeId() == TYPEID_PLAYER)
    {
        int32 amt;
        if (apply)
        {
            amt = mod->m_amount;
            if (amt < 0)
                SetNegative();
            else
                SetPositive();

        }
        else
            amt = -mod->m_amount;

        m_target->SetParryFromSpell(m_target->GetParryFromSpell() + amt);
        if (p_target != nullptr)
        {
            p_target->UpdateChances();
        }
    }
}

void Aura::SpellAuraModDodgePerc(bool apply)
{
    //if (m_target->GetTypeId() == TYPEID_PLAYER)
    {
        int32 amt = mod->m_amount;
        //		spellModFlatIntValue(m_target->SM_FSPELL_VALUE, &amt, GetSpellProto()->SpellGroupType);
        if (apply)
        {
            if (amt < 0)
                SetNegative();
            else
                SetPositive();
        }
        else
            amt = -amt;

        m_target->SetDodgeFromSpell(m_target->GetDodgeFromSpell() + amt);
        if (p_target != nullptr)
        {
            p_target->UpdateChances();
        }
    }
}

void Aura::SpellAuraModBlockPerc(bool apply)
{
    //if (m_target->GetTypeId() == TYPEID_PLAYER)
    {
        int32 amt;
        if (apply)
        {
            amt = mod->m_amount;
            if (amt < 0)
                SetNegative();
            else
                SetPositive();
        }
        else
            amt = -mod->m_amount;

        m_target->SetBlockFromSpell(m_target->GetBlockFromSpell() + amt);
        if (p_target != nullptr)
        {
            p_target->UpdateStats();
        }
    }
}

void Aura::SpellAuraModCritPerc(bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            WeaponModifier md;
            md.value = float(mod->m_amount);
            md.wclass = GetSpellInfo()->getEquippedItemClass();
            md.subclass = GetSpellInfo()->getEquippedItemSubClass();
            p_target->tocritchance.insert(std::make_pair(GetSpellId(), md));
        }
        else
        {
            /*std::list<WeaponModifier>::iterator i = TO< Player* >(m_target)->tocritchance.begin();

            for (;i!=TO< Player* >(m_target)->tocritchance.end();i++)
            {
            if ((*i).spellid==GetSpellId())
            {
            TO< Player* >(m_target)->tocritchance.erase(i);
            break;
            }
            }*/
            p_target->tocritchance.erase(GetSpellId());
        }
        p_target->UpdateChances();
    }
}

void Aura::SpellAuraPeriodicLeech(bool apply)
{
    if (apply)
    {
        SetNegative();
        uint32 amt = mod->m_amount;
        sEventMgr.AddEvent(this, &Aura::EventPeriodicLeech, amt,
                           EVENT_AURA_PERIODIC_LEECH, GetSpellInfo()->getEffectAmplitude(mod->m_effectIndex), 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Aura::EventPeriodicLeech(uint32 amount)
{
    Unit* m_caster = GetUnitCaster();

    if (m_caster == nullptr)
        return;

    if (!(m_target->isAlive() && m_caster->isAlive()))
        return;

    SpellInfo* sp = GetSpellInfo();

    if (m_target->SchoolImmunityList[sp->getSchool()])
    {
        SendTickImmune(m_target, m_caster);
        return;
    }

    bool is_critical = false;
    uint32 aproc = PROC_ON_ANY_HOSTILE_ACTION;
    uint32 vproc = PROC_ON_ANY_HOSTILE_ACTION | PROC_ON_ANY_DAMAGE_VICTIM | PROC_ON_SPELL_HIT_VICTIM;

    int amp = sp->getEffectAmplitude(mod->m_effectIndex);
    if (!amp)
        amp = event_GetEventPeriod(EVENT_AURA_PERIODIC_LEECH);

    int32 bonus = 0;

    if (GetDuration())
    {
        float fbonus = m_caster->GetSpellDmgBonus(m_target, sp, amount, true) * 0.5f;
        if (fbonus < 0)
            fbonus = 0.0f;
        bonus = float2int32(fbonus * amp / GetDuration());
    }

    amount += bonus;

    spellModFlatIntValue(m_caster->SM_FDOT, (int32*)&amount, sp->getSpellGroupType());
    spellModPercentageIntValue(m_caster->SM_PDOT, (int32*)&amount, sp->getSpellGroupType());


    if (DotCanCrit())
    {
        is_critical = m_caster->IsCriticalDamageForSpell(m_target, sp);

        if (is_critical)
        {
            amount = float2int32(m_caster->GetCriticalDamageBonusForSpell(m_target, sp, (float)amount));

            aproc |= PROC_ON_SPELL_CRIT_HIT;
            vproc |= PROC_ON_SPELL_CRIT_HIT_VICTIM;
        }
    }

    amount = (uint32)std::min(amount, m_target->getUInt32Value(UNIT_FIELD_HEALTH));

    // Apply bonus from [Warlock] Soul Siphon
    if (m_caster->m_soulSiphon.amt)
    {
        // Use std::map to prevent counting duplicate auras (stacked ones, from the same unit)
        std::map<uint64, std::set<uint32> *> auras;
        std::map<uint64, std::set<uint32> *>::iterator itx, itx2;
        int32 pct;
        int32 count = 0;

        auras.clear();
        for (uint32 x = MAX_NEGATIVE_AURAS_EXTEDED_START; x < MAX_NEGATIVE_AURAS_EXTEDED_END; x++)
        {
            if (m_target->m_auras[x] == nullptr)
                continue;

            Aura* aura = m_target->m_auras[x];
            if (aura->GetSpellInfo()->getSpellFamilyName() != 5)
                continue;

            auto skill_line_ability = objmgr.GetSpellSkill(aura->GetSpellId());
            if (skill_line_ability == nullptr || skill_line_ability->skilline != SKILL_AFFLICTION)
                continue;

            itx = auras.find(aura->GetCasterGUID());
            if (itx == auras.end())
            {
                std::set<uint32> *ids = new std::set<uint32>;
                auras.insert(make_pair(aura->GetCasterGUID(), ids));
                itx = auras.find(aura->GetCasterGUID());
            }

            std::set<uint32> *ids = itx->second;
            if (ids->find(aura->GetSpellId()) == ids->end())
            {
                ids->insert(aura->GetSpellId());
            }
        }

        if (auras.size())
        {
            itx = auras.begin();
            while (itx != auras.end())
            {
                itx2 = itx++;
                count += (int32)itx2->second->size();
                delete itx2->second;
            }
        }

        pct = count * m_caster->m_soulSiphon.amt;
        if (pct > m_caster->m_soulSiphon.max)
            pct = m_caster->m_soulSiphon.max;
        amount += amount * pct / 100;
    }

    uint32 dmg_amount = amount;
    uint32 heal_amount = float2int32(amount * sp->getEffectMultipleValue(mod->m_effectIndex));

    uint32 newHealth = m_caster->GetHealth() + heal_amount;

    uint32 mh = m_caster->GetMaxHealth();
    if (newHealth <= mh)
        m_caster->SetHealth(newHealth);
    else
        m_caster->SetHealth(mh);

    m_target->SendPeriodicHealAuraLog(m_caster->GetNewGUID(), m_caster->GetNewGUID(), sp->getId(), heal_amount, 0, false);
    m_target->SendPeriodicAuraLog(m_target->GetNewGUID(), m_target->GetNewGUID(), sp->getId(), sp->getSchool(), heal_amount, 0, 0, FLAG_PERIODIC_LEECH, is_critical);

    //deal damage before we add healing bonus to damage
    m_caster->DealDamage(m_target, dmg_amount, 0, 0, sp->getId(), true);
    m_caster->SendSpellNonMeleeDamageLog(m_caster, m_target, sp->getId(), dmg_amount, (uint8)sp->getSchool(), 0, 0, true, 0, is_critical, true);

    m_caster->HandleProc(aproc, m_target, sp, false, dmg_amount);
    m_caster->m_procCounter = 0;

    //some say this prevents some crashes atm
    if (!m_target->isAlive())
        return;

    m_target->HandleProc(vproc, m_caster, sp, false, dmg_amount);
    m_target->m_procCounter = 0;

    m_target->RemoveAurasByHeal();
}

void Aura::SendTickImmune(Unit* target, Unit* caster)
{
    WorldPacket data(SMSG_SPELLORDAMAGE_IMMUNE, 21);
    data << (caster ? caster->GetGUID() : target->GetGUID());
    data << target->GetGUID();
    data << GetSpellInfo()->getId();
    data << uint8(1);
    target->SendMessageToSet(&data, true);
}

void Aura::SpellAuraModHitChance(bool apply)
{
    if (!m_target->IsUnit()) return;

    int32 val = mod->m_amount;

    Unit* c = GetUnitCaster();
    if (c != nullptr)
    {
        spellModFlatIntValue(c->SM_FMiscEffect, &val, GetSpellInfo()->getSpellGroupType());
        spellModPercentageIntValue(c->SM_PMiscEffect, &val, GetSpellInfo()->getSpellGroupType());
    }

    if (apply)
    {
        static_cast< Unit* >(m_target)->SetHitFromMeleeSpell(static_cast< Unit* >(m_target)->GetHitFromMeleeSpell() + val);
        if (val < 0)
            SetNegative();
        else
            SetPositive();
    }
    else
    {
        static_cast< Unit* >(m_target)->SetHitFromMeleeSpell(static_cast< Unit* >(m_target)->GetHitFromMeleeSpell() - val);
        if (static_cast< Unit* >(m_target)->GetHitFromMeleeSpell() < 0)
        {
            static_cast< Unit* >(m_target)->SetHitFromMeleeSpell(0);
        }
    }
}

void Aura::SpellAuraModSpellHitChance(bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            p_target->SetHitFromSpell(p_target->GetHitFromSpell() + mod->m_amount);
            if (mod->m_amount < 0)
                SetNegative();
            else
                SetPositive();
        }
        else
        {
            p_target->SetHitFromSpell(p_target->GetHitFromSpell() - mod->m_amount);
            if (p_target->GetHitFromSpell() < 0)
            {
                p_target->SetHitFromSpell(0);
            }
        }
    }
}

void Aura::SpellAuraTransform(bool apply)
{
    // Try a dummy SpellHandler
    if (sScriptMgr.CallScriptedDummyAura(GetSpellId(), mod->m_effectIndex, this, apply))
        return;

    uint32 displayId = 0;
    CreatureProperties const* ci = sMySQLStore.getCreatureProperties(mod->m_miscValue);

    if (ci)
        displayId = ci->Male_DisplayID;

    if (p_target != nullptr)
        p_target->Dismount();

    // SetPositive();
    switch (GetSpellInfo()->getId())
    {
        case 20584://wisp
            m_target->SetDisplayId(apply ? 10045 : m_target->GetNativeDisplayId());
            break;

        case 30167: // Red Ogre Costume
        {
            if (apply)
                m_target->SetDisplayId(11549);
            else
                m_target->SetDisplayId(m_target->GetNativeDisplayId());
        }
        break;

        case 41301: // Time-Lost Figurine
        {
            if (apply)
                m_target->SetDisplayId(18628);
            else
                m_target->SetDisplayId(m_target->GetNativeDisplayId());
        }
        break;

        case 16739: // Orb of Deception
        {
            if (apply)
            {
                if (m_target->getRace() == RACE_ORC)
                {
                    if (m_target->getGender() == 0)
                        m_target->SetDisplayId(10139);
                    else
                        m_target->SetDisplayId(10140);
                }
                if (m_target->getRace() == RACE_TAUREN)
                {
                    if (m_target->getGender() == 0)
                        m_target->SetDisplayId(10136);
                    else
                        m_target->SetDisplayId(10147);
                }
                if (m_target->getRace() == RACE_TROLL)
                {
                    if (m_target->getGender() == 0)
                        m_target->SetDisplayId(10135);
                    else
                        m_target->SetDisplayId(10134);
                }
                if (m_target->getRace() == RACE_UNDEAD)
                {
                    if (m_target->getGender() == 0)
                        m_target->SetDisplayId(10146);
                    else
                        m_target->SetDisplayId(10145);
                }
                if (m_target->getRace() == RACE_BLOODELF)
                {
                    if (m_target->getGender() == 0)
                        m_target->SetDisplayId(17829);
                    else
                        m_target->SetDisplayId(17830);
                }

                if (m_target->getRace() == RACE_GNOME)
                {
                    if (m_target->getGender() == 0)
                        m_target->SetDisplayId(10148);
                    else
                        m_target->SetDisplayId(10149);
                }
                if (m_target->getRace() == RACE_DWARF)
                {
                    if (m_target->getGender() == 0)
                        m_target->SetDisplayId(10141);
                    else
                        m_target->SetDisplayId(10142);
                }
                if (m_target->getRace() == RACE_HUMAN)
                {
                    if (m_target->getGender() == 0)
                        m_target->SetDisplayId(10137);
                    else
                        m_target->SetDisplayId(10138);
                }
                if (m_target->getRace() == RACE_NIGHTELF)
                {
                    if (m_target->getGender() == 0)
                        m_target->SetDisplayId(10143);
                    else
                        m_target->SetDisplayId(10144);
                }
                if (m_target->getRace() == RACE_DRAENEI)
                {
                    if (m_target->getGender() == 0)
                        m_target->SetDisplayId(17827);
                    else
                        m_target->SetDisplayId(17828);
                }
            }
            else
                m_target->SetDisplayId(m_target->GetNativeDisplayId());
        }
        break;

        case 42365:	// murloc costume
            m_target->SetDisplayId(apply ? 21723 : m_target->GetNativeDisplayId());
            break;

        case 118://polymorph
        case 851:
        case 5254:
        case 12824:
        case 12825:
        case 12826:
        case 13323:
        case 15534:
        case 22274:
        case 23603:
        case 28270:	 // Polymorph: Cow
        case 28271:	 // Polymorph: Turtle
        case 28272:	 // Polymorph: Pig
        case 61025:  // Polymorph: Serpent
        case 61305:  // Polymorph: Black Cat
        case 61721:  // Polymorph: Rabbit
        case 61780:  // Polymorph: Turkey
        {
            if (!displayId)
            {
                switch (GetSpellInfo()->getId())
                {
                    case 28270:	 // Cow
                        displayId = 1060;
                        break;

                    case 28272:	 // Pig
                        displayId = 16356 + Util::getRandomUInt(2);
                        break;

                    case 28271:	 // Turtle
                        displayId = 16359 + Util::getRandomUInt(2);
                        break;

                    default:
                        displayId = 856;
                        break;

                }
            }

            if (apply)
            {
                Unit* caster = GetUnitCaster();
                if (caster != nullptr && m_target->IsCreature())
                    m_target->GetAIInterface()->AttackReaction(caster, 1, GetSpellId());

                m_target->SetDisplayId(displayId);

                // remove the current spell
                if (m_target->isCastingNonMeleeSpell())
                {
                    m_target->interruptSpell();
                }

                sEventMgr.AddEvent(this, &Aura::EventPeriodicHeal1, (uint32)1000, EVENT_AURA_PERIODIC_HEAL, 1000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
                m_target->polySpell = GetSpellInfo()->getId();
            }
            else
            {
                m_target->SetDisplayId(m_target->GetNativeDisplayId());
                m_target->polySpell = 0;
            }
        }
        break;

        case 19937:
        {
            if (apply)
            {
                ///\todo Sniff the spell / item, we need to know the real displayID
                // guessed this may not be correct
                // human = 7820
                // dwarf = 7819
                // Halfling = 7818
                // maybe 7842 as its from a lesser npc
                m_target->setUInt32Value(UNIT_FIELD_DISPLAYID, 7842);
            }
            else
            {
                m_target->setUInt32Value(UNIT_FIELD_DISPLAYID, m_target->GetNativeDisplayId());
            }
        }
        break;

        default:
        {
            if (!displayId) return;

            if (apply)
            {
                m_target->setUInt32Value(UNIT_FIELD_DISPLAYID, displayId);
            }
            else
            {
                m_target->setUInt32Value(UNIT_FIELD_DISPLAYID, m_target->GetNativeDisplayId());
            }
        }
        break;
    };

    m_target->EventModelChange();
}

void Aura::SpellAuraModSpellCritChance(bool apply)
{
    if (p_target != nullptr)
    {
        int32 amt;
        if (apply)
        {
            amt = mod->m_amount;
            if (amt < 0)
                SetNegative();
            else
                SetPositive();
        }
        else
            amt = -mod->m_amount;

        p_target->spellcritperc += amt;
        p_target->SetSpellCritFromSpell(p_target->GetSpellCritFromSpell() + amt);
        p_target->UpdateChanceFields();
    }
}

void Aura::SpellAuraIncreaseSwimSpeed(bool apply)
{
    if (apply)
    {
        if (m_target->isAlive())  SetPositive();
        m_target->m_currentSpeedSwim = 0.04722222f * (100 + mod->m_amount);
    }
    else
        m_target->m_currentSpeedSwim = m_target->m_basicSpeedSwim;
    if (p_target != nullptr)
    {
        WorldPacket data(SMSG_FORCE_SWIM_SPEED_CHANGE, 17);
        data << p_target->GetNewGUID();
        data << (uint32)2;
        data << m_target->m_currentSpeedSwim;
        p_target->GetSession()->SendPacket(&data);
    }
}

void Aura::SpellAuraModCratureDmgDone(bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            for (uint8 x = 0; x < 11; x++)
                if (mod->m_miscValue & ((uint32)1 << x))
                    p_target->IncreaseDamageByType[x + 1] += mod->m_amount;

            mod->m_amount < 0 ? SetNegative() : SetPositive();
        }
        else
            for (uint8 x = 0; x < 11; x++)
                if (mod->m_miscValue & (((uint32)1) << x))
                    p_target->IncreaseDamageByType[x + 1] -= mod->m_amount;
    }
}

void Aura::SpellAuraPacifySilence(bool apply)
{
    // Can't Attack or Cast Spells
    if (apply)
    {
        if (m_spellInfo->getId() == 24937)
            SetPositive();
        else
            SetNegative();

        m_target->m_pacified++;
        m_target->m_silenced++;
        m_target->addUnitStateFlag(UNIT_STATE_PACIFY | UNIT_STATE_SILENCE);
        m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_SILENCED);

        if (m_target->isCastingNonMeleeSpell())
        {
            m_target->interruptSpell();
        }
    }
    else
    {
        m_target->m_pacified--;

        if (m_target->m_pacified == 0)
        {
            m_target->removeUnitStateFlag(UNIT_STATE_PACIFY);
            m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
        }

        m_target->m_silenced--;

        if (m_target->m_silenced == 0)
        {
            m_target->removeUnitStateFlag(UNIT_STATE_SILENCE);
            m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SILENCED);
        }
    }
}

void Aura::SpellAuraModScale(bool apply)
{
    float current = m_target->GetScale();
    float delta = mod->m_amount / 100.0f;

    m_target->SetScale(apply ? (current + current * delta) : current / (1.0f + delta));
}

void Aura::SpellAuraPeriodicHealthFunnel(bool apply)
{
    if (apply)
    {
        uint32 amt = mod->m_amount;
        sEventMgr.AddEvent(this, &Aura::EventPeriodicHealthFunnel, amt,
                           EVENT_AURA_PERIODIC_HEALTH_FUNNEL, GetSpellInfo()->getEffectAmplitude(mod->m_effectIndex), 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Aura::EventPeriodicHealthFunnel(uint32 amount)
{
    //Blood Siphon
    //Deals 200 damage per second.
    //Feeding Hakkar 1000 health per second.
    Unit* m_caster = GetUnitCaster();
    if (!m_caster)
        return;
    if (m_target->isAlive() && m_caster->isAlive())
    {

        m_caster->DealDamage(m_target, amount, 0, 0, GetSpellId(), true);
        uint32 newHealth = m_caster->GetHealth() + 1000;

        uint32 mh = m_caster->GetMaxHealth();
        if (newHealth <= mh)
            m_caster->SetHealth(newHealth);
        else
            m_caster->SetHealth(mh);

        m_target->SendPeriodicAuraLog(m_target->GetNewGUID(), m_target->GetNewGUID(), m_spellInfo->getId(), m_spellInfo->getSchool(), 1000, 0, 0, FLAG_PERIODIC_LEECH, false);

        m_caster->RemoveAurasByHeal();
    }
}

void Aura::SpellAuraPeriodicManaLeech(bool apply)
{
    if (apply)
    {
        uint32 amt = mod->m_amount;
        uint32 mult = amt;

        amt = mult * m_target->GetMaxPower(POWER_TYPE_MANA) / 100;

        Unit* caster = GetUnitCaster();
        if (caster != nullptr)
        {
            if (amt > caster->GetMaxPower(POWER_TYPE_MANA) * (mult << 1) / 100)
                amt = caster->GetMaxPower(POWER_TYPE_MANA) * (mult << 1) / 100;
        }
        sEventMgr.AddEvent(this, &Aura::EventPeriodicManaLeech, amt,
                           EVENT_AURA_PERIODIC_LEECH, GetSpellInfo()->getEffectAmplitude(mod->m_effectIndex), 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Aura::EventPeriodicManaLeech(uint32 amount)
{
    Unit* m_caster = GetUnitCaster();
    if (!m_caster)
        return;
    if (m_target->isAlive() && m_caster->isAlive())
    {

        int32 amt = (int32)std::min(amount, m_target->GetPower(POWER_TYPE_MANA));
        uint32 cm = m_caster->GetPower(POWER_TYPE_MANA) + amt;
        uint32 mm = m_caster->GetMaxPower(POWER_TYPE_MANA);
        if (cm <= mm)
            m_caster->SetPower(POWER_TYPE_MANA, cm);
        else
            m_caster->SetPower(POWER_TYPE_MANA, mm);
        m_target->ModPower(POWER_TYPE_MANA, -amt);
    }
}

void Aura::SpellAuraModCastingSpeed(bool apply)
{
    float current = m_target->GetCastSpeedMod();
    if (apply)
        current -= mod->m_amount / 100.0f;
    else
        current += mod->m_amount / 100.0f;

    m_target->SetCastSpeedMod(current);
}

bool isFeignDeathResisted(uint32 playerlevel, uint32 moblevel)
{
    int fMobRes = 0;
    int diff = 0;

    if (playerlevel < moblevel)
    {
        diff = moblevel - playerlevel;

        if (diff <= 2)
            fMobRes = diff + 4;
        else
            fMobRes = (diff - 2) * 11 + 6;

        if (fMobRes > 100)
            fMobRes = 100;

        if (Util::getRandomUInt(1, 100) < static_cast<uint32>(fMobRes))
            return true;
    }

    return false;
}

void Aura::SpellAuraFeignDeath(bool apply)
{
    if (p_target != nullptr)
    {
        WorldPacket data(50);

        if (apply)
        {
            p_target->EventAttackStop();
            p_target->setDeathState(ALIVE);

#if VERSION_STRING != Classic
            p_target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
#endif
            p_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH);
            p_target->SetFlag(UNIT_DYNAMIC_FLAGS, U_DYN_FLAG_DEAD);

            //now get rid of mobs agro. pTarget->CombatStatus.AttackersForgetHate() - this works only for already attacking mobs
            for (const auto& itr : p_target->getInRangeObjectsSet())
            {
                if (itr && itr->IsUnit() && static_cast<Unit*>(itr)->isAlive())
                {
                    Unit* u = static_cast<Unit*>(itr);
                    if (isFeignDeathResisted(p_target->getLevel(), u->getLevel()))
                    {
                        sEventMgr.AddEvent(this, &Aura::Remove, EVENT_AURA_REMOVE, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT | EVENT_FLAG_DELETES_OBJECT);
                        return;
                    }

                    if (u->IsCreature())
                        u->GetAIInterface()->RemoveThreatByPtr(p_target);

                    //if this is player and targeting us then we interrupt cast
                    if (u->IsPlayer())
                    {
                        Player* plr = static_cast<Player*>(itr);
                        if (plr->isCastingNonMeleeSpell())
                            plr->interruptSpell(); // cancel current casting spell

                        plr->GetSession()->SendPacket(&data);
                    }
                }
            }

            // this looks awkward!
            p_target->SendMirrorTimer(MIRROR_TYPE_FIRE, GetDuration(), GetDuration(), 0xFFFFFFFF);

            p_target->GetSession()->SendPacket(&data);

            data.Initialize(SMSG_CLEAR_TARGET);
            data << p_target->GetGUID();

            p_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_COMBAT);

            if (p_target->hasUnitStateFlag(UNIT_STATE_ATTACKING))
                p_target->removeUnitStateFlag(UNIT_STATE_ATTACKING);

            p_target->GetSession()->OutPacket(SMSG_CANCEL_COMBAT);
            p_target->GetSession()->OutPacket(SMSG_CANCEL_AUTO_REPEAT);
        }
        else
        {
#if VERSION_STRING != Classic
            p_target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
#endif
            p_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH);
            p_target->RemoveFlag(UNIT_DYNAMIC_FLAGS, U_DYN_FLAG_DEAD);
            p_target->StopMirrorTimer(MIRROR_TYPE_FIRE);
        }
    }
}

void Aura::SpellAuraModDisarm(bool apply)
{
    uint32_t flag;
    uint16_t field;

    switch (mod->m_type)
    {
        case SPELL_AURA_MOD_DISARM:
            field = UNIT_FIELD_FLAGS;
            flag = UNIT_FLAG_DISARMED;
            break;
#if VERSION_STRING != Classic
        case SPELL_AURA_MOD_DISARM_OFFHAND:
            field = UNIT_FIELD_FLAGS_2;
            flag = UNIT_FLAG2_DISARM_OFFHAND;
            break;
        case SPELL_AURA_MOD_DISARM_RANGED:
            field = UNIT_FIELD_FLAGS_2;
            flag = UNIT_FLAG2_DISARM_RANGED;
            break;
#endif
        default:
            return;
    }

    if (apply)
    {
        if (p_target != nullptr && p_target->IsInFeralForm())
            return;

        SetNegative();

        m_target->disarmed = true;
        m_target->addUnitStateFlag(UNIT_STATE_DISARMED);
        m_target->SetFlag(field, flag);
    }
    else
    {
        m_target->disarmed = false;
        m_target->removeUnitStateFlag(UNIT_STATE_DISARMED);
        m_target->RemoveFlag(field, flag);
    }
}

void Aura::SpellAuraModStalked(bool apply)
{
    if (apply)
    {
        m_target->stalkedby = m_casterGuid;
        SetNegative();
    }
    else
    {
        m_target->stalkedby = 0;
    }
}

void Aura::SpellAuraSchoolAbsorb(bool /*apply*/)
{
    // See AbsorbAura::SpellAuraSchoolAbsorb
}

void Aura::SpellAuraModSpellCritChanceSchool(bool apply)
{
    if (apply)
    {
        for (uint8 x = 0; x < SCHOOL_COUNT; x++)
            if (mod->m_miscValue & (((uint32)1) << x))
                m_target->SpellCritChanceSchool[x] += mod->m_amount;
        if (mod->m_amount < 0)
            SetNegative();
        else
            SetPositive();
    }
    else
    {
        for (uint32 x = 0; x < 7; x++)
        {
            if (mod->m_miscValue & (((uint32)1) << x))
            {
                m_target->SpellCritChanceSchool[x] -= mod->m_amount;
                /*if (m_target->SpellCritChanceSchool[x] < 0)
                    m_target->SpellCritChanceSchool[x] = 0;*/
            }
        }
    }
    if (p_target != nullptr)
        p_target->UpdateChanceFields();
}

void Aura::SpellAuraModPowerCost(bool apply)
{
    int32 val = (apply) ? mod->m_amount : -mod->m_amount;
    if (apply)
    {
        if (val > 0)
            SetNegative();
        else
            SetPositive();
    }
    for (uint16_t x = 0; x < 7; x++)
    {
        if (mod->m_miscValue & (((uint32)1) << x))
        {
            m_target->ModPowerCostMultiplier(x, val / 100.0f);
        }
    }
}

void Aura::SpellAuraModPowerCostSchool(bool apply)
{
    if (apply)
    {
        for (uint32 x = 1; x < 7; x++)
            if (mod->m_miscValue & (((uint32)1) << x))
                m_target->PowerCostMod[x] += mod->m_amount;
    }
    else
    {
        for (uint32 x = 1; x < 7; x++)
        {
            if (mod->m_miscValue & (((uint32)1) << x))
            {
                m_target->PowerCostMod[x] -= mod->m_amount;
            }
        }
    }
}

void Aura::SpellAuraReflectSpellsSchool(bool apply)
{
    m_target->RemoveReflect(GetSpellId(), apply);

    if (apply)
    {
        ReflectSpellSchool* rss = new ReflectSpellSchool;
        rss->chance = mod->m_amount;
        rss->spellId = GetSpellId();
        rss->infront = false;

        if (m_spellInfo->getAttributes() == 0x400D0 && m_spellInfo->getAttributesEx() == 0)
            rss->school = (int)(log10((float)mod->m_miscValue) / log10((float)2));
        else
            rss->school = m_spellInfo->getSchool();

        rss->charges = 0;

        m_target->m_reflectSpellSchool.push_back(rss);
    }
}

void Aura::SpellAuraModLanguage(bool apply)
{
    if (apply)
        m_target->m_modlanguage = mod->m_miscValue;
    else
        m_target->m_modlanguage = -1;
}

void Aura::SpellAuraAddFarSight(bool apply)
{
    if (apply)
    {
        if (!m_target->IsPlayer())
            return;

        //FIXME:grep aka Nublex will fix this
        //Make update circle bigger here
    }
    else
    {
        //Destroy new updated objects here if they are still out of update range
        //w/e
    }
}

void Aura::SpellAuraMechanicImmunity(bool apply)
{
    if (apply)
    {
        ARCEMU_ASSERT(mod->m_miscValue < MECHANIC_END);
        m_target->MechanicsDispels[mod->m_miscValue]++;

        if (mod->m_miscValue != 16 && mod->m_miscValue != 25 && mod->m_miscValue != 19) // don't remove bandages, Power Word and protection effect
        {
            /* Supa's test run of Unit::RemoveAllAurasByMechanic */
            m_target->RemoveAllAurasByMechanic((uint32)mod->m_miscValue, 0, false);

            //Insignia/Medallion of A/H			//Every Man for Himself
            if (m_spellInfo->getId() == 42292 || m_spellInfo->getId() == 59752)
            {
                for (uint32 x = MAX_NEGATIVE_AURAS_EXTEDED_START; x < MAX_NEGATIVE_AURAS_EXTEDED_END; ++x)
                {
                    if (m_target->m_auras[x])
                    {
                        for (uint8_t y = 0; y < 3; ++y)
                        {
                            switch (m_target->m_auras[x]->GetSpellInfo()->getEffectApplyAuraName(y))
                            {
                                case SPELL_AURA_MOD_STUN:
                                case SPELL_AURA_MOD_CONFUSE:
                                case SPELL_AURA_MOD_ROOT:
                                case SPELL_AURA_MOD_FEAR:
                                case SPELL_AURA_MOD_DECREASE_SPEED:
                                    m_target->m_auras[x]->Remove();
                                    goto out;
                                    break;
                            }
                            continue;

                            out:
                            break;
                        }
                    }
                }
            }
        }
        else
            SetNegative();

        // Demonic Circle hack
        if (m_spellInfo->getId() == 48020 && m_target->IsPlayer() && m_target->HasAura(62388))
        {
            GameObject* obj = m_target->GetMapMgr()->GetGameObject(m_target->m_ObjectSlots[0]);

            if (obj != nullptr)
            {
                Player* ptarget = static_cast< Player* >(m_target);

                ptarget->SafeTeleport(obj->GetMapId(), obj->GetInstanceID(), obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), m_target->GetOrientation());
            }
        }
    }
    else
        m_target->MechanicsDispels[mod->m_miscValue]--;
}

void Aura::SpellAuraMounted(bool apply)
{
    if (!p_target) return;

    /*Shady: Is it necessary? Stealth should be broken since all spells with Mounted SpellEffect don't have "does not break stealth" flag (except internal Video mount spell).
    So commented, cause we don't need useless checks and hackfixes*/
    /* if (m_target->IsStealth())
    {
    uint32 id = m_target->m_stealth;
    m_target->m_stealth = 0;
    m_target->RemoveAura(id);
    }*/

    if (apply)
    {

        SetPositive();

        //p_target->AdvanceSkillLine(762); // advance riding skill

        if (p_target->m_bg)
            p_target->m_bg->HookOnMount(p_target);

        p_target->Dismount();

        m_target->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_MOUNT);

        CreatureProperties const* ci = sMySQLStore.getCreatureProperties(mod->m_miscValue);
        if (ci == nullptr)
            return;

        uint32 displayId = ci->Male_DisplayID;
        if (!displayId)
            return;

        p_target->m_MountSpellId = m_spellInfo->getId();
        p_target->flying_aura = 0;
        m_target->setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, displayId);
        //m_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);

        if (p_target->GetShapeShift() && !(p_target->GetShapeShift() & (FORM_BATTLESTANCE | FORM_DEFENSIVESTANCE | FORM_BERSERKERSTANCE)) && p_target->m_ShapeShifted != m_spellInfo->getId())
            p_target->RemoveAura(p_target->m_ShapeShifted);

        p_target->DismissActivePets();

        p_target->mountvehicleid = ci->vehicleid;

        if (p_target->mountvehicleid != 0)
        {
            p_target->AddVehicleComponent(ci->Id, ci->vehicleid);

#if VERSION_STRING > TBC
            WorldPacket data(SMSG_PLAYER_VEHICLE_DATA, 12);
            data << p_target->GetNewGUID();
            data << uint32(p_target->mountvehicleid);
            p_target->SendMessageToSet(&data, true);

            data.Initialize(SMSG_CONTROL_VEHICLE);
            p_target->SendPacket(&data);
#endif

            p_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT);
            p_target->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_PLAYER_VEHICLE);

            p_target->GetVehicleComponent()->InstallAccessories();
        }

    }
    else
    {
        if (p_target->GetVehicleComponent() != nullptr)
        {
            p_target->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_PLAYER_VEHICLE);
            p_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNT);

            p_target->GetVehicleComponent()->RemoveAccessories();
            p_target->GetVehicleComponent()->EjectAllPassengers();

#if VERSION_STRING > TBC
            WorldPacket data(SMSG_PLAYER_VEHICLE_DATA, 12);
            data << p_target->GetNewGUID();
            data << uint32(0);
            p_target->SendMessageToSet(&data, true);
#endif

            p_target->RemoveVehicleComponent();
        }

        p_target->mountvehicleid = 0;
        p_target->m_MountSpellId = 0;
        p_target->flying_aura = 0;
        m_target->SetMount(0);
        //m_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_MOUNTED_TAXI);

        //if we had pet then respawn
        p_target->SpawnActivePet();
        p_target->RemoveAurasByInterruptFlag(AURA_INTERRUPT_ON_DISMOUNT);
    }
}

void Aura::SpellAuraModDamagePercDone(bool apply)
{
    float val = (apply) ? mod->m_amount / 100.0f : -mod->m_amount / 100.0f;

    switch (GetSpellId())  //dirty or mb not fix bug with wand specializations
    {
        case 14524:
        case 14525:
        case 14526:
        case 14527:
        case 14528:
            return;
    }
    if (p_target != nullptr)
    {
        if (GetSpellInfo()->getEquippedItemClass() == -1)  //does not depend on weapon
        {
            for (uint8 x = 0; x < SCHOOL_COUNT; x++)
            {
                if (mod->m_miscValue & ((uint32)1 << x))
                {
                    // display to client (things that are weapon dependant don't get displayed)
                    p_target->modFloatValue(PLAYER_FIELD_MOD_DAMAGE_DONE_PCT + x, val);
                }
            }
        }
        if (mod->m_miscValue & 1)
        {
            if (apply)
            {
                WeaponModifier md;
                md.value = val;
                md.wclass = GetSpellInfo()->getEquippedItemClass();
                md.subclass = GetSpellInfo()->getEquippedItemSubClass();
                p_target->damagedone.insert(std::make_pair(GetSpellId(), md));
            }
            else
            {
                std::map< uint32, WeaponModifier >::iterator i = p_target->damagedone.begin();

                for (; i != p_target->damagedone.end(); ++i)
                {
                    if ((*i).first == GetSpellId())
                    {
                        p_target->damagedone.erase(i);
                        break;
                    }
                }
                p_target->damagedone.erase(GetSpellId());
            }
        }
    }
    else
    {
        for (uint8 x = 0; x < 7; x++)
        {
            if (mod->m_miscValue & ((uint32)1 << x))
            {
                static_cast< Creature* >(m_target)->ModDamageDonePct[x] += val;
            }
        }
    }
    m_target->CalcDamage();
}

void Aura::SpellAuraModPercStat(bool apply)
{
    int32 val;
    if (apply)
    {
        val = mod->m_amount;
        if (val < 0)
            SetNegative();
        else
            SetPositive();
    }
    else
        val = -mod->m_amount;

    if (mod->m_miscValue == -1) //all stats
    {
        if (p_target != nullptr)
        {
            for (uint8 x = 0; x < 5; x++)
            {
                if (mod->m_amount > 0)
                    p_target->StatModPctPos[x] += val;
                else
                    p_target->StatModPctNeg[x] -= val;

                p_target->CalcStat(x);
            }

            p_target->UpdateStats();
            p_target->UpdateChances();
        }
        else
        {
            for (uint8 x = 0; x < 5; x++)
            {
                static_cast< Creature* >(m_target)->StatModPct[x] += val;
                static_cast< Creature* >(m_target)->CalcStat(x);
            }
        }
    }
    else
    {
        ARCEMU_ASSERT(mod->m_miscValue < 5);
        uint16_t modValue = static_cast<uint16_t>(mod->m_miscValue);
        if (p_target != nullptr)
        {
            if (mod->m_amount > 0)
                p_target->StatModPctPos[modValue] += val;
            else
                p_target->StatModPctNeg[modValue] -= val;

            p_target->CalcStat(modValue);

            p_target->UpdateStats();
            p_target->UpdateChances();
        }
        else if (m_target->IsCreature())
        {
            static_cast< Creature* >(m_target)->StatModPct[modValue] += val;
            static_cast< Creature* >(m_target)->CalcStat(modValue);
        }
    }
}

void Aura::SpellAuraSplitDamage(bool apply)
{
    Unit* source = nullptr;         // This is the Unit whose damage we are splitting
    Unit* destination = nullptr;    // This is the Unit that shares the beating
    Object* caster = GetCaster();

    // We don't want to split our damage with the owner
    if ((m_spellInfo->getEffect(mod->m_effectIndex) == SPELL_EFFECT_APPLY_OWNER_AREA_AURA) &&
        (caster != nullptr) &&
        (m_target != nullptr) &&
        caster->IsPet() &&
        caster->GetGUID() == m_target->GetGUID())
        return;

    if (m_areaAura)
    {
        source = GetTarget();
        destination = GetUnitCaster();
    }
    else
    {
        source = GetUnitCaster();
        destination = GetTarget();
    }

    if (source == nullptr || destination == nullptr)
        return;

    if (source->m_damageSplitTarget != nullptr)
    {
        delete source->m_damageSplitTarget;
        source->m_damageSplitTarget = nullptr;
    }

    if (apply)
    {
        DamageSplitTarget* ds = new DamageSplitTarget;
        ds->m_flatDamageSplit = 0;
        ds->m_spellId = GetSpellInfo()->getId();
        ds->m_pctDamageSplit = mod->m_miscValue / 100.0f;
        ds->damage_type = static_cast<uint8>(mod->m_type);
        ds->creator = (void*)this;
        ds->m_target = destination->GetGUID();
        source->m_damageSplitTarget = ds;
    }
    else
    {
        DamageSplitTarget* ds = source->m_damageSplitTarget;
        source->m_damageSplitTarget = nullptr;
        delete ds;
    }
}

void Aura::SpellAuraModRegen(bool apply)
{
    if (apply)//seems like only positive
    {
        SetPositive();
        sEventMgr.AddEvent(this, &Aura::EventPeriodicHeal1, (uint32)((this->GetSpellInfo()->getEffectBasePoints(mod->m_effectIndex) + 1) / 5) * 3,
                           EVENT_AURA_PERIODIC_REGEN, 3000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Aura::SpellAuraPeriodicTriggerDummy(bool apply)
{
    if (apply)
    {
        sEventMgr.AddEvent(this, &Aura::EventPeriodicTriggerDummy, EVENT_AURA_PERIODIC_DUMMY, m_spellInfo->getEffectAmplitude(mod->m_effectIndex), 0, 0);
    }
    else
    {
        if (!sScriptMgr.CallScriptedDummyAura(m_spellInfo->getId(), mod->m_effectIndex, this, false))
            LOG_ERROR("Spell %u (%s) has an apply periodic trigger dummy aura effect, but no handler for it.", m_spellInfo->getId(), m_spellInfo->getName().c_str());
    }
}

void Aura::EventPeriodicTrigger(uint32 /*amount*/, uint32 /*type*/)
{

}

void Aura::EventPeriodicEnergizeVariable(uint32 amount, uint32 type)
{
    uint32 POWER_TYPE = UNIT_FIELD_POWER1 + type;

    ARCEMU_ASSERT(POWER_TYPE <= UNIT_FIELD_POWER5);

    Unit* ucaster = GetUnitCaster();
    if (ucaster != nullptr)
        ucaster->Energize(m_target, m_spellInfo->getId(), amount, type);
}

void Aura::EventPeriodicDrink(uint32 amount)
{
    uint32 v = m_target->GetPower(POWER_TYPE_MANA) + amount;

    if (v > m_target->GetMaxPower(POWER_TYPE_MANA))
        v = m_target->GetMaxPower(POWER_TYPE_MANA);

    m_target->SetPower(POWER_TYPE_MANA, v);
}

void Aura::EventPeriodicHeal1(uint32 amount)
{
    if (!m_target->isAlive())
        return;

    uint32 ch = m_target->GetHealth();
    ch += amount;
    uint32 mh = m_target->GetMaxHealth();

    if (ch > mh)
        m_target->SetHealth(mh);
    else
        m_target->SetHealth(ch);

    if (GetSpellInfo()->getAuraInterruptFlags() & AURA_INTERRUPT_ON_STAND_UP)
    {
        m_target->Emote(EMOTE_ONESHOT_EAT);
    }
    else
    {
        if (!(m_spellInfo->custom_BGR_one_buff_on_target & SPELL_TYPE_ARMOR))
            m_target->SendPeriodicHealAuraLog(m_casterGuid, m_target->GetNewGUID(), GetSpellId(), amount, 0, false);
    }

    m_target->RemoveAurasByHeal();
}

void Aura::SpellAuraModPowerRegen(bool apply)
{
    if (!mod->m_amount)
        return;

    if (apply)
    {
        if (mod->m_amount > 0)
            SetPositive();
        else
            SetNegative();
    }
    if (p_target != nullptr && mod->m_miscValue == POWER_TYPE_MANA)
    {
        int32 val = (apply) ? mod->m_amount : -mod->m_amount;
        p_target->m_ModInterrMRegen += val;
        p_target->UpdateStats();
    }
}

void Aura::SpellAuraChannelDeathItem(bool apply)
{
    SetNegative(); //this should always be negative as npcs remove negative auras on death

    if (apply)
    {
        //don't need for now
    }
    else
    {
        if (m_target->IsUnit())
        {
            if (m_target->IsCreature() && static_cast<Creature*>(m_target)->GetCreatureProperties()->Type == UNIT_TYPE_CRITTER)
                return;

            if (m_target->IsDead())
            {
                Player* pCaster = m_target->GetMapMgr()->GetPlayer((uint32)m_casterGuid);
                if (!pCaster)
                    return;
                /*int32 delta=pCaster->getLevel()-m_target->getLevel();
                if (abs(delta)>5)
                return;*/

                uint32 itemid = GetSpellInfo()->getEffectItemType(mod->m_effectIndex);

                //Warlocks only get Soul Shards from enemies that grant XP or Honor
                if (itemid == 6265 && (pCaster->getLevel() > m_target->getLevel()))
                    if ((pCaster->getLevel() - m_target->getLevel()) > 9)
                        return;


                ItemProperties const* proto = sMySQLStore.getItemProperties(itemid);
                if (pCaster->GetItemInterface()->CalculateFreeSlots(proto) > 0)
                {
                    Item* item = objmgr.CreateItem(itemid, pCaster);
                    if (!item)
                        return;

                    item->SetCreatorGUID(pCaster->GetGUID());
                    if (!pCaster->GetItemInterface()->AddItemToFreeSlot(item))
                    {
                        pCaster->GetItemInterface()->BuildInventoryChangeError(nullptr, nullptr, INV_ERR_INVENTORY_FULL);
                        item->DeleteMe();
                        return;
                    }
                    SlotResult* lr = pCaster->GetItemInterface()->LastSearchResult();

                    pCaster->SendItemPushResult(true, false, true, true, lr->ContainerSlot, lr->Slot, 1, item->GetEntry(), item->GetItemRandomSuffixFactor(), item->GetItemRandomPropertyId(), item->GetStackCount());
                }
            }
        }
    }
}

void Aura::SpellAuraModDamagePercTaken(bool apply)
{
    float val;
    if (apply)
    {
        val = mod->m_amount / 100.0f;
        if (val <= 0)
            SetPositive();
        else
            SetNegative();
    }
    else
    {
        val = -mod->m_amount / 100.0f;
    }

    switch (m_spellInfo->getId())   // Ardent Defender it only applys on 20% hp :/
    {
        //SPELL_HASH_ARDENT_DEFENDER
        case 31850:
        case 31851:
        case 31852:
        case 66233:
        case 66235:
            m_target->DamageTakenPctModOnHP35 += val;
            break;
        default:
            break;
    }

    for (uint32 x = 0; x < 7; x++)
    {
        if (mod->m_miscValue & (((uint32)1) << x))
        {
            m_target->DamageTakenPctMod[x] += val;
        }
    }
}

void Aura::SpellAuraModRegenPercent(bool apply)
{
    if (apply)
        m_target->PctRegenModifier += mod->m_amount;
    else
        m_target->PctRegenModifier -= mod->m_amount;
}

void Aura::SpellAuraPeriodicDamagePercent(bool apply)
{
    if (apply)
    {
        //uint32 gr = GetSpellProto()->SpellGroupType;
        //if (gr)
        //{
        //	Unit*c=GetUnitCaster();
        //	if (c)
        //	{
        //		spellModFlatIntValue(c->SM_FDOT,(int32*)&dmg,gr);
        //		spellModPercentageIntValue(c->SM_PDOT,(int32*)&dmg,gr);
        //	}
        //}

        /*if (m_spellProto->getId() == 28347) //Dimensional Siphon
        {
        uint32 dmg = (m_target->GetMaxHealth()*5)/100;
        sEventMgr.AddEvent(this, &Aura::EventPeriodicDamagePercent, dmg,
        EVENT_AURA_PERIODIC_DAMAGE_PERCENT, 1000, 0,EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }
        else*/
        {
            uint32 dmg = mod->m_amount;
            sEventMgr.AddEvent(this, &Aura::EventPeriodicDamagePercent, dmg,
                               EVENT_AURA_PERIODIC_DAMAGE_PERCENT, GetSpellInfo()->getEffectAmplitude(mod->m_effectIndex), 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }
        SetNegative();
    }
}

void Aura::EventPeriodicDamagePercent(uint32 amount)
{
    //DOT
    if (!m_target->isAlive())
        return;
    if (m_target->SchoolImmunityList[GetSpellInfo()->getSchool()])
        return;

    uint32 damage = float2int32(amount / 100.0f * m_target->GetMaxHealth());

    Unit* c = GetUnitCaster();

    if (m_target->m_damageSplitTarget)
    {
        damage = m_target->DoDamageSplitTarget(damage, GetSpellInfo()->getSchool(), false);
    }

    if (c)
        c->SpellNonMeleeDamageLog(m_target, GetSpellInfo()->getId(), damage, pSpellId == 0, true);
    else
        m_target->SpellNonMeleeDamageLog(m_target, GetSpellInfo()->getId(), damage, pSpellId == 0, true);
}

void Aura::SpellAuraModResistChance(bool apply)
{
    apply ? m_target->m_resistChance = mod->m_amount : m_target->m_resistChance = 0;
}

void Aura::SpellAuraModDetectRange(bool apply)
{
    Unit* m_caster = GetUnitCaster();
    if (!m_caster)return;
    if (apply)
    {
        SetNegative();
        m_caster->setDetectRangeMod(m_target->GetGUID(), mod->m_amount);
    }
    else
    {
        m_caster->unsetDetectRangeMod(m_target->GetGUID());
    }
}

void Aura::SpellAuraPreventsFleeing(bool /*apply*/)
{
    // Curse of Recklessness
}

void Aura::SpellAuraModUnattackable(bool /*apply*/)
{
    /*
            Also known as Apply Aura: Mod Unintractable
            Used by: Spirit of Redemption, Divine Intervention, Phase Shift, Flask of Petrification
            It uses one of the UNIT_FIELD_FLAGS, either UNIT_FLAG_NOT_SELECTABLE or UNIT_FLAG_NOT_ATTACKABLE_2
            */
}

void Aura::SpellAuraInterruptRegen(bool apply)
{
    if (apply)
        m_target->m_interruptRegen++;
    else
    {
        m_target->m_interruptRegen--;
        if (m_target->m_interruptRegen < 0)
            m_target->m_interruptRegen = 0;
    }
}

void Aura::SpellAuraGhost(bool apply)
{
    if (p_target != nullptr)
    {
        m_target->m_invisible = apply;

        if (apply)
        {
            SetNegative();
            p_target->setMoveWaterWalk();
        }
        else
        {
            p_target->setMoveLandWalk();
        }
    }
}

void Aura::SpellAuraMagnet(bool apply)
{
    if (apply)
    {
        Unit* caster = GetUnitCaster();
        if (!caster)
            return;
        SetPositive();
        m_target->m_magnetcaster = caster->GetGUID();
    }
    else
    {
        m_target->m_magnetcaster = 0;
    }
}

void Aura::SpellAuraManaShield(bool apply)
{
    if (apply)
    {
        SetPositive();
        m_target->m_manashieldamt = mod->m_amount;
        m_target->m_manaShieldId = GetSpellId();
    }
    else
    {
        m_target->m_manashieldamt = 0;
        m_target->m_manaShieldId = 0;
    }
}

void Aura::SpellAuraSkillTalent(bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            SetPositive();
            p_target->_ModifySkillBonus(mod->m_miscValue, mod->m_amount);
        }
        else
            p_target->_ModifySkillBonus(mod->m_miscValue, -mod->m_amount);

        p_target->UpdateStats();
    }
}

void Aura::SpellAuraModAttackPower(bool apply)
{
    if (mod->m_amount < 0)
        SetNegative();
    else
        SetPositive();
    m_target->ModAttackPowerMods(apply ? mod->m_amount : -mod->m_amount);
    m_target->CalcDamage();
}

void Aura::SpellAuraVisible(bool apply)
{
    //Show positive spells on target
    if (apply)
    {
        SetNegative();
    }
}

void Aura::SpellAuraModResistancePCT(bool apply)
{
    uint32 Flag = mod->m_miscValue;
    int32 amt;
    if (apply)
    {
        amt = mod->m_amount;
        //   if (amt>0)SetPositive();
        // else SetNegative();
    }
    else
        amt = -mod->m_amount;

    for (uint16 x = 0; x < SCHOOL_COUNT; x++)
    {
        if (Flag & (((uint32)1) << x))
        {
            if (p_target != nullptr)
            {
                if (mod->m_amount > 0)
                {
                    p_target->ResistanceModPctPos[x] += amt;
                }
                else
                {
                    p_target->ResistanceModPctNeg[x] -= amt;
                }
                p_target->CalcResistance(x);

            }
            else if (m_target->IsCreature())
            {
                static_cast< Creature* >(m_target)->ResistanceModPct[x] += amt;
                static_cast< Creature* >(m_target)->CalcResistance(x);
            }
        }
    }
}

void Aura::SpellAuraModCreatureAttackPower(bool apply)
{
    if (apply)
    {
        for (uint32 x = 0; x < 11; x++)
            if (mod->m_miscValue & (((uint32)1) << x))
                m_target->CreatureAttackPowerMod[x + 1] += mod->m_amount;

        if (mod->m_amount > 0)
            SetPositive();
        else
            SetNegative();
    }
    else
    {
        for (uint32 x = 0; x < 11; x++)
        {
            if (mod->m_miscValue & (((uint32)1) << x))
            {
                m_target->CreatureAttackPowerMod[x + 1] -= mod->m_amount;
            }
        }
    }
    m_target->CalcDamage();
}

void Aura::SpellAuraModTotalThreat(bool apply)
{
    if (apply)
    {
        if (mod->m_amount < 0)
            SetPositive();
        else
            SetNegative();

        m_target->ModThreatModifyer(mod->m_amount);
    }
    else
        m_target->ModThreatModifyer(-(mod->m_amount));
}

void Aura::SpellAuraWaterWalk(bool apply)
{
    if (p_target != nullptr)
    {
        WorldPacket data(12);
        if (apply)
        {
            SetPositive();
            p_target->setMoveWaterWalk();
        }
        else
        {
            p_target->setMoveLandWalk();
        }
    }
}

void Aura::SpellAuraFeatherFall(bool apply)
{
    if (p_target == nullptr)
        return;

    if (apply)
    {
        SetPositive();
        p_target->setMoveFeatherFall();
        p_target->m_noFallDamage = true;
    }
    else
    {
        p_target->setMoveNormalFall();
        p_target->m_noFallDamage = false;
    }
}

void Aura::SpellAuraHover(bool apply)
{
#if VERSION_STRING > TBC
    SetPositive();

    if (apply)
    {
        m_target->setMoveHover(true);
        m_target->setFloatValue(UNIT_FIELD_HOVERHEIGHT, (float(mod->m_amount) / 2));
    }
    else
    {
        m_target->setMoveHover(false);
        m_target->setFloatValue(UNIT_FIELD_HOVERHEIGHT, 0.0f);
    }
#endif
}

void Aura::SpellAuraAddPctMod(bool apply)
{
    int32 val = apply ? mod->m_amount : -mod->m_amount;
#if VERSION_STRING != Cata
    uint32* AffectedGroups = GetSpellInfo()->getEffectSpellClassMask(mod->m_effectIndex);
#else
    uint32* AffectedGroups = GetSpellInfo()->EffectSpellClassMask;
#endif

    switch (mod->m_miscValue)  //let's generate warnings for unknown types of modifiers
    {
        case SMT_DAMAGE_DONE:
            SendModifierLog(&m_target->SM_PDamageBonus, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_DURATION:
            SendModifierLog(&m_target->SM_PDur, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_THREAT_REDUCED:
            SendModifierLog(&m_target->SM_PThreat, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_EFFECT_1:
            SendModifierLog(&m_target->SM_PEffect1_Bonus, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_CHARGES:
            SendModifierLog(&m_target->SM_PCharges, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_RANGE:
            SendModifierLog(&m_target->SM_PRange, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_RADIUS:
            SendModifierLog(&m_target->SM_PRadius, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_CRITICAL:
            SendModifierLog(&m_target->SM_CriticalChance, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_MISC_EFFECT:
            SendModifierLog(&m_target->SM_PMiscEffect, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_NONINTERRUPT:
            SendModifierLog(&m_target->SM_PNonInterrupt, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            //SpellAuraResistPushback(true);
            break;

        case SMT_CAST_TIME:
            SendModifierLog(&m_target->SM_PCastTime, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_COOLDOWN_DECREASE:
            SendModifierLog(&m_target->SM_PCooldownTime, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_EFFECT_2:
            SendModifierLog(&m_target->SM_PEffect2_Bonus, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_COST:
            SendModifierLog(&m_target->SM_PCost, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_CRITICAL_DAMAGE:
            SendModifierLog(&m_target->SM_PCriticalDamage, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

            //case SMT_HITCHANCE: - no pct
            //case SMT_ADDITIONAL_TARGET: - no pct
            ///\todo case SMT_TRIGGER: - todo

        case SMT_AMPTITUDE:
            SendModifierLog(&m_target->SM_PAmptitude, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_JUMP_REDUCE:
            SendModifierLog(&m_target->SM_PJumpReduce, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_GLOBAL_COOLDOWN:
            SendModifierLog(&m_target->SM_PGlobalCooldown, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_SPELL_VALUE_PCT:
            SendModifierLog(&m_target->SM_PDOT, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_EFFECT_3:
            SendModifierLog(&m_target->SM_PEffect3_Bonus, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_PENALTY:
            SendModifierLog(&m_target->SM_PPenalty, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_EFFECT_BONUS:
            SendModifierLog(&m_target->SM_PEffectBonus, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_RESIST_DISPEL:
            SendModifierLog(&m_target->SM_PRezist_dispell, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        default://Unknown modifier type
            LOG_ERROR("Unknown spell modifier type %u in spell %u.<<--report this line to the developer", mod->m_miscValue, GetSpellId());
            break;
    }
}

void Aura::SendModifierLog(int32** m, int32 v, uint32* mask, uint8 type, bool pct)
{
    uint32 intbit = 0, groupnum = 0;

    if (*m == nullptr)
    {
        *m = new int32[SPELL_GROUPS];
        for (uint32 bit = 0; bit < SPELL_GROUPS; ++bit, ++intbit)
        {
            if (intbit == 32)
            {
                ++groupnum;
                intbit = 0;
            }

            if ((1 << intbit) & mask[groupnum])
            {
                (*m)[bit] = v;

                if (!m_target->IsPlayer())
                    continue;

                static_cast<Player*>(m_target)->SendSpellModifier(static_cast<uint8>(bit), type, v, pct);
            }
            else
                (*m)[bit] = 0;
        }
    }
    else
    {
        for (uint8 bit = 0; bit < SPELL_GROUPS; ++bit, ++intbit)
        {
            if (intbit == 32)
            {
                ++groupnum;
                intbit = 0;
            }

            if ((1 << intbit) & mask[groupnum])
            {
                (*m)[bit] += v;

                if (!m_target->IsPlayer())
                    continue;

                static_cast<Player*>(m_target)->SendSpellModifier(bit, type, (*m)[bit], pct);
            }
        }
    }
}

void Aura::SendDummyModifierLog(std::map< SpellInfo*, uint32 >* m, SpellInfo* spellInfo, uint32 i, bool apply, bool pct)
{
    int32 v = spellInfo->getEffectBasePoints(static_cast<uint8_t>(i)) + 1;
#if VERSION_STRING != Cata
    uint32* mask = spellInfo->getEffectSpellClassMask(static_cast<uint8_t>(i));
#else
    uint32* mask = spellInfo->EffectSpellClassMask;
#endif
    uint8 type = static_cast<uint8>(spellInfo->getEffectMiscValue(static_cast<uint8_t>(i)));

    if (apply)
    {
        m->insert(std::make_pair(spellInfo, i));
    }
    else
    {
        v = -v;
        std::map<SpellInfo*, uint32>::iterator itr = m->find(spellInfo);
        if (itr != m->end())
            m->erase(itr);
    }

    uint32 intbit = 0, groupnum = 0;
    for (uint8 bit = 0; bit < SPELL_GROUPS; ++bit, ++intbit)
    {
        if (intbit == 32)
        {
            ++groupnum;
            intbit = 0;
        }
        if ((1 << intbit) & mask[groupnum])
        {
            if (p_target == nullptr)
                continue;

            p_target->SendSpellModifier(bit, type, v, pct);
        }
    }
}

void Aura::SpellAuraAddClassTargetTrigger(bool apply)
{
    if (apply)
    {
        uint32 groupRelation[3], procClassMask[3];
        int charges;
        SpellInfo* sp;

        // Find spell of effect to be triggered
        sp = sSpellCustomizations.GetSpellInfo(GetSpellInfo()->getEffectTriggerSpell(mod->m_effectIndex));
        if (sp == nullptr)
        {
            LogDebugFlag(LF_AURA, "Warning! class trigger spell is null for spell %u", GetSpellInfo()->getId());
            return;
        }

#if VERSION_STRING != Cata
        // Initialize proc class mask
        procClassMask[0] = GetSpellInfo()->getEffectSpellClassMask(mod->m_effectIndex, 0);
        procClassMask[1] = GetSpellInfo()->getEffectSpellClassMask(mod->m_effectIndex, 1);
        procClassMask[2] = GetSpellInfo()->getEffectSpellClassMask(mod->m_effectIndex, 2);

        // Initialize mask
        groupRelation[0] = sp->getEffectSpellClassMask(mod->m_effectIndex, 0);
        groupRelation[1] = sp->getEffectSpellClassMask(mod->m_effectIndex, 1);
        groupRelation[2] = sp->getEffectSpellClassMask(mod->m_effectIndex, 2);
#else
        // Initialize proc class mask
        procClassMask[0] = GetSpellInfo()->EffectSpellClassMask[0];
        procClassMask[1] = GetSpellInfo()->EffectSpellClassMask[1];
        procClassMask[2] = GetSpellInfo()->EffectSpellClassMask[2];

        // Initialize mask
        groupRelation[0] = sp->EffectSpellClassMask[0];
        groupRelation[1] = sp->EffectSpellClassMask[1];
        groupRelation[2] = sp->EffectSpellClassMask[2];
#endif

        // Initialize charges
        charges = GetSpellInfo()->getProcCharges();
        Unit* ucaster = GetUnitCaster();
        if (ucaster != nullptr)
        {
            spellModFlatIntValue(ucaster->SM_FCharges, &charges, GetSpellInfo()->getSpellGroupType());
            spellModPercentageIntValue(ucaster->SM_PCharges, &charges, GetSpellInfo()->getSpellGroupType());
        }

        m_target->AddProcTriggerSpell(sp->getId(), GetSpellInfo()->getId(), m_casterGuid, GetSpellInfo()->getEffectBasePoints(mod->m_effectIndex) + 1, PROC_ON_CAST_SPELL, charges, groupRelation, procClassMask);

        LogDebugFlag(LF_AURA, "%u is registering %u chance %u flags %u charges %u triggeronself %u interval %u", GetSpellInfo()->getId(), sp->getId(), GetSpellInfo()->getProcChance(), PROC_ON_CAST_SPELL, charges, GetSpellInfo()->getProcFlags() & PROC_TARGET_SELF, GetSpellInfo()->custom_proc_interval);
    }
    else
    {
        // Find spell of effect to be triggered
        uint32 spellId = GetSpellInfo()->getEffectTriggerSpell(mod->m_effectIndex);
        if (spellId == 0)
        {
            LogDebugFlag(LF_AURA, "Warning! trigger spell is null for spell %u", GetSpellInfo()->getId());
            return;
        }

        m_target->RemoveProcTriggerSpell(spellId, m_casterGuid);
    }
}

void Aura::SpellAuraModPowerRegPerc(bool apply)
{
    if (apply)
        m_target->PctPowerRegenModifier[mod->m_miscValue] += ((float)(mod->m_amount)) / 100.0f;
    else
        m_target->PctPowerRegenModifier[mod->m_miscValue] -= ((float)(mod->m_amount)) / 100.0f;
    if (p_target != nullptr)
        p_target->UpdateStats();
}

void Aura::SpellAuraOverrideClassScripts(bool apply)
{
    Player* plr = GetPlayerCaster();
    if (plr == nullptr)
        return;

    //misc value is spell to add
    //spell familyname && grouprelation

    //Adding bonus to effect
    switch (mod->m_miscValue)
    {
        //----Shatter---
        // Increases crit chance against rooted targets by (Rank * 10)%.
        case 849:
        case 910:
        case 911:
        case 912:
        case 913:
            if (p_target != nullptr)
            {
                int32 val = (apply) ? (mod->m_miscValue - 908) * 10 : -(mod->m_miscValue - 908) * 10;
                if (mod->m_miscValue == 849)
                    val = (apply) ? 10 : -10;
                p_target->m_RootedCritChanceBonus += val;
            }
            break;
            // ----?
        case 3736:
        case 4415:
        case 4418:
        case 4554:
        case 4555:
        case 4953:
        case 5142:
        case 5147:
        case 5148:
        case 6953:
        {
            if (apply)
            {
                MySQLDataStore::SpellOverrideIdMap::iterator itermap = sMySQLStore._spellOverrideIdStore.find(mod->m_miscValue);
                if (itermap == sMySQLStore._spellOverrideIdStore.end())
                {
                    LOG_ERROR("Unable to find override with overrideid: %u", mod->m_miscValue);
                    break;
                }

                std::list<SpellInfo*>::iterator itrSE = itermap->second->begin();

                SpellOverrideMap::iterator itr = plr->mSpellOverrideMap.find((*itrSE)->getId());

                if (itr != plr->mSpellOverrideMap.end())
                {
                    ScriptOverrideList::iterator itrSO;
                    for (itrSO = itr->second->begin(); itrSO != itr->second->end(); ++itrSO)
                    {
                        if ((*itrSO)->id == (uint32)mod->m_miscValue)
                        {
                            if ((int32)(*itrSO)->damage > mod->m_amount)
                            {
                                (*itrSO)->damage = mod->m_amount;
                            }
                            return;
                        }
                    }
                    classScriptOverride* cso = new classScriptOverride;
                    cso->aura = 0;
                    cso->damage = mod->m_amount;
                    cso->effect = 0;
                    cso->id = mod->m_miscValue;
                    itr->second->push_back(cso);
                }
                else
                {
                    classScriptOverride* cso = new classScriptOverride;
                    cso->aura = 0;
                    cso->damage = mod->m_amount;
                    cso->effect = 0;
                    cso->id = mod->m_miscValue;
                    ScriptOverrideList* lst = new ScriptOverrideList();
                    lst->push_back(cso);

                    for (; itrSE != itermap->second->end(); ++itrSE)
                    {
                        plr->mSpellOverrideMap.insert(SpellOverrideMap::value_type((*itrSE)->getId(), lst));
                    }

                    delete lst;
                }
            }
            else
            {
                MySQLDataStore::SpellOverrideIdMap::iterator itermap = sMySQLStore._spellOverrideIdStore.find(mod->m_miscValue);
                SpellOverrideMap::iterator itr = plr->mSpellOverrideMap.begin(), itr2;
                while (itr != plr->mSpellOverrideMap.end())
                {
                    std::list<SpellInfo*>::iterator itrSE = itermap->second->begin();
                    for (; itrSE != itermap->second->end(); ++itrSE)
                    {
                        if (itr->first == (*itrSE)->getId())
                        {
                            itr2 = itr++;
                            plr->mSpellOverrideMap.erase(itr2);
                            break;
                        }
                    }
                    // Check if the loop above got to the end, if so it means the item wasn't found
                    // and the itr wasn't incremented so increment it now.
                    if (itrSE == itermap->second->end())
                        ++itr;
                }
            }
        }
        break;
        /*		case 19421: //hunter : Improved Hunter's Mark
                case 19422:
                case 19423:
                case 19424:
                case 19425:
                {
                //this should actually add a new functionality to the spell and not override it. There is a lot to decode and to be done here
                }break;*/
        case 4992: // Warlock: Soul Siphon
        case 4993:
        {
            if (apply)
                m_target->m_soulSiphon.max += mod->m_amount;
            else
                m_target->m_soulSiphon.max -= mod->m_amount;
        }
        break;
        default:
            LOG_ERROR("Unknown override report to devs: %u", mod->m_miscValue);
    };
}

void Aura::SpellAuraModRangedDamageTaken(bool apply)
{
    if (apply)
        m_target->RangedDamageTaken += mod->m_amount;
    else
    {
        m_target->RangedDamageTaken -= mod->m_amount;
        if (m_target->RangedDamageTaken < 0)
            m_target->RangedDamageTaken = 0;
    }
}

void Aura::SpellAuraModHealing(bool apply)
{
    int32 val;
    if (apply)
    {
        val = mod->m_amount;
        /*if (val>0)
         SetPositive();
         else
         SetNegative();*/
    }
    else
        val = -mod->m_amount;

    for (uint8 x = 0; x < 7; x++)
    {
        if (mod->m_miscValue & (((uint32)1) << x))
        {
            m_target->HealTakenMod[x] += val;
        }
    }
}

void Aura::SpellAuraIgnoreRegenInterrupt(bool apply)
{
    if (p_target == nullptr)
        return;

    if (apply)
        p_target->PctIgnoreRegenModifier += ((float)(mod->m_amount)) / 100;
    else
        p_target->PctIgnoreRegenModifier -= ((float)(mod->m_amount)) / 100;
}

void Aura::SpellAuraModMechanicResistance(bool apply)
{
    //silence=26 ?
    //mecanics=9 ?
    if (apply)
    {
        ARCEMU_ASSERT(mod->m_miscValue < MECHANIC_END);
        m_target->MechanicsResistancesPCT[mod->m_miscValue] += mod->m_amount;

        if (mod->m_miscValue != MECHANIC_HEALING && mod->m_miscValue != MECHANIC_INVULNARABLE && mod->m_miscValue != MECHANIC_SHIELDED)  // don't remove bandages, Power Word and protection effect
        {
            SetPositive();
        }
        else
        {
            SetNegative();
        }
    }
    else
        m_target->MechanicsResistancesPCT[mod->m_miscValue] -= mod->m_amount;
}

void Aura::SpellAuraModHealingPCT(bool apply)
{
    int32 val;
    if (apply)
    {
        val = mod->m_amount;
        if (val < 0)
            SetNegative();
        else
            SetPositive();
    }
    else
        val = -mod->m_amount;

    for (uint8 x = 0; x < 7; x++)
    {
        if (mod->m_miscValue & (((uint32)1) << x))
        {
            m_target->HealTakenPctMod[x] += ((float)(val)) / 100;
        }
    }
}

void Aura::SpellAuraUntrackable(bool apply)
{
    if (apply)
        m_target->SetFlag(UNIT_FIELD_BYTES_1, UNIT_STAND_FLAGS_UNTRACKABLE);
    else
        m_target->RemoveFlag(UNIT_FIELD_BYTES_1, UNIT_STAND_FLAGS_UNTRACKABLE);
}

void Aura::SpellAuraModRangedAttackPower(bool apply)
{
    if (apply)
    {
        if (mod->m_amount > 0)
            SetPositive();
        else
            SetNegative();
        m_target->ModRangedAttackPowerMods(mod->m_amount);
    }
    else
        m_target->ModRangedAttackPowerMods(-mod->m_amount);
    m_target->CalcDamage();
}

void Aura::SpellAuraModMeleeDamageTaken(bool apply)
{
    if (apply)
    {
        if (mod->m_amount > 0)//does not exist but let it be
            SetNegative();
        else
            SetPositive();
        m_target->DamageTakenMod[0] += mod->m_amount;
    }
    else
        m_target->DamageTakenMod[0] -= mod->m_amount;
}

void Aura::SpellAuraModMeleeDamageTakenPct(bool apply)
{
    if (apply)
    {
        if (mod->m_amount > 0) //does not exist but let it be
            SetNegative();
        else
            SetPositive();
        m_target->DamageTakenPctMod[0] += mod->m_amount / 100.0f;
    }
    else
        m_target->DamageTakenPctMod[0] -= mod->m_amount / 100.0f;
}

void Aura::SpellAuraRAPAttackerBonus(bool apply)
{
    if (apply)
    {
        m_target->RAPvModifier += mod->m_amount;
    }
    else
        m_target->RAPvModifier -= mod->m_amount;
}

void Aura::SpellAuraModIncreaseSpeedAlways(bool apply)
{
    if (apply)
    {
        SetPositive();
        m_target->m_speedModifier += mod->m_amount;
    }
    else
        m_target->m_speedModifier -= mod->m_amount;

    m_target->UpdateSpeed();
}

void Aura::SpellAuraModIncreaseEnergyPerc(bool apply)
{
    SetPositive();

    uint16_t modValue = static_cast<uint16_t>(mod->m_miscValue);
    if (apply)
    {
        mod->fixed_amount[mod->m_effectIndex] = m_target->getPercentModUInt32Value(UNIT_FIELD_MAXPOWER1 + modValue, mod->m_amount);
        m_target->ModMaxPower(modValue, mod->fixed_amount[mod->m_effectIndex]);
        if (p_target != nullptr && mod->m_miscValue == POWER_TYPE_MANA)
            p_target->SetManaFromSpell(p_target->GetManaFromSpell() + mod->fixed_amount[mod->m_effectIndex]);
    }
    else
    {
        m_target->ModMaxPower(modValue, -mod->fixed_amount[mod->m_effectIndex]);
        if (p_target != nullptr && mod->m_miscValue == POWER_TYPE_MANA)
            p_target->SetManaFromSpell(p_target->GetManaFromSpell() - mod->fixed_amount[mod->m_effectIndex]);
    }
}

void Aura::SpellAuraModIncreaseHealthPerc(bool apply)
{
    SetPositive();
    if (apply)
    {
        mod->fixed_amount[mod->m_effectIndex] = m_target->getPercentModUInt32Value(UNIT_FIELD_MAXHEALTH, mod->m_amount);
        m_target->ModMaxHealth(mod->fixed_amount[mod->m_effectIndex]);
        if (p_target != nullptr)
            p_target->SetHealthFromSpell(p_target->GetHealthFromSpell() + mod->fixed_amount[mod->m_effectIndex]);
        //		else if (m_target->IsPet())
        //			TO< Pet* >(m_target)->SetHealthFromSpell(((Pet*)m_target)->GetHealthFromSpell() + mod->fixed_amount[mod->m_effectIndex]);
    }
    else
    {
        m_target->ModMaxHealth(-mod->fixed_amount[mod->m_effectIndex]);
        if (m_target->getUInt32Value(UNIT_FIELD_HEALTH) > m_target->getUInt32Value(UNIT_FIELD_MAXHEALTH))
            m_target->SetHealth(m_target->getUInt32Value(UNIT_FIELD_MAXHEALTH));
        if (p_target != nullptr)
            p_target->SetHealthFromSpell(static_cast<Player*>(m_target)->GetHealthFromSpell() - mod->fixed_amount[mod->m_effectIndex]);
        //		else if (m_target->IsPet())
        //			TO< Pet* >(m_target)->SetHealthFromSpell(((Pet*)m_target)->GetHealthFromSpell() - mod->fixed_amount[mod->m_effectIndex]);
    }
}

void Aura::SpellAuraModManaRegInterrupt(bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
            p_target->m_ModInterrMRegenPCT += mod->m_amount;
        else
            p_target->m_ModInterrMRegenPCT -= mod->m_amount;

        p_target->UpdateStats();
    }
}

void Aura::SpellAuraModTotalStatPerc(bool apply)
{
    int32 val;
    if (apply)
    {
        val = mod->m_amount;
    }
    else
        val = -mod->m_amount;

    if (mod->m_miscValue == -1) //all stats
    {
        if (p_target != nullptr)
        {
            for (uint8 x = 0; x < 5; x++)
            {
                if (mod->m_amount > 0)
                    p_target->TotalStatModPctPos[x] += val;
                else
                    p_target->TotalStatModPctNeg[x] -= val;
                p_target->CalcStat(x);
            }

            p_target->UpdateStats();
            p_target->UpdateChances();
        }
        else if (m_target->IsCreature())
        {
            for (uint8 x = 0; x < 5; x++)
            {
                static_cast< Creature* >(m_target)->TotalStatModPct[x] += val;
                static_cast< Creature* >(m_target)->CalcStat(x);
            }
        }
    }
    else
    {
        ARCEMU_ASSERT(mod->m_miscValue < 5);
        if (p_target != nullptr)
        {
            //druid hearth of the wild should add more features based on form
            switch (m_spellInfo->getId())
            {
                //SPELL_HASH_HEART_OF_THE_WILD
                case 17003:
                case 17004:
                case 17005:
                case 17006:
                case 24894:
                {
                    //we should remove effect first
                    p_target->EventTalentHearthOfWildChange(false);
                    //set new value
                    if (apply)
                        p_target->SetTalentHearthOfWildPCT(val);
                    else
                        p_target->SetTalentHearthOfWildPCT(0);   //this happens on a talent reset
                                                                 //reapply
                    p_target->EventTalentHearthOfWildChange(true);
                } break;
            }

            uint16_t modValue = static_cast<uint16_t>(mod->m_miscValue);

            if (mod->m_amount > 0)
                p_target->TotalStatModPctPos[modValue] += val;
            else
                p_target->TotalStatModPctNeg[modValue] -= val;

            p_target->CalcStat(modValue);
            p_target->UpdateStats();
            p_target->UpdateChances();
        }
        else if (m_target->IsCreature())
        {
            uint16_t modValue = static_cast<uint16_t>(mod->m_miscValue);

            static_cast< Creature* >(m_target)->TotalStatModPct[modValue] += val;
            static_cast< Creature* >(m_target)->CalcStat(modValue);
        }
    }
}

void Aura::SpellAuraModHaste(bool apply)
{
    //blade flurry - attack a nearby opponent
    switch (m_spellInfo->getId())
    {
        //SPELL_HASH_BLADE_FLURRY
        case 13877:
        case 22482:
        case 33735:
        case 44181:
        case 51211:
        case 65956:
        {
            if (apply)
                m_target->AddExtraStrikeTarget(GetSpellInfo(), 0);
            else
                m_target->RemoveExtraStrikeTarget(GetSpellInfo());
        } break;
        default:
            break;
    }

    if (mod->m_amount < 0)
        SetNegative();
    else
        SetPositive();

    if (p_target != nullptr)
    {
        if (apply)
        {
            p_target->ModAttackSpeed(mod->m_amount, MOD_MELEE);
        }
        else
        {
            p_target->ModAttackSpeed(-mod->m_amount, MOD_MELEE);
        }

        p_target->UpdateAttackSpeed();
    }
    else
    {
        if (apply)
        {
            mod->fixed_amount[mod->m_effectIndex] = m_target->getPercentModUInt32Value(UNIT_FIELD_BASEATTACKTIME, mod->m_amount);
            mod->fixed_amount[mod->m_effectIndex * 2] = m_target->getPercentModUInt32Value(UNIT_FIELD_BASEATTACKTIME + 1, mod->m_amount);

            if ((int32)m_target->getUInt32Value(UNIT_FIELD_BASEATTACKTIME) <= mod->fixed_amount[mod->m_effectIndex])
                mod->fixed_amount[mod->m_effectIndex] = m_target->getUInt32Value(UNIT_FIELD_BASEATTACKTIME);    //watch it, a negative timer might be bad ;)
            if ((int32)m_target->getUInt32Value(UNIT_FIELD_BASEATTACKTIME + 1) <= mod->fixed_amount[mod->m_effectIndex * 2])
                mod->fixed_amount[mod->m_effectIndex * 2] = m_target->getUInt32Value(UNIT_FIELD_BASEATTACKTIME + 1); //watch it, a negative timer might be bad ;)

            m_target->ModBaseAttackTime(MELEE, -mod->fixed_amount[mod->m_effectIndex]);
            m_target->ModBaseAttackTime(OFFHAND, -mod->fixed_amount[mod->m_effectIndex * 2]);

            if (m_target->IsCreature())
                static_cast< Creature* >(m_target)->m_speedFromHaste += mod->fixed_amount[mod->m_effectIndex];
        }
        else
        {
            m_target->ModBaseAttackTime(MELEE, mod->fixed_amount[mod->m_effectIndex]);
            m_target->ModBaseAttackTime(OFFHAND, mod->fixed_amount[mod->m_effectIndex * 2]);

            if (m_target->IsCreature())
                static_cast< Creature* >(m_target)->m_speedFromHaste -= mod->fixed_amount[mod->m_effectIndex];
        }
    }
}

void Aura::SpellAuraForceReaction(bool apply)
{
    std::map<uint32, uint32>::iterator itr;
    if (p_target == nullptr)
        return;

    if (apply)
    {
        itr = p_target->m_forcedReactions.find(mod->m_miscValue);
        if (itr != p_target->m_forcedReactions.end())
            itr->second = mod->m_amount;
        else
            p_target->m_forcedReactions.insert(std::make_pair(mod->m_miscValue, mod->m_amount));
    }
    else
        p_target->m_forcedReactions.erase(mod->m_miscValue);

    WorldPacket data(SMSG_SET_FORCED_REACTIONS, (8 * p_target->m_forcedReactions.size()) + 4);
    data << uint32(p_target->m_forcedReactions.size());
    for (itr = p_target->m_forcedReactions.begin(); itr != p_target->m_forcedReactions.end(); ++itr)
    {
        data << itr->first;
        data << itr->second;
    }

    p_target->GetSession()->SendPacket(&data);
}

void Aura::SpellAuraModRangedHaste(bool apply)
{
    if (mod->m_amount < 0)
        SetNegative();
    else
        SetPositive();

    if (p_target != nullptr)
    {
        //		int32 amount = mod->m_amount;
        //		if (GetSpellProto()->getId() == 6150)// Quick Shots
        //		{
        //			Unit* pCaster = GetUnitCaster();
        //			if (pCaster)
        //				spellModFlatIntValue(pCaster->SM_FSPELL_VALUE,&amount,0x100000);
        //		}

        if (apply)
            p_target->ModAttackSpeed(mod->m_amount, MOD_RANGED);
        else
            p_target->ModAttackSpeed(-mod->m_amount, MOD_RANGED);

        p_target->UpdateAttackSpeed();
    }
    else
    {
        if (apply)
        {
            mod->fixed_amount[mod->m_effectIndex] = m_target->getPercentModUInt32Value(UNIT_FIELD_RANGEDATTACKTIME, mod->m_amount);
            m_target->ModBaseAttackTime(RANGED, -mod->fixed_amount[mod->m_effectIndex]);
        }
        else m_target->ModBaseAttackTime(RANGED, mod->fixed_amount[mod->m_effectIndex]);
    }
}

void Aura::SpellAuraModRangedAmmoHaste(bool apply)
{
    SetPositive();
    if (p_target == nullptr)
        return;

    if (apply)
        p_target->ModAttackSpeed(mod->m_amount, MOD_RANGED);
    else
        p_target->ModAttackSpeed(-mod->m_amount, MOD_RANGED);

    p_target->UpdateAttackSpeed();
}

void Aura::SpellAuraModResistanceExclusive(bool apply)
{
    SpellAuraModResistance(apply);
}

void Aura::SpellAuraRetainComboPoints(bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            p_target->m_retainComboPoints = true;
        }
        else
        {
            p_target->m_retainComboPoints = false; //Let points to be consumed

            //Remove points if aura duration has expired, no combo points will be lost if there were some
            //except the ones that were generated by this spell
            if (GetTimeLeft() == 0)
                p_target->AddComboPoints(p_target->GetSelection(), static_cast<int8_t>(-mod->m_amount));
        }
    }
}

void Aura::SpellAuraResistPushback(bool apply)
{
    //DK:This is resist for spell casting delay
    //Only use on players for now

    if (p_target != nullptr)
    {
        int32 val = 0;
        if (apply)
        {
            val = mod->m_amount;
            SetPositive();
        }
        else
            val = -mod->m_amount;

        for (uint8 x = 0; x < SCHOOL_COUNT; x++)
        {
            if (mod->m_miscValue & (((uint32)1) << x))
            {
                p_target->SpellDelayResist[x] += val;
            }
        }
    }
}

void Aura::SpellAuraModShieldBlockPCT(bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            p_target->m_modblockabsorbvalue += (uint32)mod->m_amount;
        }
        else
        {
            p_target->m_modblockabsorbvalue -= (uint32)mod->m_amount;
        }
        p_target->UpdateStats();
    }
}

void Aura::SpellAuraTrackStealthed(bool apply)
{
    Unit* c = GetUnitCaster();
    if (c == nullptr)
        return;

    c->trackStealth = apply;
}

void Aura::SpellAuraModDetectedRange(bool apply)
{
    if (p_target == nullptr)
        return;
    if (apply)
    {
        SetPositive();
        p_target->DetectedRange += mod->m_amount;
    }
    else
    {
        p_target->DetectedRange -= mod->m_amount;
    }
}

void Aura::SpellAuraSplitDamageFlat(bool apply)
{
    if (m_target->m_damageSplitTarget)
    {
        delete m_target->m_damageSplitTarget;
        m_target->m_damageSplitTarget = nullptr;
    }

    if (apply)
    {
        DamageSplitTarget* ds = new DamageSplitTarget;
        ds->m_flatDamageSplit = mod->m_miscValue;
        ds->m_spellId = GetSpellInfo()->getId();
        ds->m_pctDamageSplit = 0;
        ds->damage_type = static_cast<uint8>(mod->m_type);
        ds->creator = (void*)this;
        ds->m_target = m_casterGuid;
        m_target->m_damageSplitTarget = ds;
        //		printf("registering dmg split %u, amount= %u \n",ds->m_spellId, mod->m_amount, mod->m_miscValue, mod->m_type);
    }
}

void Aura::SpellAuraModStealthLevel(bool apply)
{
    if (apply)
    {
        SetPositive();
        m_target->m_stealthLevel += mod->m_amount;
    }
    else
        m_target->m_stealthLevel -= mod->m_amount;
}

void Aura::SpellAuraModUnderwaterBreathing(bool apply)
{
    if (p_target != nullptr)
    {
        uint32 m_UnderwaterMaxTimeSaved = p_target->m_UnderwaterMaxTime;
        if (apply)
            p_target->m_UnderwaterMaxTime *= (1 + mod->m_amount / 100);
        else
            p_target->m_UnderwaterMaxTime /= (1 + mod->m_amount / 100);
        p_target->m_UnderwaterTime *= p_target->m_UnderwaterMaxTime / m_UnderwaterMaxTimeSaved;
    }
}

void Aura::SpellAuraSafeFall(bool apply)
{
    //FIXME:Find true flag
    if (p_target != nullptr)
    {
        if (apply)
        {
            p_target->m_safeFall += mod->m_amount;
        }
        else
        {
            p_target->m_safeFall -= mod->m_amount;
        }
    }
}

void Aura::SpellAuraModReputationAdjust(bool apply)
{
    /*SetPositive();
    bool updateclient = true;
    if (IsPassive())
    updateclient = false;	 // don't update client on passive

    if (m_target->GetTypeId()==TYPEID_PLAYER)
    {
    if (apply)
    TO< Player* >(m_target)->modPercAllReputation(mod->m_amount, updateclient);
    else
    TO< Player* >(m_target)->modPercAllReputation(-mod->m_amount, updateclient);
    }*/

    // This is _actually_ "Reputation gains increased by x%."
    // not increase all rep by x%.

    if (p_target != nullptr)
    {
        SetPositive();
        if (apply)
            p_target->pctReputationMod += mod->m_amount;
        else
            p_target->pctReputationMod -= mod->m_amount;
    }
}

void Aura::SpellAuraNoPVPCredit(bool apply)
{
    if (p_target == nullptr)
        return;

    if (apply)
        p_target->m_honorless++;
    else
        p_target->m_honorless--;
}

void Aura::SpellAuraModHealthRegInCombat(bool apply)
{
    // demon armor etc, they all seem to be 5 sec.
    if (apply)
    {
        sEventMgr.AddEvent(this, &Aura::EventPeriodicHeal1, uint32(mod->m_amount), EVENT_AURA_PERIODIC_HEALINCOMB, 5000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Aura::EventPeriodicBurn(uint32 amount, uint32 misc)
{
    Unit* m_caster = GetUnitCaster();

    if (!m_caster)
        return;

    if (m_target->isAlive() && m_caster->isAlive())
    {
        if (m_target->SchoolImmunityList[GetSpellInfo()->getSchool()])
            return;

        uint32 Amount = (uint32)std::min(amount, m_target->GetPower(static_cast<uint16_t>(misc)));
        uint32 newHealth = m_target->GetPower(static_cast<uint16_t>(misc)) - Amount;

        m_target->SendPeriodicAuraLog(m_target->GetNewGUID(), m_target->GetNewGUID(), m_spellInfo->getId(), m_spellInfo->getSchool(), newHealth, 0, 0, FLAG_PERIODIC_DAMAGE, false);
        m_caster->DealDamage(m_target, Amount, 0, 0, GetSpellInfo()->getId());
    }
}

void Aura::SpellAuraPowerBurn(bool apply)
{
    //0 mana,1 rage, 3 energy
    if (apply)
        sEventMgr.AddEvent(this, &Aura::EventPeriodicBurn, uint32(mod->m_amount), (uint32)mod->m_miscValue, EVENT_AURA_PERIODIC_BURN, GetSpellInfo()->getEffectAmplitude(mod->m_effectIndex), 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void Aura::SpellAuraModCritDmgPhysical(bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            p_target->m_modphyscritdmgPCT += (uint32)mod->m_amount;
        }
        else
        {
            p_target->m_modphyscritdmgPCT -= (uint32)mod->m_amount;
        }
    }
}


void Aura::SpellAuraWaterBreathing(bool apply)
{
    if (p_target != nullptr)
    {
        if (apply)
        {
            SetPositive();
            p_target->StopMirrorTimer(MIRROR_TYPE_BREATH);
            p_target->m_UnderwaterState = 0;
        }

        p_target->m_bUnlimitedBreath = apply;
    }
}

void Aura::SpellAuraAPAttackerBonus(bool apply)
{
    if (apply)
    {
        m_target->APvModifier += mod->m_amount;
    }
    else
        m_target->APvModifier -= mod->m_amount;
}


void Aura::SpellAuraModPAttackPower(bool apply)
{
    //!!probably there is a flag or something that will signal if randeg or melee attack power !!! (still missing)
    if (p_target != nullptr)
    {
        if (apply)
        {
            p_target->ModAttackPowerMultiplier((float)mod->m_amount / 100.0f);
        }
        else
            p_target->ModAttackPowerMultiplier(-(float)mod->m_amount / 100.0f);
        p_target->CalcDamage();
    }
}

void Aura::SpellAuraModRangedAttackPowerPct(bool apply)
{
    if (m_target->IsPlayer())
    {
        m_target->ModRangedAttackPowerMultiplier(((apply) ? 1 : -1) * (float)mod->m_amount / 100);
        m_target->CalcDamage();
    }
}

void Aura::SpellAuraIncreaseDamageTypePCT(bool apply)
{
    if (m_target->IsPlayer())
    {
        if (apply)
        {
            for (uint32 x = 0; x < 11; x++)
                if (mod->m_miscValue & (((uint32)1) << x))
                    static_cast< Player* >(m_target)->IncreaseDamageByTypePCT[x + 1] += ((float)(mod->m_amount)) / 100;
            if (mod->m_amount < 0)
                SetNegative();
            else
                SetPositive();
        }
        else
        {
            for (uint32 x = 0; x < 11; x++)
            {
                if (mod->m_miscValue & (((uint32)1) << x))
                    static_cast< Player* >(m_target)->IncreaseDamageByTypePCT[x + 1] -= ((float)(mod->m_amount)) / 100;
            }
        }
    }
}

void Aura::SpellAuraIncreaseCricticalTypePCT(bool apply)
{
    if (m_target->IsPlayer())
    {
        if (apply)
        {
            for (uint32 x = 0; x < 11; x++)
                if (mod->m_miscValue & (((uint32)1) << x))
                    static_cast< Player* >(m_target)->IncreaseCricticalByTypePCT[x + 1] += ((float)(mod->m_amount)) / 100;
            if (mod->m_amount < 0)
                SetNegative();
            else
                SetPositive();
        }
        else
        {
            for (uint32 x = 0; x < 11; x++)
            {
                if (mod->m_miscValue & (((uint32)1) << x))
                    static_cast< Player* >(m_target)->IncreaseCricticalByTypePCT[x + 1] -= ((float)(mod->m_amount)) / 100;
            }
        }
    }
}

void Aura::SpellAuraIncreasePartySpeed(bool apply)
{
    if (m_target->IsPlayer() && m_target->isAlive() && m_target->GetMount() == 0)
    {
        if (apply)
        {
            m_target->m_speedModifier += mod->m_amount;
        }
        else
        {
            m_target->m_speedModifier -= mod->m_amount;
        }
        m_target->UpdateSpeed();
    }
}

void Aura::SpellAuraIncreaseSpellDamageByAttribute(bool apply)
{
    int32 val;

    if (apply)
    {
        val = mod->m_amount;
        if (val < 0)
            SetNegative();
        else
            SetPositive();

        mod->fixed_amount[mod->m_effectIndex] = val; //we wish to have the same amount when we are removing the spell as when we were applying !
    }
    else
        val = -mod->fixed_amount[mod->m_effectIndex];

    uint16_t stat = 3;
    for (uint8_t i = 0; i < 3; i++)
    {
        //bit hacky but it will work with all currently available spells
        if (m_spellInfo->getEffectApplyAuraName(i) == SPELL_AURA_INCREASE_SPELL_HEALING_PCT)
        {
            if (m_spellInfo->getEffectMiscValue(i) < 5)
                stat = static_cast<uint16_t>(m_spellInfo->getEffectMiscValue(i));
            else
                return;
        }
    }

    if (m_target->IsPlayer())
    {
        for (uint8_t x = 1; x < 7; x++)
        {
            if (mod->m_miscValue & (((uint32)1) << x))
            {
                if (apply)
                {
                    mod->realamount = float2int32(((float)val / 100) * m_target->GetStat(stat));
                    p_target->ModPosDamageDoneMod(x, mod->realamount);
                }
                else
                    p_target->ModPosDamageDoneMod(x, -mod->realamount);
            }
        }
        p_target->UpdateChanceFields();
    }
}

void Aura::SpellAuraModSpellDamageByAP(bool apply)
{
    int32 val;

    if (apply)
    {
        //!! caster may log out before spell expires on target !
        Unit* pCaster = GetUnitCaster();
        if (pCaster == nullptr)
            return;

        val = mod->m_amount * pCaster->GetAP() / 100;
        if (val < 0)
            SetNegative();
        else
            SetPositive();

        mod->fixed_amount[mod->m_effectIndex] = val; //we wish to have the same amount when we are removing the spell as when we were applying !
    }
    else
        val = -mod->fixed_amount[mod->m_effectIndex];

    if (m_target->IsPlayer())
    {
        for (uint16_t x = 1; x < 7; x++) //melee damage != spell damage.
            if (mod->m_miscValue & (((uint32)1) << x))
                p_target->ModPosDamageDoneMod(x, val);

        p_target->UpdateChanceFields();
    }
}

void Aura::SpellAuraIncreaseHealingByAttribute(bool apply)
{
    int32 val;

    if (apply)
    {
        val = mod->m_amount;

        if (val < 0)
            SetNegative();
        else
            SetPositive();

        mod->fixed_amount[mod->m_effectIndex] = val; //we wish to have the same amount when we are removing the spell as when we were applying !
    }
    else
        val = -mod->fixed_amount[mod->m_effectIndex];

    uint16_t stat;
    if (mod->m_miscValue < 5)
        stat = static_cast<uint16_t>(mod->m_miscValue);
    else
    {
        LOG_ERROR("Aura::SpellAuraIncreaseHealingByAttribute::Unknown spell attribute type %u in spell %u.\n", mod->m_miscValue, GetSpellId());
        return;
    }

    if (p_target != nullptr)
    {
        for (uint8_t x = 0; x < SCHOOL_COUNT; x++)
        {
            p_target->SpellHealDoneByAttribute[stat][x] += (float)val / 100.0f;
        }
        p_target->UpdateChanceFields();
        if (apply)
        {
            mod->realamount = float2int32(((float)val / 100.0f) * p_target->GetStat(stat));
            p_target->ModHealingDoneMod(mod->realamount);
        }
        else
            p_target->ModHealingDoneMod(-mod->realamount);
    }
}

void Aura::SpellAuraModHealingByAP(bool apply)
{
    int32 val;

    if (apply)
    {
        //!! caster may log out before spell expires on target !
        Unit* pCaster = GetUnitCaster();
        if (pCaster == nullptr)
            return;

        val = mod->m_amount * pCaster->GetAP() / 100;
        if (val < 0)
            SetNegative();
        else
            SetPositive();

        mod->fixed_amount[mod->m_effectIndex] = val; //we wish to have the same amount when we are removing the spell as when we were applying !
    }
    else
        val = -mod->fixed_amount[mod->m_effectIndex];



    for (uint8 x = 0; x < SCHOOL_COUNT; x++)
    {
        if (mod->m_miscValue  & (((uint32)1) << x))
        {
            m_target->HealDoneMod[x] += val;
        }
    }

    if (p_target != nullptr)
    {
        p_target->ModHealingDoneMod(val);
        p_target->UpdateChanceFields();
    }
}

void Aura::SpellAuraAddFlatModifier(bool apply)
{
    int32 val = apply ? mod->m_amount : -mod->m_amount;
#if VERSION_STRING != Cata
    uint32* AffectedGroups = GetSpellInfo()->getEffectSpellClassMask(mod->m_effectIndex);
#else
    uint32* AffectedGroups = GetSpellInfo()->EffectSpellClassMask;
#endif

    switch (mod->m_miscValue) //let's generate warnings for unknown types of modifiers
    {
        case SMT_DAMAGE_DONE:
            SendModifierLog(&m_target->SM_FDamageBonus, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_DURATION:
            SendModifierLog(&m_target->SM_FDur, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_THREAT_REDUCED:
            SendModifierLog(&m_target->SM_FThreat, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_EFFECT_1:
            SendModifierLog(&m_target->SM_FEffect1_Bonus, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_CHARGES:
            SendModifierLog(&m_target->SM_FCharges, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_RANGE:
            SendModifierLog(&m_target->SM_FRange, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_RADIUS:
            SendModifierLog(&m_target->SM_FRadius, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_CRITICAL:
            SendModifierLog(&m_target->SM_CriticalChance, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_MISC_EFFECT:
            SendModifierLog(&m_target->SM_FMiscEffect, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

            //case SMT_NONINTERRUPT: - no flat

        case SMT_CAST_TIME:
            SendModifierLog(&m_target->SM_FCastTime, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_COOLDOWN_DECREASE:
            SendModifierLog(&m_target->SM_FCooldownTime, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_EFFECT_2:
            SendModifierLog(&m_target->SM_FEffect2_Bonus, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_COST:
            SendModifierLog(&m_target->SM_FCost, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

            //case SMT_CRITICAL_DAMAGE: - no flat

        case SMT_HITCHANCE:
            SendModifierLog(&m_target->SM_FHitchance, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_ADDITIONAL_TARGET:
            SendModifierLog(&m_target->SM_FAdditionalTargets, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_TRIGGER:
            SendModifierLog(&m_target->SM_FChanceOfSuccess, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_AMPTITUDE:
            SendModifierLog(&m_target->SM_FAmptitude, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

            //case SMT_JUMP_REDUCE: - no flat

        case SMT_GLOBAL_COOLDOWN:
            SendModifierLog(&m_target->SM_FGlobalCooldown, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

            //case SMT_SPELL_VALUE_PCT: - pct only?
            //SendModifierLog(&m_target->SM_FDOT,val,AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            //break;

        case SMT_EFFECT_3:
            SendModifierLog(&m_target->SM_FEffect3_Bonus, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_PENALTY:
            SendModifierLog(&m_target->SM_FPenalty, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        case SMT_EFFECT_BONUS:
            SendModifierLog(&m_target->SM_FEffectBonus, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue), true);
            break;

        case SMT_RESIST_DISPEL:
            SendModifierLog(&m_target->SM_FRezist_dispell, val, AffectedGroups, static_cast<uint8>(mod->m_miscValue));
            break;

        default: //Unknown modifier type
            LOG_ERROR( "Unknown spell modifier type %u in spell %u.<<--report this line to the developer\n", mod->m_miscValue, GetSpellId());
            break;
    }

    //Hunter's BeastMastery talents.
    if (m_target->IsPlayer())
    {
        Pet* p = static_cast< Player* >(m_target)->GetSummon();
        if (p)
        {
            switch (GetSpellInfo()->getId())
            {
                // SPELL_HASH_UNLEASHED_FURY:
                case 19616:
                case 19617:
                case 19618:
                case 19619:
                case 19620:
                    p->LoadPetAuras(0);
                    break;
                // SPELL_HASH_THICK_HIDE:
                case 16929:
                case 16930:
                case 16931:
                case 19609:
                case 19610:
                case 19612:
                case 50502:
                    p->LoadPetAuras(1);
                    break;
                // SPELL_HASH_ENDURANCE_TRAINING:
                case 19583:
                case 19584:
                case 19585:
                case 19586:
                case 19587:
                    p->LoadPetAuras(2);
                    break;
                // SPELL_HASH_FERAL_SWIFTNESS:
                case 17002:
                case 24866:
                    p->LoadPetAuras(3);
                    break;
                // SPELL_HASH_BESTIAL_DISCIPLINE:
                case 19590:
                case 19592:
                    p->LoadPetAuras(4);
                    break;
                // SPELL_HASH_FEROCITY:
                case 4154:
                case 16934:
                case 16935:
                case 16936:
                case 16937:
                case 16938:
                case 19598:
                case 19599:
                case 19600:
                case 19601:
                case 19602:
                case 33667:
                    p->LoadPetAuras(5);
                    break;
                // SPELL_HASH_ANIMAL_HANDLER:
                case 34453:
                case 34454:
                case 68361:
                    p->LoadPetAuras(6);
                    break;
                // SPELL_HASH_CATLIKE_REFLEXES:
                case 34462:
                case 34464:
                case 34465:
                    p->LoadPetAuras(7);
                    break;
                // SPELL_HASH_SERPENT_S_SWIFTNESS:
                case 34466:
                case 34467:
                case 34468:
                case 34469:
                case 34470:
                    p->LoadPetAuras(8);
                    break;
            }
        }
    }
}

void Aura::SpellAuraModHealingDone(bool apply)
{
    int32 val;
    if (apply)
    {
        val = mod->m_amount;
        if (val < 0)
            SetNegative();
        else
            SetPositive();
    }
    else
        val = -mod->m_amount;

    uint32 player_class = m_target->getClass();
    if (player_class == DRUID || player_class == PALADIN || player_class == SHAMAN || player_class == PRIEST)
        val = float2int32(val * 1.88f);

    for (uint8 x = 0; x < SCHOOL_COUNT; x++)
    {
        if (mod->m_miscValue  & (((uint32)1) << x))
        {
            m_target->HealDoneMod[x] += val;
        }
    }
    if (p_target != nullptr)
    {
        p_target->UpdateChanceFields();
        p_target->ModHealingDoneMod(val);
    }
}

void Aura::SpellAuraModHealingDonePct(bool apply)
{
    int32 val;
    if (apply)
    {
        val = mod->m_amount;
        if (val < 0)
            SetNegative();
        else
            SetPositive();
    }
    else
        val = -mod->m_amount;

    for (uint32 x = 0; x < 7; x++)
    {
        if (mod->m_miscValue  & (((uint32)1) << x))
        {
            m_target->HealDonePctMod[x] += ((float)(val)) / 100;
        }
    }
}

void Aura::SpellAuraEmphaty(bool apply)
{
    SetPositive();
    Player* caster = GetPlayerCaster();
    if (caster == nullptr)
        return;

    // Show extra info about beast
    uint32 dynflags = m_target->getUInt32Value(UNIT_DYNAMIC_FLAGS);
    if (apply)
        dynflags |= U_DYN_FLAG_PLAYER_INFO;

    m_target->BuildFieldUpdatePacket(caster, UNIT_DYNAMIC_FLAGS, dynflags);
}

void Aura::SpellAuraModOffhandDamagePCT(bool apply)
{
    //Used only by talents of rogue and warrior;passive,positive
    if (p_target != nullptr)
    {
        if (apply)
        {
            SetPositive();
            p_target->offhand_dmg_mod *= (100 + mod->m_amount) / 100.0f;
        }
        else
            p_target->offhand_dmg_mod /= (100 + mod->m_amount) / 100.0f;

        p_target->CalcDamage();
    }
}

void Aura::SpellAuraModPenetration(bool apply) // armor penetration & spell penetration
{
    //SPELL_HASH_SERRATED_BLADES
    if (m_spellInfo->getId() == 14171 || m_spellInfo->getId() == 14172 || m_spellInfo->getId() == 14173)
    {
        if (p_target == nullptr)
            return;

        if (apply)
        {
            if (m_spellInfo->getId() == 14171)
                p_target->PowerCostPctMod[0] += m_target->getLevel() * 2.67f;
            else if (m_spellInfo->getId() == 14172)
                p_target->PowerCostPctMod[0] += m_target->getLevel() * 5.43f;
            else if (m_spellInfo->getId() == 14173)
                p_target->PowerCostPctMod[0] += m_target->getLevel() * 8.0f;
        }
        else
        {
            if (m_spellInfo->getId() == 14171)
                p_target->PowerCostPctMod[0] -= m_target->getLevel() * 2.67f;
            else if (m_spellInfo->getId() == 14172)
                p_target->PowerCostPctMod[0] -= m_target->getLevel() * 5.43f;
            else if (m_spellInfo->getId() == 14173)
                p_target->PowerCostPctMod[0] -= m_target->getLevel() * 8.0f;
        }
        return;
    }

    if (apply)
    {
        if (mod->m_amount < 0)
            SetPositive();
        else
            SetNegative();

        for (uint8 x = 0; x < 7; x++)
        {
            if (mod->m_miscValue & (((uint32)1) << x))
                m_target->PowerCostPctMod[x] -= mod->m_amount;
        }

        if (p_target != nullptr)
        {
#if VERSION_STRING != Classic
            if (mod->m_miscValue & 124)
                m_target->modInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE, mod->m_amount);
            if (mod->m_miscValue & 1)
                m_target->modInt32Value(PLAYER_FIELD_MOD_TARGET_PHYSICAL_RESISTANCE, mod->m_amount);
#endif
        }
    }
    else
    {
        for (uint8 x = 0; x < 7; x++)
        {
            if (mod->m_miscValue & (((uint32)1) << x))
                m_target->PowerCostPctMod[x] += mod->m_amount;
        }
        if (p_target != nullptr)
        {
#if VERSION_STRING != Classic
            if (mod->m_miscValue & 124)
                m_target->modInt32Value(PLAYER_FIELD_MOD_TARGET_RESISTANCE, -mod->m_amount);
            if (mod->m_miscValue & 1)
                m_target->modInt32Value(PLAYER_FIELD_MOD_TARGET_PHYSICAL_RESISTANCE, -mod->m_amount);
#endif
        }
    }
}

void Aura::SpellAuraIncreaseArmorByPctInt(bool apply)
{
    uint32 i_Int = m_target->GetStat(STAT_INTELLECT);

    int32 amt = float2int32(i_Int * ((float)mod->m_amount / 100.0f));
    amt *= (!apply) ? -1 : 1;

    for (uint16 x = 0; x < 7; x++)
    {
        if (mod->m_miscValue & (((uint32)1) << x))
        {
            if (p_target != nullptr)
            {
                p_target->FlatResistanceModifierPos[x] += amt;
                p_target->CalcResistance(x);
            }
            else if (m_target->IsCreature())
            {
                static_cast< Creature* >(m_target)->FlatResistanceMod[x] += amt;
                static_cast< Creature* >(m_target)->CalcResistance(x);
            }
        }
    }
}

void Aura::SpellAuraReduceAttackerMHitChance(bool apply)
{
    if (p_target == nullptr)
        return;
    if (apply)
        p_target->m_resist_hit[MOD_MELEE] += mod->m_amount;
    else
        p_target->m_resist_hit[MOD_MELEE] -= mod->m_amount;
}

void Aura::SpellAuraReduceAttackerRHitChance(bool apply)
{
    if (p_target == nullptr)
        return;
    if (apply)
        p_target->m_resist_hit[MOD_RANGED] += mod->m_amount;
    else
        p_target->m_resist_hit[MOD_RANGED] -= mod->m_amount;
}

void Aura::SpellAuraReduceAttackerSHitChance(bool apply)
{
    if (p_target == nullptr)
        return;
    for (uint8 i = 0; i < SCHOOL_COUNT; i++)
    {
        if (mod->m_miscValue & (1 << i))     // check school
        {
            // signs reversed intentionally
            if (apply)
                p_target->m_resist_hit_spell[i] -= mod->m_amount;
            else
                p_target->m_resist_hit_spell[i] += mod->m_amount;
        }
    }
}

void Aura::SpellAuraReduceEnemyMCritChance(bool apply)
{
    if (!m_target->IsPlayer())
        return;
    if (apply)
    {
        //value is negative percent
        static_cast< Player* >(m_target)->res_M_crit_set(static_cast< Player* >(m_target)->res_M_crit_get() + mod->m_amount);
    }
    else
    {
        static_cast< Player* >(m_target)->res_M_crit_set(static_cast< Player* >(m_target)->res_M_crit_get() - mod->m_amount);
    }
}

void Aura::SpellAuraReduceEnemyRCritChance(bool apply)
{
    if (!m_target->IsPlayer())
        return;
    if (apply)
    {
        //value is negative percent
        static_cast< Player* >(m_target)->res_R_crit_set(static_cast< Player* >(m_target)->res_R_crit_get() + mod->m_amount);
    }
    else
    {
        static_cast< Player* >(m_target)->res_R_crit_set(static_cast< Player* >(m_target)->res_R_crit_get() - mod->m_amount);
    }
}

void Aura::SpellAuraLimitSpeed(bool apply)
{
    int32 amount = (apply) ? mod->m_amount : -mod->m_amount;
    m_target->m_maxSpeed += (float)amount;
    m_target->UpdateSpeed();
}
void Aura::SpellAuraIncreaseTimeBetweenAttacksPCT(bool apply)
{
    int32 val = (apply) ? mod->m_amount : -mod->m_amount;
    float pct_value = -val / 100.0f;
    m_target->ModCastSpeedMod(pct_value);
}

void Aura::SpellAuraMeleeHaste(bool apply)
{
    if (mod->m_amount < 0)
        SetNegative();
    else
        SetPositive();

    if (m_target->IsPlayer())
    {
        if (apply)
            static_cast< Player* >(m_target)->ModAttackSpeed(mod->m_amount, MOD_MELEE);
        else
            static_cast< Player* >(m_target)->ModAttackSpeed(-mod->m_amount, MOD_MELEE);

        static_cast< Player* >(m_target)->UpdateAttackSpeed();
    }
    else
    {
        if (apply)
        {
            mod->fixed_amount[0] = m_target->getPercentModUInt32Value(UNIT_FIELD_BASEATTACKTIME, mod->m_amount);
            mod->fixed_amount[1] = m_target->getPercentModUInt32Value(UNIT_FIELD_BASEATTACKTIME + 1, mod->m_amount);

            if ((int32)m_target->getUInt32Value(UNIT_FIELD_BASEATTACKTIME) <= mod->fixed_amount[0])
                mod->fixed_amount[0] = m_target->getUInt32Value(UNIT_FIELD_BASEATTACKTIME);
            if ((int32)m_target->getUInt32Value(UNIT_FIELD_BASEATTACKTIME + 1) <= mod->fixed_amount[1])
                mod->fixed_amount[1] = m_target->getUInt32Value(UNIT_FIELD_BASEATTACKTIME + 1);

            m_target->ModBaseAttackTime(MELEE, -mod->fixed_amount[0]);
            m_target->ModBaseAttackTime(OFFHAND, -mod->fixed_amount[1]);
        }
        else
        {
            m_target->ModBaseAttackTime(MELEE, mod->fixed_amount[0]);
            m_target->ModBaseAttackTime(OFFHAND, mod->fixed_amount[1]);
        }
    }
}

/*
void Aura::SpellAuraIncreaseSpellDamageByInt(bool apply)
{
float val;
if (apply)
{
val = mod->m_amount/100.0f;
if (mod->m_amount>0)
SetPositive();
else
SetNegative();
}
else
val =- mod->m_amount/100.0f;

if (m_target->IsPlayer())
{
for (uint32 x=1;x<7;x++)
{
if (mod->m_miscValue & (((uint32)1)<<x))
{
TO< Player* >(m_target)->SpellDmgDoneByInt[x]+=val;
}
}
}
}

void Aura::SpellAuraIncreaseHealingByInt(bool apply)
{
float val;
if (apply)
{
val = mod->m_amount/100.0f;
if (val>0)
SetPositive();
else
SetNegative();
}
else
val =- mod->m_amount/100.0f;

if (m_target->IsPlayer())
{
for (uint32 x=1;x<7;x++)
{
//		 if (mod->m_miscValue & (((uint32)1)<<x))
{
TO< Player* >(m_target)->SpellHealDoneByInt[x]+=val;
}
}
}
}
*/
void Aura::SpellAuraModAttackerCritChance(bool apply)
{
    int32 val = (apply) ? mod->m_amount : -mod->m_amount;
    m_target->AttackerCritChanceMod[0] += val;
}

void Aura::SpellAuraIncreaseAllWeaponSkill(bool apply)
{
    if (m_target->IsPlayer())
    {
        if (apply)
        {
            SetPositive();
            //			TO< Player* >(m_target)->ModSkillBonusType(SKILL_TYPE_WEAPON, mod->m_amount);
            //since the frikkin above line does not work we have to do it manually
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_SWORDS, mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_AXES, mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_BOWS, mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_GUNS, mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_MACES, mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_2H_SWORDS, mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_STAVES, mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_2H_MACES, mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_2H_AXES, mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_DAGGERS, mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_CROSSBOWS, mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_WANDS, mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_POLEARMS, mod->m_amount);
        }
        else
        {
            //			TO< Player* >(m_target)->ModSkillBonusType(SKILL_TYPE_WEAPON, -mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_SWORDS, -mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_AXES, -mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_BOWS, -mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_GUNS, -mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_MACES, -mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_2H_SWORDS, -mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_STAVES, -mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_2H_MACES, -mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_2H_AXES, -mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_DAGGERS, -mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_CROSSBOWS, -mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_WANDS, -mod->m_amount);
            static_cast< Player* >(m_target)->_ModifySkillBonus(SKILL_POLEARMS, -mod->m_amount);
        }

        static_cast< Player* >(m_target)->UpdateStats();
    }
}

void Aura::SpellAuraIncreaseHitRate(bool apply)
{
    if (!m_target->IsPlayer())
        return;

    static_cast< Player* >(m_target)->ModifyBonuses(SPELL_HIT_RATING, mod->m_amount, apply);
    static_cast< Player* >(m_target)->UpdateStats();
}

void Aura::SpellAuraIncreaseRageFromDamageDealtPCT(bool apply)
{
    if (!m_target->IsPlayer())
        return;

    static_cast< Player* >(m_target)->rageFromDamageDealt += (apply) ? mod->m_amount : -mod->m_amount;
}

int32 Aura::event_GetInstanceID()
{
    return m_target->event_GetInstanceID();
}

void Aura::RelocateEvents()
{
    event_Relocate();
}

void Aura::SpellAuraReduceCritMeleeAttackDmg(bool apply)
{
    signed int val;
    if (apply)
        val = mod->m_amount;
    else
        val = -mod->m_amount;

    for (uint32 x = 1; x < 7; x++)
        if (mod->m_miscValue & (((uint32)1) << x))
            m_target->CritMeleeDamageTakenPctMod[x] += val / 100.0f;
}

void Aura::SpellAuraReduceCritRangedAttackDmg(bool apply)
{
    signed int val;
    if (apply)
        val = mod->m_amount;
    else
        val = -mod->m_amount;

    for (uint32 x = 1; x < 7; x++)
        if (mod->m_miscValue & (((uint32)1) << x))
            m_target->CritRangedDamageTakenPctMod[x] += val / 100.0f;
}

void Aura::SpellAuraEnableFlight(bool apply)
{
    if (apply)
    {
        m_target->setMoveCanFly(true);
        m_target->m_flyspeedModifier += mod->m_amount;
        m_target->UpdateSpeed();
        if (m_target->IsPlayer())
        {
            static_cast< Player* >(m_target)->flying_aura = m_spellInfo->getId();
        }
    }
    else
    {
        m_target->setMoveCanFly(false);
        m_target->m_flyspeedModifier -= mod->m_amount;
        m_target->UpdateSpeed();
        if (m_target->IsPlayer())
        {
            static_cast< Player* >(m_target)->flying_aura = 0;
        }
    }
}

void Aura::SpellAuraEnableFlightWithUnmountedSpeed(bool apply)
{
    // Used in flight form (only so far)
    if (apply)
    {
        m_target->setMoveCanFly(true);
        m_target->m_flyspeedModifier += mod->m_amount;
        m_target->UpdateSpeed();
        if (m_target->IsPlayer())
        {
            static_cast< Player* >(m_target)->flying_aura = m_spellInfo->getId();
        }
    }
    else
    {
        m_target->setMoveCanFly(false);
        m_target->m_flyspeedModifier -= mod->m_amount;
        m_target->UpdateSpeed();
        if (m_target->IsPlayer())
        {
            static_cast< Player* >(m_target)->flying_aura = 0;
        }
    }
}

void Aura::SpellAuraIncreaseMovementAndMountedSpeed(bool apply)
{
    if (apply)
        m_target->m_mountedspeedModifier += mod->m_amount;
    else
        m_target->m_mountedspeedModifier -= mod->m_amount;
    m_target->UpdateSpeed();
}

void Aura::SpellAuraIncreaseFlightSpeed(bool apply)
{
    if (apply)
        m_target->m_flyspeedModifier += mod->m_amount;
    else
        m_target->m_flyspeedModifier -= mod->m_amount;
    m_target->UpdateSpeed();
}


void Aura::SpellAuraIncreaseRating(bool apply)
{
    int v = apply ? mod->m_amount : -mod->m_amount;

    if (!m_target->IsPlayer())
        return;

    Player* plr = static_cast< Player* >(m_target);
    for (uint32 x = 1; x < 24; x++)  //skip x= 0
        if ((((uint32)1) << x) & mod->m_miscValue)
            plr->ModifyBonuses(11 + x, mod->m_amount, apply);

    //MELEE_CRITICAL_AVOIDANCE_RATING + RANGED_CRITICAL_AVOIDANCE_RATING + SPELL_CRITICAL_AVOIDANCE_RATING
    //comes only as combination of them  - ModifyBonuses() not adding them individually anyhow
    if (mod->m_miscValue & (0x0004000 | 0x0008000 | 0x0010000))
        plr->ModifyBonuses(RESILIENCE_RATING, mod->m_amount, apply);

    if (mod->m_miscValue & 1)  //weapon skill
    {
        std::map<uint32, uint32>::iterator i;
        for (uint32 y = 0; y < 20; y++)
            if (m_spellInfo->getEquippedItemSubClass() & (((uint32)1) << y))
            {
                i = static_cast< Player* >(m_target)->m_wratings.find(y);
                if (i == static_cast< Player* >(m_target)->m_wratings.end())    //no prev
                {
                    static_cast< Player* >(m_target)->m_wratings[y] = v;
                }
                else
                {
                    i->second += v;
                    if (i->second == 0)
                        static_cast< Player* >(m_target)->m_wratings.erase(i);
                }
            }
    }

    plr->UpdateStats();
}

void Aura::EventPeriodicRegenManaStatPct(uint32 perc, uint32 stat)
{
    if (m_target->IsDead())
        return;

    uint32 mana = m_target->GetPower(POWER_TYPE_MANA) + (m_target->GetStat(static_cast<uint16_t>(stat)) * perc) / 100;

    if (mana <= m_target->GetMaxPower(POWER_TYPE_MANA))
        m_target->SetPower(POWER_TYPE_MANA, mana);
    else
        m_target->SetPower(POWER_TYPE_MANA, m_target->GetMaxPower(POWER_TYPE_MANA));
}

void Aura::SpellAuraRegenManaStatPCT(bool apply)
{
    if (apply)
    {
        SetPositive();
        sEventMgr.AddEvent(this, &Aura::EventPeriodicRegenManaStatPct, (uint32)mod->m_amount, (uint32)mod->m_miscValue, EVENT_AURA_REGEN_MANA_STAT_PCT, 5000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
}

void Aura::SpellAuraSpellHealingStatPCT(bool apply)
{
    if (!m_target->IsPlayer())
        return;

    if (apply)
    {
        //SetPositive();
        /*mod->realamount = (mod->m_amount * m_target->GetStat(mod->m_miscValue) /1 00;

        for (uint32 x = 1; x < 7; x++)
        m_target->HealDoneMod[x] += mod->realamount;*/

        mod->realamount = ((m_target->getUInt32Value(UNIT_FIELD_STAT4) * mod->m_amount) / 100);

        static_cast<Player*>(m_target)->ModifyBonuses(CRITICAL_STRIKE_RATING, mod->realamount, true);
        static_cast<Player*>(m_target)->UpdateChances();
    }
    else
    {
        /*for (uint32 x = 1; x < 7; x++)
            m_target->HealDoneMod[x] -= mod->realamount;*/

        static_cast<Player*>(m_target)->ModifyBonuses(CRITICAL_STRIKE_RATING, mod->realamount, false);
        static_cast<Player*>(m_target)->UpdateChances();
    }
}

void Aura::SpellAuraAllowFlight(bool apply)
{
    m_target->setMoveCanFly(apply);
}

void Aura::SpellAuraFinishingMovesCannotBeDodged(bool apply)
{
    if (apply)
    {
        if (!m_target->IsPlayer())
            return;

        static_cast< Player* >(m_target)->m_finishingmovesdodge = true;
    }
    else
    {
        if (!m_target->IsPlayer())
            return;

        static_cast< Player* >(m_target)->m_finishingmovesdodge = false;
    }
}

void Aura::SpellAuraModStealthDetection(bool apply)
{
    if (apply)
    {
        m_target->m_stealthDetectBonus += 9999;
    }
    else
        m_target->m_stealthDetectBonus -= 9999;
}

void Aura::SpellAuraReduceAOEDamageTaken(bool apply)
{
    float val = mod->m_amount / 100.0f;
    if (apply)
    {
        mod->fixed_amount[0] = (int)(m_target->AOEDmgMod * val);
        m_target->AOEDmgMod += mod->fixed_amount[0];
    }
    else
        m_target->AOEDmgMod -= mod->fixed_amount[0];
}

void Aura::SpellAuraIncreaseMaxHealth(bool apply)
{
    //should only be used by a player
    //and only ever target players
    if (!m_target->IsPlayer())
        return;

    int32 amount;
    if (apply)
        amount = mod->m_amount;
    else
        amount = -mod->m_amount;

    static_cast< Player* >(m_target)->SetHealthFromSpell(static_cast< Player* >(m_target)->GetHealthFromSpell() + amount);
    static_cast< Player* >(m_target)->UpdateStats();
}

void Aura::SpellAuraSpiritOfRedemption(bool apply)
{
    if (!m_target->IsPlayer())
        return;

    if (apply)
    {
        m_target->SetScale(0.5);
        m_target->SetHealth(1);
        SpellInfo* sorInfo = sSpellCustomizations.GetSpellInfo(27792);
        Spell* sor = sSpellFactoryMgr.NewSpell(m_target, sorInfo, true, nullptr);
        SpellCastTargets spellTargets;
        spellTargets.m_unitTarget = m_target->GetGUID();
        sor->prepare(&spellTargets);
    }
    else
    {
        m_target->SetScale(1);
        m_target->RemoveAura(27792);
        m_target->SetHealth(0);
    }
}

void Aura::SpellAuraIncreaseAttackerSpellCrit(bool apply)
{
    int32 val = mod->m_amount;

    if (apply)
    {
        if (mod->m_amount > 0)
            SetNegative();
        else
            SetPositive();
    }
    else
        val = -val;

    for (uint32 x = 0; x < 7; x++)
    {
        if (mod->m_miscValue & (((uint32)1) << x))
            m_target->AttackerCritChanceMod[x] += val;
    }
}

void Aura::SpellAuraIncreaseRepGainPct(bool apply)
{
    if (p_target)
    {
        SetPositive();
        if (apply)
            p_target->pctReputationMod += mod->m_amount;//re use
        else
            p_target->pctReputationMod -= mod->m_amount;//re use
    }
}

void Aura::SpellAuraIncreaseRAPbyStatPct(bool apply)
{
    if (apply)
    {
        if (mod->m_amount > 0)
            SetPositive();
        else
            SetNegative();

        uint16_t modValue = static_cast<uint16_t>(mod->m_miscValue);
        mod->fixed_amount[mod->m_effectIndex] = m_target->GetStat(modValue) * mod->m_amount / 100;
        m_target->ModRangedAttackPowerMods(mod->fixed_amount[mod->m_effectIndex]);
    }
    else
        m_target->ModRangedAttackPowerMods(-mod->fixed_amount[mod->m_effectIndex]);

    m_target->CalcDamage();
}

/* not used
void Aura::SpellAuraModRangedDamageTakenPCT(bool apply)
{
if (apply)
m_target->RangedDamageTakenPct += mod->m_amount;
else
m_target->RangedDamageTakenPct -= mod->m_amount;
}*/

void Aura::SpellAuraModBlockValue(bool apply)
{
    if (p_target != nullptr)
    {
        int32 amt;
        if (apply)
        {
            amt = mod->m_amount;
            if (amt < 0)
                SetNegative();
            else
                SetPositive();
        }
        else
        {
            amt = -mod->m_amount;
        }
        p_target->m_modblockvaluefromspells += amt;
        p_target->UpdateStats();
    }
}

void Aura::SendInterrupted(uint8 result, Object* m_caster)
{
    if (!m_caster->IsInWorld())
        return;

    WorldPacket data(SMSG_SPELL_FAILURE, 20);
    if (m_caster->IsPlayer())
    {
        data << m_caster->GetNewGUID();
        data << m_spellInfo->getId();
        data << uint8(result);
        static_cast< Player* >(m_caster)->GetSession()->SendPacket(&data);
    }

    data.Initialize(SMSG_SPELL_FAILED_OTHER);
    data << m_caster->GetNewGUID();
    data << m_spellInfo->getId();
    m_caster->SendMessageToSet(&data, false);

    m_interrupted = (int16)result;
}

void Aura::SendChannelUpdate(uint32 time, Object* m_caster)
{
    WorldPacket data(MSG_CHANNEL_UPDATE, 18);
    data << m_caster->GetNewGUID();
    data << time;

    m_caster->SendMessageToSet(&data, true);
}

void Aura::SpellAuraExpertise(bool /*apply*/)
{
    if (p_target == nullptr)
        return;

    p_target->CalcExpertise();
}

void Aura::SpellAuraForceMoveForward(bool apply)
{
#if VERSION_STRING != Classic
    if (apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVE);
    else
        m_target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FORCE_MOVE);
#endif
}

void Aura::SpellAuraComprehendLang(bool apply)
{
#if VERSION_STRING != Classic
    if (apply)
        m_target->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_COMPREHEND_LANG);
    else
        m_target->RemoveFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_COMPREHEND_LANG);
#endif
}

void Aura::SpellAuraModPossessPet(bool apply)
{
    Player* pCaster = GetPlayerCaster();
    if (pCaster == nullptr || !pCaster->IsInWorld())
        return;

    if (!m_target->IsPet())
        return;

    std::list<Pet*> summons = pCaster->GetSummons();
    for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
    {
        if (*itr == m_target)
        {
            if (apply)
            {
                pCaster->Possess(m_target);
                pCaster->SpeedCheatDelay(GetDuration());
            }
            else
            {
                pCaster->UnPossess();
            }
            break;
        }

    }
}

void Aura::SpellAuraReduceEffectDuration(bool apply)
{
    if (!m_target->IsPlayer())
        return;

    int32 val;
    if (apply)
    {
        SetPositive();
        val = mod->m_amount; ///\todo Only maximum effect should be used for Silence or Interrupt effects reduction
    }
    else
    {
        val = -mod->m_amount;
    }
    if (mod->m_miscValue > 0 && mod->m_miscValue < 28)
    {
        static_cast< Player* >(m_target)->MechanicDurationPctMod[mod->m_miscValue] += val;
    }
}

// Caster = player
// Target = vehicle
void Aura::HandleAuraControlVehicle(bool apply)
{
    //return; Dead code reason...

    if (!GetCaster()->IsUnit())
        return;

    if (!m_target->IsVehicle())
        return;

    Unit* caster = static_cast<Unit*>(GetCaster());

    if (apply)
    {
        if (m_target->GetVehicleComponent()->HasEmptySeat())
            m_target->GetVehicleComponent()->AddPassenger(caster);

    }
    else
    {
        if ((caster->GetCurrentVehicle() != nullptr) && (caster->GetCurrentVehicle() == m_target->GetVehicleComponent()))
            m_target->GetVehicleComponent()->EjectPassenger(caster);
    }

}

void Aura::SpellAuraModCombatResultChance(bool apply)
{
    if (apply)
    {
        switch (mod->m_miscValue)
        {
            case 1:
                //m_target->m_CombatResult_Parry += mod->m_amount; // parry?
                break;
            case 2:
                m_target->m_CombatResult_Dodge += mod->m_amount;
                break;
        }
    }
    else
    {
        switch (mod->m_miscValue)
        {
            case 1:
                //m_target->m_CombatResult_Parry += -mod->m_amount; // parry?
                break;
            case 2:
                m_target->m_CombatResult_Dodge += -mod->m_amount;
                break;
        }
    }
}

void Aura::SpellAuraAddHealth(bool apply)
{
    if (apply)
    {
        SetPositive();
        m_target->ModMaxHealth(mod->m_amount);
        m_target->ModHealth(mod->m_amount);
    }
    else
    {
        m_target->ModMaxHealth(-mod->m_amount);
        uint32 maxHealth = m_target->getUInt32Value(UNIT_FIELD_MAXHEALTH);
        if (m_target->getUInt32Value(UNIT_FIELD_HEALTH) > maxHealth)
            m_target->setUInt32Value(UNIT_FIELD_MAXHEALTH, maxHealth);
    }
}

void Aura::SpellAuraRemoveReagentCost(bool apply)
{
    if (p_target == nullptr)
        return;

    if (apply)
    {
        p_target->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NO_REAGANT_COST);
    }
    else
    {
        p_target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NO_REAGANT_COST);
    }
}
void Aura::SpellAuraBlockMultipleDamage(bool apply)
{
    if (p_target == nullptr)
        return;

    if (apply)
    {
        p_target->m_BlockModPct += mod->m_amount;
    }
    else
    {
        p_target->m_BlockModPct += -mod->m_amount;
    }
}

void Aura::SpellAuraModMechanicDmgTakenPct(bool apply)
{
    if (apply)
    {
        m_target->ModDamageTakenByMechPCT[mod->m_miscValue] += (float)mod->m_amount / 100;

    }
    else
    {
        m_target->ModDamageTakenByMechPCT[mod->m_miscValue] -= (float)mod->m_amount / 100;
    }
}

void Aura::SpellAuraIgnoreTargetAuraState(bool apply)
{
    if (!m_target->IsPlayer())
        return;

    if (apply)
    {
        static_cast< Player* >(m_target)->ignoreAuraStateCheck = true;
    }
    else
    {
        static_cast< Player* >(m_target)->ignoreAuraStateCheck = false;
    }
}

void Aura::SpellAuraAllowOnlyAbility(bool apply)
{
    // cannot perform any abilities (other than those in EffectMask), currently only works on players
    if (!p_target)
        return;

    // Generic
    if (apply)
    {
        p_target->m_castFilterEnabled = true;
        for (uint8_t x = 0; x < 3; x++)
#if VERSION_STRING != Cata
            p_target->m_castFilter[x] |= m_spellInfo->getEffectSpellClassMask(mod->m_effectIndex, x);
#else
            p_target->m_castFilter[x] |= m_spellInfo->EffectSpellClassMask[x];
#endif
    }
    else
    {
        p_target->m_castFilterEnabled = false;	// check if we can turn it off
        for (uint8_t x = 0; x < 3; x++)
        {
#if VERSION_STRING != Cata
            p_target->m_castFilter[x] &= ~m_spellInfo->getEffectSpellClassMask(mod->m_effectIndex, x);
#else
            p_target->m_castFilter[x] &= ~m_spellInfo->EffectSpellClassMask[x];
#endif
            if (p_target->m_castFilter[x])
                p_target->m_castFilterEnabled = true;
        }
    }
}

void Aura::SpellAuraIncreaseAPbyStatPct(bool apply)
{
    if (apply)
    {
        if (mod->m_amount > 0)
            SetPositive();
        else
            SetNegative();

        uint16_t modValue = static_cast<uint16_t>(mod->m_miscValue);

        mod->fixed_amount[mod->m_effectIndex] = m_target->GetStat(modValue) * mod->m_amount / 100;
        m_target->ModAttackPowerMods(mod->fixed_amount[mod->m_effectIndex]);
    }
    else
        m_target->ModAttackPowerMods(-mod->fixed_amount[mod->m_effectIndex]);

    m_target->CalcDamage();
}

void Aura::SpellAuraModSpellDamageDOTPct(bool apply)
{
    int32 val = (apply) ? mod->m_amount : -mod->m_amount;

    switch (m_spellInfo->getId())
    {
        //SPELL_HASH_HAUNT
        case 48181:
        case 48184:
        case 48210:
        case 50091:
        case 59161:
        case 59163:
        case 59164:
            m_target->DoTPctIncrease[m_spellInfo->getSchool()] += val;
            break;
        default:
        {
            for (uint32 x = 0; x < 7; x++)
            {
                if (mod->m_miscValue & (((uint32)1) << x))
                {
                    m_target->DoTPctIncrease[x] += val;
                }
            }
        } break;
    }
}

void Aura::SpellAuraConsumeNoAmmo(bool apply)
{
    if (p_target == nullptr)
        return;

    if (apply)
    {
        p_target->m_requiresNoAmmo = true;
    }
    else
    {
        bool other = false;

        switch (m_spellInfo->getId())
        {
            //SPELL_HASH_REQUIRES_NO_AMMO
            case 46699:
            {
                // We are unequipping Thori'dal but have an aura with no ammo consumption effect
                if (p_target->HasAuraWithName(SPELL_AURA_CONSUMES_NO_AMMO))
                    other = true;
            } break;
            default:
            {
                // we have Thori'dal too
                if (m_spellInfo->getId() != 46699 && p_target->getAuraWithId(46699))
                    other = true;
            }
        }

        // We have more than 1 aura with no ammo consumption effect
        if (p_target->GetAuraCountWithName(SPELL_AURA_CONSUMES_NO_AMMO) >= 2)
            other = true;

        p_target->m_requiresNoAmmo = other;
    }
}

void Aura::SpellAuraIgnoreShapeshift(bool apply)
{
    if (!m_target->IsPlayer())
        return;

    if (apply)
    {
        static_cast< Player* >(m_target)->ignoreShapeShiftChecks = true;
    }
    else
    {
        static_cast< Player* >(m_target)->ignoreShapeShiftChecks = false;
    }
}

void Aura::SpellAuraModIgnoreArmorPct(bool apply)
{
    switch (GetSpellInfo()->getId())
    {
        case 5530:
        case 12284:
        case 12701:
        case 12702:
        case 12703:
        case 12704:
        case 13709:
        case 13800:
        case 13801:
        case 13802:
        case 13803:
        case 20864:
        case 59224:
        {
            if (apply)
                m_target->m_ignoreArmorPctMaceSpec += (mod->m_amount / 100.0f);
            else
                m_target->m_ignoreArmorPctMaceSpec -= (mod->m_amount / 100.0f);
        } break;
        default:
        {
            if (apply)
                m_target->m_ignoreArmorPct += (mod->m_amount / 100.0f);
            else
                m_target->m_ignoreArmorPct -= (mod->m_amount / 100.0f);
        } break;
    }
}

void Aura::SpellAuraModBaseHealth(bool apply)
{
    if (!p_target)
        return;

    if (apply)
        mod->fixed_amount[0] = p_target->GetBaseHealth();

    int32 amt = mod->fixed_amount[0] * mod->m_amount / 100;

    if (!apply)
        amt *= -1;

    p_target->SetHealthFromSpell(p_target->GetHealthFromSpell() + amt);
    p_target->UpdateStats();
}

void Aura::SpellAuraModAttackPowerOfArmor(bool apply)
{
    /* Need more info about mods, currently it's only for armor
    uint32 modifier;
    switch(mod->m_miscValue):
    {
    case 1: //Armor
    modifier = UNIT_FIELD_RESISTANCES;
    break;
    }
    */

    if (apply)
    {
        if (mod->m_amount > 0)
            SetPositive();
        else
            SetNegative();

        mod->fixed_amount[mod->m_effectIndex] = m_target->GetResistance(SCHOOL_NORMAL) / mod->m_amount;
        m_target->ModAttackPowerMods(mod->fixed_amount[mod->m_effectIndex]);
    }
    else
        m_target->ModAttackPowerMods(-mod->fixed_amount[mod->m_effectIndex]);

    m_target->CalcDamage();
}

void Aura::SpellAuraDeflectSpells(bool /*apply*/)
{
    //Currently used only by Detterence and handled in Spell::DidHit
}

void Aura::SpellAuraPhase(bool apply)
{
    if (m_target->GetAuraStackCount(SPELL_AURA_PHASE) > 1)
    {
        if (m_target->IsPlayer())
            static_cast< Player* >(m_target)->GetSession()->SystemMessage("You can have only one phase aura!");
        Remove();
        return;
    }

    if (apply)
    {
        if (m_target->IsPlayer())
            static_cast<Player*>(m_target)->Phase(PHASE_SET, m_spellInfo->getEffectMiscValue(mod->m_effectIndex));
        else
            m_target->Phase(PHASE_SET, m_spellInfo->getEffectMiscValue(mod->m_effectIndex));
    }
    else
    {
        if (m_target->IsPlayer())
            static_cast<Player*>(m_target)->Phase(PHASE_RESET);
        else
            m_target->Phase(PHASE_RESET);
    }
}

void Aura::SpellAuraCallStabledPet(bool apply)
{
    if (apply)
    {
        Player* pcaster = GetPlayerCaster();
        if (pcaster != nullptr && pcaster->getClass() == HUNTER && pcaster->GetSession() != nullptr)
            pcaster->GetSession()->SendStabledPetList(0);
    }
}

void Aura::ResetDuration()
{
    expirytime = static_cast<uint32>(UNIXTIME);
    sEventMgr.ModifyEventTimeLeft(this, EVENT_AURA_REMOVE, GetDuration());
}

void Aura::Refresh()
{
    ResetDuration();
    m_target->SendAuraUpdate(m_auraSlot, false);
}

//MIT
bool Aura::DotCanCrit()
{
#if VERSION_STRING != Cata
    Unit* caster = this->GetUnitCaster();
    if (caster == nullptr)
        return false;

    SpellInfo* spell_info = this->GetSpellInfo();
    if (spell_info == nullptr)
        return false;

    // Can be critical
    if (caster->IsPlayer())
    {
        if (caster->getClass() == ROGUE)
        {
            switch (spell_info->getId())
            {
                //SPELL_HASH_RUPTURE
                case 1943:
                case 8639:
                case 8640:
                case 11273:
                case 11274:
                case 11275:
                case 14874:
                case 14903:
                case 15583:
                case 26867:
                case 48671:
                case 48672:
                    return true;
            }
        }
    }

    Aura* aura = caster->getAuraWithAuraEffect(SPELL_AURA_ALLOW_DOT_TO_CRIT);
    if (aura == nullptr)
        return false;

    SpellInfo* aura_spell_info = aura->GetSpellInfo();

    uint8 i = 0;
    if (aura_spell_info->getEffectApplyAuraName(1) == SPELL_AURA_ALLOW_DOT_TO_CRIT)
        i = 1;
    else if (aura_spell_info->getEffectApplyAuraName(2) == SPELL_AURA_ALLOW_DOT_TO_CRIT)
        i = 2;


    if (aura_spell_info->getSpellFamilyName() == spell_info->getSpellFamilyName() &&
        (aura_spell_info->getEffectSpellClassMask(i, 0) & spell_info->getSpellGroupType(0) ||
         aura_spell_info->getEffectSpellClassMask(i, 1) & spell_info->getSpellGroupType(1) ||
         aura_spell_info->getEffectSpellClassMask(i, 2) & spell_info->getSpellGroupType(2)))
    {
        return true;
    }

    return false;
#else
    Unit* caster = this->GetUnitCaster();
    if (caster == nullptr)
        return false;

    SpellInfo* sp = this->GetSpellInfo();
    bool found = false;

    for (;;)
    {
        Aura * aura = caster->getAuraWithAuraEffect(SPELL_AURA_ALLOW_DOT_TO_CRIT);

        if (aura == nullptr)
            break;

        SpellInfo* aura_sp = aura->GetSpellInfo();

        uint8 i = 0;
        if (aura_sp->EffectApplyAuraName[1] == SPELL_AURA_ALLOW_DOT_TO_CRIT)
            i = 1;
        else if (aura_sp->EffectApplyAuraName[2] == SPELL_AURA_ALLOW_DOT_TO_CRIT)
            i = 2;

        if (aura_sp->SpellFamilyName == sp->SpellFamilyName &&
            (aura_sp->EffectSpellClassMask[0] & sp->SpellGroupType[0] ||
             aura_sp->EffectSpellClassMask[1] & sp->SpellGroupType[1] ||
             aura_sp->EffectSpellClassMask[2] & sp->SpellGroupType[2]))
        {
            found = true;
            break;
        }
    }

    if (found)
        return true;

    if (caster->IsPlayer())
    {
        switch (caster->getClass())
        {
            case ROGUE:
            {
                // Rupture can be critical in patch 3.3.3
                switch (sp->getId())
                {
                    //SPELL_HASH_RUPTURE
                    case 1943:
                    case 8639:
                    case 8640:
                    case 11273:
                    case 11274:
                    case 11275:
                    case 14874:
                    case 14903:
                    case 15583:
                    case 26867:
                    case 48671:
                    case 48672:
                        return true;
                }
            } break;
        }
    }

    return false;
#endif
}


bool Aura::IsCombatStateAffecting()
{
    SpellInfo* sp = m_spellInfo;

    if (sp->appliesAreaAura(SPELL_AURA_PERIODIC_DAMAGE) ||
        sp->appliesAreaAura(SPELL_AURA_PERIODIC_DAMAGE_PERCENT) ||
        sp->appliesAreaAura(SPELL_AURA_PERIODIC_TRIGGER_SPELL) ||
        sp->appliesAreaAura(SPELL_AURA_PERIODIC_LEECH) ||
        sp->appliesAreaAura(SPELL_AURA_PERIODIC_MANA_LEECH))
        return true;

    return false;
}

bool Aura::IsAreaAura()
{
    SpellInfo* sp = m_spellInfo;

    if (sp->HasEffect(SPELL_EFFECT_APPLY_GROUP_AREA_AURA) ||
        sp->HasEffect(SPELL_EFFECT_APPLY_RAID_AREA_AURA) ||
        sp->HasEffect(SPELL_EFFECT_APPLY_PET_AREA_AURA) ||
        sp->HasEffect(SPELL_EFFECT_APPLY_FRIEND_AREA_AURA) ||
        sp->HasEffect(SPELL_EFFECT_APPLY_ENEMY_AREA_AURA) ||
        sp->HasEffect(SPELL_EFFECT_APPLY_OWNER_AREA_AURA))
        return true;

    return false;
}

void Aura::AssignModifiers(Aura* aura)
{
    for (uint8 x = 0; x < aura->m_modcount; ++x)
        AddMod(aura->m_modList[x].m_type, aura->m_modList[x].m_amount, aura->m_modList[x].m_miscValue, x);
}

void AbsorbAura::SpellAuraSchoolAbsorb(bool apply)
{
    if (!apply)
        return;

    SetPositive();

    int32 val = CalcAbsorbAmount();

    Unit* caster = GetUnitCaster();
    if (caster != nullptr)
    {
        spellModFlatIntValue(caster->SM_FMiscEffect, &val, GetSpellInfo()->getSpellGroupType());
        spellModPercentageIntValue(caster->SM_PMiscEffect, &val, GetSpellInfo()->getSpellGroupType());

        //This will fix talents that affects damage absorbed.
        int flat = 0;
        spellModFlatIntValue(caster->SM_FMiscEffect, &flat, GetSpellInfo()->getSpellGroupType());
        val += val * flat / 100;

        //For spells Affected by Bonus Healing we use Dspell_coef_override.
        if (GetSpellInfo()->Dspell_coef_override >= 0)
            val += float2int32(caster->HealDoneMod[GetSpellInfo()->getSchool()] * GetSpellInfo()->Dspell_coef_override);
        //For spells Affected by Bonus Damage we use OTspell_coef_override.
        else if (GetSpellInfo()->OTspell_coef_override >= 0)
            val += float2int32(caster->GetDamageDoneMod(GetSpellInfo()->getSchool()) * GetSpellInfo()->OTspell_coef_override);
    }

    m_total_amount = val;
    m_amount = val;
    m_pct_damage = CalcPctDamage();
}

uint32 AbsorbAura::AbsorbDamage(uint32 School, uint32* dmg)
{
    uint32 mask = GetSchoolMask();
    if (!(mask & g_spellSchoolConversionTable[School]))
        return 0;

    uint32 dmg_absorbed = 0;
    int32 dmg_to_absorb = *dmg;

    if (m_pct_damage < 100)
        dmg_to_absorb = dmg_to_absorb * m_pct_damage / 100;

    if (dmg_to_absorb >= m_amount)
    {
        *dmg -= m_amount;
        dmg_absorbed += m_amount;

        m_target->RemoveAura(GetSpellId());
    }
    else
    {
        dmg_absorbed += dmg_to_absorb;
        m_amount -= dmg_to_absorb;
        *dmg -= dmg_to_absorb;
    }

    return dmg_absorbed;
}

void Aura::SpellAuraConvertRune(bool apply)
{
    if (p_target == nullptr || !p_target->IsDeathKnight())
        return;

    DeathKnight* dk = static_cast<DeathKnight*>(p_target);

    if (apply)
    {
        for (uint8_t i = 0; i < MAX_RUNES; ++i)
        {
            if (dk->GetRuneType(i) == mod->m_miscValue && !dk->GetRuneIsUsed(i))
            {
                dk->ConvertRune(i, static_cast<uint8_t>(GetSpellInfo()->getEffectMiscValueB(mod->m_effectIndex)));
            }
        }
    }
    else
    {
        for (uint8_t i = 0; i < MAX_RUNES; ++i)
        {
            if (dk->GetRuneType(i) == GetSpellInfo()->getEffectMiscValueB(mod->m_effectIndex))
            {
                dk->ConvertRune(i, dk->GetBaseRuneType(i));
            }
        }
    }
}

void Aura::SpellAuraMirrorImage(bool apply)
{
    if (m_target == nullptr || !m_target->IsCreature())
        return;

    if (apply && m_target->IsSummon() && (m_target->GetCreatedByGUID() == GetCasterGUID()))
    {
        Summon* s = static_cast< Summon* >(m_target);

        s->SetDisplayId(s->GetOwner()->GetDisplayId());
#if VERSION_STRING != Classic
        s->setUInt32Value(UNIT_FIELD_FLAGS_2, s->getUInt32Value(UNIT_FIELD_FLAGS_2) | UNIT_FLAG2_MIRROR_IMAGE);
#endif
    }

    SpellAuraMirrorImage2(apply);
}

void Aura::SpellAuraMirrorImage2(bool apply)
{
    if (m_target == nullptr)
        return;

    if (apply && m_target->IsSummon() && (m_target->GetCreatedByGUID() == GetCasterGUID()))
    {
        if (GetCaster()->IsPlayer())
        {
            Player* p = static_cast<Player*>(GetCaster());

            Item* item;

            item = p->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
            if (item != nullptr)
                m_target->setUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, item->GetItemProperties()->ItemId);

            item = p->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
            if (item != nullptr)
                m_target->setUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, item->GetItemProperties()->ItemId);
        }
        else
        {
            m_target->setUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID, GetCaster()->getUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID));
            m_target->setUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, GetCaster()->getUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1));
            m_target->setUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 2, GetCaster()->getUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 2));
        }
    }
}
