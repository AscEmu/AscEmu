/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"

class ConsoleThread : public ThreadBase
{
    public:

        bool runThread();
        void stopThread();

    protected:

        bool mStopConsoleThread;
        bool mIsConsoleThreadRunning;
};
