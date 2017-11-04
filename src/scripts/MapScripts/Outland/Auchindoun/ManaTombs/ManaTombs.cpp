/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "ManaTombs.h"


class ManaTombs : public InstanceScript
{
public:

    ManaTombs(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ManaTombs(pMapMgr); }
};


void ManaTombsScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(557, &ManaTombs::Create);
}

