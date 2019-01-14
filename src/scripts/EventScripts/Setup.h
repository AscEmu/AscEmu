/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Units/Creatures/AIInterface.h"
#include "Management/Item.h"
#include "Map/MapMgr.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include <Management/QuestLogEntry.hpp>
#include "Map/MapScriptInterface.h"
#include <Spell/SpellMgr.h>
#include "Map/WorldCreatorDefines.hpp"

void SetupBrewfest(ScriptMgr* mgr);
void SetupDarkmoonFaire(ScriptMgr* mgr);
void SetupDayOfTheDead(ScriptMgr* mgr);
void SetupL70ETC(ScriptMgr* mgr);
void SetupWinterVeil(ScriptMgr* mgr);
void SetupHallowsEnd(ScriptMgr* mgr);
void SetupVayrieTest(ScriptMgr* mgr);
