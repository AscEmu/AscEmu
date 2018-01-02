/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "DraktharonKeep.h"

class DraktharonKeep : public InstanceScript
{
public:

    DraktharonKeep(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new DraktharonKeep(pMapMgr); }
};


void DraktharonKeepScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(600, &DraktharonKeep::Create);
}
