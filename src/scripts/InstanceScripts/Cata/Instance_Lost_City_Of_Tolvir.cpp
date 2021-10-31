/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Lost_City_Of_Tolvir.h"
#include <Units/Creatures/Pet.h>

class LostCityOfTolvirInstanceScript : public InstanceScript
{
public:
    explicit LostCityOfTolvirInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new LostCityOfTolvirInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }
};

void SetupLostCityOfTolvir(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_LOST_CITY_OF_TOLVIR, &LostCityOfTolvirInstanceScript::Create);
}
