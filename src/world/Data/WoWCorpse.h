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
#pragma pack(push, 1)

union
{
    struct
    {
        uint8_t unk1;
        uint8_t race;
        uint8_t unk2;
        uint8_t skin_color;
    } s;
    uint32_t raw;
} typedef corpse_bytes_1_union;

union
{
    struct
    {
        uint8_t face;
        uint8_t hair_style;
        uint8_t hair_color;
        uint8_t facial_hair;
    } s;
    uint32_t raw;
} typedef corpse_bytes_2_union;

#if VERSION_STRING == Classic

#define WOWCORPSE_ITEM_COUNT 19

struct WoWCorpse : WoWObject
{
    uint64_t owner_guid;
    uint64_t party_guid;
    float o;
    float x;
    float y;
    float z;
    uint32_t display_id;
    uint32_t item[WOWCORPSE_ITEM_COUNT];
    corpse_bytes_1_union corpse_bytes_1;
    corpse_bytes_2_union corpse_bytes_2;
    uint32_t guild;
    uint32_t corpse_flags;
    uint32_t dynamic_flags;
    uint32_t corpse_padding;
};
#endif

#if VERSION_STRING == TBC

#define WOWCORPSE_ITEM_COUNT 19

struct WoWCorpse : WoWObject
{
    uint64_t owner_guid;
    uint64_t party_guid;
    float o;
    float x;
    float y;
    float z;
    uint32_t display_id;
    uint32_t item[WOWCORPSE_ITEM_COUNT];
    corpse_bytes_1_union corpse_bytes_1;
    corpse_bytes_2_union corpse_bytes_2;
    uint32_t guild;
    uint32_t corpse_flags;
    uint32_t dynamic_flags;
    uint32_t corpse_padding;
};
#endif

#if VERSION_STRING == WotLK

#define WOWCORPSE_ITEM_COUNT 19

struct WoWCorpse : WoWObject
{
    uint64_t owner_guid;
    uint64_t party_guid;
    uint32_t display_id;
    uint32_t item[WOWCORPSE_ITEM_COUNT];
    corpse_bytes_1_union corpse_bytes_1;
    corpse_bytes_2_union corpse_bytes_2;
    uint32_t guild;
    uint32_t corpse_flags;
    uint32_t dynamic_flags;
    uint32_t corpse_padding;
};
#endif

#if VERSION_STRING == Cata

#define WOWCORPSE_ITEM_COUNT 19

struct WoWCorpse : WoWObject
{
    uint64_t owner_guid;
    uint64_t party_guid;
    uint32_t display_id;
    uint32_t item[WOWCORPSE_ITEM_COUNT];
    corpse_bytes_1_union corpse_bytes_1;
    corpse_bytes_2_union corpse_bytes_2;
    uint32_t corpse_flags;
    uint32_t dynamic_flags;
};
#endif

#if VERSION_STRING == Mop

#define WOWCORPSE_ITEM_COUNT 19

struct WoWCorpse : WoWObject
{
    uint64_t owner_guid;
    uint64_t party_guid;
    uint32_t display_id;
    uint32_t item[WOWCORPSE_ITEM_COUNT];
    corpse_bytes_1_union corpse_bytes_1;
    corpse_bytes_2_union corpse_bytes_2;
    uint32_t corpse_flags;
    uint32_t dynamic_flags;
};
#endif

#pragma pack(pop)
