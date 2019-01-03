/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Gnomregan.h"


class Gnomregan : public InstanceScript
{
public:

    explicit Gnomregan(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Gnomregan(pMapMgr); }
};


void GnomreganScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(90, &Gnomregan::Create);
}

