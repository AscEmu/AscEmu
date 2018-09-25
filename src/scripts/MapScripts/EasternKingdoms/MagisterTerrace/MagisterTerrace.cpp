/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "MagisterTerrace.h"


class MagisterTerrace : public InstanceScript
{
public:

    explicit MagisterTerrace(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new MagisterTerrace(pMapMgr); }
};


void MagisterTerraceScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(585, &MagisterTerrace::Create);
}

