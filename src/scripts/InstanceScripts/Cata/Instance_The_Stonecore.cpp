/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_The_Stonecore.h"

class TheStonecoreInstanceScript : public InstanceScript
{
public:
    explicit TheStonecoreInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheStonecoreInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }
};

void SetupTheStonecore(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_STONECORE, &TheStonecoreInstanceScript::Create);
}
