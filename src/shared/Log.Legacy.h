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
#include "LogDefines.hpp"
#include "Singleton.h"

class WorldPacket;
class WorldSession;

extern SERVER_DECL time_t UNIXTIME;        //update this every loop to avoid the time() syscall!
extern SERVER_DECL tm g_localTime;


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
