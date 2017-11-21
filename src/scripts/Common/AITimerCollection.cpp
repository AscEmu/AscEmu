/*
*Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
*This file is released under the MIT license. See README-MIT for more information.
*/

#include "AITimerCollection.h"

using std::lock_guard;
using std::mutex;
using std::make_unique;

int AITimerCollection::unsafeCreateTimer(long long durationMs)
{
    // When we append to the vector, this will be the correct indexer for the timer
    int timerId = static_cast<int>(m_timers.size());

    auto timer = make_unique<LazyTimer>(durationMs);
    m_timers.push_back(move(timer));

    return timerId;
}

std::unique_ptr<LazyTimer>& AITimerCollection::unsafeGetTimer(int id)
{
    //ARCEMU_ASSERT(id > m_timers.size() || id < 0);

    return m_timers[id];
}

void AITimerCollection::unsafeStartAllTimers()
{
    for (auto& timer: m_timers)
    {
        timer->start();
    }
}

void AITimerCollection::unsafeStopAllTimers()
{
    for (auto& timer: m_timers)
    {
        timer->stop();
    }
}

void AITimerCollection::unsafeResetAllTimers()
{
    for (auto& timer: m_timers)
    {
        timer->reset();
    }
}

int AITimerCollection::createTimer(long long durationMs)
{
    lock_guard<mutex> lock(mtx);

    return unsafeCreateTimer(durationMs);
}

std::unique_ptr<LazyTimer>& AITimerCollection::getTimer(int id)
{
    lock_guard<mutex> lock(mtx);

    return unsafeGetTimer(id);
}

void AITimerCollection::startAllTimers()
{
    lock_guard<mutex> lock(mtx);

    unsafeStartAllTimers();
}

void AITimerCollection::stopAllTimers()
{
    lock_guard<mutex> lock(mtx);

    unsafeStopAllTimers();
}

void AITimerCollection::resetAllTimers()
{
    lock_guard<mutex> lock(mtx);
    unsafeResetAllTimers();
}
