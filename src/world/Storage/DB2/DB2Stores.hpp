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

extern SERVER_DECL DB2Storage<DB2::Structures::ItemEntry>                    sItemStore;
extern SERVER_DECL DB2Storage<DB2::Structures::ItemCurrencyCostEntry>        sItemCurrencyCostStore;
extern SERVER_DECL DB2Storage<DB2::Structures::ItemExtendedCostEntry>        sItemExtendedCostStore;

void LoadDB2Stores();

DB2Storage <DB2::Structures::ItemEntry> const* GetItemDisplayStore();

#endif  //_DB2STORES_HPP
