/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "TheUnderbog.h"

class TheUnderbog : public InstanceScript
{
public:

    explicit TheUnderbog(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TheUnderbog(pMapMgr); }
};


void TheUnderbogScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(546, &TheUnderbog::Create);
}
