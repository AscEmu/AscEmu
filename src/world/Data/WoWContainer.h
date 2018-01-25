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
#include "WoWItem.h"

#define WOWCONTAINER_ITEM_SLOT_COUNT 72

#pragma pack(push, 1)
struct WoWContainer : WoWItem
{
    uint32_t slot_count;
    uint32_t padding_container;
    uint64_t item_slot[WOWCONTAINER_ITEM_SLOT_COUNT];
};
#pragma pack(pop)