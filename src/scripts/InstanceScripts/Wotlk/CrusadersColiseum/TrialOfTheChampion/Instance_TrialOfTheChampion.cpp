/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_TrialOfTheChampion.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class TrialOfTheChampionInstanceScript : public InstanceScript
{
public:
    explicit TrialOfTheChampionInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new TrialOfTheChampionInstanceScript(pMapMgr); }
};

void SetupTrialOfTheChampion(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_TRIAL_OF_THE_CHAMPION, &TrialOfTheChampionInstanceScript::Create);
}
