/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Zul_Gurub_Cata.h"

class ZulGurubCataInstanceScript : public InstanceScript
{
public:
    explicit ZulGurubCataInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new ZulGurubCataInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }
};

void SetupZulGurubCata(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ZUL_GURUB_CATACLYSM, &ZulGurubCataInstanceScript::Create);
}
