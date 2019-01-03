/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

//\brief: no WorldBosses in WotLK and old script was too horrible and wrong to keep it as part of AE.
void SetupWorldBosses(ScriptMgr* mgr)
{
    if (mgr)
        return;
}
