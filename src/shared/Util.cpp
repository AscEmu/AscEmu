/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Util.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <random>

namespace Util
{

    //////////////////////////////////////////////////////////////////////////////////////////
    // String functions

    void StringToLowerCase(std::string& str)
    {
        // C4244
        //std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        for (std::size_t i = 0; i < str.length(); ++i)
            str[i] = static_cast<char>(::tolower(str[i]));
    }

    void StringToUpperCase(std::string& str)
    {
        // C4244
        //std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        for (std::size_t i = 0; i < str.length(); ++i)
            str[i] = static_cast<char>(::toupper(str[i]));
    }

    void CapitalizeString(std::string& str)
    {
        if (!str.empty())
        {
            str[0] = static_cast<char>(::toupper(str[0]));

            for (std::size_t i = 1; i < str.length(); ++i)
                str[i] = static_cast<char>(::tolower(str[i]));
        }
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

    bool findXinYString(std::string& x, std::string& y)
    {
        return y.find(x) != std::string::npos;
    }

    uint32_t getLanguagesIdFromString(std::string langstr)
    {
        if (langstr.compare("enGB") == 0 || langstr.compare("enUS") == 0)
            return 0;

        if (langstr.compare("koKR") == 0)
            return 1;

        if (langstr.compare("frFR") == 0)
            return 2;

        if (langstr.compare("deDE") == 0)
            return 3;

        if (langstr.compare("esES") == 0)
            return 4;

        if (langstr.compare("ruRU") == 0)
            return 5;

        return 0;
    }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Time calculation
    // \note typedef high_resolution_clock system_clock
    // for further information check out https://msdn.microsoft.com/en-us/library/hh874757.aspx

    std::chrono::high_resolution_clock::time_point TimeNow()
    {
        return std::chrono::high_resolution_clock::now();
    }

    uint32_t getMSTime()
    {
        static const std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

        return uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count());
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

    std::string GetCurrentDateTimeString()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

#ifndef _WIN32
        char buff[20];
        char string = strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&in_time_t));
        std::string str(buff);
        return str;
#else
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        return ss.str();
#endif
    }

    std::string GetCurrentTimeString()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

#ifndef _WIN32
        char buff[20];
        char string = strftime(buff, 20, "%H:%M:%S", localtime(&in_time_t));
        std::string str(buff);
        return str;
#else
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%X");
        return ss.str();
#endif
    }

    std::string GetDateTimeStringFromTimeStamp(uint32_t timestamp)
    {
        std::time_t raw_time = (std::time_t)timestamp;

#ifndef _WIN32
        char buff[20];
        char string = strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&raw_time));
        std::string str(buff);
        return str;
#else
        std::stringstream ss;
        ss << std::put_time(std::localtime(&raw_time), "%Y-%m-%d %X");
        return ss.str();
#endif
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

    uint32_t GetTimePeriodFromString(const char* str)
    {
        uint32_t multiplier;

        std::string time_str = str;

        std::istringstream read_time_var(time_str);
        uint32_t time_period = 0;
        std::string time_var;
        read_time_var >> time_period;
        read_time_var >> time_var;

        Util::StringToLowerCase(time_var);

        if (time_var.compare("y") == 0)
            multiplier = TIME_YEAR;
        else if (time_var.compare("m") == 0)
            multiplier = TIME_MONTH;
        else if (time_var.compare("d") == 0)
            multiplier = TIME_DAY;
        else if (time_var.compare("h") == 0)
            multiplier = TIME_HOUR;
        else
            multiplier = TIME_MINUTE;

        time_period = (multiplier * time_period);

        return time_period;
    }

    std::string ByteArrayToHexString(uint8_t const* bytes, uint32_t arrayLength, bool reverseArray)
    {
        int32_t initPos = 0;
        int32_t endPos = arrayLength;
        int8_t op = 1;

        if (reverseArray)
        {
            initPos = arrayLength - 1;
            endPos = -1;
            op = -1;
        }

        std::ostringstream ss;
        for (int32_t i = initPos; i != endPos; i += op)
        {
            char buffer[4];
            sprintf(buffer, "%02X", bytes[i]);
            ss << buffer;
        }

        return ss.str();
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Random number helper functions

    int getRandomInt(int end)
    {
        return getRandomUInt(0, end);
    }

    int getRandomInt(int start, int end)
    {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<int> dist(start, end);
        return dist(mt);
    }

    uint32_t getRandomUInt(uint32_t end)
    {
        return getRandomUInt(0, end);
    }

    uint32_t getRandomUInt(uint32_t start, uint32_t end)
    {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<uint32_t> dist(start, end);
        return dist(mt);
    }

    float getRandomFloat(float end)
    {
        return getRandomFloat(0.0f, end);
    }

    float getRandomFloat(float start, float end)
    {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_real_distribution<float> dist(start, end);
        return dist(mt);
    }

}
