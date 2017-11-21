/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "HallsOfReflection.h"

class HallsOfReflection : public InstanceScript
{
public:

    HallsOfReflection(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new HallsOfReflection(pMapMgr); }
};

#ifdef UseNewMapScriptsProject
void HallsOfReflectionScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(668, &HallsOfReflection::Create);
}
#endif
