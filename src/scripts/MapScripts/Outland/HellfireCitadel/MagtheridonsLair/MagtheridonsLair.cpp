/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "MagtheridonsLair.h"

class MagtheridonsLair : public InstanceScript
{
public:

    MagtheridonsLair(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new MagtheridonsLair(pMapMgr); }
};


void MagtheridonsLairScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(544, &MagtheridonsLair::Create);
}
