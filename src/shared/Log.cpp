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
                return CONSOLE_COLOR_YELLOW;
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
                return CONSOLE_COLOR_YELLOW;
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
        fprintf(stderr, "%s: Error opening '%s': %s\n", __FUNCTION__, normal_filename.c_str(), strerror(errno));
    /*else
        outBasic("=================[%s]=================", current_date_time.c_str());*/

    m_errorFile = fopen(error_filename.c_str(), "a");
    if (m_errorFile == nullptr)
        fprintf(stderr, "%s: Error opening '%s': %s\n", __FUNCTION__, error_filename.c_str(), strerror(errno));
    /*else
        outErrorSilent("=================[%s]=================", current_date_time.c_str());*/
}


//////////////////////////////////////////////////////////////////////////////////////////
// AscEmuLog functions
createFileSingleton(AscEmuLog);

void AscEmuLog::InitalizeLogFiles(std::string file_prefix)
{
#ifdef _WIN32
    handle_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

    std::string normal_filename = file_prefix + "-normal.log";
    std::string error_filename = file_prefix + "-error.log";

    std::string current_date_time = Util::GetCurrentDateTimeString();

    normal_log_file = fopen(normal_filename.c_str(), "a");
    if (normal_log_file == nullptr)
        std::cerr << __FUNCTION__ << " : Error opening file " << normal_filename << std::endl;
    else
        ConsoleLogDefault(false, "=================[%s]=================", current_date_time.c_str());

    error_log_file = fopen(error_filename.c_str(), "a");
    if (error_log_file == nullptr)
        std::cerr << __FUNCTION__ << " : Error opening file " << normal_filename << std::endl;
    else
        ConsoleLogError(true, "=================[%s]=================", current_date_time.c_str());
}

void AscEmuLog::WriteFile(FILE* file, char* msg, const char* source)
{
    std::string current_time = "[" + Util::GetCurrentTimeString() + "] ";
    if (source != NULL)
        fprintf(file, "%s %s: %s\n", current_time.c_str(), source, msg);
    else
        fprintf(file, "%s %s\n", current_time.c_str(), msg);
}

#ifndef _WIN32
void AscEmuLog::SetConsoleColor(const char* color)
{
    fputs(color, stdout);
}

#else
void AscEmuLog::SetConsoleColor(int color)
{
    SetConsoleTextAttribute(handle_stdout, (WORD)color);
}
#endif

void AscEmuLog::SetFileLoggingLevel(uint32_t level)
{
    aelog_file_log_level = level;
}

void AscEmuLog::SetDebugFlags(uint32_t flags)
{
    aelog_debug_flags = flags;
}

// Log types
void AscEmuLog::ConsoleLogDefault(bool file_only, const char* format, ...)
{
    if (normal_log_file == nullptr)
        return;

    char message_buffer[32768];
    va_list ap;

    va_start(ap, format);
    vsnprintf(message_buffer, 32768, format, ap);
    va_end(ap);

    if (!file_only)
        std::cout << message_buffer << std::endl;

    WriteFile(normal_log_file, message_buffer);
}

void AscEmuLog::ConsoleLogDefaultFunction(bool file_only, const char* function, const char* format, ...)
{
    if (normal_log_file == nullptr)
        return;

    char function_message[32768];
    snprintf(function_message, 32768, "[BASIC] %s %s", function, format);

    char message_buffer[32768];
    va_list ap;

    va_start(ap, format);
    vsnprintf(message_buffer, 32768, function_message, ap);
    va_end(ap);

    if (!file_only)
    {
        SetConsoleColor(CONSOLE_COLOR_WHITE);
        std::cout << message_buffer << std::endl;
        SetConsoleColor(CONSOLE_COLOR_NORMAL);
    }

    WriteFile(normal_log_file, message_buffer);
}

