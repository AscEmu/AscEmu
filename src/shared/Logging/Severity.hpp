/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace AscEmu::Logging
{
    enum Severity
    {
        NONE,
        INFO,
        WARNING,
        BLUE,
        PURPLE,
        YELLOW,
        CYAN,
        FAILURE,
        FATAL
    };

    enum DebugFlags
    {
        LF_NONE         = 0x000,
        LF_OPCODE       = 0x001,
        LF_MAP          = 0x002,
        LF_MAP_CELL     = 0x004,
        LF_VMAP         = 0x008,
        LF_MMAP         = 0x010,
        LF_SPELL        = 0x020,
        LF_AURA         = 0x040,
        LF_SPELL_EFF    = 0x080,
        LF_AURA_EFF     = 0x100,
        LF_SCRIPT_MGR   = 0x200,
        LF_DB_TABLES    = 0x400,

        LF_ALL          = 0x800 - 0x001
    };
}
