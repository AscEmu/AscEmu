/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/TaxiMgr.h"
#include "Server/Script/ScriptMgr.h"
#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Server/WorldSession.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"

void SetupDalaranGossip(ScriptMgr* mgr);
void SetupInnkeepers(ScriptMgr* mgr);
void SetupTrainerScript(ScriptMgr* mgr);
void SetupMoongladeGossip(ScriptMgr* mgr);
