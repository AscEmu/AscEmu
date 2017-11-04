/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "UtgardeKeep.h"

class UtgardeKeep : public InstanceScript
{
public:

    UtgardeKeep(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new UtgardeKeep(pMapMgr); }
};


void UtgardeKeepScripts(ScriptMgr* scriptMgr)
{
#ifdef UseNewMapScriptsProject
    scriptMgr->register_instance_script(574, &UtgardeKeep::Create);
#endif
}
