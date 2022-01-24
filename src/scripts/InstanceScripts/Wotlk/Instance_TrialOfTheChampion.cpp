/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_TrialOfTheChampion.h"
#include <Objects/Units/Creatures/Pet.h>

class TrialOfTheChampionInstanceScript : public InstanceScript
{
public:
    explicit TrialOfTheChampionInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new TrialOfTheChampionInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }
};

void SetupTrialOfTheChampion(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_TRIAL_OF_THE_CHAMPION, &TrialOfTheChampionInstanceScript::Create);
}
