/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MAPDEFINES_H
#define _MAPDEFINES_H

#include "DetourNavMesh.h"

const uint32_t MMAP_MAGIC = 0x4d4d4150; // 'MMAP'
#define MMAP_VERSION 5

#define NO_WMO_HEIGHT -200000

struct MmapTileHeader
{
    uint32_t mmapMagic;
    uint32_t dtVersion;
    uint32_t mmapVersion;
    uint32_t size;
    uint32_t usesLiquids;

    MmapTileHeader() : mmapMagic(MMAP_MAGIC), dtVersion(DT_NAVMESH_VERSION),
        mmapVersion(MMAP_VERSION), size(0), usesLiquids(0) {}
};

enum NavArea
{
    NAV_AREA_EMPTY = 0,
    // areas 1-60 will be used for destructible areas (currently skipped in vmaps, WMO with flag 1)
    // ground is the highest value to make recast choose ground over water when merging surfaces very close to each other (shallow water would be walkable)
    NAV_AREA_GROUND = 11,
    NAV_AREA_GROUND_STEEP = 10,
    NAV_AREA_WATER = 9,
    NAV_AREA_MAGMA_SLIME = 8, // don't need to differentiate between them
    NAV_AREA_MAX_VALUE = NAV_AREA_GROUND,
    NAV_AREA_MIN_VALUE = NAV_AREA_MAGMA_SLIME,
    NAV_AREA_ALL_MASK = 0x3F // max allowed value
};

enum NavTerrainFlag
{
    NAV_EMPTY = 0x00,
    NAV_GROUND = 1 << (NAV_AREA_MAX_VALUE - NAV_AREA_GROUND),
    NAV_GROUND_STEEP = 1 << (NAV_AREA_MAX_VALUE - NAV_AREA_GROUND_STEEP),
    NAV_WATER = 1 << (NAV_AREA_MAX_VALUE - NAV_AREA_WATER),
    NAV_MAGMA_SLIME = 1 << (NAV_AREA_MAX_VALUE - NAV_AREA_MAGMA_SLIME)
};

#endif
