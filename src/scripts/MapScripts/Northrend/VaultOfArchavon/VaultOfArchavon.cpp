/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "VaultOfArchavon.h"

class VaultOfArchavon : public InstanceScript
{
public:

    explicit VaultOfArchavon(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new VaultOfArchavon(pMapMgr); }
};


void VaultOfArchavonScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(624, &VaultOfArchavon::Create);
}
