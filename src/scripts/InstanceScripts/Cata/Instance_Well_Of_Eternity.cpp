/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Well_Of_Eternity.h"
#include <Units/Creatures/Pet.h>

class WellOfEternityInstanceScript : public InstanceScript
{
public:
    explicit WellOfEternityInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new WellOfEternityInstanceScript(pMapMgr); }
};

void SetupWellOfEternity(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_WELL_OF_ETERNITY, &WellOfEternityInstanceScript::Create);
}
