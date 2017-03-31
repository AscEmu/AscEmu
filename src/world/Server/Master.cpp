/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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

#include "StdAfx.h"
#include "WorldConf.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Storage/MySQLDataStore.hpp"
#include "WorldRunnable.h"
#include "Server/Console/CConsole.h"
#include "Server/MainServerDefines.h"
#include "Server/Master.h"
#include "CommonScheduleThread.h"
#include "Storage/DayWatcherThread.h"

createFileSingleton(Master);
std::string LogFileName;
bool bLogChat;

volatile bool Master::m_stopEvent = false;

// Database defines.
SERVER_DECL Database* Database_Character;
SERVER_DECL Database* Database_World;

// mainserv defines
SERVER_DECL SessionLogWriter* GMCommand_Log;
SERVER_DECL SessionLogWriter* Anticheat_Log;
SERVER_DECL SessionLogWriter* Player_Log;

// threads
extern DayWatcherThread* dw;
extern CommonScheduleThread* cs;

// DB version
#if VERSION_STRING != Cata
static const char* REQUIRED_CHAR_DB_VERSION = "2017-03-20_01_mailbox";
static const char* REQUIRED_WORLD_DB_VERSION = "2017-02-25_01_gameobject_spawns";
#else
static const char* REQUIRED_CHAR_DB_VERSION = "2017-03-20_01_mailbox";
static const char* REQUIRED_WORLD_DB_VERSION = "2017-03-20_01_trainer_spells";
#endif

void Master::_OnSignal(int s)
{
    switch (s)
    {
#ifndef WIN32
        case SIGHUP:
            sWorld.Rehash(true);
            break;
#endif
        case SIGINT:
        case SIGTERM:
        case SIGABRT:
#ifdef _WIN32
        case SIGBREAK:
#endif
            Master::m_stopEvent = true;
            break;
    }

    signal(s, _OnSignal);
}

Master::Master()
{
    m_ShutdownTimer = 0;
    m_ShutdownEvent = false;
    m_restartEvent = false;
}

Master::~Master()
{}

struct Addr
{
    unsigned short sa_family;
    /* sa_data */
    unsigned short Port;
    unsigned long IP; // inet_addr
    unsigned long unusedA;
    unsigned long unusedB;
};


bool bServerShutdown = false;
bool StartConsoleListener();
void CloseConsoleListener();
ThreadBase* GetConsoleListener();

