/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_DragonSoul.h"
#include <Units/Creatures/Pet.h>

class DragonSoulInstanceScript : public InstanceScript
{
public:
    explicit DragonSoulInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new DragonSoulInstanceScript(pMapMgr); }
};

void SetupDragonSoul(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_DRAGON_SOUL, &DragonSoulInstanceScript::Create);
}
