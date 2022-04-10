/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_DragonSoul.h"

class DragonSoulInstanceScript : public InstanceScript
{
public:
    explicit DragonSoulInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new DragonSoulInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }
};

void SetupDragonSoul(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_DRAGON_SOUL, &DragonSoulInstanceScript::Create);
}
