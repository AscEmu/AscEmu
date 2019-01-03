/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Naxxramas.h"

class Naxxramas : public InstanceScript
{
public:

    explicit Naxxramas(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Naxxramas(pMapMgr); }
};

#ifdef UseNewMapScriptsProject
void NaxxramasScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(533, &Naxxramas::Create);
}
#endif
