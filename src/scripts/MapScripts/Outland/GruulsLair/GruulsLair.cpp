/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "GruulsLair.h"

class GruulsLair : public InstanceScript
{
public:

    GruulsLair(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        generateBossDataState();
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new GruulsLair(pMapMgr); }

    void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
    {
        setData(pCreature->GetEntry(), Finished);
    }
};


void GruulsLairScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(565, &GruulsLair::Create);
}
