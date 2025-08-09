/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

namespace InstanceDifficulty
{
    enum Difficulties : uint8_t
    {
        DUNGEON_NORMAL          = 0,
        DUNGEON_HEROIC          = 1,

        RAID_10MAN_NORMAL       = 0,
        RAID_25MAN_NORMAL       = 1,
        RAID_10MAN_HEROIC       = 2,
        RAID_25MAN_HEROIC       = 3,

        MAX_DUNGEON_DIFFICULTY  = 2,
#if VERSION_STRING > TBC
        MAX_RAID_DIFFICULTY     = 4,
        MAX_DIFFICULTY          = 4
#else
        MAX_RAID_DIFFICULTY     = 2,
        MAX_DIFFICULTY          = 2
#endif
    };
}

enum InstanceType : uint8_t
{
    INSTANCE_NULL,                              // open world
    INSTANCE_RAID,                              // all raids
    INSTANCE_DUNGEON,                           // 5 man dungeons only with normal mode
    INSTANCE_BATTLEGROUND,                      // battlegrounds
    INSTANCE_MULTIMODE,                         // 5 man dungeons with heroic mode
};

enum EnterState
{
    CAN_ENTER                                   = 0,
    CANNOT_ENTER_ALREADY_IN_MAP                 = 1,
    CANNOT_ENTER_NO_ENTRY                       = 2,
    CANNOT_ENTER_UNINSTANCED_DUNGEON            = 3,
    CANNOT_ENTER_DIFFICULTY_UNAVAILABLE         = 4,
    CANNOT_ENTER_NOT_IN_RAID                    = 5,
    CANNOT_ENTER_CORPSE_IN_DIFFERENT_INSTANCE   = 6,
    CANNOT_ENTER_INSTANCE_BIND_MISMATCH         = 7,
    CANNOT_ENTER_TOO_MANY_INSTANCES             = 8,
    CANNOT_ENTER_MAX_PLAYERS                    = 9,
    CANNOT_ENTER_ENCOUNTER                      = 10,
    CANNOT_ENTER_UNSPECIFIED_REASON             = 11,
    CANNOT_ENTER_XPACK01                        = 12,
    CANNOT_ENTER_XPACK02                        = 13,
    CANNOT_ENTER_MIN_LEVEL                      = 14,
    CANNOT_ENTER_MIN_LEVEL_HC                   = 15,
    CANNOT_ENTER_ATTUNE_QA                      = 16,
    CANNOT_ENTER_ATTUNE_QH                      = 17,
    CANNOT_ENTER_ATTUNE_ITEM                    = 18,
    CANNOT_ENTER_KEY                            = 19,
};

enum INSTANCE_ABORT_ERROR
{
    INSTANCE_OK                                 = 0x00,
    INSTANCE_ABORT_ERROR                        = 0x01,
    INSTANCE_ABORT_FULL                         = 0x02,
    INSTANCE_ABORT_NOT_FOUND                    = 0x03,
    INSTANCE_ABORT_TOO_MANY                     = 0x04,
    INSTANCE_ABORT_ENCOUNTER                    = 0x06,
    INSTANCE_ABORT_EXPANSION                    = 0x07,
    INSTANCE_ABORT_HEROIC_MODE_NOT_AVAILABLE    = 0x08,
    INSTANCE_ABORT_UNIQUE_MESSAGE               = 0x09,
    INSTANCE_ABORT_TOO_MANY_REALM_INSTANCES     = 0x0A,
    INSTANCE_ABORT_NOT_IN_RAID_GROUP            = 0x0B,
    INSTANCE_ABORT_REALM_ONLY                   = 0x0F,
    INSTANCE_ABORT_MAP_NOT_ALLOWED              = 0x10
};

enum INSTANCE_RESET_ERROR
{
    INSTANCE_RESET_ERROR_PLAYERS_INSIDE         = 0x00,
    INSTANCE_RESET_ERROR_MEMBERS_OFFLINE        = 0x01,
    INSTANCE_RESET_ERROR_PLAYERS_ENTERING       = 0x02
};

enum InstanceResetMethod
{
    INSTANCE_RESET_ALL,
    INSTANCE_RESET_CHANGE_DIFFICULTY,
    INSTANCE_RESET_GLOBAL,
    INSTANCE_RESET_GROUP_DISBAND,
    INSTANCE_RESET_GROUP_JOIN,
    INSTANCE_RESET_RESPAWN_DELAY
};

enum InstanceResetWarningType
{
    RAID_INSTANCE_WARNING_HOURS                 = 1,                    // WARNING! %s is scheduled to reset in %d hour(s).
    RAID_INSTANCE_WARNING_MIN                   = 2,                    // WARNING! %s is scheduled to reset in %d minute(s)!
    RAID_INSTANCE_WARNING_MIN_SOON              = 3,                    // WARNING! %s is scheduled to reset in %d minute(s). Please exit the zone or you will be returned to your bind location!
    RAID_INSTANCE_WELCOME                       = 4,                    // Welcome to %s. This raid instance is scheduled to reset in %s.
    RAID_INSTANCE_EXPIRED                       = 5
};

enum BindExtensionState
{
    EXTEND_STATE_EXPIRED = 0,
    EXTEND_STATE_NORMAL = 1,
    EXTEND_STATE_EXTENDED = 2,
    EXTEND_STATE_KEEP = 255
};

const uint32_t defaultUpdateFrequency = 1000;
