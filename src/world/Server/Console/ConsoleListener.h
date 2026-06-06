/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#ifndef ASCEMU_USE_AE_NETWORK_THREADPOOL
    #include "Threading/LegacyThreadBase.h"
#else
    template <class T>
    class ListenSocket;

    class ConsoleSocket;
#endif

bool StartConsoleListener();
void CloseConsoleListener();

#ifdef _WIN32
    #ifdef ASCEMU_USE_AE_NETWORK_THREADPOOL
        ListenSocket<ConsoleSocket>* GetConsoleListener();
    #else
        // Returns the console listener thread
        ThreadBase* GetConsoleListener();
    #endif
#endif
