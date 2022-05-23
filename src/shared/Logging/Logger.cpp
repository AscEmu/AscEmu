/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logger.hpp"
#include "LoggerDefines.hpp"
#include "Util.hpp"
#include "Config/Config.h"

#include <iostream>
#include <cstdarg>
#include <string>

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

    void Logger::initalizeLogger(std::string file_prefix)
    {
#ifdef _WIN32
        handle_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

        std::string normal_filename = file_prefix + "-normal.log";
        std::string error_filename = file_prefix + "-error.log";

        std::string current_date_time = Util::GetCurrentDateTimeString();

        char logMessage[32768];
        sprintf(logMessage, "=================[%s]=================", current_date_time.c_str());

        this->normalLogFile = fopen(normal_filename.c_str(), "a");
        if (this->normalLogFile == nullptr)
            std::cerr << __FUNCTION__ << " : Error opening file " << normal_filename << std::endl;
        else
            writeFile(this->normalLogFile, logMessage);

        this->errorLogFile = fopen(error_filename.c_str(), "a");
        if (this->errorLogFile == nullptr)
            std::cerr << __FUNCTION__ << " : Error opening file " << error_filename << std::endl;
        else
            writeFile(this->errorLogFile, logMessage);
    }

    void Logger::setMinimumMessageType(MessageType _minimumMessageType)
    {
        this->minimumMessageType = _minimumMessageType;
    }

    void Logger::setDebugFlags(DebugFlags debug_flags)
    {
        this->aelog_debug_flags = debug_flags;
    }

    void Logger::trace(const char* message, ...)
    {
        va_list arguments;
        va_start(arguments, message);
        log(Severity::INFO, MessageType::TRACE, message, arguments);
        va_end(arguments);
    }

    void Logger::debug(const char* message, ...)
    {
        va_list arguments;
        va_start(arguments, message);
        log(Severity::INFO, MessageType::DEBUG, message, arguments);
        va_end(arguments);
    }

    void Logger::debugFlag(DebugFlags log_flags, const char* message, ...)
    {
        if (!(aelog_debug_flags & log_flags))
            return;

        va_list arguments;
        va_start(arguments, message);
        auto severity = getSeverityConsoleColorByDebugFlag(log_flags);
        log(severity, MessageType::DEBUG, message, arguments);
        va_end(arguments);

    }

    void Logger::info(const char* message, ...)
    {
        va_list arguments;
        va_start(arguments, message);
        log(Severity::INFO, MessageType::MINOR, message, arguments);
        va_end(arguments);
    }

    void Logger::warning(const char* message, ...)
    {
        va_list arguments;
        va_start(arguments, message);
        log(Severity::WARNING, MessageType::MINOR, message, arguments);
        va_end(arguments);
    }

    void Logger::failure(const char* message, ...)
    {
        va_list arguments;
        va_start(arguments, message);
        log(Severity::FAILURE, MessageType::MAJOR, message, arguments);
        va_end(arguments);
    }

    void Logger::fatal(const char* message, ...)
    {
        va_list arguments;
        va_start(arguments, message);
        log(Severity::FATAL, MessageType::MAJOR, message, arguments);
        va_end(arguments);
    }

    void Logger::log(Severity severity, MessageType messageType, const char* message, ...)
    {
        va_list arguments;
        va_start(arguments, message);
        log(severity, messageType, message, arguments);
        va_end(arguments);
    }

    void Logger::log(Severity severity, MessageType messageType, const char* message, va_list arguments)
    {
        if (this->minimumMessageType > messageType)
            return;

        char logMessage[327680];
        createLogMessage(logMessage, severity, messageType, message, arguments);
  
        setSeverityConsoleColor(severity);
        std::cout << logMessage << "\n";
        setConsoleColor(CONSOLE_COLOR_NORMAL);

        if (severity >= Severity::FAILURE)
            writeFile(this->errorLogFile, logMessage);

        writeFile(this->normalLogFile, logMessage);
    }

    void Logger::file(Severity severity, MessageType messageType, const char* message, ...)
    {
        char logMessage[327680];
        va_list arguments;
        va_start(arguments, message);
        createLogMessage(logMessage, severity, messageType, message, arguments);
        va_end(arguments);

        if (severity >= Severity::FAILURE)
            writeFile(this->errorLogFile, logMessage);

        writeFile(this->normalLogFile, logMessage);
    }

    void Logger::createLogMessage(char* result, Severity severity, MessageType messageType, const char* message, va_list arguments)
    {
        char formattedMessage[32768];
        vsnprintf(formattedMessage, 32768, message, arguments);

        std::string currentTime = Util::GetCurrentTimeString();
        std::string severityText = getSeverityText(severity);
        std::string messageTypeText = getMessageTypeText(messageType);

        sprintf(result, "%s %s%s: %s", currentTime.c_str(), severityText.c_str(), messageTypeText.c_str(), formattedMessage);
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

    void Logger::writeFile(FILE* file, const char* msg)
    {
        if (file == nullptr)
            return;
        fprintf(file, "%s\n", msg);
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
}
