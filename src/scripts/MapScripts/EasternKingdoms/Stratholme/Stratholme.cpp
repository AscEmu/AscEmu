/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Stratholme.h"


class Stratholme : public InstanceScript
{
public:

    Stratholme(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Stratholme(pMapMgr); }
};


void StratholmeScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(329, &Stratholme::Create);
}

