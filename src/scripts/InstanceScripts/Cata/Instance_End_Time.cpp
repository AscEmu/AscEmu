/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_End_Time.h"
#include <Units/Creatures/Pet.h>

class EndTimeInstanceScript : public InstanceScript
{
public:
    explicit EndTimeInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new EndTimeInstanceScript(pMapMgr); }
};

void SetupEndTime(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_END_TIME, &EndTimeInstanceScript::Create);
}
