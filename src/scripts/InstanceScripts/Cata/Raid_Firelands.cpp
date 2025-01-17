/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Firelands.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class FirelandsInstanceScript : public InstanceScript
{
public:
    explicit FirelandsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new FirelandsInstanceScript(pMapMgr); }
};

void SetupFirelands(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_FIRELANDS, &FirelandsInstanceScript::Create);
}
