/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/Script/ScriptSetup.h"

extern "C" SCRIPT_DECL void _exp_set_serverstate_singleton(ServerState* state)
{
    ServerState::instance(state);
}

extern "C" SCRIPT_DECL uint32 _exp_get_script_type()
{
    return SCRIPT_TYPE_MISC;
}

extern "C" SCRIPT_DECL void _exp_script_register(ScriptMgr* mgr)
{
#if VERSION_STRING >= WotLK
    setupDeathKnightSpells(mgr);
#endif
    setupDruidSpells(mgr);
    setupHunterSpells(mgr);
    setupMageSpells(mgr);
#if VERSION_STRING >= Mop
    setupMonkSpells(mgr);
#endif
    setupPaladinSpells(mgr);
    setupPriestSpells(mgr);
    setupRogueSpells(mgr);
    setupShamanSpells(mgr);
    setupWarlockSpells(mgr);
    setupWarriorSpells(mgr);

    setupItemSpells(mgr);
    setupMiscSpells(mgr);
    setupPetSpells(mgr);
    setupQuestSpells(mgr);
}

#ifdef WIN32
BOOL APIENTRY DllMain(HANDLE /*hModule*/, DWORD  /*ul_reason_for_call*/, LPVOID /*lpReserved*/)
{
    return TRUE;
}
#endif
