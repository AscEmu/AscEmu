/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
#include "world/WorldConf.h"

#pragma pack(push, 1)
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

#if VERSION_STRING == Cata
    uint64_t data;
#endif

    uint32_t type;
    uint32_t entry;
    float_t scale_x;
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
#pragma pack(pop)