void AscEmuLog::ConsoleLogError(bool file_only, const char* format, ...)
{
    if (error_log_file == nullptr)
        return;

    char message_buffer[32768];
    va_list ap;

    va_start(ap, format);
    vsnprintf(message_buffer, 32768, format, ap);
    va_end(ap);

    if (!file_only)
    {
        SetConsoleColor(CONSOLE_COLOR_RED);
        std::cerr << message_buffer << std::endl;
        SetConsoleColor(CONSOLE_COLOR_NORMAL);
    }

    WriteFile(error_log_file, message_buffer);
}

void AscEmuLog::ConsoleLogErrorFunction(bool file_only, const char* function, const char* format, ...)
{
    if (error_log_file == nullptr)
        return;

    char function_message[32768];
    snprintf(function_message, 32768, "[ERROR] %s %s", function, format);

    char message_buffer[32768];
    va_list ap;

    va_start(ap, format);
    vsnprintf(message_buffer, 32768, function_message, ap);
    va_end(ap);

    if (!file_only)
    {
        SetConsoleColor(CONSOLE_COLOR_RED);
        std::cout << message_buffer << std::endl;
        SetConsoleColor(CONSOLE_COLOR_NORMAL);
    }

    WriteFile(error_log_file, message_buffer);
}

void AscEmuLog::ConsoleLogDetail(bool file_only, const char* format, ...)
{
    if (aelog_file_log_level < LOG_LEVEL_DETAIL || normal_log_file == nullptr)
        return;

    char message_buffer[32768];
    va_list ap;

    va_start(ap, format);
    vsnprintf(message_buffer, 32768, format, ap);
    va_end(ap);

    if (!file_only)
        std::cout << message_buffer << std::endl;

    WriteFile(normal_log_file, message_buffer);
}

void AscEmuLog::ConsoleLogDetailFunction(bool file_only, const char* function, const char* format, ...)
{
    if (aelog_file_log_level < LOG_LEVEL_DETAIL || normal_log_file == nullptr)
        return;

    char function_message[32768];
    snprintf(function_message, 32768, "[DETAIL] %s %s", function, format);

    char message_buffer[32768];
    va_list ap;

    va_start(ap, format);
    vsnprintf(message_buffer, 32768, function_message, ap);
    va_end(ap);

    if (!file_only)
    {
        SetConsoleColor(CONSOLE_COLOR_CYAN);
        std::cout << message_buffer << std::endl;
        SetConsoleColor(CONSOLE_COLOR_NORMAL);
    }

    WriteFile(normal_log_file, message_buffer);
}

void AscEmuLog::ConsoleLogDebugFlag(bool file_only, LogFlags log_flags, const char* format, ...)
{
    if (aelog_file_log_level < LOG_LEVEL_DEBUG || error_log_file == nullptr)
        return;

    if (!(aelog_debug_flags & log_flags))
        return;

    char message_buffer[32768];
    va_list ap;

    va_start(ap, format);
    vsnprintf(message_buffer, 32768, format, ap);
    va_end(ap);

    if (!file_only)
    {
        SetConsoleColor(AELog::GetColorForDebugFlag(log_flags));
        std::cout << message_buffer << std::endl;
        SetConsoleColor(CONSOLE_COLOR_NORMAL);
    }

    WriteFile(error_log_file, message_buffer);
}

void AscEmuLog::ConsoleLogDebugFlagFunction(bool file_only, LogFlags log_flags, const char* function, const char* format, ...)
{
    if (aelog_file_log_level < LOG_LEVEL_DEBUG || error_log_file == nullptr)
        return;

    char function_message[32768];
    snprintf(function_message, 32768, "[DEBUG] %s %s", function, format);

    char message_buffer[32768];
    va_list ap;

    va_start(ap, format);
    vsnprintf(message_buffer, 32768, function_message, ap);
    va_end(ap);

    if (!file_only)
    {
        SetConsoleColor(AELog::GetColorForDebugFlag(log_flags));
        std::cout << message_buffer << std::endl;
        SetConsoleColor(CONSOLE_COLOR_NORMAL);
    }

    WriteFile(normal_log_file, message_buffer);
}

