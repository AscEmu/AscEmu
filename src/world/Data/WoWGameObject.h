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
#include "WoWObject.h"
#include "GuidData.h"
#pragma pack(push, 1)

#if VERSION_STRING == Classic

#define GAMEOBJECT_ROTATION_COUNT 4

struct WoWGameObject : WoWObject
{
    guid_union object_field_created_by;
    uint32_t display_id;
    uint32_t flags;
    float rotation[GAMEOBJECT_ROTATION_COUNT];
    uint32_t state;
    float x;
    float y;
    float z;
    float o;
    uint32_t dynamic;
    uint32_t faction_template;
    uint32_t type;
    uint32_t level;
    uint32_t art_kit;
    uint32_t animation_progress;
    uint32_t gameobject_padding;
};
#endif

#if VERSION_STRING == TBC

#define GAMEOBJECT_ROTATION_COUNT 4

struct WoWGameObject : WoWObject
{
    guid_union object_field_created_by;
    uint32_t display_id;
    uint32_t flags;
    float rotation[GAMEOBJECT_ROTATION_COUNT];
    uint32_t state;
    float x;
    float y;
    float z;
    float o;
    uint32_t dynamic;
    uint32_t faction_template;
    uint32_t type;
    uint32_t level;
    uint32_t art_kit;
    uint32_t animation_progress;
    uint32_t gameobject_padding;
};
#endif

#if VERSION_STRING == WotLK

#define GAMEOBJECT_ROTATION_COUNT 4

struct WoWGameObject : WoWObject
{
    guid_union object_field_created_by;
    uint32_t display_id;
    uint32_t flags;
    float rotation[GAMEOBJECT_ROTATION_COUNT];
    uint32_t dynamic;
    uint32_t faction_template;
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

#if VERSION_STRING == Cata

#define GAMEOBJECT_ROTATION_COUNT 4

struct WoWGameObject : WoWObject
{
    guid_union object_field_created_by;
    uint32_t display_id;
    uint32_t flags;
    float rotation[GAMEOBJECT_ROTATION_COUNT];
    uint32_t dynamic;
    uint32_t faction_template;
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

#if VERSION_STRING == Mop

#define GAMEOBJECT_ROTATION_COUNT 4

struct WoWGameObject : WoWObject
{
    guid_union object_field_created_by;
    uint32_t display_id;
    uint32_t flags;
    float rotation[GAMEOBJECT_ROTATION_COUNT];
    uint32_t faction_template;
    uint32_t level;
    uint32_t dynamic;
    
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
