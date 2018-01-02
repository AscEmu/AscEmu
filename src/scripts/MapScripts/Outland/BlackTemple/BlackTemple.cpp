/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "BlackTemple.h"

class BlackTemple : public InstanceScript
{
public:

    BlackTemple(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new BlackTemple(pMapMgr); }
};


#ifdef UseNewMapScriptsProject
void BlackTempleScript(ScriptMgr* scriptMgr)
{
    scriptMgr->register_instance_script(564, &BlackTemple::Create);
}
#endif
