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
#include "WoWObject.h"
#pragma pack(push, 1)

union
{
    struct
    {
        uint8_t type;
        uint8_t unk1;
        uint8_t unk2;
        uint8_t unk3;
    } s;
    uint32_t raw;
} typedef dynamic_bytes_union;

#if VERSION_STRING == Classic
struct WoWDynamicObject : WoWObject
{
    uint64_t caster_guid;
    dynamic_bytes_union dynamicobject_bytes;
    uint32_t spell_id;
    float_t radius;
    float_t x;
    float_t y;
    float_t z;
    float_t o;
    uint32_t padding;
};
#endif

#if VERSION_STRING == TBC
struct WoWDynamicObject : WoWObject
{
    uint64_t caster_guid;
    dynamic_bytes_union dynamicobject_bytes;
    uint32_t spell_id;
    float_t radius;
    float_t x;
    float_t y;
    float_t z;
    float_t o;
    uint32_t cast_time;
};
#endif

#if VERSION_STRING == WotLK
struct WoWDynamicObject : WoWObject
{
    uint64_t caster_guid;
    dynamic_bytes_union dynamicobject_bytes;
    uint32_t spell_id;
    float_t radius;
    uint32_t cast_time;
};
#endif

#if VERSION_STRING == Cata
struct WoWDynamicObject : WoWObject
{
    uint64_t caster_guid;
    dynamic_bytes_union dynamicobject_bytes;
    uint32_t spell_id;
    float_t radius;
    uint32_t cast_time;
};
#endif

#pragma pack(pop)
