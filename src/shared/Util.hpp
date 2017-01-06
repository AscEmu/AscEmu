/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef UTIL_HPP
#define UTIL_HPP

#include "Util.Legacy.h"
#include <chrono>

namespace Util
{
    /*! \brief Manipulates the string to lowercase */
    void StringToLowerCase(std::string& str);

    /*! \brief Manipulates the string to uppercase */
    void StringToUpperCase(std::string& str);

    /*! \brief Returns the current point in time */
    std::chrono::time_point<std::chrono::steady_clock> TimeNow();

    /*! \brief Returns the difference between start_time and now in milliseconds */
    long long GetTimeDifferenceToNow(std::chrono::time_point<std::chrono::steady_clock> start_time);

    /*! \brief Returns the difference between start_time and end_time in milliseconds */
    long long GetTimeDifference(std::chrono::time_point<std::chrono::steady_clock> start_time, std::chrono::time_point<std::chrono::steady_clock> end_time);

}

#endif  // UTIL_HPP
