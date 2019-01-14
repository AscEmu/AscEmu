/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Karazhan.h"


class Karazhan : public InstanceScript
{
public:

    explicit Karazhan(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Karazhan(pMapMgr); }
};


void KarazhanScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(532, &Karazhan::Create);
}

