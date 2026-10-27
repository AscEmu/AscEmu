/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const MawOfSoulsEncounterCount = 3;

enum MawOfSoulsData
{
    DATA_YMIRON     = 0,  // 1822
    DATA_HARBARON   = 1,  // 1823
    DATA_HELYA      = 2   // 1824
};

enum MawOfSoulsCreatures
{
    BOSS_YMIRON                     = 96756,

    BOSS_HARBARON                   = 96754,
    NPC_SHACKLED_SERVITOR           = 98693,
    NPC_SOUL_FRAGMENT               = 98761,
    NPC_COSMIC_SCYTHE               = 100839,

    // guards the way down to the Naglfar
    NPC_SKJAL                       = 99307,

    BOSS_HELYA                      = 96759,
    NPC_DESTRUCTOR_TENTACLE         = 99801,
    NPC_GRASPING_TENTACLE_LEFT_1    = 100362,
    NPC_GRASPING_TENTACLE_LEFT_2    = 100360,
    NPC_GRASPING_TENTACLE_RIGHT_1   = 98363,
    NPC_GRASPING_TENTACLE_RIGHT_2   = 100354,
    NPC_PIERCING_TENTACLE           = 100188
};

enum MawOfSoulsGameObjects
{
    GO_HARBARON_WALL_OF_SOULS       = 246947,
    GO_HARBARON_BARNACLED_DOOR      = 246888,

    // opened once Skjal is gone
    GO_SKJAL_WALL_OF_SOULS          = 246435,
    GO_SKJAL_MURKY_FOG_1            = 265477,
    GO_SKJAL_MURKY_FOG_2            = 265476
};

enum MawOfSoulsYmironSpells
{
    SPELL_YMIRON_DARK_SLASH         = 193211,
    SPELL_YMIRON_SCREAMS_OF_THE_DEAD = 193364,
    SPELL_YMIRON_WINDS_OF_NORTHREND = 193977,
    SPELL_YMIRON_BANE               = 193460
};

enum MawOfSoulsHarbaronSpells
{
    SPELL_HARBARON_COSMIC_SCYTHE    = 205330,
    SPELL_HARBARON_FRAGMENT         = 194325,
    SPELL_HARBARON_NETHER_RIP       = 194668,

    SPELL_SCYTHE_GLOW               = 198580,
    SPELL_SCYTHE_STRIKE             = 194218,

    SPELL_SERVITOR_SHACKLED         = 194259,
    SPELL_SERVITOR_VOID_SNAP        = 194266
};

enum MawOfSoulsHelyaSpells
{
    SPELL_HELYA_TAINT_OF_THE_SEA    = 197517,
    SPELL_HELYA_TORRENT             = 197805,
    SPELL_HELYA_BRACKWATER_BARRAGE  = 202088,
    SPELL_HELYA_SUBMERGED           = 196947,
    SPELL_HELYA_TRUE_FORM           = 197734,
    SPELL_HELYA_SMASH               = 197689,
    SPELL_HELYA_WING_BUFFET         = 198520,
    SPELL_HELYA_DECK_TORRENT        = 198495,
    SPELL_HELYA_CORRUPTED_BELLOW    = 227233,

    SPELL_DESTRUCTOR_RAPID_RUPTURE  = 185539,
    SPELL_DESTRUCTOR_SMASH          = 196534,
    SPELL_PIERCING_TENTACLE         = 197112
};
