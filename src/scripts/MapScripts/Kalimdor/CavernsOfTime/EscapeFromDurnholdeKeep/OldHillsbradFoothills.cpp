/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "OldHillsbradFoothills.h"


class OldHillsbradFoothills : public InstanceScript
{
public:

    OldHillsbradFoothills(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new OldHillsbradFoothills(pMapMgr); }
};


void OldHillsbradFoothillsScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(560, &OldHillsbradFoothills::Create);
}

