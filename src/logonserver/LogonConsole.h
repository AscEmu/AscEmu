/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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

#ifndef __LOGONCONSOLE_H
#define __LOGONCONSOLE_H

#include "Common.h"
#include "CThreads.h"

enum checkType
{
    CHECK_LOG_NONE = 0,
    ACC_NAME_DO_EXIST = 1,
    ACC_NAME_NOT_EXIST = 2
};

class LogonConsoleThread : public ThreadBase
{
    public:
    Arcemu::Threading::AtomicBoolean kill;
    LogonConsoleThread();
    ~LogonConsoleThread();
    bool run();
};

class LogonConsole : public Singleton < LogonConsole >
{
    friend class LogonConsoleThread;

    public:                        // Public methods:

        void Kill();

    protected:                    // Protected methods:

        LogonConsoleThread* _thread;

        // Process one command
        void ProcessCmd(char* cmd);

        // quit | exit
        void TranslateQuit(char* str);
        void ProcessQuit(int delay);
        void CancelShutdown(char* str);

        // help | ?
        void TranslateHelp(char* str);
        void ProcessHelp(char* command);

        void ReloadAccts(char* str);
        void TranslateRehash(char* str);

        void NetworkStatus(char* str);

        void Info(char* str);

        //AccountHandling
        void AccountCreate(char* str);
        void AccountDelete(char* str);
        void AccountSetGm(char* str);
        void AccountSetPassword(char* str);
        void AccountChangePassword(char* str);

        void checkAccountName(std::string name, uint8 type);
};

#define sLogonConsole LogonConsole::getSingleton()


#endif      // __LOGONCONSOLE_H
