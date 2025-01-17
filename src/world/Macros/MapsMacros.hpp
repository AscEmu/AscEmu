/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////
// IS_INSTANCE
//
// \param
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
    #define IS_INSTANCE(a) ((a > 1))                                                         // Classic WoW
#elif VERSION_STRING == TBC
    #define IS_INSTANCE(a) ((a > 1) && (a != 530))                                           // The Burning Crusade
#elif VERSION_STRING == WotLK
    #define IS_INSTANCE(a) ((a > 1) && (a != 530) && (a != 571))                             // Wrath of the Lich King
#elif VERSION_STRING == Cata
    #define IS_INSTANCE(a) ((a > 1) && (a != 530) && (a != 571) && (a != 637))               // Cataclysm
#elif VERSION_STRING == Mop
    #define IS_INSTANCE(a) ((a > 1) && (a != 530) && (a != 571) && (a != 637) && (a != 860)) //  Mists of Pandaria (untested)
#endif

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
    #define MAX_NUM_MAPS 975 // up
#endif

#define CREATURE_SPAWNS_FIELDCOUNT 32
#define GO_SPAWNS_FIELDCOUNT 17

#define MAP_CELL_DEFAULT_UNLOAD_TIME 300
#define MAKE_CELL_EVENT(x, y) (((x) * 1000) + 200 + y)
#define DECODE_CELL_EVENT(dest_x, dest_y, ev) (dest_x) = ((ev - 200) / 1000); (dest_y) = ((ev - 200) % 1000);

#define GO_GUID_RECYCLE_INTERVAL 2048 /// client will cache GO positions. Using same guid for same client will make GO appear at wrong possition so we try to avoid assigning same guid

#define ZONE_MASK_ALL -1

/// MapId -1 doesn't exist (0 is Eastern Kingdoms)
#define MAPID_NOT_IN_WORLD 0xFFFFFFFF

/// Instance Id 0 doesn't exist (-1 is World Instance)
#define INSTANCEID_NOT_IN_WORLD 0

#define RESERVE_EXPAND_SIZE 1024

#define MAP_AREA_NO_AREA      0x0001

#define MAP_HEIGHT_NO_HEIGHT  0x0001
#define MAP_HEIGHT_AS_INT16   0x0002
#define MAP_HEIGHT_AS_INT8    0x0004

#define MAP_LIQUID_NO_TYPE    0x0001
#define MAP_LIQUID_NO_HEIGHT  0x0002

// Extra tolerance to z position to check if it is in air or on ground.
#define GROUND_HEIGHT_TOLERANCE 0.05f
