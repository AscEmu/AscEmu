/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const SanguineDepthsEncounterCount = 4;

enum SanguineDepthsData
{
    DATA_KRYXIS_THE_VORACIOUS       = 0,    // 2360
    DATA_EXECUTOR_TARVOLD           = 1,    // 2361
    DATA_GRAND_PROCTOR_BERYLLIA     = 2,    // 2362
    DATA_GENERAL_KAAL               = 3     // 2363
};

enum SanguineDepthsCreatures
{
    // Kryxis the Voracious
    BOSS_KRYXIS_THE_VORACIOUS               = 162100,

    // Executor Tarvold
    BOSS_EXECUTOR_TARVOLD                   = 162103,
    NPC_TARVOLD_FLEETING_MANIFESTATION      = 165556,
    // second Fleeting Manifestation template of the dungeon
    NPC_TARVOLD_FLEETING_MANIFESTATION_ALT  = 168882,

    // Grand Proctor Beryllia
    BOSS_GRAND_PROCTOR_BERYLLIA             = 162102,

    // General Kaal
    BOSS_GENERAL_KAAL                       = 162133,
    // second General Kaal template of the dungeon
    BOSS_GENERAL_KAAL_ALT                   = 162099,

    // the imprisoned naaru helping the players
    NPC_ZRALI                               = 166642
};

enum KryxisSpells
{
    SPELL_KRYXIS_SEVERING_SMASH             = 319685,
    SPELL_KRYXIS_JUGGERNAUT_RUSH            = 319713,
    SPELL_KRYXIS_HUNGERING_DRAIN            = 319654,
    SPELL_KRYXIS_VICIOUS_HEADBUTT           = 319650
};

enum ExecutorTarvoldSpells
{
    SPELL_TARVOLD_CASTIGATE                 = 322554,

    SPELL_MANIFESTATION_GROWING_PRIDE       = 323548,
    SPELL_MANIFESTATION_RESIDUE             = 323551
};

enum GrandProctorBerylliaSpells
{
    SPELL_BERYLLIA_RITE_OF_SUPREMACY        = 325360,
    SPELL_BERYLLIA_ENDLESS_TORMENT          = 326039,
    SPELL_BERYLLIA_ANGUISHED_CRIES          = 325885,
    SPELL_BERYLLIA_AGONIZE                  = 328593,
    SPELL_BERYLLIA_IRON_SPIKES              = 325254,

    SPELL_ZRALI_SHINING_RADIANCE_RITE       = 325742
};

enum GeneralKaalSpells
{
    SPELL_KAAL_GLOOM_SQUALL                 = 322895,
    SPELL_KAAL_WICKED_RUSH                  = 323845,
    SPELL_KAAL_WICKED_GASH                  = 322796,
    SPELL_KAAL_PIERCING_BLUR                = 323810,

    SPELL_ZRALI_SHINING_RADIANCE_SQUALL     = 324086
};
