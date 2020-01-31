/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

#if VERSION_STRING >= WotLK
void setupDeathKnightSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyDeathKnightSpells(mgr);
}
#endif
