/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "BlackrockDepths.h"


class BlackrockDepths : public InstanceScript
{
public:

    explicit BlackrockDepths(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new BlackrockDepths(pMapMgr); }
};


void BlackrockDepthsScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(230, &BlackrockDepths::Create);
}

