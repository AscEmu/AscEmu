/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Blackrock_Caverns.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class BlackrockCavernsInstanceScript : public InstanceScript
{
public:
    explicit BlackrockCavernsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new BlackrockCavernsInstanceScript(pMapMgr); }
};

void SetupBlackrockCaverns(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLACKROCK_CAVERNS, &BlackrockCavernsInstanceScript::Create);
}
