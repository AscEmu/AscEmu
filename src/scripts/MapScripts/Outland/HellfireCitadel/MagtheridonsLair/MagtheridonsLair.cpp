/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "MagtheridonsLair.h"

class MagtheridonsLair : public InstanceScript
{
public:

    MagtheridonsLair(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        generateBossDataState();
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new MagtheridonsLair(pMapMgr); }

    void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
    {
        setData(pCreature->GetEntry(), Finished);
    }
};


void MagtheridonsLairScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(544, &MagtheridonsLair::Create);
}
