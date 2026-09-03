/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Siege_Of_Niuzao_Temple.hpp"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class SiegeOfNiuzaoTempleInstanceScript : public InstanceScript
{
public:
    explicit SiegeOfNiuzaoTempleInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new SiegeOfNiuzaoTempleInstanceScript(pMapMgr); }
};

void SetupSiegeOfNiuzaoTemple(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SIEGE_OF_NIUZAO_TEMPLE, &SiegeOfNiuzaoTempleInstanceScript::Create);
}
