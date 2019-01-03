/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "ForgeOfSouls.h"

class ForgeOfSouls : public InstanceScript
{
public:

    explicit ForgeOfSouls(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ForgeOfSouls(pMapMgr); }
};

#ifdef UseNewMapScriptsProject
void ForgeOfSoulsScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(632, &ForgeOfSouls::Create);
}
#endif
