/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

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
