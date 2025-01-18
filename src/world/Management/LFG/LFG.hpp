/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <set>
#include <map>

enum LfgRoles
{
    ROLE_NONE       = 0x00,
    ROLE_LEADER     = 0x01,
    ROLE_TANK       = 0x02,
    ROLE_HEALER     = 0x04,
    ROLE_DAMAGE     = 0x08
};

enum LfgUpdateType
{
    LFG_UPDATETYPE_DEFAULT                       = 0,      // Internal Use
    LFG_UPDATETYPE_LEADER                        = 1,
    LFG_UPDATETYPE_ROLECHECK_ABORTED             = 4,
    LFG_UPDATETYPE_JOIN_PROPOSAL                 = 5,
    LFG_UPDATETYPE_ROLECHECK_FAILED              = 6,
    LFG_UPDATETYPE_REMOVED_FROM_QUEUE            = 7,
    LFG_UPDATETYPE_PROPOSAL_FAILED               = 8,
    LFG_UPDATETYPE_PROPOSAL_DECLINED             = 9,
    LFG_UPDATETYPE_GROUP_FOUND                   = 10,
    LFG_UPDATETYPE_ADDED_TO_QUEUE                = 12,
    LFG_UPDATETYPE_PROPOSAL_BEGIN                = 13,
    LFG_UPDATETYPE_CLEAR_LOCK_LIST               = 14,
    LFG_UPDATETYPE_GROUP_MEMBER_OFFLINE          = 15,
    LFG_UPDATETYPE_GROUP_DISBAND                 = 16
};

enum LfgState
{
    LFG_STATE_NONE,                 // Not using LFG / LFR
    LFG_STATE_ROLECHECK,            // Rolecheck active
    LFG_STATE_QUEUED,               // Queued
    LFG_STATE_PROPOSAL,             // Proposal active
    LFG_STATE_BOOT,                 // Vote kick active
    LFG_STATE_DUNGEON,              // In LFG Group, in a Dungeon
    LFG_STATE_FINISHED_DUNGEON,     // In LFG Group, in a finished Dungeon
    LFG_STATE_RAIDBROWSER           // Using Raid finder
};

/// Instance lock types
enum LfgLockStatusType
{
    LFG_LOCKSTATUS_OK                            = 0,      // Internal use only
    LFG_LOCKSTATUS_INSUFFICIENT_EXPANSION        = 1,
    LFG_LOCKSTATUS_TOO_LOW_LEVEL                 = 2,
    LFG_LOCKSTATUS_TOO_HIGH_LEVEL                = 3,
    LFG_LOCKSTATUS_TOO_LOW_GEAR_SCORE            = 4,
    LFG_LOCKSTATUS_TOO_HIGH_GEAR_SCORE           = 5,
    LFG_LOCKSTATUS_RAID_LOCKED                   = 6,
    LFG_LOCKSTATUS_ATTUNEMENT_TOO_LOW_LEVEL      = 1001,
    LFG_LOCKSTATUS_ATTUNEMENT_TOO_HIGH_LEVEL     = 1002,
    LFG_LOCKSTATUS_QUEST_NOT_COMPLETED           = 1022,
    LFG_LOCKSTATUS_MISSING_ITEM                  = 1025,
    LFG_LOCKSTATUS_NOT_IN_SEASON                 = 1031
};

/// Dungeon and reason why player can't join
struct LfgLockStatus
{
    uint32_t dungeon;                     ///< Dungeon Id
    LfgLockStatusType lockstatus;         ///< Lock type
};

typedef std::set<uint32_t> LfgDungeonSet;
typedef std::map<uint32_t, LfgLockStatusType> LfgLockMap;
typedef std::map<uint64_t, LfgLockMap> LfgLockPartyMap;
