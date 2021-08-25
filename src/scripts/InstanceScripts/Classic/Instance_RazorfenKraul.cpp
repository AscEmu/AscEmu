/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_RazorfenKraul.h"
#include <Units/Creatures/Pet.h>

class RazorfenKraulInstanceScript : public InstanceScript
{
public:

    explicit RazorfenKraulInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new RazorfenKraulInstanceScript(pMapMgr); }
};

void SetupRazorfenKraul(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_RAZORFEN_KRAUL, &RazorfenKraulInstanceScript::Create);
}
