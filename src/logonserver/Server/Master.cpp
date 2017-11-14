/*
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "LogonStdAfx.h"

// Database impl
Database* sLogonSQL;
initialiseSingleton(LogonServer);
std::atomic<bool> mrunning(true);
Mutex _authSocketLock;
std::set<AuthSocket*> _authSockets;

ConfigMgr Config;

void LogonServer::Run(int argc, char** argv)
{
    UNIXTIME = time(NULL);
    g_localTime = *localtime(&UNIXTIME);

    char* config_file = (char*)CONFDIR "/logon.conf";

    int file_log_level = DEF_VALUE_NOT_SET;
    int screen_log_level = DEF_VALUE_NOT_SET;

    // Zyres: The commandline options (especially the config_file value) is leaking our memory (CID 52921). This feature seems to be unfinished.
#ifdef COMMANDLINE_OPT_ENABLE

    int do_check_conf = 0;
    int do_version = 0;

    struct arcemu_option longopts[] =
    {
        { "checkconf",          arcemu_no_argument,              &do_check_conf,         1 },
        { "screenloglevel",     arcemu_required_argument,        &screen_log_level,      1 },
        { "fileloglevel",       arcemu_required_argument,        &file_log_level,        1 },
        { "version",            arcemu_no_argument,              &do_version,            1 },
        { "conf",               arcemu_required_argument,        NULL,                  'c' },
        { 0, 0, 0, 0 }
    };

    int c;
    while ((c = arcemu_getopt_long_only(argc, argv, ":f:", longopts, NULL)) != -1)
    {
        switch (c)
        {
            case 'c':
                /* Log filename was set */
                config_file = new char[strlen(arcemu_optarg) + 1];
                strcpy(config_file, arcemu_optarg);
                break;
            case 0:
                break;
            default:
                return;
        }
    }
#endif
    AscLog.InitalizeLogFiles("logon");

    PrintBanner();

#ifdef COMMANDLINE_OPT_ENABLE
    if (do_version)
    {
        AscLog.~AscEmuLog();
        return;
    }

    if (do_check_conf)
    {
        LOG_BASIC("Checking config file: %s", config_file.c_str());
        if (Config.MainConfig.SetSource(config_file, true))
            LOG_BASIC("  Passed without errors.");
        else
            LOG_BASIC("  Encountered one or more errors.");

        AscLog.~AscEmuLog();
        return;
    }

    // set new log levels
    if (file_log_level != (int)DEF_VALUE_NOT_SET)
        AscLog.SetFileLoggingLevel(file_log_level);
