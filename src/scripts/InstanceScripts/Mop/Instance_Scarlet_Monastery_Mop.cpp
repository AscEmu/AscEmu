/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Scarlet_Monastery_Mop.hpp"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

// Mists redesigned Scarlet Monastery into its own instance with a new map id
// (MAP_SCARLET_MONASTERY_MOP, 1004) - unrelated to the Classic instance registered on
// MAP_SCARLET_MONASTERY (189) by SetupScarletMonastery(), which stays untouched.
class ScarletMonasteryMopInstanceScript : public InstanceScript
{
public:
    explicit ScarletMonasteryMopInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new ScarletMonasteryMopInstanceScript(pMapMgr); }
};

void SetupScarletMonasteryMop(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SCARLET_MONASTERY_MOP, &ScarletMonasteryMopInstanceScript::Create);
}