bool Master::Run(int argc, char** argv)
{
    char* config_file = (char*)CONFDIR "/world.conf";
    char* optional_config_file = (char*)CONFDIR "/optional.conf";
    char* realm_config_file = (char*)CONFDIR "/realms.conf";

    int file_log_level = DEF_VALUE_NOT_SET;
    int screen_log_level = DEF_VALUE_NOT_SET;

    // Zyres: The commandline options (especially the config_file value) is leaking our memory (CID 52872 and 52620). This feature seems to be unfinished.
#ifdef COMMANDLINE_OPT_ENABLE

    int do_check_conf = 0;
    int do_version = 0;
    int do_cheater_check = 0;
    int do_database_clean = 0;

    struct arcemu_option longopts[] =
    {
        { "checkconf", arcemu_no_argument, &do_check_conf, 1 },
        { "screenloglevel", arcemu_required_argument, &screen_log_level, 1 },
        { "fileloglevel", arcemu_required_argument, &file_log_level, 1 },
        { "version", arcemu_no_argument, &do_version, 1 },
        { "conf", arcemu_required_argument, NULL, 'c' },
        { "realmconf", arcemu_required_argument, NULL, 'r' },
        { "databasecleanup", arcemu_no_argument, &do_database_clean, 1 },
        { "cheatercheck", arcemu_no_argument, &do_cheater_check, 1 },
        { 0, 0, 0, 0 }
    };

    char c;
    while ((c = static_cast<char>(arcemu_getopt_long_only(argc, argv, ":f:", longopts, NULL))) != -1)
    {
        switch (c)
        {
            case 'c':
                config_file = new char[strlen(arcemu_optarg) + 1];
                strcpy(config_file, arcemu_optarg);
                break;

            case 'r':
                realm_config_file = new char[strlen(arcemu_optarg) + 1];
                strcpy(realm_config_file, arcemu_optarg);
                break;

            case 0:
                break;
            default:
                Log.Init(0, WORLD_LOG);
                printf("Usage: %s [--checkconf] [--fileloglevel <level>] [--conf <filename>] [--realmconf <filename>] [--version] [--databasecleanup] [--cheatercheck]\n", argv[0]);
                AscLog.~AscEmuLog();
                return true;
        }
    }
#endif
    // Startup banner
    UNIXTIME = time(NULL);
    g_localTime = *localtime(&UNIXTIME);

    AscLog.InitalizeLogFiles("world");

    PrintBanner();

#ifdef COMMANDLINE_OPT_ENABLE
    if (do_version)
    {
        AscLog.~AscEmuLog();
        return true;
    }

    if (do_check_conf)
    {
        LogNotice("Config : Checking config file: %s", config_file);
        if (Config.MainConfig.SetSource(config_file, true))
            LogDetail("Config : Passed world.conf without errors.");
        else
            LOG_ERROR("Encountered one or more errors while loading world.conf.");

        LogNotice("Config : Checking config file: %s", realm_config_file);
        if (Config.RealmConfig.SetSource(realm_config_file, true))
            LogDetail("Config : Passed realm.conf without errors.");
        else
            LOG_ERROR("Encountered one or more errors while loading realm.conf.");

        LogNotice("Config : Checking config file:: %s", optional_config_file);
        if (Config.OptionalConfig.SetSource(optional_config_file, true))
            LogDetail("Config : Passed optional.conf without errors.");
        else
            LOG_ERROR("Encountered one or more errors while loading optional.conf.");

        AscLog.~AscEmuLog();
        return true;
    }
#endif

    LogDefault("The key combination <Ctrl-C> will safely shut down the server.");

#ifndef WIN32
    if (geteuid() == 0 || getegid() == 0)
        AscLog.ConsoleLogMajorError("You are running AscEmu as root.", "This is not needed, and may be a possible security risk.", "It is advised to hit CTRL+C now and", "start as a non-privileged user.");
#endif

    InitImplicitTargetFlags();
    InitRandomNumberGenerators();
    LogNotice("Rnd : Initialized Random Number Generators.");

    ThreadPool.Startup();
    uint32 LoadingTime = getMSTime();

    if (!LoadWorldConfiguration(config_file, optional_config_file, realm_config_file))
    {
        return false;
    }

    if (!_StartDB())
    {
        Database::CleanupLibs();
        AscLog.~AscEmuLog();
        return false;
    }

    if (!_CheckDBVersion())
    {
        AscLog.~AscEmuLog();
        return false;
    }
#ifdef COMMANDLINE_OPT_ENABLE
    if (do_database_clean)
    {
        LogDebug("Entering database maintenance mode.");
        new DatabaseCleaner;
        DatabaseCleaner::getSingleton().Run();
        delete DatabaseCleaner::getSingletonPtr();
        LogDebug("Maintenance finished.");
    }
#endif
    new EventMgr;
    new World;

    OpenCheatLogFiles();

    /* load the config file */
    sWorld.Rehash(false);

#ifdef COMMANDLINE_OPT_ENABLE
    /* set new log levels */
    if (file_log_level != (int)DEF_VALUE_NOT_SET)
        Log.SetFileLoggingLevel(file_log_level);
#endif

    // Initialize Opcode Table
    WorldSession::InitPacketHandlerTable();

    std::string host = Config.MainConfig.GetStringDefault("Listen", "Host", "0.0.0.0");
    int wsport = Config.MainConfig.GetIntDefault("Listen", "WorldServerPort", 8129);

    new ScriptMgr;

    if (!sWorld.SetInitialWorldSettings())
    {
        LOG_ERROR("SetInitialWorldSettings() failed. Something went wrong? Exiting.");
        AscLog.~AscEmuLog();
        return false;
    }

    sWorld.SetStartTime((uint32)UNIXTIME);

    WorldRunnable* wr = new WorldRunnable();
    ThreadPool.ExecuteTask(wr);

    _HookSignals();

    ConsoleThread* console = new ConsoleThread();
    ThreadPool.ExecuteTask(console);

    StartNetworkSubsystem();
    
    sSocketMgr.SpawnWorkerThreads();

    sScriptMgr.LoadScripts();

    if (Config.MainConfig.GetBoolDefault("Startup", "EnableSpellIDDump", false))
        sScriptMgr.DumpUnimplementedSpells();

    LoadingTime = getMSTime() - LoadingTime;
    LogDetail("Server : Ready for connections. Startup time: %ums", LoadingTime);

    ThreadPool.ExecuteTask(new GameEventMgr::GameEventMgrThread());

    StartRemoteConsole();

    WritePidFile();

    //ThreadPool.Gobble();

    /* Connect to realmlist servers / logon servers */
    new LogonCommHandler();
    sLogonCommHandler.Startup();

    // Create listener
    ListenSocket<WorldSocket> * ls = new ListenSocket<WorldSocket>(host.c_str(), wsport);
    bool listnersockcreate = ls->IsOpen();
#ifdef WIN32
    if (listnersockcreate)
        ThreadPool.ExecuteTask(ls);
#endif

    ShutdownThreadPools(listnersockcreate);

    _UnhookSignals();

    wr->SetThreadState(THREADSTATE_TERMINATE);
    ThreadPool.ShowStats();

    /* Shut down console system */
    console->terminate();
    delete console;

    // begin server shutdown

    ShutdownLootSystem();
    
    // send a query to wake it up if its inactive
    LogNotice("Database : Clearing all pending queries...");

    // kill the database thread first so we don't lose any queries/data
    CharacterDatabase.EndThreads();
    WorldDatabase.EndThreads();

    LogNotice("DayWatcherThread : Exiting...");
    dw->terminate();
    dw = NULL;

    LogNotice("CommonScheduleThread : Exiting...");
    cs->terminate();
    cs = NULL;

    ls->Close();

    CloseConsoleListener();
    sWorld.SaveAllPlayers();

    LogNotice("Network : Shutting down network subsystem.");
#ifdef WIN32
    sSocketMgr.ShutdownThreads();
#endif
    sSocketMgr.CloseAll();

    bServerShutdown = true;
    ThreadPool.Shutdown();

    delete ls;

    sWorld.LogoutPlayers();

    delete LogonCommHandler::getSingletonPtr();

    sWorld.ShutdownClasses();
    LogNotice("World : ~World()");
    delete World::getSingletonPtr();

    sScriptMgr.UnloadScripts();
    delete ScriptMgr::getSingletonPtr();

    LogNotice("ChatHandler : ~ChatHandler()");
    delete ChatHandler::getSingletonPtr();

    LogNotice("EventMgr : ~EventMgr()");
    delete EventMgr::getSingletonPtr();

    LogNotice("Database : Closing Connections...");
    _StopDB();

    LogNotice("Network : Deleting Network Subsystem...");
    delete SocketMgr::getSingletonPtr();
    delete SocketGarbageCollector::getSingletonPtr();

    delete GMCommand_Log;
    delete Anticheat_Log;
    delete Player_Log;

    // remove pid
    if (remove("worldserver.pid") != 0)
        LOG_ERROR("Error deleting file worldserver.pid");
    else
        LOG_DEBUG("File worldserver.pid successfully deleted");

    LogDetail("Shutdown : Shutdown complete.");
    AscLog.~AscEmuLog();

#ifdef WIN32
    WSACleanup();

    // Terminate Entire Application
    //HANDLE pH = OpenProcess(PROCESS_TERMINATE, TRUE, GetCurrentProcessId());
    //TerminateProcess(pH, 0);
    //CloseHandle(pH);

#endif

    return true;
}

