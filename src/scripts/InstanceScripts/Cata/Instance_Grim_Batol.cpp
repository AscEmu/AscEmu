/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Grim_Batol.h"
#include <Units/Creatures/Pet.h>

class GrimBatolInstanceScript : public InstanceScript
{
public:
    explicit GrimBatolInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new GrimBatolInstanceScript(pMapMgr); }
};

void SetupGrimBatol(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_GRIM_BATOL, &GrimBatolInstanceScript::Create);
}
