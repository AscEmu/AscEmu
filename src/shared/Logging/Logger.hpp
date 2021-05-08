/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"
#include "Config/Config.h"
#include "MessageType.hpp"
#include "Severity.hpp"

namespace AscEmu::Logging
{
    class SERVER_DECL Logger
    {
        FILE* normalLogFile = nullptr;
        FILE* errorLogFile = nullptr;;
        MessageType minimumMessageType = MessageType::MINOR;

#ifdef _WIN32
        HANDLE handle_stdout;
#endif

    public:
        Logger(Logger&&) = delete;
        Logger(Logger const&) = delete;
        Logger& operator=(Logger&&) = delete;
        Logger& operator=(Logger const&) = delete;

        static Logger& getInstance();

        void finalize();

        void initalizeLogger(std::string file_prefix);

        void setMinimumMessageType(MessageType messsageType);

        void trace(const char* message, ...);

        void debug(const char* message, ...);

        void info(const char* message, ...);

        void warning(const char* message, ...);

        void failure(const char* message, ...);

        void fatal(const char* message, ...);

        void log(Severity severity, MessageType messageType, const char* message, ...);

        void log(Severity severity, MessageType messageType, const char* message, va_list arguments);

        void file(Severity severity, MessageType messageType, const char* message, ...);

    private:
        Logger() = default;
        ~Logger() = default;

        void createLogMessage(char* result, Severity severity, MessageType messageType, const char* message, va_list arguments);
        std::string getMessageTypeText(MessageType messageType);
        std::string getSeverityText(Severity severity);

        void writeFile(FILE* file, const char* msg);

#ifndef _WIN32
        void setConsoleColor(const char* color);
#else
        void setConsoleColor(int color);
#endif

        void setSeverityConsoleColor(Severity severity);
    };

    std::string getFormattedFileName(std::string path_prefix, std::string file_prefix, bool use_date_time);
}
#define sLogger AscEmu::Logging::Logger::getInstance()
