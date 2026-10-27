/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const PlaguefallEncounterCount = 4;

enum PlaguefallData
{
    DATA_GLOBGROG               = 0,    // 2382
    DATA_DOCTOR_ICKUS           = 1,    // 2384
    DATA_DOMINA_VENOMBLADE      = 2,    // 2385
    DATA_MARGRAVE_STRADAMA      = 3     // 2386
};

enum PlaguefallCreatures
{
    // Globgrog
    BOSS_GLOBGROG                       = 164255,
    NPC_GLOBGROG_SLIMY_MORSEL           = 164362,
    // further Slimy Morsel templates of the dungeon
    NPC_GLOBGROG_SLIMY_MORSEL_ALT       = 168394,
    NPC_GLOBGROG_SLIMY_MORSEL_ALT2      = 168398,

    // Doctor Ickus
    BOSS_DOCTOR_ICKUS                   = 164967,
    NPC_ICKUS_PLAGUE_BOMB               = 169498,
    NPC_ICKUS_ERUPTING_OOZE             = 165001,
    // second Erupting Ooze template of the dungeon
    NPC_ICKUS_ERUPTING_OOZE_ALT         = 170927,
    NPC_ICKUS_PESTILENCE_SLIME          = 165357,

    // Domina Venomblade
    BOSS_DOMINA_VENOMBLADE              = 164266,
    NPC_DOMINA_BROOD_ASSASSIN           = 170474,

    // Margrave Stradama
    BOSS_MARGRAVE_STRADAMA              = 164267,
    NPC_STRADAMA_MALIGNANT_SPAWN        = 165430,
    NPC_STRADAMA_PLAGUE_TENTACLE        = 168364
};

enum GlobgrogSpells
{
    SPELL_GLOBGROG_CONSUME_SLIME        = 319780,
    SPELL_GLOBGROG_BECKON_SLIME         = 324459,
    SPELL_GLOBGROG_PLAGUESTOMP          = 324527,
    SPELL_GLOBGROG_SLIME_WAVE           = 324667
};

enum DoctorIckusSpells
{
    SPELL_ICKUS_SLIME_INJECTION         = 329110,
    SPELL_ICKUS_SLIME_LUNGE             = 329217,
    SPELL_ICKUS_PESTILENCE_SURGE        = 348159,
    SPELL_ICKUS_PESTILENCE_SURGE_HEROIC = 332617,
    SPELL_ICKUS_BURNING_STRAIN          = 322358,

    SPELL_PLAGUE_BOMB_VIRULENT_EXPLOSION = 321406,
    SPELL_ERUPTING_OOZE_BURSTING_OOZE   = 341595,
    SPELL_ERUPTING_OOZE_ERUPT           = 341444,
    SPELL_PESTILENCE_SLIME_RAPID_INFECTION = 331967,
    SPELL_PESTILENCE_SLIME_VIRAL_GLOBS  = 321999
};

enum DominaVenombladeSpells
{
    SPELL_DOMINA_SHADOW_AMBUSH          = 325245,
    SPELL_DOMINA_CYTOTOXIC_SLASH        = 325552,
    SPELL_DOMINA_VENOMBLADES            = 325551,

    SPELL_BROOD_ASSASSIN_SHROUDWEB      = 332397,
    SPELL_BROOD_ASSASSIN_ASSASSINATE    = 333406
};

enum MargraveStradamaSpells
{
    SPELL_STRADAMA_PLAGUE_CRASH         = 322473,
    SPELL_STRADAMA_MALIGNANT_GROWTH     = 322304,
    SPELL_STRADAMA_INFECTIOUS_RAIN      = 322232,
    SPELL_STRADAMA_PLAGUE_ROT           = 322491,

    SPELL_MALIGNANT_SPAWN_TOUCH_OF_SLIME = 322236
};
