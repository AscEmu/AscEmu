/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Zul_Aman_Cata.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class ZulAmanCataInstanceScript : public InstanceScript
{
public:
    explicit ZulAmanCataInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new ZulAmanCataInstanceScript(pMapMgr); }
};

void SetupZulAmanCata(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ZUL_AMAN_CATACLYSM, &ZulAmanCataInstanceScript::Create);
}
