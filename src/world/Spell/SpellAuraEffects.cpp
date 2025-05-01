/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Spell/Spell.hpp"
#include "SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Definitions/SpellFamily.hpp"
#include "Definitions/SpellIsFlags.hpp"
#include "Definitions/SpellTypes.hpp"
#include "SpellMgr.hpp"
#include "Logging/Logger.hpp"
#include "Management/ItemInterface.h"
#include "Storage/WDB/WDBStores.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Item.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Random.hpp"

pSpellAura SpellAuraHandler[TOTAL_SPELL_AURAS] =
{
    &Aura::spellAuraEffectNotImplemented,                                   //   0 SPELL_AURA_NONE
    &Aura::SpellAuraBindSight,                                              //   1 SPELL_AURA_BIND_SIGHT
    &Aura::SpellAuraModPossess,                                             //   2 SPELL_AURA_MOD_POSSESS
    &Aura::spellAuraEffectPeriodicDamage,                                   //   3 SPELL_AURA_PERIODIC_DAMAGE
    &Aura::spellAuraEffectDummy,                                            //   4 SPELL_AURA_DUMMY
    &Aura::SpellAuraModConfuse,                                             //   5 SPELL_AURA_MOD_CONFUSE
    &Aura::SpellAuraModCharm,                                               //   6 SPELL_AURA_MOD_CHARM
    &Aura::SpellAuraModFear,                                                //   7 SPELL_AURA_MOD_FEAR
    &Aura::spellAuraEffectPeriodicHeal,                                     //   8 SPELL_AURA_PERIODIC_HEAL
    &Aura::SpellAuraModAttackSpeed,                                         //   9 SPELL_AURA_MOD_ATTACK_SPEED
    &Aura::SpellAuraModThreatGenerated,                                     //  10 SPELL_AURA_MOD_THREAT_GENERATED
    &Aura::SpellAuraModTaunt,                                               //  11 SPELL_AURA_MOD_TAUNT
    &Aura::SpellAuraModStun,                                                //  12 SPELL_AURA_MOD_STUN
    &Aura::SpellAuraModDamageDone,                                          //  13 SPELL_AURA_MOD_DAMAGE_DONE
    &Aura::SpellAuraModDamageTaken,                                         //  14 SPELL_AURA_MOD_DAMAGE_TAKEN
    &Aura::SpellAuraDamageShield,                                           //  15 SPELL_AURA_DAMAGE_SHIELD
    &Aura::SpellAuraModStealth,                                             //  16 SPELL_AURA_MOD_STEALTH
    &Aura::SpellAuraModStealthDetection,                                    //  17 SPELL_AURA_MOD_STEALTH_DETECTION
    &Aura::SpellAuraModInvisibility,                                        //  18 SPELL_AURA_MOD_INVISIBILITY
    &Aura::SpellAuraModInvisibilityDetection,                               //  19 SPELL_AURA_MOD_INVISIBILITY_DETECTION
    &Aura::spellAuraEffectPeriodicHealPct,                                  //  20 SPELL_AURA_PERIODIC_HEAL_PCT
    &Aura::spellAuraEffectPeriodicPowerPct,                                 //  21 SPELL_AURA_PERIODIC_POWER_PCT
    &Aura::SpellAuraModResistance,                                          //  22 SPELL_AURA_MOD_RESISTANCE
    &Aura::spellAuraEffectPeriodicTriggerSpell,                             //  23 SPELL_AURA_PERIODIC_TRIGGER_SPELL
    &Aura::spellAuraEffectPeriodicEnergize,                                 //  24 SPELL_AURA_PERIODIC_ENERGIZE
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
    &Aura::spellAuraEffectModShapeshift,                                    //  36 SPELL_AURA_MOD_SHAPESHIFT
    &Aura::SpellAuraModEffectImmunity,                                      //  37 SPELL_AURA_MOD_EFFECT_IMMUNITY
    &Aura::SpellAuraModStateImmunity,                                       //  38 SPELL_AURA_MOD_STATE_IMMUNITY
    &Aura::SpellAuraModSchoolImmunity,                                      //  39 SPELL_AURA_MOD_SCHOOL_IMMUNITY
    &Aura::SpellAuraModDmgImmunity,                                         //  40 SPELL_AURA_MOD_DMG_IMMUNITY
    &Aura::SpellAuraModDispelImmunity,                                      //  41 SPELL_AURA_MOD_DISPEL_IMMUNITY
    &Aura::SpellAuraProcTriggerSpell,                                       //  42 SPELL_AURA_PROC_TRIGGER_SPELL
    &Aura::SpellAuraProcTriggerDamage,                                      //  43 SPELL_AURA_PROC_TRIGGER_DAMAGE
    &Aura::SpellAuraTrackCreatures,                                         //  44 SPELL_AURA_TRACK_CREATURES
    &Aura::SpellAuraTrackResources,                                         //  45 SPELL_AURA_TRACK_RESOURCES
    &Aura::spellAuraEffectNotImplemented,                                   //  46 SPELL_AURA_46
    &Aura::SpellAuraModParryPerc,                                           //  47 SPELL_AURA_MOD_PARRY_PERC
    &Aura::spellAuraEffectNotImplemented,                                   //  48 SPELL_AURA_48
    &Aura::SpellAuraModDodgePerc,                                           //  49 SPELL_AURA_MOD_DODGE_PERC
    &Aura::spellAuraEffectNotImplemented,                                   //  50 SPELL_AURA_50
    &Aura::SpellAuraModBlockPerc,                                           //  51 SPELL_AURA_MOD_BLOCK_PERC
    &Aura::SpellAuraModCritPerc,                                            //  52 SPELL_AURA_MOD_CRIT_PERC
    &Aura::spellAuraEffectPeriodicLeech,                                    //  53 SPELL_AURA_PERIODIC_LEECH
    &Aura::SpellAuraModHitChance,                                           //  54 SPELL_AURA_MOD_HIT_CHANCE
    &Aura::SpellAuraModSpellHitChance,                                      //  55 SPELL_AURA_MOD_SPELL_HIT_CHANCE
    &Aura::spellAuraEffectTransform,                                        //  56 SPELL_AURA_TRANSFORM
    &Aura::SpellAuraModSpellCritChance,                                     //  57 SPELL_AURA_MOD_SPELL_CRIT_CHANCE
    &Aura::SpellAuraIncreaseSwimSpeed,                                      //  58 SPELL_AURA_INCREASE_SWIM_SPEED
    &Aura::SpellAuraModCratureDmgDone,                                      //  59 SPELL_AURA_MOD_CRATURE_DMG_DONE
    &Aura::SpellAuraPacifySilence,                                          //  60 SPELL_AURA_PACIFY_SILENCE
    &Aura::SpellAuraModScale,                                               //  61 SPELL_AURA_MOD_SCALE
    &Aura::spellAuraEffectPeriodicHealthFunnel,                             //  62 SPELL_AURA_PERIODIC_HEALTH_FUNNEL
    &Aura::spellAuraEffectNotImplemented,                                   //  63 SPELL_AURA_63
    &Aura::spellAuraEffectPeriodicManaLeech,                                //  64 SPELL_AURA_PERIODIC_MANALEECH
    &Aura::SpellAuraModCastingSpeed,                                        //  65 SPELL_AURA_MOD_CASTING_SPEED
    &Aura::SpellAuraFeignDeath,                                             //  66 SPELL_AURA_FEIGN_DEATH
    &Aura::SpellAuraModDisarm,                                              //  67 SPELL_AURA_MOD_DISARM
    &Aura::SpellAuraModStalked,                                             //  68 SPELL_AURA_MOD_STALKED
    &Aura::spellAuraEffectSchoolAbsorb,                                     //  69 SPELL_AURA_SCHOOL_ABSORB
    &Aura::spellAuraEffectNotImplemented,                                   //  70 SPELL_AURA_70
    &Aura::SpellAuraModSpellCritChanceSchool,                               //  71 SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
    &Aura::SpellAuraModPowerCost,                                           //  72 SPELL_AURA_MOD_POWER_COST
    &Aura::SpellAuraModPowerCostSchool,                                     //  73 SPELL_AURA_MOD_POWER_COST_SCHOOL
    &Aura::SpellAuraReflectSpellsSchool,                                    //  74 SPELL_AURA_REFLECT_SPELLS_SCHOOL
    &Aura::SpellAuraModLanguage,                                            //  75 SPELL_AURA_MOD_LANGUAGE
    &Aura::SpellAuraAddFarSight,                                            //  76 SPELL_AURA_ADD_FAR_SIGHT
    &Aura::SpellAuraMechanicImmunity,                                       //  77 SPELL_AURA_MECHANIC_IMMUNITY
    &Aura::SpellAuraMounted,                                                //  78 SPELL_AURA_MOUNTED
    &Aura::SpellAuraModDamagePercDone,                                      //  79 SPELL_AURA_MOD_DAMAGE_PERC_DONE
    &Aura::SpellAuraModPercStat,                                            //  80 SPELL_AURA_MOD_PERC_STAT
    &Aura::SpellAuraSplitDamage,                                            //  81 SPELL_AURA_SPLIT_DAMAGE
    &Aura::SpellAuraWaterBreathing,                                         //  82 SPELL_AURA_WATER_BREATHING
    &Aura::SpellAuraModBaseResistance,                                      //  83 SPELL_AURA_MOD_BASE_RESISTANCE
    &Aura::spellAuraEffectNotUsed,                                          //  84 SPELL_AURA_MOD_HEALTH_REGEN // Implemented in Player::calculateHealthRegenerationValue, Creature::RegenerateHealth
    &Aura::spellAuraEffectModPowerRegen,                                    //  85 SPELL_AURA_MOD_POWER_REGEN
    &Aura::SpellAuraChannelDeathItem,                                       //  86 SPELL_AURA_CHANNEL_DEATH_ITEM
    &Aura::SpellAuraModDamagePercTaken,                                     //  87 SPELL_AURA_MOD_DAMAGE_PERC_TAKEN
    &Aura::spellAuraEffectNotUsed,                                          //  88 SPELL_AURA_MOD_HEALTH_REGEN_PERCENT // Implemented in Player::calculateHealthRegenerationValue, Creature::RegenerateHealth
    &Aura::spellAuraEffectPeriodicDamagePercent,                            //  89 SPELL_AURA_PERIODIC_DAMAGE_PERCENT
    &Aura::SpellAuraModResistChance,                                        //  90 SPELL_AURA_MOD_RESIST_CHANCE
    &Aura::SpellAuraModDetectRange,                                         //  91 SPELL_AURA_MOD_DETECT_RANGE
    &Aura::SpellAuraPreventsFleeing,                                        //  92 SPELL_AURA_PREVENTS_FLEEING
    &Aura::SpellAuraModUnattackable,                                        //  93 SPELL_AURA_MOD_UNATTACKABLE
    &Aura::SpellAuraInterruptRegen,                                         //  94 SPELL_AURA_INTERRUPT_REGEN
    &Aura::SpellAuraGhost,                                                  //  95 SPELL_AURA_GHOST
    &Aura::SpellAuraMagnet,                                                 //  96 SPELL_AURA_MAGNET
    &Aura::SpellAuraManaShield,                                             //  97 SPELL_AURA_MANA_SHIELD
    &Aura::SpellAuraSkillTalent,                                            //  98 SPELL_AURA_SKILL_TALENT
    &Aura::SpellAuraModAttackPower,                                         //  99 SPELL_AURA_MOD_ATTACK_POWER
    &Aura::SpellAuraVisible,                                                // 100 SPELL_AURA_VISIBLE
    &Aura::SpellAuraModResistancePCT,                                       // 101 SPELL_AURA_MOD_RESISTANCE_PCT
    &Aura::SpellAuraModCreatureAttackPower,                                 // 102 SPELL_AURA_MOD_CREATURE_ATTACK_POWER
    &Aura::SpellAuraModTotalThreat,                                         // 103 SPELL_AURA_MOD_TOTAL_THREAT
    &Aura::SpellAuraWaterWalk,                                              // 104 SPELL_AURA_WATER_WALK
    &Aura::SpellAuraFeatherFall,                                            // 105 SPELL_AURA_FEATHER_FALL
    &Aura::SpellAuraHover,                                                  // 106 SPELL_AURA_HOVER
    &Aura::spellAuraEffectAddModifier,                                      // 107 SPELL_AURA_ADD_FLAT_MODIFIER
    &Aura::spellAuraEffectAddModifier,                                      // 108 SPELL_AURA_ADD_PCT_MOD
    &Aura::SpellAuraAddClassTargetTrigger,                                  // 109 SPELL_AURA_ADD_CLASS_TARGET_TRIGGER
    &Aura::spellAuraEffectModPowerRegen,                                    // 110 SPELL_AURA_MOD_POWER_REGEN_PERCENT
    &Aura::spellAuraEffectNotImplemented,                                   // 111 SPELL_AURA_111
    &Aura::SpellAuraOverrideClassScripts,                                   // 112 SPELL_AURA_OVERRIDE_CLASS_SCRIPTS
    &Aura::SpellAuraModRangedDamageTaken,                                   // 113 SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN
    &Aura::spellAuraEffectNotImplemented,                                   // 114 SPELL_AURA_114
    &Aura::SpellAuraModHealing,                                             // 115 SPELL_AURA_MOD_HEALING
    &Aura::spellAuraEffectNotUsed,                                          // 116 SPELL_AURA_MOD_HEALTH_REGEN_DURING_COMBAT // Implemented in Player::calculateHealthRegenerationValue
    &Aura::SpellAuraModMechanicResistance,                                  // 117 SPELL_AURA_MOD_MECHANIC_RESISTANCE
    &Aura::SpellAuraModHealingPCT,                                          // 118 SPELL_AURA_MOD_HEALING_PCT
    &Aura::spellAuraEffectNotImplemented,                                   // 119 SPELL_AURA_119
    &Aura::SpellAuraUntrackable,                                            // 120 SPELL_AURA_UNTRACKABLE
    &Aura::SpellAuraEmphaty,                                                // 121 SPELL_AURA_EMPHATY
    &Aura::SpellAuraModOffhandDamagePCT,                                    // 122 SPELL_AURA_MOD_OFFHANDDAMAGE_PCT
    &Aura::SpellAuraModPenetration,                                         // 123 SPELL_AURA_MOD_PENETRATION
    &Aura::SpellAuraModRangedAttackPower,                                   // 124 SPELL_AURA_MOD_RANGED_ATTACK_POWER
    &Aura::SpellAuraModMeleeDamageTaken,                                    // 125 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN
    &Aura::SpellAuraModMeleeDamageTakenPct,                                 // 126 SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT
    &Aura::SpellAuraRAPAttackerBonus,                                       // 127 SPELL_AURA_RAP_ATTACKER_BONUS
    &Aura::SpellAuraModPossessPet,                                          // 128 SPELL_AURA_MOD_POSSESS_PET
    &Aura::SpellAuraModIncreaseSpeedAlways,                                 // 129 SPELL_AURA_MOD_INCREASE_SPEED_ALWAYS
    &Aura::SpellAuraModIncreaseMountedSpeed,                                // 130 SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED
    &Aura::SpellAuraModCreatureRangedAttackPower,                           // 131 SPELL_AURA_MOD_CREATURE_RANGED_ATTACK_POWER
    &Aura::SpellAuraModIncreaseEnergyPerc,                                  // 132 SPELL_AURA_MOD_INCREASE_ENERGY_PERC
    &Aura::SpellAuraModIncreaseHealthPerc,                                  // 133 SPELL_AURA_MOD_INCREASE_HEALTH_PERC
    &Aura::SpellAuraModManaRegInterrupt,                                    // 134 SPELL_AURA_MOD_MANA_REG_INTERRUPT
    &Aura::SpellAuraModHealingDone,                                         // 135 SPELL_AURA_MOD_HEALING_DONE
    &Aura::SpellAuraModHealingDonePct,                                      // 136 SPELL_AURA_MOD_HEALING_DONE_PCT
    &Aura::SpellAuraModTotalStatPerc,                                       // 137 SPELL_AURA_MOD_TOTAL_STAT_PERC
    &Aura::SpellAuraModHaste,                                               // 138 SPELL_AURA_MOD_HASTE
    &Aura::SpellAuraForceReaction,                                          // 139 SPELL_AURA_FORCE_REACTION
    &Aura::SpellAuraModRangedHaste,                                         // 140 SPELL_AURA_MOD_RANGED_HASTE
    &Aura::SpellAuraModRangedAmmoHaste,                                     // 141 SPELL_AURA_MOD_RANGED_AMMO_HASTE
    &Aura::SpellAuraModBaseResistancePerc,                                  // 142 SPELL_AURA_MOD_BASE_RESISTANCE_PERC
    &Aura::SpellAuraModResistanceExclusive,                                 // 143 SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE
    &Aura::SpellAuraSafeFall,                                               // 144 SPELL_AURA_SAFE_FALL
    &Aura::spellAuraEffectNotImplemented,                                   // 145 SPELL_AURA_145
    &Aura::spellAuraEffectNotImplemented,                                   // 146 SPELL_AURA_146
    &Aura::spellAuraEffectNotImplemented,                                   // 147 SPELL_AURA_147
    &Aura::spellAuraEffectRetainComboPoints,                                // 148 SPELL_AURA_RETAIN_COMBO_POINTS
    &Aura::SpellAuraResistPushback,                                         // 149 SPELL_AURA_RESIST_PUSHBACK
    &Aura::SpellAuraModShieldBlockPCT,                                      // 150 SPELL_AURA_MOD_SHIELD_BLOCK_PCT
    &Aura::SpellAuraTrackStealthed,                                         // 151 SPELL_AURA_TRACK_STEALTHED
    &Aura::SpellAuraModDetectedRange,                                       // 152 SPELL_AURA_MOD_DETECTED_RANGE
    &Aura::SpellAuraSplitDamageFlat,                                        // 153 SPELL_AURA_SPLIT_DAMAGE_FLAT
    &Aura::SpellAuraModStealthLevel,                                        // 154 SPELL_AURA_MOD_STEALTH_LEVEL
    &Aura::SpellAuraModUnderwaterBreathing,                                 // 155 SPELL_AURA_MOD_UNDERWATER_BREATHING
    &Aura::SpellAuraModReputationAdjust,                                    // 156 SPELL_AURA_MOD_REPUTATION_ADJUST
    &Aura::spellAuraEffectNotImplemented,                                   // 157 SPELL_AURA_157
    &Aura::SpellAuraModBlockValue,                                          // 158 SPELL_AURA_MOD_BLOCK_VALUE
    &Aura::SpellAuraNoPVPCredit,                                            // 159 SPELL_AURA_NO_PVP_CREDIT
    &Aura::spellAuraEffectNotImplemented,                                   // 160 SPELL_AURA_160
    &Aura::spellAuraEffectNotUsed,                                          // 161 SPELL_AURA_MOD_HEALTH_REGEN_ALWAYS // implemented in Player::calculateHealthRegenerationValue
    &Aura::spellAuraEffectPeriodicPowerBurn,                                // 162 SPELL_AURA_PERIODIC_POWER_BURN
    &Aura::SpellAuraModCritDmgPhysical,                                     // 163 SPELL_AURA_MOD_CRIT_DMG_PHYSICAL
    &Aura::spellAuraEffectNotImplemented,                                   // 164 SPELL_AURA_164
    &Aura::SpellAuraAPAttackerBonus,                                        // 165 SPELL_AURA_AP_ATTACKER_BONUS
    &Aura::SpellAuraModPAttackPower,                                        // 166 SPELL_AURA_MOD_P_ATTACK_POWER
    &Aura::SpellAuraModRangedAttackPowerPct,                                // 167 SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT
    &Aura::SpellAuraIncreaseDamageTypePCT,                                  // 168 SPELL_AURA_INCREASE_DAMAGE_TYPE_PCT
    &Aura::SpellAuraIncreaseCricticalTypePCT,                               // 169 SPELL_AURA_INCREASE_CRICTICAL_TYPE_PCT
    &Aura::spellAuraEffectNotImplemented,                                   // 170 SPELL_AURA_170
    &Aura::SpellAuraIncreasePartySpeed,                                     // 171 SPELL_AURA_INCREASE_PARTY_SPEED
    &Aura::SpellAuraIncreaseMovementAndMountedSpeed,                        // 172 SPELL_AURA_INCREASE_MOVEMENT_AND_MOUNTED_SPEED
    &Aura::spellAuraEffectNotImplemented,                                   // 173 SPELL_AURA_173
    &Aura::SpellAuraIncreaseSpellDamageByAttribute,                         // 174 SPELL_AURA_INCREASE_SPELL_DAMAGE_BY_ATTRIBUTE
    &Aura::SpellAuraIncreaseHealingByAttribute,                             // 175 SPELL_AURA_INCREASE_HEALING_BY_ATTRIBUTE
    &Aura::SpellAuraSpiritOfRedemption,                                     // 176 SPELL_AURA_SPIRIT_OF_REDEMPTION
    &Aura::spellAuraEffectNotImplemented,                                   // 177 SPELL_AURA_177
    &Aura::spellAuraEffectNotImplemented,                                   // 178 SPELL_AURA_178
    &Aura::SpellAuraIncreaseAttackerSpellCrit,                              // 179 SPELL_AURA_INCREASE_ATTACKER_SPELL_CRIT
    &Aura::spellAuraEffectNotImplemented,                                   // 180 SPELL_AURA_180
    &Aura::spellAuraEffectNotImplemented,                                   // 181 SPELL_AURA_181
    &Aura::SpellAuraIncreaseArmorByPctInt,                                  // 182 SPELL_AURA_INCREASE_ARMOR_BY_PCT_INT
    &Aura::spellAuraEffectNotImplemented,                                   // 183 SPELL_AURA_183
    &Aura::SpellAuraReduceAttackerMHitChance,                               // 184 SPELL_AURA_REDUCE_ATTACKER_M_HIT_CHANCE
    &Aura::SpellAuraReduceAttackerRHitChance,                               // 185 SPELL_AURA_REDUCE_ATTACKER_R_HIT_CHANCE
    &Aura::SpellAuraReduceAttackerSHitChance,                               // 186 SPELL_AURA_REDUCE_ATTACKER_S_HIT_CHANCE
    &Aura::SpellAuraReduceEnemyMCritChance,                                 // 187 SPELL_AURA_REDUCE_ENEMY_M_CRIT_CHANCE
    &Aura::SpellAuraReduceEnemyRCritChance,                                 // 188 SPELL_AURA_REDUCE_ENEMY_R_CRIT_CHANCE
    &Aura::SpellAuraIncreaseRating,                                         // 189 SPELL_AURA_INCREASE_RATING
    &Aura::SpellAuraIncreaseRepGainPct,                                     // 190 SPELL_AURA_INCREASE_REP_GAIN_PCT
    &Aura::SpellAuraLimitSpeed,                                             // 191 SPELL_AURA_LIMIT_SPEED
#if VERSION_STRING >= TBC
    &Aura::SpellAuraMeleeHaste,                                             // 192 SPELL_AURA_MELEE_HASTE
    &Aura::SpellAuraIncreaseTimeBetweenAttacksPCT,                          // 193 SPELL_AURA_INCREASE_TIME_BETWEEN_ATTACKS_PCT
    &Aura::spellAuraEffectNotImplemented,                                   // 194 SPELL_AURA_194
    &Aura::spellAuraEffectNotImplemented,                                   // 195 SPELL_AURA_195
    &Aura::spellAuraEffectNotImplemented,                                   // 196 SPELL_AURA_196
    &Aura::SpellAuraModAttackerCritChance,                                  // 197 SPELL_AURA_MOD_ATTACKER_CRIT_CHANCE
    &Aura::SpellAuraIncreaseAllWeaponSkill,                                 // 198 SPELL_AURA_INCREASE_ALL_WEAPON_SKILL 
    &Aura::SpellAuraIncreaseHitRate,                                        // 199 SPELL_AURA_INCREASE_HIT_RATE
    &Aura::spellAuraEffectNotImplemented,                                   // 200 SPELL_AURA_200
    &Aura::SpellAuraAllowFlight,                                            // 201 SPELL_AURA_ALLOW_FLIGHT
    &Aura::SpellAuraFinishingMovesCannotBeDodged,                           // 202 SPELL_AURA_FINISHING_MOVES_CANNOT_BE_DODGED
    &Aura::SpellAuraReduceCritMeleeAttackDmg,                               // 203 SPELL_AURA_REDUCE_CRIT_MELEE_ATTACK_DMG
    &Aura::SpellAuraReduceCritRangedAttackDmg,                              // 204 SPELL_AURA_REDUCE_CRIT_RANGED_ATTACK_DMG
    &Aura::spellAuraEffectNotImplemented,                                   // 205 SPELL_AURA_205
    &Aura::SpellAuraEnableFlight,                                           // 206 SPELL_AURA_ENABLE_FLIGHT
    &Aura::SpellAuraEnableFlight,                                           // 207 SPELL_AURA_ENABLE_FLIGHT
    &Aura::SpellAuraEnableFlightWithUnmountedSpeed,                         // 208 SPELL_AURA_ENABLE_FLIGHT_WITH_UNMOUNTED_SPEED
    &Aura::spellAuraEffectNotImplemented,                                   // 209 SPELL_AURA_209
    &Aura::spellAuraEffectNotImplemented,                                   // 210 SPELL_AURA_210
    &Aura::SpellAuraIncreaseFlightSpeed,                                    // 211 SPELL_AURA_INCREASE_FLIGHT_SPEED
    &Aura::SpellAuraIncreaseRAPbyStatPct,                                   // 212 SPELL_AURA_INCREASE_RAP_BY_STAT_PCT
    &Aura::SpellAuraIncreaseRageFromDamageDealtPCT,                         // 213 SPELL_AURA_INCREASE_RAGE_FROM_DAMAGE_DEALT_PCT
    &Aura::spellAuraEffectNotImplemented,                                   // 214 SPELL_AURA_214
    &Aura::SpellAuraRemoveReagentCost,                                      // 215 SPELL_AURA_REMOVE_REAGENT_COST
    &Aura::SpellAuraModCastingSpeed,                                        // 216 SPELL_AURA_MOD_CASTING_SPEED
    &Aura::spellAuraEffectNotImplemented,                                   // 217 SPELL_AURA_217
    &Aura::spellAuraEffectNotImplemented,                                   // 218 SPELL_AURA_218
    &Aura::SpellAuraRegenManaStatPCT,                                       // 219 SPELL_AURA_REGEN_MANA_STAT_PCT
    &Aura::SpellAuraSpellHealingStatPCT,                                    // 220 SPELL_AURA_SPELL_HEALING_STAT_PCT
    &Aura::SpellAuraModDetaunt,                                             // 221 SPELL_AURA_MOD_DETAUNT
    &Aura::spellAuraEffectNotImplemented,                                   // 222 SPELL_AURA_222
    &Aura::spellAuraEffectNotImplemented,                                   // 223 SPELL_AURA_223
    &Aura::spellAuraEffectNotImplemented,                                   // 224 SPELL_AURA_224
    &Aura::spellAuraEffectNotImplemented,                                   // 225 SPELL_AURA_225
    &Aura::spellAuraEffectPeriodicTriggerDummy,                             // 226 SPELL_AURA_PERIODIC_TRIGGER_DUMMY
    &Aura::spellAuraEffectPeriodicTriggerSpellWithValue,                    // 227 SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE
    &Aura::spellAuraEffectNotImplemented,                                   // 228 SPELL_AURA_228
    &Aura::SpellAuraReduceAOEDamageTaken,                                   // 229 SPELL_AURA_REDUCE_AOE_DAMAGE_TAKEN
    &Aura::SpellAuraIncreaseMaxHealth,                                      // 230 SPELL_AURA_INCREASE_MAX_HEALTH
    &Aura::SpellAuraProcTriggerSpell,                                       // 231 SPELL_AURA_PROC_TRIGGER_SPELL
    &Aura::SpellAuraReduceEffectDuration,                                   // 232 SPELL_AURA_REDUCE_EFFECT_DURATION
    &Aura::spellAuraEffectNotImplemented,                                   // 233 SPELL_AURA_233
    &Aura::SpellAuraReduceEffectDuration,                                   // 234 SPELL_AURA_REDUCE_EFFECT_DURATION
    &Aura::spellAuraEffectNotImplemented,                                   // 235 SPELL_AURA_235
    &Aura::HandleAuraControlVehicle,                                        // 236 HANDLE_AURA_CONTROL_VEHICLE
    &Aura::SpellAuraModHealingByAP,                                         // 237 SPELL_AURA_MOD_HEALING_BY_AP
    &Aura::SpellAuraModSpellDamageByAP,                                     // 238 SPELL_AURA_MOD_SPELL_DAMAGE_BY_AP
    &Aura::SpellAuraModScale,                                               // 239 SPELL_AURA_MOD_SCALE
    &Aura::SpellAuraExpertise,                                              // 240 SPELL_AURA_EXPERTISE
    &Aura::SpellAuraForceMoveForward,                                       // 241 SPELL_AURA_FORCE_MOVE_FORWARD
    &Aura::spellAuraEffectNotImplemented,                                   // 242 SPELL_AURA_242
    &Aura::spellAuraEffectNotImplemented,                                   // 243 SPELL_AURA_243
    &Aura::SpellAuraComprehendLang,                                         // 244 SPELL_AURA_COMPREHEND_LANG
    &Aura::spellAuraEffectNotImplemented,                                   // 245 SPELL_AURA_245
    &Aura::spellAuraEffectNotImplemented,                                   // 246 SPELL_AURA_246
    &Aura::SpellAuraMirrorImage,                                            // 247 SPELL_AURA_MIRROR_IMAGE
    &Aura::SpellAuraModCombatResultChance,                                  // 248 SPELL_AURA_MOD_COMBAT_RESULT_CHANCE
    &Aura::SpellAuraConvertRune,                                            // 249 SPELL_AURA_CONVERT_RUNE
    &Aura::SpellAuraAddHealth,                                              // 250 SPELL_AURA_ADD_HEALTH
    &Aura::spellAuraEffectNotImplemented,                                   // 251 SPELL_AURA_251
    &Aura::spellAuraEffectNotImplemented,                                   // 252 SPELL_AURA_252
    &Aura::SpellAuraBlockMultipleDamage,                                    // 253 SPELL_AURA_BLOCK_MULTIPLE_DAMAGE
    &Aura::SpellAuraModDisarm,                                              // 254 SPELL_AURA_MOD_DISARM
    &Aura::SpellAuraModMechanicDmgTakenPct,                                 // 255 SPELL_AURA_MOD_MECHANIC_DMG_TAKEN_PCT
    &Aura::SpellAuraRemoveReagentCost,                                      // 256 SPELL_AURA_REMOVE_REAGENT_COST
    &Aura::spellAuraEffectNotImplemented,                                   // 257 SPELL_AURA_257
    &Aura::spellAuraEffectNotImplemented,                                   // 258 SPELL_AURA_258
    &Aura::spellAuraEffectNotImplemented,                                   // 259 SPELL_AURA_259
    &Aura::spellAuraEffectNotImplemented,                                   // 260 SPELL_AURA_260
    &Aura::SpellAuraPhase,                                                  // 261 SPELL_AURA_PHASE
#endif
#if VERSION_STRING >= WotLK
    &Aura::spellAuraEffectNotImplemented,                                   // 262 SPELL_AURA_262
    &Aura::SpellAuraAllowOnlyAbility,                                       // 263 SPELL_AURA_ALLOW_ONLY_ABILITY
    &Aura::spellAuraEffectNotImplemented,                                   // 264 SPELL_AURA_264
    &Aura::spellAuraEffectNotImplemented,                                   // 265 SPELL_AURA_265
    &Aura::spellAuraEffectNotImplemented,                                   // 266 SPELL_AURA_266
    &Aura::spellAuraEffectNotImplemented,                                   // 267 SPELL_AURA_267
    &Aura::SpellAuraIncreaseAPbyStatPct,                                    // 268 SPELL_AURA_INCREASE_AP_BY_STAT_PCT
    &Aura::spellAuraEffectNotImplemented,                                   // 269 SPELL_AURA_269
    &Aura::spellAuraEffectNotImplemented,                                   // 270 SPELL_AURA_270
    &Aura::SpellAuraModSpellDamageDOTPct,                                   // 271 SPELL_AURA_MOD_SPELL_DAMAGE_DOT_PCT
    &Aura::spellAuraEffectNotImplemented,                                   // 272 SPELL_AURA_272
    &Aura::spellAuraEffectNotImplemented,                                   // 273 SPELL_AURA_273
    &Aura::SpellAuraConsumeNoAmmo,                                          // 274 SPELL_AURA_CONSUME_NO_AMMO
    &Aura::spellAuraEffectNotImplemented,                                   // 275 SPELL_AURA_275
    &Aura::spellAuraEffectNotImplemented,                                   // 276 SPELL_AURA_276
    &Aura::spellAuraEffectNotImplemented,                                   // 277 SPELL_AURA_277
    &Aura::SpellAuraModDisarm,                                              // 278 SPELL_AURA_MOD_DISARM
    &Aura::SpellAuraMirrorImage2,                                           // 279 SPELL_AURA_MIRROR_IMAGE2
    &Aura::SpellAuraModIgnoreArmorPct,                                      // 280 SPELL_AURA_MOD_IGNORE_ARMOR_PCT
    &Aura::spellAuraEffectNotImplemented,                                   // 281 SPELL_AURA_281
    &Aura::SpellAuraModBaseHealth,                                          // 282 SPELL_AURA_MOD_BASE_HEALTH
    &Aura::SpellAuraModHealingPCT,                                          // 283 SPELL_AURA_MOD_HEALING_PCT
    &Aura::spellAuraEffectNotImplemented,                                   // 284 SPELL_AURA_284
    &Aura::SpellAuraModAttackPowerOfArmor,                                  // 285 SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR
    &Aura::spellAuraEffectNotImplemented,                                   // 286 SPELL_AURA_286
    &Aura::SpellAuraDeflectSpells,                                          // 287 SPELL_AURA_DEFLECT_SPELLS
    &Aura::spellAuraEffectNotImplemented,                                   // 288 SPELL_AURA_288
    &Aura::spellAuraEffectNotImplemented,                                   // 289 SPELL_AURA_289
    &Aura::spellAuraEffectNotImplemented,                                   // 290 SPELL_AURA_290
    &Aura::spellAuraEffectNotImplemented,                                   // 291 SPELL_AURA_291
    &Aura::SpellAuraCallStabledPet,                                         // 292 SPELL_AURA_CALL_STABLED_PET
    &Aura::spellAuraEffectNotImplemented,                                   // 293 SPELL_AURA_293
    &Aura::spellAuraEffectNotImplemented,                                   // 294 SPELL_AURA_294
    &Aura::spellAuraEffectNotImplemented,                                   // 295 SPELL_AURA_295
    &Aura::spellAuraEffectNotImplemented,                                   // 296 SPELL_AURA_296
    &Aura::spellAuraEffectNotImplemented,                                   // 297 SPELL_AURA_297
    &Aura::spellAuraEffectNotImplemented,                                   // 298 SPELL_AURA_298
    &Aura::spellAuraEffectNotImplemented,                                   // 299 SPELL_AURA_299
    &Aura::spellAuraEffectNotImplemented,                                   // 300 SPELL_AURA_300
    &Aura::spellAuraEffectNotImplemented,                                   // 301 SPELL_AURA_301
    &Aura::spellAuraEffectNotImplemented,                                   // 302 SPELL_AURA_302
    &Aura::spellAuraEffectNotImplemented,                                   // 303 SPELL_AURA_303
    &Aura::spellAuraEffectNotImplemented,                                   // 304 SPELL_AURA_304
    &Aura::spellAuraEffectNotImplemented,                                   // 305 SPELL_AURA_305
    &Aura::spellAuraEffectNotImplemented,                                   // 306 SPELL_AURA_306
    &Aura::spellAuraEffectNotImplemented,                                   // 307 SPELL_AURA_307
    &Aura::spellAuraEffectNotImplemented,                                   // 308 SPELL_AURA_308
    &Aura::spellAuraEffectNotImplemented,                                   // 309 SPELL_AURA_309
    &Aura::spellAuraEffectNotImplemented,                                   // 310 SPELL_AURA_310
    &Aura::spellAuraEffectNotImplemented,                                   // 311 SPELL_AURA_311
    &Aura::spellAuraEffectNotImplemented,                                   // 312 SPELL_AURA_312
    &Aura::spellAuraEffectNotImplemented,                                   // 313 SPELL_AURA_313
    &Aura::spellAuraEffectNotImplemented,                                   // 314 SPELL_AURA_314
    &Aura::spellAuraEffectNotImplemented,                                   // 315 SPELL_AURA_315
    &Aura::spellAuraEffectNotImplemented,                                   // 316 SPELL_AURA_ALLOW_HASTE_AFFECT_DURATION
#endif
#if VERSION_STRING >= Cata
    &Aura::spellAuraEffectNotImplemented,                                   // 317 SPELL_AURA_317
    &Aura::spellAuraEffectNotImplemented,                                   // 318 SPELL_AURA_318
    &Aura::spellAuraEffectNotImplemented,                                   // 319 SPELL_AURA_319
    &Aura::spellAuraEffectNotImplemented,                                   // 320 SPELL_AURA_320
    &Aura::spellAuraEffectNotImplemented,                                   // 321 SPELL_AURA_321
    &Aura::spellAuraEffectNotImplemented,                                   // 322 SPELL_AURA_322
    &Aura::spellAuraEffectNotImplemented,                                   // 323 SPELL_AURA_323
    &Aura::spellAuraEffectNotImplemented,                                   // 324 SPELL_AURA_324
    &Aura::spellAuraEffectNotImplemented,                                   // 325 SPELL_AURA_325
    &Aura::spellAuraEffectNotImplemented,                                   // 326 SPELL_AURA_326
    &Aura::spellAuraEffectNotImplemented,                                   // 327 SPELL_AURA_327
    &Aura::spellAuraEffectNotImplemented,                                   // 328 SPELL_AURA_328
    &Aura::spellAuraEffectNotImplemented,                                   // 329 SPELL_AURA_329
    &Aura::spellAuraEffectNotImplemented,                                   // 330 SPELL_AURA_330
    &Aura::spellAuraEffectNotImplemented,                                   // 331 SPELL_AURA_331
    &Aura::spellAuraEffectNotImplemented,                                   // 332 SPELL_AURA_332
    &Aura::spellAuraEffectNotImplemented,                                   // 333 SPELL_AURA_333
    &Aura::spellAuraEffectNotImplemented,                                   // 334 SPELL_AURA_334
    &Aura::spellAuraEffectNotImplemented,                                   // 335 SPELL_AURA_335
    &Aura::spellAuraEffectNotImplemented,                                   // 336 SPELL_AURA_336
    &Aura::spellAuraEffectNotImplemented,                                   // 337 SPELL_AURA_337
    &Aura::spellAuraEffectNotImplemented,                                   // 338 SPELL_AURA_338
    &Aura::spellAuraEffectNotImplemented,                                   // 339 SPELL_AURA_339
    &Aura::spellAuraEffectNotImplemented,                                   // 340 SPELL_AURA_340
    &Aura::spellAuraEffectNotImplemented,                                   // 341 SPELL_AURA_341
    &Aura::spellAuraEffectNotImplemented,                                   // 342 SPELL_AURA_342
    &Aura::spellAuraEffectNotImplemented,                                   // 343 SPELL_AURA_343
    &Aura::spellAuraEffectNotImplemented,                                   // 344 SPELL_AURA_344
    &Aura::spellAuraEffectNotImplemented,                                   // 345 SPELL_AURA_345
    &Aura::spellAuraEffectNotImplemented,                                   // 346 SPELL_AURA_346
    &Aura::spellAuraEffectNotImplemented,                                   // 347 SPELL_AURA_347
    &Aura::spellAuraEffectNotImplemented,                                   // 348 SPELL_AURA_348
    &Aura::spellAuraEffectNotImplemented,                                   // 349 SPELL_AURA_349
    &Aura::spellAuraEffectNotImplemented,                                   // 350 SPELL_AURA_350
    &Aura::spellAuraEffectNotImplemented,                                   // 351 SPELL_AURA_351
    &Aura::spellAuraEffectNotImplemented,                                   // 352 SPELL_AURA_352
    &Aura::spellAuraEffectNotImplemented,                                   // 353 SPELL_AURA_353
    &Aura::spellAuraEffectNotImplemented,                                   // 354 SPELL_AURA_354
    &Aura::spellAuraEffectNotImplemented,                                   // 355 SPELL_AURA_355
    &Aura::spellAuraEffectNotImplemented,                                   // 356 SPELL_AURA_356
    &Aura::spellAuraEffectNotImplemented,                                   // 357 SPELL_AURA_357
    &Aura::spellAuraEffectNotImplemented,                                   // 358 SPELL_AURA_358
    &Aura::spellAuraEffectNotImplemented,                                   // 359 SPELL_AURA_359
    &Aura::spellAuraEffectNotImplemented,                                   // 360 SPELL_AURA_360
    &Aura::spellAuraEffectNotImplemented,                                   // 361 SPELL_AURA_361
    &Aura::spellAuraEffectNotImplemented,                                   // 362 SPELL_AURA_362
    &Aura::spellAuraEffectNotImplemented,                                   // 363 SPELL_AURA_363
    &Aura::spellAuraEffectNotImplemented,                                   // 364 SPELL_AURA_364
    &Aura::spellAuraEffectNotImplemented,                                   // 365 SPELL_AURA_365
    &Aura::spellAuraEffectNotImplemented,                                   // 366 SPELL_AURA_366
    &Aura::spellAuraEffectNotImplemented,                                   // 367 SPELL_AURA_367
    &Aura::spellAuraEffectNotImplemented,                                   // 368 SPELL_AURA_368
    &Aura::spellAuraEffectNotImplemented,                                   // 369 SPELL_AURA_369
    &Aura::spellAuraEffectNotImplemented,                                   // 370 SPELL_AURA_370
#endif
#if VERSION_STRING >= Mop
    &Aura::spellAuraEffectNotImplemented,                                   // 371 SPELL_AURA_371
    &Aura::spellAuraEffectNotImplemented,                                   // 372 SPELL_AURA_372
    &Aura::spellAuraEffectNotImplemented,                                   // 373 SPELL_AURA_373
    &Aura::spellAuraEffectNotImplemented,                                   // 374 SPELL_AURA_374
    &Aura::spellAuraEffectNotImplemented,                                   // 375 SPELL_AURA_375
    &Aura::spellAuraEffectNotImplemented,                                   // 376 SPELL_AURA_376
    &Aura::spellAuraEffectNotImplemented,                                   // 377 SPELL_AURA_377
    &Aura::spellAuraEffectNotImplemented,                                   // 378 SPELL_AURA_378
    &Aura::spellAuraEffectNotImplemented,                                   // 379 SPELL_AURA_379
    &Aura::spellAuraEffectNotImplemented,                                   // 380 SPELL_AURA_380
    &Aura::spellAuraEffectNotImplemented,                                   // 381 SPELL_AURA_381
    &Aura::spellAuraEffectNotImplemented,                                   // 382 SPELL_AURA_382
    &Aura::spellAuraEffectNotImplemented,                                   // 383 SPELL_AURA_383
    &Aura::spellAuraEffectNotImplemented,                                   // 384 SPELL_AURA_384
    &Aura::spellAuraEffectNotImplemented,                                   // 385 SPELL_AURA_385
    &Aura::spellAuraEffectNotImplemented,                                   // 386 SPELL_AURA_386
    &Aura::spellAuraEffectNotImplemented,                                   // 387 SPELL_AURA_387
    &Aura::spellAuraEffectNotImplemented,                                   // 388 SPELL_AURA_388
    &Aura::spellAuraEffectNotImplemented,                                   // 389 SPELL_AURA_389
    &Aura::spellAuraEffectNotImplemented,                                   // 390 SPELL_AURA_390
    &Aura::spellAuraEffectNotImplemented,                                   // 391 SPELL_AURA_391
    &Aura::spellAuraEffectNotImplemented,                                   // 392 SPELL_AURA_392
    &Aura::spellAuraEffectNotImplemented,                                   // 393 SPELL_AURA_393
    &Aura::spellAuraEffectNotImplemented,                                   // 394 SPELL_AURA_394
    &Aura::spellAuraEffectNotImplemented,                                   // 395 SPELL_AURA_395
    &Aura::spellAuraEffectNotImplemented,                                   // 396 SPELL_AURA_396
    &Aura::spellAuraEffectNotImplemented,                                   // 397 SPELL_AURA_397
    &Aura::spellAuraEffectNotImplemented,                                   // 398 SPELL_AURA_398
    &Aura::spellAuraEffectNotImplemented,                                   // 399 SPELL_AURA_399
    &Aura::spellAuraEffectNotImplemented,                                   // 400 SPELL_AURA_400
    &Aura::spellAuraEffectNotImplemented,                                   // 401 SPELL_AURA_401
    &Aura::spellAuraEffectNotImplemented,                                   // 402 SPELL_AURA_402
    &Aura::spellAuraEffectNotImplemented,                                   // 403 SPELL_AURA_403
    &Aura::spellAuraEffectNotImplemented,                                   // 404 SPELL_AURA_404
    &Aura::spellAuraEffectNotImplemented,                                   // 405 SPELL_AURA_405
    &Aura::spellAuraEffectNotImplemented,                                   // 406 SPELL_AURA_406
    &Aura::spellAuraEffectNotImplemented,                                   // 407 SPELL_AURA_407
    &Aura::spellAuraEffectNotImplemented,                                   // 408 SPELL_AURA_408
    &Aura::spellAuraEffectNotImplemented,                                   // 409 SPELL_AURA_409
    &Aura::spellAuraEffectNotImplemented,                                   // 410 SPELL_AURA_410
    &Aura::spellAuraEffectNotImplemented,                                   // 411 SPELL_AURA_411
    &Aura::spellAuraEffectNotImplemented,                                   // 412 SPELL_AURA_412
    &Aura::spellAuraEffectNotImplemented,                                   // 413 SPELL_AURA_413
    &Aura::spellAuraEffectNotImplemented,                                   // 414 SPELL_AURA_414
    &Aura::spellAuraEffectNotImplemented,                                   // 415 SPELL_AURA_415
    &Aura::spellAuraEffectNotImplemented,                                   // 416 SPELL_AURA_416
    &Aura::spellAuraEffectNotImplemented,                                   // 417 SPELL_AURA_417
    &Aura::spellAuraEffectNotImplemented,                                   // 418 SPELL_AURA_418
    &Aura::spellAuraEffectNotImplemented,                                   // 419 SPELL_AURA_419
    &Aura::spellAuraEffectNotImplemented,                                   // 420 SPELL_AURA_420
    &Aura::spellAuraEffectNotImplemented,                                   // 421 SPELL_AURA_421
    &Aura::spellAuraEffectNotImplemented,                                   // 422 SPELL_AURA_422
    &Aura::spellAuraEffectNotImplemented,                                   // 423 SPELL_AURA_423
    &Aura::spellAuraEffectNotImplemented,                                   // 424 SPELL_AURA_424
    &Aura::spellAuraEffectNotImplemented,                                   // 425 SPELL_AURA_425
    &Aura::spellAuraEffectNotImplemented,                                   // 426 SPELL_AURA_426
    &Aura::spellAuraEffectNotImplemented,                                   // 427 SPELL_AURA_427
    &Aura::spellAuraEffectNotImplemented,                                   // 428 SPELL_AURA_428
    &Aura::spellAuraEffectNotImplemented,                                   // 429 SPELL_AURA_429
    &Aura::spellAuraEffectNotImplemented,                                   // 430 SPELL_AURA_430
    &Aura::spellAuraEffectNotImplemented,                                   // 431 SPELL_AURA_431
    &Aura::spellAuraEffectNotImplemented,                                   // 432 SPELL_AURA_432
    &Aura::spellAuraEffectNotImplemented,                                   // 433 SPELL_AURA_433
    &Aura::spellAuraEffectNotImplemented,                                   // 434 SPELL_AURA_434
    &Aura::spellAuraEffectNotImplemented,                                   // 435 SPELL_AURA_435
    &Aura::spellAuraEffectNotImplemented,                                   // 436 SPELL_AURA_436
    &Aura::spellAuraEffectNotImplemented,                                   // 437 SPELL_AURA_437
#endif
};

