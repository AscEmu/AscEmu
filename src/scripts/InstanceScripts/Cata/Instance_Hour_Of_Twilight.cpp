/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Hour_Of_Twilight.h"
#include <Units/Creatures/Pet.h>

class HourOfTwilightInstanceScript : public InstanceScript
{
public:
    explicit HourOfTwilightInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new HourOfTwilightInstanceScript(pMapMgr); }
};

void SetupHourOfTwilight(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HOUR_OF_TWILIGHT, &HourOfTwilightInstanceScript::Create);
}
