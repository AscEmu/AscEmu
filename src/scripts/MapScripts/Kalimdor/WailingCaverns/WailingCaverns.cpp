/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "WailingCaverns.h"


class WailingCaverns : public InstanceScript
{
public:

    WailingCaverns(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        generateBossDataState();
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new WailingCaverns(pMapMgr); }

    void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
    {
        setData(pCreature->GetEntry(), Finished);
    }
};


void WailingCavernsScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(43, &WailingCaverns::Create);
}

