/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

/*
 * THIS IS A WOW DATA STRUCT FILE
 *
 * DO NOT ADD ADDITIONAL MEMBERS TO IT
 *
 * DO NOT EDIT IT UNLESS YOU KNOW EXACTLY WHAT YOU ARE DOING
 */

#pragma once

#include <cstdint>
#include <cmath>
#include "GuidData.h"
#include "WorldConf.h"

#pragma pack(push, 1)
#if VERSION_STRING < Cata
struct WoWObject
{
    union
    {
        struct
        {
            uint32_t low;
            uint32_t high;
        } guid_parts;
        uint64_t guid;
    };

#if VERSION_STRING >= Cata
    uint64_t data;
#endif

    uint32_t type;
    uint32_t entry;
    float scale_x;
    uint32_t padding_object;

    void setLowGuid(uint32_t val)
    {
        *reinterpret_cast<uint32_t*>(&guid) = val;
    }

    void setHighGuid(uint32_t val)
    {
        *(reinterpret_cast<uint32_t*>(&guid) + 1) = val;
    }
};
#elif VERSION_STRING == Cata
struct WoWObject
{
    union
    {
        struct
        {
            uint32_t low;
            uint32_t high;
        } guid_parts;
        uint64_t guid;
    };

    uint64_t data;

    uint32_t type;
    uint32_t entry;
    float scale_x;
    uint32_t padding_object;

    void setLowGuid(uint32_t val)
    {
        *reinterpret_cast<uint32_t*>(&guid) = val;
    }

    void setHighGuid(uint32_t val)
    {
        *(reinterpret_cast<uint32_t*>(&guid) + 1) = val;
    }
};
#elif VERSION_STRING == Mop
struct WoWObject
{
    union
    {
        struct
        {
            uint32_t low;
            uint32_t high;
        } guid_parts;
        uint64_t guid;
    };

    uint64_t data;

    uint32_t type;
    uint32_t entry;
    uint32_t dynamic_flags;
    float scale_x;

    void setLowGuid(uint32_t val)
    {
        *reinterpret_cast<uint32_t*>(&guid) = val;
    }

    void setHighGuid(uint32_t val)
    {
        *(reinterpret_cast<uint32_t*>(&guid) + 1) = val;
    }
};
#endif
#pragma pack(pop)