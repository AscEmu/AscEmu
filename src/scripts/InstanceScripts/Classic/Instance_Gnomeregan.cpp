/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Gnomeregan.h"

class GnomereganInstanceScript : public InstanceScript
{
public:
    explicit GnomereganInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr) {}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new GnomereganInstanceScript(pMapMgr); }
};

void SetupGnomeregan(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_GNOMEREGAN, &GnomereganInstanceScript::Create);
}
