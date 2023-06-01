/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Scholomance.h"

class ScholomanceInstanceScript : public InstanceScript
{
public:
    explicit ScholomanceInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) { }
    static InstanceScript* Create(WorldMap* pMapMgr) { return new ScholomanceInstanceScript(pMapMgr); }
};

void SetupScholomance(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SCHOLOMANCE, &ScholomanceInstanceScript::Create);
}
