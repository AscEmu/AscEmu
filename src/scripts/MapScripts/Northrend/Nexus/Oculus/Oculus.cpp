/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Oculus.h"

class Oculus : public InstanceScript
{
public:

    explicit Oculus(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Oculus(pMapMgr); }
};


void OculusScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(578, &Oculus::Create);
}
