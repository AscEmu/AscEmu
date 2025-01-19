/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Stratholme.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class StratholmeInstanceScript : public InstanceScript
{
public:
    explicit StratholmeInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) { }
    static InstanceScript* Create(WorldMap* pMapMgr) { return new StratholmeInstanceScript(pMapMgr); }
};

void SetupStratholme(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_STRATHOLME, &StratholmeInstanceScript::Create);
}
