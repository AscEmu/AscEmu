/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const DeOtherSideEncounterCount = 4;

enum DeOtherSideData
{
    // Millhouse Manastorm and Millificent Manastorm
    DATA_THE_MANASTORMS     = 0,  // 2394
    DATA_HAKKAR_SOULFLAYER  = 1,  // 2395
    DATA_MUEHZALA           = 2,  // 2396
    DATA_DEALER_XYEXA       = 3   // 2400
};

enum DeOtherSideCreatures
{
    BOSS_MILLHOUSE_MANASTORM    = 164556,
    BOSS_MILLIFICENT_MANASTORM  = 164555,

    BOSS_HAKKAR_THE_SOULFLAYER  = 164558,
    NPC_SON_OF_HAKKAR           = 165905,

    BOSS_MUEHZALA               = 166608,
    NPC_SHATTERED_VISAGE        = 168326,
    // second Shattered Visage template of the encounter
    NPC_SHATTERED_VISAGE_ALT    = 171665,

    BOSS_DEALER_XYEXA           = 164450
};

enum DeOtherSideActions
{
    ACTION_MANASTORM_TAKE_FLOOR = 1
};

enum DeOtherSideManastormSpells
{
    SPELL_MILLHOUSE_FROSTBOLT                   = 320008,
    SPELL_MILLHOUSE_POWER_OVERWHELMING          = 320786,
    SPELL_MILLHOUSE_DIABOLICAL_DOOM             = 320142,
    SPELL_MILLHOUSE_SHADOWFURY                  = 320132,

    SPELL_MILLIFICENT_THROW_BUZZ_SAW            = 320168,
    SPELL_MILLIFICENT_ECHO_FINGER_LASER         = 324047,
    SPELL_MILLIFICENT_ERUPTION                  = 324010,
    SPELL_MILLIFICENT_BUZZ_SAW                  = 320145,
    SPELL_MILLIFICENT_SQUIRREL_BOMB             = 320811,
    SPELL_MILLIFICENT_ROCKET_CHICKEN_BARRAGE    = 321011
};

enum DeOtherSideHakkarSpells
{
    SPELL_HAKKAR_BLOOD_BARRIER      = 322773,
    SPELL_HAKKAR_BLOOD_BARRAGE      = 323064,
    SPELL_HAKKAR_CORRUPTED_BLOOD    = 322746,
    SPELL_HAKKAR_PIERCING_BARB      = 322736,

    SPELL_SON_OF_HAKKAR_BLOOD_NOVA  = 323544
};

enum DeOtherSideMuehzalaSpells
{
    SPELL_MUEHZALA_MASTER_OF_DEATH      = 325258,
    SPELL_MUEHZALA_COSMIC_ARTIFICE      = 325725,
    SPELL_MUEHZALA_SOULCRUSHER          = 327646,
    SPELL_MUEHZALA_CRUSH                = 334961,
    SPELL_MUEHZALA_STELLAR_CLOUD        = 335000,

    SPELL_VISAGE_SHATTERED_DOMINION     = 327427
};

enum DeOtherSideXyexaSpells
{
    SPELL_XYEXA_DISPLACEMENT_TRAP                   = 319619,
    SPELL_XYEXA_EXPLOSIVE_CONTRIVANCE               = 320232,
    SPELL_XYEXA_LOCALIZED_EXPLOSIVE_CONTRIVANCE     = 321948,
    SPELL_XYEXA_ARCANE_LIGHTNING                    = 323687
};
