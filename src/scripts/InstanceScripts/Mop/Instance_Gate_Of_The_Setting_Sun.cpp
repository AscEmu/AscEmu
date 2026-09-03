/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Gate_Of_The_Setting_Sun.hpp"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class GateOfTheSettingSunInstanceScript : public InstanceScript
{
public:
    explicit GateOfTheSettingSunInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new GateOfTheSettingSunInstanceScript(pMapMgr); }
};

void SetupGateOfTheSettingSun(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_GATE_OF_THE_SETTING_SUN, &GateOfTheSettingSunInstanceScript::Create);
}
