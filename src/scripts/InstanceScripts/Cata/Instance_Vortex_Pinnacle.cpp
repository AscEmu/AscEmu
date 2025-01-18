/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Vortex_Pinnacle.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class VortexPinnacleInstanceScript : public InstanceScript
{
public:
    explicit VortexPinnacleInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new VortexPinnacleInstanceScript(pMapMgr); }
};

void SetupVortexPinnacle(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_VORTEX_PINNACLE, &VortexPinnacleInstanceScript::Create);
}
