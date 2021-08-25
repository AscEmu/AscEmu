/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_DireMaul.h"

#include "Server/Script/CreatureAIScript.h"
#include "Macros/ScriptMacros.hpp"

class DireMaulInstanceScript : public InstanceScript
{
public:

    explicit DireMaulInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new DireMaulInstanceScript(pMapMgr); }
};


void SetupDireMaul(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_DIRE_MAUL, &DireMaulInstanceScript::Create);
}
