/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Uldaman.h"


class Uldaman : public InstanceScript
{
public:

    Uldaman(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Uldaman(pMapMgr); }
};


void UldamanScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(70, &Uldaman::Create);
}

