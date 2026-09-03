/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Heart_Of_Fear.hpp"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class HeartOfFearInstanceScript : public InstanceScript
{
public:
    explicit HeartOfFearInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new HeartOfFearInstanceScript(pMapMgr); }
};

void SetupHeartOfFear(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_HEART_OF_FEAR, &HeartOfFearInstanceScript::Create);
}