const char* SpellAuraNames[TOTAL_SPELL_AURAS] =
{
    "SPELL_AURA_NONE",                                                      //   0 None
    "SPELL_AURA_BIND_SIGHT",                                                //   1 Bind Sight
    "SPELL_AURA_MOD_POSSESS",                                               //   2 Mod Possess
    "SPELL_AURA_PERIODIC_DAMAGE",                                           //   3 Periodic Damage
    "SPELL_AURA_DUMMY",                                                     //   4 Script Aura
    "SPELL_AURA_MOD_CONFUSE",                                               //   5 Mod Confuse
    "SPELL_AURA_MOD_CHARM",                                                 //   6 Mod Charm
    "SPELL_AURA_MOD_FEAR",                                                  //   7 Mod Fear
    "SPELL_AURA_PERIODIC_HEAL",                                             //   8 Periodic Heal
    "SPELL_AURA_MOD_ATTACK_SPEED",                                          //   9 Mod Attack Speed
    "SPELL_AURA_MOD_THREAT_GENERATED",                                      //  10 Mod Threat
    "SPELL_AURA_MOD_TAUNT",                                                 //  11 Taunt
    "SPELL_AURA_MOD_STUN",                                                  //  12 Stun
    "SPELL_AURA_MOD_DAMAGE_DONE",                                           //  13 Mod Damage Done
    "SPELL_AURA_MOD_DAMAGE_TAKEN",                                          //  14 Mod Damage Taken
    "SPELL_AURA_DAMAGE_SHIELD",                                             //  15 Damage Shield
    "SPELL_AURA_MOD_STEALTH",                                               //  16 Mod Stealth
    "SPELL_AURA_MOD_STEALTH_DETECTION",                                     //  17 Mod Detect
    "SPELL_AURA_MOD_INVISIBILITY",                                          //  18 Mod Invisibility
    "SPELL_AURA_MOD_INVISIBILITY_DETECTION",                                //  19 Mod Invisibility Detection
    "SPELL_AURA_PERIODIC_HEAL_PCT",                                         //  20
    "SPELL_AURA_PERIODIC_POWER_PCT",                                        //  21
    "SPELL_AURA_MOD_RESISTANCE",                                            //  22 Mod Resistance
    "SPELL_AURA_PERIODIC_TRIGGER_SPELL",                                    //  23 Periodic Trigger
    "SPELL_AURA_PERIODIC_ENERGIZE",                                         //  24 Periodic Energize
    "SPELL_AURA_MOD_PACIFY",                                                //  25 Pacify
    "SPELL_AURA_MOD_ROOT",                                                  //  26 Root
    "SPELL_AURA_MOD_SILENCE",                                               //  27 Silence
    "SPELL_AURA_REFLECT_SPELLS",                                            //  28 Reflect Spells %
    "SPELL_AURA_MOD_STAT",                                                  //  29 Mod Stat
    "SPELL_AURA_MOD_SKILL",                                                 //  30 Mod Skill
    "SPELL_AURA_MOD_INCREASE_SPEED",                                        //  31 Mod Speed
    "SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED",                                //  32 Mod Speed Mounted
    "SPELL_AURA_MOD_DECREASE_SPEED",                                        //  33 Mod Speed Slow
    "SPELL_AURA_MOD_INCREASE_HEALTH",                                       //  34 Mod Increase Health
    "SPELL_AURA_MOD_INCREASE_ENERGY",                                       //  35 Mod Increase Energy
    "SPELL_AURA_MOD_SHAPESHIFT",                                            //  36 Shapeshift
    "SPELL_AURA_MOD_EFFECT_IMMUNITY",                                       //  37 Immune Effect
    "SPELL_AURA_MOD_STATE_IMMUNITY",                                        //  38 Immune State
    "SPELL_AURA_MOD_SCHOOL_IMMUNITY",                                       //  39 Immune School
    "SPELL_AURA_MOD_DMG_IMMUNITY",                                          //  40 Immune Damage
    "SPELL_AURA_MOD_DISPEL_IMMUNITY",                                       //  41 Immune Dispel Type
    "SPELL_AURA_PROC_TRIGGER_SPELL",                                        //  42 Proc Trigger Spell
    "SPELL_AURA_PROC_TRIGGER_DAMAGE",                                       //  43 Proc Trigger Damage
    "SPELL_AURA_TRACK_CREATURES",                                           //  44 Track Creatures
    "SPELL_AURA_TRACK_RESOURCES",                                           //  45 Track Resources
    "SPELL_AURA_46",                                                        //  46 Mod Parry Skill
    "SPELL_AURA_MOD_PARRY_PERC",                                            //  47 Mod Parry Percent
    "SPELL_AURA_48",                                                        //  48 Mod Dodge Skill
    "SPELL_AURA_MOD_DODGE_PERC",                                            //  49 Mod Dodge Percent
    "SPELL_AURA_50",                                                        //  50 Mod Block Skill
    "SPELL_AURA_MOD_BLOCK_PERC",                                            //  51 Mod Block Percent
    "SPELL_AURA_MOD_CRIT_PERC",                                             //  52 Mod Crit Percent
    "SPELL_AURA_PERIODIC_LEECH",                                            //  53 Periodic Leech
    "SPELL_AURA_MOD_HIT_CHANCE",                                            //  54 Mod Hit Chance
    "SPELL_AURA_MOD_SPELL_HIT_CHANCE",                                      //  55 Mod Spell Hit Chance
    "SPELL_AURA_TRANSFORM",                                                 //  56 Transform
    "SPELL_AURA_MOD_SPELL_CRIT_CHANCE",                                     //  57 Mod Spell Crit Chance
    "SPELL_AURA_INCREASE_SWIM_SPEED",                                       //  58 Mod Speed Swim
    "SPELL_AURA_MOD_CRATURE_DMG_DONE",                                      //  59 Mod Creature Dmg Done
    "SPELL_AURA_PACIFY_SILENCE",                                            //  60 Pacify & Silence
    "SPELL_AURA_MOD_SCALE",                                                 //  61 Mod Scale
    "SPELL_AURA_PERIODIC_HEALTH_FUNNEL",                                    //  62 Periodic Health Funnel
    "SPELL_AURA_63",                                                        //  63 Periodic Mana Funnel
    "SPELL_AURA_PERIODIC_MANALEECH",                                        //  64 Periodic Mana Leech
    "SPELL_AURA_MOD_CASTING_SPEED",                                         //  65 Haste - Spells
    "SPELL_AURA_FEIGN_DEATH",                                               //  66 Feign Death
    "SPELL_AURA_MOD_DISARM",                                                //  67 Disarm
    "SPELL_AURA_MOD_STALKED",                                               //  68 Mod Stalked
    "SPELL_AURA_SCHOOL_ABSORB",                                             //  69 School Absorb
    "SPELL_AURA_70",                                                        //  70 Extra Attacks
    "SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL",                              //  71 Mod School Spell Crit Chance
    "SPELL_AURA_MOD_POWER_COST",                                            //  72 Mod Power Cost
    "SPELL_AURA_MOD_POWER_COST_SCHOOL",                                     //  73 Mod School Power Cost
    "SPELL_AURA_REFLECT_SPELLS_SCHOOL",                                     //  74 Reflect School Spells %
    "SPELL_AURA_MOD_LANGUAGE",                                              //  75 Mod Language
    "SPELL_AURA_ADD_FAR_SIGHT",                                             //  76 Far Sight
    "SPELL_AURA_MECHANIC_IMMUNITY",                                         //  77 Immune Mechanic
    "SPELL_AURA_MOUNTED",                                                   //  78 Mounted
    "SPELL_AURA_MOD_DAMAGE_PERC_DONE",                                      //  79 Mod Dmg %
    "SPELL_AURA_MOD_PERC_STAT",                                             //  80 Mod Stat %
    "SPELL_AURA_SPLIT_DAMAGE",                                              //  81 Split Damage
    "SPELL_AURA_WATER_BREATHING",                                           //  82 Water Breathing
    "SPELL_AURA_MOD_BASE_RESISTANCE",                                       //  83 Mod Base Resistance
    "SPELL_AURA_MOD_HEALTH_REGEN",                                          //  84 Mod Health Regen
    "SPELL_AURA_MOD_POWER_REGEN",                                           //  85 Mod Power Regen
    "SPELL_AURA_CHANNEL_DEATH_ITEM",                                        //  86 Create Death Item
    "SPELL_AURA_MOD_DAMAGE_PERC_TAKEN",                                     //  87 Mod Dmg % Taken
    "SPELL_AURA_MOD_HEALTH_REGEN_PERCENT",                                  //  88 Mod Health Regen Percent
    "SPELL_AURA_PERIODIC_DAMAGE_PERCENT",                                   //  89 Periodic Damage Percent
    "SPELL_AURA_MOD_RESIST_CHANCE",                                         //  90 Mod Resist Chance
    "SPELL_AURA_MOD_DETECT_RANGE",                                          //  91 Mod Detect Range
    "SPELL_AURA_PREVENTS_FLEEING",                                          //  92 Prevent Fleeing
    "SPELL_AURA_MOD_UNATTACKABLE",                                          //  93 Mod Uninteractible
    "SPELL_AURA_INTERRUPT_REGEN",                                           //  94 Interrupt Regen
    "SPELL_AURA_GHOST",                                                     //  95 Ghost
    "SPELL_AURA_MAGNET",                                                    //  96 Spell Magnet
    "SPELL_AURA_MANA_SHIELD",                                               //  97 Mana Shield
    "SPELL_AURA_SKILL_TALENT",                                              //  98 Mod Skill Talent
    "SPELL_AURA_MOD_ATTACK_POWER",                                          //  99 Mod Attack Power
    "SPELL_AURA_VISIBLE",                                                   // 100 Auras Visible
    "SPELL_AURA_MOD_RESISTANCE_PCT",                                        // 101 Mod Resistance %
    "SPELL_AURA_MOD_CREATURE_ATTACK_POWER",                                 // 102 Mod Creature Attack Power
    "SPELL_AURA_MOD_TOTAL_THREAT",                                          // 103 Mod Total Threat (Fade)
    "SPELL_AURA_WATER_WALK",                                                // 104 Water Walk
    "SPELL_AURA_FEATHER_FALL",                                              // 105 Feather Fall
    "SPELL_AURA_HOVER",                                                     // 106 Hover
    "SPELL_AURA_ADD_FLAT_MODIFIER",                                         // 107 Add Flat Modifier
    "SPELL_AURA_ADD_PCT_MOD",                                               // 108 Add % Modifier
    "SPELL_AURA_ADD_CLASS_TARGET_TRIGGER",                                  // 109 Add Class Target Trigger
    "SPELL_AURA_MOD_POWER_REGEN_PERCENT",                                   // 110 Mod Power Regen %
    "SPELL_AURA_111",                                                       // 111 Add Class Caster Hit Trigger
    "SPELL_AURA_OVERRIDE_CLASS_SCRIPTS",                                    // 112 Override Class Scripts
    "SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN",                                   // 113 Mod Ranged Dmg Taken
    "SPELL_AURA_114",                                                       // 114 Mod Ranged % Dmg Taken
    "SPELL_AURA_MOD_HEALING",                                               // 115 Mod Healing
    "SPELL_AURA_MOD_HEALTH_REGEN_DURING_COMBAT",                            // 116 Mod Health Regen During Combat
    "SPELL_AURA_MOD_MECHANIC_RESISTANCE",                                   // 117 Mod Mechanic Resistance
    "SPELL_AURA_MOD_HEALING_PCT",                                           // 118 Mod Healing %
    "SPELL_AURA_119",                                                       // 119 Share Pet Tracking
    "SPELL_AURA_UNTRACKABLE",                                               // 120 Untrackable
    "SPELL_AURA_EMPHATY",                                                   // 121 Empathy (Lore, whatever)
    "SPELL_AURA_MOD_OFFHANDDAMAGE_PCT",                                     // 122 Mod Offhand Dmg %
    "SPELL_AURA_MOD_PENETRATION",                                           // 123 Mod Power Cost % (armor penetration & spell penetration. NOT power cost!)
    "SPELL_AURA_MOD_RANGED_ATTACK_POWER",                                   // 124 Mod Ranged Attack Power
    "SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN",                                    // 125 Mod Melee Dmg Taken
    "SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT",                                // 126 Mod Melee % Dmg Taken
    "SPELL_AURA_RAP_ATTACKER_BONUS",                                        // 127 Rngd Atk Pwr Attckr Bonus
    "SPELL_AURA_MOD_POSSESS_PET",                                           // 128 Mod Possess Pet
    "SPELL_AURA_MOD_INCREASE_SPEED_ALWAYS",                                 // 129 Mod Speed Always
    "SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED",                                // 130 Mod Mounted Speed Always
    "SPELL_AURA_MOD_CREATURE_RANGED_ATTACK_POWER",                          // 131 Mod Creature Ranged Attack Power
    "SPELL_AURA_MOD_INCREASE_ENERGY_PERC",                                  // 132 Mod Increase Energy %
    "SPELL_AURA_MOD_INCREASE_HEALTH_PERC",                                  // 133 Mod Max Health %
    "SPELL_AURA_MOD_MANA_REG_INTERRUPT",                                    // 134 Mod Interrupted Mana Regen
    "SPELL_AURA_MOD_HEALING_DONE",                                          // 135 Mod Healing Done
    "SPELL_AURA_MOD_HEALING_DONE_PCT",                                      // 136 Mod Healing Done %
    "SPELL_AURA_MOD_TOTAL_STAT_PERC",                                       // 137 Mod Total Stat %
    "SPELL_AURA_MOD_HASTE",                                                 // 138 Haste - Melee
    "SPELL_AURA_FORCE_REACTION",                                            // 139 Force Reaction
    "SPELL_AURA_MOD_RANGED_HASTE",                                          // 140 Haste - Ranged
    "SPELL_AURA_MOD_RANGED_AMMO_HASTE",                                     // 141 Haste - Ranged (Ammo Only)
    "SPELL_AURA_MOD_BASE_RESISTANCE_PERC",                                  // 142 Mod Base Resistance %
    "SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE",                                  // 143 Mod Resistance Exclusive
    "SPELL_AURA_SAFE_FALL",                                                 // 144 Safe Fall
    "SPELL_AURA_145",                                                       // 145 Charisma
    "SPELL_AURA_146",                                                       // 146 Persuaded
    "SPELL_AURA_147",                                                       // 147 Add Creature Immunity // https://www.wowhead.com/spell=36798/
    "SPELL_AURA_RETAIN_COMBO_POINTS",                                       // 148 Retain Combo Points
    "SPELL_AURA_RESIST_PUSHBACK",                                           // 149 Resist Pushback // Simply resist spell casting delay
    "SPELL_AURA_MOD_SHIELD_BLOCK_PCT",                                      // 150 Mod Shield Block %
    "SPELL_AURA_TRACK_STEALTHED",                                           // 151 Track Stealthed
    "SPELL_AURA_MOD_DETECTED_RANGE",                                        // 152 Mod Detected Range
    "SPELL_AURA_SPLIT_DAMAGE_FLAT",                                         // 153 Split Damage Flat
    "SPELL_AURA_MOD_STEALTH_LEVEL",                                         // 154 Stealth Level Modifier
    "SPELL_AURA_MOD_UNDERWATER_BREATHING",                                  // 155 Mod Water Breathing
    "SPELL_AURA_MOD_REPUTATION_ADJUST",                                     // 156 Mod Reputation Gain
    "SPELL_AURA_157",                                                       // 157 Mod Pet Damage
    "SPELL_AURA_MOD_BLOCK_VALUE",                                           // 158 used Apply Aura: Mod Shield Block // https://classic.wowhead.com/spell=25036/
    "SPELL_AURA_NO_PVP_CREDIT",                                             // 159 used Apply Aura: No PVP Credit // https://classic.wowhead.com/spell=2479/
    "SPELL_AURA_160",                                                       // 160 used Apply Aura: Mod Side/Rear PBAE Damage Taken % // https://classic.wowhead.com/spell=23198
    "SPELL_AURA_MOD_HEALTH_REGEN_ALWAYS",                                   // 161 Mod Health Regen Always (In combat as well)
    "SPELL_AURA_PERIODIC_POWER_BURN",                                       // 162 Power Burn
    "SPELL_AURA_MOD_CRIT_DMG_PHYSICAL",                                     // 163 missing Apply Aura: Mod Critical Damage Bonus (Physical)
    "SPELL_AURA_164",                                                       // 164 missing used // test spell
    "SPELL_AURA_AP_ATTACKER_BONUS",                                         // 165 Melee AP Attacker Bonus
    "SPELL_AURA_MOD_P_ATTACK_POWER",                                        // 166 missing used Apply Aura: Mod Attack Power % // https://classic.wowhead.com/spell=30803/
    "SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT",                               // 167 Mod Ranged Attack Power % // missing // http://www.thottbot.com/s34485
    "SPELL_AURA_INCREASE_DAMAGE_TYPE_PCT",                                  // 168 missing used // Apply Aura: Increase Damage % *type* // https://classic.wowhead.com/spell=24991
    "SPELL_AURA_INCREASE_CRICTICAL_TYPE_PCT",                               // 169 missing used // Apply Aura: Increase Critical % *type* // https://classic.wowhead.com/spell=24293
    "SPELL_AURA_170",                                                       // 170 Detect Amore // Apply Aura: Detect Amore // https://classic.wowhead.com/spell=26802
    "SPELL_AURA_INCREASE_PARTY_SPEED",                                      // 171
    "SPELL_AURA_INCREASE_MOVEMENT_AND_MOUNTED_SPEED",                       // 172 // used // Apply Aura: Increase Movement and Mounted Speed (Non-Stacking) // https://classic.wowhead.com/spell=26022 2e effect
    "SPELL_AURA_173",                                                       // 173 // Apply Aura: Allow Champion Spells
    "SPELL_AURA_INCREASE_SPELL_DAMAGE_BY_ATTRIBUTE",                        // 174 // used // Apply Aura: Increase Spell Damage by % Spirit (Spells) // https://classic.wowhead.com/spell=15031
    "SPELL_AURA_INCREASE_HEALING_BY_ATTRIBUTE",                             // 175 // used // Apply Aura: Increase Spell Healing by % Spirit // https://classic.wowhead.com/spell=15031
    "SPELL_AURA_SPIRIT_OF_REDEMPTION",                                      // 176 // used // Apply Aura: Spirit of Redemption
    "SPELL_AURA_177",                                                       // 177 // used // Apply Aura: Area Charm // https://classic.wowhead.com/spell=26740
    "SPELL_AURA_178",                                                       // 178 missing // Apply Aura: Increase Debuff Resistance 
    "SPELL_AURA_INCREASE_ATTACKER_SPELL_CRIT",                              // 179 Apply Aura: Increase Attacker Spell Crit % *type* // https://classic.wowhead.com/spell=12579
    "SPELL_AURA_180",                                                       // 180 // Apply Aura: Increase Spell Damage *type* // https://classic.wowhead.com/spell=29113
    "SPELL_AURA_181",                                                       // 181 missing
    "SPELL_AURA_INCREASE_ARMOR_BY_PCT_INT",                                 // 182 missing // Apply Aura: Increase Armor by % of Intellect // https://classic.wowhead.com/spell=28574
    "SPELL_AURA_183",                                                       // 183 // used // Apply Aura: Decrease Critical Threat by % (Spells) // https://classic.wowhead.com/spell=28746/ (SPELL_AURA_MOD_CRITICAL_THREAT)
    "SPELL_AURA_REDUCE_ATTACKER_M_HIT_CHANCE",                              // 184 // Apply Aura: Reduces Attacker Chance to Hit with Melee // http://www.thottbot.com/s31678
    "SPELL_AURA_REDUCE_ATTACKER_R_HIT_CHANCE",                              // 185 // Apply Aura: Reduces Attacker Chance to Hit with Ranged // https://classic.wowhead.com/spell=30895
    "SPELL_AURA_REDUCE_ATTACKER_S_HIT_CHANCE",                              // 186 // Apply Aura: Reduces Attacker Chance to Hit with Spells (Spells) // https://classic.wowhead.com/spell=30895
    "SPELL_AURA_REDUCE_ENEMY_M_CRIT_CHANCE",                                // 187 missing // used // Apply Aura: Reduces Attacker Chance to Crit with Melee (Ranged?)
    "SPELL_AURA_REDUCE_ENEMY_R_CRIT_CHANCE",                                // 188 missing // used // Apply Aura: Reduces Attacker Chance to Crit with Ranged (Melee?)
    "SPELL_AURA_INCREASE_RATING",                                           // 189 missing // Apply Aura: Increases Rating
    "SPELL_AURA_INCREASE_REP_GAIN_PCT",                                     // 190 // used // Apply Aura: Increases Reputation Gained by % // https://classic.wowhead.com/spell=30754/ (SPELL_AURA_MOD_FACTION_REPUTATION_GAIN)
    "SPELL_AURA_LIMIT_SPEED",                                               // 191 speed limit // https://classic.wowhead.com/spell=29894/
#if VERSION_STRING >= TBC
    "SPELL_AURA_MELEE_HASTE",                                               // 192 Apply Aura: Melee Slow %
    "SPELL_AURA_INCREASE_TIME_BETWEEN_ATTACKS_PCT",                         // 193 Apply Aura: Increase Time Between Attacks (Melee, Ranged and Spell) by %
    "SPELL_AURA_194",                                                       // 194 NOT USED ANYMORE - 174 used instead // Apply Aura: Increase Spell Damage by % of Intellect (All)
    "SPELL_AURA_195",                                                       // 195 NOT USED ANYMORE - 175 used instead // Apply Aura: Increase Healing by % of Intellect
    "SPELL_AURA_196",                                                       // 196 Apply Aura: Mod All Weapon Skills (6)
    "SPELL_AURA_MOD_ATTACKER_CRIT_CHANCE",                                  // 197 Apply Aura: Reduce Attacker Critical Hit Chance by %
    "SPELL_AURA_INCREASE_ALL_WEAPON_SKILL",                                 // 198
    "SPELL_AURA_INCREASE_HIT_RATE",                                         // 199 Apply Aura: Increases Spell % To Hit (Fire, Nature, Frost)
    "SPELL_AURA_200",                                                       // 200 Increases experience earned by $s1%. Lasts $d.
    "SPELL_AURA_ALLOW_FLIGHT",                                              // 201 isn't it same like 206 and 207?
    "SPELL_AURA_FINISHING_MOVES_CANNOT_BE_DODGED",                          // 202 Finishing moves cannot be dodged - 32601, 44452
    "SPELL_AURA_REDUCE_CRIT_MELEE_ATTACK_DMG",                              // 203 Apply Aura: Reduces Attacker Critical Hit Damage with Melee by %
    "SPELL_AURA_REDUCE_CRIT_RANGED_ATTACK_DMG",                             // 204 Apply Aura: Reduces Attacker Critical Hit Damage with Ranged by %
    "SPELL_AURA_205",                                                       // 205 "School" Vulnerability
    "SPELL_AURA_ENABLE_FLIGHT",                                             // 206 Take flight on a worn old carpet. - Spell 43343
    "SPELL_AURA_ENABLE_FLIGHT",                                             // 207 set fly mod flight speed?
    "SPELL_AURA_ENABLE_FLIGHT_WITH_UNMOUNTED_SPEED",                        // 208 mod flight speed?
    "SPELL_AURA_209",                                                       // 209 mod flight speed?
    "SPELL_AURA_210",                                                       // 210 commentator's command - spell 42009
    "SPELL_AURA_INCREASE_FLIGHT_SPEED",                                     // 211 Apply Aura: Increase Ranged Atk Power by % of stat
    "SPELL_AURA_INCREASE_RAP_BY_STAT_PCT",                                  // 212 Apply Aura: Increase Rage from Damage Dealt by %
    "SPELL_AURA_INCREASE_RAGE_FROM_DAMAGE_DEALT_PCT",                       // 213 // Tamed Pet Passive (DND)
    "SPELL_AURA_214",                                                       // 214 // arena preparation buff - cancel soul shard requirement?
    "SPELL_AURA_REMOVE_REAGENT_COST",                                       // 215 Increases casting time %, reuse existing handler...
    "SPELL_AURA_MOD_CASTING_SPEED",                                         // 216 // not used
    "SPELL_AURA_217",                                                       // 217 // increases time between ranged attacks
    "SPELL_AURA_218",                                                       // 218 Regenerate mana equal to $s1% of your Intellect every 5 sec, even while casting
    "SPELL_AURA_REGEN_MANA_STAT_PCT",                                       // 219 Increases your healing spells by up to $s1% of your Strength // increases your critical strike rating by 35% of your spirit // Molten Armor only?
    "SPELL_AURA_SPELL_HEALING_STAT_PCT",                                    // 220 Detaunt "Ignores an enemy, forcing the caster to not attack it unless there is no other target nearby. When the effect wears off, the creature will attack the most threatening target."
    "SPELL_AURA_221",                                                       // 221 // not used
    "SPELL_AURA_222",                                                       // 222 // used in one spell, cold stare 43593
    "SPELL_AURA_223",                                                       // 223 // not used
    "SPELL_AURA_224",                                                       // 224
    "SPELL_AURA_225",                                                       // 225 // Prayer of Mending "Places a spell on the target that heals them for $s1 the next time they take damage.  When the heal occurs, Prayer of Mending jumps to a raid member within $a1 yards.  Jumps up to $n times and lasts $d after each jump.  This spell can only be placed on one target at a time."
    "SPELL_AURA_PERIODIC_TRIGGER_DUMMY",                                    // 226 // used in brewfest spells, headless horseman, Aspect of the Viper
    "SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE",                         // 227 // Used by Mind Flay, Siege Turrets 'Machine gun' and a few other spells.
    "SPELL_AURA_228",                                                       // 228 Stealth Detection. https://www.wowhead.com/spell=34709 // handled in Unit::canSee
    "SPELL_AURA_REDUCE_AOE_DAMAGE_TAKEN",                                   // 229 Apply Aura:Reduces the damage your pet takes from area of effect attacks // http://www.thottbot.com/s35694
    "SPELL_AURA_INCREASE_MAX_HEALTH",                                       // 230 Used by Increase Max Health (commanding shout);
    "SPELL_AURA_PROC_TRIGGER_SPELL",                                        // 231 curse a target https://www.wowhead.com/spell=40303
    "SPELL_AURA_REDUCE_EFFECT_DURATION",                                    // 232 // Reduces duration of Magic effects by $s2%.
    "SPELL_AURA_233",                                                       // 233 // Beer Goggles
    "SPELL_AURA_REDUCE_EFFECT_DURATION",                                    // 234 Apply Aura: Reduces Silence or Interrupt effects, Item spell magic https://www.wowhead.com/spell=42184
    "SPELL_AURA_235",                                                       // 235 // 33206 Instantly reduces a friendly target's threat by $44416s1%, reduces all damage taken by $s1% and increases resistance to Dispel mechanics by $s2% for $d.
    "HANDLE_AURA_CONTROL_VEHICLE",                                          // 236
    "SPELL_AURA_MOD_HEALING_BY_AP",                                         // 237 // increase spell healing by X pct from attack power
    "SPELL_AURA_MOD_SPELL_DAMAGE_BY_AP",                                    // 238 // increase spell dmg by X pct from attack power
    "SPELL_AURA_MOD_SCALE",                                                 // 239
    "SPELL_AURA_EXPERTISE",                                                 // 240
    "SPELL_AURA_FORCE_MOVE_FORWARD",                                        // 241 makes target to run forward
    "SPELL_AURA_242",                                                       // 242
    "SPELL_AURA_243",                                                       // 243
    "SPELL_AURA_COMPREHEND_LANG",                                           // 244 allows target to understand itself while talking in different language
    "SPELL_AURA_245",                                                       // 245
    "SPELL_AURA_246",                                                       // 246
    "SPELL_AURA_MIRROR_IMAGE",                                              // 247
    "SPELL_AURA_MOD_COMBAT_RESULT_CHANCE",                                  // 248
    "SPELL_AURA_CONVERT_RUNE",                                              // 249 Convert rune
    "SPELL_AURA_ADD_HEALTH",                                                // 250
    "SPELL_AURA_251",                                                       // 251 Mod Enemy Dodge
    "SPELL_AURA_252",                                                       // 252 Reduces the target's ranged, melee attack, and casting speed by X pct for Y sec.
    "SPELL_AURA_BLOCK_MULTIPLE_DAMAGE",                                     // 253
    "SPELL_AURA_MOD_DISARM",                                                // 254
    "SPELL_AURA_MOD_MECHANIC_DMG_TAKEN_PCT",                                // 255
    "SPELL_AURA_REMOVE_REAGENT_COST",                                       // 256 Remove reagent cost
    "SPELL_AURA_257",                                                       // 257 Mod Target Resist By Spell Class (does damage in the form of X damage, ignoring all resistances, absorption, and immunity mechanics. - http://thottbot.com/s47271)
    "SPELL_AURA_258",                                                       // 258 Mod Spell Visual
    "SPELL_AURA_259",                                                       // 259 Mod Periodic Damage Taken Pct - Periodic Shadow damage taken increased by 3% // http://thottbot.com/s60448
    "SPELL_AURA_260",                                                       // 260 Screen Effect
    "SPELL_AURA_PHASE",                                                     // 261
#endif
#if VERSION_STRING >= WotLK
    "SPELL_AURA_262",                                                       // 262
    "SPELL_AURA_ALLOW_ONLY_ABILITY",                                        // 263
    "SPELL_AURA_264",                                                       // 264
    "SPELL_AURA_265",                                                       // 265
    "SPELL_AURA_266",                                                       // 266
    "SPELL_AURA_267",                                                       // 267 Prevent the application of harmful magical effects. used only by Dk's Anti Magic Shell
    "SPELL_AURA_INCREASE_AP_BY_STAT_PCT",                                   // 268 Mental Dexterity (increases ap by x% of intellect)
    "SPELL_AURA_269",                                                       // 269 Damage reduction effects ignored. (?) - http://thottbot.com/s57318
    "SPELL_AURA_270",                                                       // 270 Ignore target resist
    "SPELL_AURA_MOD_SPELL_DAMAGE_DOT_PCT",                                  // 271
    "SPELL_AURA_272",                                                       // 272
    "SPELL_AURA_273",                                                       // 273 Some sort of dummy aura? https://www.wowhead.com/spell=54844 + https://classic.wowhead.com/spell=26659
    "SPELL_AURA_CONSUME_NO_AMMO",                                           // 274 Consumes no ammo
    "SPELL_AURA_275",                                                       // 275 Ignores form/shapeshift requirements
    "SPELL_AURA_276",                                                       // 276 Mod Damage % Mechanic
    "SPELL_AURA_277",                                                       // 277
    "SPELL_AURA_MOD_DISARM",                                                // 278
    "SPELL_AURA_MIRROR_IMAGE2",                                             // 279 Modify models(?)
    "SPELL_AURA_MOD_IGNORE_ARMOR_PCT",                                      // 280
    "SPELL_AURA_281",                                                       // 281 Mod Honor gain increased by X pct. Final Reward Honor increased by X pct for Y Rank and above. https://www.wowhead.com/spell=58560 && https://www.wowhead.com/spell=58557/
    "SPELL_AURA_MOD_BASE_HEALTH",                                           // 282
    "SPELL_AURA_MOD_HEALING_PCT",                                           // 283 Increases all healing received by X pct
    "SPELL_AURA_284",                                                       // 284 not used by any spells (3.08a)
    "SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR",                                 // 285
    "SPELL_AURA_286",                                                       // 286
    "SPELL_AURA_DEFLECT_SPELLS",                                            // 287
    "SPELL_AURA_288",                                                       // 288 not used by any spells (3.09) except 1 test spell.
    "SPELL_AURA_289",                                                       // 289
    "SPELL_AURA_290",                                                       // 290
    "SPELL_AURA_291",                                                       // 291
    "SPELL_AURA_CALL_STABLED_PET",                                          // 292 call stabled pet
    "SPELL_AURA_293",                                                       // 293 2 test spells
    "SPELL_AURA_294",                                                       // 294 2 spells, possible prevent mana regen
    "SPELL_AURA_295",                                                       // 295
    "SPELL_AURA_296",                                                       // 296
    "SPELL_AURA_297",                                                       // 297
    "SPELL_AURA_298",                                                       // 298
    "SPELL_AURA_299",                                                       // 299
    "SPELL_AURA_300",                                                       // 300
    "SPELL_AURA_301",                                                       // 301
    "SPELL_AURA_302",                                                       // 302
    "SPELL_AURA_303",                                                       // 303
    "SPELL_AURA_304",                                                       // 304
    "SPELL_AURA_305",                                                       // 305
    "SPELL_AURA_306",                                                       // 306
    "SPELL_AURA_307",                                                       // 307
    "SPELL_AURA_308",                                                       // 308
    "SPELL_AURA_309",                                                       // 309
    "SPELL_AURA_310",                                                       // 310
    "SPELL_AURA_311",                                                       // 311
    "SPELL_AURA_312",                                                       // 312
    "SPELL_AURA_313",                                                       // 313
    "SPELL_AURA_314",                                                       // 314
    "SPELL_AURA_315",                                                       // 315
    "SPELL_AURA_ALLOW_HASTE_AFFECT_DURATION",                               // 316
#endif
#if VERSION_STRING >= Cata
    "SPELL_AURA_317",                                                       // 317
    "SPELL_AURA_318",                                                       // 318
    "SPELL_AURA_319",                                                       // 319
    "SPELL_AURA_320",                                                       // 320
    "SPELL_AURA_321",                                                       // 321
    "SPELL_AURA_322",                                                       // 322
    "SPELL_AURA_323",                                                       // 323
    "SPELL_AURA_324",                                                       // 324
    "SPELL_AURA_325",                                                       // 325
    "SPELL_AURA_326",                                                       // 326
    "SPELL_AURA_327",                                                       // 327
    "SPELL_AURA_328",                                                       // 328
    "SPELL_AURA_329",                                                       // 329
    "SPELL_AURA_330",                                                       // 330
    "SPELL_AURA_331",                                                       // 331
    "SPELL_AURA_332",                                                       // 332
    "SPELL_AURA_333",                                                       // 333
    "SPELL_AURA_334",                                                       // 334
    "SPELL_AURA_335",                                                       // 335
    "SPELL_AURA_336",                                                       // 336
    "SPELL_AURA_337",                                                       // 337
    "SPELL_AURA_338",                                                       // 338
    "SPELL_AURA_339",                                                       // 339
    "SPELL_AURA_340",                                                       // 340
    "SPELL_AURA_341",                                                       // 341
    "SPELL_AURA_342",                                                       // 342
    "SPELL_AURA_343",                                                       // 343
    "SPELL_AURA_344",                                                       // 344
    "SPELL_AURA_345",                                                       // 345
    "SPELL_AURA_346",                                                       // 346
    "SPELL_AURA_347",                                                       // 347
    "SPELL_AURA_348",                                                       // 348
    "SPELL_AURA_349",                                                       // 349
    "SPELL_AURA_350",                                                       // 350
    "SPELL_AURA_351",                                                       // 351
    "SPELL_AURA_352",                                                       // 352
    "SPELL_AURA_353",                                                       // 353
    "SPELL_AURA_354",                                                       // 354
    "SPELL_AURA_355",                                                       // 355
    "SPELL_AURA_356",                                                       // 356
    "SPELL_AURA_357",                                                       // 357
    "SPELL_AURA_358",                                                       // 358
    "SPELL_AURA_359",                                                       // 359
    "SPELL_AURA_360",                                                       // 360
    "SPELL_AURA_361",                                                       // 361
    "SPELL_AURA_362",                                                       // 362
    "SPELL_AURA_363",                                                       // 363
    "SPELL_AURA_364",                                                       // 364
    "SPELL_AURA_365",                                                       // 365
    "SPELL_AURA_366",                                                       // 366
    "SPELL_AURA_367",                                                       // 367
    "SPELL_AURA_368",                                                       // 368
    "SPELL_AURA_369",                                                       // 369
    "SPELL_AURA_370",                                                       // 370
#endif
#if VERSION_STRING >= Mop
    "SPELL_AURA_371",                                                       // 371
    "SPELL_AURA_372",                                                       // 372
    "SPELL_AURA_373",                                                       // 373
    "SPELL_AURA_374",                                                       // 374 SPELL_AURA_MODIFY_FALL_DAMAGE_PCT
    "SPELL_AURA_375",                                                       // 375
    "SPELL_AURA_376",                                                       // 376 SPELL_AURA_MOD_CURRENCY_GAIN2
    "SPELL_AURA_377",                                                       // 377 SPELL_AURA_CAST_WHILE_WALKING2
    "SPELL_AURA_378",                                                       // 378
    "SPELL_AURA_379",                                                       // 379
    "SPELL_AURA_380",                                                       // 380
    "SPELL_AURA_381",                                                       // 381
    "SPELL_AURA_382",                                                       // 382
    "SPELL_AURA_383",                                                       // 383
    "SPELL_AURA_384",                                                       // 384
    "SPELL_AURA_385",                                                       // 385
    "SPELL_AURA_386",                                                       // 386
    "SPELL_AURA_387",                                                       // 387
    "SPELL_AURA_388",                                                       // 388
    "SPELL_AURA_389",                                                       // 389
    "SPELL_AURA_390",                                                       // 390
    "SPELL_AURA_391",                                                       // 391
    "SPELL_AURA_392",                                                       // 392
    "SPELL_AURA_393",                                                       // 393
    "SPELL_AURA_394",                                                       // 394
    "SPELL_AURA_395",                                                       // 395
    "SPELL_AURA_396",                                                       // 396
    "SPELL_AURA_397",                                                       // 397
    "SPELL_AURA_398",                                                       // 398
    "SPELL_AURA_399",                                                       // 399
    "SPELL_AURA_400",                                                       // 400
    "SPELL_AURA_401",                                                       // 401
    "SPELL_AURA_402",                                                       // 402
    "SPELL_AURA_403",                                                       // 403
    "SPELL_AURA_404",                                                       // 404
    "SPELL_AURA_405",                                                       // 405
    "SPELL_AURA_406",                                                       // 406
    "SPELL_AURA_407",                                                       // 407
    "SPELL_AURA_408",                                                       // 408
    "SPELL_AURA_409",                                                       // 409
    "SPELL_AURA_410",                                                       // 410
    "SPELL_AURA_411",                                                       // 411
    "SPELL_AURA_412",                                                       // 412
    "SPELL_AURA_413",                                                       // 413
    "SPELL_AURA_414",                                                       // 414
    "SPELL_AURA_415",                                                       // 415
    "SPELL_AURA_416",                                                       // 416
    "SPELL_AURA_417",                                                       // 417
    "SPELL_AURA_418",                                                       // 418
    "SPELL_AURA_419",                                                       // 419
    "SPELL_AURA_420",                                                       // 420
    "SPELL_AURA_421",                                                       // 421
    "SPELL_AURA_422",                                                       // 422
    "SPELL_AURA_423",                                                       // 423
    "SPELL_AURA_424",                                                       // 424
    "SPELL_AURA_425",                                                       // 425
    "SPELL_AURA_426",                                                       // 426
    "SPELL_AURA_427",                                                       // 427
    "SPELL_AURA_428",                                                       // 428
    "SPELL_AURA_429",                                                       // 429
    "SPELL_AURA_430",                                                       // 430
    "SPELL_AURA_431",                                                       // 431
    "SPELL_AURA_432",                                                       // 432
    "SPELL_AURA_433",                                                       // 433
    "SPELL_AURA_434",                                                       // 434
    "SPELL_AURA_435",                                                       // 435
    "SPELL_AURA_436",                                                       // 436
    "SPELL_AURA_437",                                                       // 437
#endif
};

