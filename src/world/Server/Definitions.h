/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////
// MAX_NUM_MAPS
//
// \param Max maps 
//
// Vanilla = 600 - untested
// The Burning Crusade = 600 - untested
// Wrath of the Lich King = 800
// Cataclysm = 975
// Mists of Pandaria = 975 - untested
// Warlords of Draenor = untested
// Legion = untested
//
//////////////////////////////////////////////////////////////////////////////////////////

#if VERSION_STRING == Classic
    #define MAX_NUM_MAPS 600
#elif VERSION_STRING == TBC
    #define MAX_NUM_MAPS 600
#elif VERSION_STRING == WotLK
    #define MAX_NUM_MAPS 800
#elif VERSION_STRING == Cata
    #define MAX_NUM_MAPS 975
#elif VERSION_STRING == Mop
    #define MAX_NUM_MAPS 975
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

#if VERSION_STRING >= Cata
#define RACEMASK_ALL_PLAYABLE \
    ((1<<(RACE_HUMAN-1))    |(1<<(RACE_ORC-1))      |(1<<(RACE_DWARF-1))   | \
    (1<<(RACE_NIGHTELF-1))  |(1<<(RACE_UNDEAD-1))   |(1<<(RACE_TAUREN-1))  | \
    (1<<(RACE_GNOME-1))     |(1<<(RACE_TROLL-1))    |(1<<(RACE_BLOODELF-1))| \
    (1<<(RACE_DRAENEI-1))   |(1<<(RACE_GOBLIN-1))   |(1<<(RACE_WORGEN-1)))
#elif VERSION_STRING > Classic
#define RACEMASK_ALL_PLAYABLE \
    ((1<<(RACE_HUMAN-1))    |(1<<(RACE_ORC-1))      |(1<<(RACE_DWARF-1))   | \
    (1<<(RACE_NIGHTELF-1))  |(1<<(RACE_UNDEAD-1))   |(1<<(RACE_TAUREN-1))  | \
    (1<<(RACE_GNOME-1))     |(1<<(RACE_TROLL-1))    |(1<<(RACE_BLOODELF-1))| \
    (1<<(RACE_DRAENEI-1)))
#else
#define RACEMASK_ALL_PLAYABLE \
    ((1<<(RACE_HUMAN-1))    |(1<<(RACE_ORC-1))      |(1<<(RACE_DWARF-1))   | \
    (1<<(RACE_NIGHTELF-1))  |(1<<(RACE_UNDEAD-1))   |(1<<(RACE_TAUREN-1))  | \
    (1<<(RACE_GNOME-1))     |(1<<(RACE_TROLL-1)))
#endif

#if VERSION_STRING >= Cata
#define RACEMASK_ALLIANCE \
    ((1<<(RACE_HUMAN-1))    |(1<<(RACE_DWARF-1))    |(1<<(RACE_NIGHTELF-1)) | \
    (1<<(RACE_GNOME-1))     |(1<<(RACE_DRAENEI-1))  |(1<<(RACE_WORGEN-1)))
#elif VERSION_STRING > Classic
#define RACEMASK_ALLIANCE \
    ((1<<(RACE_HUMAN-1))    |(1<<(RACE_DWARF-1))    |(1<<(RACE_NIGHTELF-1)) | \
    (1<<(RACE_GNOME-1))     |(1<<(RACE_DRAENEI-1)))
#else
#define RACEMASK_ALLIANCE \
    ((1<<(RACE_HUMAN-1))    |(1<<(RACE_DWARF-1))    |(1<<(RACE_NIGHTELF-1)) | \
    (1<<(RACE_GNOME-1)))
#endif

#define RACEMASK_HORDE RACEMASK_ALL_PLAYABLE & ~RACEMASK_ALLIANCE

#define MAKE_NEW_GUID(l, e, h)   uint64(uint64(l) | (uint64(e) << 24) | (uint64(h) << 48))

#define MAKE_PAIR32(l, h)  uint32(uint16(l) | (uint32(h) << 16))
