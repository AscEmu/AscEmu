/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "ZulGurub.h"


class ZulGurub : public InstanceScript
{
public:

    ZulGurub(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        generateBossDataState();
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ZulGurub(pMapMgr); }

    void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
    {
        setData(pCreature->GetEntry(), Finished);
    }
};


void ZulGurubScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(309, &ZulGurub::Create);
}

