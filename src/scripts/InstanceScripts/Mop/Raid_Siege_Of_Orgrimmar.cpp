/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Siege_Of_Orgrimmar.hpp"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class SiegeOfOrgrimmarInstanceScript : public InstanceScript
{
public:
    explicit SiegeOfOrgrimmarInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new SiegeOfOrgrimmarInstanceScript(pMapMgr); }
};

void SetupSiegeOfOrgrimmar(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SIEGE_OF_ORGRIMMAR, &SiegeOfOrgrimmarInstanceScript::Create);
}
