/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "CullingOfStratholm.h"


class CullingOfStratholm : public InstanceScript
{
public:

    CullingOfStratholm(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new CullingOfStratholm(pMapMgr); }
};


void CullingOfStratholmScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(595, &CullingOfStratholm::Create);
}

