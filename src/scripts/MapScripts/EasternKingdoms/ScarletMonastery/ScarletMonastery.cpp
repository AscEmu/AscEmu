/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "ScarletMonastery.h"


class ScarletMonastery : public InstanceScript
{
public:

    ScarletMonastery(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ScarletMonastery(pMapMgr); }
};


void ScarletMonasteryScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(189, &ScarletMonastery::Create);
}

