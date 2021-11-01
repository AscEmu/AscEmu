/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_Blackrock_Caverns.h"

class BlackrockCavernsInstanceScript : public InstanceScript
{
public:
    explicit BlackrockCavernsInstanceScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr){}
    static InstanceScript* Create(MapMgr* pMapMgr) { return new BlackrockCavernsInstanceScript(pMapMgr); }

    void OnLoad() override
    {
        // Load All Cells in Our Instance
        GetInstance()->updateAllCells(true);
    }
};

void SetupBlackrockCaverns(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLACKROCK_CAVERNS, &BlackrockCavernsInstanceScript::Create);
}