bool Master::_CheckDBVersion()
{
    QueryResult* wqr = WorldDatabase.QueryNA("SELECT LastUpdate FROM world_db_version;");
    if (wqr == NULL)
    {
        LogError("Database : World database is missing the table `world_db_version` OR the table doesn't contain any rows. Can't validate database version. Exiting.");
        LogError("Database : You may need to update your database");
        return false;
    }

    Field* f = wqr->Fetch();
    const char *WorldDBVersion = f->GetString();

    LogNotice("Database : Last world database update: %s", WorldDBVersion);
    int result = strcmp(WorldDBVersion, REQUIRED_WORLD_DB_VERSION);
    if (result != 0)
    {
        LogError("Database : Last world database update doesn't match the required one which is %s.", REQUIRED_WORLD_DB_VERSION);

        if (result < 0)
        {
            LogError("Database : You need to apply the world update queries that are newer than %s. Exiting.", WorldDBVersion);
            LogError("Database : You can find the world update queries in the sql/world_updates sub-directory of your AscEmu source directory.");
        }
        else
            LogError("Database : Your world database is probably too new for this AscEmu version, you need to update your server. Exiting.");

        delete wqr;
        return false;
    }

    delete wqr;

    QueryResult* cqr = CharacterDatabase.QueryNA("SELECT LastUpdate FROM character_db_version;");
    if (cqr == NULL)
    {
        LogError("Database : Character database is missing the table `character_db_version` OR the table doesn't contain any rows. Can't validate database version. Exiting.");
        LogError("Database : You may need to update your database");
        return false;
    }

    f = cqr->Fetch();
    const char *CharDBVersion = f->GetString();

    LogNotice("Database : Last character database update: %s", CharDBVersion);
    result = strcmp(CharDBVersion, REQUIRED_CHAR_DB_VERSION);
    if (result != 0)
    {
        LogError("Database : Last character database update doesn't match the required one which is %s.", REQUIRED_CHAR_DB_VERSION);
        if (result < 0)
        {
            LogError("Database : You need to apply the character update queries that are newer than %s. Exiting.", CharDBVersion);
            LogError("Database : You can find the character update queries in the sql/character_updates sub-directory of your AscEmu source directory.");
        }
        else
            LogError("Database : Your character database is too new for this AscEmu version, you need to update your server. Exiting.");

        delete cqr;
        return false;
    }

    delete cqr;

    LogDetail("Database : Database successfully validated.");

    return true;
}

