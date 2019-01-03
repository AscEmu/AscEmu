/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "SteamVault.h"

class SteamVault : public InstanceScript
{
public:

    explicit SteamVault(MapMgr* pMapMgr) : InstanceScript(pMapMgr) { }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new SteamVault(pMapMgr); }
};


void SteamVaultScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(545, &SteamVault::Create);
}
