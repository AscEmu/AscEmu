/*
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
#include <Threading/AEThreadPool.h>
#include "Util.hpp"

using AscEmu::Threading::AEThread;
using AscEmu::Threading::AEThreadPool;
using std::chrono::milliseconds;

// Database impl
Database* sLogonSQL;
initialiseSingleton(LogonServer);
std::atomic<bool> mrunning(true);
Mutex _authSocketLock;
std::set<AuthSocket*> _authSockets;

ConfigMgr Config;

// MIT start
static const char* REQUIRED_LOGON_DB_VERSION = "20180729-00_logon_db_version";

#ifdef USE_EXPERIMENTAL_FILESYSTEM

#include <fstream>
#include <iostream>
#include <string>

struct DatabaseUpdateFile
{
    std::string fullName;
    uint32_t majorVersion;
    uint32_t minorVersion;
};

void applyUpdatesForDatabase(std::string database)
{
    const std::string sqlUpdateDir = "sql/" + database + "/updates";

    //////////////////////////////////////////////////////////////////////////////////////////
    // 1. get current version
    QueryResult* result;

    if (database == "logon")
        result = sLogonSQL->Query("SELECT LastUpdate FROM logon_db_version ORDER BY LastUpdate DESC LIMIT 1");

    if (!result)
    {
        LogError("%s_db_version query failed!", database.c_str());
        return;
    }

    Field* fields = result->Fetch();
    const std::string dbLastUpdate = fields[0].GetString();

    LogDetail(" %s Database Version : %s", database.c_str(), dbLastUpdate.c_str());

    const auto lastUpdateMajor = Util::readMajorVersionFromString(dbLastUpdate);
    const auto lastUpdateMinor = Util::readMinorVersionFromString(dbLastUpdate);

    //////////////////////////////////////////////////////////////////////////////////////////
    // 2. check if update folder exist in *dir*/sql/
    std::map<uint32_t, DatabaseUpdateFile> updateSqlStore;

    uint32_t count = 0;
    for (auto& p : fs::recursive_directory_iterator(sqlUpdateDir))
    {
        const std::string filePathName = p.path().string();

        std::string fileName = filePathName;
        fileName.erase(0, sqlUpdateDir.size() + 1);

        const uint32_t majorVersion = Util::readMajorVersionFromString(fileName);
        const uint32_t minorVersion = Util::readMinorVersionFromString(fileName);

        DatabaseUpdateFile dbUpdateFile;
        dbUpdateFile.fullName = filePathName;
        dbUpdateFile.majorVersion = majorVersion;
        dbUpdateFile.minorVersion = minorVersion;

        //\todo Remove me
        LogDetail("Available file in updates dir: %s", filePathName.c_str());

        updateSqlStore.insert(std::pair<uint32_t, DatabaseUpdateFile>(count, dbUpdateFile));
        ++count;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // 3. save filenames into vector, when newer than current db version
    std::map<uint32_t, DatabaseUpdateFile> applyNewUpdateFilesStore;

    if (!updateSqlStore.empty())
    {
        LogDebug("=========== New %s update files in %s ===========", database.c_str(), sqlUpdateDir.c_str());
        //compare it with latest update in mysql
        for (const auto update : updateSqlStore)
        {
            bool addToUpdateFiles = false;
            if (update.second.majorVersion == lastUpdateMajor && update.second.minorVersion > lastUpdateMinor)
                addToUpdateFiles = true;

            if (update.second.majorVersion > lastUpdateMajor)
                addToUpdateFiles = true;

            if (addToUpdateFiles)
            {
                applyNewUpdateFilesStore.insert(update);
                LogDebug("Updatefile %s, Major(%u), Minor(%u) - added and ready to be applied!", update.second.fullName.c_str(), update.second.majorVersion, update.second.minorVersion);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // 4. open/parse files and apply to db
    if (!applyNewUpdateFilesStore.empty())
    {
        LogDebugFlag(LF_DB_TABLES, "=========== Applying sql updates from %s ===========", sqlUpdateDir.c_str());

        for (const auto execute : applyNewUpdateFilesStore)
        {
            const fs::path sqlFile = fs::current_path() /= execute.second.fullName;

            if (fs::exists(sqlFile))
            {
                LogDebugFlag(LF_DB_TABLES, "%s", execute.second.fullName.c_str());
                std::string loadedFile = Util::readFileIntoString(sqlFile);

                // split into seperated string
                std::vector<std::string> seglist;
                std::string delimiter = ";\n";

                size_t pos = 0;
                std::string token;
                while ((pos = loadedFile.find(delimiter)) != std::string::npos)
                {
                    token = loadedFile.substr(0, pos);
                    seglist.push_back(token + ";");
                    loadedFile.erase(0, pos + delimiter.length());
                }

                for (const auto& statements : seglist)
                {
                    if (database == "logon")
                        if (sLogonSQL->WaitExecuteNA(statements.c_str()))
                            continue;
                }
            }
        }
    }
}

void setupDatabase(std::string database)
{
    const std::string sqlBaseDir = "sql/" + database;
    fs::path baseFilePath = fs::current_path();
    baseFilePath /= sqlBaseDir + "/logon_structure.sql";

    if (fs::exists(baseFilePath))
    {
        LogDebugFlag(LF_DB_TABLES, "%s", baseFilePath.c_str());
        std::string loadedFile = Util::readFileIntoString(baseFilePath);

        // split into seperated string
        std::vector<std::string> seglist;
        std::string delimiter = ";\n";

        size_t pos = 0;
        std::string token;
        while ((pos = loadedFile.find(delimiter)) != std::string::npos)
        {
            token = loadedFile.substr(0, pos);
            seglist.push_back(token + ";");
            loadedFile.erase(0, pos + delimiter.length());
        }

        for (const auto& statements : seglist)
            sLogonSQL->ExecuteNA(statements.c_str());
    }
}
#endif

// MIT end

void LogonServer::Run(int /*argc*/, char** /*argv*/)
{
    UNIXTIME = time(NULL);
    g_localTime = *localtime(&UNIXTIME);

    AscLog.InitalizeLogFiles("logon");

    PrintBanner();

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

#ifdef USE_EXPERIMENTAL_FILESYSTEM
    {
        std::string dbName = Config.MainConfig.getStringDefault("LogonDatabase", "Name", "");
        QueryResult* dbResult = sLogonSQL->Query("SHOW TABLES FROM %s", dbName.c_str());
        if (dbResult == nullptr)
        {
            LogDetail("Database: Your Database %s has no tables. AE is setting up the database for you.", dbName.c_str());
            setupDatabase("logon");
        }

        while (sLogonSQL->GetQueueSize() > 0)
        {
            LogDetail("-- busy creating database. Waiting for %u queries to be executed.", sLogonSQL->GetQueueSize());
            Arcemu::Sleep(500);
        }
    }

    {
        applyUpdatesForDatabase("logon");

        while (sLogonSQL->GetQueueSize() > 0)
        {
            LogDetail("-- busy updating database. Waiting for %u queries to be executed.", sLogonSQL->GetQueueSize());
            Arcemu::Sleep(500);
        }
    }

    // now we are ready to check the db version.

#endif

    if (!CheckDBVersion())
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

    new RealmsMgr;
    sRealmsMgr.LoadRealms();
    LogDetail("Loaded %u realms definitisons.", static_cast<uint32_t>(sRealmsMgr._realmStore.size()));

    // Spawn periodic function caller thread for account reload every 10mins
    uint32 accountReloadPeriod = Config.MainConfig.getIntDefault("Rates", "AccountRefresh", 600);
    accountReloadPeriod *= 1000;

    PeriodicFunctionCaller<AccountMgr> * periodicReloadAccounts = new PeriodicFunctionCaller<AccountMgr>(AccountMgr::getSingletonPtr(), &AccountMgr::ReloadAccountsCallback, accountReloadPeriod);
    ThreadPool.ExecuteTask(periodicReloadAccounts);
    //AEThreadPool::globalThreadPool()->queueRecurringTask([](AEThread&) {AccountMgr::getSingletonPtr()->ReloadAccountsCallback();}, milliseconds(accountReloadPeriod), "Reload Accounts");

    // periodic ping check for realm status
    PeriodicFunctionCaller<RealmsMgr> * checkRealmStatusFromPing = new PeriodicFunctionCaller<RealmsMgr>(RealmsMgr::getSingletonPtr(), &RealmsMgr::checkRealmStatus, 60000);
    ThreadPool.ExecuteTask(checkRealmStatusFromPing);

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

void OnCrash(bool /*Terminate*/)
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

bool LogonServer::CheckDBVersion()
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
