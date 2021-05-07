/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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
#include "Database/DatabaseUpdater.hpp"
#include "Packets/SmsgServerMessage.h"
#include "OpcodeTable.hpp"

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
static const char* REQUIRED_CHAR_DB_VERSION = "20201216-00_rename_event_properties";
static const char* REQUIRED_WORLD_DB_VERSION = "20210501-02_creature_spawn";

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

Master& Master::getInstance()
{
    static Master mInstance;
    return mInstance;
}

void Master::initialize()
{
    m_ShutdownTimer = 0;
    m_ShutdownEvent = false;
    m_restartEvent = false;
}

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
    Util::BenchmarkTime benchmark;
    std::string logDir = worldConfig.logger.extendedLogsDir;

    if (!logDir.empty())
        fs::create_directories(logDir);
}

void checkRequiredDirs()
{
    std::vector<std::string> requiredDirs;
    requiredDirs.emplace_back(CONFDIR);
    requiredDirs.emplace_back("dbc");
    requiredDirs.emplace_back("maps");

    if (worldConfig.terrainCollision.isCollisionEnabled)
        requiredDirs.emplace_back("vmaps");

    if (worldConfig.terrainCollision.isPathfindingEnabled)
    {
        // Check that vertical maps are also enabled
        if (!worldConfig.terrainCollision.isCollisionEnabled)
        {
            sLogger.failure("Pathfinding is enabled but collision is disabled. Disabling pathfinding.");
            worldConfig.terrainCollision.isPathfindingEnabled = false;

            // Give user a chance to read the error message
            Arcemu::Sleep(2000);
        }
        else
        {
            requiredDirs.emplace_back("mmaps");
        }
    }

    std::string dataDir = worldConfig.server.dataDir;
    dataDir.erase(0, 2); //remove ./ from string

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
            sLogger.info("Required dir %s found!", requiredPath.u8string().c_str());
        }
        else
        {
            if (dir == "mmaps")
            {
                sLogger.failure("Movement maps in %s not found. Disabling pathfinding.", requiredPath.u8string().c_str());
                worldConfig.terrainCollision.isPathfindingEnabled = false;
            }
            else if (dir == "vmaps")
            {
                sLogger.failure("Vertical maps in %s not found. Disabling collision.", requiredPath.u8string().c_str());
                worldConfig.terrainCollision.isCollisionEnabled = false;
            }
            else
            {
                sLogger.failure("Required dir %s not found!", requiredPath.u8string().c_str());
            }

            // Give user a chance to read the error message
            Arcemu::Sleep(2000);
        }
    }

    // Check again that collision is also enabled if pathfinding is enabled
    // could happen if mmaps directory was found but vmaps directory wasn't found
    if (worldConfig.terrainCollision.isPathfindingEnabled && !worldConfig.terrainCollision.isCollisionEnabled)
    {
        sLogger.failure("Movement maps were found but collision was disabled. Disabling pathfinding.");
        worldConfig.terrainCollision.isPathfindingEnabled = false;

        // Give user a chance to read the error message
        Arcemu::Sleep(2000);
    }
}

/////////////////////////////////////////////////////////////////////////////

