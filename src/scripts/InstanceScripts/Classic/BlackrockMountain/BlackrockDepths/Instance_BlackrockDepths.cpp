/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_BlackrockDepths.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class BlackrockDepthsInstanceScript : public InstanceScript
{
public:
    explicit BlackrockDepthsInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new BlackrockDepthsInstanceScript(pMapMgr); }
};

void SetupBlackrockDepths(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLACKROCK_DEPTHS, &BlackrockDepthsInstanceScript::Create);
}
