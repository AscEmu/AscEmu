/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Item.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Management/ItemInterface.h"
#include <Management/QuestLogEntry.hpp>
#include "Movement/Spline/MoveSpline.h"
#include "Storage/MySQLDataStore.hpp"
#include <Spell/SpellMgr.hpp>

void SetupBrewfest(ScriptMgr* mgr);
void SetupDarkmoonFaire(ScriptMgr* mgr);
void SetupDayOfTheDead(ScriptMgr* mgr);
void SetupL70ETC(ScriptMgr* mgr);
void SetupWinterVeil(ScriptMgr* mgr);
void SetupHallowsEnd(ScriptMgr* mgr);
