/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Util.hpp"

#include <array>

#include "CommonTime.hpp"
#include "Strings.hpp"
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>

namespace Util
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // WoW String functions

    uint8_t getLanguagesIdFromString(const std::string& langstr)
    {
        if (langstr.compare("enGB") == 0 || langstr.compare("enUS") == 0)
            return 0;

        if (langstr.compare("koKR") == 0)
            return 1;

        if (langstr.compare("frFR") == 0)
            return 2;

        if (langstr.compare("deDE") == 0)
            return 3;

        if (langstr.compare("zhCN") == 0)
            return 4;

        if (langstr.compare("zhTW") == 0)
            return 5;

        if (langstr.compare("esES") == 0)
            return 6;

        // TBC
        if (langstr.compare("esMX") == 0)
            return 7;

        if (langstr.compare("ruRU") == 0)
            return 8;

        // Cata
        if (langstr.compare("ptBR") == 0 || langstr.compare("ptPT") == 0)
            return 10;

        // Mop
        if (langstr.compare("itIT") == 0)
            return 11;

        return 0;
    }

    std::string getLanguagesStringFromId(uint8_t id)
    {
        switch (id)
        {
            case 1: return "koKR";
            case 2: return "frFR";
            case 3: return "deDE";
            case 4: return "zhCN";
            case 5: return "zhTW";
            case 6: return "esES";
            // TBC
            case 7: return "esMX";
            case 8: return "ruRU";
            // Cata
            case 10: return "ptBR"; // also ptPT
            // Mop
            case 11: return "itIT";

            default: return "enGB"; // also enUS
        }
    }

    uint32_t getNumberFromStringByRange(const std::string& string, int startCharacter, int endCharacter)
    {
        auto const stringVersion = string.substr(startCharacter, endCharacter);
        return std::stoul(stringVersion);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // utf8String functions
    std::size_t max_consecutive(std::string_view name, const bool case_insensitive, const std::locale& locale)
    {
        std::size_t current_run = 0;
        std::size_t longest_run = 0;
        char last = 0;

        for(auto c : name)
        {
            if(case_insensitive)
            {
                c = std::tolower(c, locale);
            }

            if(c == last)
            {
                ++current_run;
            } else 
            {
                current_run = 1;
            }

            if(current_run > longest_run)
            {
                longest_run = current_run;
            }

            last = c;
        }

        return longest_run;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Time calculation
    // \note typedef high_resolution_clock system_clock
    // for further information check out https://msdn.microsoft.com/en-us/library/hh874757.aspx

    std::chrono::high_resolution_clock::time_point TimeNow()
    {
        return std::chrono::high_resolution_clock::now();
    }

    time_t getTimeNow()
    {
        const auto now_c = std::chrono::system_clock::now();
        const auto now = std::chrono::system_clock::to_time_t(now_c);

        return now;
    }

    uint32_t getMSTime()
    {
        static const std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

        return uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count());
    }

    long long GetTimeDifferenceToNow(const std::chrono::high_resolution_clock::time_point& start_time)
    {
        std::chrono::duration<float> float_diff = TimeNow() - start_time;
        std::chrono::milliseconds time_difference = std::chrono::duration_cast<std::chrono::milliseconds>(float_diff);
        return time_difference.count();
    }

    long long GetTimeDifference(const std::chrono::high_resolution_clock::time_point& start_time, const std::chrono::high_resolution_clock::time_point& end_time)
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

        AscEmu::Util::Strings::toLowerCase(time_var);

        if (time_var.compare("y") == 0)
            multiplier = TimeVars::Year;
        else if (time_var.compare("m") == 0)
            multiplier = TimeVars::Month;
        else if (time_var.compare("d") == 0)
            multiplier = TimeVars::Day;
        else if (time_var.compare("h") == 0)
            multiplier = TimeVars::Hour;
        else
            multiplier = TimeVars::Minute;

        time_period = (multiplier * time_period);

        return time_period;
    }

    uint32_t getGameTime()
    {
        const auto now = std::chrono::system_clock::now();
        auto inTimeT = std::chrono::system_clock::to_time_t(now);

        const uint32_t currentYear = localtime(&inTimeT)->tm_year - 100;
        const uint32_t currentMonth = localtime(&inTimeT)->tm_mon;
        const uint32_t currentDayInMonth = localtime(&inTimeT)->tm_mday - 1;

        const uint32_t currentDayInWeek = localtime(&inTimeT)->tm_wday == 0 ? 6 : localtime(&inTimeT)->tm_wday - 1;

        const uint32_t currentHours = localtime(&inTimeT)->tm_hour;
        const uint32_t currentMinutes = localtime(&inTimeT)->tm_min;


        uint32_t gameTimeValue = currentMinutes << TimeShiftmask::Minute & TimeBitmask::Minute;
        gameTimeValue |= currentHours << TimeShiftmask::Hour & TimeBitmask::Hour;
        gameTimeValue |= currentDayInWeek << TimeShiftmask::Weekday & TimeBitmask::Weekday;
        gameTimeValue |= currentDayInMonth << TimeShiftmask::Day & TimeBitmask::Day;
        gameTimeValue |= currentMonth << TimeShiftmask::Month & TimeBitmask::Month;
        gameTimeValue |= currentYear << TimeShiftmask::Year & TimeBitmask::Year;

        return gameTimeValue;
    }

    time_t getLocalHourTimestamp(time_t time, uint8_t hour, bool onlyAfterTime)
    {
        const auto now = std::chrono::system_clock::now();
        const auto now_t = std::chrono::system_clock::to_time_t(now);

        auto date = std::localtime(&now_t);
        date->tm_hour = 0;
        date->tm_min = 0;
        date->tm_sec = 0;
        const auto midnightLocal = std::mktime(date);
        time_t hourLocal = midnightLocal + hour * 3600;

        if (onlyAfterTime && hourLocal <= time)
            hourLocal += 86400;

        return hourLocal;
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
    // C++17 filesystem dependent functions

    std::map<uint32_t, std::string> getDirectoryContent(const std::string& pathName, const std::string& specialSuffix, bool withPath)
    {
        std::map<uint32_t, std::string> directoryContentMap;

        uint32_t count = 0;
        for (auto& p : std::filesystem::recursive_directory_iterator(pathName))
        {
            const std::string filePathName = p.path().string();

            if (!specialSuffix.empty())
            {
                if (filePathName.size() >= specialSuffix.size() &&
                    filePathName.compare(filePathName.size() - specialSuffix.size(), specialSuffix.size(), specialSuffix) == 0)
                {
                    std::string fileName = filePathName;
                    if (!withPath)
                        fileName.erase(0, pathName.size());

                    directoryContentMap.emplace(std::pair<uint32_t, std::string>(count, fileName));
                    ++count;
                }
            }
            else
            {
                std::string fileName = filePathName;
                if (!withPath)
                    fileName.erase(0, pathName.size());

                directoryContentMap.emplace(std::pair<uint32_t, std::string>(count, fileName));
                ++count;
            }
        }

        return directoryContentMap;
    }

    std::string readFileIntoString(std::filesystem::path path)
    {
        std::ifstream fileStream{ path };

        auto const fileSize = static_cast<unsigned int>(std::filesystem::file_size(path));

        std::string fileString(fileSize, ' ');

        //\ brief: fileString[0] here. Replace it with fileString.data() as soon as compilers
        //         implemented C++17 completely (or at least the filesystem).
        fileStream.read(&fileString[0], fileSize);

        return fileString;
    }

    // Database update files only
    uint32_t readMajorVersionFromString(const std::string& fileName)
    {
        //         <-------->  0 - 8
        // example: 20180722-00_some_update_file.sql
        uint32_t const version = getNumberFromStringByRange(fileName, 0, 8);
        return version;
    }

    // Database update files only
    uint32_t readMinorVersionFromString(const std::string& fileName)
    {
        //                  <-->  9 - 11
        // example: 20180722-00_some_update_file.sql
        uint32_t const version = getNumberFromStringByRange(fileName, 9, 11);
        return version;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    unsigned int makeIP(std::string_view _str)
    {
        std::array<unsigned int, 4> bytes;
        char dot; // To capture the dots between the numbers

        std::stringstream ss{ std::string{_str} };  // Convert string_view to string for stringstream

        for (auto& byte : bytes)
        {
            if (!(ss >> byte) || byte > 255)
            {
                return 0;  // Return 0 if parsing fails or if any byte is out of range
            }
            if (&byte != &bytes.back())
            {
                if (!(ss >> dot) || dot != '.')
                {
                    return 0;  // Return 0 if dots are incorrectly placed
                }
            }
        }

        return (bytes[0]) | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    }

    bool parseCIDRBan(uint32_t _ip, uint32_t _mask, uint32_t _maskBits)
    {
        if (_maskBits > 32)
            return false;  // Sanity check

        // Define the leftover bits comparison table
        constexpr std::array<uint8_t, 9> leftover_bits_compare = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF };

        int full_bytes = _maskBits / 8;
        int leftover_bits = _maskBits % 8;

        // Convert IP and Mask to arrays of bytes
        auto ip_bytes = reinterpret_cast<uint8_t*>(&_ip);
        auto mask_bytes = reinterpret_cast<uint8_t*>(&_mask);

        // Compare full bytes
        if (full_bytes > 0)
        {
            if (std::memcmp(ip_bytes, mask_bytes, full_bytes) != 0)
                return false;
        }

        // Compare leftover bits
        if (leftover_bits > 0)
        {
            uint8_t mask = leftover_bits_compare[leftover_bits];
            if ((ip_bytes[full_bytes] & mask) != (mask_bytes[full_bytes] & mask))
                return false;
        }

        return true;  // All testable bits match
    }
}
