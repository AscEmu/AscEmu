/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_RazorfenDowns.h"
#include "Server/Script/CreatureAIScript.h"

class RazorfenDownsInstanceScript : public InstanceScript
{
public:
    explicit RazorfenDownsInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new RazorfenDownsInstanceScript(pMapMgr); }
};

void SetupRazorfenDowns(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_RAZORFEN_DOWNS, &RazorfenDownsInstanceScript::Create);
}
