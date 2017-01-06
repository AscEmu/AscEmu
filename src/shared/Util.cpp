/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Util.hpp"

namespace Util
{
    void StringToLowerCase(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

    void StringToUpperCase(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    }

    std::chrono::time_point<std::chrono::steady_clock> TimeNow()
    {
        return std::chrono::high_resolution_clock::now();
    }

    long long GetTimeDifferenceToNow(std::chrono::time_point<std::chrono::steady_clock> start_time)
    {
        std::chrono::duration<float> float_diff = std::chrono::high_resolution_clock::now() - start_time;
        std::chrono::milliseconds time_difference = std::chrono::duration_cast<std::chrono::milliseconds>(float_diff);
        return time_difference.count();
    }

    long long GetTimeDifference(std::chrono::time_point<std::chrono::steady_clock> start_time, std::chrono::time_point<std::chrono::steady_clock> end_time)
    {
        std::chrono::duration<float> float_diff = end_time - start_time;
        std::chrono::milliseconds time_difference = std::chrono::duration_cast<std::chrono::milliseconds>(float_diff);
        return time_difference.count();
    }
}