bool Master::_StartDB()
{
    Database_World = NULL;
    Database_Character = NULL;
    std::string hostname, username, password, database;
    int port = 0;

    bool wdb_result = Config.MainConfig.GetString("WorldDatabase", "Username", &username);
    wdb_result = !wdb_result ? wdb_result : Config.MainConfig.GetString("WorldDatabase", "Password", &password);
    wdb_result = !wdb_result ? wdb_result : Config.MainConfig.GetString("WorldDatabase", "Hostname", &hostname);
    wdb_result = !wdb_result ? wdb_result : Config.MainConfig.GetString("WorldDatabase", "Name", &database);
    wdb_result = !wdb_result ? wdb_result : Config.MainConfig.GetInt("WorldDatabase", "Port", &port);

    Database_World = Database::CreateDatabaseInterface();

    if (wdb_result == false)
    {
        LogError("Configs : One or more parameters were missing for WorldDatabase connection.");
        return false;
    }

    // Initialize it
    if (!WorldDatabase.Initialize(hostname.c_str(), (unsigned int)port, username.c_str(),
        password.c_str(), database.c_str(), Config.MainConfig.GetIntDefault("WorldDatabase", "ConnectionCount", 3), 16384))
    {
        LogError("Configs : Connection to WorldDatabase failed. Check your database configurations!");
        return false;
    }

    bool cdb_result = Config.MainConfig.GetString("CharacterDatabase", "Username", &username);
    cdb_result = !cdb_result ? cdb_result : Config.MainConfig.GetString("CharacterDatabase", "Password", &password);
    cdb_result = !cdb_result ? cdb_result : Config.MainConfig.GetString("CharacterDatabase", "Hostname", &hostname);
    cdb_result = !cdb_result ? cdb_result : Config.MainConfig.GetString("CharacterDatabase", "Name", &database);
    cdb_result = !cdb_result ? cdb_result : Config.MainConfig.GetInt("CharacterDatabase", "Port", &port);

    Database_Character = Database::CreateDatabaseInterface();

    if (cdb_result == false)
    {
        LogError("Configs : Connection to CharacterDatabase failed. Check your database configurations!");
        return false;
    }

    // Initialize it
    if (!CharacterDatabase.Initialize(hostname.c_str(), (unsigned int)port, username.c_str(),
        password.c_str(), database.c_str(), Config.MainConfig.GetIntDefault("CharacterDatabase", "ConnectionCount", 5), 16384))
    {
        LogError("Configs : Connection to CharacterDatabase failed. Check your database configurations!");
        return false;
    }

    return true;
}

void Master::_StopDB()
{
    if (Database_World != NULL)
        delete Database_World;
    if (Database_Character != NULL)
        delete Database_Character;
    Database::CleanupLibs();
}

