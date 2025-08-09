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

#include "WoWObject.hpp"

#pragma pack(push, 1)

#if VERSION_STRING == Classic
union dynamic_bytes_union
{
    struct parts
    {
        uint8_t type;
        uint8_t unk1;
        uint8_t unk2;
        uint8_t unk3;
    } s;
    uint32_t raw;
};

struct WoWDynamicObject : WoWObject
{
    uint64_t caster_guid;
    dynamic_bytes_union dynamicobject_bytes;
    uint32_t spell_id;
    float radius;
    float x;
    float y;
    float z;
    float o;
    uint32_t padding;
};
#endif

#if VERSION_STRING == TBC
union dynamic_bytes_union
{
    struct parts
    {
        uint8_t type;
        uint8_t unk1;
        uint8_t unk2;
        uint8_t unk3;
    } s;
    uint32_t raw;
};

struct WoWDynamicObject : WoWObject
{
    uint64_t caster_guid;
    dynamic_bytes_union dynamicobject_bytes;
    uint32_t spell_id;
    float radius;
    float x;
    float y;
    float z;
    float o;
    uint32_t cast_time;
};
#endif

#if VERSION_STRING == WotLK
union dynamic_bytes_union
{
    struct parts
    {
        uint8_t type;
        uint8_t unk1;
        uint8_t unk2;
        uint8_t unk3;
    } s;
    uint32_t raw;
};

struct WoWDynamicObject : WoWObject
{
    uint64_t caster_guid;
    dynamic_bytes_union dynamicobject_bytes;
    uint32_t spell_id;
    float radius;
    uint32_t cast_time;
};
#endif

#if VERSION_STRING == Cata
union dynamic_bytes_union
{
    // todo: verify bits
    struct parts
    {
        uint32_t spell_visual_id : 28; // not used
        uint32_t type : 4;
    } s;
    uint32_t raw;
};

struct WoWDynamicObject : WoWObject
{
    uint64_t caster_guid;
    dynamic_bytes_union dynamicobject_bytes;
    uint32_t spell_id;
    float radius;
    uint32_t cast_time;
};
#endif

#if VERSION_STRING == Mop
union dynamic_bytes_union
{
    // todo: verify bits
    struct parts
    {
        uint32_t spell_visual_id : 28; // not used
        uint32_t type : 4;
    } s;
    uint32_t raw;
};

struct WoWDynamicObject : WoWObject
{
    uint64_t caster_guid;
    dynamic_bytes_union dynamicobject_bytes;
    uint32_t spell_id;
    float radius;
    uint32_t cast_time;
};
#endif

#pragma pack(pop)
