/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "ZulAman.h"


class ZulAman : public InstanceScript
{
public:

    ZulAman(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ZulAman(pMapMgr); }
};


void ZulAmanScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(568, &ZulAman::Create);
}

