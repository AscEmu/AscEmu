/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

constexpr bool isInstance(uint32_t mapId) noexcept
{
    if (mapId <= 1)
        return false; // 0 = Eastern Kingdoms, 1 = Kalimdor

    switch (mapId)
    {
        // Open continents (Classic to The War Within / Midnight)
        case 530: // Outland (TBC)
        case 571: // Northrend (WotLK)
        case 637: // Abyssal Depths / Gilneas / Kezan (Cata)
        case 860: // Wandering Isle (MoP)
        case 870: // Pandaria (MoP)
        case 1116: // Draenor (WoD)
        case 1220: // Broken Isles (Legion)
        case 1642: // Zandalar (BfA)
        case 1643: // Kul Tiras (BfA)
        case 1669: // Argus (Legion)
        case 1718: // Nazjatar (BfA)
        case 2175: // Shadowlands
        case 2222: // Zereth Mortis (Shadowlands)
        case 2444: // Dragon Isles (Dragonflight)
        case 2454: // Zaralek Cavern (Dragonflight)
        case 2548: // Emerald Dream (Dragonflight)
        case 2552: // Khaz Algar (The War Within / Midnight)
            return false;
        default:
            return true;
    }
}

inline constexpr uint32_t MAX_NUM_MAPS = 3106; // Maximum value for all expansions

#define CREATURE_SPAWNS_FIELDCOUNT 32
#define GO_SPAWNS_FIELDCOUNT 17

#define MAP_CELL_DEFAULT_UNLOAD_TIME 300
#define MAKE_CELL_EVENT(x, y) (((x) * 1000) + 200 + y)
// #define DECODE_CELL_EVENT(dest_x, dest_y, ev) (dest_x) = ((ev - 200) / 1000); (dest_y) = ((ev - 200) % 1000);

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
