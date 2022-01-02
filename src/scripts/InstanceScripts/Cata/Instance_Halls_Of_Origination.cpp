/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Halls_Of_Origination.h"

class HallsOfOriginationInstanceScript : public InstanceScript
{
public:
    explicit HallsOfOriginationInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new HallsOfOriginationInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }
};

void SetupHallsOfOrigination(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HALLS_OF_ORIGINATION, &HallsOfOriginationInstanceScript::Create);
}
