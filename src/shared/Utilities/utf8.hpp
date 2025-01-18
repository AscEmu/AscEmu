/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "utf8String.hpp"
#include <locale>
#include <string>
#include <cstddef>

namespace AscEmu::Util::utf8 {

utf8_string name_format(const utf8_string& string, const std::locale& locale);
bool is_alpha(const utf8_string& string, const std::locale& locale);
std::size_t max_consecutive(const utf8_string& string, bool case_insensitive = false, const std::locale& locale = std::locale());
std::size_t length(const utf8_string& utf8_string);
bool is_valid(const utf8_string& utf8_string);
bool is_valid(const char* utf8_string, std::size_t byte_length);

} // utf8, Util, AscEmu
