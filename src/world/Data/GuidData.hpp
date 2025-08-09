/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#pragma pack(push, 1)
union guid_union
{
    uint64_t guid;
    struct parts
    {
        uint32_t low;
        uint32_t high;
    } parts;
};
#pragma pack(pop)
