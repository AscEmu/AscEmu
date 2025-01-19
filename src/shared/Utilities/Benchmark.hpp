/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <chrono>

namespace Util
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Benchmark
    class BenchmarkTime
    {
    public:
        BenchmarkTime(std::string function = "");
        ~BenchmarkTime();

        void Stop();

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
        std::string functionName;
    };
}
