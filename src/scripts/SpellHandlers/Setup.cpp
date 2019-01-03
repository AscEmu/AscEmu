/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/Script/ScriptSetup.h"
#include "Server/Script/ScriptMgr.h"

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
    SetupShamanSpells(mgr);
    SetupWarlockSpells(mgr);
    SetupWarriorSpells(mgr);
    SetupHunterSpells(mgr);
    SetupItemSpells_1(mgr);
    SetupQuestItems(mgr); //this was commented for crash reason, let see what are those...
    SetupMageSpells(mgr);
    SetupPaladinSpells(mgr);
    SetupRogueSpells(mgr);
    SetupPriestSpells(mgr);
    SetupPetAISpells(mgr);
    SetupDruidSpells(mgr);
    SetupDeathKnightSpells(mgr);
    SetupMiscSpellhandlers(mgr);
}

#ifdef WIN32
BOOL APIENTRY DllMain(HANDLE /*hModule*/, DWORD  /*ul_reason_for_call*/, LPVOID /*lpReserved*/)
{
    return TRUE;
}
#endif