void Aura::spellAuraEffectNotImplemented(AuraEffectModifier* aurEff, bool /*apply*/)
{
    sLogger.debugFlag(AscEmu::Logging::LF_AURA_EFF, "Aura::applyModifiers : Unknown aura id {} for spell id {}", aurEff->getAuraEffectType(), getSpellId());
}

void Aura::spellAuraEffectNotUsed(AuraEffectModifier* /*aurEff*/, bool /*apply*/)
{
    // Handled elsewhere or not used, so do nothing
}

void Aura::spellAuraEffectPeriodicDamage(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        // Hackfixes from legacy aura effect
        // Move these to spellscript
        switch (getSpellInfo()->getId())
        {
            //mage talent ignite
            case 12654:
            {
                if (!pSpellId) //we need a parent spell and should always have one since it procs on it
                    break;
                SpellInfo const* parentsp = sSpellMgr.getSpellInfo(pSpellId);
                if (!parentsp)
                    return;
                auto c = GetUnitCaster();
                if (c != nullptr && c->isPlayer())
                {
                    aurEff->setEffectDamage(Util::float2int32(static_cast<Player*>(c)->m_castedAmount[SCHOOL_FIRE] * parentsp->getEffectBasePoints(0) / 100.0f));
                }
                else if (c != nullptr)
                {
                    if (!aurEff->getEffectDamage())
                        return;

                    Spell* spell = sSpellMgr.newSpell(c, parentsp, false, nullptr);
                    SpellCastTargets castTargets(m_target->getGuid());

                    //this is so not good, maybe parent spell has more then dmg effect and we use it to calc our new dmg :(
                    aurEff->setEffectDamage(0);
                    for (uint8_t i = 0; i < 3; ++i)
                    {
                        const auto curVal = aurEff->getEffectDamage();
                        aurEff->setEffectDamage(curVal + (spell->calculateEffect(i) * parentsp->getEffectBasePoints(0) / 100));
                    }
                    delete spell;
                    spell = nullptr;
                }

            }
            // Warrior deep wounds
            case 12162:
            case 12721:
            {
                auto c = GetUnitCaster();
                if (!c->isPlayer())
                    break;

                uint32_t multiplyer = 0;
                if (pSpellId == 12834)
                    multiplyer = 16; //level 1 of the talent should apply 16 of average melee weapon dmg
                else if (pSpellId == 12849)
                    multiplyer = 32;
                else if (pSpellId == 12867)
                    multiplyer = 48;

                if (multiplyer)
                {
                    Player* pr = static_cast<Player*>(c);
                    Item* it;
                    it = pr->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                    if (it)
                    {
                        aurEff->setEffectDamage(0);
                        for (uint8_t i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
                            if (it->getItemProperties()->Damage[i].Type == SCHOOL_NORMAL)
                                aurEff->setEffectDamage(aurEff->getEffectDamage() + int32_t((it->getItemProperties()->Damage[i].Min + it->getItemProperties()->Damage[i].Max) / 2));
                        aurEff->setEffectDamage((int32_t)multiplyer * aurEff->getEffectDamage() / 100);
                    }
                }
            }
        }

        // Hackfixes end

        const auto casterUnit = GetUnitCaster();
        float_t damage = aurEff->getEffectFloatDamage();

        // Get bonus damage from spell power and attack power
        if (casterUnit != nullptr && !aurEff->isEffectDamageStatic())
            damage = casterUnit->applySpellDamageBonus(getSpellInfo(), aurEff->getEffectDamage(), aurEff->getEffectPercentModifier(), true, nullptr, this);

        if (damage <= 0.0f)
            return;

        aurEff->setEffectDamage(damage);

        // Set periodic timer only if timer was resetted
        if (m_periodicTimer[aurEff->getEffectIndex()] == 0)
            m_periodicTimer[aurEff->getEffectIndex()] = aurEff->getEffectAmplitude();

        // Hackfixes from legacy aura effect
        if (getSpellInfo()->custom_BGR_one_buff_on_target & SPELL_TYPE_WARLOCK_IMMOLATE)
            m_target->addAuraStateAndAuras(AURASTATE_FLAG_CONFLAGRATE);
        //maybe poison aurastate should get triggered on other spells too ?
        else if (getSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_POISON)  //deadly poison
            m_target->addAuraStateAndAuras(AURASTATE_FLAG_ENVENOM);
    }
    else
    {
#if VERSION_STRING < Cata
        // Prior to cata periodic timer was resetted on refresh
        m_periodicTimer[aurEff->getEffectIndex()] = 0;
#endif

        if (m_spellInfo->custom_BGR_one_buff_on_target & SPELL_TYPE_WARLOCK_IMMOLATE)
            m_target->removeAuraStateAndAuras(AURASTATE_FLAG_CONFLAGRATE);
        //maybe poison aurastate should get triggered on other spells too ?
        else if (m_spellInfo->custom_c_is_flags & SPELL_FLAG_IS_POISON)  //deadly poison
            m_target->removeAuraStateAndAuras(AURASTATE_FLAG_ENVENOM);
    }
}

