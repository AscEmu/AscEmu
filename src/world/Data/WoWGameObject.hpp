/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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
#include "GuidData.hpp"
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
    
    union
    {
        struct
        {
            uint8_t state;
            uint8_t type;
            uint8_t unk;
            uint8_t health;
        } bytes_1_gameobject;
        uint32_t bytes_1;
    };

    union
    {
        struct
        {
            uint8_t transparency;
            uint8_t art_kit;
            uint8_t unk2;
            uint8_t animation_progress;
        } bytes_2_gameobject;
        uint32_t bytes_2;
    };
};
#endif

#pragma pack(pop)
