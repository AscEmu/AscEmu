/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
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

    // BattlemasterList.dbc entry the client uses for every arena queue (2v2, 3v3 and 5v5)
    const uint32_t BATTLEMASTER_LIST_ALL_ARENAS = 6;

    // SMSG_GROUP_JOINED_BATTLEGROUND (Classic - WotLK)
    enum GroupJoinStatus : int32_t
    {
        GROUP_JOIN_STATUS_FAIL = 0,
        GROUP_JOIN_STATUS_NOT_ELIGIBLE = -1,                // Your group has joined a battleground queue, but you are not eligible
        GROUP_JOIN_STATUS_DESERTERS = -2,                   // You cannot join the battleground yet because you or one of your party members is flagged as a Deserter.
        GROUP_JOIN_STATUS_NOT_IN_TEAM = -3,                 // Incorrect party size for this arena.
        GROUP_JOIN_STATUS_TOO_MANY_QUEUES = -4,             // You can only be queued for 2 battles at once
        GROUP_JOIN_STATUS_CANNOT_QUEUE_FOR_RATED = -5,      // You cannot queue for a rated match while queued for other battles
        GROUP_JOIN_STATUS_QUEUED_FOR_RATED = -6,            // You cannot queue for another battle while queued for a rated arena match
        GROUP_JOIN_STATUS_TEAM_LEFT_QUEUE = -7,             // Your team has left the arena queue
        GROUP_JOIN_STATUS_NOT_IN_BATTLEGROUND = -8,         // You can't do that in a battleground.
        GROUP_JOIN_STATUS_XP_GAIN = -9,
        GROUP_JOIN_STATUS_JOIN_RANGE_INDEX = -10,           // Cannot join the queue unless all members of your party are in the same battleground level range.
        GROUP_JOIN_STATUS_JOIN_TIMED_OUT = -11,             // %s was unavailable to join the queue.
        GROUP_JOIN_STATUS_JOIN_FAILED = -12,
        GROUP_JOIN_STATUS_LFG_CANT_USE_BATTLEGROUND = -13,  // You cannot queue for a battleground or arena while using the dungeon system.
        GROUP_JOIN_STATUS_IN_RANDOM_BG = -14,               // Can't do that while in a Random Battleground queue.
        GROUP_JOIN_STATUS_IN_NON_RANDOM_BG = -15            // Can't queue for Random Battleground while in another Battleground queue.
    };

    // SMSG_BATTLEFIELD_STATUS_FAILED (Cata - Mop)
    enum JoinResult : uint32_t
    {
        JOIN_RESULT_NONE = 0,
        JOIN_RESULT_DESERTERS = 2,                          // You cannot join the battleground yet because you or one of your party members is flagged as a Deserter.
        JOIN_RESULT_ARENA_TEAM_PARTY_SIZE = 3,              // Incorrect party size for this arena.
        JOIN_RESULT_TOO_MANY_QUEUES = 4,                    // You can only be queued for 2 battles at once
        JOIN_RESULT_CANNOT_QUEUE_FOR_RATED = 5,             // You cannot queue for a rated match while queued for other battles
        JOIN_RESULT_QUEUED_FOR_RATED = 6,                   // You cannot queue for another battle while queued for a rated arena match
        JOIN_RESULT_TEAM_LEFT_QUEUE = 7,                    // Your team has left the arena queue
        JOIN_RESULT_NOT_IN_BATTLEGROUND = 8,                // You can't do that in a battleground.
        JOIN_RESULT_XP_GAIN = 9,
        JOIN_RESULT_RANGE_INDEX = 10,                       // Cannot join the queue unless all members of your party are in the same battleground level range.
        JOIN_RESULT_TIMED_OUT = 11,                         // %s was unavailable to join the queue.
        JOIN_RESULT_LFG_CANT_USE_BATTLEGROUND = 14,         // You cannot queue for a battleground or arena while using the dungeon system.
        JOIN_RESULT_IN_RANDOM_BG = 15,                      // Can't do that while in a Random Battleground queue.
        JOIN_RESULT_IN_NON_RANDOM_BG = 16,                  // Can't queue for Random Battleground while in another Battleground queue.
        JOIN_RESULT_BG_DEVELOPER_ONLY = 17,
        JOIN_RESULT_INVITATION_DECLINED = 18,
        JOIN_RESULT_MEETING_STONE_NOT_FOUND = 19,
        JOIN_RESULT_WARGAME_REQUEST_FAILURE = 20,
        JOIN_RESULT_BATTLEFIELD_TEAM_PARTY_SIZE = 22,
        JOIN_RESULT_NOT_ON_TOURNAMENT_REALM = 23,
        JOIN_RESULT_PLAYERS_FROM_DIFFERENT_REALMS = 24,
        JOIN_RESULT_REMOVE_FROM_PVP_QUEUE_GRANT_LEVEL = 33,
        JOIN_RESULT_REMOVE_FROM_PVP_QUEUE_FACTION_CHANGE = 34,
        JOIN_RESULT_JOIN_FAILED = 35,
        JOIN_RESULT_DUPE_QUEUE = 43
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
#elif defined(AE_FOREVER)
// Copied from MoP as a temporary baseline. Replace with dedicated Forever values once verified.
#define BATTLEGROUND_NUM_TYPES 758
#endif
