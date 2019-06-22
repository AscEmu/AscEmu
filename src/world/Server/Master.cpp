/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "Server/Console/ConsoleThread.h"
#include "Server/MainServerDefines.h"
#include "Server/Master.h"
#include "Server/BroadcastMgr.h"
#include "Storage/DayWatcherThread.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Management/AddonMgr.h"
#include "Management/AuctionMgr.h"
#include "Spell/SpellTarget.h"
#include "Util.hpp"
#include "DatabaseUpdater.h"

createFileSingleton(Master);
std::string LogFileName;
bool bLogChat;

volatile bool Master::m_stopEvent = false;

// Database defines.
SERVER_DECL Database* Database_Character;
SERVER_DECL Database* Database_World;

// mainserv defines
SERVER_DECL SessionLog* GMCommand_Log;
SERVER_DECL SessionLog* Anticheat_Log;
SERVER_DECL SessionLog* Player_Log;

ConfigMgr Config;

// DB version
static const char* REQUIRED_CHAR_DB_VERSION = "20180916-00_guild_tables";
static const char* REQUIRED_WORLD_DB_VERSION = "20190114-00_spell_custom_override";

void Master::_OnSignal(int s)
{
    switch (s)
    {
#ifndef WIN32
        case SIGHUP:
            sWorld.loadWorldConfigValues(true);
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

std::unique_ptr<WorldRunnable> worldRunnable = nullptr;

/////////////////////////////////////////////////////////////////////////////
// Testscript fo experimental filesystem

#include <fstream>
#include <iostream>
#include <string>

void createExtendedLogDir()
{
    std::string logDir = worldConfig.log.extendedLogsDir;

    if (!logDir.empty())
        fs::create_directories(logDir);
}

bool checkRequiredDirs()
{
    std::vector<std::string> requiredDirs;
    requiredDirs.emplace_back(CONFDIR);
    requiredDirs.emplace_back("dbc");
    requiredDirs.emplace_back("maps");

    std::string dataDir = worldConfig.server.dataDir;
    dataDir.erase(0, 2); //remove ./ from string

    bool requiredDirsExist = true;
    for (const auto& dir : requiredDirs)
    {
        fs::path requiredPath = fs::current_path();

        if (dataDir.empty() || dir == CONFDIR)
        {
            requiredPath /= dir;
        }
        else
        {
            requiredPath /= dataDir;
            requiredPath /= dir;
        }

        if (fs::exists(requiredPath))
        {
            std::cout << "Required dir " << requiredPath << " found!" << std::endl;
        }
        else
        {
            std::cout << "Required dir " << requiredPath << " not found!" << std::endl;
            requiredDirsExist = false;
        }
    }

    return requiredDirsExist;
}

/////////////////////////////////////////////////////////////////////////////

bool Master::Run(int /*argc*/, char** /*argv*/)
{
    char* config_file = (char*)CONFDIR "/world.conf";

    UNIXTIME = time(NULL);
    g_localTime = *localtime(&UNIXTIME);

    AscLog.InitalizeLogFiles("world");

    PrintBanner();

    LogDefault("The key combination <Ctrl-C> will safely shut down the server.");

#ifndef WIN32
    if (geteuid() == 0 || getegid() == 0)
        AscLog.ConsoleLogMajorError("You are running AscEmu as root.", "This is not needed, and may be a possible security risk.", "It is advised to hit CTRL+C now and", "start as a non-privileged user.");
#endif

    InitImplicitTargetFlags();

    ThreadPool.Startup();
    auto startTime = Util::TimeNow();

    new EventMgr;
    new World;

    if (!LoadWorldConfiguration(config_file))
    {
        return false;
    }

    sWorld.loadWorldConfigValues();

    AscLog.SetFileLoggingLevel(worldConfig.log.worldFileLogLevel);
    AscLog.SetDebugFlags(worldConfig.log.worldDebugFlags);

    OpenCheatLogFiles();

    if (!_StartDB())
    {
        Database::CleanupLibs();
        AscLog.~AscEmuLog();
        return false;
    }

    createExtendedLogDir();

    checkRequiredDirs();

    const std::string charDbName = worldConfig.charDb.dbName;
    DatabaseUpdater::initBaseIfNeeded(charDbName, "character", CharacterDatabase);
    DatabaseUpdater::checkAndApplyDBUpdatesIfNeeded("character", CharacterDatabase);

    const std::string worldDbName = worldConfig.worldDb.dbName;
    DatabaseUpdater::initBaseIfNeeded(worldDbName, "world", WorldDatabase);
    DatabaseUpdater::checkAndApplyDBUpdatesIfNeeded("world", WorldDatabase);

    if (!_CheckDBVersion())
    {
        AscLog.~AscEmuLog();
        return false;
    }

    // Initialize Opcode Table
    WorldSession::InitPacketHandlerTable();

    new ScriptMgr;

    if (!sWorld.setInitialWorldSettings())
    {
        LOG_ERROR("SetInitialWorldSettings() failed. Something went wrong? Exiting.");
        AscLog.~AscEmuLog();
        return false;
    }

    sWorld.setWorldStartTime((uint32)UNIXTIME);

    worldRunnable = std::move(std::make_unique<WorldRunnable>());

    _HookSignals();

    ConsoleThread* console = new ConsoleThread();
    ThreadPool.ExecuteTask(console);

    StartNetworkSubsystem();

    sSocketMgr.SpawnWorkerThreads();

    sScriptMgr.LoadScripts();

    sSpellMgr.loadSpellScripts();

    if (worldConfig.startup.enableSpellIdDump)
    {
        sScriptMgr.DumpUnimplementedSpells();
    }

    LogDetail("Server : Ready for connections. Startup time: %u ms", Util::GetTimeDifferenceToNow(startTime));

    ThreadPool.ExecuteTask(new GameEventMgr::GameEventMgrThread());

    StartRemoteConsole();

    WritePidFile();

    if (!ChannelMgr::getSingletonPtr())
        new ChannelMgr;

    channelmgr.seperatechannels = !worldConfig.player.isInterfactionChannelEnabled;

    if (!MailSystem::getSingletonPtr())
        new MailSystem;

    uint32_t mailFlags = 0;

    if (worldConfig.mail.isCostsForGmDisabled)
        mailFlags |= MAIL_FLAG_NO_COST_FOR_GM;

    if (worldConfig.mail.isCostsForEveryoneDisabled)
        mailFlags |= MAIL_FLAG_DISABLE_POSTAGE_COSTS;

    if (worldConfig.mail.isDelayItemsDisabled)
        mailFlags |= MAIL_FLAG_DISABLE_HOUR_DELAY_FOR_ITEMS;

    if (worldConfig.mail.isMessageExpiryDisabled)
        mailFlags |= MAIL_FLAG_NO_EXPIRY;

    if (worldConfig.player.isInterfactionMailEnabled)
        mailFlags |= MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION;

    if (worldConfig.mail.isInterfactionMailForGmEnabled)
        mailFlags |= MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION_GM;

    sMailSystem.config_flags = mailFlags;

    //ThreadPool.Gobble();

    /* Connect to realmlist servers / logon servers */
    new LogonCommHandler();
    sLogonCommHandler.startLogonCommHandler();

    // Create listener
    ListenSocket<WorldSocket> * ls = new ListenSocket<WorldSocket>(worldConfig.listen.listenHost.c_str(), worldConfig.listen.listenPort);
    bool listnersockcreate = ls->IsOpen();
#ifdef WIN32
    if (listnersockcreate)
        ThreadPool.ExecuteTask(ls);
#endif

    ShutdownThreadPools(listnersockcreate);

    _UnhookSignals();

    worldRunnable->threadShutdown();
    worldRunnable = nullptr;

    ThreadPool.ShowStats();
    /* Shut down console system */
    console->stopThread();
    delete console;

    // begin server shutdown

    ShutdownLootSystem();

    // send a query to wake it up if its inactive
    LogNotice("Database : Clearing all pending queries...");

    // kill the database thread first so we don't lose any queries/data
    CharacterDatabase.EndThreads();
    WorldDatabase.EndThreads();

    ls->Close();

    CloseConsoleListener();
    sWorld.saveAllPlayersToDb();

    LogNotice("Network : Shutting down network subsystem.");
#ifdef WIN32
    sSocketMgr.ShutdownThreads();
#endif
    sSocketMgr.CloseAll();

    bServerShutdown = true;
    ThreadPool.Shutdown();

    delete ls;

    sWorld.logoutAllPlayers();

    delete LogonCommHandler::getSingletonPtr();

    LogNotice("AddonMgr : ~AddonMgr()");
#if VERSION_STRING < Cata
    sAddonMgr.SaveToDB();
#endif
    delete AddonMgr::getSingletonPtr();

    LogNotice("AuctionMgr : ~AuctionMgr()");
    delete AuctionMgr::getSingletonPtr();

    LogNotice("LootMgr : ~LootMgr()");
    delete LootMgr::getSingletonPtr();

    LogNotice("MailSystem : ~MailSystem()");
    delete MailSystem::getSingletonPtr();

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
    {
        LOG_ERROR("Error deleting file worldserver.pid");
    }
    else
    {
        LOG_DEBUG("File worldserver.pid successfully deleted");
    }

    LogDetail("Shutdown : Shutdown complete.");
    AscLog.~AscEmuLog();

#ifdef WIN32
    WSACleanup();
#endif

    return true;
}

bool Master::_CheckDBVersion()
{
    QueryResult* wqr = WorldDatabase.QueryNA("SELECT LastUpdate FROM world_db_version ORDER BY id DESC LIMIT 1;");
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
        {
            LogError("Database : Your world database is probably too new for this AscEmu version, you need to update your server. Exiting.");
        }

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
    Database_World = nullptr;
    Database_Character = nullptr;

    bool wdb_result = !worldConfig.worldDb.user.empty();
    wdb_result = !wdb_result ? wdb_result : !worldConfig.worldDb.password.empty();
    wdb_result = !wdb_result ? wdb_result : !worldConfig.worldDb.host.empty();
    wdb_result = !wdb_result ? wdb_result : !worldConfig.worldDb.dbName.empty();
    wdb_result = !wdb_result ? wdb_result : worldConfig.worldDb.port != 0;

    Database_World = Database::CreateDatabaseInterface();

    if (wdb_result == false)
    {
        LogError("Configs : One or more parameters were missing for WorldDatabase connection.");
        return false;
    }

    // Initialize it
    if (!WorldDatabase.Initialize(worldConfig.worldDb.host.c_str(), (unsigned int)worldConfig.worldDb.port, worldConfig.worldDb.user.c_str(),
                                             worldConfig.worldDb.password.c_str(), worldConfig.worldDb.dbName.c_str(), worldConfig.worldDb.connections, 16384))
    {
        LogError("Configs : Connection to WorldDatabase failed. Check your database configurations!");
        return false;
    }

    bool cdb_result = !worldConfig.charDb.user.empty();
    cdb_result = !cdb_result ? cdb_result : !worldConfig.charDb.password.empty();
    cdb_result = !cdb_result ? cdb_result : !worldConfig.charDb.host.empty();
    cdb_result = !cdb_result ? cdb_result : !worldConfig.charDb.dbName.empty();
    cdb_result = !cdb_result ? cdb_result : worldConfig.charDb.port != 0;

    Database_Character = Database::CreateDatabaseInterface();

    if (cdb_result == false)
    {
        LogError("Configs : Connection to CharacterDatabase failed. Check your database configurations!");
        return false;
    }

    // Initialize it
    if (!CharacterDatabase.Initialize(worldConfig.charDb.host.c_str(), (unsigned int)worldConfig.charDb.port, worldConfig.charDb.user.c_str(),
                                                 worldConfig.charDb.password.c_str(), worldConfig.charDb.dbName.c_str(), worldConfig.charDb.connections, 16384))
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
            sWorld.saveAllPlayersToDb();
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

bool Master::LoadWorldConfiguration(char* config_file)
{
    LogNotice("Config : Loading Config Files...");
    if (Config.MainConfig.openAndLoadConfigFile(config_file))
    {
        LogDetail("Config : " CONFDIR "/world.conf loaded");
    }
    else
    {
        LogError("Config : error occurred loading " CONFDIR "/world.conf");
        AscLog.~AscEmuLog();
        return false;
    }

#if !defined(WIN32) && defined(__DEBUG__)
    if (!worldConfig.log.disableCrashdump)
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
    bool useTimeStamp = worldConfig.log.enableTimeStamp;
    std::string logDir = worldConfig.log.extendedLogsDir;

    Anticheat_Log = new SessionLog(AELog::GetFormattedFileName(logDir.c_str(), "cheaters", useTimeStamp).c_str(), false);
    GMCommand_Log = new SessionLog(AELog::GetFormattedFileName(logDir.c_str(), "gmcommands", useTimeStamp).c_str(), false);
    Player_Log = new SessionLog(AELog::GetFormattedFileName(logDir.c_str(), "players", useTimeStamp).c_str(), false);

    if (Anticheat_Log->isSessionLogOpen())
    {
        if (!worldConfig.log.enableCheaterLog)
        {
            Anticheat_Log->closeSessionLog();
        }
    }
    else if (worldConfig.log.enableCheaterLog)
    {
        Anticheat_Log->openSessionLog();
    }

    if (GMCommand_Log->isSessionLogOpen())
    {
        if (!worldConfig.log.enableGmCommandLog)
        {
            GMCommand_Log->closeSessionLog();
        }
    }
    else if (worldConfig.log.enableGmCommandLog)
    {
        GMCommand_Log->openSessionLog();
    }

    if (Player_Log->isSessionLogOpen())
    {
        if (!worldConfig.log.enablePlayerLog)
        {
            Player_Log->closeSessionLog();
        }
    }
    else if (worldConfig.log.enablePlayerLog)
    {
        Player_Log->openSessionLog();
    }
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
    uint32 loopcounter = 0;
    auto last_time = Util::TimeNow();
    uint32 next_printout = Util::getMSTime(), next_send = Util::getMSTime();

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
                fprintf(f, "%u %u %u %u", sWorld.GetUptime(), sWorld.GetSessionCount(), sWorld.getPeakSessionCount(), sWorld.getAcceptedConnections());
                fclose(f);
            }
#endif
        }

        /* since time() is an expensive system call, we only update it once per server loop */
        time_t curTime = time(nullptr);
        if (UNIXTIME != curTime)
        {
            UNIXTIME = time(nullptr);
            g_localTime = *localtime(&curTime);
        }

        sSocketGarbageCollector.Update();

        /* UPDATE */
        last_time = Util::TimeNow();
        auto etime = Util::GetTimeDifference(start, last_time);
        if (m_ShutdownEvent)
        {
            if (Util::getMSTime() >= next_printout)
            {
                if (m_ShutdownTimer > 60000.0f)
                {
                    if (!(static_cast<int>(m_ShutdownTimer) % 60000))
                    LogNotice("Server : Shutdown in %i minutes.", static_cast<int>(m_ShutdownTimer / 60000.0f));
                }
                else
                LogNotice("Server : Shutdown in %i seconds.", static_cast<int>(m_ShutdownTimer / 1000.0f));

                next_printout = Util::getMSTime() + 500;
            }

            if (Util::getMSTime() >= next_send)
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
                        sWorld.sendGlobalMessage(&data);
                    }
                }
                next_send = Util::getMSTime() + 1000;
            }
            if (diff >= m_ShutdownTimer)
                break;
            else
                m_ShutdownTimer -= static_cast<uint32>(diff);
        }

        if (50 > etime)
        {

            Arcemu::Sleep(static_cast<unsigned long>(50 - etime));

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
