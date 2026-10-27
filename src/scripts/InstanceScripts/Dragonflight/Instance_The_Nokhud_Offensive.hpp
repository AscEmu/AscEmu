/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const TheNokhudOffensiveEncounterCount = 4;

enum TheNokhudOffensiveData
{
    DATA_BALAKAR_KHAN       = 0, // 2580
    // Teera and Maruuk
    DATA_TEERA_AND_MARUUK   = 1, // 2581
    DATA_THE_RAGING_TEMPEST = 2, // 2636
    DATA_GRANYTH            = 3  // 2637
};

enum TheNokhudOffensiveCreatures
{
    BOSS_BALAKAR_KHAN           = 186151,
    NPC_NOKHUD_STORMCASTER      = 190294,

    BOSS_TEERA                  = 186339,
    BOSS_MARUUK                 = 186338,

    BOSS_THE_RAGING_TEMPEST     = 186615,

    BOSS_GRANYTH                = 186616,
    NPC_DRAGONKILLER_LANCE      = 194367,
    NPC_NOKHUD_SABOTEUR         = 195580,
    // further saboteur templates of the same encounter
    NPC_NOKHUD_SABOTEUR_SECOND  = 195820,
    NPC_NOKHUD_SABOTEUR_THIRD   = 195821
};

enum TheNokhudOffensiveActions
{
    ACTION_MARUUK_FRIGHTFUL_ROAR = 1,
    ACTION_TEERA_REPEL
};

enum NokhudOffensiveBalakarKhanSpells
{
    SPELL_BALAKAR_IRON_SPEAR            = 376634,
    SPELL_BALAKAR_UPHEAVAL              = 375943,
    SPELL_BALAKAR_RENDING_STRIKE        = 375937,
    SPELL_BALAKAR_SAVAGE_STRIKE         = 375929,
    SPELL_BALAKAR_STORMWINDS            = 376730,
    SPELL_BALAKAR_LIGHTNING             = 376737,
    SPELL_BALAKAR_STATIC_SPEAR          = 376864,
    SPELL_BALAKAR_CRACKLING_UPHEAVAL    = 376892,
    SPELL_BALAKAR_CONDUCTIVE_STRIKE     = 376827,
    SPELL_BALAKAR_THUNDER_STRIKE        = 376829,

    SPELL_STORMCASTER_STORM_BOLT        = 376725
};

enum NokhudOffensiveTeeraAndMaruukSpells
{
    SPELL_TEERA_GALE_ARROW              = 382670,
    SPELL_TEERA_REPEL                   = 386547,
    SPELL_TEERA_GUARDIAN_WIND           = 384808,
    SPELL_TEERA_QUICK_SHOT              = 386411,
    SPELL_TEERA_SPIRIT_LEAP             = 385412,

    SPELL_MARUUK_EARTHSPLITTER          = 385339,
    SPELL_MARUUK_FRIGHTFUL_ROAR         = 386063,
    SPELL_MARUUK_BRUTALIZE              = 382836
};

enum NokhudOffensiveRagingTempestSpells
{
    SPELL_TEMPEST_UNCONTROLLABLE_ENERGY = 382630,
    SPELL_TEMPEST_ELECTRICAL_STORM      = 384620,
    SPELL_TEMPEST_LIGHTNING_STRIKE      = 384186,
    SPELL_TEMPEST_ENERGY_SURGE          = 384686,
    SPELL_TEMPEST_WIND_BURST            = 384761
};

enum NokhudOffensiveGranythSpells
{
    SPELL_GRANYTH_ERUPTION              = 388283,
    SPELL_GRANYTH_ERUPTION_NORMAL       = 455678,
    SPELL_GRANYTH_SHARDS_OF_STONE       = 388817,
    SPELL_GRANYTH_TECTONIC_STOMP        = 385916,

    SPELL_SABOTEUR_DISMANTLE            = 386490
};
