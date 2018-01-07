/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"
#include <chrono>
#include <iomanip>

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

//////////////////////////////////////////////////////////////////////////////////////////
// Utf8 string functions

bool Utf8WStr(std::string utf8str, std::wstring& wstr);
bool WStrUtf8(std::wstring wstr, std::string& utf8str);

size_t utf8length(std::string& utf8str);

inline wchar_t wcharToUpper(wchar_t wchar)
{
    if(wchar >= L'a' && wchar <= L'z')
        return wchar_t(uint16(wchar)-0x0020);
    if(wchar == 0x00DF)
        return wchar_t(0x1E9E);
    if(wchar >= 0x00E0 && wchar <= 0x00F6)
        return wchar_t(uint16(wchar)-0x0020);
    if(wchar >= 0x00F8 && wchar <= 0x00FE)
        return wchar_t(uint16(wchar)-0x0020);
    if(wchar >= 0x0101 && wchar <= 0x012F)
    {
        if(wchar % 2 == 1)
            return wchar_t(uint16(wchar)-0x0001);
    }
    if(wchar >= 0x0430 && wchar <= 0x044F)
        return wchar_t(uint16(wchar)-0x0020);

    return wchar;
}

inline wchar_t wcharToLower(wchar_t wchar)
{
    if(wchar >= L'A' && wchar <= L'Z')
        return wchar_t(uint16(wchar)+0x0020);
    if(wchar >= 0x00C0 && wchar <= 0x00D6)
        return wchar_t(uint16(wchar)+0x0020);
    if(wchar >= 0x00D8 && wchar <= 0x00DF)
        return wchar_t(uint16(wchar)+0x0020);
    if(wchar >= 0x0100 && wchar <= 0x012E)
    {
        if(wchar % 2 == 0)
            return wchar_t(uint16(wchar)+0x0001);
    }
    if(wchar == 0x1E9E)
        return wchar_t(0x00DF);
    if(wchar >= 0x0410 && wchar <= 0x042F)
        return wchar_t(uint16(wchar)+0x0020);

    return wchar;
}

inline void wstrToUpper(std::wstring& str)
{
    std::transform( str.begin(), str.end(), str.begin(), wcharToUpper );
}

inline void wstrToLower(std::wstring& str)
{
    std::transform( str.begin(), str.end(), str.begin(), wcharToLower );
}
