/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Stormstout_Brewery.hpp"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class StormstoutBreweryInstanceScript : public InstanceScript
{
public:
    explicit StormstoutBreweryInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new StormstoutBreweryInstanceScript(pMapMgr); }
};

void SetupStormstoutBrewery(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_STORMSTOUT_BREWERY, &StormstoutBreweryInstanceScript::Create);
}
