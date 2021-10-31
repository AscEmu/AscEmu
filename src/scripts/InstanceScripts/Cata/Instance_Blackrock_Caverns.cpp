/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Blackrock_Caverns.h"
#include <Units/Creatures/Pet.h>

class BlackrockCavernsInstanceScript : public InstanceScript
{
public:
    explicit BlackrockCavernsInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new BlackrockCavernsInstanceScript(pMapMgr); }
};

void SetupBlackrockCaverns(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLACKROCK_CAVERNS, &BlackrockCavernsInstanceScript::Create);
}
