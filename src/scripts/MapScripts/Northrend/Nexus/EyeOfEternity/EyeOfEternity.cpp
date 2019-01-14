/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "EyeOfEternity.h"

class EyeOfEternity : public InstanceScript
{
public:

    explicit EyeOfEternity(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new EyeOfEternity(pMapMgr); }
};


void EyeOfEternityScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(616, &EyeOfEternity::Create);
}
