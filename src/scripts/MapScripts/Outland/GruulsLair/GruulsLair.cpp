/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "GruulsLair.h"

class GruulsLair : public InstanceScript
{
public:

    GruulsLair(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new GruulsLair(pMapMgr); }
};


void GruulsLairScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(565, &GruulsLair::Create);
}
