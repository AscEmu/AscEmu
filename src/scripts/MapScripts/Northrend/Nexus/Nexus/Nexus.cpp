/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Nexus.h"

class Nexus : public InstanceScript
{
public:

    Nexus(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Nexus(pMapMgr); }
};


void NexusScripts(ScriptMgr* scriptMgr)
{
#ifdef UseNewMapScriptsProject
    scriptMgr->register_instance_script(576, &Nexus::Create);
#endif
}