void Aura::spellAuraEffectDummy(AuraEffectModifier* aurEff, bool apply)
{
    // Check that the dummy effect is handled properly in spell script
    // In case it's not, generate warning to debug log
    const auto scriptResult = sScriptMgr.callScriptedAuraOnDummyEffect(this, aurEff, apply);
    if (scriptResult == SpellScriptCheckDummy::DUMMY_OK)
        return;

    if (sScriptMgr.CallScriptedDummyAura(getSpellId(), aurEff->getEffectIndex(), this, apply))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_AURA_EFF, "Aura::spellAuraEffectDummy : Spell {} ({}) has a dummy aura effect, but no handler for it.", m_spellInfo->getId(), m_spellInfo->getName());
}

void Aura::spellAuraEffectPeriodicHeal(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        const auto casterUnit = GetUnitCaster();
        float_t heal = aurEff->getEffectFloatDamage();

        if (casterUnit != nullptr)
        {
            // Get bonus healing from spell power and attack power
            if (!aurEff->isEffectDamageStatic())
                heal = casterUnit->applySpellHealingBonus(getSpellInfo(), aurEff->getEffectDamage(), aurEff->getEffectPercentModifier(), true, nullptr, this);
        }

        if (heal <= 0)
            return;

        aurEff->setEffectDamage(heal);

        // Set periodic timer only if timer was resetted
        if (m_periodicTimer[aurEff->getEffectIndex()] == 0)
            m_periodicTimer[aurEff->getEffectIndex()] = aurEff->getEffectAmplitude();

        // Hackfix for aura state
        if (getSpellInfo()->getSpellFamilyName() == SPELLFAMILY_DRUID &&
            (getSpellInfo()->getSpellFamilyFlags(0) == 0x10 || getSpellInfo()->getSpellFamilyFlags(0) == 0x40))
        {
            getOwner()->addAuraStateAndAuras(AURASTATE_FLAG_SWIFTMEND);
            if (!sEventMgr.HasEvent(m_target, EVENT_REJUVENATION_FLAG_EXPIRE))
                sEventMgr.AddEvent(m_target, &Unit::removeAuraStateAndAuras, AURASTATE_FLAG_SWIFTMEND, EVENT_REJUVENATION_FLAG_EXPIRE, getTimeLeft(), 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            else
                sEventMgr.ModifyEventTimeLeft(m_target, EVENT_REJUVENATION_FLAG_EXPIRE, getTimeLeft(), 0);
        }
    }
    else
    {
#if VERSION_STRING < Cata
        // Prior to cata periodic timer was resetted on refresh
        m_periodicTimer[aurEff->getEffectIndex()] = 0;
#endif
    }
}

