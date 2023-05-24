/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Script/ScriptMgr.h"
#include "Management/ItemInterface.h"
#include "Management/QuestLogEntry.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Movement/Spline/MoveSpline.h"
#include "Objects/Item.hpp"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/Script/CreatureAIScript.h"
#include "Spell/SpellMgr.hpp"
#include "Storage/MySQLDataStore.hpp"

void SetupNeutralGuards(ScriptMgr* mgr);
