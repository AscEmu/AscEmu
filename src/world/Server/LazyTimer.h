/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <atomic>

class LazyTimer
{
    enum class TimerState
    {
        Started,
        Stopped,
        Expired
    };
    std::atomic<long long> m_elapsedTime;
    std::atomic<long long> m_startedAt;
    std::atomic<long long> m_duration;

    std::atomic<bool> m_expired;
    std::atomic<TimerState> m_state;

    long long getDelta() const;
public:
    void start();
    void stop();

    // Update the timer and return true if expired
    void evaluate();

    long long getDuration() const;
    // Reset the timer
    void reset(long long durationMs = -1);
    void resetAndRestart(long long durationMs = -1);

    // Has the timer expired
    bool isExpired() const;

    LazyTimer(long long duration);

    // Just for testing - will be removed
    long long getRealDelta();
};