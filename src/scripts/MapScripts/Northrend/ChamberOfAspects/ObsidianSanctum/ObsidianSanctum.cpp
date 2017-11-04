/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "ObsidianSanctum.h"

class ObsidianSanctum : public InstanceScript
{
public:

    ObsidianSanctum(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ObsidianSanctum(pMapMgr); }
};


void ObsidianSanctumScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(615, &ObsidianSanctum::Create);
}
