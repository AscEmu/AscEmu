/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#ifdef ASCEMU_USE_AE_NETWORK_THREADPOOL
namespace AscEmu::Threading
{
    class AEThread;
}

class ConsoleThread
{
public:
    void run(AscEmu::Threading::AEThread& thread);
    void stopThread();

protected:
    bool mStopConsoleThread;
    bool mIsConsoleThreadRunning;
};
#else
#include "Threading/LegacyThreadBase.h"

class ConsoleThread : public ThreadBase
{
public:
    bool runThread();
    void stopThread();

protected:
    bool mStopConsoleThread;
    bool mIsConsoleThreadRunning;
};
#endif
