/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "SerpentShrine.h"

class SerpentShrine : public InstanceScript
{
public:

    SerpentShrine(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new SerpentShrine(pMapMgr); }
};

#ifdef UseNewMapScriptsProject
void SerpentShrineScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(548, &SerpentShrine::Create);
}
#endif
