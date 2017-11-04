/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "SunwellPlateau.h"


class SunwellPlateau : public InstanceScript
{
public:

    SunwellPlateau(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new SunwellPlateau(pMapMgr); }
};


void SunwellPlateauScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(580, &SunwellPlateau::Create);
}

