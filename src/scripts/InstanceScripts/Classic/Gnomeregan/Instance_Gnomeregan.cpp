/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Gnomeregan.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class GnomereganInstanceScript : public InstanceScript
{
public:
    explicit GnomereganInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new GnomereganInstanceScript(pMapMgr); }
};

void SetupGnomeregan(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_GNOMEREGAN, &GnomereganInstanceScript::Create);
}
