/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "TimeTracker.hpp"

namespace Util
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // TimeTracker
    SmallTimeTracker::SmallTimeTracker(uint32_t expired) : mExpireTime(static_cast<int32_t>(expired)) {}

    void SmallTimeTracker::updateTimer(uint32_t diffTime) { mExpireTime -= diffTime; }
    void SmallTimeTracker::resetInterval(uint32_t intervalTime) { mExpireTime = static_cast<int32_t>(intervalTime); }

    int32_t SmallTimeTracker::getExpireTime() const { return mExpireTime; }
    bool SmallTimeTracker::isTimePassed() const { return mExpireTime <= 0; }

}
