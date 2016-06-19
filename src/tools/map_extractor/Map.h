/**
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MAP_H
#define _MAP_H
#include "loadlib.h"
#include <cstdint>
#include <cmath>

///- Some parts of System.cpp moved to header.
namespace ASCEMU
{
    ///\todo Check structure naming for mmap & map actual: mmaps != map
    struct GridMapFileHeader
    {
        uint32_t mapMagic;
        uint32_t versionMagic;
        uint32_t buildMagic;
        uint32_t areaMapOffset;
        uint32_t areaMapSize;
        uint32_t heightMapOffset;
        uint32_t heightMapSize;
        uint32_t liquidMapOffset;
        uint32_t liquidMapSize;
        uint32_t holesOffset;
        uint32_t holesSize;
    };

#define MAP_HEIGHT_NO_HEIGHT  0x0001
#define MAP_HEIGHT_AS_INT16   0x0002
#define MAP_HEIGHT_AS_INT8    0x0004

    struct GridMapHeightHeader
    {
        uint32_t fourcc;
        uint32_t flags;
        float gridHeight;
        float gridMaxHeight;
    };

#define MAP_LIQUID_TYPE_NO_WATER    0x00
#define MAP_LIQUID_TYPE_WATER       0x01
#define MAP_LIQUID_TYPE_OCEAN       0x02
#define MAP_LIQUID_TYPE_MAGMA       0x04
#define MAP_LIQUID_TYPE_SLIME       0x08

#define MAP_LIQUID_TYPE_DARK_WATER  0x10
#define MAP_LIQUID_TYPE_WMO_WATER   0x20


#define MAP_LIQUID_NO_TYPE    0x0001
#define MAP_LIQUID_NO_HEIGHT  0x0002

    struct GridMapLiquidHeader
    {
        uint32_t fourcc;
        uint16_t flags;
        uint16_t liquidType;
        uint8_t offsetX;
        uint8_t offsetY;
        uint8_t width;
        uint8_t height;
        float liquidLevel;
    };

#define MAP_LIQUID_TYPE_NO_WATER    0x00
#define MAP_LIQUID_TYPE_WATER       0x01
#define MAP_LIQUID_TYPE_OCEAN       0x02
#define MAP_LIQUID_TYPE_MAGMA       0x04
#define MAP_LIQUID_TYPE_SLIME       0x08

#define MAP_ALL_LIQUIDS   (MAP_LIQUID_TYPE_WATER | MAP_LIQUID_TYPE_OCEAN | MAP_LIQUID_TYPE_MAGMA | MAP_LIQUID_TYPE_SLIME)

#define MAP_LIQUID_TYPE_DARK_WATER  0x10
#define MAP_LIQUID_TYPE_WMO_WATER   0x20

}

#endif