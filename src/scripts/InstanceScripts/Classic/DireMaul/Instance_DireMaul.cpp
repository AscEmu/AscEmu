/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_DireMaul.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class DireMaulInstanceScript : public InstanceScript
{
public:
    explicit DireMaulInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new DireMaulInstanceScript(pMapMgr); }
};

void SetupDireMaul(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_DIRE_MAUL, &DireMaulInstanceScript::Create);
}
