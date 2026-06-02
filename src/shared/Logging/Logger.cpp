/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logger.hpp"
#include "LoggerDefines.hpp"
#include "Utilities/Util.hpp"
#include "Config/Config.h"

#include <string>
#include <algorithm>
#include <fmt/format.h>

namespace AscEmu::Logging
{
    Logger& Logger::getInstance()
    {
        static Logger mInstance;
        return mInstance;
    }

    void Logger::finalize()
    {
        if (this->normalLogFile != nullptr)
        {
            fflush(this->normalLogFile);
            fclose(this->normalLogFile);
            this->normalLogFile = nullptr;
        }

        if (this->errorLogFile != nullptr)
        {
            fflush(this->errorLogFile);
            fclose(this->errorLogFile);
            this->errorLogFile = nullptr;
        }
    }

    void Logger::initializeLogger(std::string file_prefix)
    {
#ifdef _WIN32
        handle_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleOutputCP(65001);
#endif
        std::string normal_filename = file_prefix + "-normal.log";
        std::string error_filename = file_prefix + "-error.log";

        std::string current_date_time = Util::GetCurrentDateTimeString();

        auto logMessage = fmt::format("=================[{}]=================", current_date_time);

        this->normalLogFile = fopen(normal_filename.c_str(), "a");
        if (this->normalLogFile == nullptr)
        {
            setConsoleColor(CONSOLE_COLOR_RED);
            fmt::println("{} : Error opening file {}", __FUNCTION__, normal_filename);
            setConsoleColor(CONSOLE_COLOR_NORMAL);
        }
        else
        {
            writeFile(this->normalLogFile, logMessage);
        }

        this->errorLogFile = fopen(error_filename.c_str(), "a");
        if (this->errorLogFile == nullptr)
        {
            setConsoleColor(CONSOLE_COLOR_RED);
            fmt::println("{} : Error opening file {}", __FUNCTION__, error_filename);
            setConsoleColor(CONSOLE_COLOR_NORMAL);
        }
        else
        {
            writeFile(this->errorLogFile, logMessage);
        }
    }

    void Logger::setMinimumMessageType(MessageType _minimumMessageType)
    {
        this->minimumMessageType = _minimumMessageType;
    }

    void Logger::setDebugFlags(DebugFlags debug_flags)
    {
        this->aelog_debug_flags = debug_flags;
    }

    void Logger::log(Severity severity, MessageType messageType, std::string_view message)
    {
        if (this->minimumMessageType > messageType)
            return;

        auto logMessage = fmt::format("{} {}{}: {}", Util::GetCurrentTimeString(), getSeverityText(severity), getMessageTypeText(messageType), message);

        setSeverityConsoleColor(severity);
        fmt::println("{}", logMessage);
        setConsoleColor(CONSOLE_COLOR_NORMAL);

        writeFile(this->normalLogFile, logMessage);
        if (severity >= Severity::FAILURE)
            writeFile(this->errorLogFile, logMessage);
    }

    void Logger::file(Severity severity, MessageType messageType, std::string_view message)
    {
        auto logMessage = fmt::format("{} {}{}: {}", Util::GetCurrentTimeString(), getSeverityText(severity), getMessageTypeText(messageType), message);

        writeFile(this->normalLogFile, logMessage);
        if (severity >= Severity::FAILURE)
            writeFile(this->errorLogFile, logMessage);
    }

    std::string Logger::getMessageTypeText(MessageType messageType)
    {
        switch (messageType)
        {
        case TRACE:
            return "[TRACE]";
        case DEBUG:
            return "[DEBUG]";
        case MAJOR:
            return "[MAJOR]";
        default:
            return "";
        }
    }

    std::string Logger::getSeverityText(Severity severity)
    {
        switch (severity)
        {
        case WARNING:
            return "[WARNING]";
        case FAILURE:
            return "[ERROR]";
        case FATAL:
            return "[FATAL]";
        case INFO:
        default:
            return "[INFO]";
        }
    }

    void Logger::writeFile(FILE* file, std::string_view msg)
    {
        if (file == nullptr || msg.empty())
            return;

        fmt::println(file, "{}", msg);
        fflush(file);
    }

#ifndef _WIN32
    void Logger::setConsoleColor(const char* color)
    {
        fputs(color, stdout);
    }

#else
    void Logger::setConsoleColor(int color)
    {
        SetConsoleTextAttribute(handle_stdout, (WORD)color);
    }
#endif

    void Logger::setSeverityConsoleColor(Severity severity)
    {
        switch (severity)
        {
            case FAILURE:
            case FATAL:
                setConsoleColor(CONSOLE_COLOR_RED);
                break;
            case BLUE:
                setConsoleColor(CONSOLE_COLOR_BLUE);
                break;
            case YELLOW:
            case WARNING:
                setConsoleColor(CONSOLE_COLOR_YELLOW);
                break;
            case PURPLE:
                setConsoleColor(CONSOLE_COLOR_PURPLE);
                break;
            case CYAN:
                setConsoleColor(CONSOLE_COLOR_CYAN);
                break;
            case INFO:
            default:
                setConsoleColor(CONSOLE_COLOR_NORMAL);
                break;
        }
    }

    Severity Logger::getSeverityConsoleColorByDebugFlag(DebugFlags log_flags)
    {
        switch (log_flags)
        {
            case LF_MAP:
            case LF_MAP_CELL:
            case LF_VMAP:
            case LF_MMAP:
                return BLUE;
            case LF_OPCODE:
                return CYAN;
            case LF_SPELL:
            case LF_AURA:
            case LF_SPELL_EFF:
            case LF_AURA_EFF:
                return PURPLE;
            default:
                return YELLOW;
        }
    }

    std::string getFormattedFileName(const std::string& path_prefix, const std::string& file_prefix, bool use_date_time)
    {
        if (use_date_time)
        {
            std::string current_date_time = Util::GetCurrentDateTimeString();
            // replace time seperator with valid character for file name
            std::replace(current_date_time.begin(), current_date_time.end(), ':', '-');
            std::replace(current_date_time.begin(), current_date_time.end(), ' ', '_');

            return fmt::format("{}{}_{}.log", path_prefix, current_date_time, file_prefix);
        }

        return fmt::format("{}{}.log", path_prefix, file_prefix);
    }
}
