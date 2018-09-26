/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "HellfireRamparts.h"

class HellfireRamparts : public InstanceScript
{
public:

    explicit HellfireRamparts(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new HellfireRamparts(pMapMgr); }
};


void HellfireRampartsScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(543, &HellfireRamparts::Create);
}
