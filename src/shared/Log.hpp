/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef LOG_HPP
#define LOG_HPP

#include "Log.Legacy.h"

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

namespace AELog
{
    /*! \brief Returns formatted file name based on input */
    std::string GetFormattedFileName(std::string path_prefix, std::string file_prefix, bool use_date_time);
}

#endif  // LOG_HPP
