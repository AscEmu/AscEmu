/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

uint32_t const DenOfNalorakkEncounterCount = 3;

enum DenOfNalorakkData
{
    DATA_HOARDMONGER            = 0,    // 3207
    DATA_SENTINEL_OF_WINTER     = 1,    // 3208
    DATA_LOA_NALORAKK           = 2     // 3209
};

enum DenOfNalorakkCreatures
{
    BOSS_HOARDMONGER                = 241812,
    // second template of the same boss
    BOSS_HOARDMONGER_ALT            = 261050,
    NPC_HOARD_BONE_PILE             = 244036,
    NPC_HOARD_MEAT_PILE             = 244041,
    NPC_HOARD_MUSHROOM_PILE         = 244061,
    NPC_HOARD_ROTTEN_MUSHROOM       = 244584,

    BOSS_SENTINEL_OF_WINTER         = 244100,
    BOSS_SENTINEL_OF_WINTER_ALT     = 261053,
    NPC_FRACTURED_SHIVERCORE        = 244759,
    NPC_RAGING_SQUALL               = 244696,
    NPC_RAGING_SQUALL_ALT           = 250992,

    BOSS_LOA_NALORAKK               = 246404,
    NPC_LOA_ZULJARRA                = 246409,
    NPC_ECHO_OF_NALORAKK            = 247301,
    NPC_ECHO_OF_NALORAKK_ALT        = 247521,
    NPC_ECHO_OF_NALORAKK_THIRD      = 261451
};

enum DenOfNalorakkGameObjects
{
    // behind the Hoardmonger's den
    GO_HOARD_WATER_DOOR     = 531150,
    // entrance of the Loa's arena
    GO_LOA_VINE_DOOR        = 581781
};

enum DenOfNalorakkActions
{
    ACTION_ECHO_JOINS_MAUL = 1
};

enum HoardmongerSpells
{
    SPELL_HOARD_RESOURCEFUL_MEASURES    = 1235072,
    SPELL_HOARD_OVERFLOWING_SUPPLIES    = 1235105,
    SPELL_HOARD_BONESPIKE_SLAM          = 1235129,
    SPELL_HOARD_HEARTY_BELLOW           = 1235125,
    SPELL_HOARD_SPOILED_SUPPLIES        = 1234233,
    SPELL_HOARD_EARTHSHATTER_SLAM       = 1234021,
    SPELL_HOARD_RAVENOUS_BELLOW         = 1234681,

    SPELL_MUSHROOM_TOXIC_SPORES         = 1234846,
    SPELL_MUSHROOM_PUTRID_BURST         = 1245593
};

enum SentinelOfWinterSpells
{
    SPELL_SENTINEL_FROZEN_TEMPEST       = 1235656,
    SPELL_SENTINEL_SHATTERING_FROSTSPIKE = 1235783,
    SPELL_SENTINEL_RAGING_SQUALL        = 1235623,
    SPELL_SENTINEL_GLACIAL_TORMENT      = 1235548,

    SPELL_SHIVERCORE_WINTERS_SHROUD     = 1235829,
    SPELL_SHIVERCORE_SNOWDRIFT          = 1234314
};

enum LoaNalorakkSpells
{
    SPELL_LOA_FURY_OF_THE_WAR_GOD       = 1243002,
    SPELL_LOA_CONCUSSIVE_SHOCK          = 1243063,
    SPELL_LOA_DEMORALIZING_SCREAM       = 1262253,
    SPELL_LOA_ECHOING_MAUL              = 1242860,
    SPELL_LOA_FORCEFUL_ROAR             = 1255385,

    SPELL_ECHO_ECHOING_FURY             = 1243408,
    SPELL_ECHO_SPECTRAL_SLASH           = 1255577
};
