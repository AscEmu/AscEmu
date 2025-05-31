/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <set>
#include <atomic>
#include <mutex>

class AuthSocket;
class Database;

extern std::unique_ptr<Database> sLogonSQL;

extern std::atomic<bool> mrunning;

class MasterLogon
{
private:
    MasterLogon() = default;
    ~MasterLogon() = default;

public:
    static MasterLogon& getInstance();

    MasterLogon(MasterLogon&&) = delete;
    MasterLogon(MasterLogon const&) = delete;
    MasterLogon& operator=(MasterLogon&&) = delete;
    MasterLogon& operator=(MasterLogon const&) = delete;

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

    uint32_t m_clientMinBuild;
    uint32_t m_clientMaxBuild;

    void addAuthSocket(AuthSocket* _authSocket);
    void removeAuthSocket(AuthSocket* _authSocket);

private:
    void _HookSignals();
    void _UnhookSignals();

    static void _OnSignal(int s);
    bool m_stopEvent;

    std::set<AuthSocket*> m_authSockets;
    std::mutex m_authSocketLock;
};

#define sMasterLogon MasterLogon::getInstance()
