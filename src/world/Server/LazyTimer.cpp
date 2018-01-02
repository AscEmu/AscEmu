/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LazyTimer.h"
#include "ServerState.h"

long long LazyTimer::getDelta() const
{
    return ServerState::instance()->getDelta() - (m_startedAt + m_elapsedTime);
}

void LazyTimer::start()
{
    if (m_state == TimerState::Expired)
    {
        // Timer is in reset state, so init elapsedTime
        m_elapsedTime = 0;
    }

    // Update state
    m_state = TimerState::Started;

    // Set startedAt to current delta
    m_startedAt = ServerState::instance()->getDelta();
    // ... and subtract the elapsedTime to account for pauses
    m_startedAt -= m_elapsedTime;

    // Finally call eval in case the timer has already expired (moves us to expired state immediately)
    evaluate();
}

void LazyTimer::stop()
{
    // Call eval first in case we need to expire the timer
    evaluate();

    if (m_state == TimerState::Expired)
    {
        // Cannot stop an expired timer
        return;
    }

    m_state = TimerState::Stopped;
}

void LazyTimer::evaluate()
{
    if (m_state == TimerState::Expired)
    {
        return;
    }

    // Update elapsed time
    m_elapsedTime += getDelta();

    if (m_elapsedTime >= m_duration)
    {
        m_state = TimerState::Expired;
    }
}

long long LazyTimer::getDuration() const
{
    return m_duration;
}

void LazyTimer::reset(long long durationMs /*= -1*/)
{
    if (durationMs == -1)
    {
        durationMs = getDuration();
    }

    m_state = TimerState::Stopped;
    m_elapsedTime = 0;
    m_duration = durationMs;
    m_startedAt = 0;
}

void LazyTimer::resetAndRestart(long long durationMs)
{
    reset(durationMs);
    start();
}

bool LazyTimer::isExpired() const
{
    return m_state == TimerState::Expired;
}

LazyTimer::LazyTimer(long long duration)
{
    reset(duration);
}

// Just for testing - will be removed
long long LazyTimer::getRealDelta()
{
    ServerState::instance()->update();
    return ServerState::instance()->getDelta();
}