#endif

    LogDefault("The key combination <Ctrl-C> will safely shut down the server.");

    LogDetail("Config : Loading Config Files...");
    if (!LoadLogonConfiguration())
    {
        AscLog.~AscEmuLog();
        return;
    }

    AscLog.SetFileLoggingLevel(Config.MainConfig.getIntDefault("LogLevel", "File", 0));

    LogDetail("ThreadMgr : Starting...");
    ThreadPool.Startup();

    if (!StartDb())
    {
        AscLog.~AscEmuLog();
        return;
    }

    LogDetail("AccountMgr : Starting...");
    new AccountMgr;
    new IPBanner;

    LogDetail("InfoCore : Starting...");
    new InformationCore;

    new PatchMgr;
    LogNotice("AccountMgr : Precaching accounts...");
    sAccountMgr.ReloadAccounts(true);
    LogDetail("AccountMgr : %u accounts are loaded and ready.", sAccountMgr.GetCount());

    // Spawn periodic function caller thread for account reload every 10mins
    uint32 accountReloadPeriod = Config.MainConfig.getIntDefault("Rates", "AccountRefresh", 600);
    accountReloadPeriod *= 1000;

    PeriodicFunctionCaller<AccountMgr> * periodicReloadAccounts = new PeriodicFunctionCaller<AccountMgr>(AccountMgr::getSingletonPtr(), &AccountMgr::ReloadAccountsCallback, accountReloadPeriod);
    ThreadPool.ExecuteTask(periodicReloadAccounts);

    // Load conf settings..
    uint32 realmlistPort = Config.MainConfig.getIntDefault("Listen", "RealmListPort", 3724);
    uint32 logonServerPort = Config.MainConfig.getIntDefault("Listen", "ServerPort", 8093);

    std::string host = Config.MainConfig.getStringDefault("Listen", "Host", "0.0.0.0");
    std::string shost = Config.MainConfig.getStringDefault("Listen", "ISHost", host.c_str());

    clientMinBuild = 5875;
    clientMaxBuild = 15595;

    std::string logonServerPassword = Config.MainConfig.getStringDefault("LogonServer", "RemotePassword", "r3m0t3b4d");
    Sha1Hash hash;
    hash.UpdateData(logonServerPassword);
    hash.Finalize();
    memcpy(sql_hash, hash.GetDigest(), 20);

    ThreadPool.ExecuteTask(new LogonConsoleThread);

    new SocketMgr;
    new SocketGarbageCollector;

    ListenSocket<AuthSocket> * realmlistSocket = new ListenSocket<AuthSocket>(host.c_str(), realmlistPort);
    ListenSocket<LogonCommServerSocket> * logonServerSocket = new ListenSocket<LogonCommServerSocket>(shost.c_str(), logonServerPort);

    sSocketMgr.SpawnWorkerThreads();

    // Spawn auth listener
    // Spawn interserver listener
    bool isAuthsockCreated = realmlistSocket->IsOpen();
    bool isIntersockCreated = logonServerSocket->IsOpen();
    if (isAuthsockCreated && isIntersockCreated)
    {
#ifdef WIN32
        ThreadPool.ExecuteTask(realmlistSocket);
        ThreadPool.ExecuteTask(logonServerSocket);
#endif
        _HookSignals();

        WritePidFile();

        uint32 loop_counter = 0;

        LogDefault("Success! Ready for connections");
        while (mrunning)
        {
            if (!(++loop_counter % 20))             // 20 seconds
                CheckForDeadSockets();

            if (!(loop_counter % 300))              // 5mins
                ThreadPool.IntegrityCheck();

            if (!(loop_counter % 5))
            {
                sInfoCore.TimeoutSockets();
                sSocketGarbageCollector.Update();
                CheckForDeadSockets();              // Flood Protection
                UNIXTIME = time(NULL);
                g_localTime = *localtime(&UNIXTIME);
            }

            PatchMgr::getSingleton().UpdateJobs();
            Arcemu::Sleep(1000);
        }

        LogDefault("Shutting down...");

        _UnhookSignals();
    }
    else
    {
        LOG_ERROR("Error creating sockets. Shutting down...");
    }

    periodicReloadAccounts->kill();

    realmlistSocket->Close();
    logonServerSocket->Close();
    sSocketMgr.CloseAll();
#ifdef WIN32
    sSocketMgr.ShutdownThreads();
#endif
    sLogonConsole.Kill();
    delete LogonConsole::getSingletonPtr();

    // kill db
    LogDefault("Waiting for database to close..");
    sLogonSQL->EndThreads();
    sLogonSQL->Shutdown();
    delete sLogonSQL;

    ThreadPool.Shutdown();

    // delete pid file
    if (remove("logonserver.pid") != 0)
        LOG_ERROR("Error deleting file logonserver.pid");
    else
        LOG_DEBUG("File logonserver.pid successfully deleted");

    delete AccountMgr::getSingletonPtr();
    delete InformationCore::getSingletonPtr();
    delete PatchMgr::getSingletonPtr();
    delete IPBanner::getSingletonPtr();
    delete SocketMgr::getSingletonPtr();
    delete SocketGarbageCollector::getSingletonPtr();
    delete periodicReloadAccounts;
    delete realmlistSocket;
    delete logonServerSocket;
    LOG_BASIC("Shutdown complete.");
    AscLog.~AscEmuLog();
}

void OnCrash(bool Terminate)
{

}

void LogonServer::CheckForDeadSockets()
{
    _authSocketLock.Acquire();
    time_t t = time(NULL);
    time_t diff;
    std::set<AuthSocket*>::iterator itr = _authSockets.begin();
    std::set<AuthSocket*>::iterator it2;
    AuthSocket* s;

    for (itr = _authSockets.begin(); itr != _authSockets.end();)
    {
        it2 = itr;
        s = (*it2);
        ++itr;

        diff = t - s->GetLastRecv();
        if (diff > 300)           // More than 5mins
        {
            _authSockets.erase(it2);
            s->removedFromSet = true;
            s->Disconnect();
        }
    }
    _authSocketLock.Release();
}

