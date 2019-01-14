/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "BlackrockSpire.h"


class BlackrockSpire : public InstanceScript
{
public:

    explicit BlackrockSpire(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new BlackrockSpire(pMapMgr); }
};


void BlackrockSpireScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(229, &BlackrockSpire::Create);
}

