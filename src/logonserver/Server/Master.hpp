/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../shared/Singleton.h"
#include "LogonServerDefines.hpp"
#include "../shared/AscemuServerDefines.hpp"

extern std::atomic<bool> mrunning;
class AuthSocket;
extern std::set<AuthSocket*> _authSockets;
extern Mutex _authSocketLock;

class MasterLogon;
class MasterLogon : public Singleton<MasterLogon>
{
    public:

        bool LoadLogonConfiguration();
        void CheckForDeadSockets();
        void Run(int argc, char** argv);
        void Stop();

        bool StartDb();
        bool CheckDBVersion();
        bool SetLogonConfiguration();

        bool IsServerAllowed(unsigned int IP);
        bool IsServerAllowedMod(unsigned int IP);

        void PrintBanner();
        void WritePidFile();

        uint32 clientMinBuild;
        uint32 clientMaxBuild;

    private:

        void _HookSignals();
        void _UnhookSignals();

        static void _OnSignal(int s);
        bool m_stopEvent;
};

#define sMasterLogon MasterLogon::getSingleton()
