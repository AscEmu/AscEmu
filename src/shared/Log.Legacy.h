/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LOG_H
#define _LOG_H

#include "Common.hpp"
#include "Singleton.h"

class WorldPacket;
class WorldSession;

enum LogType
{
    WORLD_LOG,
    LOGON_LOG
};

enum LogLevel
{
    LOG_LEVEL_NORMAL    = 0,
    LOG_LEVEL_DETAIL    = 1,
    LOG_LEVEL_DEBUG     = 2
};

enum LogFlags
{
    LF_NONE         = 0x00,
    LF_OPCODE       = 0x01,
    LF_MAP          = 0x02,
    LF_MAP_CELL     = 0x04,
    LF_VMAP         = 0x08,
    LF_MMAP         = 0x10,
    LF_SPELL        = 0x20,
    LF_AURA         = 0x40,
    LF_SPELL_EFF    = 0x80,
    LF_AURA_EFF     = 0x100,
    LF_SCRIPT_MGR   = 0x200,
    LF_DB_TABLES    = 0x400,

    LF_ALL          = 0x800 - 0x01
};

extern SERVER_DECL time_t UNIXTIME;        //update this every loop to avoid the time() syscall!
extern SERVER_DECL tm g_localTime;

class SERVER_DECL oLog : public Singleton< oLog >
{
    public:

        //////////////////////////////////////////////////////////////////////////////////////////
        // AscEmu functions begin
        oLog() : m_fileLogLevel(0) {}

        void InitalizeLogFiles(std::string file_prefix);

        void DebugFlag(LogFlags log_flags, const char* format, ...);

    private:
        /*! \brief Returns color defines for plattform */
#ifndef _WIN32
        void SetConsoleColor(const char* color);    //AscEmu
#else
        void SetConsoleColor(int color);            //AscEmu
#endif
        // AscEmu functions end
        //////////////////////////////////////////////////////////////////////////////////////////


    public:

        //log level 0
        void outString(const char* str, ...);
        void outError(const char* err, ...);
        void outErrorSilent(const char* err, ...);      // Writes into the error log without giving console output. Used for the startup banner.
        void outBasic(const char* str, ...);
        //log level 1
        void outDetail(const char* str, ...);
        //log level 2
        void outDebug(const char* str, ...);

        void logError(const char* file, int line, const char* fncname, const char* msg, ...);
        void logDebug(const char* file, int line, const char* fncname, const char* msg, ...);
        void logBasic(const char* file, int line, const char* fncname,  const char* msg, ...);
        void logDetail(const char* file, int line, const char* fncname, const char* msg, ...);

        //old NGLog.h methods
        //log level 0
        void Success(const char* source, const char* format, ...);
        void Error(const char* source, const char* format, ...);
        void LargeErrorMessage(const char* str, ...);
        //log level 1
        void Notice(const char* source, const char* format, ...);
        void Warning(const char* source, const char* format, ...);
        //log level 2
        void Debug(const char* source, const char* format, ...);

        void SetFileLoggingLevel(int32 level);
        void SetDebugFlags(uint32 flags);

        void Close();

        int32 m_fileLogLevel;
        uint32 mDebugFlags;

    private:

        FILE* m_normalFile, *m_errorFile;
        void outFile(FILE* file, char* msg, const char* source = NULL);
        void outFileSilent(FILE* file, char* msg, const char* source = NULL);   // Prints text to file without showing it to the user. Used for the startup banner.


#ifdef _WIN32
        HANDLE stdout_handle;
#endif

        inline char dcd(char in)
        {
            char out = in;
            out -= 13;
            out ^= 131;
            return out;
        }

        void dcds(char* str)
        {
            unsigned long i = 0;
            size_t len = strlen(str);

            for(i = 0; i < len; ++i)
                str[i] = dcd(str[i]);

        }

        void pdcds(const char* str, char* buf)
        {
            strcpy(buf, str);
            dcds(buf);
        }
};

class SERVER_DECL SessionLogWriter
{
        FILE* m_file;
        char* m_filename;

    public:

        SessionLogWriter(const char* filename, bool open);
        ~SessionLogWriter();

        void write(const char* format, ...);
        void writefromsession(WorldSession* session, const char* format, ...);

        inline bool IsOpen() { return (m_file != NULL); }

        void Open();
        void Close();
};


#define Log oLog::getSingleton()

#define LOG_BASIC(msg, ...) Log.logBasic(__FILE__, __LINE__, __FUNCTION__, msg, ##__VA_ARGS__)
#define LOG_DETAIL(msg, ...) Log.logDetail(__FILE__, __LINE__, __FUNCTION__, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) Log.logError(__FILE__, __LINE__, __FUNCTION__, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) Log.logDebug(__FILE__, __LINE__, __FUNCTION__, msg, ##__VA_ARGS__)


class WorldLog : public Singleton<WorldLog>
{
    public:

        WorldLog();
        ~WorldLog();

        void LogPacket(uint32 len, uint16 opcode, const uint8* data, uint8 direction, uint32 accountid = 0);
        void Enable();
        void Disable();

    private:

        FILE* m_file;
        Mutex mutex;
        bool bEnabled;
};

#define sWorldLog WorldLog::getSingleton()

#endif      //_LOG_H
