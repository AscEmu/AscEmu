/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "ShadowLabyrinth.h"


class ShadowLabyrinth : public InstanceScript
{
public:

    explicit ShadowLabyrinth(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new ShadowLabyrinth(pMapMgr); }
};


void ShadowLabyrinthScripts(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(555, &ShadowLabyrinth::Create);
}

