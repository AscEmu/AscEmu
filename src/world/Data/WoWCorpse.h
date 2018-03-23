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

#if VERSION_STRING == Classic

#define WOWCORPSE_ITEM_COUNT 19

struct WoWCorpse : WoWObject
{
    uint64_t owner_guid;
    uint64_t party_guid;
    float_t o;
    float_t x;
    float_t y;
    float_t z;
    uint32_t display_id;
    uint32_t item[WOWCORPSE_ITEM_COUNT];
    uint32_t corpse_bytes_1;
    uint32_t corpse_bytes_2;
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
    float_t o;
    float_t x;
    float_t y;
    float_t z;
    uint32_t display_id;
    uint32_t item[WOWCORPSE_ITEM_COUNT];
    uint32_t corpse_bytes_1;
    uint32_t corpse_bytes_2;
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
    uint32_t corpse_bytes_1;
    uint32_t corpse_bytes_2;
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
    uint32_t corpse_bytes_1;
    uint32_t corpse_bytes_2;
    uint32_t corpse_flags;
    uint32_t dynamic_flags;
};
#endif

#pragma pack(pop)
