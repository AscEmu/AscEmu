/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "PerformanceCounter.hpp"
#include "SysInfo.hpp"

namespace AscEmu
{
    namespace
    {
        constexpr double bytesToMegabytes = 1024.0 * 1024.0;
        constexpr double millisecondsToMicroseconds = 1000.0;
    }

    PerformanceCounter::PerformanceCounter()
        : cpu_count(SysInfo::getCPUCount())
        , last_update(SysInfo::getTickCount())
        , last_cpu_usage(SysInfo::getCPUUsage())
    {
    }

    float PerformanceCounter::getCurrentRAMUsage() const
    {
        const auto usage = SysInfo::getRAMUsage();

        return static_cast<float>(usage / bytesToMegabytes);
    }

    float PerformanceCounter::getCurrentCPUUsage()
    {
        const auto now = SysInfo::getTickCount();
        const auto now_cpu_usage = SysInfo::getCPUUsage();

        const auto cpu_usage = now_cpu_usage - last_cpu_usage; // microseconds
        const auto elapsed_ms = now - last_update; // milliseconds

        last_update = now;
        last_cpu_usage = now_cpu_usage;

        if (elapsed_ms == 0 || cpu_count == 0)
            return 0.0f;

        const double elapsed_us =
            static_cast<double>(elapsed_ms) * millisecondsToMicroseconds;

        const double cpu_usage_percent =
            (static_cast<double>(cpu_usage) / elapsed_us) *
            100.0 /
            static_cast<double>(cpu_count);

        return static_cast<float>(cpu_usage_percent);
    }
}
