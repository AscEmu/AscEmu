/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Terrace_Of_Endless_Spring.hpp"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class TerraceOfEndlessSpringInstanceScript : public InstanceScript
{
public:
    explicit TerraceOfEndlessSpringInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new TerraceOfEndlessSpringInstanceScript(pMapMgr); }
};

void SetupTerraceOfEndlessSpring(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_TERRACE_OF_ENDLESS_SPRING, &TerraceOfEndlessSpringInstanceScript::Create);
}
