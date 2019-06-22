/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"
#include <chrono>
#include <iomanip>

//\ brief: C++17 filesystem. It is currently experimental.
//         On MSVC it is included wit <filesystem> (which includes <experimental/filesystem>
//         On GCC and Clang you have to include <experimental/filesystem> and set the
//         compilerflag =stdc++17 and link stdc++fs.
//         We use the namespace fs to simplify it. On GCC it is v1.
#if (WIN32 || _WIN64)
#include <filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem::v1;
#endif

namespace Util
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // String functions

    /*! \brief Manipulates the string to lowercase */
    void StringToLowerCase(std::string& str);

    /*! \brief Manipulates the string to uppercase */
    void StringToUpperCase(std::string& str);

    /*! \brief Capitalize word (uppercase first char, lowercase rest) */
    void CapitalizeString(std::string& str);

    /*! \brief Seperates string by seperator (one char) returns string vecotr */
    std::vector<std::string> SplitStringBySeperator(const std::string& str_src, const std::string& str_sep);

    /*! \brief Returns true if string x is in sttrin y */
    bool findXinYString(std::string& x, std::string& y);

    /*! \brief Returns wow specific language string to id*/
    uint32_t getLanguagesIdFromString(std::string langstr);

    /*! \brief Returns an uint32_t from a string between start/endcharacter */
    uint32_t getNumberFromStringByRange(std::string string, int startCharacter, int endCharacter);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Time calculation/formatting

    /*! \brief Returns the current point in time */
    std::chrono::high_resolution_clock::time_point TimeNow();

    /*! \ brief Returns TimeNow() as uint32_t*/
    uint32_t getMSTime();

    /*! \brief Returns the difference between start_time and now in milliseconds */
    long long GetTimeDifferenceToNow(std::chrono::high_resolution_clock::time_point start_time);

    /*! \brief Returns the difference between start_time and end_time in milliseconds */
    long long GetTimeDifference(std::chrono::high_resolution_clock::time_point start_time, std::chrono::high_resolution_clock::time_point end_time);

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

    std::string ByteArrayToHexString(uint8_t const* bytes, uint32_t arrayLength, bool reverseArray = false);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Random number helper functions

    int getRandomInt(int end);
    int getRandomInt(int start, int end);

    uint32_t getRandomUInt(uint32_t end);
    uint32_t getRandomUInt(uint32_t start, uint32_t end);

    float getRandomFloat(float end);
    float getRandomFloat(float start, float end);


    //////////////////////////////////////////////////////////////////////////////////////////
    // C++17 filesystem dependent functions
    /*! \brief Reads the file into a string based on the given path. */
    std::string readFileIntoString(fs::path path);

    /*! \brief Returns the first 8 chars of the file name as major version. */
    uint32_t readMajorVersionFromString(std::string fileName);

    uint32_t readMinorVersionFromString(std::string fileName);
}

struct SmallTimeTracker
{
    int32_t mExpireTime;

    public:

        SmallTimeTracker(uint32_t expired = 0) : mExpireTime(expired) {}

        void updateTimer(int32_t diffTime) { mExpireTime -= diffTime; }
        void resetInterval(uint32_t intervalTime) { mExpireTime = intervalTime; }

        int32_t getExpireTime() const { return mExpireTime; }
        bool isTimePassed() const { return mExpireTime <= 0; }
};
