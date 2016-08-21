/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _DB2STRUCTURE_HPP
#define _DB2STRUCTURE_HPP

#include "Common.h"
#include "Platform/Define.h"

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
            uint32 ID;                      // 0
            uint32 Class;                   // 1
            uint32 SubClass;                // 2 some items have strange subclasses
            int32 SoundOverrideSubclass;    // 3
            int32 Material;                 // 4
            uint32 DisplayId;               // 5
            uint32 InventoryType;           // 6
            uint32 Sheath;                  // 7
        };

        struct ItemCurrencyCostEntry
        {
            //uint32 id;                    // 0
            uint32 itemid;                  // 1
        };

        struct ItemExtendedCostEntry
        {
            uint32 costid;                  // 0
            uint32 honor_points;            // 1
            uint32 arena_points;            // 2
            uint32 arena_slot;              // 3
            uint32 item[5];                 // 5-8
            uint32 count[5];                // 9-13
            uint32 personalrating;          // 14
            //uint32 unk;                   // 15
            uint32 reqcur[5];               // 16-20
            uint32 reqcurrcount[5];         // 21-25
            //uint32 something[5];          // 26-30
        };
    }
}

#endif  //_DB2STRUCTURE_HPP
