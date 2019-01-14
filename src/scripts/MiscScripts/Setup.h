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
#include "Management/QuestLogEntry.hpp"
#include "Map/MapScriptInterface.h"
#include "Spell/SpellMgr.h"
#include "Map/WorldCreatorDefines.hpp"

void SetupGoHandlers(ScriptMgr* mgr);
void SetupQDGoHandlers(ScriptMgr* mgr);
void SetupRandomScripts(ScriptMgr* mgr);
void SetupMiscCreatures(ScriptMgr* mgr);
void InitializeGameObjectTeleportTable(ScriptMgr* mgr);
void SetupCityDalaran(ScriptMgr* mgr);
