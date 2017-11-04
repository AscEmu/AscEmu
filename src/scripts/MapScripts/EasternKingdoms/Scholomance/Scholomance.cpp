/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Scholomance.h"


class Scholomance : public InstanceScript
{
public:

    Scholomance(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Scholomance(pMapMgr); }
};


void ScholomanceScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(289, &Scholomance::Create);
}

