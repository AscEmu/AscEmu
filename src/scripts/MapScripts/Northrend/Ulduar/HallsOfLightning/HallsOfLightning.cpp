/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "HallsOfLightning.h"

class HallsOfLightning : public InstanceScript
{
public:

    explicit HallsOfLightning(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new HallsOfLightning(pMapMgr); }
};

#ifdef UseNewMapScriptsProject
void HallsOfLightningScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(602, &HallsOfLightning::Create);
}
#endif
