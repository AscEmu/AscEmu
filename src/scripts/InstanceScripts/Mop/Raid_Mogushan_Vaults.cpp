/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Mogushan_Vaults.hpp"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class MogushanVaultsInstanceScript : public InstanceScript
{
public:
    explicit MogushanVaultsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new MogushanVaultsInstanceScript(pMapMgr); }
};

void SetupMogushanVaults(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_MOGUSHAN_VAULTS, &MogushanVaultsInstanceScript::Create);
}
