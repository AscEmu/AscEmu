/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Zul_Gurub_Cata.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class ZulGurubCataInstanceScript : public InstanceScript
{
public:
    explicit ZulGurubCataInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new ZulGurubCataInstanceScript(pMapMgr); }
};

void SetupZulGurubCata(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ZUL_GURUB_CATACLYSM, &ZulGurubCataInstanceScript::Create);
}
