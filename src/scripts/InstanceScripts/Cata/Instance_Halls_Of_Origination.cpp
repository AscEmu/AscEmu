/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Halls_Of_Origination.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class HallsOfOriginationInstanceScript : public InstanceScript
{
public:
    explicit HallsOfOriginationInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new HallsOfOriginationInstanceScript(pMapMgr); }
};

void SetupHallsOfOrigination(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HALLS_OF_ORIGINATION, &HallsOfOriginationInstanceScript::Create);
}