void Aura::spellAuraEffectPeriodicHealPct(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        aurEff->setEffectDamage(static_cast<int32_t>(std::ceil(getOwner()->getMaxHealth() * (aurEff->getEffectBaseDamage() / 100.0f))) * getStackCount());

        // Get bonus healing from spell power and attack power
        const auto casterUnit = GetUnitCaster();
        if (casterUnit != nullptr && !aurEff->isEffectDamageStatic())
            aurEff->setEffectDamage(casterUnit->applySpellHealingBonus(getSpellInfo(), aurEff->getEffectDamage(), aurEff->getEffectPercentModifier(), true, nullptr, this));

        // Set periodic timer only if timer was resetted
        if (m_periodicTimer[aurEff->getEffectIndex()] == 0)
            m_periodicTimer[aurEff->getEffectIndex()] = aurEff->getEffectAmplitude();
    }
    else
    {
#if VERSION_STRING < Cata
        // Prior to cata periodic timer was resetted on refresh
        m_periodicTimer[aurEff->getEffectIndex()] = 0;
#endif
    }
}

void Aura::spellAuraEffectPeriodicPowerPct(AuraEffectModifier* aurEff, bool apply)
{
    if (aurEff->getEffectMiscValue() < POWER_TYPE_MANA || aurEff->getEffectMiscValue() >= TOTAL_PLAYER_POWER_TYPES)
        return;

    if (apply)
    {
        const auto powerType = static_cast<PowerType>(aurEff->getEffectMiscValue());
        aurEff->setEffectDamage(static_cast<int32_t>(std::ceil(getOwner()->getMaxPower(powerType) * (aurEff->getEffectBaseDamage() / 100.0f))) * getStackCount());

        // Set periodic timer only if timer was resetted
        if (m_periodicTimer[aurEff->getEffectIndex()] == 0)
            m_periodicTimer[aurEff->getEffectIndex()] = aurEff->getEffectAmplitude();
    }
    else
    {
#if VERSION_STRING < Cata
        // Prior to cata periodic timer was resetted on refresh
        m_periodicTimer[aurEff->getEffectIndex()] = 0;
#endif
    }
}

