/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Common.hpp"
#include "LogDefines.hpp"
#include "Singleton.h"
#include "Config/Config.h"

class WorldPacket;

extern SERVER_DECL time_t UNIXTIME;        //update this every loop to avoid the time() syscall!
extern SERVER_DECL tm g_localTime;

//////////////////////////////////////////////////////////////////////////////////////////
// WorldPacketLog
class WorldPacketLog : public Singleton<WorldPacketLog>
{
    FILE* mPacketLogFile;
    bool isLogEnabled;

    Mutex mPacketLogMutex;

    public:

        WorldPacketLog();
        ~WorldPacketLog();

        void initWorldPacketLog(bool enableLog);
        void enablePacketLog();
        void disablePacketLog();

        //WorldSocket.cpp
        void logPacket(uint32_t len, uint16_t opcode, const uint8_t* data, uint8_t direction, uint32_t accountid = 0);
};
#define sWorldPacketLog WorldPacketLog::getSingleton()


//////////////////////////////////////////////////////////////////////////////////////////
// SessionLog
class WorldSession;

class SERVER_DECL SessionLog
{
    FILE* mSessionLogFile;
    char* mFileName;

public:

    SessionLog(const char* filename, bool open);
    ~SessionLog();

    void openSessionLog();
    bool isSessionLogOpen();
    void closeSessionLog();

    void write(const char* format, ...);

    //WorldSession.cpp
    void writefromsession(WorldSession* session, const char* format, ...);
};

//////////////////////////////////////////////////////////////////////////////////////////
// AscEmuLog format/color

namespace AELog
{
    /*! \brief Returns formatted file name based on input */
    std::string GetFormattedFileName(std::string path_prefix, std::string file_prefix, bool use_date_time);

    /*! \brief Returns cons char* (linux) or int (windows) color definition for console */
#ifndef _WIN32
    const char* GetColorForDebugFlag(LogFlags log_flags);
#else
    int GetColorForDebugFlag(LogFlags log_flags);
#endif
}

class SERVER_DECL AscEmuLog : public Singleton<AscEmuLog>
{
    FILE* normal_log_file;
    FILE* error_log_file;

    uint32_t aelog_file_log_level;
    uint32_t aelog_debug_flags;

#ifdef _WIN32
    HANDLE handle_stdout;
#endif

    public:

        AscEmuLog() : normal_log_file(nullptr), error_log_file(nullptr), aelog_file_log_level(0), aelog_debug_flags(0) {}
        ~AscEmuLog()
        {
            if (normal_log_file != nullptr)
            {
                fflush(normal_log_file);
                fclose(normal_log_file);
                normal_log_file = nullptr;
            }

            if (error_log_file != nullptr)
            {
                fflush(error_log_file);
                fclose(error_log_file);
                error_log_file = nullptr;
            }
        }

        void InitalizeLogFiles(std::string file_prefix);

        void WriteFile(FILE* file, char* msg, const char* source = NULL);

#ifndef _WIN32
        void SetConsoleColor(const char* color);
#else
        void SetConsoleColor(int color);
#endif
        void SetFileLoggingLevel(uint32_t level);
        void SetDebugFlags(uint32_t flags);

        void ConsoleLogDefault(bool file_only, const char* format, ...);
        void ConsoleLogDefaultFunction(bool file_only, const char* function, const char* format, ...);

        void ConsoleLogError(bool file_only, const char* format, ...);
        void ConsoleLogErrorFunction(bool file_only, const char* function, const char* format, ...);

        void ConsoleLogDetail(uint8_t color, bool file_only, const char* format, ...);
        void ConsoleLogDetailFunction(bool file_only, const char* function, const char* format, ...);

        void ConsoleLogDebugFlag(bool file_only, LogFlags log_flags, const char* format, ...);
        void ConsoleLogDebugFlagFunction(bool file_only, LogFlags log_flags, const char* function, const char* format, ...);

        void ConsoleLogMajorError(std::string line1, std::string line2, std::string line3, std::string line4);
};

#define AscLog AscEmuLog::getSingleton()

/*! \brief Logging Level: Normal */
#define LogDefault(msg, ...) AscLog.ConsoleLogDefault(false, msg, ##__VA_ARGS__)
#define LogError(msg, ...) AscLog.ConsoleLogError(false, msg, ##__VA_ARGS__)

/*! \brief Logging Level: Detail */
#define LogDetail(msg, ...) AscLog.ConsoleLogDetail(0, false, msg, ##__VA_ARGS__)
#define LogNotice(msg, ...) AscLog.ConsoleLogDetail(1, false, msg, ##__VA_ARGS__)
#define LogWarning(msg, ...) AscLog.ConsoleLogDetail(2, false, msg, ##__VA_ARGS__)

/*! \brief Logging Level: Debug */
#define LogDebug(msg, ...) AscLog.ConsoleLogDebugFlag(false, LF_NONE, msg, ##__VA_ARGS__)
#define LogDebugFlag(db_flag, msg, ...) AscLog.ConsoleLogDebugFlag(false, db_flag, msg, ##__VA_ARGS__)


#define LOG_BASIC(msg, ...) AscLog.ConsoleLogDefaultFunction(false, __FUNCTION__, msg, ##__VA_ARGS__)
#define LOG_DETAIL(msg, ...) AscLog.ConsoleLogDetailFunction(false, __FUNCTION__, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) AscLog.ConsoleLogErrorFunction(false, __FUNCTION__, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) AscLog.ConsoleLogDebugFlagFunction(false, LF_NONE, __FUNCTION__, msg, ##__VA_ARGS__)
