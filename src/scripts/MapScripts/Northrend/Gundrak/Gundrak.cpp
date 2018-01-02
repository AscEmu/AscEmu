/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

#ifdef UseNewMapScriptsProject
void GundrakScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(604, &Gundrak::Create);
}
#endif
