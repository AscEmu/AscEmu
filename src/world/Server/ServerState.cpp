/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "ServerState.h"
#include <mutex>

using namespace std::chrono;
using std::mutex;
using std::unique_ptr;
using std::make_unique;

ServerState* ServerState::singletonPtr;

ServerState* ServerState::instance(ServerState* existingPtr)
{
    if (singletonPtr == nullptr)
    {
        singletonPtr = existingPtr ? existingPtr : new ServerState;
    }

    return singletonPtr;
}

long long ServerState::getDelta() const
{
    return m_delta;
}

void ServerState::update()
{
    m_currentTime = high_resolution_clock::now();

    // Precalculate delta
    auto delta = duration_cast<milliseconds>(m_currentTime - m_initTime);
    m_delta = delta.count();
}

ServerState::ServerState() : m_initTime(high_resolution_clock::now()), m_currentTime(m_initTime), m_delta(0)
{
}
