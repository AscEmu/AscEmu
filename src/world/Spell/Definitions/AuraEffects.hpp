/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldConf.h"

#include <cstdint>

enum AuraEffect : uint32_t
{
    SPELL_AURA_NONE = 0,                                                // None
    SPELL_AURA_BIND_SIGHT = 1,                                          // Bind Sight
    SPELL_AURA_MOD_POSSESS = 2,                                         // Mod Possess
    SPELL_AURA_PERIODIC_DAMAGE = 3,                                     // Periodic Damage
    SPELL_AURA_DUMMY = 4,                                               // Script Aura
    SPELL_AURA_MOD_CONFUSE = 5,                                         // Mod Confuse
    SPELL_AURA_MOD_CHARM = 6,                                           // Mod Charm
    SPELL_AURA_MOD_FEAR = 7,                                            // Mod Fear
    SPELL_AURA_PERIODIC_HEAL = 8,                                       // Periodic Heal
    SPELL_AURA_MOD_ATTACKSPEED = 9,                                     // Mod Attack Speed
    SPELL_AURA_MOD_THREAT = 10,                                         // Mod Threat
    SPELL_AURA_MOD_TAUNT = 11,                                          // Taunt
    SPELL_AURA_MOD_STUN = 12,                                           // Stun
    SPELL_AURA_MOD_DAMAGE_DONE = 13,                                    // Mod Damage Done
    SPELL_AURA_MOD_DAMAGE_TAKEN = 14,                                   // Mod Damage Taken
    SPELL_AURA_DAMAGE_SHIELD = 15,                                      // Damage Shield
    SPELL_AURA_MOD_STEALTH = 16,                                        // Mod Stealth
    SPELL_AURA_MOD_STEALTH_DETECTION = 17,                              // Mod Stealth Detection
    SPELL_AURA_MOD_INVISIBILITY = 18,                                   // Mod Invisibility
    SPELL_AURA_MOD_INVISIBILITY_DETECTION = 19,                         // Mod Invisibility Detection
    SPELL_AURA_PERIODIC_HEAL_PCT = 20,
    SPELL_AURA_PERIODIC_POWER_PCT = 21,
    SPELL_AURA_MOD_RESISTANCE = 22,                                     // Mod Resistance
    SPELL_AURA_PERIODIC_TRIGGER_SPELL = 23,                             // Periodic Trigger
    SPELL_AURA_PERIODIC_ENERGIZE = 24,                                  // Periodic Energize
    SPELL_AURA_MOD_PACIFY = 25,                                         // Pacify
    SPELL_AURA_MOD_ROOT = 26,                                           // Root
    SPELL_AURA_MOD_SILENCE = 27,                                        // Silence
    SPELL_AURA_REFLECT_SPELLS = 28,                                     // Reflect Spells %
    SPELL_AURA_MOD_STAT = 29,                                           // Mod Stat
    SPELL_AURA_MOD_SKILL = 30,                                          // Mod Skill
    SPELL_AURA_MOD_INCREASE_SPEED = 31,                                 // Mod Speed
    SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED = 32,                         // Mod Speed Mounted
    SPELL_AURA_MOD_DECREASE_SPEED = 33,                                 // Mod Speed Slow
    SPELL_AURA_MOD_INCREASE_HEALTH = 34,                                // Mod Increase Health
    SPELL_AURA_MOD_INCREASE_ENERGY = 35,                                // Mod Increase Energy
    SPELL_AURA_MOD_SHAPESHIFT = 36,                                     // Shapeshift
    SPELL_AURA_EFFECT_IMMUNITY = 37,                                    // Immune Effect
    SPELL_AURA_STATE_IMMUNITY = 38,                                     // Immune State
    SPELL_AURA_SCHOOL_IMMUNITY = 39,                                    // Immune School
    SPELL_AURA_DAMAGE_IMMUNITY = 40,                                    // Immune Damage
    SPELL_AURA_DISPEL_IMMUNITY = 41,                                    // Immune Dispel Type
    SPELL_AURA_PROC_TRIGGER_SPELL = 42,                                 // Proc Trigger Spell
    SPELL_AURA_PROC_TRIGGER_DAMAGE = 43,                                // Proc Trigger Damage
    SPELL_AURA_TRACK_CREATURES = 44,                                    // Track Creatures
    SPELL_AURA_TRACK_RESOURCES = 45,                                    // Track Resources
    SPELL_AURA_MOD_PARRY_SKILL = 46,                                    // Mod Parry Skill
    SPELL_AURA_MOD_PARRY_PERCENT = 47,                                  // Mod Parry Percent
    SPELL_AURA_MOD_DODGE_SKILL = 48,                                    // Mod Dodge Skill
    SPELL_AURA_MOD_DODGE_PERCENT = 49,                                  // Mod Dodge Percent
    SPELL_AURA_MOD_BLOCK_SKILL = 50,                                    // Mod Block Skill
    SPELL_AURA_MOD_BLOCK_PERCENT = 51,                                  // Mod Block Percent
    SPELL_AURA_MOD_CRIT_PERCENT = 52,                                   // Mod Crit Percent
    SPELL_AURA_PERIODIC_LEECH = 53,                                     // Periodic Leech
    SPELL_AURA_MOD_HIT_CHANCE = 54,                                     // Mod Hit Chance
    SPELL_AURA_MOD_SPELL_HIT_CHANCE = 55,                               // Mod Spell Hit Chance
    SPELL_AURA_TRANSFORM = 56,                                          // Transform
    SPELL_AURA_MOD_SPELL_CRIT_CHANCE = 57,                              // Mod Spell Crit Chance
    SPELL_AURA_MOD_INCREASE_SWIM_SPEED = 58,                            // Mod Speed Swim
    SPELL_AURA_MOD_DAMAGE_DONE_CREATURE = 59,                           // Mod Creature Dmg Done
    SPELL_AURA_MOD_PACIFY_SILENCE = 60,                                 // Pacify & Silence
    SPELL_AURA_MOD_SCALE = 61,                                          // Mod Scale
    SPELL_AURA_PERIODIC_HEALTH_FUNNEL = 62,                             // Periodic Health Funnel
    SPELL_AURA_PERIODIC_MANA_FUNNEL = 63,                               // Periodic Mana Funnel
    SPELL_AURA_PERIODIC_MANA_LEECH = 64,                                // Periodic Mana Leech
    SPELL_AURA_MOD_CASTING_SPEED = 65,                                  // Haste - Spells
    SPELL_AURA_FEIGN_DEATH = 66,                                        // Feign Death
    SPELL_AURA_MOD_DISARM = 67,                                         // Disarm
    SPELL_AURA_MOD_STALKED = 68,                                        // Mod Stalked
    SPELL_AURA_SCHOOL_ABSORB = 69,                                      // School Absorb
    SPELL_AURA_EXTRA_ATTACKS = 70,                                      // Extra Attacks
    SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL = 71,                       // Mod School Spell Crit Chance
    SPELL_AURA_MOD_POWER_COST = 72,                                     // Mod Power Cost
    SPELL_AURA_MOD_POWER_COST_SCHOOL = 73,                              // Mod School Power Cost
    SPELL_AURA_REFLECT_SPELLS_SCHOOL = 74,                              // Reflect School Spells %
    SPELL_AURA_MOD_LANGUAGE = 75,                                       // Mod Language
    SPELL_AURA_FAR_SIGHT = 76,                                          // Far Sight
    SPELL_AURA_MECHANIC_IMMUNITY = 77,                                  // Immune Mechanic
    SPELL_AURA_MOUNTED = 78,                                            // Mounted
    SPELL_AURA_MOD_DAMAGE_PERCENT_DONE = 79,                            // Mod Dmg %
    SPELL_AURA_MOD_PERCENT_STAT = 80,                                   // Mod Stat %
    SPELL_AURA_SPLIT_DAMAGE = 81,                                       // Split Damage
    SPELL_AURA_WATER_BREATHING = 82,                                    // Water Breathing
    SPELL_AURA_MOD_BASE_RESISTANCE = 83,                                // Mod Base Resistance
    SPELL_AURA_MOD_REGEN = 84,                                          // Mod Health Regen
    SPELL_AURA_MOD_POWER_REGEN = 85,                                    // Mod Power Regen
    SPELL_AURA_CHANNEL_DEATH_ITEM = 86,                                 // Create Death Item
    SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN = 87,                           // Mod Dmg % Taken
    SPELL_AURA_MOD_PERCENT_REGEN = 88,                                  // Mod Health Regen Percent
    SPELL_AURA_PERIODIC_DAMAGE_PERCENT = 89,                            // Periodic Damage Percent
    SPELL_AURA_MOD_RESIST_CHANCE = 90,                                  // Mod Resist Chance
    SPELL_AURA_MOD_DETECT_RANGE = 91,                                   // Mod Detect Range
    SPELL_AURA_PREVENTS_FLEEING = 92,                                   // Prevent Fleeing
    SPELL_AURA_MOD_UNATTACKABLE = 93,                                   // Mod Unintractable
    SPELL_AURA_INTERRUPT_REGEN = 94,                                    // Interrupt Regen
    SPELL_AURA_GHOST = 95,                                              // Ghost
    SPELL_AURA_SPELL_MAGNET = 96,                                       // Spell Magnet
    SPELL_AURA_MANA_SHIELD = 97,                                        // Mana Shield
    SPELL_AURA_MOD_SKILL_TALENT = 98,                                   // Mod Skill Talent
    SPELL_AURA_MOD_ATTACK_POWER = 99,                                   // Mod Attack Power
    SPELL_AURA_AURAS_VISIBLE = 100,                                     // Auras Visible
    SPELL_AURA_MOD_RESISTANCE_PCT = 101,                                // Mod Resistance %
    SPELL_AURA_MOD_CREATURE_ATTACK_POWER = 102,                         // Mod Creature Attack Power
    SPELL_AURA_MOD_TOTAL_THREAT = 103,                                  // Mod Total Threat (Fade)
    SPELL_AURA_WATER_WALK = 104,                                        // Water Walk
    SPELL_AURA_FEATHER_FALL = 105,                                      // Feather Fall
    SPELL_AURA_HOVER = 106,                                             // Hover
    SPELL_AURA_ADD_FLAT_MODIFIER = 107,                                 // Add Flat Modifier
    SPELL_AURA_ADD_PCT_MODIFIER = 108,                                  // Add % Modifier
    SPELL_AURA_ADD_CLASS_TARGET_TRIGGER = 109,                          // Add Class Target Trigger
    SPELL_AURA_MOD_POWER_REGEN_PERCENT = 110,                           // Mod Power Regen %
    SPELL_AURA_ADD_CASTER_HIT_TRIGGER = 111,                            // Add Class Caster Hit Trigger
    SPELL_AURA_OVERRIDE_CLASS_SCRIPTS = 112,                            // Override Class Scripts
    SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN = 113,                           // Mod Ranged Dmg Taken
    SPELL_AURA_MOD_RANGED_DAMAGE_TAKEN_PCT = 114,                       // Mod Ranged % Dmg Taken
    SPELL_AURA_MOD_HEALING = 115,                                       // Mod Healing
    SPELL_AURA_IGNORE_REGEN_INTERRUPT = 116,                            // Regen During Combat
    SPELL_AURA_MOD_MECHANIC_RESISTANCE = 117,                           // Mod Mechanic Resistance
    SPELL_AURA_MOD_HEALING_PCT = 118,                                   // Mod Healing %
    SPELL_AURA_SHARE_PET_TRACKING = 119,                                // Share Pet Tracking
    SPELL_AURA_UNTRACKABLE = 120,                                       // Untrackable
    SPELL_AURA_EMPATHY = 121,                                           // Empathy (Lore, whatever)
    SPELL_AURA_MOD_OFFHAND_DAMAGE_PCT = 122,                            // Mod Offhand Dmg %
    SPELL_AURA_MOD_POWER_COST_PCT = 123,                                // Mod Power Cost % --> armor penetration & spell penetration
    SPELL_AURA_MOD_RANGED_ATTACK_POWER = 124,                           // Mod Ranged Attack Power
    SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN = 125,                            // Mod Melee Dmg Taken
    SPELL_AURA_MOD_MELEE_DAMAGE_TAKEN_PCT = 126,                        // Mod Melee % Dmg Taken
    SPELL_AURA_RANGED_ATTACK_POWER_ATTACKER_BONUS = 127,                // Rngd Atk Pwr Attckr Bonus
    SPELL_AURA_MOD_POSSESS_PET = 128,                                   // Mod Possess Pet
    SPELL_AURA_MOD_INCREASE_SPEED_ALWAYS = 129,                         // Mod Speed Always
    SPELL_AURA_MOD_MOUNTED_SPEED_ALWAYS = 130,                          // Mod Mounted Speed Always
    SPELL_AURA_MOD_CREATURE_RANGED_ATTACK_POWER = 131,                  // Mod Creature Ranged Attack Power
    SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT = 132,                       // Mod Increase Energy %
    SPELL_AURA_MOD_INCREASE_HEALTH_PERCENT = 133,                       // Mod Max Health %
    SPELL_AURA_MOD_MANA_REGEN_INTERRUPT = 134,                          // Mod Interrupted Mana Regen
    SPELL_AURA_MOD_HEALING_DONE = 135,                                  // Mod Healing Done
    SPELL_AURA_MOD_HEALING_DONE_PERCENT = 136,                          // Mod Healing Done %
    SPELL_AURA_MOD_TOTAL_STAT_PERCENTAGE = 137,                         // Mod Total Stat %
    SPELL_AURA_MOD_HASTE = 138,                                         // Haste - Melee
    SPELL_AURA_FORCE_REACTION = 139,                                    // Force Reaction
    SPELL_AURA_MOD_RANGED_HASTE = 140,                                  // Haste - Ranged
    SPELL_AURA_MOD_RANGED_AMMO_HASTE = 141,                             // Haste - Ranged (Ammo Only)
    SPELL_AURA_MOD_BASE_RESISTANCE_PCT = 142,                           // Mod Base Resistance %
    SPELL_AURA_MOD_RESISTANCE_EXCLUSIVE = 143,                          // Mod Resistance Exclusive
    SPELL_AURA_SAFE_FALL = 144,                                         // Safe Fall
    SPELL_AURA_CHARISMA = 145,                                          // Charisma
    SPELL_AURA_PERSUADED = 146,                                         // Persuaded
    SPELL_AURA_MECHANIC_IMMUNITY_MASK = 147,                            // Mechanic Immunity Mask
    SPELL_AURA_RETAIN_COMBO_POINTS = 148,                               // Retain Combo Points
    SPELL_AURA_RESIST_PUSHBACK = 149,                                   // Resist Pushback
    SPELL_AURA_MOD_SHIELD_BLOCK_PCT = 150,                              // Mod Shield Block %
    SPELL_AURA_TRACK_STEALTHED = 151,                                   // Track Stealthed
    SPELL_AURA_MOD_DETECTED_RANGE = 152,                                // Mod Detected Range
    SPELL_AURA_SPLIT_DAMAGE_FLAT = 153,                                 // Split Damage Flat
    SPELL_AURA_MOD_STEALTH_LEVEL = 154,                                 // Stealth Level Modifier
    SPELL_AURA_MOD_WATER_BREATHING = 155,                               // Mod Water Breathing
    SPELL_AURA_MOD_REPUTATION_ADJUST = 156,                             // Mod Reputation Gain
    SPELL_AURA_PET_DAMAGE_MULTI = 157,                                  // Mod Pet Damage
    SPELL_AURA_MOD_SHIELD_BLOCK = 158,                                  // Mod Shield Block
    SPELL_AURA_NO_PVP_CREDIT = 159,                                     // No PVP Credit
    SPELL_AURA_MOD_SIDE_REAR_PDAE_DAMAGE_TAKEN = 160,                   // Mod Side/Rear PBAE Damage Taken
    SPELL_AURA_MOD_HEALTH_REGEN_IN_COMBAT = 161,                        // Mod Health Regen In Combat
    SPELL_AURA_PERIODIC_POWER_BURN = 162,                               // Power Burn
    SPELL_AURA_MOD_CRIT_DAMAGE_BONUS_MELEE = 163,                       // Mod Critical Damage Bonus (Physical)
    SPELL_AURA_MELEE_ATTACK_POWER_ATTACKER_BONUS = 165,                 // Melee AP Attacker Bonus
    SPELL_AURA_MOD_ATTACK_POWER_PCT = 166,                              // Mod Attack Power
    SPELL_AURA_MOD_RANGED_ATTACK_POWER_PCT = 167,                       // Mod Ranged Attack Power %
    SPELL_AURA_INCREASE_DAMAGE = 168,                                   // Increase Damage Type
    SPELL_AURA_INCREASE_CRITICAL = 169,                                 // Increase Critical Type
    SPELL_AURA_DETECT_AMORE = 170,                                      // Detect Amore
    SPELL_AURA_INCREASE_MOVEMENT_AND_MOUNTED_SPEED = 172,               // Increase Movement and Mounted Speed (Non-Stacking)
    SPELL_AURA_INCREASE_SPELL_DAMAGE_PCT = 174,                         // Increase Spell Damage by % status
    SPELL_AURA_INCREASE_SPELL_HEALING_PCT = 175,                        // Increase Spell Healing by % status
    SPELL_AURA_SPIRIT_OF_REDEMPTION = 176,                              // Spirit of Redemption Auras
    SPELL_AURA_AREA_CHARM = 177,                                        // Area Charm
    SPELL_AURA_INCREASE_ATTACKER_SPELL_CRIT = 179,                      // Increase Attacker Spell Crit Type
    SPELL_AURA_INCREASE_SPELL_DAMAGE_VS_TYPE = 180,                     // Increase Spell Damage Type
    SPELL_AURA_INCREASE_ARMOR_BASED_ON_INTELLECT_PCT = 182,             // Increase Armor based on Intellect
    SPELL_AURA_DECREASE_CRIT_THREAT = 183,                              // Decrease Critical Threat by
    SPELL_AURA_DECREASE_ATTACKER_CHANCE_TO_HIT_MELEE = 184,             // Reduces Attacker Chance to Hit with Melee
    SPELL_AURA_DECREASE_ATTACKER_CHANGE_TO_HIT_RANGED = 185,            // Reduces Attacker Chance to Hit with Ranged
    SPELL_AURA_DECREASE_ATTACKER_CHANGE_TO_HIT_SPELLS = 186,            // Reduces Attacker Chance to Hit with Spells
    SPELL_AURA_DECREASE_ATTACKER_CHANGE_TO_CRIT_MELEE = 187,            // Reduces Attacker Chance to Crit with Melee (Ranged?)
    SPELL_AURA_DECREASE_ATTACKER_CHANGE_TO_CRIT_RANGED = 188,           // Reduces Attacker Chance to Crit with Ranged (Melee?)
    SPELL_AURA_INCREASE_REPUTATION = 190,                               // Increases reputation from killed creatures
    SPELL_AURA_SPEED_LIMIT = 191,                                       // speed limit
    // TBC begins
#if VERSION_STRING >= TBC
    SPELL_AURA_MELEE_SLOW_PCT = 192,
    SPELL_AURA_INCREASE_TIME_BETWEEN_ATTACKS = 193,
    SPELL_AURA_INREASE_SPELL_DAMAGE_PCT_OF_INTELLECT = 194,             // NOT USED ANYMORE - 174 used instead
    SPELL_AURA_INCREASE_HEALING_PCT_OF_INTELLECT = 195,                 // NOT USED ANYMORE - 175 used instead
    SPELL_AURA_MOD_ALL_WEAPON_SKILLS = 196,
    SPELL_AURA_REDUCE_ATTACKER_CRICTICAL_HIT_CHANCE_PCT = 197,
    SPELL_AURA_198 = 198,
    SPELL_AURA_INCREASE_SPELL_HIT_PCT = 199,
    SPELL_AURA_FLY = 201,
    SPELL_AURA_FINISHING_MOVES_CANNOT_BE_DODGED = 202,
    SPELL_AURA_REDUCE_ATTACKER_CRICTICAL_HIT_DAMAGE_MELEE_PCT = 203,
    SPELL_AURA_REDUCE_ATTACKER_CRICTICAL_HIT_DAMAGE_RANGED_PCT = 204,
    SPELL_AURA_ENABLE_FLIGHT = 206,
    SPELL_AURA_ENABLE_FLIGHT2 = 207,
    SPELL_AURA_ENABLE_FLIGHT_WITH_UNMOUNTED_SPEED = 208,
    SPELL_AURA_MOD_RANGED_ATTACK_POWER_BY_STAT_PCT = 212,
    SPELL_AURA_INCREASE_RAGE_FROM_DAMAGE_DEALT_PCT = 213,
    SPELL_AURA_INCREASE_CASTING_TIME_PCT = 216,
    SPELL_AURA_REGEN_MANA_STAT_PCT = 219,
    SPELL_AURA_HEALING_STAT_PCT = 220,
    SPELL_AURA_MOD_DETAUNT = 221,
    SPELL_AURA_PERIODIC_TRIGGER_DUMMY = 226,
    SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE = 227,                 // Used by Mind Flay etc
    SPELL_AURA_DETECT_STEALTH = 228,
    SPELL_AURA_REDUCE_AOE_DAMAGE_TAKEN = 229,
    SPELL_AURA_INCREASE_MAX_HEALTH = 230,                               // Used by Commanding Shout
    SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE = 231,
    SPELL_AURA_MECHANIC_DURATION_MOD = 232,
    SPELL_AURA_CONTROL_VEHICLE = 236,
    SPELL_AURA_MOD_HEALING_BY_AP = 237,
    SPELL_AURA_MOD_SPELL_DAMAGE_BY_AP = 238,
    SPELL_AURA_EXPERTISE = 240,
    SPELL_AURA_FORCE_MOVE_FORWARD = 241,
    SPELL_AURA_MOD_SPELL_DAMAGE_FROM_HEALING = 242,
    SPELL_AURA_243 = 243,
    SPELL_AURA_COMPREHEND_LANG = 244,
    SPELL_AURA_MOD_DURATION_OF_MAGIC_EFFECTS = 245,
    SPELL_AURA_246 = 246,
    SPELL_AURA_MIRROR_IMAGE = 247,
    SPELL_AURA_MOD_COMBAT_RESULT_CHANCE = 248,
    SPELL_AURA_CONVERT_RUNE = 249,
    SPELL_AURA_MOD_INCREASE_HEALTH_2 = 250,
    SPELL_AURA_MOD_ENEMY_DODGE = 251,
    SPELL_AURA_252 = 252,
    SPELL_AURA_BLOCK_MULTIPLE_DAMAGE = 253,
    SPELL_AURA_MOD_DISARM_OFFHAND = 254,
    SPELL_AURA_MOD_MECHANIC_DAMAGE_TAKEN_PERCENT = 255,
    SPELL_AURA_256 = 256,
    SPELL_AURA_257 = 257,
    SPELL_AURA_258 = 258,
    SPELL_AURA_259 = 259,
    SPELL_AURA_260 = 260,
    SPELL_AURA_PHASE = 261,
#endif
    // Wotlk begins
#if VERSION_STRING >= WotLK
    SPELL_AURA_IGNORE_TARGET_AURA_STATE = 262,
    SPELL_AURA_ALLOW_ONLY_ABILITY = 263,
    SPELL_AURA_264 = 264,
    SPELL_AURA_265 = 265,
    SPELL_AURA_266 = 266,
    SPELL_AURA_267 = 267,
    SPELL_AURA_MOD_ATTACK_POWER_BY_STAT_PCT = 268,
    SPELL_AURA_269 = 269,
    SPELL_AURA_270 = 270,
    SPELL_AURA_INCREASE_SPELL_DOT_DAMAGE_PCT = 271,
    SPELL_AURA_272 = 272,
    SPELL_AURA_273 = 273,
    SPELL_AURA_CONSUMES_NO_AMMO = 274,
    SPELL_AURA_IGNORE_SHAPESHIFT = 275,
    SPELL_AURA_276 = 276,
    SPELL_AURA_277 = 277,
    SPELL_AURA_MOD_DISARM_RANGED = 278,
    SPELL_AURA_279 = 279,
    SPELL_AURA_IGNORE_ARMOR_PCT = 280,
    SPELL_AURA_281 = 281,
    SPELL_AURA_MOD_BASE_HEALTH = 282,
    SPELL_AURA_283 = 283,
    SPELL_AURA_284 = 284,
    SPELL_AURA_MOD_ATTACK_POWER_OF_ARMOR = 285,
    SPELL_AURA_ALLOW_DOT_TO_CRIT = 286,
    SPELL_AURA_DEFLECT_SPELLS = 287,
    SPELL_AURA_288 = 288,
    SPELL_AURA_289 = 289,
    SPELL_AURA_290 = 290,
    SPELL_AURA_MOD_XP_QUEST_PCT = 291,
    SPELL_AURA_292 = 292,
    SPELL_AURA_293 = 293,
    SPELL_AURA_294 = 294,
    SPELL_AURA_295 = 295,
    SPELL_AURA_296 = 296,
    SPELL_AURA_297 = 297,
    SPELL_AURA_298 = 298,
    SPELL_AURA_299 = 299,
    SPELL_AURA_300 = 300,
    SPELL_AURA_301 = 301,
    SPELL_AURA_302 = 302,
    SPELL_AURA_303 = 303,
    SPELL_AURA_304 = 304,
    SPELL_AURA_305 = 305,
    SPELL_AURA_306 = 306,
    SPELL_AURA_307 = 307,
    SPELL_AURA_308 = 308,
    SPELL_AURA_309 = 309,
    SPELL_AURA_310 = 310,
    SPELL_AURA_311 = 311,
    SPELL_AURA_312 = 312,
    SPELL_AURA_313 = 313,
    SPELL_AURA_PREVENT_RESURRECTION = 314,
    SPELL_AURA_315 = 315,
    SPELL_AURA_ALLOW_HASTE_AFFECT_DURATION = 316,
#endif
    // Cata begins
#if VERSION_STRING >= Cata
    SPELL_AURA_317 = 317,
    SPELL_AURA_318 = 318,
    SPELL_AURA_319 = 319,
    SPELL_AURA_320 = 320,
    SPELL_AURA_321 = 321,
    SPELL_AURA_322 = 322,
    SPELL_AURA_323 = 323,
    SPELL_AURA_324 = 324,
    SPELL_AURA_325 = 325,
    SPELL_AURA_326 = 326,
    SPELL_AURA_327 = 327,
    SPELL_AURA_328 = 328,
    SPELL_AURA_329 = 329,
    SPELL_AURA_330 = 330,
    SPELL_AURA_331 = 331,
    SPELL_AURA_332 = 332,
    SPELL_AURA_333 = 333,
    SPELL_AURA_334 = 334,
    SPELL_AURA_335 = 335,
    SPELL_AURA_336 = 336,
    SPELL_AURA_337 = 337,
    SPELL_AURA_338 = 338,
    SPELL_AURA_339 = 339,
    SPELL_AURA_340 = 340,
    SPELL_AURA_341 = 341,
    SPELL_AURA_342 = 342,
    SPELL_AURA_343 = 343,
    SPELL_AURA_344 = 344,
    SPELL_AURA_345 = 345,
    SPELL_AURA_346 = 346,
    SPELL_AURA_347 = 347,
    SPELL_AURA_348 = 348,
    SPELL_AURA_349 = 349,
    SPELL_AURA_350 = 350,
    SPELL_AURA_351 = 351,
    SPELL_AURA_352 = 352,
    SPELL_AURA_353 = 353,
    SPELL_AURA_354 = 354,
    SPELL_AURA_355 = 355,
    SPELL_AURA_356 = 356,
    SPELL_AURA_357 = 357,
    SPELL_AURA_358 = 358,
    SPELL_AURA_359 = 359,
    SPELL_AURA_360 = 360,
    SPELL_AURA_361 = 361,
    SPELL_AURA_362 = 362,
    SPELL_AURA_363 = 363,
    SPELL_AURA_364 = 364,
    SPELL_AURA_365 = 365,
    SPELL_AURA_366 = 366,
    SPELL_AURA_367 = 367,
    SPELL_AURA_368 = 368,
    SPELL_AURA_369 = 369,
    SPELL_AURA_370 = 370,
#endif
    // Mop begins
#if VERSION_STRING >= Mop
    SPELL_AURA_371 = 371,
    SPELL_AURA_372 = 372,
    SPELL_AURA_373 = 373,
    SPELL_AURA_374 = 374,                                               // SPELL_AURA_MODIFY_FALL_DAMAGE_PCT
    SPELL_AURA_375 = 375,
    SPELL_AURA_376 = 376,                                               // SPELL_AURA_MOD_CURRENCY_GAIN2
    SPELL_AURA_377 = 377,                                               // SPELL_AURA_CAST_WHILE_WALKING2
    SPELL_AURA_378 = 378,
    SPELL_AURA_379 = 379,
    SPELL_AURA_380 = 380,
    SPELL_AURA_381 = 381,
    SPELL_AURA_382 = 382,
    SPELL_AURA_383 = 383,
    SPELL_AURA_384 = 384,
    SPELL_AURA_385 = 385,
    SPELL_AURA_386 = 386,
    SPELL_AURA_387 = 387,
    SPELL_AURA_388 = 388,
    SPELL_AURA_389 = 389,
    SPELL_AURA_390 = 390,
    SPELL_AURA_391 = 391,
    SPELL_AURA_392 = 392,
    SPELL_AURA_393 = 393,
    SPELL_AURA_394 = 394,
    SPELL_AURA_395 = 395,
    SPELL_AURA_396 = 396,
    SPELL_AURA_397 = 397,
    SPELL_AURA_398 = 398,
    SPELL_AURA_399 = 399,
    SPELL_AURA_400 = 400,
    SPELL_AURA_401 = 401,
    SPELL_AURA_402 = 402,
    SPELL_AURA_403 = 403,
    SPELL_AURA_404 = 404,
    SPELL_AURA_405 = 405,
    SPELL_AURA_406 = 406,
    SPELL_AURA_407 = 407,
    SPELL_AURA_408 = 408,
    SPELL_AURA_409 = 409,
    SPELL_AURA_410 = 410,
    SPELL_AURA_411 = 411,
    SPELL_AURA_412 = 412,
    SPELL_AURA_413 = 413,
    SPELL_AURA_414 = 414,
    SPELL_AURA_415 = 415,
    SPELL_AURA_416 = 416,
    SPELL_AURA_417 = 417,
    SPELL_AURA_418 = 418,
    SPELL_AURA_419 = 419,
    SPELL_AURA_420 = 420,
    SPELL_AURA_421 = 421,
    SPELL_AURA_422 = 422,
    SPELL_AURA_423 = 423,
    SPELL_AURA_424 = 424,
    SPELL_AURA_425 = 425,
    SPELL_AURA_426 = 426,
    SPELL_AURA_427 = 427,
    SPELL_AURA_428 = 428,
    SPELL_AURA_429 = 429,
    SPELL_AURA_430 = 430,
    SPELL_AURA_431 = 431,
    SPELL_AURA_432 = 432,
    SPELL_AURA_433 = 433,
    SPELL_AURA_434 = 434,
    SPELL_AURA_435 = 435,
    SPELL_AURA_436 = 436,
    SPELL_AURA_437 = 437,
#endif
    TOTAL_SPELL_AURAS
};
