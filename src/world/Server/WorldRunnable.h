/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Threading/AEThread.h"

class WorldRunnable
{
    std::unique_ptr<AscEmu::Threading::AEThread> m_thread;
    void threadRunner(AscEmu::Threading::AEThread& thread);
    void threadInit();

public:
    WorldRunnable();
    ~WorldRunnable() = default;

    void threadShutdown();

private:
    uint32_t m_lastWorldUpdate = 0;
    uint32_t m_lastSessionsUpdate = 0;
};
