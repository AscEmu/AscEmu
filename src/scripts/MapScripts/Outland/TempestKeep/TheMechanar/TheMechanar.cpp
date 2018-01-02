/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "TheMechanar.h"


class TheMechanar : public InstanceScript
{
public:

    TheMechanar(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TheMechanar(pMapMgr); }
};


void TheMechanarScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(554, &TheMechanar::Create);
}
