/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "ShadowLabyrinth.h"


class ShadowLabyrinth : public InstanceScript
{
public:

    ShadowLabyrinth(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        generateBossDataState();
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ShadowLabyrinth(pMapMgr); }

    void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
    {
        setData(pCreature->GetEntry(), Finished);
    }
};


void ShadowLabyrinthScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(555, &ShadowLabyrinth::Create);
}

