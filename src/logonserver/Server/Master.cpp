/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "LogonStdAfx.h"
#include <Threading/AEThreadPool.h>
#include "Util.hpp"
#include "DatabaseUpdater.h"
#include "Logon.h"
#include "IpBanMgr.h"
#include "RealmsMgr.h"

using std::chrono::milliseconds;

// Database impl
Database* sLogonSQL;
initialiseSingleton(MasterLogon);
std::atomic<bool> mrunning(true);
Mutex _authSocketLock;
std::set<AuthSocket*> _authSockets;

ConfigMgr Config;

static const char* REQUIRED_LOGON_DB_VERSION = "20180810-00_realms";

void MasterLogon::Run(int /*argc*/, char** /*argv*/)
{
    UNIXTIME = time(nullptr);
    g_localTime = *localtime(&UNIXTIME);

    AscLog.InitalizeLogFiles("logon");

    PrintBanner();

    LogDefault("The key combination <Ctrl-C> will safely shut down the server.");

    new Logon;
    LogDetail("Config : Loading Config Files...");
    if (!LoadLogonConfiguration())
    {
        AscLog.~AscEmuLog();
        return;
    }

    if (!SetLogonConfiguration())
    {
        AscLog.~AscEmuLog();
        return;
    }

    AscLog.SetFileLoggingLevel(logonConfig.logLevel.file);

    LogDetail("ThreadMgr : Starting...");
    ThreadPool.Startup();

    if (!StartDb())
    {
        AscLog.~AscEmuLog();
        return;
    }

    DatabaseUpdater::initBaseIfNeeded(logonConfig.logonDb.db, "logon", *sLogonSQL);

    DatabaseUpdater::checkAndApplyDBUpdatesIfNeeded("logon", *sLogonSQL);

    if (!CheckDBVersion())
    {
        AscLog.~AscEmuLog();
        return;
    }

    new IpBanMgr;
    new AccountMgr;

    new PatchMgr;
    

    new RealmsMgr;
    

    // Spawn periodic function caller thread for account reload every 10mins
    const uint32 accountReloadPeriod = logonConfig.rates.accountRefreshTime * 1000;

    auto periodicReloadAccounts = new PeriodicFunctionCaller<AccountMgr>(AccountMgr::getSingletonPtr(), &AccountMgr::reloadAccountsCallback, accountReloadPeriod);
    ThreadPool.ExecuteTask(periodicReloadAccounts);

    // periodic ping check for realm status
    const auto checkRealmStatusFromPing = new PeriodicFunctionCaller<RealmsMgr>(RealmsMgr::getSingletonPtr(), &RealmsMgr::checkRealmStatus, 60000);
    ThreadPool.ExecuteTask(checkRealmStatusFromPing);

    // Load conf settings..
    clientMinBuild = 5875;
    clientMaxBuild = 15595;

    ThreadPool.ExecuteTask(new LogonConsoleThread);

    new SocketMgr;
    new SocketGarbageCollector;

    auto realmlistSocket = new ListenSocket<AuthSocket>(logonConfig.listen.host.c_str(), logonConfig.listen.realmListPort);
    auto logonServerSocket = new ListenSocket<LogonCommServerSocket>(logonConfig.listen.interServerHost.c_str(), logonConfig.listen.port);

    sSocketMgr.SpawnWorkerThreads();

    // Spawn auth listener
    // Spawn interserver listener
    const bool isAuthsockCreated = realmlistSocket->IsOpen();
    const bool isIntersockCreated = logonServerSocket->IsOpen();
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
                sRealmsMgr.timeoutSockets();
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
    checkRealmStatusFromPing->kill();

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
    delete PatchMgr::getSingletonPtr();
    delete IpBanMgr::getSingletonPtr();
    delete SocketMgr::getSingletonPtr();
    delete SocketGarbageCollector::getSingletonPtr();
    delete periodicReloadAccounts;
    delete realmlistSocket;
    delete logonServerSocket;
    LOG_BASIC("Shutdown complete.");
    AscLog.~AscEmuLog();
}

void OnCrash(bool /*Terminate*/)
{

}

