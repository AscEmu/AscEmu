/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

namespace BattlegroundDef
{
    const uint8_t MAX_LEVEL_GROUP = 9;
    const uint32_t LEVEL_GROUP_70 = 8;

    enum Sounds : uint32_t
    {
        BATTLEGROUND_BEGIN = 3439,
        FLAG_RESPAWN = 8232,
        HORDE_SCORES = 8213,
        ALLIANCE_SCORES = 8173,
        ALLIANCE_CAPTURE = 8174,
        HORDE_CAPTURE = 8212,
        FLAG_RETURNED = 8192,
        HORDEWINS = 8454,
        ALLIANCEWINS = 8455,
        HORDE_BGALMOSTEND = 8456,
        ALLIANCE_BGALMOSTEND = 8457
    };

    enum Spells : uint32_t
    {
        PREPARATION = 44521,
        REVIVE_PREPARATION = 44535,
        RESURRECT = 21074, // Spirit Healer Res
        DESERTER = 26013
    };

    enum Types : uint32_t
    {
        TYPE_ALTERAC_VALLEY = 1,
        TYPE_WARSONG_GULCH = 2,
        TYPE_ARATHI_BASIN = 3,
        TYPE_ARENA_2V2 = 4,
        TYPE_ARENA_3V3 = 5,
        TYPE_ARENA_5V5 = 6,
        TYPE_EYE_OF_THE_STORM = 7,
        // WOTLK
        TYPE_STRAND_OF_THE_ANCIENT = 9,
        TYPE_ISLE_OF_CONQUEST = 30,
        TYPE_RANDOM = 32,
        // Cataclysm
        TYPE_RATED_10 = 100,
        TYPE_RATED_15 = 101,
        TYPE_RATED_5 = 102,
        TYPE_TWIN_PEAKS = 108,
        TYPE_BATTLE_FOR_GILNEAS_CITY = 120,
        TYPE_ICECROWN_CITADEL = 441,
        TYPE_RUBY_SANCTUM = 443,
        TYPE_EYE_OF_THE_STORM_RATED = 656,
        // MOP
        TYPE_TEMPLE_OF_KOTMOGU = 699,
        TYPE_SILVERSHARD_MINES = 708,
        TYPE_TOL_VIRON_ARENA = 719,
        TYPE_DEEPWIND_GORGE = 754,
        TYPE_TIGERS_PEAK = 757,
    };

    enum Status
    {
        STATUS_NOFLAGS = 0, // wtfbbq, why aren't there any flags?
        STATUS_INQUEUE = 1, // Battleground has a queue, player is now in queue
        STATUS_READY = 2,   // Battleground is ready to join
        STATUS_TIME = 3     // Ex. Wintergrasp time remaining
    };

    //\todo: Zyres: move this to scripts
    enum ScoreDataIndex
    {
        AB_BASES_ASSAULTED = 0,
        AB_BASES_CAPTURED = 1,
        AV_GRAVEYARDS_ASSAULTED = 0,
        AV_GRAVEYARDS_DEFENDED = 1,
        AV_TOWERS_ASSAULTED = 2,
        AV_TOWERS_DEFENDED = 3,
        AV_MINES_CAPTURES = 4,
        EOTS_FLAGS_CAPTURED = 0,
        WSG_FLAGS_CAPTURED = 0,
        WSG_FLAGS_RETURNED = 1,
        IOC_BASES_ASSAULTED = 0,
        IOC_BASES_DEFENDED = 1
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
//based on BattlemasterList.dbc (BATTLEGROUND_NUM_TYPES = Max entry's +1)
#if VERSION_STRING == Classic
#define BATTLEGROUND_NUM_TYPES 8
#elif VERSION_STRING == TBC
#define BATTLEGROUND_NUM_TYPES 8
#elif VERSION_STRING == WotLK
#define BATTLEGROUND_NUM_TYPES 33
#elif VERSION_STRING == Cata
#define BATTLEGROUND_NUM_TYPES 657
#elif VERSION_STRING == Mop
#define BATTLEGROUND_NUM_TYPES 758
#endif
