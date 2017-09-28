/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "ScarletEnclave.h"


class ScarletEnclave : public InstanceScript
{
public:

    ScarletEnclave(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        generateBossDataState();
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ScarletEnclave(pMapMgr); }

    void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
    {
        setData(pCreature->GetEntry(), Finished);
    }
};


void ScarletEnclaveScripts(ScriptMgr* scriptMgr)
{
    //scriptMgr->register_instance_script(000, &ScarletEnclave::Create);
}

