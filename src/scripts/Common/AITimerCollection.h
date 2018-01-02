/*
*Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
*This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

#include "Server/LazyTimer.h"

class AITimerCollection
{
    std::mutex mtx;
    std::atomic<int> m_nextTimerId;
    std::vector<std::unique_ptr<LazyTimer>> m_timers;

    int unsafeCreateTimer(long long durationMs);
    std::unique_ptr<LazyTimer>& unsafeGetTimer(int id);
    void unsafeStartAllTimers();
    void unsafeStopAllTimers();
    void unsafeResetAllTimers();
public:
    int createTimer(long long durationMs);
    std::unique_ptr<LazyTimer>& getTimer(int id);
    void startAllTimers();
    void stopAllTimers();
    void resetAllTimers();
};
