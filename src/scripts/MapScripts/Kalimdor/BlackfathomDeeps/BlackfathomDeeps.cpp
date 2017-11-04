/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "BlackfathomDeeps.h"


class BlackfathomDeeps : public InstanceScript
{
public:

    BlackfathomDeeps(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new BlackfathomDeeps(pMapMgr); }
};


void BlackfathomDeepsScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(48, &BlackfathomDeeps::Create);
}

