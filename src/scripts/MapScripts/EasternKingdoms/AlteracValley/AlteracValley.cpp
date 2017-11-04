/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "AlteracValley.h"


class AlteracValley : public InstanceScript
{
public:

    AlteracValley(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new AlteracValley(pMapMgr); }
};


void AlteracValleyScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(30, &AlteracValley::Create);
}

