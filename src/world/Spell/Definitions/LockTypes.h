/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum LOCKTYPES
{
    LOCKTYPE_PICKLOCK             = 1,
    LOCKTYPE_HERBALISM            = 2,
    LOCKTYPE_MINING               = 3,
    LOCKTYPE_DISARM_TRAP          = 4,
    LOCKTYPE_OPEN                 = 5,
    LOCKTYPE_TREASURE             = 6,
    LOCKTYPE_CALCIFIED_ELVEN_GEMS = 7,
    LOCKTYPE_CLOSE                = 8,
    LOCKTYPE_ARM_TRAP             = 9,
    LOCKTYPE_QUICK_OPEN           = 10,
    LOCKTYPE_QUICK_CLOSE          = 11,
    LOCKTYPE_OPEN_TINKERING       = 12,
    LOCKTYPE_OPEN_KNEELING        = 13,
    LOCKTYPE_OPEN_ATTACKING       = 14,
    LOCKTYPE_GAHZRIDIAN           = 15,
    LOCKTYPE_BLASTING             = 16,
    LOCKTYPE_SLOW_OPEN            = 17,
    LOCKTYPE_SLOW_CLOSE           = 18,
    LOCKTYPE_FISHING              = 19,
    LOCKTYPE_INSCRIPTION          = 20,
    LOCKTYPE_VEHICLE              = 21
};
