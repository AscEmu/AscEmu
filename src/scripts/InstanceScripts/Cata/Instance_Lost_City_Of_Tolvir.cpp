/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Lost_City_Of_Tolvir.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class LostCityOfTolvirInstanceScript : public InstanceScript
{
public:
    explicit LostCityOfTolvirInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new LostCityOfTolvirInstanceScript(pMapMgr); }
};

void SetupLostCityOfTolvir(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_LOST_CITY_OF_TOLVIR, &LostCityOfTolvirInstanceScript::Create);
}
