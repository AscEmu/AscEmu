/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Maraudon.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class MaraudonInstanceScript : public InstanceScript
{
public:
    explicit MaraudonInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new MaraudonInstanceScript(pMapMgr); }
};

void SetupMaraudon(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_MARAUDON, &MaraudonInstanceScript::Create);
}
