/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <memory>
#include "Threading/AEThread.h"

class BroadcastMgr
{
    std::unique_ptr<AscEmu::Threading::AEThread> m_thread;
    void threadRunner(AscEmu::Threading::AEThread& thread);
    void threadInit();
    void sendBroadcast();
public:
    BroadcastMgr();
    ~BroadcastMgr();
};
