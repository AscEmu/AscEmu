/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Uldaman.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class UldamanInstanceScript : public InstanceScript
{
public:
    explicit UldamanInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) { }
    static InstanceScript* Create(WorldMap* pMapMgr) { return new UldamanInstanceScript(pMapMgr); }
};

void SetupUldaman(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ULDAMAN, &UldamanInstanceScript::Create);
}
