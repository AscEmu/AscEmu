/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _DB2STORES_HPP
#define _DB2STORES_HPP

#include "Common.h"
#include "DB2Storage.hpp"
#include "DB2Structures.hpp"

#include <list>

extern DB2Storage <ItemEntry2>                    sItemStore2;
extern DB2Storage <ItemCurrencyCostEntry2>        sItemCurrencyCostStore2;
extern DB2Storage <ItemExtendedCostEntry2>        sItemExtendedCostStore2;

void LoadDB2Stores();

DB2Storage <ItemEntry2> const* GetItemDisplayStore();

#endif  //_DB2STORES_HPP
