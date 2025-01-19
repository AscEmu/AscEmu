/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_RazorfenKraul.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class RazorfenKraulInstanceScript : public InstanceScript
{
public:
    explicit RazorfenKraulInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new RazorfenKraulInstanceScript(pMapMgr); }
};

void SetupRazorfenKraul(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_RAZORFEN_KRAUL, &RazorfenKraulInstanceScript::Create);
}
