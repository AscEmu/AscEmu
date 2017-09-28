/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "OldHillsbradFoothills.h"


class OldHillsbradFoothills : public InstanceScript
{
public:

    OldHillsbradFoothills(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        generateBossDataState();
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new OldHillsbradFoothills(pMapMgr); }

    void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
    {
        setData(pCreature->GetEntry(), Finished);
    }
};


void OldHillsbradFoothillsScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(560, &OldHillsbradFoothills::Create);
}

