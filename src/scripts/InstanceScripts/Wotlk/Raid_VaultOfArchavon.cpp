/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_VaultOfArchavon.h"
#include <Units/Creatures/Pet.h>
#include "Server/Script/CreatureAIScript.h"

class VaultOfArchavonInstanceScript : public InstanceScript
{
public:
    explicit VaultOfArchavonInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new VaultOfArchavonInstanceScript(pMapMgr); }
};

void SetupVaultOfArchavon(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_VAULT_OF_ARCHAVON, &VaultOfArchavonInstanceScript::Create);
}
