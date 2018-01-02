/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "ZulGurub.h"


class ZulGurub : public InstanceScript
{
public:

    ZulGurub(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ZulGurub(pMapMgr); }
};


void ZulGurubScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(309, &ZulGurub::Create);
}

