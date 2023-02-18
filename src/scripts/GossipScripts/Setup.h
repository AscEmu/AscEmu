/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <Management/ObjectMgr.h>
#include <Management/TransporterHandler.h>
#include <Objects/Transporter.hpp>

class ScriptMgr;

void SetupDalaranGossip(ScriptMgr* mgr);
void SetupInnkeepers(ScriptMgr* mgr);
void SetupTrainerScript(ScriptMgr* mgr);
void SetupMoongladeGossip(ScriptMgr* mgr);
