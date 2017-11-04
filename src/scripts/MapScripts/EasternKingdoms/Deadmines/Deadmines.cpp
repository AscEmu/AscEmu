/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Deadmines.h"


class Deadmines : public InstanceScript
{
public:

    Deadmines(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Deadmines(pMapMgr); }
};


void DeadminesScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(36, &Deadmines::Create);
}

