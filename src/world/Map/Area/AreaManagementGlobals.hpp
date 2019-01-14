/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AreaStorage.hpp"

namespace MapManagement
{
    namespace AreaManagement
    {
        enum AreaFlags
        {
            AREA_FLAG_UNK0               = 0x00000001,                // Unknown
            AREA_FLAG_UNK1               = 0x00000002,                // Razorfen Downs, Naxxramas and Acherus: The Ebon Hold (3.3.5a)
            AREA_FLAG_UNK2               = 0x00000004,                // Only used for areas on map 571 (development before)
            AREA_FLAG_SLAVE_CAPITAL      = 0x00000008,                // city and city subsones
            AREA_FLAG_UNK3               = 0x00000010,                // can't find common meaning
            AREA_FLAG_SLAVE_CAPITAL2     = 0x00000020,                // slave capital city flag?
            AREA_FLAG_ALLOW_DUELS        = 0x00000040,                // allow to duel here
            AREA_FLAG_ARENA              = 0x00000080,                // arena, both instanced and world arenas
            AREA_FLAG_CAPITAL            = 0x00000100,                // main capital city flag
            AREA_FLAG_CITY               = 0x00000200,                // only for one zone named "City" (where it located?)
            AREA_FLAG_OUTLAND            = 0x00000400,                // expansion zones? (only Eye of the Storm not have this flag, but have 0x00004000 flag)
            AREA_FLAG_SANCTUARY          = 0x00000800,                // sanctuary area (PvP disabled)
            AREA_FLAG_NEED_FLY           = 0x00001000,                // Respawn alive at the graveyard without corpse
            AREA_FLAG_UNUSED1            = 0x00002000,                // Unused in 3.3.5a
            AREA_FLAG_OUTLAND2           = 0x00004000,                // expansion zones? (only Circle of Blood Arena not have this flag, but have 0x00000400 flag)
            AREA_FLAG_OUTDOOR_PVP        = 0x00008000,                // pvp objective area? (Death's Door also has this flag although it's no pvp object area)
            AREA_FLAG_ARENA_INSTANCE     = 0x00010000,                // used by instanced arenas only
            AREA_FLAG_UNUSED2            = 0x00020000,                // Unused in 3.3.5a
            AREA_FLAG_CONTESTED_AREA     = 0x00040000,                // On PvP servers these areas are considered contested, even though the zone it is contained in is a Horde/Alliance territory.
            AREA_FLAG_UNK4               = 0x00080000,                // Valgarde and Acherus: The Ebon Hold
            AREA_FLAG_LOWLEVEL           = 0x00100000,                // used for some starting areas with area_level <= 15
            AREA_FLAG_TOWN               = 0x00200000,                // small towns with Inn
            AREA_FLAG_REST_ZONE_HORDE    = 0x00400000,                // Instead of using areatriggers, the zone will act as one for Horde players (Warsong Hold, Acherus: The Ebon Hold, New Agamand Inn, Vengeance Landing Inn, Sunreaver Pavilion, etc)
            AREA_FLAG_REST_ZONE_ALLIANCE = 0x00800000,                // Instead of using areatriggers, the zone will act as one for Alliance players (Valgarde, Acherus: The Ebon Hold, Westguard Inn, Silver Covenant Pavilion, etc)
            AREA_FLAG_WINTERGRASP        = 0x01000000,                // Wintergrasp and it's subzones
            AREA_FLAG_INSIDE             = 0x02000000,                // used for determinating spell related inside/outside questions in Map::IsOutdoors
            AREA_FLAG_OUTSIDE            = 0x04000000,                // used for determinating spell related inside/outside questions in Map::IsOutdoors
            AREA_FLAG_WINTERGRASP_2      = 0x08000000,                // Can Hearth And Resurrect From Area
            AREA_FLAG_NO_FLY_ZONE        = 0x20000000                 // Marks zones where you cannot fly
        };
        
    } // </ AreaManagementNamespace>
} // </ MapManagementNamespace>
