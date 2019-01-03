/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "TrialOfTheChampion.h"

class TrialOfTheChampion : public InstanceScript
{
public:

    explicit TrialOfTheChampion(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TrialOfTheChampion(pMapMgr); }
};


void TrialOfTheChampionScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(650, &TrialOfTheChampion::Create);
}
