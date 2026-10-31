/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"

#include "Server/ServerState.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Server/Script/ScriptSetup.hpp"

extern "C" SCRIPT_DECL void _exp_set_serverstate_singleton(ServerState* state)
{
    ServerState::instance(state);
}

extern "C" SCRIPT_DECL uint32_t _exp_get_script_type()
{
    return SCRIPT_TYPE_MISC;
}

extern "C" SCRIPT_DECL void _exp_script_register(ScriptMgr* mgr)
{
    setupDeathKnightSpells(mgr);
    setupDruidSpells(mgr);
    setupHunterSpells(mgr);
    setupMageSpells(mgr);
    setupMonkSpells(mgr);
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

#ifdef _WIN32
#include <Windows.h>
BOOL APIENTRY DllMain(HANDLE /*hModule*/, DWORD  /*ul_reason_for_call*/, LPVOID /*lpReserved*/)
{
    return TRUE;
}
#endif
