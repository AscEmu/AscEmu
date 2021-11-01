/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Zul_Aman_Cata.h"

class ZulAmanCataInstanceScript : public InstanceScript
{
public:
    explicit ZulAmanCataInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new ZulAmanCataInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }
};

void SetupZulAmanCata(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ZUL_AMAN_CATACLYSM, &ZulAmanCataInstanceScript::Create);
}