void Aura::spellAuraEffectPeriodicTriggerSpell(AuraEffectModifier* aurEff, bool apply)
{
    // Hackfixes from legacy methods
    switch (m_spellInfo->getId())
    {
        case 23493:
        case 24379:
        {
            Unit* caster = m_target;
            if (caster != nullptr)
            {
                ///\ todo: fix this
                //sEventMgr.AddEvent(this, &Aura::EventPeriodicHealPct, 10.0f , EVENT_AURA_PERIODIC_HEALPERC, 1000, 10, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

                if (caster->getMaxPower(POWER_TYPE_MANA))
                {
                    //sEventMgr.AddEvent(this, &Aura::EventPeriodicManaPct, 10.0f, EVENT_AURA_PERIOCIC_MANA, 1000, 10, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
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

    if (apply)
    {
        // Set periodic timer only if timer was resetted
        if (m_periodicTimer[aurEff->getEffectIndex()] == 0)
            m_periodicTimer[aurEff->getEffectIndex()] = aurEff->getEffectAmplitude();
    }
    else
    {
#if VERSION_STRING < Cata
        // Prior to cata periodic timer was resetted on refresh
        m_periodicTimer[aurEff->getEffectIndex()] = 0;
#endif
    }
}

void Aura::spellAuraEffectPeriodicEnergize(AuraEffectModifier* aurEff, bool apply)
{
    if (aurEff->getEffectMiscValue() < POWER_TYPE_MANA || aurEff->getEffectMiscValue() >= TOTAL_PLAYER_POWER_TYPES)
        return;

    if (apply)
    {
        // Set periodic timer only if timer was resetted
        if (m_periodicTimer[aurEff->getEffectIndex()] == 0)
            m_periodicTimer[aurEff->getEffectIndex()] = aurEff->getEffectAmplitude();
    }
    else
    {
#if VERSION_STRING < Cata
        // Prior to cata periodic timer was resetted on refresh
        m_periodicTimer[aurEff->getEffectIndex()] = 0;
#endif
    }
}

void Aura::spellAuraEffectModShapeshift(AuraEffectModifier* aurEff, bool apply)
{
    // Dismount
    if (p_target != nullptr)
    {
        switch (aurEff->getEffectMiscValue())
        {
            case FORM_BATTLESTANCE:
            case FORM_DEFENSIVESTANCE:
            case FORM_BERSERKERSTANCE:
                break;
            default:
                p_target->dismount();
                break;
        }
    }

    const auto shapeshiftForm = sSpellShapeshiftFormStore.lookupEntry(static_cast<uint32_t>(aurEff->getEffectMiscValue()));
    if (shapeshiftForm == nullptr)
        return;

    const auto oldForm = getOwner()->getShapeShiftForm();
    const uint8_t newForm = apply ? static_cast<uint8_t>(aurEff->getEffectMiscValue()) : FORM_NORMAL;

    // Remove previous shapeshift aura on apply
    if (apply)
        getOwner()->removeAllAurasByAuraEffect(SPELL_AURA_MOD_SHAPESHIFT, getSpellId());

    // Some forms have two additional hidden passive aura
    uint32_t passiveSpellId = 0, secondaryPassiveSpell = 0;
#if VERSION_STRING > Classic
    auto modelId = shapeshiftForm->modelId;
#else
    uint32_t modelId = 0;
#endif
    auto freeMovements = false, removePolymorph = false;

    switch (shapeshiftForm->id)
    {
        case FORM_CAT:
        {
            freeMovements = true;
            removePolymorph = true;
            passiveSpellId = 3025;

            // todo: there should be other model ids as well
            if (getOwner()->getRace() == RACE_TAUREN)
                modelId = 8571;

            if (apply)
            {
                getOwner()->setPowerType(POWER_TYPE_ENERGY);
                getOwner()->setMaxPower(POWER_TYPE_ENERGY, 100);
                getOwner()->setPower(POWER_TYPE_ENERGY, 0);
            }
            else
            {
                getOwner()->setPowerType(POWER_TYPE_MANA);
                // Remove Prowl
                getOwner()->removeAllAurasByAuraEffect(SPELL_AURA_MOD_STEALTH);
            }
        } break;
        case FORM_TREE:
        {
            freeMovements = true;
            removePolymorph = true;
            passiveSpellId = 5420;
            secondaryPassiveSpell = 34123;
        } break;
        case FORM_TRAVEL:
        {
            freeMovements = true;
            removePolymorph = true;
            passiveSpellId = 5419;
        } break;
        case FORM_AQUA:
        {
            freeMovements = true;
            removePolymorph = true;
            passiveSpellId = 5421;
        } break;
        case FORM_BEAR:
        {
            freeMovements = true;
            removePolymorph = true;
            passiveSpellId = 1178;
            secondaryPassiveSpell = 21178;

            // todo: there should be other model ids as well
            if (getOwner()->getRace() == RACE_TAUREN)
                modelId = 2289;

            if (apply)
            {
                getOwner()->setPowerType(POWER_TYPE_RAGE);
                getOwner()->setMaxPower(POWER_TYPE_RAGE, 1000);
                getOwner()->setPower(POWER_TYPE_RAGE, 0);
            }
            else
            {
                getOwner()->setPowerType(POWER_TYPE_MANA);
            }
        } break;
        case FORM_GHOUL:
        case FORM_SKELETON:
        case FORM_ZOMBIE:
        {
            if (getPlayerOwner() != nullptr)
                getPlayerOwner()->sendAvailSpells(shapeshiftForm, apply);
        } break;
        case FORM_DIREBEAR:
        {
            freeMovements = true;
            removePolymorph = true;
            passiveSpellId = 9635;
            // Same spell as in Bear Form, increases threat generate
            secondaryPassiveSpell = 21178;

            // todo: there should be other model ids as well
            if (getOwner()->getRace() == RACE_TAUREN)
                modelId = 2289;

            if (apply)
            {
                getOwner()->setPowerType(POWER_TYPE_RAGE);
                getOwner()->setMaxPower(POWER_TYPE_RAGE, 1000);
                getOwner()->setPower(POWER_TYPE_RAGE, 0);
            }
            else
            {
                getOwner()->setPowerType(POWER_TYPE_MANA);
            }
        } break;
        case FORM_BATTLESTANCE:
        case FORM_DEFENSIVESTANCE:
        case FORM_BERSERKERSTANCE:
        {
            if (shapeshiftForm->id == FORM_BATTLESTANCE)
                passiveSpellId = 21156;
            else if (shapeshiftForm->id == FORM_DEFENSIVESTANCE)
                passiveSpellId = 7376;
            else if (shapeshiftForm->id == FORM_BERSERKERSTANCE)
                passiveSpellId = 7381;

            // Check retained rage
            if (apply && getPlayerOwner() != nullptr && getPlayerOwner()->isClassWarrior())
            {
                if (getPlayerOwner()->getPower(POWER_TYPE_RAGE) > getPlayerOwner()->m_retaineDrage)
                    getPlayerOwner()->setPower(POWER_TYPE_RAGE, getPlayerOwner()->m_retaineDrage);
            }
        } break;
#if VERSION_STRING >= WotLK
        case FORM_METAMORPHOSIS:
        {
            // This form has a lot of passive auras
            if (apply)
            {
                // Stun and slow reduction
                getOwner()->castSpell(getOwner(), 54817, true);
                // Demonic language
                getOwner()->castSpell(getOwner(), 54879, true);
                // Enslave immunity
                getOwner()->castSpell(getOwner(), 61610, true);
            }
            else
            {
                getOwner()->removeAllAurasById(54817);
                getOwner()->removeAllAurasById(54879);
                getOwner()->removeAllAurasById(61610);
            }
        } break;
#endif
        // Druid's epic flying form
        case FORM_SWIFT:
        {
            freeMovements = true;
            removePolymorph = true;
            passiveSpellId = 40121;
            secondaryPassiveSpell = 40122;

            // todo: there should be other model ids as well
            if (getOwner()->getRace() == RACE_TAUREN)
                modelId = 21244;
        } break;
        case FORM_SHADOW:
        {
#if VERSION_STRING >= WotLK
            passiveSpellId = 49868;
#endif
            // todo: is this needed?
            if (apply && getPlayerOwner() != nullptr)
                getPlayerOwner()->sendSpellCooldownEventPacket(getSpellId());
        } break;
        case FORM_FLIGHT:
        {
            freeMovements = true;
            removePolymorph = true;
            passiveSpellId = 33948;
            secondaryPassiveSpell = 34764;

            // todo: there should be other model ids as well
            if (getOwner()->getRace() == RACE_TAUREN)
                modelId = 20872;
        } break;
        case FORM_MOONKIN:
        {
            freeMovements = true;
            removePolymorph = true;
            passiveSpellId = 24905;

            // todo: there should be other model ids as well
            if (getOwner()->getRace() == RACE_TAUREN)
                modelId = 15375;
        } break;
        case FORM_SPIRITOFREDEMPTION:
        {
            passiveSpellId = 27795;
        } break;
    }

    if (freeMovements)
    {
        getOwner()->removeAllAurasByAuraEffect(SPELL_AURA_MOD_DECREASE_SPEED, true);
        getOwner()->removeAllAurasByAuraEffect(SPELL_AURA_MOD_ROOT);
    }

    if (apply)
    {
        if (removePolymorph && getOwner()->hasUnitStateFlag(UNIT_STATE_POLYMORPHED))
            getOwner()->removeAllAurasById(getOwner()->getTransformAura());

        if (modelId != 0)
        {
            // By default shapeshifting overwrites transforms
            auto overWriteDisplay = true;

            const auto transformSpell = sSpellMgr.getSpellInfo(getOwner()->getTransformAura());
            if (transformSpell != nullptr)
            {
                // Check if the transform spell has higher priority
                // Also, check for skeleton transforms (i.e. Noggenfogger Elixir) which overwrite shapeshifting in Classic
                const auto takePriority = (transformSpell->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY && transformSpell->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO) || transformSpell->getAttributesExC() & ATTRIBUTESEXC_HIGH_PRIORITY;
                if (takePriority || transformSpell->isNegativeAura()
#if VERSION_STRING == Classic
                    || transformSpell->getAttributes() == 0x28000000
#endif
                    )
                    overWriteDisplay = false;
            }

            if (overWriteDisplay)
            {
                getOwner()->setDisplayId(modelId);
                getOwner()->eventModelChange();
            }

            // Save model id for later use
            aurEff->setEffectFixedDamage(modelId);
        }

        getOwner()->setShapeShiftForm(newForm);

        if (passiveSpellId != 0)
            getOwner()->castSpell(getOwner(), passiveSpellId, true);
        if (secondaryPassiveSpell != 0)
            getOwner()->castSpell(getOwner(), secondaryPassiveSpell, true);
    }
    else
    {
        getOwner()->setShapeShiftForm(newForm);
        getOwner()->restoreDisplayId();

        if (passiveSpellId != 0)
            getOwner()->removeAllAurasById(passiveSpellId);
        if (secondaryPassiveSpell != 0)
            getOwner()->removeAllAurasById(secondaryPassiveSpell);
    }

    // Remove auras which unit should not have anymore
    for (uint16_t i = AuraSlots::PASSIVE_SLOT_START; i < AuraSlots::POSITIVE_SLOT_END; ++i)
    {
        auto* const aur = getOwner()->getAuraWithAuraSlot(i);
        if (aur == nullptr)
            continue;

        const auto requiredForm = aur->getSpellInfo()->getRequiredShapeShift();
        if (requiredForm != 0)
        {
            if (oldForm != FORM_NORMAL && oldForm != FORM_SHADOW && oldForm != FORM_STEALTH)
            {
                const uint32_t oldFormMask = 1U << (oldForm - 1);
                const uint32_t newFormMask = 1U << (newForm - 1);
                // Check if the aura is usable in new form
                if (oldFormMask & requiredForm && !(newFormMask & requiredForm))
                {
                    aur->removeAura();
                    continue;
                }
            }
        }
    }

    if (getPlayerOwner() != nullptr)
    {
        // Apply talents and spells that require this form
        for (const auto& spell : getPlayerOwner()->getSpellSet())
        {
            const auto spellInfo = sSpellMgr.getSpellInfo(spell);
            if (spellInfo == nullptr)
                continue;

            if (spellInfo->isPassive() && spellInfo->getRequiredShapeShift() > 0)
            {
                const uint32_t newFormMask = 1U << (newForm - 1);
                if (newFormMask & spellInfo->getRequiredShapeShift())
                    getPlayerOwner()->castSpell(getPlayerOwner(), spellInfo, true);
            }
        }

        // Feral attack power
        if (getPlayerOwner()->isClassDruid())
        {
            // Change from normal form to feral form
            if (!(oldForm == FORM_MOONKIN || oldForm == FORM_CAT || oldForm == FORM_BEAR || oldForm == FORM_DIREBEAR) &&
                (newForm == FORM_MOONKIN || newForm == FORM_CAT || newForm == FORM_BEAR || newForm == FORM_DIREBEAR))
                getPlayerOwner()->applyFeralAttackPower(true);
            // Change from feral form to normal form
            else if ((oldForm == FORM_MOONKIN || oldForm == FORM_CAT || oldForm == FORM_BEAR || oldForm == FORM_DIREBEAR) &&
                !(newForm == FORM_MOONKIN || newForm == FORM_CAT || newForm == FORM_BEAR || newForm == FORM_DIREBEAR))
                getPlayerOwner()->applyFeralAttackPower(false);
        }

        // Apply dummy shapeshift spells
        for (const auto& spell : getPlayerOwner()->getShapeshiftSpells())
        {
            const auto spellInfo = sSpellMgr.getSpellInfo(spell);
            if (spellInfo == nullptr)
                continue;

            const uint32_t newFormMask = 1U << (newForm - 1);
            if (spellInfo->getRequiredShapeShift() > 0 && (newFormMask & spellInfo->getRequiredShapeShift()))
                getPlayerOwner()->castSpell(getPlayerOwner(), spellInfo, true);
        }

        // Hackfix - Heart of the Wild talent
        getPlayerOwner()->eventTalentHearthOfWildChange(apply);

        getPlayerOwner()->updateStats();
        getPlayerOwner()->updateAttackSpeed();
    }
}

void Aura::spellAuraEffectPeriodicLeech(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        const auto casterUnit = GetUnitCaster();
        float_t damage = aurEff->getEffectFloatDamage();

        // Get bonus damage from spell power and attack power
        if (casterUnit != nullptr && !aurEff->isEffectDamageStatic())
            damage = casterUnit->applySpellDamageBonus(getSpellInfo(), aurEff->getEffectDamage(), aurEff->getEffectPercentModifier(), true, nullptr, this);

        if (casterUnit != nullptr)
        {
#if VERSION_STRING <= Cata
            // Hackfix from legacy method
            // Apply bonus from [Warlock] Soul Siphon
            if (casterUnit->m_soulSiphon.m_amount)
            {
                // Use std::map to prevent counting duplicate auras (stacked ones, from the same unit)
                std::map<uint64_t, std::unique_ptr<std::set<uint32_t>>> auras;
                std::map<uint64_t, std::unique_ptr<std::set<uint32_t>>>::iterator itx, itx2;
                int32_t pct;
                int32_t count = 0;

                auras.clear();
                for (uint16_t i = AuraSlots::TOTAL_SLOT_START; i < AuraSlots::TOTAL_SLOT_END; ++i)
                {
                    Aura* aura = m_target->getAuraWithAuraSlot(i);
                    if (aura == nullptr)
                        continue;

                    if (aura->getSpellInfo()->getSpellFamilyName() != 5)
                        continue;

                    auto _continue = false;
                    const auto spellSkillBounds = sSpellMgr.getSkillEntryForSpellBounds(aura->getSpellId());
                    for (auto spellSkillItr = spellSkillBounds.first; spellSkillItr != spellSkillBounds.second; ++spellSkillItr)
                    {
                        auto skill_line_ability = spellSkillItr->second;
                        if (skill_line_ability->skilline != SKILL_AFFLICTION)
                        {
                            _continue = true;
                            break;
                        }
                    }

                    if (_continue)
                        continue;

                    itx = auras.find(aura->getCasterGuid());
                    if (itx == auras.end())
                    {
                        const auto [insertItr, _] = auras.emplace(aura->getCasterGuid(), std::make_unique<std::set<uint32_t>>());
                        itx = insertItr;
                    }

                    const auto& ids = itx->second;
                    if (ids->find(aura->getSpellId()) == ids->end())
                    {
                        ids->insert(aura->getSpellId());
                    }
                }

                if (auras.size())
                {
                    itx = auras.begin();
                    while (itx != auras.end())
                    {
                        itx2 = itx++;
                        count += (int32_t)itx2->second->size();
                    }
                }

                pct = count * casterUnit->m_soulSiphon.m_amount;
                if (pct > casterUnit->m_soulSiphon.m_max)
                    pct = casterUnit->m_soulSiphon.m_max;
                damage += aurEff->getEffectFloatDamage() * pct / 100;
            }
#endif
        }

        aurEff->setEffectDamage(damage);

        // Set periodic timer only if timer was resetted
        if (m_periodicTimer[aurEff->getEffectIndex()] == 0)
            m_periodicTimer[aurEff->getEffectIndex()] = aurEff->getEffectAmplitude();
    }
    else
    {
#if VERSION_STRING < Cata
        // Prior to cata periodic timer was resetted on refresh
        m_periodicTimer[aurEff->getEffectIndex()] = 0;
#endif
    }
}

void Aura::spellAuraEffectTransform(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        uint32_t displayId = 0;
        std::vector<uint32_t> displayIds;
        const auto properties = sMySQLStore.getCreatureProperties(static_cast<uint32_t>(aurEff->getEffectMiscValue()));
        if (properties == nullptr)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_AURA_EFF, "Aura::spellAuraEffectTransform : Unknown creature entry {} in misc value for spell {}", aurEff->getEffectMiscValue(), getSpellId());
            return;
        }

        // Take all possible display ids from creature properties
        if (properties->Male_DisplayID != 0)
            displayIds.push_back(properties->Male_DisplayID);
        if (properties->Female_DisplayID != 0)
            displayIds.push_back(properties->Female_DisplayID);
        if (properties->Male_DisplayID2 != 0)
            displayIds.push_back(properties->Male_DisplayID2);
        if (properties->Female_DisplayID2 != 0)
            displayIds.push_back(properties->Female_DisplayID2);

        if (displayIds.size() > 0)
            displayId = displayIds[Util::getRandomUInt(static_cast<uint32_t>(displayIds.size() - 1))];

        if (displayId == 0)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_AURA_EFF, "Aura::spellAuraEffectTransform : Creature entry {} has no display id for spell {}", properties->Id, getSpellId());
            return;
        }

        const auto transformAura = sSpellMgr.getSpellInfo(getOwner()->getTransformAura());
        if (transformAura == nullptr || !transformAura->isNegativeAura())
        {
            // Check if there is an existing transform with higher priority
            const auto takePriority = transformAura != nullptr && ((transformAura->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY && transformAura->getAttributesEx() & ATTRIBUTESEX_NO_INITIAL_AGGRO) || transformAura->getAttributesExC() & ATTRIBUTESEXC_HIGH_PRIORITY);
            if (getSpellInfo()->isNegativeAura() || !(transformAura != nullptr && takePriority))
            {
                getOwner()->setDisplayId(displayId);
                getOwner()->eventModelChange();

                getOwner()->setTransformAura(getSpellId());
            }
        }

        // Save model id for later use
        aurEff->setEffectFixedDamage(displayId);
    }
    else
    {
        if (getOwner()->getTransformAura() == getSpellId())
            getOwner()->setTransformAura(0);

        getOwner()->restoreDisplayId();
    }
}

