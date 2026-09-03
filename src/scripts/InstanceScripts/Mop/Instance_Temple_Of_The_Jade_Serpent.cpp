/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Temple_Of_The_Jade_Serpent.hpp"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class TempleOfTheJadeSerpentInstanceScript : public InstanceScript
{
public:
    explicit TempleOfTheJadeSerpentInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new TempleOfTheJadeSerpentInstanceScript(pMapMgr); }
};

void SetupTempleOfTheJadeSerpent(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_TEMPLE_OF_THE_JADE_SERPENT, &TempleOfTheJadeSerpentInstanceScript::Create);
}