void MasterLogon::CheckForDeadSockets()
{
    _authSocketLock.Acquire();
    time_t t = time(nullptr);
    for (auto itr = _authSockets.begin(); itr != _authSockets.end();)
    {
        auto it2 = itr;
        auto s = (*it2);
        ++itr;

        time_t diff = t - s->GetLastRecv();
        if (diff > 300)           // More than 5mins
        {
            _authSockets.erase(it2);
            s->removedFromSet = true;
            s->Disconnect();
        }
    }
    _authSocketLock.Release();
}

void MasterLogon::PrintBanner()
{
    AscLog.ConsoleLogDefault(false, "<< AscEmu %s/%s-%s (%s) :: Logon Server >>", BUILD_HASH_STR, CONFIG, PLATFORM_TEXT, ARCH);
    AscLog.ConsoleLogDefault(false, "========================================================");
    AscLog.ConsoleLogError(true, "<< AscEmu %s/%s-%s (%s) :: Logon Server >>", BUILD_HASH_STR, CONFIG, PLATFORM_TEXT, ARCH); // Echo off.
    AscLog.ConsoleLogError(true, "========================================================"); // Echo off.
}

void MasterLogon::WritePidFile()
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
        fprintf(pidFile, "%u", static_cast<unsigned int>(pid));
        fclose(pidFile);
    }
}

void MasterLogon::_HookSignals()
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

void MasterLogon::_UnhookSignals()
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

bool MasterLogon::StartDb()
{
    std::string dbHostname = logonConfig.logonDb.host;
    std::string dbUsername = logonConfig.logonDb.user;
    std::string dbPassword = logonConfig.logonDb.password;
    std::string dbDatabase = logonConfig.logonDb.db;

    const int dbPort = logonConfig.logonDb.port;

    // Configure Main Database
    const bool existsUsername = !dbUsername.empty();
    const bool existsPassword = !dbPassword.empty();
    const bool existsHostname = !dbHostname.empty();
    const bool existsName = !dbDatabase.empty();

    const bool result = existsUsername && existsPassword && existsHostname && existsName;

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

            if (!existsHostname)
                errorMessage += "    Hostname\r\n";

            if (!existsUsername)
                errorMessage += "    Username\r\n";

            if (!existsPassword)
                errorMessage += "    Password\r\n";

            if (!existsName)
                errorMessage += "    Name\r\n";
        }

        LOG_ERROR(errorMessage.c_str());
        return false;
    }

    sLogonSQL = Database::CreateDatabaseInterface();

    // Initialize it
    if (!sLogonSQL->Initialize(dbHostname.c_str(), (unsigned int)dbPort, dbUsername.c_str(),
        dbPassword.c_str(), dbDatabase.c_str(), logonConfig.logonDb.connections,
        16384))
    {
        LOG_ERROR("sql: Logon database initialization failed. Exiting.");
        return false;
    }

    return true;
}

bool MasterLogon::CheckDBVersion()
{
    QueryResult* versionQuery = sLogonSQL->QueryNA("SELECT LastUpdate FROM logon_db_version;");
    if (!versionQuery)
    {
        LogError("Database : logon database is missing the table `logon_db_version`. AE will create one for you now!");
        std::string createTable = "CREATE TABLE `logon_db_version` (`LastUpdate` varchar(255) NOT NULL DEFAULT '', PRIMARY KEY(`LastUpdate`)) ENGINE = InnoDB DEFAULT CHARSET = utf8;";
        sLogonSQL->ExecuteNA(createTable.c_str());

        std::string insertData = "INSERT INTO `logon_db_version` VALUES ('20180729-00_logon_db_version');";
        sLogonSQL->ExecuteNA(insertData.c_str());
    }

    QueryResult* cqr = sLogonSQL->QueryNA("SELECT LastUpdate FROM logon_db_version;");
    if (cqr == NULL)
    {
        LogError("Database : logon database is missing the table `logon_db_version` OR the table doesn't contain any rows. Can't validate database version. Exiting.");
        LogError("Database : You may need to update your database");
        return false;
    }

    Field* f = cqr->Fetch();
    const char *LogonDBVersion = f->GetString();

    LogNotice("Database : Last logon database update: %s", LogonDBVersion);
    int result = strcmp(LogonDBVersion, REQUIRED_LOGON_DB_VERSION);
    if (result != 0)
    {
        LogError("Database : Last logon database update doesn't match the required one which is %s.", REQUIRED_LOGON_DB_VERSION);
        if (result < 0)
        {
            LogError("Database : You need to apply the logon update queries that are newer than %s. Exiting.", LogonDBVersion);
            LogError("Database : You can find the logon update queries in the sql/logon/updates sub-directory of your AscEmu source directory.");
        }
        else
            LogError("Database : Your logon database is too new for this AscEmu version, you need to update your server. Exiting.");

        delete cqr;
        return false;
    }

    delete cqr;

    LogDetail("Database : Database successfully validated.");

    return true;
}

