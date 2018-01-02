/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef LOG_DEFINES_HPP
#define LOG_DEFINES_HPP

#include "Common.hpp"

#ifdef _WIN32
    #define CONSOLE_COLOR_RED       FOREGROUND_INTENSITY | FOREGROUND_RED
    #define CONSOLE_COLOR_GREEN     FOREGROUND_INTENSITY | FOREGROUND_GREEN
    #define CONSOLE_COLOR_YELLOW    FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN
    #define CONSOLE_COLOR_NORMAL    FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE
    #define CONSOLE_COLOR_WHITE     FOREGROUND_INTENSITY | CONSOLE_COLOR_NORMAL 
    #define CONSOLE_COLOR_BLUE      FOREGROUND_INTENSITY | FOREGROUND_BLUE
    #define CONSOLE_COLOR_CYAN      FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE
    #define CONSOLE_COLOR_PURPLE    FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_RED
#else
    #define CONSOLE_COLOR_RED       "\033[0;31m"
    #define CONSOLE_COLOR_GREEN     "\033[0;32m"
    #define CONSOLE_COLOR_YELLOW    "\033[1;33m"
    #define CONSOLE_COLOR_NORMAL    "\033[0m"
    #define CONSOLE_COLOR_WHITE     "\033[1;37m"
    #define CONSOLE_COLOR_BLUE      "\033[0;34m"
    #define CONSOLE_COLOR_CYAN      "\033[1;36m"
    #define CONSOLE_COLOR_PURPLE    "\033[0;35m"
#endif

enum LogFlags
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

enum LogLevel
{
    LL_NORMAL    = 0,
    LL_DETAIL    = 1,
    LL_DEBUG     = 2
};

#endif  // LOG_DEFINES_HPP
