/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "TheArcatraz.h"


class TheArcatraz : public InstanceScript
{
public:

    TheArcatraz(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TheArcatraz(pMapMgr); }
};


void TheArcatrazScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(552, &TheArcatraz::Create);
}

