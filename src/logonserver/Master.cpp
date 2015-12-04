/*
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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
Arcemu::Threading::AtomicBoolean mrunning(true);
Mutex _authSocketLock;
std::set<AuthSocket*> _authSockets;

void LogonServer::Run(int argc, char** argv)
{
    UNIXTIME = time(NULL);
    g_localTime = *localtime(&UNIXTIME);
    char* config_file = (char*)CONFDIR "/logon.conf";
    int file_log_level = DEF_VALUE_NOT_SET;
    int screen_log_level = DEF_VALUE_NOT_SET;
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
                sLog.Init(0, LOGON_LOG);
                sLog.outBasic("Usage: %s [--checkconf] [--fileloglevel <level>] [--conf <filename>] [--version]", argv[0]);
                return;
        }
    }

    sLog.Init(0, LOGON_LOG);

    PrintBanner();

    if (do_version)
    {
        sLog.Close();
        return;
    }

    if (do_check_conf)
    {
        LOG_BASIC("Checking config file: %s", config_file);
        if (Config.MainConfig.SetSource(config_file, true))
            LOG_BASIC("  Passed without errors.");
        else
            LOG_BASIC("  Encountered one or more errors.");

        sLog.Close();
        delete config_file;
        return;
    }

    /* set new log levels */
    if (file_log_level != (int)DEF_VALUE_NOT_SET)
        sLog.SetFileLoggingLevel(file_log_level);

    sLog.outBasic("The key combination <Ctrl-C> will safely shut down the server.");
    Log.Success("System", "Initializing Random Number Generators...");

    Log.Success("Config", "Loading Config Files...");
    if (!Rehash())
    {
        sLog.Close();
        return;
    }

    Log.Success("ThreadMgr", "Starting...");
    sLog.SetFileLoggingLevel(Config.MainConfig.GetIntDefault("LogLevel", "File", 0));

    ThreadPool.Startup();

    if (!StartDb())
    {
        sLog.Close();
        return;
    }

    Log.Success("AccountMgr", "Starting...");
    new AccountMgr;
    new IPBanner;

    Log.Success("InfoCore", "Starting...");
    new InformationCore;

    new PatchMgr;
    Log.Notice("AccountMgr", "Precaching accounts...");
    sAccountMgr.ReloadAccounts(true);
    Log.Success("AccountMgr", "%u accounts are loaded and ready.", sAccountMgr.GetCount());

    // Spawn periodic function caller thread for account reload every 10mins
    int atime = Config.MainConfig.GetIntDefault("Rates", "AccountRefresh", 600);
    atime *= 1000;
    //SpawnPeriodicCallThread(AccountMgr, AccountMgr::getSingletonPtr(), &AccountMgr::ReloadAccountsCallback, time);
    PeriodicFunctionCaller<AccountMgr> * pfc = new PeriodicFunctionCaller<AccountMgr>(AccountMgr::getSingletonPtr(), &AccountMgr::ReloadAccountsCallback, atime);
    ThreadPool.ExecuteTask(pfc);

    // Load conf settings..
    uint32 cport = Config.MainConfig.GetIntDefault("Listen", "RealmListPort", 3724);
    uint32 sport = Config.MainConfig.GetIntDefault("Listen", "ServerPort", 8093);
    //uint32 threadcount = Config.MainConfig.GetIntDefault("Network", "ThreadCount", 5);
    //uint32 threaddelay = Config.MainConfig.GetIntDefault("Network", "ThreadDelay", 20);
    std::string host = Config.MainConfig.GetStringDefault("Listen", "Host", "0.0.0.0");
    std::string shost = Config.MainConfig.GetStringDefault("Listen", "ISHost", host.c_str());

    min_build = LOGON_MINBUILD;
    max_build = LOGON_MAXBUILD;

    std::string logon_pass = Config.MainConfig.GetStringDefault("LogonServer", "RemotePassword", "r3m0t3b4d");
    Sha1Hash hash;
    hash.UpdateData(logon_pass);
    hash.Finalize();
    memcpy(sql_hash, hash.GetDigest(), 20);

    ThreadPool.ExecuteTask(new LogonConsoleThread);

    new SocketMgr;
    new SocketGarbageCollector;

    ListenSocket<AuthSocket> * cl = new ListenSocket<AuthSocket>(host.c_str(), cport);
    ListenSocket<LogonCommServerSocket> * sl = new ListenSocket<LogonCommServerSocket>(shost.c_str(), sport);

    sSocketMgr.SpawnWorkerThreads();

    // Spawn auth listener
    // Spawn interserver listener
    bool authsockcreated = cl->IsOpen();
    bool intersockcreated = sl->IsOpen();
    if (authsockcreated && intersockcreated)
    {
#ifdef WIN32
        ThreadPool.ExecuteTask(cl);
        ThreadPool.ExecuteTask(sl);
#endif
        _HookSignals();

        WritePidFile();

        uint32 loop_counter = 0;
        //ThreadPool.Gobble();
        sLog.outString("Success! Ready for connections");
        while (mrunning.GetVal())
        {
            if (!(++loop_counter % 20))     // 20 seconds
                CheckForDeadSockets();

            if (!(loop_counter % 300))    // 5mins
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

        sLog.outString("Shutting down...");

        _UnhookSignals();
    }
    else
    {
        LOG_ERROR("Error creating sockets. Shutting down...");
    }

    pfc->kill();

    cl->Close();
    sl->Close();
    sSocketMgr.CloseAll();
#ifdef WIN32
    sSocketMgr.ShutdownThreads();
#endif
    sLogonConsole.Kill();
    delete LogonConsole::getSingletonPtr();

    // kill db
    sLog.outString("Waiting for database to close..");
    sLogonSQL->EndThreads();
    sLogonSQL->Shutdown();
    delete sLogonSQL;

    ThreadPool.Shutdown();

    // delete pid file
    remove("logonserver.pid");

    delete AccountMgr::getSingletonPtr();
    delete InformationCore::getSingletonPtr();
    delete PatchMgr::getSingletonPtr();
    delete IPBanner::getSingletonPtr();
    delete SocketMgr::getSingletonPtr();
    delete SocketGarbageCollector::getSingletonPtr();
    delete pfc;
    delete cl;
    delete sl;
    LOG_BASIC("Shutdown complete.");
    sLog.Close();
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
    sLog.outBasic(LOGON_BANNER, BUILD_HASH_STR, CONFIG, PLATFORM_TEXT, ARCH);
    sLog.outBasic("========================================================");
    sLog.outErrorSilent(LOGON_BANNER, BUILD_HASH_STR, CONFIG, PLATFORM_TEXT, ARCH); // Echo off.
    sLog.outErrorSilent("========================================================"); // Echo off.
}

