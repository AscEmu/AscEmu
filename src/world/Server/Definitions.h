/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WOWSERVER_DEFINITION_H
#define WOWSERVER_DEFINITION_H


#if VERSION_STRING < Cata
const unsigned NUM_MAPS = 800;
#else
const unsigned NUM_MAPS = 975; // at least Darkmoon Faire
#endif

const unsigned NUM_INSTANCE_MODES = 4;


/// Sorry...need this enumeration in Player.*
enum INSTANCE_MODE
{
    MODE_NORMAL = 0,
    MODE_HEROIC = 1
};

enum RAID_MODE
{
    MODE_NORMAL_10MEN    = 0,
    MODE_NORMAL_25MEN    = 1,
    MODE_HEROIC_10MEN    = 2,
    MODE_HEROIC_25MEN    = 3,
    TOTAL_RAID_MODES     = 4
};

enum TimeConstants
{
    MINUTE          = 60,
    HOUR            = MINUTE * 60,
    DAY             = HOUR * 24,
    WEEK            = DAY * 7,
    MONTH           = DAY * 30,
    YEAR            = MONTH * 12,
    IN_MILLISECONDS = 1000
};

#define MAX_RACES         12

#if VERSION_STRING == Cata
#define RACEMASK_ALL_PLAYABLE \
    ((1<<(RACE_HUMAN-1))    |(1<<(RACE_ORC-1))      |(1<<(RACE_DWARF-1))   | \
    (1<<(RACE_NIGHTELF-1))  |(1<<(RACE_UNDEAD-1))   |(1<<(RACE_TAUREN-1))  | \
    (1<<(RACE_GNOME-1))     |(1<<(RACE_TROLL-1))    |(1<<(RACE_BLOODELF-1))| \
    (1<<(RACE_DRAENEI-1))   |(1<<(RACE_GOBLIN-1))   |(1<<(RACE_WORGEN-1)))
#else
#define RACEMASK_ALL_PLAYABLE \
    ((1<<(RACE_HUMAN-1))    |(1<<(RACE_ORC-1))      |(1<<(RACE_DWARF-1))   | \
    (1<<(RACE_NIGHTELF-1))  |(1<<(RACE_UNDEAD-1))   |(1<<(RACE_TAUREN-1))  | \
    (1<<(RACE_GNOME-1))     |(1<<(RACE_TROLL-1))    |(1<<(RACE_BLOODELF-1))| \
    (1<<(RACE_DRAENEI-1)))
#endif

#if VERSION_STRING == Cata
#define RACEMASK_ALLIANCE \
    ((1<<(RACE_HUMAN-1))    |(1<<(RACE_DWARF-1))    |(1<<(RACE_NIGHTELF-1)) | \
    (1<<(RACE_GNOME-1))     |(1<<(RACE_DRAENEI-1))  |(1<<(RACE_WORGEN-1)))
#else
#define RACEMASK_ALLIANCE \
    ((1<<(RACE_HUMAN-1))    |(1<<(RACE_DWARF-1))    |(1<<(RACE_NIGHTELF-1)) | \
    (1<<(RACE_GNOME-1))     |(1<<(RACE_DRAENEI-1)))
#endif

#define RACEMASK_HORDE RACEMASK_ALL_PLAYABLE & ~RACEMASK_ALLIANCE

#define MAKE_NEW_GUID(l, e, h)   uint64(uint64(l) | (uint64(e) << 24) | (uint64(h) << 48))

#define MAKE_PAIR32(l, h)  uint32(uint16(l) | (uint32(h) << 16))

#endif // WOWSERVER_DEFINITION_H