void Master::_HookSignals()
{
    signal(SIGINT, _OnSignal);
    signal(SIGTERM, _OnSignal);
    signal(SIGABRT, _OnSignal);
#ifdef _WIN32
    signal(SIGBREAK, _OnSignal);
#else
    signal(SIGHUP, _OnSignal);
    signal(SIGUSR1, _OnSignal);
#endif
}

void Master::_UnhookSignals()
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

#ifdef WIN32

Mutex m_crashedMutex;

// Crash Handler
void OnCrash(bool Terminate)
{
    LogError("Crash Handler : Advanced crash handler initialized.");

    if (!m_crashedMutex.AttemptAcquire())
        TerminateThread(GetCurrentThread(), 0);

    try
    {
        if (World::getSingletonPtr() != 0)
        {
            LogNotice("sql : Waiting for all database queries to finish...");
            WorldDatabase.EndThreads();
            CharacterDatabase.EndThreads();
            LogNotice("sql : All pending database operations cleared.");
            sWorld.SaveAllPlayers();
            LogNotice("sql : Data saved.");
        }
    }
    catch (...)
    {
        LogError("sql : Threw an exception while attempting to save all data.");
    }

    LogNotice("Server : Closing.");

    // beep
    //printf("\x7");

    // Terminate Entire Application
    if (Terminate)
    {
        HANDLE pH = OpenProcess(PROCESS_TERMINATE, TRUE, GetCurrentProcessId());
        TerminateProcess(pH, 1);
        CloseHandle(pH);
    }
}

#endif

void Master::PrintBanner()
{
    AscLog.ConsoleLogDefault(false, "<< AscEmu %s/%s-%s (%s) :: World Server >>", BUILD_HASH_STR, CONFIG, PLATFORM_TEXT, ARCH);
    AscLog.ConsoleLogDefault(false, "========================================================");
    AscLog.ConsoleLogError(true, "<< AscEmu %s/%s-%s (%s) :: World Server >>", BUILD_HASH_STR, CONFIG, PLATFORM_TEXT, ARCH); // Echo off.
    AscLog.ConsoleLogError(true, "========================================================"); // Echo off.
}

bool Master::LoadWorldConfiguration(char* config_file, char* optional_config_file, char* realm_config_file)
{
    LogNotice("Config : Loading Config Files...");
    if (Config.MainConfig.SetSource(config_file))
    {
        LogDetail("Config : " CONFDIR "/world.conf loaded");
    }
    else
    {
        LogError("Config : error occurred loading " CONFDIR "/world.conf");
        AscLog.~AscEmuLog();
        return false;
    }

    if (Config.OptionalConfig.SetSource(optional_config_file))
    {
        LogDetail("Config : " CONFDIR "/optional.conf loaded");
    }
    else
    {
        LogError("Config : error occurred loading " CONFDIR "/optional.conf");
        AscLog.~AscEmuLog();
        return false;
    }

    if (Config.RealmConfig.SetSource(realm_config_file))
    {
        LogDetail("Config : " CONFDIR "/realms.conf loaded");
    }
    else
    {
        LogError("Config : error occurred loading " CONFDIR "/realms.conf");
        AscLog.~AscEmuLog();
        return false;
    }

#if !defined(WIN32) && defined(__DEBUG__)
    if (Config.MainConfig.GetIntDefault("LogLevel", "DisableCrashdumpReport", 0) == 0)
    {
        char cmd[1024];
        char banner[1024];
        snprintf(banner, 1024, BANNER, BUILD_TAG, BUILD_REVISION, CONFIG, PLATFORM_TEXT, ARCH);
        snprintf(cmd, 1024, "./crashreport -r %d -d \'%s\'", BUILD_REVISION, banner);
        system(cmd);
    }
    unlink("worldserver.uptime");
#endif
    return true;
}

void Master::OpenCheatLogFiles()
{
    bool useTimeStamp = Config.MainConfig.GetBoolDefault("log", "TimeStamp", false);

    Anticheat_Log = new SessionLogWriter(AELog::GetFormattedFileName("logs", "cheaters", useTimeStamp).c_str(), false);
    GMCommand_Log = new SessionLogWriter(AELog::GetFormattedFileName("logs", "gmcommand", useTimeStamp).c_str(), false);
    Player_Log = new SessionLogWriter(AELog::GetFormattedFileName("logs", "players", useTimeStamp).c_str(), false);
}

