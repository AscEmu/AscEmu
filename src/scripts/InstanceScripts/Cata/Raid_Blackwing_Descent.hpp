/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Blackwing Descent - six bosses (verified against wowhead).

uint32_t const BlackwingDescentEncounterCount = 6;

enum BlackwingDescentData
{
    DATA_MAGMAW                  = 0,
    DATA_OMNOTRON_DEFENSE_SYSTEM = 1,
    DATA_CHIMAERON               = 2,
    DATA_ATRAMEDES               = 3,
    DATA_MALORIAK                = 4,
    DATA_NEFARIANS_END           = 5
};

enum BlackwingDescentGameObjects
{
    GO_ATHENAEUM_DOOR     = 208291,
    GO_INNER_CHAMBER_DOOR = 205830
};

enum BlackwingDescentCreatures
{
    BOSS_MAGMAW      = 41570,

    NPC_ELECTRON     = 42179,
    NPC_MAGMATRON    = 42178,
    NPC_TOXITRON     = 42180,
    NPC_ARCANOTRON   = 42166,

    BOSS_CHIMAERON   = 43296,
    BOSS_ATRAMEDES   = 41442,
    BOSS_MALORIAK    = 41378,

    BOSS_NEFARIAN    = 41376,
    NPC_ONYXIA       = 41270,

    NPC_GOLEM_SENTRY = 42800
};

enum GolemSentrySpells
{
    SPELL_GOLEM_SENTRY_1 = 81063,
    SPELL_GOLEM_SENTRY_2 = 81055
};

enum MagmawSpells
{
    SPELL_MAGMAW_LAVA_SPEW       = 77839,
    SPELL_MAGMAW_MANGLE          = 92047,
    SPELL_MAGMAW_PILLAR_OF_FLAME = 77998
};

enum OmnotronSpells
{
    SPELL_ELECTRON_STATIC_SHOCK      = 79912,
    SPELL_MAGMATRON_INCINERATION     = 79023,
    SPELL_ARCANOTRON_POWER_GENERATOR = 79624,
    SPELL_TOXITRON_CHEMICAL_BOMB     = 80157
};

enum ChimaeronSpells
{
    SPELL_CHIMAERON_CAUSTIC_SLIME = 82871,
    SPELL_CHIMAERON_BREAK         = 82881,
    SPELL_CHIMAERON_MASSACRE      = 82848
};

enum AtramedesSpells
{
    SPELL_ATRAMEDES_ROARING_BREATH = 81573,
    SPELL_ATRAMEDES_SEARING_FLAME  = 77840,
    SPELL_ATRAMEDES_SONIC_BREATH   = 78075
};

enum MaloriakSpells
{
    SPELL_MALORIAK_ARCANE_STORM       = 77896,
    SPELL_MALORIAK_THROW_RED_BOTTLE   = 77925,
    SPELL_MALORIAK_THROW_BLUE_BOTTLE  = 77932,
    SPELL_MALORIAK_THROW_GREEN_BOTTLE = 77937
};

enum NefarianEndSpells
{
    SPELL_NEFARIAN_SHADOWFLAME_BARRAGE = 78621,
    SPELL_NEFARIAN_ELECTRICAL_CHARGE   = 95793,
    SPELL_NEFARIAN_BERSERK             = 26662,

    SPELL_ONYXIA_ELECTRICAL_CHARGE     = 78949,
    SPELL_ONYXIA_TAIL_LASH             = 77827,
    SPELL_ONYXIA_SHADOWFLAME_BREATH    = 77826
};
