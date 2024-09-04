/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <utility>
#include <filesystem>
#include <locale>

namespace Util
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // WoW String functions

    /*! \brief Returns wow specific language string to id*/
    uint8_t getLanguagesIdFromString(const std::string& langstr);

    /*! \brief Returns wow specific language id to string*/
    std::string getLanguagesStringFromId(uint8_t id);

    /*! \brief Returns an uint32_t from a string between start/endcharacter */
    uint32_t getNumberFromStringByRange(const std::string& string, int startCharacter, int endCharacter);

    //////////////////////////////////////////////////////////////////////////////////////////
    // utf8String functions

    std::size_t max_consecutive(std::string_view name, bool case_insensitive = false, const std::locale& locale = std::locale());

    //////////////////////////////////////////////////////////////////////////////////////////
    // Narrowing
    inline uint32_t MAKE_PAIR32(uint16_t l, uint16_t h)
    {
        return uint32_t(l | (uint32_t(h) << 16));
    }

    // Narrowing
    inline int32_t float2int32(float value)
    {
        return static_cast<int32_t>(std::round(value));
    }

    // Fastest Method of long2int32
    inline int32_t long2int32(const double value)
    {
        return static_cast<int32_t>(std::lround(value));
    }

    template <typename T>
    T int32abs(int value)
    {
        return static_cast<T>(std::abs(value));
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Narrowing std::string and cstring to specific types

    // General template for unsigned types
    template <typename T>
    T safeStringToUnsigned(const std::string& _str, bool _silencedError)
    {
        try
        {
            unsigned long long value = std::stoull(_str);  // Convert to unsigned long long
            if (value > std::numeric_limits<T>::max())
            {
                throw std::out_of_range("Overflow for type");
            }
            return static_cast<T>(value);
        }
        catch (const std::invalid_argument& e)
        {
            if (!_silencedError)
                std::cerr << "Invalid argument: " << e.what() << std::endl;
            return 0; // Return 0 or handle error as needed
        }
        catch (const std::out_of_range& e)
        {
            if (!_silencedError)
                std::cerr << "Overflow error: " << e.what() << std::endl;
            return std::numeric_limits<T>::max(); // Return max value for overflow
        }
    }

    // General template for signed types
    template <typename T>
    T safeStringToSigned(const std::string& _str)
    {
        long long value = std::stoll(_str);  // Convert to long long
        try
        {
            if (value > std::numeric_limits<T>::max() || value < std::numeric_limits<T>::min())
            {
                throw std::out_of_range("Overflow/underflow for type");
            }
            return static_cast<T>(value);
        }
        catch (const std::invalid_argument& e)
        {
            std::cerr << "Invalid argument: " << e.what() << std::endl;
            return 0; // Return 0 or handle error as needed
        }
        catch (const std::out_of_range& e)
        {
            std::cerr << "Overflow/underflow error: " << e.what() << std::endl;
            if (value < std::numeric_limits<T>::min())
            {
                return std::numeric_limits<T>::min(); // Handle underflow
            }
            return std::numeric_limits<T>::max(); // Handle overflow
        }
    }

    // Specialization for float with error handling
    template <>
    inline float safeStringToSigned<float>(const std::string& str)
    {
        try
        {
            return std::stof(str);  // Convert to float
        }
        catch (const std::invalid_argument& e)
        {
            std::cerr << "Invalid argument: " << e.what() << std::endl;
            return 0.0f;  // Return 0.0 for invalid input
        }
        catch (const std::out_of_range& e)
        {
            std::cerr << "Overflow error: " << e.what() << std::endl;
            return std::numeric_limits<float>::max();  // Return max float for overflow
        }
    }

    // Specialization for double with error handling
    template <>
    inline double safeStringToSigned<double>(const std::string& str)
    {
        try
        {
            return std::stod(str);  // Convert to double
        }
        catch (const std::invalid_argument& e)
        {
            std::cerr << "Invalid argument: " << e.what() << std::endl;
            return 0.0;  // Return 0.0 for invalid input
        }
        catch (const std::out_of_range& e)
        {
            std::cerr << "Overflow error: " << e.what() << std::endl;
            return std::numeric_limits<double>::max();  // Return max double for overflow
        }
    }

    // Specialization for bool (any value > 0 = true, <= 0 = false)
    template <>
    inline bool safeStringToSigned<bool>(const std::string& str)
    {
        try
        {
            int value = std::stoi(str);  // Convert to int
            return value > 0;  // true if greater than 0, false otherwise
        }
        catch (const std::invalid_argument& e)
        {
            std::cerr << "Invalid argument: " << e.what() << std::endl;
            return false;  // Return false for invalid input
        }
    }

    // Overload for char* input
    template <typename T>
    T safeStringToUnsigned(const char* _cstr, bool _silencedError)
    {
        // Check for empty C-string and return 0
        if (!_cstr || *_cstr == '\0')
        {
            if (!_silencedError)
                std::cerr << "Empty string provided. Returning 0." << std::endl;

            return 0; // Return 0 for empty strings
        }
        std::string str(_cstr);  // Convert C-string to std::string
        return safeStringToUnsigned<T>(str, _silencedError);
    }

    // Overload for char* input
    template <typename T>
    T safeStringToSigned(const char* _cstr)
    {
        if (!_cstr || *_cstr == '\0')
        {
            std::cerr << "Empty string provided. Returning 0." << std::endl;
        }
        std::string str(_cstr);  // Convert C-string to std::string
        return safeStringToSigned<T>(str);
    }

    // Convenience functions for each specific type
    // bool conversion
    bool stringToBool(const std::string& _str);
    bool stringToBool(const char* _cstr);

    // uint8_t conversion
    uint8_t stringToUint8(const std::string& _str, bool _silencedError);
    uint8_t stringToUint8(const char* _cstr, bool _silencedError);

    // int8_t conversion
    int8_t stringToInt8(const std::string& _str);
    int8_t stringToInt8(const char* _cstr);

    // uint16_t conversion
    uint16_t stringToUint16(const std::string& _str, bool _silencedError);
    uint16_t stringToUint16(const char* _cstr, bool _silencedError);

    // int16_t conversion
    int16_t stringToInt16(const std::string& _str);
    int16_t stringToInt16(const char* _cstr);

    // uint32_t conversion
    uint32_t stringToUint32(const std::string& _str, bool _silencedError);
    uint32_t stringToUint32(const char* _cstr, bool _silencedError);

    // int32_t conversion
    int32_t stringToInt32(const std::string& _str);
    int32_t stringToInt32(const char* _cstr);

    // uint64_t conversion
    uint64_t stringToUint64(const std::string& _str, bool _silencedError);
    uint64_t stringToUint64(const char* _cstr, bool _silencedError);

    // int64_t conversion
    int64_t stringToInt64(const std::string& _str);
    int64_t stringToInt64(const char* cstr);

    // float conversion
    float stringToFloat(const std::string& _str);
    float stringToFloat(const char* _cstr);

    // double conversion
    double stringToDouble(const std::string& _str);
    double stringToDouble(const char* _cstr);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Time calculation/formatting

    /*! \brief Returns the current point in time */
    std::chrono::high_resolution_clock::time_point TimeNow();

    /*! \ brief Returns TimeNow() as time_t*/
    time_t getTimeNow();

    /*! \ brief Returns TimeNow() as uint32_t*/
    uint32_t getMSTime();

    /*! \brief Returns the difference between start_time and now in milliseconds */
    long long GetTimeDifferenceToNow(const std::chrono::high_resolution_clock::time_point& start_time);

    /*! \brief Returns the difference between start_time and end_time in milliseconds */
    long long GetTimeDifference(const std::chrono::high_resolution_clock::time_point& start_time, const std::chrono::high_resolution_clock::time_point& end_time);

    /*! \brief Returns the current Date Time as string */
    std::string GetCurrentDateTimeString();

    /*! \brief Returns the current Time as string */
    std::string GetCurrentTimeString();

    /*! \brief Returns Date Time as string from timestamp */
    std::string GetDateTimeStringFromTimeStamp(uint32_t timestamp);

    /*! \brief Returns years months days hours minutes seconds as string from seconds value */
    std::string GetDateStringFromSeconds(uint32_t seconds);

    /*! \brief Returns calculated time based on (second) values e.g. 5h will return 5 * 60 * 60 */
    uint32_t GetTimePeriodFromString(const char* str);

    /*! \brief Returns generated time value for client packets */
    uint32_t getGameTime();

    time_t getLocalHourTimestamp(time_t time, uint8_t hour, bool onlyAfterTime = true);

    std::string ByteArrayToHexString(uint8_t const* bytes, uint32_t arrayLength, bool reverseArray = false);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Random number helper functions

    int getRandomInt(int end);
    int getRandomInt(int start, int end);

    uint32_t getRandomUInt(uint32_t end);
    uint32_t getRandomUInt(uint32_t start, uint32_t end);

    float getRandomFloat(float end);
    float getRandomFloat(float start, float end);

    // Gets random number from 1-100 and returns true if val is greater than the number
    bool checkChance(uint32_t val);
    // Gets random number from 1-100 and returns true if val is greater than the number
    bool checkChance(int32_t val);
    // Gets random number from 1-100 and returns true if val is greater than the number
    bool checkChance(float_t val);

    template <class T>
    inline T square(T x) { return x * x; }

    // Percentage calculation
    template <class T, class U>
    inline T calculatePct(T base, U pct)
    {
        return T(base * static_cast<float>(pct) / 100.0f);
    }

    template <class T, class U>
    inline T addPct(T& base, U pct)
    {
        return base += calculatePct(base, pct);
    }

    template <class T, class U>
    inline T applyPct(T& base, U pct)
    {
        return base = calculatePct(base, pct);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Container helper functions

    template<typename T>
    inline void randomShuffleVector(std::vector<T>* vector)
    {
        std::random_device rd;
        std::mt19937 mt(rd());

        std::shuffle(vector->begin(), vector->end(), mt);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // C++17 filesystem dependent functions

    /*! \brief Returns map of directory file names. */
    std::map<uint32_t, std::string> getDirectoryContent(const std::string& pathName, const std::string& specialSuffix = "", bool withPath = false);

    /*! \brief Reads the file into a string based on the given path. */
    std::string readFileIntoString(std::filesystem::path path);

    /*! \brief Returns the first 8 chars of the file name as major version. */
    uint32_t readMajorVersionFromString(const std::string& fileName);

    uint32_t readMinorVersionFromString(const std::string& fileName);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Benchmark
    class BenchmarkTime
    {
    public:
        BenchmarkTime(std::string function = "")
        {
            m_startTime = std::chrono::high_resolution_clock::now();
            functionName = std::move(function);
        }

        ~BenchmarkTime()
        {
            Stop();
        }

        void Stop()
        {
            const auto endTime = std::chrono::high_resolution_clock::now();

            const auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTime).time_since_epoch().count();
            const auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

            const auto duration = end - start;
            const double ms = duration * 0.001;

            std::cout << "BenchmarkTime:" << (functionName.empty() ? "" : functionName) << duration << ": microseconds (" << ms << "ms)\n";
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
        std::string functionName;
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    // Misc
    unsigned int makeIP(std::string_view _str);

    bool parseCIDRBan(uint32_t _ip, uint32_t _mask, uint32_t _maskBits);
}

struct SmallTimeTracker
{
    int32_t mExpireTime;

public:
    SmallTimeTracker(uint32_t expired = 0) : mExpireTime(static_cast<int32_t>(expired)) {}

    void updateTimer(uint32_t diffTime) { mExpireTime -= diffTime; }
    void resetInterval(uint32_t intervalTime) { mExpireTime = static_cast<int32_t>(intervalTime); }

    int32_t getExpireTime() const { return mExpireTime; }
    bool isTimePassed() const { return mExpireTime <= 0; }
};
