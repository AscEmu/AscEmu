/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

namespace Util
{
    struct SmallTimeTracker
    {
        int32_t mExpireTime;

    public:
        SmallTimeTracker(uint32_t expired = 0);
        ~SmallTimeTracker() = default;

        void updateTimer(uint32_t diffTime);
        void resetInterval(uint32_t intervalTime);

        int32_t getExpireTime() const;
        bool isTimePassed() const;
    };
}
