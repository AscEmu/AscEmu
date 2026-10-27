/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const ShrineOfTheStormEncounterCount = 4;

enum ShrineOfTheStormData
{
    DATA_AQUSIRR          = 0, // 2130
    // Galecaller Faye and Brother Ironhull
    DATA_TIDESAGE_COUNCIL = 1, // 2131
    DATA_LORD_STORMSONG   = 2, // 2132
    DATA_VOLZITH          = 3  // 2133
};

enum ShrineOfTheStormCreatures
{
    BOSS_AQUSIRR                  = 134056,
    NPC_AQUALING                  = 134828,
    NPC_GRASPING_TENTACLES        = 134612,

    BOSS_GALECALLER_FAYE          = 134058,
    BOSS_BROTHER_IRONHULL         = 134063,

    BOSS_LORD_STORMSONG           = 134060,
    NPC_AWOKEN_VOID               = 137036,

    BOSS_VOLZITH                  = 134069,
    NPC_VOLZITH_TENTACLE          = 135987,
    NPC_FORGOTTEN_DENIZEN         = 136297,
    NPC_SUNKEN_DENIZEN            = 136295,
    NPC_MANIFESTATION_OF_THE_DEEP = 135903
};

enum ShrineOfTheStormActions
{
    ACTION_VOLZITH_ABYSS_ENERGY = 1
};

enum AqusirrSpells
{
    SPELL_AQUSIRR_SEA_BLAST             = 265001,
    SPELL_AQUSIRR_CHOKING_BRINE         = 264560,
    SPELL_AQUSIRR_SURGING_RUSH          = 264101,
    SPELL_AQUSIRR_UNDERTOW              = 264144,
    SPELL_AQUSIRR_GRASP_FROM_THE_DEPTHS = 264477,
    SPELL_AQUSIRR_GRASP_HOLD            = 264526,
    SPELL_AQUALING_DIMINISH             = 264899
};

enum TidesageCouncilSpells
{
    SPELL_IRONHULL_HINDERING_CLEAVE      = 267899,
    SPELL_IRONHULL_BLESSING_OF_IRONSIDES = 267901,
    SPELL_IRONHULL_REINFORCING_WARD      = 267905,

    SPELL_FAYE_SLICING_BLAST             = 267818,
    SPELL_FAYE_BLESSING_OF_THE_TEMPEST   = 267830,
    SPELL_FAYE_SWIFTNESS_WARD            = 267891
};

enum LordStormsongSpells
{
    SPELL_STORMSONG_VOID_BOLT          = 268347,
    SPELL_STORMSONG_MIND_REND          = 268896,
    SPELL_STORMSONG_WAKEN_THE_VOID     = 269097,
    SPELL_STORMSONG_ANCIENT_MINDBENDER = 269131,

    SPELL_AWOKEN_VOID_FIXATE           = 269103,
    SPELL_AWOKEN_VOID_EXPLOSIVE_VOID   = 269104
};

enum VolzithSpells
{
    SPELL_VOLZITH_YAWNING_GATE              = 269399,
    SPELL_VOLZITH_WHISPERS_OF_POWER         = 267034,
    SPELL_VOLZITH_CALL_THE_ABYSS            = 267299,
    SPELL_VOLZITH_GRASP_OF_THE_SUNKEN_CITY  = 267444,

    SPELL_TENTACLE_SLAM                     = 267385,
    SPELL_FORGOTTEN_DENIZEN_CONSUME_ESSENCE = 267809,
    SPELL_SUNKEN_DENIZEN_LEVIATHANS_WAKE    = 267548,
    SPELL_MANIFESTATION_ABYSSAL_ERUPTION    = 267310
};
