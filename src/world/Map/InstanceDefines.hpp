/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <stdint.h>

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
        MAX_RAID_DIFFICULTY     = 4,
        MAX_DIFFICULTY          = 4
    };
}
