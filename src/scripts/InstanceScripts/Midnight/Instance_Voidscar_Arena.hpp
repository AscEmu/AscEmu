/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const VoidscarArenaEncounterCount = 3;

enum VoidscarArenaData
{
    DATA_TAZRAH   = 0,  // 3285
    DATA_ATROXUS  = 1,  // 3286
    DATA_CHARONUS = 2   // 3287
};

enum VoidscarArenaCreatures
{
    BOSS_TAZRAH         = 238887,
    NPC_ETHEREAL_SHADE  = 254677,

    BOSS_ATROXUS        = 239008,
    // second Atroxus template
    BOSS_ATROXUS_ALT    = 248014,
    NPC_TOXIC_CREEPER   = 239070,

    BOSS_CHARONUS       = 239167,
    // second Charonus template
    BOSS_CHARONUS_ALT   = 248015,
    NPC_GRAVITIC_ORB    = 255001
};

enum VoidscarArenaTazrahSpells
{
    SPELL_TAZRAH_VOID_BLAST     = 1297017,
    SPELL_TAZRAH_DARK_BLOOM     = 1300259,
    SPELL_TAZRAH_VOID_FISSURE   = 1296967,
    SPELL_TAZRAH_UMBRAL_RUPTURE = 1296963,

    SPELL_SHADE_NETHER_DASH     = 1222098
};

enum VoidscarArenaAtroxusSpells
{
    SPELL_ATROXUS_HULKING_CLAW   = 1222642,
    SPELL_ATROXUS_NOXIOUS_BREATH = 1222724,
    SPELL_ATROXUS_POISON_SPLASH  = 1226031,
    SPELL_ATROXUS_MONSTROUS_ROAR = 1262497,

    SPELL_CREEPER_SICKENING_BITE = 1282892
};

enum VoidscarArenaCharonusSpells
{
    SPELL_CHARONUS_UNSTABLE_SINGULARITY = 1264191,
    SPELL_CHARONUS_DARK_WAVES           = 1311923,
    SPELL_CHARONUS_COSMIC_CRASH         = 1227197,

    SPELL_ORB_CONDENSED_MASS            = 1263983
};
