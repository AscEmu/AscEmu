/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_The_Stonecore.h"
#include <Units/Creatures/Pet.h>

class TheStonecoreInstanceScript : public InstanceScript
{
public:
    explicit TheStonecoreInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new TheStonecoreInstanceScript(pMapMgr); }
};

void SetupTheStonecore(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_STONECORE, &TheStonecoreInstanceScript::Create);
}
