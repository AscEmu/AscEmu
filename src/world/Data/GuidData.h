/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#pragma pack(push, 1)
union
{
    uint64_t guid;
    struct
    {
        uint32_t low;
        uint32_t high;
    } parts;
} typedef guid_union;
#pragma pack(pop)
