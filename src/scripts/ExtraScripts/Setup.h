/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Units/Creatures/AIInterface.h"
#include "Management/Item.h"
#include "Map/MapMgr.h"
#include "Map/MapScriptInterface.h"
#include "Map/WorldCreatorDefines.hpp"
#include "Management/ItemInterface.h"
#include <Management/QuestLogEntry.hpp>
#include "Movement/Spline/MoveSpline.h"
#include "Storage/MySQLDataStore.hpp"
#include <Spell/SpellMgr.hpp>

void SetupNeutralGuards(ScriptMgr* mgr);
