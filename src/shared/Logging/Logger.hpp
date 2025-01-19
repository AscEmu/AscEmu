/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CThreads.h"
#include "MessageType.hpp"
#include "Severity.hpp"
#include "StringFormat.hpp"

// Required for fmt lib 10.0+ because enums are not formatted automatically anymore -Appled
template <typename EnumType>
    requires std::is_enum_v<EnumType>
struct fmt::formatter<EnumType> : fmt::formatter<std::underlying_type_t<EnumType>>
{
    // Forwards the formatting by casting the enum to it's underlying type
    auto format(const EnumType& enumValue, format_context& ctx) const
    {
        return fmt::formatter<std::underlying_type_t<EnumType>>::format(
            static_cast<std::underlying_type_t<EnumType>>(enumValue), ctx);
    }
};

namespace AscEmu::Logging
{
    class SERVER_DECL Logger
    {
        FILE* normalLogFile = nullptr;
        FILE* errorLogFile = nullptr;
        MessageType minimumMessageType = MessageType::MINOR;
        uint32_t aelog_debug_flags = 0;

#ifdef _WIN32
        HANDLE handle_stdout = nullptr;
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

        void setDebugFlags(DebugFlags debug_flags);

        template<typename... Args>
        inline void trace(std::string_view fmt, Args&&... args)
        {
            log(Severity::INFO, MessageType::TRACE, StringFormat(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        inline void debug(std::string_view fmt, Args&&... args)
        {
            log(Severity::INFO, MessageType::DEBUG, StringFormat(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        inline void debugFlag(DebugFlags log_flags, std::string_view fmt, Args&&... args)
        {
            if (!(aelog_debug_flags & log_flags))
                return;
            log(getSeverityConsoleColorByDebugFlag(log_flags), MessageType::DEBUG, StringFormat(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        inline void info(std::string_view fmt, Args&&... args)
        {
            log(Severity::INFO, MessageType::MINOR, StringFormat(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        inline void warning(std::string_view fmt, Args&&... args)
        {
            log(Severity::WARNING, MessageType::MINOR, StringFormat(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        inline void failure(std::string_view fmt, Args&&... args)
        {
            log(Severity::FAILURE, MessageType::MAJOR, StringFormat(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        inline void fatal(std::string_view fmt, Args&&... args)
        {
            log(Severity::FATAL, MessageType::MAJOR, StringFormat(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        inline void log(Severity severity, MessageType messageType, std::string_view fmt, Args&&... args)
        {
            log(severity, messageType, StringFormat(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        inline void file(Severity severity, MessageType messageType, std::string_view fmt, Args&&... args)
        {
            file(severity, messageType, StringFormat(fmt, std::forward<Args>(args)...));
        }

        void log(Severity severity, MessageType messageType, std::string_view message);
        void file(Severity severity, MessageType messageType, std::string_view message);

    private:
        Logger() = default;
        ~Logger() = default;

        std::string getMessageTypeText(MessageType messageType);
        std::string getSeverityText(Severity severity);

        void writeFile(FILE* file, std::string_view msg);

#ifndef _WIN32
        void setConsoleColor(const char* color);
#else
        void setConsoleColor(int color);
#endif

        void setSeverityConsoleColor(Severity severity);
        Severity getSeverityConsoleColorByDebugFlag(DebugFlags log_flags);
    };

    std::string getFormattedFileName(const std::string& path_prefix, const std::string& file_prefix, bool use_date_time);
}

#define sLogger AscEmu::Logging::Logger::getInstance()
