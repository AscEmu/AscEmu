/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Throne_Of_Tides.h"

class ThroneOfTidesInstanceScript : public InstanceScript
{
public:
    explicit ThroneOfTidesInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new ThroneOfTidesInstanceScript(pMapMgr); }
};

void SetupThroneOfTides(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THRONE_OF_THE_TIDES, &ThroneOfTidesInstanceScript::Create);
}
