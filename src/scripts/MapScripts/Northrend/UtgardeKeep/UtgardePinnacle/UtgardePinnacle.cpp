/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "UtgardePinnacle.h"

class UtgardePinnacle : public InstanceScript
{
public:

    UtgardePinnacle(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new UtgardePinnacle(pMapMgr); }
};


void UtgardePinnacleScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(575, &UtgardePinnacle::Create);
}
