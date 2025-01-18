/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Bastion_Of_Twilight.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class BastionOfTwilightInstanceScript : public InstanceScript
{
public:
    explicit BastionOfTwilightInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new BastionOfTwilightInstanceScript(pMapMgr); }
};

void SetupBastionOfTwilight(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BASTION_OF_TWILIGHT, &BastionOfTwilightInstanceScript::Create);
}
