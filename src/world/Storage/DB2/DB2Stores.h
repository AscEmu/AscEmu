/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"
#include "DB2Storage.h"
#include "DB2Structures.h"

#include <list>

extern SERVER_DECL DB2Storage<DB2::Structures::ItemEntry>                    sItemStore;
extern SERVER_DECL DB2Storage<DB2::Structures::ItemCurrencyCostEntry>        sItemCurrencyCostStore;
extern SERVER_DECL DB2Storage<DB2::Structures::ItemExtendedCostEntry>        sItemExtendedCostStore;

void LoadDB2Stores();
