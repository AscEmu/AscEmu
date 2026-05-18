/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace AscEmu
{
    class SERVER_DECL PerformanceCounter
    {
    public:
        PerformanceCounter();
        ~PerformanceCounter() = default;

        [[nodiscard]] float getCurrentCPUUsage();
        [[nodiscard]] float getCurrentRAMUsage() const;

    private:
        unsigned long cpu_count{};              // the number of CPUs in this system  ( CPU cores count as separate CPUs )
        unsigned long long last_update{};       // the time the last sample was created
        unsigned long long last_cpu_usage{};    // the last sample of CPU usage
    };
}
