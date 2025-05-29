/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <string>
#include <vector>

namespace AscEmu::Util::Strings
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // String functions

    /*! \brief Manipulates the string to lowercase */
    void toLowerCase(std::string& source);

    /*! \brief Manipulates the string to uppercase */
    void toUpperCase(std::string& source);

    /*! \brief Capitalize word (uppercase first char, lowercase rest) */
    void capitalize(std::string& source);

    /*! \brief Seperates string by seperator (one char) returns string vecotr */
    std::vector<std::string> split(const std::string& source, const std::string& seperator);

    /*! \brief Returns true if string is in source */
    bool contains(std::string const& string, std::string const& source);

    /*! \brief Returns true if both strings are equal (case sensitive) */
    bool isEqual(const char* lhs, const char* rhs);
    bool isEqual(std::string lhs, const char* rhs);
}
