/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Maraudon.h"


class Maraudon : public InstanceScript
{
public:

    explicit Maraudon(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Maraudon(pMapMgr); }
};


void MaraudonScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(349, &Maraudon::Create);
}

