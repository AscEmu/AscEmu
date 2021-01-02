/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_TheTempleOfAtalHakkar.h"
#include <Units/Creatures/Pet.h>

class TheTempleOfAtalHakkarInstanceScript : public InstanceScript
{
public:

    explicit TheTempleOfAtalHakkarInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TheTempleOfAtalHakkarInstanceScript(pMapMgr); }


};

void SetupTheTempleOfAtalHakkar(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_SUNKEN_TEMPLE, &TheTempleOfAtalHakkarInstanceScript::Create);
}
