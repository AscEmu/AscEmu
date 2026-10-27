/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const AlgetharAcademyEncounterCount = 4;

enum AlgetharAcademyData
{
    DATA_VEXAMUS           = 0, // 2562
    DATA_OVERGROWN_ANCIENT = 1, // 2563
    DATA_CRAWTH            = 2, // 2564
    DATA_ECHO_OF_DORAGOSA  = 3  // 2565
};

enum AlgetharAcademyCreatures
{
    BOSS_VEXAMUS           = 194181,
    // tells the story of Vexamus in the library
    NPC_PROFESSOR_MAXDORMU = 189668,

    BOSS_OVERGROWN_ANCIENT = 196482,
    NPC_ANCIENT_BRANCH     = 196548,
    NPC_HUNGRY_LASHER      = 197398,

    BOSS_CRAWTH            = 191736,

    BOSS_ECHO_OF_DORAGOSA  = 190609,
    NPC_ARCANE_RIFT        = 196664
};

enum AlgetharAcademyActions
{
    ACTION_LASHER_DORMANT = 1,
    ACTION_LASHER_AWAKEN
};

enum AlgetharAcademyVexamusSpells
{
    SPELL_VEXAMUS_ARCANE_FISSURE          = 388537,
    SPELL_VEXAMUS_ARCANE_FISSURE_NORMAL   = 454314,
    SPELL_VEXAMUS_MANA_BOMBS              = 386173,
    SPELL_VEXAMUS_ARCANE_EXPULSION        = 385958
};

enum AlgetharAcademyOvergrownAncientSpells
{
    SPELL_ANCIENT_GERMINATE               = 388796,
    SPELL_ANCIENT_BURST_FORTH             = 388923,
    SPELL_ANCIENT_BRANCH_OUT              = 388623,
    SPELL_ANCIENT_BARKBREAKER             = 388544,

    SPELL_BRANCH_HEALING_TOUCH            = 396640
};

enum AlgetharAcademyCrawthSpells
{
    SPELL_CRAWTH_PLAY_BALL                = 377182,
    SPELL_CRAWTH_RUINOUS_WINDS            = 1276752,
    SPELL_CRAWTH_OVERPOWERING_GUST        = 377034,
    SPELL_CRAWTH_DEAFENING_SCREECH        = 377004,
    SPELL_CRAWTH_DEAFENING_SCREECH_NORMAL = 454341,
    SPELL_CRAWTH_SAVAGE_PECK              = 376997
};

enum AlgetharAcademyEchoOfDoragosaSpells
{
    SPELL_DORAGOSA_OVERWHELMING_POWER     = 389011,
    SPELL_DORAGOSA_ASTRAL_BLAST           = 1282251,
    SPELL_DORAGOSA_POWER_VACUUM           = 388822,
    SPELL_DORAGOSA_POWER_VACUUM_NORMAL    = 454362,
    SPELL_DORAGOSA_ARCANE_MISSILES        = 373326,
    SPELL_DORAGOSA_ARCANE_MISSILES_NORMAL = 454365,
    SPELL_DORAGOSA_ENERGY_BOMB            = 374352,
    SPELL_DORAGOSA_ENERGY_BOMB_NORMAL     = 454366,

    SPELL_ARCANE_RIFT_UNCONTROLLED_ENERGY = 388951
};
