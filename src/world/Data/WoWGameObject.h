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

#define GAMEOBJECT_ROTATION_COUNT 4

#pragma pack(push, 1)
#if VERSION_STRING == TBC
struct WoWGameObject : WoWObject
{
    uint64_t object_field_created_by;
    uint32_t display_id;
    uint32_t flags;
    float_t rotation[GAMEOBJECT_ROTATION_COUNT];
    uint32_t state;
    float_t x;
    float_t y;
    float_t z;
    float_t o;
    uint32_t dynamic;
    uint32_t faction;
    uint32_t type_id;
    uint32_t level;
    uint32_t art_kit;
    uint32_t animation_progress;
    uint32_t gameobject_padding;
};
#else
struct WoWGameObject : WoWObject
{
    uint64_t object_field_created_by;
    uint32_t display_id;
    uint32_t flags;
    float_t rotation[GAMEOBJECT_ROTATION_COUNT];
    uint32_t dynamic;
    uint32_t faction;
    uint32_t level;
    union
    {
        struct
        {
            uint8_t state;
            uint8_t type;
            uint8_t art_kit;
            uint8_t animation_progress;
        } bytes_1_gameobject;
        uint32_t bytes_1;
    };
};
#endif
#pragma pack(pop)