void LogonServer::WritePidFile()
{
    FILE* fPid = fopen("logonserver.pid", "w");
    if (fPid)
    {
        uint32 pid;
#ifdef WIN32
        pid = GetCurrentProcessId();
#else
        pid = getpid();
#endif
        fprintf(fPid, "%u", (unsigned int)pid);
        fclose(fPid);
    }
}

void LogonServer::_HookSignals()
{
    sLog.outString("Hooking signals...");
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
    std::string lhostname, lusername, lpassword, ldatabase;
    int lport = 0;
    // Configure Main Database

    bool result;

    // Set up reusable parameter checks for each parameter
    // Note that the Config.MainConfig.Get[$type] methods returns boolean value and not $type

    bool existsUsername = Config.MainConfig.GetString("LogonDatabase", "Username", &lusername);
    bool existsPassword = Config.MainConfig.GetString("LogonDatabase", "Password", &lpassword);
    bool existsHostname = Config.MainConfig.GetString("LogonDatabase", "Hostname", &lhostname);
    bool existsName = Config.MainConfig.GetString("LogonDatabase", "Name", &ldatabase);
    bool existsPort = Config.MainConfig.GetInt("LogonDatabase", "Port", &lport);

    // Configure Logon Database...

    // logical AND every parameter to ensure we catch any error
    result = existsUsername && existsPassword && existsHostname && existsName && existsPort;

    if (!result)
    {
        //Build informative error message
        //Built as one string and then printed rather than calling sLog.outString(...) for every line,
        //  as experiments has seen other thread write to the console inbetween calls to sLog.outString(...)
        //  resulting in unreadable error messages.
        //If the <LogonDatabase> tag is malformed, all parameters will fail, and a different error message is given

        std::string errorMessage = "sql: Certain <LogonDatabase> parameters not found in " CONFDIR "\\logon.conf \r\n";
        if (!(existsHostname || existsUsername || existsPassword ||
            existsName || existsPort))
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
            if (!existsPort) { errorMessage += "    Port\r\n"; }
        }

        LOG_ERROR(errorMessage.c_str());
        return false;
    }

    sLogonSQL = Database::CreateDatabaseInterface();

    // Initialize it
    if (!sLogonSQL->Initialize(lhostname.c_str(), (unsigned int)lport, lusername.c_str(),
        lpassword.c_str(), ldatabase.c_str(), Config.MainConfig.GetIntDefault("LogonDatabase", "ConnectionCount", 5),
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

bool LogonServer::Rehash()
{
    char* config_file = (char*)CONFDIR "/logon.conf";
    if (!Config.MainConfig.SetSource(config_file))
    {
        LOG_ERROR("Config file could not be rehashed.");
        return false;
    }

    // re-set the allowed server IP's
    std::string ips = Config.MainConfig.GetStringDefault("LogonServer", "AllowedIPs", "");
    std::string ipsmod = Config.MainConfig.GetStringDefault("LogonServer", "AllowedModIPs", "");

    std::vector<std::string> vips = StrSplit(ips, " ");
    std::vector<std::string> vipsmod = StrSplit(ips, " ");

    m_allowedIpLock.Acquire();
    m_allowedIps.clear();
    m_allowedModIps.clear();
    std::vector<std::string>::iterator itr;
    for (itr = vips.begin(); itr != vips.end(); ++itr)
    {
        std::string::size_type i = itr->find("/");
        if (i == std::string::npos)
        {
            LOG_ERROR("IP: %s could not be parsed. Ignoring", itr->c_str());
            continue;
        }

        std::string stmp = itr->substr(0, i);
        std::string smask = itr->substr(i + 1);

        unsigned int ipraw = MakeIP(stmp.c_str());
        unsigned char ipmask = (char)atoi(smask.c_str());
        if (ipraw == 0 || ipmask == 0)
        {
            LOG_ERROR("IP: %s could not be parsed. Ignoring", itr->c_str());
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
            LOG_ERROR("IP: %s could not be parsed. Ignoring", itr->c_str());
            continue;
        }

        std::string stmp = itr->substr(0, i);
        std::string smask = itr->substr(i + 1);

        unsigned int ipraw = MakeIP(stmp.c_str());
        unsigned char ipmask = (char)atoi(smask.c_str());
        if (ipraw == 0 || ipmask == 0)
        {
            LOG_ERROR("IP: %s could not be parsed. Ignoring", itr->c_str());
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
            mrunning.SetVal(false);
            break;
    }

    signal(s, _OnSignal);
}
