/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_ZulGurub.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class ZulGurubInstanceScript : public InstanceScript
{
public:
    explicit ZulGurubInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new ZulGurubInstanceScript(pMapMgr); }
};

void SetupZulGurub(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ZUL_GURUB, &ZulGurubInstanceScript::Create);
}
