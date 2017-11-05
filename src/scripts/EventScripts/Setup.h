/*
 Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 This file is released under the MIT license. See README-MIT for more information.
 */

#include "Units/Creatures/AIInterface.h"
#include "Management/Item.h"
#include "Map/MapMgr.h"
#include "Management/ItemInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include <Management/QuestLogEntry.hpp>
#include "Map/MapScriptInterface.h"
#include <Spell/Customization/SpellCustomizations.hpp>
#include "Map/WorldCreatorDefines.hpp"
#include "../Common/Base.h"

#ifndef EVENT_SCRIPTS_SETUP_H
#define EVENT_SCRIPTS_SETUP_H

void SetupBrewfest(ScriptMgr* mgr);
void SetupDarkmoonFaire(ScriptMgr* mgr);
void SetupDayOfTheDead(ScriptMgr* mgr);
void SetupL70ETC(ScriptMgr* mgr);
void SetupWinterVeil(ScriptMgr* mgr);
void SetupHallowsEnd(ScriptMgr* mgr);
void SetupVayrieTest(ScriptMgr* mgr);

#endif      //EVENT_SCRIPTS_SETUP_H
