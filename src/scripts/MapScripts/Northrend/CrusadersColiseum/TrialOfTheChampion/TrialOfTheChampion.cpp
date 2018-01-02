/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "TrialOfTheChampion.h"

class TrialOfTheChampion : public InstanceScript
{
public:

    TrialOfTheChampion(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TrialOfTheChampion(pMapMgr); }
};


void TrialOfTheChampionScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(650, &TrialOfTheChampion::Create);
}
