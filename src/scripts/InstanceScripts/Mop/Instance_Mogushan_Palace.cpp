/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Mogushan_Palace.hpp"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class MogushanPalaceInstanceScript : public InstanceScript
{
public:
    explicit MogushanPalaceInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new MogushanPalaceInstanceScript(pMapMgr); }
};

void SetupMogushanPalace(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_MOGUSHAN_PALACE, &MogushanPalaceInstanceScript::Create);
}
