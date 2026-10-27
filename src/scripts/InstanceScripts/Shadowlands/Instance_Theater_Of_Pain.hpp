/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const TheaterOfPainEncounterCount = 5;

enum TheaterOfPainData
{
    DATA_KULTHAROK              = 0,  // 2364
    DATA_GORECHOP               = 1,  // 2365
    DATA_XAV_THE_UNFALLEN       = 2,  // 2366
    // Dessia the Decapitator, Paceran the Virulent and Sathel the Accursed
    DATA_AFFRONT_OF_CHALLENGERS = 3,  // 2391
    DATA_MORDRETHA              = 4   // 2404
};

enum TheaterOfPainCreatures
{
    BOSS_KULTHAROK              = 162309,

    BOSS_GORECHOP               = 162317,
    NPC_OOZING_LEFTOVERS        = 165260,

    BOSS_XAV_THE_UNFALLEN       = 162329,

    BOSS_DESSIA_THE_DECAPITATOR = 164451,
    BOSS_PACERAN_THE_VIRULENT   = 164463,
    BOSS_SATHEL_THE_ACCURSED    = 164461,

    BOSS_MORDRETHA              = 165946
};

enum TheaterOfPainActions
{
    ACTION_THEATER_CHALLENGER_FALLEN = 1
};

enum TheaterKultharokSpells
{
    SPELL_KULTHAROK_DRAW_SOUL           = 473959,
    SPELL_KULTHAROK_FEAST_OF_THE_DAMNED = 473513,
    SPELL_KULTHAROK_DEATH_SPIRAL        = 1216474,
    SPELL_KULTHAROK_WELL_OF_DARKNESS    = 473540,
    SPELL_KULTHAROK_NECROTIC_ERUPTION   = 474087,
    SPELL_KULTHAROK_NECROTIC_BOLT       = 1216475
};

enum TheaterGorechopSpells
{
    SPELL_GORECHOP_MEAT_HOOKS           = 322795,
    SPELL_GORECHOP_TENDERIZING_SMASH    = 318406,
    SPELL_GORECHOP_HATEFUL_STRIKE       = 323515,

    SPELL_LEFTOVERS_OOZING              = 323542,
    SPELL_LEFTOVERS_LEAPING_THRASH      = 323107
};

enum TheaterXavSpells
{
    SPELL_XAV_BRUTAL_COMBO              = 320644,
    SPELL_XAV_MIGHT_OF_MALDRAXXUS       = 320050,
    SPELL_XAV_DEAFENING_CRASH           = 339415,
    SPELL_XAV_MASSIVE_CLEAVE            = 320729,
    SPELL_XAV_CRUSHING_SLAM             = 317231,
    SPELL_XAV_OPPRESSIVE_BANNER         = 331618
};

enum TheaterChallengerSpells
{
    SPELL_CHALLENGER_FINAL_WILL         = 1215747,

    SPELL_DESSIA_MIGHTY_SMASH           = 1215741,
    SPELL_DESSIA_MORTAL_STRIKE          = 320069,

    SPELL_PACERAN_NOXIOUS_SPORES        = 320180,
    SPELL_PACERAN_DECAYING_BREATH       = 1215738,

    SPELL_SATHEL_SEARING_DEATH          = 333292,
    SPELL_SATHEL_WITHERING_TOUCH        = 1215600,
    SPELL_SATHEL_NECROMANTIC_BOLT       = 320300
};

enum TheaterMordrethaSpells
{
    SPELL_MORDRETHA_REAPING_SCYTHE      = 324079,
    SPELL_MORDRETHA_DARK_DEVASTATION    = 323681,
    SPELL_MORDRETHA_MANIFEST_DEATH      = 324449,
    SPELL_MORDRETHA_GRASPING_RIFT       = 323685,
    SPELL_MORDRETHA_ECHOES_OF_CARNAGE   = 339573
};
