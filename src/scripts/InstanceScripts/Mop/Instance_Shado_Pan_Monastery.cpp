/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Shado_Pan_Monastery.hpp"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class ShadoPanMonasteryInstanceScript : public InstanceScript
{
public:
    explicit ShadoPanMonasteryInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new ShadoPanMonasteryInstanceScript(pMapMgr); }
};

void SetupShadoPanMonastery(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SHADO_PAN_MONASTERY, &ShadoPanMonasteryInstanceScript::Create);
}
