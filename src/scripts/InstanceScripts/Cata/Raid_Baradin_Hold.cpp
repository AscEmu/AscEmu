/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Baradin_Hold.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class BaradinHoldInstanceScript : public InstanceScript
{
public:
    explicit BaradinHoldInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new BaradinHoldInstanceScript(pMapMgr); }
};

void SetupBaradinHold(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BARADIN_HOLD, &BaradinHoldInstanceScript::Create);
}
