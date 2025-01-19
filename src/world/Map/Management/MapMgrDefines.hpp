/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum MapMgrTimers
{
    MMUPDATE_OBJECTS        = 0,
    MMUPDATE_SESSIONS       = 1,
    MMUPDATE_FIELDS         = 2,
    MMUPDATE_IDLE_OBJECTS   = 3,
    MMUPDATE_ACTIVE_OBJECTS = 4,
    MMUPDATE_COUNT          = 5
};

enum ObjectActiveState
{
    OBJECT_STATE_NONE       = 0,
    OBJECT_STATE_INACTIVE   = 1,
    OBJECT_STATE_ACTIVE     = 2
};
