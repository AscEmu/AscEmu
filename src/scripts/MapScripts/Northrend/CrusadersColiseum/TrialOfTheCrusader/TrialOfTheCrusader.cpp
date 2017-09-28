/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "TrialOfTheCrusader.h"

class TrialOfTheCrusader : public InstanceScript
{
public:

    TrialOfTheCrusader(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        generateBossDataState();
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TrialOfTheCrusader(pMapMgr); }

    void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
    {
        setData(pCreature->GetEntry(), Finished);
    }
};


void TrialOfTheCrusaderScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(649, &TrialOfTheCrusader::Create);
}
