/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "AuchenaiCrypts.h"


class AuchenaiCrypts : public InstanceScript
{
public:

    explicit AuchenaiCrypts(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new AuchenaiCrypts(pMapMgr); }
};


void AuchenaiCryptsScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(558, &AuchenaiCrypts::Create);
}

