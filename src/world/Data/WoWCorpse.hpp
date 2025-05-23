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

#include <array>

#pragma pack(push, 1)

union corpse_bytes_1_union
{
    struct parts
    {
        uint8_t unk1;
        uint8_t race;
        uint8_t gender;
        uint8_t skin_color;
    } s;
    uint32_t raw;
};

union corpse_bytes_2_union
{
    struct parts
    {
        uint8_t face;
        uint8_t hair_style;
        uint8_t hair_color;
        uint8_t facial_hair;
    } s;
    uint32_t raw;
};

#if VERSION_STRING == Classic

static inline constexpr uint8_t WOWCORPSE_ITEM_COUNT = 19;

struct WoWCorpse : WoWObject
{
    uint64_t owner_guid;
    uint64_t party_guid;
    float o;
    float x;
    float y;
    float z;
    uint32_t display_id;
    std::array<uint32_t, WOWCORPSE_ITEM_COUNT> item;
    corpse_bytes_1_union corpse_bytes_1;
    corpse_bytes_2_union corpse_bytes_2;
    uint32_t guild;
    uint32_t corpse_flags;
    uint32_t dynamic_flags;
    uint32_t corpse_padding;
};
#endif

#if VERSION_STRING == TBC

static inline constexpr uint8_t WOWCORPSE_ITEM_COUNT = 19;

struct WoWCorpse : WoWObject
{
    uint64_t owner_guid;
    uint64_t party_guid;
    float o;
    float x;
    float y;
    float z;
    uint32_t display_id;
    std::array<uint32_t, WOWCORPSE_ITEM_COUNT> item;
    corpse_bytes_1_union corpse_bytes_1;
    corpse_bytes_2_union corpse_bytes_2;
    uint32_t guild;
    uint32_t corpse_flags;
    uint32_t dynamic_flags;
    uint32_t corpse_padding;
};
#endif

#if VERSION_STRING == WotLK

static inline constexpr uint8_t WOWCORPSE_ITEM_COUNT = 19;

struct WoWCorpse : WoWObject
{
    uint64_t owner_guid;
    uint64_t party_guid;
    uint32_t display_id;
    std::array<uint32_t, WOWCORPSE_ITEM_COUNT> item;
    corpse_bytes_1_union corpse_bytes_1;
    corpse_bytes_2_union corpse_bytes_2;
    uint32_t guild;
    uint32_t corpse_flags;
    uint32_t dynamic_flags;
    uint32_t corpse_padding;
};
#endif

#if VERSION_STRING == Cata

static inline constexpr uint8_t WOWCORPSE_ITEM_COUNT = 19;

struct WoWCorpse : WoWObject
{
    uint64_t owner_guid;
    uint64_t party_guid;
    uint32_t display_id;
    std::array<uint32_t, WOWCORPSE_ITEM_COUNT> item;
    corpse_bytes_1_union corpse_bytes_1;
    corpse_bytes_2_union corpse_bytes_2;
    uint32_t corpse_flags;
    uint32_t dynamic_flags;
};
#endif

#if VERSION_STRING == Mop

static inline constexpr uint8_t WOWCORPSE_ITEM_COUNT = 19;

struct WoWCorpse : WoWObject
{
    uint64_t owner_guid;
    uint64_t party_guid;
    uint32_t display_id;
    std::array<uint32_t, WOWCORPSE_ITEM_COUNT> item;
    corpse_bytes_1_union corpse_bytes_1;
    corpse_bytes_2_union corpse_bytes_2;
    uint32_t corpse_flags;
    uint32_t dynamic_flags;
};
#endif

#pragma pack(pop)
