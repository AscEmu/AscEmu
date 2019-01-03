/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Ahnkahet.h"

class Ahnkahet : public InstanceScript
{
public:

    explicit Ahnkahet(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Ahnkahet(pMapMgr); }
};


void AhnkahetScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(619, &Ahnkahet::Create);
}