void Aura::spellAuraEffectPeriodicHealthFunnel(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        // Get bonus damage from spell power and attack power
        const auto casterUnit = GetUnitCaster();
        if (casterUnit != nullptr && !aurEff->isEffectDamageStatic())
            aurEff->setEffectDamage(casterUnit->applySpellDamageBonus(getSpellInfo(), aurEff->getEffectDamage(), aurEff->getEffectPercentModifier(), true, nullptr, this));

        // Set periodic timer only if timer was resetted
        if (m_periodicTimer[aurEff->getEffectIndex()] == 0)
            m_periodicTimer[aurEff->getEffectIndex()] = aurEff->getEffectAmplitude();
    }
    else
    {
#if VERSION_STRING < Cata
        // Prior to cata periodic timer was resetted on refresh
        m_periodicTimer[aurEff->getEffectIndex()] = 0;
#endif
    }
}

void Aura::spellAuraEffectPeriodicManaLeech(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        // Set periodic timer only if timer was resetted
        if (m_periodicTimer[aurEff->getEffectIndex()] == 0)
            m_periodicTimer[aurEff->getEffectIndex()] = aurEff->getEffectAmplitude();
    }
    else
    {
#if VERSION_STRING < Cata
        // Prior to cata periodic timer was resetted on refresh
        m_periodicTimer[aurEff->getEffectIndex()] = 0;
#endif
    }
}

