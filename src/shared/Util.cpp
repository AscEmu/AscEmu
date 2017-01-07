/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Util.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

namespace Util
{

    //////////////////////////////////////////////////////////////////////////////////////////
    // String manipulation

    void StringToLowerCase(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

    void StringToUpperCase(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    }

    std::vector<std::string> SplitStringBySeperator(const std::string& str_src, const std::string& str_sep)
    {
        std::vector<std::string> string_vector;
        std::stringstream string_stream(str_src);
        std::string isolated_string;

        std::vector<char> seperator(str_sep.c_str(), str_sep.c_str() + str_sep.size() + 1);

        while (std::getline(string_stream, isolated_string, seperator[0]))
            string_vector.push_back(isolated_string);

        return string_vector;
    }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Time calculation
    // \note typedef high_resolution_clock system_clock
    // for further information check out https://msdn.microsoft.com/en-us/library/hh874757.aspx

    std::chrono::high_resolution_clock::time_point TimeNow()
    {
        return std::chrono::high_resolution_clock::now();
    }

    long long GetTimeDifferenceToNow(std::chrono::high_resolution_clock::time_point start_time)
    {
        std::chrono::duration<float> float_diff = TimeNow() - start_time;
        std::chrono::milliseconds time_difference = std::chrono::duration_cast<std::chrono::milliseconds>(float_diff);
        return time_difference.count();
    }

    long long GetTimeDifference(std::chrono::high_resolution_clock::time_point start_time, std::chrono::high_resolution_clock::time_point end_time)
    {
        std::chrono::duration<float> float_diff = end_time - start_time;
        std::chrono::milliseconds time_difference = std::chrono::duration_cast<std::chrono::milliseconds>(float_diff);
        return time_difference.count();
    }

    std::string GetCurrentTimeString()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        return ss.str();
    }

    std::string GetTimeStringFromTimeStamp(uint32_t timestamp)
    {
        std::time_t raw_time = (std::time_t)timestamp;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&raw_time), "%Y-%m-%d %X");
        return ss.str();
    }

    std::string GetDateStringFromSeconds(uint32_t seconds)
    {
        uint32_t in_seconds = seconds;
        uint32_t in_minutes = 0;
        uint32_t in_hours = 0;
        uint32_t in_days = 0;
        uint32_t in_months = 0;
        uint32_t in_years = 0;

        in_years = (in_seconds / 60 / 60 / 24 / 30 / 12);
        in_months = (in_seconds / 60 / 60 / 24 / 30) % 12;
        in_days = (in_seconds / 60 / 60 / 24) % 30;
        in_hours = (in_seconds / 60 / 60) % 24;
        in_minutes = (in_seconds / 60) % 60;
        in_seconds = in_seconds % 60;

        std::stringstream date_time_stream;
        if (in_years)
            date_time_stream << in_years << " years, ";

        if (in_months)
            date_time_stream << in_months << " months, ";

        if (in_days)
            date_time_stream << in_days << " days, ";

        if (in_hours)
            date_time_stream << in_hours << " hours, ";

        if (in_minutes)
            date_time_stream << in_minutes << " minutes, ";

        if (in_seconds)
            date_time_stream << in_seconds << " seconds, ";

        return date_time_stream.str();
    }
}

