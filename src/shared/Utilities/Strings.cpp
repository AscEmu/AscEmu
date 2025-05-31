/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Strings.hpp"
#include <cstring>
#include <sstream>

namespace AscEmu::Util::Strings
{
    void toLowerCase(std::string& source)
    {
        // C4244
        //std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        for (std::size_t i = 0; i < source.length(); ++i)
            source[i] = static_cast<char>(::tolower(source[i]));
    }

    void toUpperCase(std::string& source)
    {
        // C4244
        //std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        for (std::size_t i = 0; i < source.length(); ++i)
            source[i] = static_cast<char>(::toupper(source[i]));
    }

    void capitalize(std::string& str)
    {
        if (!str.empty())
        {
            str[0] = static_cast<char>(::toupper(str[0]));

            for (std::size_t i = 1; i < str.length(); ++i)
                str[i] = static_cast<char>(::tolower(str[i]));
        }
    }

    std::vector<std::string> split(const std::string& source, const std::string& seperator)
    {
        std::vector<std::string> string_vector{};

        //\NOTE: somehow people think it is a good idea to use a separator as last char in a string
        //       just remove it from the string before processing single strings (shitty db saving and loading)
        std::string result = source;
        if (result[result.size()] == seperator[0])
            result = result.substr(0, result.size() - 1);

        std::stringstream string_stream(result);
        std::string isolated_string;

        std::vector<char> seperatorCharacter(seperator.c_str(), seperator.c_str() + seperator.size() + 1);

        while (std::getline(string_stream, isolated_string, seperatorCharacter[0]))
        {
            if (isolated_string.size() != 0)
                string_vector.push_back(isolated_string);
        }

        return string_vector;
    }

    bool contains(std::string const& string, std::string const& source)
    {
        return source.find(string) != std::string::npos;
    }

    bool isEqual(const char* lhs, const char* rhs)
    {
        return !std::strcmp(lhs, rhs);
    }

    bool isEqual(std::string lhs, const char* rhs)
    {
        return !std::strcmp(lhs.c_str(), rhs);
    }
}