void LogonServer::PrintBanner()
{
    AscLog.ConsoleLogDefault(false, "<< AscEmu %s/%s-%s (%s) :: Logon Server >>", BUILD_HASH_STR, CONFIG, PLATFORM_TEXT, ARCH);
    AscLog.ConsoleLogDefault(false, "========================================================");
    AscLog.ConsoleLogError(true, "<< AscEmu %s/%s-%s (%s) :: Logon Server >>", BUILD_HASH_STR, CONFIG, PLATFORM_TEXT, ARCH); // Echo off.
    AscLog.ConsoleLogError(true, "========================================================"); // Echo off.
}

void LogonServer::WritePidFile()
{
    FILE* pidFile = fopen("logonserver.pid", "w");
    if (pidFile)
    {
        uint32 pid;
#ifdef WIN32
        pid = GetCurrentProcessId();
#else
        pid = getpid();
#endif
        fprintf(pidFile, "%u", (unsigned int)pid);
        fclose(pidFile);
    }
}

void LogonServer::_HookSignals()
{
    LogDefault("Hooking signals...");
    signal(SIGINT, _OnSignal);
    signal(SIGTERM, _OnSignal);
    signal(SIGABRT, _OnSignal);
#ifdef _WIN32
    signal(SIGBREAK, _OnSignal);
#else
    signal(SIGHUP, _OnSignal);
#endif
}

void LogonServer::_UnhookSignals()
{
    signal(SIGINT, 0);
    signal(SIGTERM, 0);
    signal(SIGABRT, 0);
#ifdef _WIN32
    signal(SIGBREAK, 0);
#else
    signal(SIGHUP, 0);
#endif
}

bool LogonServer::StartDb()
{
    std::string dbHostname = Config.MainConfig.getStringDefault("LogonDatabase", "Hostname", "");
    std::string dbUsername = Config.MainConfig.getStringDefault("LogonDatabase", "Username", "");
    std::string dbPassword = Config.MainConfig.getStringDefault("LogonDatabase", "Password", "");
    std::string dbDatabase = Config.MainConfig.getStringDefault("LogonDatabase", "Name", "");

    int dbPort = Config.MainConfig.getIntDefault("LogonDatabase", "Port", 3306);

    // Configure Main Database
    bool existsUsername = !dbUsername.empty();
    bool existsPassword = !dbPassword.empty();
    bool existsHostname = !dbHostname.empty();
    bool existsName = !dbDatabase.empty();

    bool result = existsUsername && existsPassword && existsHostname && existsName;

    if (!result)
    {
        //Build informative error message
        //Built as one string and then printed rather than calling Log.outString(...) for every line,
        //  as experiments has seen other thread write to the console inbetween calls to Log.outString(...)
        //  resulting in unreadable error messages.
        //If the <LogonDatabase> tag is malformed, all parameters will fail, and a different error message is given

        std::string errorMessage = "sql: Certain <LogonDatabase> parameters not found in " CONFDIR "\\logon.conf \r\n";
        if (!(existsHostname || existsUsername || existsPassword || existsName))
        {
            errorMessage += "  Double check that you have remembered the entire <LogonDatabase> tag.\r\n";
            errorMessage += "  All parameters missing. It is possible you forgot the first '<' character.\r\n";
        }
        else
        {
            errorMessage += "  Missing paramer(s):\r\n";
            if (!existsHostname) { errorMessage += "    Hostname\r\n"; }
            if (!existsUsername) { errorMessage += "    Username\r\n"; }
            if (!existsPassword) { errorMessage += "    Password\r\n"; }
            if (!existsName) { errorMessage += "    Name\r\n"; }
        }

        LOG_ERROR(errorMessage.c_str());
        return false;
    }

    sLogonSQL = Database::CreateDatabaseInterface();

    // Initialize it
    if (!sLogonSQL->Initialize(dbHostname.c_str(), (unsigned int)dbPort, dbUsername.c_str(),
        dbPassword.c_str(), dbDatabase.c_str(), Config.MainConfig.getIntDefault("LogonDatabase", "ConnectionCount", 5),
        16384))
    {
        LOG_ERROR("sql: Logon database initialization failed. Exiting.");
        return false;
    }

    return true;
}

