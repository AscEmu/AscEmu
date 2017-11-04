/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "RubySanctum.h"

class RubySanctum : public InstanceScript
{
public:

    RubySanctum(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new RubySanctum(pMapMgr); }
};


void RubySanctumScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(724, &RubySanctum::Create);
}
