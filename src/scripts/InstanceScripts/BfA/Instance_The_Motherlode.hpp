/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const TheMotherlodeEncounterCount = 4;

enum TheMotherlodeData
{
    DATA_CROWD_PUMMELER     = 0,  // 2105
    DATA_AZEROKK            = 1,  // 2106
    DATA_RIXXA_FLUXFLAME    = 2,  // 2107
    DATA_MOGUL_RAZDUNK      = 3   // 2108
};

enum TheMotherlodeCreatures
{
    BOSS_CROWD_PUMMELER     = 129214,
    NPC_AZERITE_FOOTBOMB    = 129246,

    BOSS_AZEROKK            = 129227,
    NPC_EARTHRAGER          = 129802,

    BOSS_RIXXA_FLUXFLAME    = 129231,

    BOSS_MOGUL_RAZDUNK      = 129232
};

enum TheMotherlodePummelerSpells
{
    SPELL_PUMMELER_STATIC_PULSE         = 262347,
    SPELL_PUMMELER_FOOTBOMB_LAUNCHER    = 269493,
    SPELL_PUMMELER_SHOCKING_CLAW        = 257337,
    SPELL_PUMMELER_COIN_MAGNET          = 271903,
    SPELL_PUMMELER_PAY_TO_WIN           = 271867,
    SPELL_FOOTBOMB_TIMED_DETONATION     = 256137
};

enum TheMotherlodeAzerokkSpells
{
    SPELL_AZEROKK_RESONANT_PULSE        = 258622,
    SPELL_AZEROKK_AZERITE_INFUSION      = 271698,
    SPELL_AZEROKK_CALL_EARTHRAGER       = 257593,
    SPELL_AZEROKK_TECTONIC_SMASH        = 275907,

    SPELL_EARTHRAGER_JAGGED_CUT         = 257543,
    SPELL_EARTHRAGER_RESONANT_QUAKE     = 258627,
    // player totem, rooted ragers are skipped
    SPELL_FRACKING_TOTEM_STUN           = 257481
};

enum TheMotherlodeRixxaSpells
{
    SPELL_RIXXA_CHEMICAL_BURN           = 259853,
    SPELL_RIXXA_PROPELLANT_BLAST        = 260669,
    SPELL_RIXXA_SEARING_REAGENT         = 259474
};

enum TheMotherlodeRazdunkSpells
{
    SPELL_RAZDUNK_GATLING_GUN           = 260280,
    SPELL_RAZDUNK_HOMING_MISSILE        = 260838,
    SPELL_RAZDUNK_ALPHA_CANNON          = 260318,
    SPELL_RAZDUNK_CONFIGURATION_DRILL   = 260189,
    SPELL_RAZDUNK_DRILL_SMASH           = 260202,
    SPELL_RAZDUNK_MICRO_MISSILES        = 276229
};
