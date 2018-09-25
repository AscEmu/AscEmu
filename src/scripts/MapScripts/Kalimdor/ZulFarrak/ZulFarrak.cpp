/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "ZulFarrak.h"


class ZulFarrak : public InstanceScript
{
public:

    explicit ZulFarrak(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ZulFarrak(pMapMgr); }
};


void ZulFarrakScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(209, &ZulFarrak::Create);
}

