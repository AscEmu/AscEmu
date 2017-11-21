/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "PitOfSaron.h"

class PitOfSaron : public InstanceScript
{
public:

    PitOfSaron(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new PitOfSaron(pMapMgr); }
};

#ifdef UseNewMapScriptsProject
void PitOfSaronScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(658, &PitOfSaron::Create);
}
#endif
