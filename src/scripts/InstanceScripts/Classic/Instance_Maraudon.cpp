/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Maraudon.h"

#include "Server/Script/CreatureAIScript.h"
#include "Macros/ScriptMacros.hpp"

class MaraudonInstanceScript : public InstanceScript
{
public:

    explicit MaraudonInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new MaraudonInstanceScript(pMapMgr); }
};

void SetupMaraudon(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_MARAUDON, &MaraudonInstanceScript::Create);
}
