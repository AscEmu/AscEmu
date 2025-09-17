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

std::size_t length(const utf8_string& utf8_string);
bool is_valid(const utf8_string& utf8_string);
bool is_valid(const char* utf8_string, std::size_t byte_length);

} // utf8, Util, AscEmu
