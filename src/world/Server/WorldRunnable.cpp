/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Utilities/Util.hpp"
#include "Debugging/CrashHandler.h"
#include "WorldRunnable.h"
#include "World.h"
#include "ServerState.h"

using AscEmu::Threading::AEThread;
using std::chrono::milliseconds;
using std::make_unique;

WorldRunnable::WorldRunnable()
{
    m_thread = make_unique<AEThread>("WorldRunnable", [this](AEThread& thread) { this->threadRunner(thread); }, milliseconds(50), false);
    this->threadInit();
}

void WorldRunnable::threadShutdown()
{
    m_thread->killAndJoin();
}

void WorldRunnable::threadRunner(AEThread& /*thread*/)
{
    ServerState::instance()->update();
    uint32_t diff;

    auto now = Util::getMSTime();
    if (now < m_lastWorldUpdate)
        diff = 50;
    else
        diff = now - m_lastWorldUpdate;

    sWorld.Update(diff);
    m_lastWorldUpdate = now;

    now = Util::getMSTime();
    if (now < m_lastSessionsUpdate)
        diff = 50;
    else
        diff = now - m_lastSessionsUpdate;

    sWorld.updateGlobalSession(diff);
    m_lastSessionsUpdate = now;
}

void WorldRunnable::threadInit()
{
    m_thread->reboot();
}
