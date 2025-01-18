/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Script/ScriptMgr.hpp"

void SetupGoHandlers(ScriptMgr* mgr);
void SetupQDGoHandlers(ScriptMgr* mgr);
void SetupRandomScripts(ScriptMgr* mgr);
void SetupMiscCreatures(ScriptMgr* mgr);
void InitializeGameObjectTeleportTable(ScriptMgr* mgr);
void SetupCityDalaran(ScriptMgr* mgr);
void SetupNeutralGuards(ScriptMgr* mgr);
