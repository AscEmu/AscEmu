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
#include "WoWItem.hpp"
#pragma pack(push, 1)

#if VERSION_STRING == Classic

//todo verify - descriptor is 72, enum 58.
#define WOWCONTAINER_ITEM_SLOT_COUNT 29

struct WoWContainer : WoWItem
{
    uint32_t slot_count;
    uint32_t padding_container;
    guid_union item_slot[WOWCONTAINER_ITEM_SLOT_COUNT];
};
#endif

#if VERSION_STRING == TBC

#define WOWCONTAINER_ITEM_SLOT_COUNT 36

struct WoWContainer : WoWItem
{
    uint32_t slot_count;
    uint32_t padding_container;
    guid_union item_slot[WOWCONTAINER_ITEM_SLOT_COUNT];
};
#endif

#if VERSION_STRING == WotLK

#define WOWCONTAINER_ITEM_SLOT_COUNT 36

struct WoWContainer : WoWItem
{
    uint32_t slot_count;
    uint32_t padding_container;
    guid_union item_slot[WOWCONTAINER_ITEM_SLOT_COUNT];
};
#endif

#if VERSION_STRING == Cata

#define WOWCONTAINER_ITEM_SLOT_COUNT 36

struct WoWContainer : WoWItem
{
    uint32_t slot_count;
    uint32_t container_padding_0;
    guid_union item_slot[WOWCONTAINER_ITEM_SLOT_COUNT];
};
#endif

#if VERSION_STRING == Mop

#define WOWCONTAINER_ITEM_SLOT_COUNT 36

struct WoWContainer : WoWItem
{
    guid_union item_slot[WOWCONTAINER_ITEM_SLOT_COUNT];
    uint32_t slot_count;
};
#endif

#pragma pack(pop)
