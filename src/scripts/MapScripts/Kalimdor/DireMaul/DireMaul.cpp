/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "DireMaul.h"


class DireMaul : public InstanceScript
{
public:

    explicit DireMaul(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new DireMaul(pMapMgr); }
};


void DireMaulScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(429, &DireMaul::Create);
}

