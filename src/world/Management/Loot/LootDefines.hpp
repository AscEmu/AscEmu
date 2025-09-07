/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <set>
#include <cstdint>

#define MAX_NR_LOOT_ITEMS 16
#define MAX_NR_LOOT_QUESTITEMS 32
#define MAX_NR_LOOT_EQUIPABLE 2
#define MAX_NR_LOOT_NONEQUIPABLE 3

typedef std::set<uint32_t> LooterSet;

enum PartyLootMethod
{
    PARTY_LOOT_FREE_FOR_ALL         = 0,            // Party Loot Method set to Free for All
    PARTY_LOOT_ROUND_ROBIN          = 1,            // Party Loot Method set to Round Robin
    PARTY_LOOT_MASTER_LOOTER        = 2,            // Party Loot Method set to Master Looter
    PARTY_LOOT_GROUP                = 3,            // Party Loot Method set to Group
    PARTY_LOOT_NEED_BEFORE_GREED    = 4             // Party Loot Method set to Need before Greed
};

enum LootSlotType
{
    LOOT_SLOT_TYPE_ALLOW_LOOT       = 0,            // player can loot the item.
    LOOT_SLOT_TYPE_ROLL_ONGOING     = 1,            // roll is ongoing. player cannot loot.
    LOOT_SLOT_TYPE_MASTER           = 2,            // item can only be distributed by group loot master.
    LOOT_SLOT_TYPE_LOCKED           = 3,            // item is shown in red. player cannot loot.
    LOOT_SLOT_TYPE_OWNER            = 4             // ignore binding confirmation and etc, for single player looting
};

enum LootRollType
{
    ROLL_PASS                       = 0,            // Player Picks Passe for an Item
    ROLL_NEED                       = 1,            // Player Picks Need for an Item
    ROLL_GREED                      = 2,            // Player Picks Greed for an Item
    ROLL_DISENCHANT                 = 3             // Player Picks Disenchant for an Item
};

enum LootModes
{
    LOOT_MODE_DEFAULT               = 0x1,
    LOOT_MODE_HARD_MODE_1           = 0x2,
    LOOT_MODE_HARD_MODE_2           = 0x4,
    LOOT_MODE_HARD_MODE_3           = 0x8,
    LOOT_MODE_HARD_MODE_4           = 0x10,
    LOOT_MODE_JUNK_FISH             = 0x8000
};
