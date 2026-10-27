/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const PrioryOfTheSacredFlameEncounterCount = 3;

enum PrioryOfTheSacredFlameData
{
    DATA_BARON_BRAUNPYKE        = 0,    // 2835
    // Captain Dailcry and his Trusted Guard
    DATA_CAPTAIN_DAILCRY        = 1,    // 2847
    DATA_PRIORESS_MURRPRAY      = 2     // 2848
};

enum PrioryOfTheSacredFlameCreatures
{
    BOSS_BARON_BRAUNPYKE        = 207939,

    BOSS_CAPTAIN_DAILCRY        = 207946,
    BOSS_ELAENA_EMBERLANZ       = 211290,
    BOSS_SERGEANT_SHAYNEMAIL    = 211291,
    BOSS_TAENER_DUELMAL         = 211289,
    // second templates of the Trusted Guard
    BOSS_ELAENA_EMBERLANZ_ALT   = 239833,
    BOSS_SERGEANT_SHAYNEMAIL_ALT = 239836,
    BOSS_TAENER_DUELMAL_ALT     = 239834,

    BOSS_PRIORESS_MURRPRAY      = 207940,
    NPC_ARATHI_NEOPHYTE         = 207943,
    NPC_ETERNAL_FLAME           = 210851
};

enum BaronBraunpykeSpells
{
    SPELL_BRAUNPYKE_VINDICTIVE_WRATH        = 422969,
    SPELL_BRAUNPYKE_CASTIGATORS_SHIELD      = 423015,
    SPELL_BRAUNPYKE_CASTIGATORS_DETONATION  = 423019,
    SPELL_BRAUNPYKE_BURNING_LIGHT           = 423051,
    SPELL_BRAUNPYKE_HAMMER_OF_PURITY        = 423062
};

enum CaptainDailcrySpells
{
    SPELL_DAILCRY_SAVAGE_MAULING            = 447439,
    SPELL_DAILCRY_BATTLE_CRY                = 424419,
    SPELL_DAILCRY_EARTHSHATTERING_SPEAR     = 1238779,
    SPELL_DAILCRY_PIERCE_ARMOR              = 424414,

    SPELL_GUARD_STRENGTH_IN_NUMBERS         = 424628,
    SPELL_ELAENA_HOLY_RADIANCE              = 424431,
    SPELL_ELAENA_DIVINE_JUDGMENT            = 448515,
    SPELL_SHAYNEMAIL_BRUTAL_SMASH           = 424621,
    SPELL_SHAYNEMAIL_LUNGING_STRIKE         = 424423,
    SPELL_TAENER_EMBER_STORM                = 424462,
    SPELL_TAENER_FIREBALL                   = 424421
};

enum PrioressMurrpraySpells
{
    SPELL_MURRPRAY_BARRIER_OF_LIGHT         = 423588,
    SPELL_MURRPRAY_EMBRACE_THE_LIGHT        = 423665,
    SPELL_MURRPRAY_THE_SACRED_FLAME         = 425544,
    SPELL_MURRPRAY_INNER_FIRE               = 423539,
    SPELL_MURRPRAY_HOLY_FLAME               = 451606,
    SPELL_MURRPRAY_HOLY_SMITE               = 423536,

    SPELL_NEOPHYTE_OVERWHELMING_POWER       = 425022
};
