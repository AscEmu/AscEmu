/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const TolDagorEncounterCount = 4;

enum TolDagorData
{
    DATA_SAND_QUEEN             = 0,  // 2101
    DATA_JES_HOWLIS             = 1,  // 2102
    DATA_KNIGHT_CAPTAIN_VALYRI  = 2,  // 2103
    DATA_OVERSEER_KORGUS        = 3   // 2104
};

enum TolDagorCreatures
{
    BOSS_SAND_QUEEN             = 127479,
    // marks a sand trap
    NPC_BUZZING_DRONE           = 131785,

    BOSS_JES_HOWLIS             = 127484,

    BOSS_KNIGHT_CAPTAIN_VALYRI  = 127490,

    BOSS_OVERSEER_KORGUS        = 127503,
    NPC_HEAVY_CANNON            = 134025
};

enum TolDagorGameObjects
{
    GO_SAND_QUEEN_SEWER_GRATE   = 284467,
    GO_VALYRI_ARMORY_GATE_1     = 281030,
    GO_VALYRI_ARMORY_GATE_2     = 281049
};

enum TolDagorSandQueenSpells
{
    SPELL_SAND_QUEEN_SANDSTORM          = 257495,
    SPELL_SAND_QUEEN_SAND_TRAP          = 257092,
    SPELL_SAND_QUEEN_SAND_TRAP_SPRUNG   = 257119,
    SPELL_SAND_QUEEN_UPHEAVAL_BURROW    = 257608,
    SPELL_SAND_QUEEN_UPHEAVAL_ERUPTION  = 257617
};

enum TolDagorJesHowlisSpells
{
    SPELL_JES_CRIPPLING_SHIV            = 257777,
    SPELL_JES_HOWLING_FEAR              = 257791,
    SPELL_JES_FLASHING_DAGGERS          = 257785
};

enum TolDagorValyriSpells
{
    SPELL_VALYRI_CINDERFLAME            = 256955,
    SPELL_VALYRI_FUSELIGHTER            = 257028,
    SPELL_VALYRI_IGNITION               = 256970
};

enum TolDagorKorgusSpells
{
    SPELL_KORGUS_ROUNDS_BLAST           = 256199,
    SPELL_KORGUS_ROUNDS_INCENDIARY      = 256198,
    SPELL_KORGUS_CROSS_IGNITION         = 256083,
    SPELL_KORGUS_DEADEYE_MARK           = 256038,
    SPELL_KORGUS_DEADEYE_SHOT           = 256044,
    SPELL_KORGUS_EXPLOSIVE_BURST_MARK   = 256105,
    SPELL_KORGUS_EXPLOSIVE_BURST_BLAST  = 256128,
    SPELL_HEAVY_CANNON_MASSIVE_BLAST    = 263345
};
