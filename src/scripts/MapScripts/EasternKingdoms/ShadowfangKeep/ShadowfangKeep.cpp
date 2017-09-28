/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "ShadowfangKeep.h"


class ShadowfangKeep : public InstanceScript
{
public:

    ShadowfangKeep(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        generateBossDataState();
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ShadowfangKeep(pMapMgr); }

    void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
    {
        setData(pCreature->GetEntry(), Finished);
    }
};


void ShadowfangKeepScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(33, &ShadowfangKeep::Create);
}

