/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Blackrock Caverns - five bosses, no gating doors. Identities and mechanics verified against
// wowhead.

uint32_t const BlackrockCavernsEncounterCount = 5;

enum BlackrockCavernsData
{
    DATA_ROMOGG_BONECRUSHER       = 0,
    DATA_CORLA_HERALD_OF_TWILIGHT = 1,
    DATA_KARSH_STEELBENDER        = 2,
    DATA_BEAUTY                   = 3,
    DATA_ASCENDANT_LORD_OBSIDIUS  = 4
};

enum BlackrockCavernsCreatures
{
    BOSS_ROMOGG_BONECRUSHER       = 39665,
    BOSS_CORLA_HERALD_OF_TWILIGHT = 39679,
    BOSS_KARSH_STEELBENDER        = 39698,
    BOSS_BEAUTY                   = 39700,
    BOSS_ASCENDANT_LORD_OBSIDIUS  = 39705,

    NPC_TWILIGHT_FLAME_CALLER     = 39708,
    NPC_TWILIGHT_TORTURER         = 39978,
    NPC_TWILIGHT_SADIST           = 39980,
    NPC_MAD_PRISONER              = 39985,
    NPC_CRAZED_MAGE               = 39982,

    NPC_CONFLAGRATION             = 39994,
    NPC_TWILIGHT_ELEMENT_WARDEN   = 40017
};

enum TrashSpells
{
    SPELL_FLAME_CALLER_BLAST_WAVE    = 76473,
    SPELL_FLAME_CALLER_CALL_FLAMES   = 76325,

    SPELL_CONFLAGRATION_BURNING_HEAT = 82301,
    SPELL_ELEMENT_WARDEN_BORE        = 75205,

    SPELL_TORTURER_RED_HOT_POKER     = 76478,
    SPELL_TORTURER_WILD_BEATDOWN     = 76487,

    SPELL_SADIST_HEAT_SEEKER_BLADE   = 76502,
    SPELL_SADIST_SINISTER_STRIKE     = 76500,

    SPELL_PRISONER_HEAD_CRACK        = 77568,
    SPELL_PRISONER_INFECTED_WOUND    = 76512
};

enum RomoggSpells
{
    SPELL_ROMOGG_CHAINS_OF_WOE   = 75539,
    SPELL_ROMOGG_QUAKE           = 75272,
    SPELL_ROMOGG_WOUNDING_STRIKE = 75571
};

enum CorlaSpells
{
    SPELL_CORLA_EVOLUTION       = 75610,
    SPELL_CORLA_DARK_COMMAND_10 = 75823,
    SPELL_CORLA_DARK_COMMAND_25 = 93462
};

enum KarshSpells
{
    SPELL_KARSH_CLEAVE    = 15284,
    SPELL_KARSH_HEAT_WAVE = 75851
};

enum BeautySpells
{
    SPELL_BEAUTY_MAGMA_SPIT       = 76031,
    SPELL_BEAUTY_BERSERKER_CHARGE = 76030,
    SPELL_BEAUTY_FLAMEBREAK       = 76032,
    SPELL_BEAUTY_TERRIFYING_ROAR  = 76028
};

enum ObsidiusSpells
{
    SPELL_OBSIDIUS_STONE_BLOW          = 76185,
    SPELL_OBSIDIUS_TWILIGHT_CORRUPTION = 75054,
    SPELL_OBSIDIUS_THUNDERCLAP         = 76186
};
