/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "utf8.hpp"
#include "utf8.h" // lib
#include <locale>

namespace AscEmu::Util::utf8 {

utf8_string name_format(const utf8_string& string, const std::locale& locale)
{
    utf8_string formatted = string;
    const auto data_beg = formatted.data();
    const auto data_end = formatted.data() + formatted.size();
    auto it = ::utf8::iterator(data_beg, data_beg, data_end);
    auto beg = it;
    auto end = ::utf8::iterator(data_end, data_beg, data_end);

    while(it != end)
    {
        if(it == beg)
        {
            *it.base() = std::toupper(*it, locale);
        } else {
            *it.base() = std::tolower(*it, locale);
        }

        ++it;
    }

    return formatted;
}

bool is_alpha(const utf8_string& string, const std::locale& locale)
{
    const auto data_beg = string.data();
    const auto data_end = string.data() + string.size();
    auto it = ::utf8::iterator(data_beg, data_beg, data_end);
    auto end = ::utf8::iterator(data_end, data_beg, data_end);

    while(it != end)
    {
        if(!std::isalpha(*it, locale))
        {
            return false;
        }

        ++it;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Operates on codepoints
std::size_t max_consecutive(const utf8_string& string, const bool case_insensitive, const std::locale& locale)
{
    const auto data_beg = string.data();
    const auto data_end = string.data() + string.size();
    auto it = ::utf8::iterator(data_beg, data_beg, data_end);
    auto end = ::utf8::iterator(data_end, data_beg, data_end);

    std::size_t current_run = 0;
    std::size_t longest_run = 0;
    std::uint32_t last = 0;

    while (it != end)
    {
        const std::uint32_t current = case_insensitive? std::tolower(*it, locale) : *it;

        if(current == last)
        {
            ++current_run;
        } else {
            current_run = 1;
        }

        if(current_run > longest_run)
        {
            longest_run = current_run;
        }

        last = current;
        ++it;
    }

    return longest_run;
}

std::size_t length(const utf8_string& string)
{
    return ::utf8::distance(string.begin(), string.end());
}

bool is_valid(const utf8_string& string)
{
    return ::utf8::is_valid(string.begin(), string.end());
}

bool is_valid(const char* string, const std::size_t byte_length)
{
    return ::utf8::is_valid(string, string + byte_length);
}

} // utf8, Util, AscEmu
