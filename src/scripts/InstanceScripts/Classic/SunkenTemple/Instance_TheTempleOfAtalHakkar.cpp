/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_TheTempleOfAtalHakkar.h"

#include "Server/Script/InstanceScript.hpp"
#include "Server/Script/ScriptMgr.hpp"

class TheTempleOfAtalHakkarInstanceScript : public InstanceScript
{
public:
    explicit TheTempleOfAtalHakkarInstanceScript(WorldMap* pMapMgr) : InstanceScript(pMapMgr) { }
    static InstanceScript* Create(WorldMap* pMapMgr) { return new TheTempleOfAtalHakkarInstanceScript(pMapMgr); }
};

void SetupTheTempleOfAtalHakkar(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SUNKEN_TEMPLE, &TheTempleOfAtalHakkarInstanceScript::Create);
}
