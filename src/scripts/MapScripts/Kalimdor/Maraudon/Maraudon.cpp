/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Maraudon.h"


class Maraudon : public InstanceScript
{
public:

    Maraudon(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Maraudon(pMapMgr); }
};


void MaraudonScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(349, &Maraudon::Create);
}

