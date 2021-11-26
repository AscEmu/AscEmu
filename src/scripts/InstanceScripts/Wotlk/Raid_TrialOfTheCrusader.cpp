/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_TrialOfTheCrusader.h"
#include <Objects/Units/Creatures/Pet.h>

class TrialOfTheCrusaderInstanceScript : public InstanceScript
{
public:
    explicit TrialOfTheCrusaderInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new TrialOfTheCrusaderInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }
};

void SetupTrialOfTheCrusader(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_TRIAL_OF_THE_CRUSADER, &TrialOfTheCrusaderInstanceScript::Create);
}
