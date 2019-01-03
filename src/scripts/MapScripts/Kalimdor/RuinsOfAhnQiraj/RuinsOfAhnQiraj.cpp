/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "RuinsOfAhnQiraj.h"


class RuinsOfAhnQiraj : public InstanceScript
{
public:

    explicit RuinsOfAhnQiraj(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new RuinsOfAhnQiraj(pMapMgr); }
};


void RuinsOfAhnQirajScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(509, &RuinsOfAhnQiraj::Create);
}

