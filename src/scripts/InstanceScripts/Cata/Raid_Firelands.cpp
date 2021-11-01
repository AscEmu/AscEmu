/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Firelands.h"

class FirelandsInstanceScript : public InstanceScript
{
public:
    explicit FirelandsInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new FirelandsInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }
};

void SetupFirelands(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_FIRELANDS, &FirelandsInstanceScript::Create);
}
