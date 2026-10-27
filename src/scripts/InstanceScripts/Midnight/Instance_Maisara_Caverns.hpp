/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const MaisaraCavernsEncounterCount = 3;

enum MaisaraCavernsData
{
    // Muro'jin and Nekraxx
    DATA_MUROJIN_AND_NEKRAXX = 0,  // 3212
    DATA_VORDAZA             = 1,  // 3213
    DATA_RAKTUL              = 2   // 3214
};

enum MaisaraCavernsCreatures
{
    BOSS_MUROJIN          = 247570,
    BOSS_NEKRAXX          = 247572,

    BOSS_VORDAZA          = 248595,
    NPC_UNSTABLE_PHANTOM  = 250443,

    BOSS_RAKTUL           = 248605,
    NPC_SOULBIND_TOTEM    = 251047,
    NPC_LOST_SOUL         = 251639,
    NPC_MALIGNANT_SOUL    = 251674
};

enum MaisaraCavernsGameObjects
{
    // on the floor of the first arena, shut while the pair fights
    GO_MAISARA_BOSS_ONE_GATE = 616338
};

enum MaisaraCavernsActions
{
    ACTION_MAISARA_PET_DOWN = 1,
    ACTION_MAISARA_PET_REVIVED,
    ACTION_MAISARA_HUNTER_FELL
};

enum MaisaraCavernsMurojinSpells
{
    SPELL_MUROJIN_FREEZING_TRAP  = 1260731,
    SPELL_MUROJIN_BARRAGE        = 1260643,
    SPELL_MUROJIN_FLANKING_SPEAR = 1266480,
    SPELL_MUROJIN_REVIVE_PET     = 1249789
};

enum MaisaraCavernsNekraxxSpells
{
    SPELL_NEKRAXX_CARRION_SWOOP    = 1249479,
    SPELL_NEKRAXX_FETID_QUILLSTORM = 1243900,
    SPELL_NEKRAXX_INFECTED_PINIONS = 1246666,
    SPELL_NEKRAXX_BESTIAL_WRATH    = 1249948
};

enum MaisaraCavernsVordazaSpells
{
    SPELL_VORDAZA_NECROTIC_CONVERGENCE = 1250708,
    SPELL_VORDAZA_DEATHSHROUD          = 1251598,
    SPELL_VORDAZA_COALESCED_DEATH      = 1252611,
    SPELL_VORDAZA_WREST_PHANTOMS       = 1251204,
    SPELL_VORDAZA_UNMAKE               = 1252054,
    SPELL_VORDAZA_DRAIN_SOUL           = 1251554,
    SPELL_VORDAZA_WITHERING_MIASMA     = 1264987,

    SPELL_PHANTOM_SOULROT              = 1251833,
    SPELL_PHANTOM_LINGERING_DREAD      = 1251813,
    SPELL_PHANTOM_VEILED_PRESENCE      = 1264974
};

enum MaisaraCavernsRaktulSpells
{
    SPELL_RAKTUL_CRUSH_SOULS               = 1252676,
    SPELL_RAKTUL_SPIRITBREAKER             = 1251023,
    // damage pulse of the vessel, the journal aura only drives it
    SPELL_RAKTUL_DEATHGORGED_VESSEL        = 1248879,
    SPELL_RAKTUL_SOULRENDING_ROAR          = 1253788,
    SPELL_RAKTUL_WITHERING_SOUL            = 1253844,

    SPELL_TOTEM_SOULBIND                   = 1252777,
    SPELL_TOTEM_CHILL_OF_DEATH             = 1252816,

    SPELL_LOST_SOUL_CRIES_OF_THE_FALLEN    = 1254175,
    SPELL_MALIGNANT_SOUL_ETERNAL_SUFFERING = 1254010
};
