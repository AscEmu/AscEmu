/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

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