bool Master::Run(int /*argc*/, char** /*argv*/)
{
    char* config_file = (char*)CONFDIR "/world.conf";

    UNIXTIME = time(NULL);
    g_localTime = *localtime(&UNIXTIME);

    sLogger.initalizeLogger("world");

    PrintBanner();

    sLogger.info("The key combination <Ctrl-C> will safely shut down the server.");

#ifndef WIN32
    if (geteuid() == 0 || getegid() == 0)
        sLogger.warning("You are running AscEmu as root. This is not needed, and may be a possible security risk. It is advised to hit CTRL+C now and start as a non-privileged user.");
#endif

    ThreadPool.Startup();
    auto startTime = Util::TimeNow();

    sWorld.initialize();

    if (!LoadWorldConfiguration(config_file))
    {
        return false;
    }

    sWorld.loadWorldConfigValues();

    sLogger.setMinimumMessageType(static_cast<AscEmu::Logging::MessageType>(worldConfig.logger.minimumMessageType));

    OpenCheatLogFiles();

    if (!_StartDB())
    {
        Database::CleanupLibs();
        sLogger.finalize();
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
        sLogger.finalize();
        return false;
    }

    sOpcodeTables.initialize();

    WorldSession::InitPacketHandlerTable();

    if (!sWorld.setInitialWorldSettings())
    {
        sLogger.failure("SetInitialWorldSettings() failed. Something went wrong? Exiting.");
        sLogger.finalize();
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

    sLogger.info("Server : Ready for connections. Startup time: %u ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    sGameEventMgrThread.initialize();

    StartRemoteConsole();

    WritePidFile();

    sChannelMgr.initialize();
    sChannelMgr.setSeperatedChannels(!worldConfig.player.isInterfactionChannelEnabled);

    sMailSystem.StartMailSystem();

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
    sLogonCommHandler.initialize();
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
    sLogger.info("Database : Clearing all pending queries...");

    // kill the database thread first so we don't lose any queries/data
    CharacterDatabase.EndThreads();
    WorldDatabase.EndThreads();

    ls->Close();

    CloseConsoleListener();
    sWorld.saveAllPlayersToDb();

    sLogger.info("Network : Shutting down network subsystem.");
#ifdef WIN32
    sSocketMgr.ShutdownThreads();
#endif
    sSocketMgr.CloseAll();

    bServerShutdown = true;
    ThreadPool.Shutdown();

    delete ls;

    sWorld.logoutAllPlayers();

    sLogonCommHandler.finalize();

#if VERSION_STRING < Cata
    sLogger.info("AddonMgr : ~AddonMgr()");
    sAddonMgr.SaveToDB();
    sAddonMgr.finalize();
#endif

    sLogger.info("AuctionMgr : ~AuctionMgr()");
    sAuctionMgr.finalize();

    sLogger.info("LootMgr : ~LootMgr()");
    sLootMgr.finalize();

    sLogger.info("World : ~World()");
    sWorld.finalize();

    sScriptMgr.UnloadScripts();

    sLogger.info("ChatHandler : ~ChatHandler()");
    sChatHandler.finalize();

    sLogger.info("Database : Closing Connections...");
    _StopDB();

    sLogger.info("Network : Deleting Network Subsystem...");
    sSocketMgr.finalize();
    sSocketGarbageCollector.finalize();

    delete GMCommand_Log;
    delete Anticheat_Log;
    delete Player_Log;

    // remove pid
    if (remove("worldserver.pid") != 0)
    {
        sLogger.failure("Error deleting file worldserver.pid");
    }
    else
    {
        sLogger.debug("File worldserver.pid successfully deleted");
    }

    sLogger.info("Shutdown : Shutdown complete.");
    sLogger.finalize();

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
        sLogger.fatal("Database : World database is missing the table `world_db_version` OR the table doesn't contain any rows. Can't validate database version. Exiting.");
        sLogger.fatal("Database : You may need to update your database");
        return false;
    }

    Field* f = wqr->Fetch();
    const char *WorldDBVersion = f->GetString();

    sLogger.info("Database : Last world database update: %s", WorldDBVersion);
    int result = strcmp(WorldDBVersion, REQUIRED_WORLD_DB_VERSION);
    if (result != 0)
    {
        sLogger.fatal("Database : Last world database update doesn't match the required one which is %s.", REQUIRED_WORLD_DB_VERSION);

        if (result < 0)
        {
            sLogger.fatal("Database : You need to apply the world update queries that are newer than %s. Exiting.", WorldDBVersion);
            sLogger.fatal("Database : You can find the world update queries in the sql/world_updates sub-directory of your AscEmu source directory.");
        }
        else
        {
            sLogger.fatal("Database : Your world database is probably too new for this AscEmu version, you need to update your server. Exiting.");
        }

        delete wqr;
        return false;
    }

    delete wqr;

    QueryResult* cqr = CharacterDatabase.QueryNA("SELECT LastUpdate FROM character_db_version;");
    if (cqr == NULL)
    {
        sLogger.fatal("Database : Character database is missing the table `character_db_version` OR the table doesn't contain any rows. Can't validate database version. Exiting.");
        sLogger.fatal("Database : You may need to update your database");
        return false;
    }

    f = cqr->Fetch();
    const char *CharDBVersion = f->GetString();

    sLogger.info("Database : Last character database update: %s", CharDBVersion);
    result = strcmp(CharDBVersion, REQUIRED_CHAR_DB_VERSION);
    if (result != 0)
    {
        sLogger.fatal("Database : Last character database update doesn't match the required one which is %s.", REQUIRED_CHAR_DB_VERSION);
        if (result < 0)
        {
            sLogger.fatal("Database : You need to apply the character update queries that are newer than %s. Exiting.", CharDBVersion);
            sLogger.fatal("Database : You can find the character update queries in the sql/character_updates sub-directory of your AscEmu source directory.");
        }
        else
            sLogger.fatal("Database : Your character database is too new for this AscEmu version, you need to update your server. Exiting.");

        delete cqr;
        return false;
    }

    delete cqr;

    sLogger.info("Database : Database successfully validated.");

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
        sLogger.fatal("Configs : One or more parameters were missing for WorldDatabase connection.");
        return false;
    }

    // Initialize it
    if (!WorldDatabase.Initialize(worldConfig.worldDb.host.c_str(), (unsigned int)worldConfig.worldDb.port, worldConfig.worldDb.user.c_str(),
                                             worldConfig.worldDb.password.c_str(), worldConfig.worldDb.dbName.c_str(), worldConfig.worldDb.connections, 16384))
    {
        sLogger.fatal("Configs : Connection to WorldDatabase failed. Check your database configurations!");
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
        sLogger.fatal("Configs : Connection to CharacterDatabase failed. Check your database configurations!");
        return false;
    }

    // Initialize it
    if (!CharacterDatabase.Initialize(worldConfig.charDb.host.c_str(), (unsigned int)worldConfig.charDb.port, worldConfig.charDb.user.c_str(),
                                                 worldConfig.charDb.password.c_str(), worldConfig.charDb.dbName.c_str(), worldConfig.charDb.connections, 16384))
    {
        sLogger.fatal("Configs : Connection to CharacterDatabase failed. Check your database configurations!");
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
    sLogger.failure("Crash Handler : Advanced crash handler initialized.");

    if (!m_crashedMutex.AttemptAcquire())
        TerminateThread(GetCurrentThread(), 0);

    try
    {
        sLogger.info("sql : Waiting for all database queries to finish...");
        WorldDatabase.EndThreads();
        CharacterDatabase.EndThreads();
        sLogger.info("sql : All pending database operations cleared.");
        sWorld.saveAllPlayersToDb();
        sLogger.info("sql : Data saved.");
    }
    catch (...)
    {
        sLogger.failure("sql : Threw an exception while attempting to save all data.");
    }

    sLogger.info("Server : Closing.");

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
    sLogger.file(AscEmu::Logging::Severity::FAILURE, AscEmu::Logging::MessageType::MINOR, "<< AscEmu %s/%s-%s (%s) :: World Server >>", BUILD_HASH_STR, CONFIG, PLATFORM_TEXT, ARCH);
    sLogger.file(AscEmu::Logging::Severity::FAILURE, AscEmu::Logging::MessageType::MINOR, "========================================================");
}

bool Master::LoadWorldConfiguration(char* config_file)
{
    sLogger.info("Config : Loading Config Files...");
    if (Config.MainConfig.openAndLoadConfigFile(config_file))
    {
        sLogger.info("Config : " CONFDIR "/world.conf loaded");
    }
    else
    {
        sLogger.failure("Config : error occurred loading " CONFDIR "/world.conf");
        sLogger.finalize();
        return false;
    }

    return true;
}

void Master::OpenCheatLogFiles()
{
    bool useTimeStamp = worldConfig.logger.enableTimeStamp;
    std::string logDir = worldConfig.logger.extendedLogsDir;

    Anticheat_Log = new SessionLog(AscEmu::Logging::getFormattedFileName(logDir, "cheaters", useTimeStamp).c_str(), false);
    GMCommand_Log = new SessionLog(AscEmu::Logging::getFormattedFileName(logDir, "gmcommands", useTimeStamp).c_str(), false);
    Player_Log = new SessionLog(AscEmu::Logging::getFormattedFileName(logDir, "players", useTimeStamp).c_str(), false);

    if (Anticheat_Log->isSessionLogOpen())
    {
        if (!worldConfig.logger.enableCheaterLog)
        {
            Anticheat_Log->closeSessionLog();
        }
    }
    else if (worldConfig.logger.enableCheaterLog)
    {
        Anticheat_Log->openSessionLog();
    }

    if (GMCommand_Log->isSessionLogOpen())
    {
        if (!worldConfig.logger.enableGmCommandLog)
        {
            GMCommand_Log->closeSessionLog();
        }
    }
    else if (worldConfig.logger.enableGmCommandLog)
    {
        GMCommand_Log->openSessionLog();
    }

    if (Player_Log->isSessionLogOpen())
    {
        if (!worldConfig.logger.enablePlayerLog)
        {
            Player_Log->closeSessionLog();
        }
    }
    else if (worldConfig.logger.enablePlayerLog)
    {
        Player_Log->openSessionLog();
    }
}

void Master::StartRemoteConsole()
{
    sLogger.info("RemoteConsole : Starting...");
    if (StartConsoleListener())
    {
#ifdef WIN32
        ThreadPool.ExecuteTask(GetConsoleListener());
#endif
        sLogger.info("RemoteConsole : Now open.");
    }
    else
    {
        sLogger.warning("RemoteConsole : Not enabled or failed listen.");
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
                    sLogger.info("Server : Shutdown in %i minutes.", static_cast<int>(m_ShutdownTimer / 60000.0f));
                }
                else
                sLogger.info("Server : Shutdown in %i seconds.", static_cast<int>(m_ShutdownTimer / 1000.0f));

                next_printout = Util::getMSTime() + 500;
            }

            if (Util::getMSTime() >= next_send)
            {
                int time = m_ShutdownTimer / 1000;
                if ((time % 30 == 0) || time < 10)
                {
                    // broadcast packet.
                    uint32_t messageType;
                    if (m_restartEvent)
                        messageType = SERVER_MSG_RESTART_TIME;
                    else
                        messageType = SERVER_MSG_SHUTDOWN_TIME;

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

                        sWorld.sendGlobalMessage(AscEmu::Packets::SmsgServerMessage(messageType, str).serialise().get());
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
    sLogger.info("Network : Starting subsystem...");
    sSocketMgr.initialize();
}

void Master::ShutdownLootSystem()
{
    sLogger.info("Shutdown : Initiated at %s", Util::GetDateTimeStringFromTimeStamp((uint32)UNIXTIME).c_str());

    if (sLootMgr.is_loading)
    {
        sLogger.info("Shutdown : Waiting for loot to finish loading...");
        while (sLootMgr.is_loading)
            Arcemu::Sleep(100);
    }
}
