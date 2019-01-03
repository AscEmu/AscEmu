/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Nexus.h"

class Nexus : public InstanceScript
{
public:

    explicit Nexus(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Nexus(pMapMgr); }
};

#ifdef UseNewMapScriptsProject
void NexusScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(576, &Nexus::Create);
}
#endif
