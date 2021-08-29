/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Scholomance.h"
#include "Server/Script/CreatureAIScript.h"

class ScholomanceInstanceScript : public InstanceScript
{
public:

    explicit ScholomanceInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ScholomanceInstanceScript(pMapMgr); }
};


void SetupScholomance(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SCHOLOMANCE, &ScholomanceInstanceScript::Create);
}
