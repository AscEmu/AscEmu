/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_End_Time.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class EndTimeInstanceScript : public InstanceScript
{
public:
    explicit EndTimeInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new EndTimeInstanceScript(pMapMgr); }
};

void SetupEndTime(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_END_TIME, &EndTimeInstanceScript::Create);
}
