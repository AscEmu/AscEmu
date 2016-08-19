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

// Structures using to access raw DB2 data and required packing to portability
struct ItemEntry2
{
    uint32   ID;                                             // 0
    uint32   Class;                                          // 1
    uint32   SubClass;                                       // 2
    int32    Unk0;                                           // 3
    int32    Material;                                       // 4
    uint32   DisplayId;                                      // 5
    uint32   InventoryType;                                  // 6
    uint32   Sheath;                                         // 7
};

struct ItemCurrencyCostEntry2
{
    //uint32 id;                                            // 0
    uint32 itemid;                                          // 1
};

#define MAX_EXTENDED_COST_ITEMS         5
#define MAX_EXTENDED_COST_CURRENCIES    5

struct ItemExtendedCostEntry2
{
    uint32      Id;                                         // 0
    uint32      reqhonorpoints;                             // 1        m_honorPoints
    uint32      reqarenapoints;                             // 2        m_arenaPoints
    uint32      reqarenaslot;                               // 3        m_arenaBracket
    uint32      reqitem[MAX_EXTENDED_COST_ITEMS];           // 5-8      m_itemID
    uint32      reqitemcount[MAX_EXTENDED_COST_ITEMS];      // 9-13     m_itemCount
    uint32      reqpersonalarenarating;                     // 14       m_requiredArenaRating
    //uint32                                                // 15       m_itemPurchaseGroup
    uint32    reqcur[MAX_EXTENDED_COST_CURRENCIES];         // 16-20
    uint32    reqcurrcount[MAX_EXTENDED_COST_CURRENCIES];   // 21-25
    //uint32    something[5];                               // 26-30
};

namespace DB2
{
    namespace Structures
    {
        namespace
        {
            const char Item_format[] = "niiiiiii";
            const char ItemCurrencyCost_format[] = "di";
            const char ItemExtendedCostEntry_format[] = "niiiiiiiiiiiiiixiiiiiiiiiixxxxx";
        }
    }
}

#endif  //_DB2STRUCTURE_HPP
