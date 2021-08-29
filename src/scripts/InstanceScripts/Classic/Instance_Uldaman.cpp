/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Uldaman.h"

#include "Server/Script/CreatureAIScript.h"
#include "Macros/ScriptMacros.hpp"

class UldamanInstanceScript : public InstanceScript
{
public:

    explicit UldamanInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new UldamanInstanceScript(pMapMgr); }
};

void SetupUldaman(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_ULDAMAN, &UldamanInstanceScript::Create);
}
