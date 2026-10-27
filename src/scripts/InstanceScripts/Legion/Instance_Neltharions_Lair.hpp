/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const NeltharionsLairEncounterCount = 4;

enum NeltharionsLairData
{
    DATA_ROKMORA               = 0, // 1790
    DATA_ULAROGG_CRAGSHAPER    = 1, // 1791
    DATA_NARAXAS               = 2, // 1792
    DATA_DARGRUL_THE_UNDERKING = 3  // 1793
};

enum NeltharionsLairCreatures
{
    BOSS_ROKMORA              = 91003,

    BOSS_ULAROGG_CRAGSHAPER   = 91004,
    NPC_BELLOWING_IDOL_DEEPS  = 98081,
    NPC_BELLOWING_IDOL_STANCE = 100818,

    BOSS_NARAXAS              = 91005,
    NPC_WORMSPEAKER_DEVOUT    = 101075,

    BOSS_DARGRUL              = 91007,
    NPC_MOLTEN_CHARSKIN       = 101476
};

enum NeltharionsLairGameObjects
{
    // Mystic Barriers
    GO_ROKMORA_EXIT_BARRIER   = 248871,
    GO_ULAROGG_EXIT_BARRIER   = 248765,
    GO_ULAROGG_ROOM_BARRIER   = 248763,
    GO_NARAXAS_ROOM_BARRIER   = 248764
};

enum NeltharionsLairRokmoraSpells
{
    SPELL_ROKMORA_SHATTER            = 188114,
    SPELL_ROKMORA_RAZOR_SHARDS       = 188169,
    SPELL_ROKMORA_CRYSTALLINE_GROUND = 198024
};

enum NeltharionsLairUlaroggSpells
{
    SPELL_ULAROGG_SUNDER             = 198496,
    SPELL_ULAROGG_STRIKE_MOUNTAIN    = 198428,
    SPELL_ULAROGG_STANCE_FORM        = 198510,
    SPELL_ULAROGG_STANCE_DEBRIS      = 198719,

    SPELL_BELLOWING_IDOL_DEBRIS      = 198717
};

enum NeltharionsLairNaraxasSpells
{
    SPELL_NARAXAS_RANCID_MAW         = 205549,
    SPELL_NARAXAS_TOXIC_RETCH        = 210150,
    SPELL_NARAXAS_SPIKED_TONGUE      = 199176,
    SPELL_NARAXAS_SPIKED_TONGUE_PULL = 199178,
    SPELL_NARAXAS_PUTRID_SKIES       = 198963,
    SPELL_NARAXAS_FRENZY             = 199775,
    SPELL_NARAXAS_ANGRY_CROWD        = 217028,
    SPELL_NARAXAS_DEVOUR_FANATIC     = 216797,
    SPELL_NARAXAS_RAVENOUS           = 199246,

    SPELL_DEVOUT_FANATICS_SACRIFICE  = 209906
};

enum NeltharionsLairDargrulSpells
{
    SPELL_DARGRUL_LANDSLIDE          = 200700,
    SPELL_DARGRUL_MOLTEN_CRASH       = 200732,
    SPELL_DARGRUL_CRYSTAL_SPIKES     = 200551,
    SPELL_DARGRUL_MAGMA_WAVE         = 200404,
    SPELL_DARGRUL_MAGMA_BREAKER      = 216368,
    SPELL_DARGRUL_LAVA_GEYSER        = 216373,

    SPELL_CHARSKIN_BURNING_HATRED    = 200154,
    SPELL_CHARSKIN_MAGMA_BREAKER     = 209920,
    SPELL_CHARSKIN_RISING_INFERNO    = 216369
};
