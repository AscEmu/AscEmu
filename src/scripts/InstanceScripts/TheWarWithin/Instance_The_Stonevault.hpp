/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const TheStonevaultEncounterCount = 4;

enum TheStonevaultData
{
    DATA_EDNA                   = 0,    // 2854
    DATA_SKARMORAK              = 1,    // 2880
    DATA_VOID_SPEAKER_EIRICH    = 2,    // 2883
    // Speaker Brokk and Speaker Dorlita
    DATA_MASTER_MACHINISTS      = 3     // 2888
};

enum TheStonevaultCreatures
{
    BOSS_EDNA                   = 210108,
    NPC_EDNA_VOLATILE_SPIKE     = 223237,

    BOSS_SKARMORAK              = 210156,
    NPC_SKARMORAK_CRYSTAL_SHARD = 214443,

    BOSS_VOID_SPEAKER_EIRICH    = 213119,

    BOSS_SPEAKER_BROKK          = 213217,
    BOSS_SPEAKER_DORLITA        = 213216
};

enum TheStonevaultGameObjects
{
    // E.D.N.A. hall: entrance shut during the fight, side gates open after it
    GO_EDNA_ENTRANCE_DOOR       = 440236,
    GO_EDNA_SOUTH_GATE          = 440235,
    GO_EDNA_NORTH_GATE          = 440239,

    // flames in front of the machinists' forge
    GO_MACHINISTS_FIRE_BARRIER  = 445512
};

enum TheStonevaultActions
{
    ACTION_SKARMORAK_SHARD_FEED     = 1,
    ACTION_SKARMORAK_SHARD_ERUPT,
    ACTION_MACHINISTS_PARTNER_FELL
};

enum EdnaSpells
{
    SPELL_EDNA_VOLATILE_SPIKE           = 424903,
    SPELL_EDNA_SPIKE_VOLATILE_EXPLOSION = 424913,
    SPELL_EDNA_REFRACTING_BEAM          = 424795,
    SPELL_EDNA_SEISMIC_SMASH            = 424888
};

enum SkarmorakSpells
{
    SPELL_SKARMORAK_CRYSTALLINE_SMASH   = 422233,
    SPELL_SKARMORAK_UNSTABLE_CRASH      = 423538,
    SPELL_SKARMORAK_UNSTABLE_FRAGMENTS  = 443405,
    SPELL_SKARMORAK_FORTIFIED_SHELL     = 445409,
    SPELL_SKARMORAK_VOID_DISCHARGE      = 423324,
    SPELL_SKARMORAK_SHATTERED_SHELL     = 423246,
    SPELL_SKARMORAK_SHARD_SHELL         = 426215,
    SPELL_SKARMORAK_SHARD_ERUPTION      = 443494
};

enum EirichSpells
{
    SPELL_EIRICH_VOID_RIFT              = 427315,
    SPELL_EIRICH_VOID_CORRUPTION        = 427329,
    SPELL_EIRICH_ENTROPIC_RECKONING     = 427854,
    SPELL_EIRICH_UNBRIDLED_VOID         = 427869
};

enum MachinistSpells
{
    SPELL_MACHINISTS_SILENCED_SPEAKER   = 439577,

    SPELL_BROKK_MOLTEN_METAL            = 428161,
    SPELL_BROKK_SCRAP_SONG              = 428202,
    SPELL_BROKK_SCRAP_CUBE              = 428547,
    SPELL_BROKK_EXHAUST_VENTS           = 443954,

    SPELL_DORLITA_LAVA_CANNON           = 428120,
    SPELL_DORLITA_LAVA_CANNON_HIT       = 449129,
    SPELL_DORLITA_IGNEOUS_HAMMER        = 428711,
    SPELL_DORLITA_BLAZING_CRESCENDO     = 428508,
    SPELL_DORLITA_CRESCENDO_BLAST       = 428535
};
