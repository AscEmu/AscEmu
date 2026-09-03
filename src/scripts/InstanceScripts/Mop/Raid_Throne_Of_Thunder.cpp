/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Throne_Of_Thunder.hpp"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class ThroneOfThunderInstanceScript : public InstanceScript
{
public:
    explicit ThroneOfThunderInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new ThroneOfThunderInstanceScript(pMapMgr); }
};

void SetupThroneOfThunder(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THRONE_OF_THUNDER, &ThroneOfThunderInstanceScript::Create);
}
