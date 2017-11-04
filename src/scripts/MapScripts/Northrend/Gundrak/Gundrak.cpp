/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Gundrak.h"

class Gundrak : public InstanceScript
{
public:

    Gundrak(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Gundrak(pMapMgr); }
};


void GundrakScripts(ScriptMgr* scriptMgr)
{
#ifdef UseNewMapScriptsProject
    scriptMgr->register_instance_script(604, &Gundrak::Create);
#endif
}
