/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "RazorfenKraul.h"


class RazorfenKraul : public InstanceScript
{
public:

    explicit RazorfenKraul(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new RazorfenKraul(pMapMgr); }
};


void RazorfenKraulScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(47, &RazorfenKraul::Create);
}

