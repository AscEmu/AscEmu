/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "TempleOfAhnQiraj.h"


class TempleOfAhnQiraj : public InstanceScript
{
public:

    explicit TempleOfAhnQiraj(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TempleOfAhnQiraj(pMapMgr); }
};


void TempleOfAhnQirajScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(531, &TempleOfAhnQiraj::Create);
}
