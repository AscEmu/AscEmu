/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "TheBlackMorass.h"


class TheBlackMorass : public InstanceScript
{
public:

    TheBlackMorass(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        generateBossDataState();
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TheBlackMorass(pMapMgr); }

    void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
    {
        setData(pCreature->GetEntry(), Finished);
    }
};


void TheBlackMorassScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(269, &TheBlackMorass::Create);
}

