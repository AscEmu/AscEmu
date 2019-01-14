/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"

#include <map>
#include <set>
#include <vector>


namespace DB2
{
    namespace Structures
    {
        namespace
        {
            const char item_entry_format[] = "niiiiiii";
            const char item_currency_cost_format[] = "di";
            const char item_extended_cost_format[] = "niiiiiiiiiiiiiixiiiiiiiiiixxxxx";
        }

        struct ItemEntry
        {
            uint32_t ID;                      // 0
            uint32_t Class;                   // 1
            uint32_t SubClass;                // 2
            int32_t SoundOverrideSubclass;    // 3
            int32_t Material;                 // 4
            uint32_t DisplayId;               // 5
            uint32_t InventoryType;           // 6
            uint32_t Sheath;                  // 7
        };

        struct ItemCurrencyCostEntry
        {
            //uint32_t id;                    // 0
            uint32_t itemid;                  // 1
        };

        struct ItemExtendedCostEntry
        {
            uint32_t costid;                  // 0
            uint32_t honor_points;            // 1
            uint32_t arena_points;            // 2
            uint32_t arena_slot;              // 3
            uint32_t item[5];                 // 5-8
            uint32_t count[5];                // 9-13
            uint32_t personalrating;          // 14
            //uint32_t unk;                   // 15
            uint32_t reqcur[5];               // 16-20
            uint32_t reqcurrcount[5];         // 21-25
            //uint32_t unkunk[5];             // 26-30
        };
    }
}