void Master::StartRemoteConsole()
{
    LogNotice("RemoteConsole : Starting...");
    if (StartConsoleListener())
    {
#ifdef WIN32
        ThreadPool.ExecuteTask(GetConsoleListener());
#endif
        LogNotice("RemoteConsole : Now open.");
    }
    else
    {
        LogWarning("RemoteConsole : Not enabled or failed listen.");
    }
}

void Master::WritePidFile()
{
    FILE* fPid = fopen("worldserver.pid", "w");
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

void Master::ShutdownThreadPools(bool listnersockcreate)
{
    // Socket loop!
    time_t curTime;
    uint32 loopcounter = 0;
    auto last_time = Util::TimeNow();
    uint32 next_printout = getMSTime(), next_send = getMSTime();

    while (!m_stopEvent && listnersockcreate)
    {
        auto start = Util::TimeNow();
        auto diff = Util::GetTimeDifference(last_time, start);
        if (!((++loopcounter) % 10000))        // 5mins
        {
            ThreadPool.ShowStats();
            ThreadPool.IntegrityCheck();
#if !defined(WIN32) && defined(__DEBUG__)
            FILE* f = fopen("worldserver.uptime", "w");
            if (f)
            {
                fprintf(f, "%u %u %u %u", sWorld.GetUptime(), sWorld.GetSessionCount(), sWorld.PeakSessionCount, sWorld.mAcceptedConnections);
                fclose(f);
            }
#endif
        }

        /* since time() is an expensive system call, we only update it once per server loop */
        curTime = time(NULL);
        if (UNIXTIME != curTime)
        {
            UNIXTIME = time(NULL);
            g_localTime = *localtime(&curTime);
        }

        sSocketGarbageCollector.Update();

        /* UPDATE */
        auto last_time = Util::TimeNow();
        auto etime = Util::GetTimeDifference(start, last_time);
        if (m_ShutdownEvent)
        {
            if (getMSTime() >= next_printout)
            {
                if (m_ShutdownTimer > 60000.0f)
                {
                    if (!((int)(m_ShutdownTimer) % 60000))
                        LogNotice("Server : Shutdown in %i minutes.", (int)(m_ShutdownTimer / 60000.0f));
                }
                else
                    LogNotice("Server : Shutdown in %i seconds.", (int)(m_ShutdownTimer / 1000.0f));

                next_printout = getMSTime() + 500;
            }

            if (getMSTime() >= next_send)
            {
                int time = m_ShutdownTimer / 1000;
                if ((time % 30 == 0) || time < 10)
                {
                    // broadcast packet.
                    WorldPacket data(20);
                    data.SetOpcode(SMSG_SERVER_MESSAGE);
                    if (m_restartEvent)
                        data << uint32(SERVER_MSG_RESTART_TIME);
                    else
                        data << uint32(SERVER_MSG_SHUTDOWN_TIME);

                    if (time > 0)
                    {
                        int mins = 0, secs = 0;
                        if (time > 60)
                            mins = time / 60;
                        if (mins)
                            time -= (mins * 60);
                        secs = time;
                        char str[20];
                        snprintf(str, 20, "%02u:%02u", mins, secs);
                        data << str;
                        sWorld.SendGlobalMessage(&data, NULL);
                    }
                }
                next_send = getMSTime() + 1000;
            }
            if (diff >= m_ShutdownTimer)
                break;
            else
                m_ShutdownTimer -= diff;
        }

        if (50 > etime)
        {

            Arcemu::Sleep(50 - etime);

        }
    }
}

void Master::StartNetworkSubsystem()
{
    LogNotice("Network : Starting subsystem...");
    new SocketMgr;
    new SocketGarbageCollector;
}

void Master::ShutdownLootSystem()
{
    LogNotice("Shutdown : Initiated at %s", Util::GetDateTimeStringFromTimeStamp((uint32)UNIXTIME).c_str());

    if (lootmgr.is_loading)
    {
        LogNotice("Shutdown : Waiting for loot to finish loading...");
        while (lootmgr.is_loading)
            Arcemu::Sleep(100);
    }
}
