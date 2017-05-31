/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once


#include "Threading/ConditionVariable.h"
#include "CThreads.h"

class BroadcastMgr : public CThread
{
    public:

        BroadcastMgr();
        ~BroadcastMgr();

        void sendBroadcast();

        bool runThread();
        void terminate();

    private:

        Arcemu::Threading::ConditionVariable condition;

        bool mIsRunning;
};
