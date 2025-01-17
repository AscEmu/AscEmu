/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "PerformanceCounter.hpp"
#include "SysInfo.hpp"

namespace Ascemu
{
    PerformanceCounter::PerformanceCounter()
    {
        cpu_count = Ascemu::SysInfo::GetCPUCount();
        last_update = Ascemu::SysInfo::GetTickCount();
        last_cpu_usage = Ascemu::SysInfo::GetCPUUsage();
    }

    float PerformanceCounter::GetCurrentRAMUsage()
    {
        unsigned long long usage = Ascemu::SysInfo::GetRAMUsage();

        return static_cast< float >(usage / (1024.0 * 1024.0));
    }

    float PerformanceCounter::GetCurrentCPUUsage()
    {
        unsigned long long now = Ascemu::SysInfo::GetTickCount();
        unsigned long long now_cpu_usage = Ascemu::SysInfo::GetCPUUsage();
        unsigned long long cpu_usage = now_cpu_usage - last_cpu_usage; // micro seconds
        unsigned long long time_elapsed = now - last_update; // milli seconds

        // converting to micro seconds
        time_elapsed *= 1000;

        float cpu_usage_percent = static_cast< float >(double(cpu_usage) / double(time_elapsed));

        cpu_usage_percent *= 100;

        // If we have more than 1 CPUs/cores,
        // without dividing here we could get over 100%
        cpu_usage_percent /= cpu_count;

        last_update = now;
        last_cpu_usage = now_cpu_usage;

        return static_cast< float >(cpu_usage_percent);
    }
}
