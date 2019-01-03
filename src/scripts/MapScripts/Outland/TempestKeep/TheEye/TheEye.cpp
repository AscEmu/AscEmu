/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "TheEye.h"


class TheEye : public InstanceScript
{
public:

    explicit TheEye(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TheEye(pMapMgr); }
};


void TheEyeScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(550, &TheEye::Create);
}
