/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "WorldRunnable.h"
#include <CrashHandler.h>
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

WorldRunnable::~WorldRunnable()
{

}

void WorldRunnable::threadShutdown()
{
    m_thread->killAndJoin();
}

void WorldRunnable::threadRunner(AEThread& /*thread*/)
{
    uint32_t lastWorldUpdate = Util::getMSTime();
    uint32_t lastSessionsUpdate = Util::getMSTime();

    ServerState::instance()->update();

    uint32_t diff;
    uint32_t now = Util::getMSTime();

    if (now < lastWorldUpdate)
        diff = 50;
    else
        diff = now - lastWorldUpdate;

    sWorld.Update(diff);

    now = Util::getMSTime();

    if (now < lastSessionsUpdate)
        diff = 50;
    else
        diff = now - lastSessionsUpdate;

    sWorld.updateGlobalSession(diff);
}

void WorldRunnable::threadInit()
{
    m_thread->reboot();
}
