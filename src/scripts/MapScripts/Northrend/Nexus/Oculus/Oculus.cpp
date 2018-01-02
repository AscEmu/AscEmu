/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Oculus.h"

class Oculus : public InstanceScript
{
public:

    Oculus(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Oculus(pMapMgr); }
};


void OculusScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(578, &Oculus::Create);
}
