/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "SunkenTemple.h"


class SunkenTemple : public InstanceScript
{
public:

    SunkenTemple(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new SunkenTemple(pMapMgr); }
};


void SunkenTempleScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(109, &SunkenTemple::Create);
}

