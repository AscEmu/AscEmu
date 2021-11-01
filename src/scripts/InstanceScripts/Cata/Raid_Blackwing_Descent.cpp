/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Blackwing_Descent.h"

class BlackwingDescentInstanceScript : public InstanceScript
{
public:
    explicit BlackwingDescentInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new BlackwingDescentInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }
};

void SetupBlackwingDescent(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLACKWING_DESCENT, &BlackwingDescentInstanceScript::Create);
}
