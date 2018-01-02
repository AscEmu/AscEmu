/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "TrialOfTheCrusader.h"

class TrialOfTheCrusader : public InstanceScript
{
public:

    TrialOfTheCrusader(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new TrialOfTheCrusader(pMapMgr); }
};


void TrialOfTheCrusaderScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(649, &TrialOfTheCrusader::Create);
}