void Aura::spellAuraEffectSchoolAbsorb(AuraEffectModifier* /*aurEff*/, bool /*apply*/)
{
    // See AbsorbAura::spellAuraEffectSchoolAbsorb
}

void Aura::spellAuraEffectModPowerRegen(AuraEffectModifier* aurEff, bool apply)
{
    if (aurEff->getEffectDamage() == 0)
        return;

    // Update only necessary powers, the rest are handled in regeneratePower
    switch (aurEff->getEffectMiscValue())
    {
        case POWER_TYPE_MANA:
            // TODO: missing update for creatures
            if (getPlayerOwner() == nullptr)
                break;
            getPlayerOwner()->updateManaRegeneration();
            break;
        case POWER_TYPE_RAGE:
            if (getPlayerOwner() == nullptr)
                break;
            getPlayerOwner()->updateRageRegeneration();
            break;
        case POWER_TYPE_FOCUS:
            getOwner()->updateFocusRegeneration();
            break;
        case POWER_TYPE_ENERGY:
            getOwner()->updateEnergyRegeneration();
            break;
#if VERSION_STRING >= WotLK
        case POWER_TYPE_RUNIC_POWER:
            if (getPlayerOwner() == nullptr)
                break;
            getPlayerOwner()->updateRunicPowerRegeneration();
            break;
#endif
        default:
            break;
    }
}

void Aura::spellAuraEffectPeriodicDamagePercent(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        aurEff->setEffectDamage(static_cast<int32_t>(std::ceil(aurEff->getEffectBaseDamage() / 100.0f * getOwner()->getMaxHealth())) * getStackCount());

        // Get bonus damage from spell power and attack power
        const auto casterUnit = GetUnitCaster();
        if (casterUnit != nullptr && !aurEff->isEffectDamageStatic())
            aurEff->setEffectDamage(casterUnit->applySpellDamageBonus(getSpellInfo(), aurEff->getEffectDamage(), aurEff->getEffectPercentModifier(), true, nullptr, this));

        // Set periodic timer only if timer was resetted
        if (m_periodicTimer[aurEff->getEffectIndex()] == 0)
            m_periodicTimer[aurEff->getEffectIndex()] = aurEff->getEffectAmplitude();
    }
    else
    {
#if VERSION_STRING < Cata
        // Prior to cata periodic timer was resetted on refresh
        m_periodicTimer[aurEff->getEffectIndex()] = 0;
#endif
    }
}

void Aura::spellAuraEffectAddModifier(AuraEffectModifier* aurEff, bool apply)
{
    if (aurEff->getEffectMiscValue() >= MAX_SPELLMOD_TYPE)
    {
        sLogger.failure("Aura::spellAuraEffectAddModifier : Unknown spell modifier type {} in spell {}, skipping", aurEff->getEffectMiscValue(), getSpellId());
        return;
    }

    getOwner()->addSpellModifier(aurEff, apply);

    // Attempt to prevent possible memory corruption
    // Multiple spells could use this same spell modifier, and when the first spell removes this modifier
    // the second spell has a modifier at invalid memory address
    // Anyhow, the chance for this to happen is rather low
    if (!apply)
    {
        getOwner()->removeSpellModifierFromCurrentSpells(aurEff);
        if (getCasterGuid() != getOwner()->getGuid())
        {
            const auto casterObj = getCaster();
            if (casterObj != nullptr)
                casterObj->removeSpellModifierFromCurrentSpells(aurEff);
        }
    }

    if (!getOwner()->isPlayer())
        return;

    // todo: hackfix (?) from the old effect, handle this in pet system
    // Hunter's beastmastery talents
    if (aurEff->getAuraEffectType() == SPELL_AURA_ADD_FLAT_MODIFIER)
    {
        const auto pet = getPlayerOwner()->getPet();
        if (pet != nullptr)
        {
            switch (getSpellInfo()->getId())
            {
                // SPELL_HASH_UNLEASHED_FURY:
                case 19616:
                case 19617:
                case 19618:
                case 19619:
                case 19620:
                    pet->LoadPetAuras(0);
                    break;
                // SPELL_HASH_THICK_HIDE:
                case 16929:
                case 16930:
                case 16931:
                case 19609:
                case 19610:
                case 19612:
                case 50502:
                    pet->LoadPetAuras(1);
                    break;
                // SPELL_HASH_ENDURANCE_TRAINING:
                case 19583:
                case 19584:
                case 19585:
                case 19586:
                case 19587:
                    pet->LoadPetAuras(2);
                    break;
                // SPELL_HASH_FERAL_SWIFTNESS:
                case 17002:
                case 24866:
                    pet->LoadPetAuras(3);
                    break;
                // SPELL_HASH_BESTIAL_DISCIPLINE:
                case 19590:
                case 19592:
                    pet->LoadPetAuras(4);
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
                    pet->LoadPetAuras(5);
                    break;
                // SPELL_HASH_ANIMAL_HANDLER:
                case 34453:
                case 34454:
                case 68361:
                    pet->LoadPetAuras(6);
                    break;
                // SPELL_HASH_CATLIKE_REFLEXES:
                case 34462:
                case 34464:
                case 34465:
                    pet->LoadPetAuras(7);
                    break;
                // SPELL_HASH_SERPENT_S_SWIFTNESS:
                case 34466:
                case 34467:
                case 34468:
                case 34469:
                case 34470:
                    pet->LoadPetAuras(8);
                    break;
            }
        }
    }
}

void Aura::spellAuraEffectRetainComboPoints(AuraEffectModifier* aurEff, bool apply)
{
    if (getPlayerOwner() == nullptr)
        return;

    if (!apply)
    {
        // Remove combo points created by this aura only if duration has expired
        if (getTimeLeft() == 0)
            getPlayerOwner()->addComboPoints(getPlayerOwner()->getTargetGuid(), static_cast<int8_t>(-aurEff->getEffectDamage()));
    }
}

void Aura::spellAuraEffectPeriodicPowerBurn(AuraEffectModifier* aurEff, bool apply)
{
    if (aurEff->getEffectMiscValue() < POWER_TYPE_MANA || aurEff->getEffectMiscValue() >= TOTAL_PLAYER_POWER_TYPES)
        return;

    const auto powerType = static_cast<PowerType>(aurEff->getEffectMiscValue());
    if (getOwner()->getMaxPower(powerType) == 0)
        return;

    if (apply)
    {
        // Set periodic timer only if timer was resetted
        if (m_periodicTimer[aurEff->getEffectIndex()] == 0)
            m_periodicTimer[aurEff->getEffectIndex()] = aurEff->getEffectAmplitude();
    }
    else
    {
#if VERSION_STRING < Cata
        // Prior to cata periodic timer was resetted on refresh
        m_periodicTimer[aurEff->getEffectIndex()] = 0;
#endif
    }
}

void Aura::spellAuraEffectPeriodicTriggerDummy(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        // Set periodic timer only if timer was resetted
        if (m_periodicTimer[aurEff->getEffectIndex()] == 0)
            m_periodicTimer[aurEff->getEffectIndex()] = aurEff->getEffectAmplitude();
    }
    else
    {
        if (!sScriptMgr.CallScriptedDummyAura(getSpellId(), aurEff->getEffectIndex(), this, false))
            sLogger.debugFlag(AscEmu::Logging::LF_AURA_EFF, "Spell aura {} has a periodic trigger dummy effect but no handler for it", getSpellId());

#if VERSION_STRING < Cata
        // Prior to cata periodic timer was resetted on refresh
        m_periodicTimer[aurEff->getEffectIndex()] = 0;
#endif
    }
}

void Aura::spellAuraEffectPeriodicTriggerSpellWithValue(AuraEffectModifier* aurEff, bool apply)
{
    if (apply)
    {
        const auto triggeredInfo = sSpellMgr.getSpellInfo(getSpellInfo()->getEffectTriggerSpell(aurEff->getEffectIndex()));
        if (triggeredInfo == nullptr)
            return;

        // Set periodic timer only if timer was resetted
        if (m_periodicTimer[aurEff->getEffectIndex()] == 0)
            m_periodicTimer[aurEff->getEffectIndex()] = aurEff->getEffectAmplitude();
    }
    else
    {
#if VERSION_STRING < Cata
        // Prior to cata periodic timer was resetted on refresh
        m_periodicTimer[aurEff->getEffectIndex()] = 0;
#endif
    }
}
