/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Well_Of_Eternity.h"

class WellOfEternityInstanceScript : public InstanceScript
{
public:
    explicit WellOfEternityInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new WellOfEternityInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }
};

void SetupWellOfEternity(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_WELL_OF_ETERNITY, &WellOfEternityInstanceScript::Create);
}
