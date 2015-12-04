/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org/>
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
 *
 */

#ifndef _LOGONSERVER_MAIN_H
#define _LOGONSERVER_MAIN_H

#include "../shared/Singleton.h"
#include "LogonServer.hpp"
#include "../shared/AscemuServerDefines.hpp"

extern Arcemu::Threading::AtomicBoolean mrunning;
class AuthSocket;
extern std::set<AuthSocket*> _authSockets;
extern Mutex _authSocketLock;


class LogonServer;
class LogonServer : public Singleton< LogonServer >
{
    public:

        void CheckForDeadSockets();
        void Run(int argc, char** argv);
        void Stop();

        bool StartDb();
        bool Rehash();

        bool IsServerAllowed(unsigned int IP);
        bool IsServerAllowedMod(unsigned int IP);

        void PrintBanner();
        void WritePidFile();

        uint8 sql_hash[20];
        uint32 max_build;
        uint32 min_build;

        Arcemu::PerformanceCounter perfcounter;
    private:

        void _HookSignals();
        void _UnhookSignals();

        static void _OnSignal(int s);
        bool m_stopEvent;
};

#define sLogonServer LogonServer::getSingleton()

#endif      //_LOGONSERVER_MAIN_H
