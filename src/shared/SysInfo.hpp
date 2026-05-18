/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

namespace AscEmu
{
    class SysInfo
    {
    public:
        [[nodiscard]] static std::uint32_t getCPUCount();
        [[nodiscard]] static std::uint64_t getCPUUsage();
        [[nodiscard]] static std::uint64_t getRAMUsage();
        [[nodiscard]] static std::uint64_t getTickCount();
    };
}