Mutex m_allowedIpLock;
std::vector<AllowedIP> m_allowedIps;
std::vector<AllowedIP> m_allowedModIps;

bool LogonServer::LoadLogonConfiguration()
{
    char* config_file = (char*)CONFDIR "/logon.conf";
    if (!Config.MainConfig.openAndLoadConfigFile(config_file))
    {
        LOG_ERROR("Config file could not be rehashed.");
        return false;
    }

    // re-set the allowed server IP's
    std::string allowedIps = Config.MainConfig.getStringDefault("LogonServer", "AllowedIPs", "");
    std::vector<std::string> vips = Util::SplitStringBySeperator(allowedIps, " ");

    std::string allowedModIps = Config.MainConfig.getStringDefault("LogonServer", "AllowedModIPs", "");
    std::vector<std::string> vipsmod = Util::SplitStringBySeperator(allowedModIps, " ");

    m_allowedIpLock.Acquire();
    m_allowedIps.clear();
    m_allowedModIps.clear();

    std::vector<std::string>::iterator itr;

    for (itr = vips.begin(); itr != vips.end(); ++itr)
    {
        std::string::size_type i = itr->find("/");
        if (i == std::string::npos)
        {
            LOG_ERROR("Ips: %s could not be parsed. Ignoring", itr->c_str());
            continue;
        }

        std::string stmp = itr->substr(0, i);
        std::string smask = itr->substr(i + 1);

        unsigned int ipraw = MakeIP(stmp.c_str());
        unsigned char ipmask = (char)atoi(smask.c_str());
        if (ipraw == 0 || ipmask == 0)
        {
            LOG_ERROR("Ips: %s could not be parsed. Ignoring", itr->c_str());
            continue;
        }

        AllowedIP tmp;
        tmp.Bytes = ipmask;
        tmp.IP = ipraw;
        m_allowedIps.push_back(tmp);
    }

    for (itr = vipsmod.begin(); itr != vipsmod.end(); ++itr)
    {
        std::string::size_type i = itr->find("/");
        if (i == std::string::npos)
        {
            LOG_ERROR("ModIps: %s could not be parsed. Ignoring", itr->c_str());
            continue;
        }

        std::string stmp = itr->substr(0, i);
        std::string smask = itr->substr(i + 1);

        unsigned int ipraw = MakeIP(stmp.c_str());
        unsigned char ipmask = (char)atoi(smask.c_str());
        if (ipraw == 0 || ipmask == 0)
        {
            LOG_ERROR("ModIps: %s could not be parsed. Ignoring", itr->c_str());
            continue;
        }

        AllowedIP tmp;
        tmp.Bytes = ipmask;
        tmp.IP = ipraw;
        m_allowedModIps.push_back(tmp);
    }

    if (InformationCore::getSingletonPtr() != NULL)
        sInfoCore.CheckServers();

    m_allowedIpLock.Release();

    return true;
}

bool LogonServer::IsServerAllowed(unsigned int IP)
{
    m_allowedIpLock.Acquire();
    for (std::vector<AllowedIP>::iterator itr = m_allowedIps.begin(); itr != m_allowedIps.end(); ++itr)
    {
        if (ParseCIDRBan(IP, itr->IP, itr->Bytes))
        {
            m_allowedIpLock.Release();
            return true;
        }
    }
    m_allowedIpLock.Release();
    return false;
}

bool LogonServer::IsServerAllowedMod(unsigned int IP)
{
    m_allowedIpLock.Acquire();
    for (std::vector<AllowedIP>::iterator itr = m_allowedModIps.begin(); itr != m_allowedModIps.end(); ++itr)
    {
        if (ParseCIDRBan(IP, itr->IP, itr->Bytes))
        {
            m_allowedIpLock.Release();
            return true;
        }
    }
    m_allowedIpLock.Release();
    return false;
}

void LogonServer::_OnSignal(int s)
{
    switch (s)
    {
#ifndef WIN32
        case SIGHUP:
        {
            LOG_DETAIL("Received SIGHUP signal, reloading accounts.");
            AccountMgr::getSingleton().ReloadAccounts(true);
        }
        break;
#endif
        case SIGINT:
        case SIGTERM:
        case SIGABRT:
#ifdef _WIN32
        case SIGBREAK:
#endif
            mrunning = false;
            break;
    }

    signal(s, _OnSignal);
}
