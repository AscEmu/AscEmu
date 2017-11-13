/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Log.hpp"
#include "Util.hpp"
#include "Config/Config.h"

#include <iostream>
#include <cstdarg>
#include <string>

//////////////////////////////////////////////////////////////////////////////////////////
// World functions
initialiseSingleton(WorldPacketLog);

SERVER_DECL time_t UNIXTIME;
SERVER_DECL tm g_localTime;

WorldPacketLog::WorldPacketLog() : isLogEnabled(false), mPacketLogFile(nullptr)
{
}

WorldPacketLog::~WorldPacketLog()
{
    if (mPacketLogFile)
    {
        fclose(mPacketLogFile);
        mPacketLogFile = nullptr;
    }
}

void WorldPacketLog::initWorldPacketLog(bool enableLog)
{
    isLogEnabled = enableLog;

    if (isLogEnabled)
    {
        LogNotice("WorldPacketLog : Enabling packetlog output to \"world-packet.log\"");
        enablePacketLog();
    }
    else
    {
        disablePacketLog();
    }
}

void WorldPacketLog::enablePacketLog()
{
    if (mPacketLogFile != nullptr)
    {
        disablePacketLog();
        isLogEnabled = true;
    }
    mPacketLogFile = fopen("world-packet.log", "a");
}

void WorldPacketLog::disablePacketLog()
{
    if (mPacketLogFile != nullptr)
    {
        fflush(mPacketLogFile);
        fclose(mPacketLogFile);
        mPacketLogFile = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// SessionLog functions

SessionLog::SessionLog(const char* filename, bool open)
{
#if defined(linux) || defined(__linux)
    mFileName = strdup(filename);
#else
    mFileName = _strdup(filename);
#endif
    
    mSessionLogFile = nullptr;
    if (open)
    {
        openSessionLog();
    }
}

SessionLog::~SessionLog()
{
    if (mSessionLogFile != nullptr)
    {
        closeSessionLog();
    }

    free(mFileName);
}

void SessionLog::openSessionLog()
{
    mSessionLogFile = fopen(mFileName, "a");
}

bool SessionLog::isSessionLogOpen()
{
    return (mSessionLogFile != nullptr);
}

void SessionLog::closeSessionLog()
{
    if (mSessionLogFile != nullptr)
    {
        fflush(mSessionLogFile);
        fclose(mSessionLogFile);
        mSessionLogFile = nullptr;
    }
}

void SessionLog::write(const char* format, ...)
{
    if (mSessionLogFile != nullptr)
    {
        char out[32768];
        va_list ap;

        va_start(ap, format);

        std::string current_time = "[" + Util::GetCurrentDateTimeString() + "] ";
        sprintf(out, current_time.c_str());

        size_t l = strlen(out);
        vsnprintf(&out[l], 32768 - l, format, ap);
        fprintf(mSessionLogFile, "%s\n", out);
        va_end(ap);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// AscEmuLog format/color functions
namespace AELog
{
    std::string GetFormattedFileName(std::string path_prefix, std::string file_prefix, bool use_date_time)
    {
        std::stringstream path_name;
        path_name << path_prefix;

        if (use_date_time)
        {
            std::string current_date_time = Util::GetCurrentDateTimeString();
            //replace time seperator with valid character for file name
            std::replace(current_date_time.begin(), current_date_time.end(), ':', '-');
            std::replace(current_date_time.begin(), current_date_time.end(), ' ', '_');

            path_name << current_date_time << "_";
        }

        path_name << file_prefix << ".log";

        return path_name.str();
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

void AscEmuLog::ConsoleLogDetail(uint8_t color, bool file_only, const char* format, ...)
{
    if (aelog_file_log_level < LL_DETAIL || normal_log_file == nullptr)
        return;

    char message_buffer[32768];
    va_list ap;

    va_start(ap, format);
    vsnprintf(message_buffer, 32768, format, ap);
    va_end(ap);

    if (!file_only)
    {
        switch (color)
        {
            case 0:
                AscLog.SetConsoleColor(CONSOLE_COLOR_CYAN);
                break;
            case 1:
                AscLog.SetConsoleColor(CONSOLE_COLOR_GREEN);
                break;
            default:
                AscLog.SetConsoleColor(CONSOLE_COLOR_WHITE);
                break;
        }

        std::cout << message_buffer << std::endl;

        AscLog.SetConsoleColor(CONSOLE_COLOR_NORMAL);
    }
        

    WriteFile(normal_log_file, message_buffer);
}

void AscEmuLog::ConsoleLogDetailFunction(bool file_only, const char* function, const char* format, ...)
{
    if (aelog_file_log_level < LL_DETAIL || normal_log_file == nullptr)
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
    if (aelog_file_log_level < LL_DEBUG || error_log_file == nullptr)
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
    if (aelog_file_log_level < LL_DEBUG || error_log_file == nullptr)
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

void AscEmuLog::ConsoleLogMajorError(std::string line1, std::string line2, std::string line3, std::string line4)
{
    std::stringstream sstream;
    sstream << "*********************************************************************" << std::endl;
    sstream << "*                        MAJOR ERROR/WARNING                         " << std::endl;
    sstream << "*                        *******************                         " << std::endl;
    sstream << "* " << line1 << std::endl;
    if (!line2.empty())
        sstream << "* " << line2 << std::endl;
    if (!line3.empty())
        sstream << "* " << line3 << std::endl;
    if (!line4.empty())
        sstream << "* " << line4 << std::endl;
    sstream << "*********************************************************************" << std::endl;

    SetConsoleColor(CONSOLE_COLOR_RED);
    std::cout << sstream.str() << std::endl;
    SetConsoleColor(CONSOLE_COLOR_NORMAL);

#if defined(linux) || defined(__linux)
    WriteFile(error_log_file, strdup(sstream.str().c_str()));
#else
    WriteFile(error_log_file, _strdup(sstream.str().c_str()));
#endif
}

