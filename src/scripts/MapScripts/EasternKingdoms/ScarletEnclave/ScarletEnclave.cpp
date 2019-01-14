/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "ScarletEnclave.h"


class ScarletEnclave : public InstanceScript
{
public:

    explicit ScarletEnclave(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ScarletEnclave(pMapMgr); }
};


void ScarletEnclaveScripts(ScriptMgr* /*scriptMgr*/)
{
    //scriptMgr->register_instance_script(000, &ScarletEnclave::Create);
}

