/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Scholomance_Mop.hpp"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

// Mists redesigned Scholomance into its own instance with a new map id
// (MAP_SCHOLOMANCE_MOP, 1007) - unrelated to the Classic instance registered on
// MAP_SCHOLOMANCE (289) by SetupScholomance(), which stays untouched.
class ScholomanceMopInstanceScript : public InstanceScript
{
public:
    explicit ScholomanceMopInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new ScholomanceMopInstanceScript(pMapMgr); }
};

void SetupScholomanceMop(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SCHOLOMANCE_MOP, &ScholomanceMopInstanceScript::Create);
}
