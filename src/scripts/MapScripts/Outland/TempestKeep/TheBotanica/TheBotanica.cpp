/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "TheBotanica.h"


class TheBotanica : public InstanceScript
{
public:

    TheBotanica(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TheBotanica(pMapMgr); }
};


void TheBotanicaScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(553, &TheBotanica::Create);
}
