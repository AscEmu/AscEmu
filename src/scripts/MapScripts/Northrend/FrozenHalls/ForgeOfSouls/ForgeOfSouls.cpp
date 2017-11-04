/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "ForgeOfSouls.h"

class ForgeOfSouls : public InstanceScript
{
public:

    ForgeOfSouls(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ForgeOfSouls(pMapMgr); }
};


void ForgeOfSoulsScripts(ScriptMgr* scriptMgr)
{
#ifdef UseNewMapScriptsProject
    scriptMgr->register_instance_script(632, &ForgeOfSouls::Create);
#endif
}
