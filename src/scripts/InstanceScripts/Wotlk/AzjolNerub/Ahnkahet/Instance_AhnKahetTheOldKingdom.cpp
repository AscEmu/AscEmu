/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_AhnKahetTheOldKingdom.h"

#include "Server/Script/InstanceScript.h"

class AhnKahetTheOldKingdomInstanceScript : public InstanceScript
{
public:
    explicit AhnKahetTheOldKingdomInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new AhnKahetTheOldKingdomInstanceScript(pMapMgr); }
};

void SetupAhnKahetTheOldKingdom(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_AHN_KAHET, &AhnKahetTheOldKingdomInstanceScript::Create);
}
