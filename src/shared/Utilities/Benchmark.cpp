/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Benchmark.hpp"
#include <iostream>

namespace Util
{
    BenchmarkTime::BenchmarkTime(std::string function)
    {
        m_startTime = std::chrono::high_resolution_clock::now();
        functionName = std::move(function);
    }

    BenchmarkTime::~BenchmarkTime()
    {
        Stop();
    }

    void BenchmarkTime::Stop()
    {
        const auto endTime = std::chrono::high_resolution_clock::now();

        const auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTime).time_since_epoch().count();
        const auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

        const auto duration = end - start;
        const double ms = duration * 0.001;

        std::cout << "BenchmarkTime:" << (functionName.empty() ? "" : functionName) << duration << ": microseconds (" << ms << "ms)\n";
    }
}
