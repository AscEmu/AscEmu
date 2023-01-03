/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Item.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include "Management/QuestLogEntry.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Spell/SpellMgr.hpp"

void SetupGoHandlers(ScriptMgr* mgr);
void SetupQDGoHandlers(ScriptMgr* mgr);
void SetupRandomScripts(ScriptMgr* mgr);
void SetupMiscCreatures(ScriptMgr* mgr);
void InitializeGameObjectTeleportTable(ScriptMgr* mgr);
void SetupCityDalaran(ScriptMgr* mgr);
