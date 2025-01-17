/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Well_Of_Eternity.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class WellOfEternityInstanceScript : public InstanceScript
{
public:
    explicit WellOfEternityInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new WellOfEternityInstanceScript(pMapMgr); }
};

void SetupWellOfEternity(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_WELL_OF_ETERNITY, &WellOfEternityInstanceScript::Create);
}
