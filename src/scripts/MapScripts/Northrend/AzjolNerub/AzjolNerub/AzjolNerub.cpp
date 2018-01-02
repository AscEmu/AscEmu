/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "AzjolNerub.h"

class AzjolNerub : public InstanceScript
{
public:

    AzjolNerub(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new AzjolNerub(pMapMgr); }
};


void AzjolNerubScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(601, &AzjolNerub::Create);
}
