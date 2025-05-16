/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"
#include "Utilities/Util.hpp"

#include <array>
#include <cstddef>

#define getOffsetForStructuredField(s,m) static_cast<uint32_t>(offsetof(s,m) / 4)

template <typename T>
concept isStdArray = Util::is_size_based_specialization_of_v<T, std::array>;

// Works for both std::array and C style arrays
#define getOffsetForStructuredArrayField(s,m,index) \
    (isStdArray<decltype(s::m)> || std::is_array_v<decltype(s::m)> ? \
        static_cast<uint32_t>((offsetof(s,m) + sizeof(s::m[0]) * index) / 4) : \
        getOffsetForStructuredField(s,m))

#define getSizeOfStructure(s) static_cast<uint32_t>(sizeof(s) / 4)

#define getSizeOfStructuredField(s,m)
