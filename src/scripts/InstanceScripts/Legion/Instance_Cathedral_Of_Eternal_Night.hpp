/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const CathedralEternalNightEncounterCount = 4;

enum CathedralEternalNightData
{
    DATA_MEPHISTROTH    = 0,  // 2039
    DATA_DOMATRAX       = 1,  // 2053
    DATA_AGRONOX        = 2,  // 2055
    DATA_THRASHBITE     = 3   // 2057
};

enum CathedralEternalNightCreatures
{
    BOSS_AGRONOX                = 117193,
    NPC_FULMINATING_LASHER      = 119169,
    NPC_CHOKING_VINES           = 120014,

    BOSS_THRASHBITE_THE_SCORNFUL = 117194,

    BOSS_DOMATRAX               = 118804,
    NPC_FEL_PORTAL_GUARDIAN     = 118834,
    NPC_DOMATRAX_IMP            = 118801,
    NPC_DOMATRAX_FELGUARD       = 119157,

    BOSS_MEPHISTROTH            = 116944,
    NPC_SHADOW_OF_MEPHISTROTH   = 117590
};

enum CathedralEternalNightGameObjects
{
    GO_CEN_AGRONOX_GARDEN_DOOR  = 269221,
    GO_CEN_THRASHBITE_ENTRANCE  = 268709,
    GO_CEN_THRASHBITE_EXIT      = 268710,
    GO_CEN_SANCTUM_DOOR         = 268680,

    // Thrashbite charges into these
    GO_CEN_BOOKCASE_1           = 268372,
    GO_CEN_BOOKCASE_2           = 268373,
    GO_CEN_BOOKCASE_3           = 268374,
    GO_CEN_BOOKCASE_4           = 268375
};

enum AgronoxSpells
{
    SPELL_AGRONOX_TIMBER_SMASH          = 235751,
    SPELL_AGRONOX_FULMINATING_LASHERS   = 236527,
    SPELL_AGRONOX_POISONOUS_SPORES      = 236524,
    SPELL_LASHER_FIXATE                 = 243613,
    SPELL_CHOKING_VINES_GRIP            = 238598
};

enum ThrashbiteSpells
{
    SPELL_THRASHBITE_PULVERIZING_CUDGEL = 237276,
    SPELL_THRASHBITE_HEAVE_CUDGEL       = 243124,
    SPELL_THRASHBITE_SCORNFUL_GAZE      = 237726,
    SPELL_THRASHBITE_SCORNFUL_CHARGE    = 238469,
    SPELL_THRASHBITE_WILD_THRASH        = 240547
};

enum DomatraxSpells
{
    SPELL_DOMATRAX_FELSOUL_CLEAVE       = 236543,
    SPELL_DOMATRAX_CHAOTIC_ENERGY_AURA  = 239339,
    SPELL_DOMATRAX_CHAOTIC_ENERGY       = 234107,
    SPELL_DOMATRAX_FRENZY               = 243157,
    SPELL_FEL_DISSOLVE_IN               = 211762
};

enum MephistrothSpells
{
    SPELL_MEPHISTROTH_DEMONIC_UPHEAVAL  = 233196,
    SPELL_MEPHISTROTH_DARK_SOLITUDE     = 234817,
    SPELL_MEPHISTROTH_CARRION_SWARM     = 233155,
    SPELL_MEPHISTROTH_SHADOW_FADE       = 233206,
    SPELL_MEPHISTROTH_CREEPING_SHADOWS  = 234369
};
