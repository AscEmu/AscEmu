/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace Ascemu
{
    class SysInfo
    {
    public:
        static long GetCPUCount();

        static unsigned long long GetCPUUsage();

        static unsigned long long GetRAMUsage();

        static unsigned long long GetTickCount();

    };
}
