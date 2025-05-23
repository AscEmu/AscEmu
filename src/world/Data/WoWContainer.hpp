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

#include "WoWItem.hpp"

#pragma pack(push, 1)

#if VERSION_STRING == Classic

//todo verify - descriptor is 72, enum 58.
static inline constexpr uint8_t WOWCONTAINER_ITEM_SLOT_COUNT = 29;

struct WoWContainer : WoWItem
{
    uint32_t slot_count;
    uint32_t padding_container;
    std::array<guid_union, WOWCONTAINER_ITEM_SLOT_COUNT> item_slot;
};
#endif

#if VERSION_STRING == TBC

static inline constexpr uint8_t WOWCONTAINER_ITEM_SLOT_COUNT = 36;

struct WoWContainer : WoWItem
{
    uint32_t slot_count;
    uint32_t padding_container;
    std::array<guid_union, WOWCONTAINER_ITEM_SLOT_COUNT> item_slot;
};
#endif

#if VERSION_STRING == WotLK

static inline constexpr uint8_t WOWCONTAINER_ITEM_SLOT_COUNT = 36;

struct WoWContainer : WoWItem
{
    uint32_t slot_count;
    uint32_t padding_container;
    std::array<guid_union, WOWCONTAINER_ITEM_SLOT_COUNT> item_slot;
};
#endif

#if VERSION_STRING == Cata

static inline constexpr uint8_t WOWCONTAINER_ITEM_SLOT_COUNT = 36;

struct WoWContainer : WoWItem
{
    uint32_t slot_count;
    uint32_t container_padding_0;
    std::array<guid_union, WOWCONTAINER_ITEM_SLOT_COUNT> item_slot;
};
#endif

#if VERSION_STRING == Mop

static inline constexpr uint8_t WOWCONTAINER_ITEM_SLOT_COUNT = 36;

struct WoWContainer : WoWItem
{
    std::array<guid_union, WOWCONTAINER_ITEM_SLOT_COUNT> item_slot;
    uint32_t slot_count;
};
#endif

#pragma pack(pop)
