/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/Script/CreatureAIScript.h"

class ZulGurubInstanceScript : public InstanceScript
{
public:

    explicit ZulGurubInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ZulGurubInstanceScript(pMapMgr); }
};

void SetupZulGurub(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ZUL_GURUB, &ZulGurubInstanceScript::Create);
}
