/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "HellfireRamparts.h"

class HellfireRamparts : public InstanceScript
{
public:

    HellfireRamparts(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        generateBossDataState();
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new HellfireRamparts(pMapMgr); }

    void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
    {
        setData(pCreature->GetEntry(), Finished);
    }
};


void HellfireRampartsScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(543, &HellfireRamparts::Create);
}
