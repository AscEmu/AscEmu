/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Server/Script/ScriptSetup.h"

#define SKIP_ALLOCATOR_SHARING 1

extern "C" {
    SCRIPT_DECL void _exp_set_serverstate_singleton(ServerState* serverState)
    {
        ServerState::instance(serverState);
    }

    SCRIPT_DECL uint32_t _exp_get_script_type()
    {
        return SCRIPT_TYPE_MISC;
    }

    SCRIPT_DECL void _exp_script_register(ScriptMgr* scriptMgr)
    {
        IcecrownCitadel(scriptMgr);
    }
}

#ifdef WIN32

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

#endif  //Win32
