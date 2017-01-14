/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Log.hpp"
#include "Util.hpp"

#include <iostream>

namespace AELog
{
    std::string GetFormattedFileName(std::string path_prefix, std::string file_prefix, bool use_date_time)
    {
        std::string full_name;

        std::string basic_path_name = path_prefix + "/" + file_prefix;

        if (use_date_time)
        {
            std::string current_date_time = Util::GetCurrentDateTimeString();
            //replace time seperator with valid character for file name
            std::replace(current_date_time.begin(), current_date_time.end(), ':', '-');

            full_name = basic_path_name + "-" + current_date_time + ".log";
        }
        else
        {
            full_name = basic_path_name + ".log";
        }

        return full_name;
    }

#ifndef _WIN32
    const char* GetColorForDebugFlag(LogFlags log_flags)
    {
        switch (log_flags)
        {
            case LF_MAP:
            case LF_MAP_CELL:
            case LF_VMAP:
            case LF_MMAP:
                return CONSOLE_COLOR_BLUE;
            case LF_OPCODE:
                return CONSOLE_COLOR_WHITE;
            case LF_SPELL:
            case LF_AURA:
            case LF_SPELL_EFF:
            case LF_AURA_EFF:
                return CONSOLE_COLOR_PURPLE;
            case LF_SCRIPT_MGR:
            case LF_DB_TABLES:
                return CONSOLE_COLOR_YELLOW;
            default:
                return CONSOLE_COLOR_NORMAL;
}
}
#else
    int GetColorForDebugFlag(LogFlags log_flags)
    {
        switch (log_flags)
        {
            case LF_MAP:
            case LF_MAP_CELL:
            case LF_VMAP:
            case LF_MMAP:
                return CONSOLE_COLOR_BLUE;
            case LF_OPCODE:
                return CONSOLE_COLOR_WHITE;
            case LF_SPELL:
            case LF_AURA:
            case LF_SPELL_EFF:
            case LF_AURA_EFF:
                return CONSOLE_COLOR_PURPLE;
            case LF_SCRIPT_MGR:
            case LF_DB_TABLES:
                return CONSOLE_COLOR_YELLOW;
            default:
                return CONSOLE_COLOR_NORMAL;
        }
    }
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// AscEmu oLog functions

void oLog::InitalizeLogFiles(std::string file_prefix)
{
#ifdef _WIN32
    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

    std::string normal_filename = file_prefix + "-normal.log";
    std::string error_filename = file_prefix + "-error.log";

    std::string current_date_time = Util::GetCurrentDateTimeString();

    m_normalFile = fopen(normal_filename.c_str(), "a");
    if (m_normalFile == nullptr)
        fprintf(stderr, "%s: Error opening '%s': %s\n", __FUNCTION__, normal_filename, strerror(errno));
    else
        outBasic("=================[%s]=================", current_date_time.c_str());

    m_errorFile = fopen(error_filename.c_str(), "a");
    if (m_errorFile == nullptr)
        fprintf(stderr, "%s: Error opening '%s': %s\n", __FUNCTION__, error_filename, strerror(errno));
    else
        outErrorSilent("=================[%s]=================", current_date_time.c_str());
}

void oLog::DebugFlag(LogFlags log_flags, const char* format, ...)
{
    if (m_fileLogLevel < LOG_LEVEL_DEBUG || m_errorFile == NULL)
        return;

    if (!(mDebugFlags & log_flags))
        return;

    char buf[32768];
    va_list ap;

    va_start(ap, format);
    vsnprintf(buf, 32768, format, ap);
    va_end(ap);
    SetConsoleColor(AELog::GetColorForDebugFlag(log_flags));
    std::cout << buf << std::endl;
    SetConsoleColor(CONSOLE_COLOR_NORMAL);
    outFile(m_errorFile, buf);
}

#ifndef _WIN32
void oLog::SetConsoleColor(const char* color)
{
    fputs(color, stdout);
}

#else
void oLog::SetConsoleColor(int color)
{
    SetConsoleTextAttribute(stdout_handle, (WORD)color);
}
#endif
