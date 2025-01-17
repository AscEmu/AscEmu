/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace Ascemu
{
    class PerformanceCounter
    {
    public:
        PerformanceCounter();
        ~PerformanceCounter() = default;

        float GetCurrentCPUUsage();

        float GetCurrentRAMUsage();

    private:
        unsigned long cpu_count;            // The number of CPUs in this system  ( CPU cores count as separate CPUs )
        unsigned long long last_update;     // The time the last sample was created
        unsigned long long last_cpu_usage;  // The last sample of CPU usage
    };

}
