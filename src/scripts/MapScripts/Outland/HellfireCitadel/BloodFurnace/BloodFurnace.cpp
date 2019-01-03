/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "BloodFurnace.h"

class BloodFurnace : public InstanceScript
{
public:

    explicit BloodFurnace(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new BloodFurnace(pMapMgr); }
};


void BloodFurnaceScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(542, &BloodFurnace::Create);
}
