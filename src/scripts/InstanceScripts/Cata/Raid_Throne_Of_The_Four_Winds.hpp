/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Throne of the Four Winds - two encounters (verified against wowhead): the Conclave of Wind
// (three elemental adds fought together) and Al'Akir. We had no instance script previously
// for this raid. This raid has no trash - confirmed against its own real creature spawn data,
// which contains only the bosses themselves plus vehicle/trigger entries (players fly directly
// between the two encounters), matching the real game's layout.

uint32_t const ThroneOfTheFourWindsEncounterCount = 2;

enum ThroneOfTheFourWindsData
{
    DATA_CONCLAVE_OF_WIND = 0,
    DATA_ALAKIR           = 1
};

enum ThroneOfTheFourWindsCreatures
{
    BOSS_ANSHAL = 45870,
    BOSS_NEZIR  = 45871,
    BOSS_ROHASH = 45872,
    BOSS_ALAKIR = 46753
};

// Cross-checked against wowhead's own Conclave of Wind ability list: Anshal's Zephyr - a
// channeled heal-plus-damage-buff on his two allies, and the actual reason Anshal is a
// priority interrupt target in the real fight - was missing entirely and is added below.
enum ConclaveSpells
{
    SPELL_ANSHAL_NURTURE      = 85422,
    SPELL_ANSHAL_TOXIC_SPORES = 86290,
    SPELL_ANSHAL_ZEPHYR       = 84638,

    SPELL_NEZIR_PERMAFROST    = 86082,
    SPELL_NEZIR_ICE_PATCH     = 86122,
    SPELL_NEZIR_SLEET_STORM   = 84644,

    SPELL_ROHASH_SLICING_GALE = 86182,
    SPELL_ROHASH_WIND_BLAST   = 86193
};

// Cross-checked against wowhead's own Al'Akir ability list: Wind Burst was already correct;
// Squall Line, Static Shock, Feedback and Lightning Rod were missing from the rotation
// entirely and are added below.
enum AlakirSpells
{
    SPELL_ALAKIR_WIND_BURST    = 87770,
    SPELL_ALAKIR_ICE_STORM     = 88239,
    SPELL_ALAKIR_ACID_RAIN     = 88290,
    SPELL_ALAKIR_LIGHTNING     = 89644,
    SPELL_ALAKIR_SQUALL_LINE   = 87856,
    SPELL_ALAKIR_STATIC_SHOCK  = 87873,
    SPELL_ALAKIR_FEEDBACK      = 87904,
    SPELL_ALAKIR_LIGHTNING_ROD = 89666
};
