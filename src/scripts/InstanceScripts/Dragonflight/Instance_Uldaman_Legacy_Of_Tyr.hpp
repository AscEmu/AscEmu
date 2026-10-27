/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const UldamanLegacyOfTyrEncounterCount = 5;

enum UldamanLegacyOfTyrData
{
    // Baelog, Eric "The Swift" and Olaf
    DATA_THE_LOST_DWARVES   = 0, // 2555
    DATA_BROMACH            = 1, // 2556
    DATA_SENTINEL_TALONDRAS = 2, // 2557
    DATA_EMBERON            = 3, // 2558
    DATA_CHRONO_LORD_DEIOS  = 4  // 2559
};

enum UldamanLegacyOfTyrCreatures
{
    BOSS_BAELOG              = 184581,
    BOSS_ERIC_THE_SWIFT      = 184582,
    BOSS_OLAF                = 184580,
    NPC_LONGBOAT_RAID        = 191459,

    BOSS_BROMACH             = 184018,
    NPC_QUAKING_TOTEM        = 186696,
    NPC_STONEVAULT_GEOMANCER = 186658,
    NPC_STONEVAULT_AMBUSHER  = 186664,

    BOSS_SENTINEL_TALONDRAS  = 184124,

    BOSS_EMBERON             = 184422,
    NPC_VAULT_KEEPER         = 186107,

    BOSS_CHRONO_LORD_DEIOS   = 184125
};

enum UldamanLegacyOfTyrGameObjects
{
    // hall behind Bromach's cave
    GO_PREHISTORIA_VAULT_DOOR  = 376557,
    // Talondras' hall, south entrance and east way on
    GO_TALONDRAS_ENTRANCE_DOOR = 377366,
    GO_TALONDRAS_EXIT_DOOR     = 376554,
    // Emberon's vault, north entrance and east way on
    GO_EMBERON_ENTRANCE_DOOR   = 376191,
    GO_EMBERON_EXIT_DOOR       = 376640,
    // chamber of Deios
    GO_DEIOS_ENTRANCE_DOOR     = 376637
};

enum UldamanLegacyOfTyrActions
{
    ACTION_LONGBOAT_RAID = 1,
    ACTION_VAULT_KEEPER_ACTIVATE,
    ACTION_VAULT_KEEPER_DEACTIVATE
};

enum UldamanLegacyLostDwarvesSpells
{
    SPELL_BAELOG_HEAVY_ARROW            = 369573,
    SPELL_BAELOG_HEAVY_ARROW_NORMAL     = 455242,
    SPELL_BAELOG_WILD_CLEAVE            = 369563,

    SPELL_ERIC_SKULLCRACKER             = 369791,
    SPELL_ERIC_DAGGER_THROW             = 369781,

    SPELL_OLAF_RICOCHETING_SHIELD       = 369677,
    SPELL_OLAF_DEFENSIVE_BULWARK        = 369603,

    SPELL_DWARF_RECOVERING              = 375339,

    SPELL_LONGBOAT_SEARING_CANNONFIRE   = 375286,
    SPELL_LONGBOAT_BURNING_PITCH        = 377825,
    SPELL_LONGBOAT_FIERY_SURGE          = 395647,
    SPELL_LONGBOAT_HASTY_REARMING       = 375398
};

enum UldamanLegacyBromachSpells
{
    SPELL_BROMACH_CALL_OF_THE_DEEP      = 369605,
    SPELL_BROMACH_QUAKING_TOTEM         = 369615,
    SPELL_BROMACH_BLOODLUST             = 369754,
    SPELL_BROMACH_THUNDERING_SLAM       = 369703,

    SPELL_QUAKING_TOTEM_SHOCKING_QUAKE  = 369610,
    SPELL_QUAKING_TOTEM_TREMOR          = 369660,

    SPELL_AMBUSHER_AMBUSH               = 369697,
    SPELL_GEOMANCER_CHAIN_LIGHTNING     = 369675,
    SPELL_GEOMANCER_STONE_SPIKE         = 369674
};

enum UldamanLegacyTalondrasSpells
{
    SPELL_TALONDRAS_INEXORABLE          = 372600,
    SPELL_TALONDRAS_ANCIENT_DYNAMO      = 372716,
    SPELL_TALONDRAS_TITANIC_EMPOWERMENT = 372719,
    SPELL_TALONDRAS_RESONATING_ORB      = 372623,
    SPELL_TALONDRAS_CRUSHING_STOMP      = 372701,
    SPELL_TALONDRAS_EARTHEN_SHARDS      = 372718
};

enum UldamanLegacyEmberonSpells
{
    SPELL_EMBERON_PURGING_FLAMES        = 368990,
    SPELL_EMBERON_PURGING_FLAMES_NORMAL = 455334,
    SPELL_EMBERON_SACRED_BARRIER        = 369031,
    SPELL_EMBERON_ACTIVATE_KEEPERS      = 369033,
    SPELL_EMBERON_UNSTABLE_EMBERS       = 369110,
    SPELL_EMBERON_SEARING_CLAP          = 369061,

    SPELL_VAULT_KEEPER_SEEKING_FLAME    = 369052
};

enum UldamanLegacyChronoLordDeiosSpells
{
    SPELL_DEIOS_ETERNITY_ORB            = 376319,
    SPELL_DEIOS_REWIND_TIMEFLOW         = 376209,
    SPELL_DEIOS_REWIND_TIMEFLOW_NORMAL  = 455346,
    SPELL_DEIOS_WING_BUFFET             = 376049,
    SPELL_DEIOS_TIME_SINK               = 377405,
    SPELL_DEIOS_SAND_BREATH             = 375727
};
