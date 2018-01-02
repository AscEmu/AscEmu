/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "MoltenCore.h"


class MoltenCore : public InstanceScript
{
public:

    MoltenCore(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new MoltenCore(pMapMgr); }
};


void MoltenCoreScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(409, &MoltenCore::Create);
}

