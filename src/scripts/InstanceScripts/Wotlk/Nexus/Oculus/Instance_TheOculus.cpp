/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_TheOculus.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class TheOculusInstanceScript : public InstanceScript
{
public:
    explicit TheOculusInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheOculusInstanceScript(pMapMgr); }
};

void SetupTheOculus(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_THE_OCULUS, &TheOculusInstanceScript::Create);
}
