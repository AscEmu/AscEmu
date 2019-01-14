/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Ulduar.h"

class Ulduar : public InstanceScript
{
public:

    explicit Ulduar(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new Ulduar(pMapMgr); }
};


void UlduarScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(603, &Ulduar::Create);
}
