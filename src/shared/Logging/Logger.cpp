/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logger.hpp"
#include "LoggerDefines.hpp"
#include "MessageType.hpp"
#include "Severity.hpp"
#include "Utilities/Util.hpp"

#include <cstdio>
#include <string>
#include <string_view>
#include <algorithm>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <Windows.h>
#endif

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
            static_cast<void>(std::fclose(normalLogFile));
            this->normalLogFile = nullptr;
        }

        if (this->errorLogFile != nullptr)
        {
            static_cast<void>(std::fclose(errorLogFile));
            this->errorLogFile = nullptr;
        }
    }

    void Logger::initializeLogger(std::string_view filePrefix)
    {
#ifdef _WIN32
        handleStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleOutputCP(CP_UTF8);

        DWORD consoleMode = 0;
        if (GetConsoleMode(handleStdout, &consoleMode))
        {
            SetConsoleMode(handleStdout, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
#endif
        const std::string normalFilename = fmt::format("{}-normal.log", filePrefix);
        const std::string errorFilename = fmt::format("{}-error.log", filePrefix);

        std::string currentDateTime = ::Util::GetCurrentDateTimeString();

        auto logMessage = fmt::format("=================[{}]=================", currentDateTime);

        this->normalLogFile = fopen(normalFilename.c_str(), "a");
        if (this->normalLogFile == nullptr)
        {
            setConsoleColor(CONSOLE_COLOR_RED);
            fmt::println("{} : Error opening file {}", __FUNCTION__, normalFilename);
            setConsoleColor(CONSOLE_COLOR_NORMAL);
        }
        else
        {
            writeFile(this->normalLogFile, logMessage);
        }

        this->errorLogFile = fopen(errorFilename.c_str(), "a");
        if (this->errorLogFile == nullptr)
        {
            setConsoleColor(CONSOLE_COLOR_RED);
            fmt::println("{} : Error opening file {}", __FUNCTION__, errorFilename);
            setConsoleColor(CONSOLE_COLOR_NORMAL);
        }
        else
        {
            writeFile(this->errorLogFile, logMessage);
        }
    }

    void Logger::setMinimumMessageType(MessageType messageType)
    {
        this->minimumMessageType = messageType;
    }

    void Logger::setDebugFlags(DebugFlags debugFlags, bool enabled)
    {
        if (enabled)
            aelogDebugFlags |= debugFlags;
        else
            aelogDebugFlags &= ~debugFlags;
    }

    void Logger::log(Severity severity, MessageType messageType, std::string_view message)
    {
        if (this->minimumMessageType > messageType)
            return;

        auto logMessage = fmt::format("{} {}{}: {}", ::Util::GetCurrentTimeString(), getSeverityText(severity), getMessageTypeText(messageType), message);

        setSeverityConsoleColor(severity);
        fmt::println("{}", logMessage);
        setConsoleColor(CONSOLE_COLOR_NORMAL);

        writeFile(this->normalLogFile, logMessage);
        if (severity >= Severity::FAILURE)
            writeFile(this->errorLogFile, logMessage);
    }

    void Logger::file(Severity severity, MessageType messageType, std::string_view message)
    {
        auto logMessage = fmt::format("{} {}{}: {}", ::Util::GetCurrentTimeString(), getSeverityText(severity), getMessageTypeText(messageType), message);

        writeFile(this->normalLogFile, logMessage);
        if (severity >= Severity::FAILURE)
            writeFile(this->errorLogFile, logMessage);
    }

    std::string_view Logger::getMessageTypeText(MessageType messageType)
    {
        switch (messageType)
        {
            case MessageType::TRACE:
                return "[TRACE]";
            case MessageType::DEBUG:
                return "[DEBUG]";
            case MessageType::MINOR:
                return "";
            case MessageType::MAJOR:
                return "[MAJOR]";
        }

        return "";
    }

    std::string_view Logger::getSeverityText(Severity severity)
    {
        switch (severity)
        {
            case Severity::WARNING:
                return "[WARNING]";
            case Severity::FAILURE:
                return "[ERROR]";
            case Severity::FATAL:
                return "[FATAL]";
            case Severity::INFO:
                return "[INFO]";
            case Severity::NONE:
            case Severity::BLUE:
            case Severity::PURPLE:
            case Severity::YELLOW:
            case Severity::CYAN:
                return "";
        }

        return "";
    }

    void Logger::writeFile(FILE* file, std::string_view msg)
    {
        if (file == nullptr || msg.empty())
            return;

        fmt::println(file, "{}", msg);
        static_cast<void>(std::fflush(file));
    }

    void Logger::setConsoleColor(std::string_view color)
    {
        fmt::print("{}", color);
    }

    void Logger::setSeverityConsoleColor(Severity severity)
    {
        switch (severity)
        {
            case Severity::FAILURE:
            case Severity::FATAL:
                setConsoleColor(CONSOLE_COLOR_RED);
                break;
            case Severity::BLUE:
                setConsoleColor(CONSOLE_COLOR_BLUE);
                break;
            case Severity::YELLOW:
            case Severity::WARNING:
                setConsoleColor(CONSOLE_COLOR_YELLOW);
                break;
            case Severity::PURPLE:
                setConsoleColor(CONSOLE_COLOR_PURPLE);
                break;
            case Severity::CYAN:
                setConsoleColor(CONSOLE_COLOR_CYAN);
                break;
            case Severity::INFO:
            case Severity::NONE:
                setConsoleColor(CONSOLE_COLOR_NORMAL);
                break;
        }
    }

    Severity Logger::getSeverityConsoleColorByDebugFlag(DebugFlags logFlags)
    {
        switch (logFlags)
        {
            case LF_MAP:
            case LF_MAP_CELL:
            case LF_VMAP:
            case LF_MMAP:
                return Severity::BLUE;
            case LF_OPCODE:
                return Severity::CYAN;
            case LF_SPELL:
            case LF_AURA:
            case LF_SPELL_EFF:
            case LF_AURA_EFF:
                return Severity::PURPLE;
            case LF_NONE:
            case LF_SCRIPT_MGR:
            case LF_DB_TABLES:
            case LF_MOVE:
            case LF_CREATURE_AI:
            case LF_ALL:
                return Severity::YELLOW;
        }
        return Severity::YELLOW;
    }

    std::string getFormattedFileName(std::string_view pathPrefix, std::string_view filePrefix, bool useDateTime)
    {
        if (useDateTime)
        {
            std::string currentDateTime = ::Util::GetCurrentDateTimeString();
            // replace time separator with valid character for file name
            std::ranges::replace(currentDateTime, ':', '-');
            std::ranges::replace(currentDateTime, ' ', '_');

            return fmt::format("{}{}_{}.log", pathPrefix, currentDateTime, filePrefix);
        }

        return fmt::format("{}{}.log", pathPrefix, filePrefix);
    }
}
