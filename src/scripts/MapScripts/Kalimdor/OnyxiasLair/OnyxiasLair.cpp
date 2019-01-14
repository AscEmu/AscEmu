/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "OnyxiasLair.h"


class OnyxiasLair : public InstanceScript
{
public:

    explicit OnyxiasLair(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new OnyxiasLair(pMapMgr); }
};


void OnyxiasLairScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(249, &OnyxiasLair::Create);
}

