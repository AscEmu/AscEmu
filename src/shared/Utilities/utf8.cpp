/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "utf8.hpp"
#include "utf8.h" // lib
#include <cstddef>
#include <cstdint>

namespace AscEmu::Util::utf8 {

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