Mutex m_allowedIpLock;
std::vector<AllowedIP> m_allowedIps;
std::vector<AllowedIP> m_allowedModIps;

bool MasterLogon::LoadLogonConfiguration()
{
    if (Config.MainConfig.openAndLoadConfigFile(CONFDIR "/logon.conf"))
    {
        LogDetail("Config : " CONFDIR "/logon.conf loaded");
    }
    else
    {
        LogError("Config : error occurred loading " CONFDIR "/logon.conf");
        return false;
    }

    return true;
}

bool MasterLogon::SetLogonConfiguration()
{
    logonConfig.loadConfigValues();

    std::vector<std::string> allowedIPs = Util::SplitStringBySeperator(logonConfig.logonServer.allowedIps, " ");
    std::vector<std::string> allowedModIPs = Util::SplitStringBySeperator(logonConfig.logonServer.allowedModIps, " ");

    m_allowedIpLock.Acquire();
    m_allowedIps.clear();
    m_allowedModIps.clear();

    std::vector<std::string>::iterator itr;

    for (const auto allowedIP : allowedIPs)
    {
        std::string::size_type i = allowedIP.find('/');
        if (i == std::string::npos)
        {
            LOG_ERROR("Ips: %s could not be parsed. Ignoring", allowedIP.c_str());
            continue;
        }

        std::string stmp = allowedIP.substr(0, i);
        std::string smask = allowedIP.substr(i + 1);

        const unsigned int ipraw = MakeIP(stmp.c_str());
        const unsigned char ipmask = static_cast<char>(atoi(smask.c_str()));
        if (ipraw == 0 || ipmask == 0)
        {
            LOG_ERROR("Ips: %s could not be parsed. Ignoring", allowedIP.c_str());
            continue;
        }

        AllowedIP tmp;
        tmp.Bytes = ipmask;
        tmp.IP = ipraw;
        m_allowedIps.push_back(tmp);
    }

    for (const auto allowedModIP : allowedModIPs)
    {
        std::string::size_type i = allowedModIP.find('/');
        if (i == std::string::npos)
        {
            LOG_ERROR("ModIps: %s could not be parsed. Ignoring", allowedModIP.c_str());
            continue;
        }

        std::string stmp = allowedModIP.substr(0, i);
        std::string smask = allowedModIP.substr(i + 1);

        unsigned int ipraw = MakeIP(stmp.c_str());
        unsigned char ipmask = static_cast<char>(atoi(smask.c_str()));
        if (ipraw == 0 || ipmask == 0)
        {
            LOG_ERROR("ModIps: %s could not be parsed. Ignoring", allowedModIP.c_str());
            continue;
        }

        AllowedIP tmp;
        tmp.Bytes = ipmask;
        tmp.IP = ipraw;
        m_allowedModIps.push_back(tmp);
    }

    //\todo always nullptr!
    if (RealmsMgr::getSingletonPtr() != nullptr)
        sRealmsMgr.checkServers();

    m_allowedIpLock.Release();

    return true;
}

bool MasterLogon::IsServerAllowed(unsigned int IP)
{
    m_allowedIpLock.Acquire();
    for (auto itr = m_allowedIps.begin(); itr != m_allowedIps.end(); ++itr)
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

bool MasterLogon::IsServerAllowedMod(unsigned int IP)
{
    m_allowedIpLock.Acquire();
    for (auto itr = m_allowedModIps.begin(); itr != m_allowedModIps.end(); ++itr)
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

void MasterLogon::_OnSignal(int s)
{
    switch (s)
    {
#ifndef WIN32
        case SIGHUP:
        {
            LOG_DETAIL("Received SIGHUP signal, reloading accounts.");
            AccountMgr::getSingleton().reloadAccounts(true);
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
