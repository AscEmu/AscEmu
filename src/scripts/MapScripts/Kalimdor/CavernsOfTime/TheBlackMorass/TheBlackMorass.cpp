/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "TheBlackMorass.h"


class TheBlackMorass : public InstanceScript
{
public:

    explicit TheBlackMorass(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TheBlackMorass(pMapMgr); }
};


void TheBlackMorassScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(269, &TheBlackMorass::Create);
}

