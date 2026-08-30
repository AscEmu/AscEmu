/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

namespace cp
{
    enum class BinaryType : uint32_t
    {
        None   = 0,
        Pe32   = 0x0000014C,
        Pe64   = 0x00008664,
        Mach32 = 0xFEEDFACE,
        Mach64 = 0xFEEDFACF
    };
} // namespace cp
