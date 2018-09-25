/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "BlackwingLair.h"


class BlackwingLair : public InstanceScript
{
public:

    explicit BlackwingLair(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new BlackwingLair(pMapMgr); }
};


void BlackwingLairScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(469, &BlackwingLair::Create);
}

