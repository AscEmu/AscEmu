/*
*Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
*This file is released under the MIT license. See README-MIT for more information.
*/

#include "ServerState.h"

using namespace std::chrono;
using std::mutex;
using std::unique_ptr;
using std::make_unique;

unique_ptr<ServerState> ServerState::singletonPtr;

unique_ptr<ServerState>& ServerState::instance()
{
    if (singletonPtr == nullptr)
    {
        singletonPtr = make_unique<ServerState>();
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

ServerState::ServerState() : m_initTime(high_resolution_clock::now()), m_currentTime(m_initTime)
{
}
