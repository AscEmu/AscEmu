/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Raid_Blackwing_Descent.h"

class BlackwingDescentInstanceScript : public InstanceScript
{
public:
    explicit BlackwingDescentInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new BlackwingDescentInstanceScript(pMapMgr); }
};

void SetupBlackwingDescent(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLACKWING_DESCENT, &BlackwingDescentInstanceScript::Create);
}
