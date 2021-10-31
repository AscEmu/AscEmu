/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Bastion_Of_Twilight.h"
#include <Units/Creatures/Pet.h>

class BastionOfTwilightInstanceScript : public InstanceScript
{
public:
    explicit BastionOfTwilightInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new BastionOfTwilightInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }
};

void SetupBastionOfTwilight(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BASTION_OF_TWILIGHT, &BastionOfTwilightInstanceScript::Create);
}
