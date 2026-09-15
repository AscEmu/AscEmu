/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#ifdef __INTELLISENSE__
#define FMT_USE_CONSTEVAL 0
#endif
#include <fmt/format.h>

#include "MessageType.hpp"
#include "Severity.hpp"
#include "Platform/SymbolVisibility.hpp"

#ifdef _WIN32
    #include <Windows.h>
#endif

// Required for fmt lib 10.0+ because enums are not formatted automatically anymore -Appled
template <typename EnumType>
    requires std::is_enum_v<EnumType>
struct fmt::formatter<EnumType> : fmt::formatter<std::underlying_type_t<EnumType>>
{
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
        uint32_t aelogDebugFlags = 0;

#ifdef _WIN32
        HANDLE handleStdout = nullptr;
#endif

    public:
        Logger(Logger&&) = delete;
        Logger(Logger const&) = delete;
        Logger& operator=(Logger&&) = delete;
        Logger& operator=(Logger const&) = delete;

        [[nodiscard]] static Logger& getInstance();

        void finalize();

        void initializeLogger(std::string_view filePrefix);

        void setMinimumMessageType(MessageType messageType);

        void setDebugFlags(DebugFlags debugFlags, bool enabled);

        template <typename... Args>
        void trace(std::string_view fmt, Args&&... args)
        {
            if (minimumMessageType > MessageType::TRACE)
                return;

            log(Severity::INFO, MessageType::TRACE, fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void debug(std::string_view fmt, Args&&... args)
        {
            if (minimumMessageType > MessageType::DEBUG)
                return;

            log(Severity::INFO, MessageType::DEBUG, fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void debugFlag(DebugFlags logFlags, std::string_view fmt, Args&&... args)
        {
            if (minimumMessageType > MessageType::DEBUG || !(aelogDebugFlags & logFlags))
                return;

            log(getSeverityConsoleColorByDebugFlag(logFlags), MessageType::DEBUG, fmt::format(fmt, std::forward<Args>(args)...));
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        // specialized debug helpers for frequently used logging categories.
        template <typename... Args>
        void debugOpcode(std::string_view fmt, Args&&... args)
        {
            debugFlag(LF_OPCODE, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void debugDbTables(std::string_view fmt, Args&&... args)
        {
            debugFlag(LF_DB_TABLES, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void debugMap(std::string_view fmt, Args&&... args)
        {
            debugFlag(LF_MAP, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void debugMapCell(std::string_view fmt, Args&&... args)
        {
            debugFlag(LF_MAP_CELL, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void debugMove(std::string_view fmt, Args&&... args)
        {
            debugFlag(LF_MOVE, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void debugSpell(std::string_view fmt, Args&&... args)
        {
            debugFlag(LF_SPELL, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void debugSpellEffect(std::string_view fmt, Args&&... args)
        {
            debugFlag(LF_SPELL_EFF, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void debugAura(std::string_view fmt, Args&&... args)
        {
            debugFlag(LF_AURA, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void debugAuraEffect(std::string_view fmt, Args&&... args)
        {
            debugFlag(LF_AURA_EFF, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void debugScript(std::string_view fmt, Args&&... args)
        {
            debugFlag(LF_SCRIPT_MGR, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void debugCreatureAi(std::string_view fmt, Args&&... args)
        {
            debugFlag(LF_CREATURE_AI, fmt, std::forward<Args>(args)...);
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        // standard logging helpers for different message severities.
        template <typename... Args>
        void info(std::string_view fmt, Args&&... args)
        {
            if (minimumMessageType > MessageType::MINOR)
                return;

            log(Severity::INFO, MessageType::MINOR, fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void warning(std::string_view fmt, Args&&... args)
        {
            if (minimumMessageType > MessageType::MINOR)
                return;

            log(Severity::WARNING, MessageType::MINOR, fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void failure(std::string_view fmt, Args&&... args)
        {
            log(Severity::FAILURE, MessageType::MAJOR, fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void fatal(std::string_view fmt, Args&&... args)
        {
            log(Severity::FATAL, MessageType::MAJOR, fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void log(Severity severity, MessageType messageType, std::string_view fmt, Args&&... args)
        {
            log(severity, messageType, fmt::format(fmt, std::forward<Args>(args)...));
        }

        //////////////////////////////////////////////////////////////////////////////////////////
        // formatted file logging interface.
        template <typename... Args>
        void file(Severity severity, MessageType messageType, std::string_view fmt, Args&&... args)
        {
            file(severity, messageType, fmt::format(fmt, std::forward<Args>(args)...));
        }

        void log(Severity severity, MessageType messageType, std::string_view message);
        void file(Severity severity, MessageType messageType, std::string_view message);

    private:
        Logger() = default;
        ~Logger() = default;

        [[nodiscard]] static std::string_view getMessageTypeText(MessageType messageType);
        [[nodiscard]] static std::string_view getSeverityText(Severity severity);

        static void writeFile(FILE* file, std::string_view msg);
        static void setConsoleColor(std::string_view color);
        static void setSeverityConsoleColor(Severity severity);

        static Severity getSeverityConsoleColorByDebugFlag(DebugFlags logFlags);
    };

    [[nodiscard]] std::string getFormattedFileName(std::string_view pathPrefix, std::string_view filePrefix, bool useDateTime);
}

#define sLogger AscEmu::Logging::Logger::getInstance()
