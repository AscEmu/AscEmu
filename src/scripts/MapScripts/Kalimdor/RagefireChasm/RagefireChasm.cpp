/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "RagefireChasm.h"


class RagefireChasm : public InstanceScript
{
public:

    explicit RagefireChasm(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new RagefireChasm(pMapMgr); }
};


void RagefireChasmScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(389, &RagefireChasm::Create);
}

