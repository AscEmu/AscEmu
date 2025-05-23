/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "AEVersion.hpp"
#include "GuidData.hpp"

#include <cstdint>

#pragma pack(push, 1)

#if VERSION_STRING < Cata
struct WoWObject
{
    guid_union guid;
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
    guid_union guid;
    uint64_t data;

    union field_type_union
    {
        struct parts
        {
            uint16_t type;
            uint16_t guild_id;
        } parts;
        uint32_t raw;
    } field_type;

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
    guid_union guid;
    uint64_t data;

    union field_type_union
    {
        struct parts
        {
            uint16_t type;
            uint16_t guild_id;
        } parts;
        uint32_t raw;
    } field_type;

    uint32_t entry;
    union field_dynamic_union
    {
        struct parts
        {
            uint16_t dynamic_flags;
            int16_t path_progress;
        } dynamic_field_parts;
        uint32_t raw;
    } dynamic_field;
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
