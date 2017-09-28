/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "HallsOfStone.h"

class HallsOfStone : public InstanceScript
{
public:

    HallsOfStone(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        generateBossDataState();
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new HallsOfStone(pMapMgr); }

    void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
    {
        setData(pCreature->GetEntry(), Finished);
    }
};


void HallsOfStoneScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(599, &HallsOfStone::Create);
}
