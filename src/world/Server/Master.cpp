/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
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

#include "Server/Master.h"

#include "AEVersion.hpp"
#include "Common.hpp"
#include "ConfigMgr.hpp"
#include "DatabaseDefinition.hpp"
#include "git_version.hpp"
#include "OpcodeTable.hpp"
#include "World.h"
#include "WorldConf.h"
#include "WorldRunnable.h"
#include "WorldSession.h"
#include "Chat/Channel.hpp"
#include "Chat/ChannelMgr.hpp"
#include "Chat/ChatCommandHandler.hpp"
#include "Database/Database.h"
#include "Database/DatabaseUpdater.hpp"
#include "Database/Field.hpp"
#include "Logging/Log.hpp"
#include "Logging/Logger.hpp"
#include "Logging/MessageType.hpp"
#include "Logging/Severity.hpp"
#include "Management/AddonMgr.h"
#include "Management/AuctionMgr.hpp"
#include "Management/GameEventMgr.hpp"
#include "Management/MailMgr.h"
#include "Management/Loot/LootMgr.hpp"
#include "Network/Network.h"
#include "Packets/SmsgServerMessage.h"
#include "Script/ScriptMgr.hpp"
#include "Server/BroadcastMgr.h"
#include "Server/EventMgr.h"
#include "Server/WorldSocket.h"
#include "Server/Console/ConsoleAuthMgr.h"
#include "Server/Console/ConsoleListener.h"
#include "Server/Console/ConsoleThread.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Spell/SpellMgr.hpp"
#include "Storage/DayWatcherThread.h"
#include "Storage/MySQLDataStore.hpp"
#include "Threading/LegacyThreading.h"
#include "Utilities/Benchmark.hpp"
#include "Utilities/Util.hpp"

#if VERSION_STRING == Mop
#include "Data/WoWDynamicObject.hpp"
#include "Data/WoWGameObject.hpp"
#include "Data/WoWItem.hpp"
#include "Data/WoWObject.hpp"
#include "Data/WoWPlayer.hpp"
#include "Data/WoWUnit.hpp"
#endif


#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#include <WinSock2.h>
#include <Threading/LegacyThreadPool.h>
#endif

namespace
{
    // DB version
    constexpr std::string_view REQUIRED_CHAR_DB_VERSION = "20250921-00_playerpets";
    constexpr std::string_view REQUIRED_WORLD_DB_VERSION = "20260203-00_pandaren_playercreateinfo";

    void printBanner()
    {
        sLogger.info("<< AscEmu {}/{}-{} {} :: World Server >>", AE_BUILD_HASH, CONFIG, AE_PLATFORM, AE_ARCHITECTURE);
        sLogger.info("========================================================");
    }

    void startRemoteConsole()
    {
        sLogger.info("RemoteConsole : Starting...");
        if (StartConsoleListener())
        {
#ifdef _WIN32
            ThreadPool.ExecuteTask(GetConsoleListener());
#endif
            sLogger.info("RemoteConsole : Now open.");
        }
        else
        {
            sLogger.warning("RemoteConsole : Not enabled or failed listen.");
        }
    }

    bool loadWorldConfiguration(const std::string& config_file)
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

    void startNetworkSubsystem()
    {
        sLogger.info("Network : Starting subsystem...");
        sSocketMgr.initialize();
    }

    void writePidFile()
    {
        if (std::ofstream pidFile{"worldserver.pid"})
        {
            uint32_t processId;

#ifdef _WIN32
            processId = GetCurrentProcessId();
#else
            processId = getpid();
#endif

            pidFile << processId;
        } // File is automatically closed when pidFile goes out of scope
    }

    void shutdownLootSystem()
    {
        sLogger.info("Shutdown : Initiated at {}", Util::GetDateTimeStringFromTimeStamp(static_cast<uint32_t>(UNIXTIME)));

        if (sLootMgr.isLoading())
        {
            sLogger.info("Shutdown : Waiting for loot to finish loading...");
            while (sLootMgr.isLoading())
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    /////////////////////////////////////////////////////////////////////////////
    // Testscript fo experimental filesystem

    void createExtendedLogDir()
    {
        Util::BenchmarkTime benchmark;

        if (std::string logDir = worldConfig.logger.extendedLogsDir; !logDir.empty())
            fs::create_directories(logDir);
    }

    bool checkRequiredDirs()
    {
        std::vector<std::string> requiredDirs;
        requiredDirs.reserve(3);

        requiredDirs.emplace_back(CONFDIR);
        requiredDirs.emplace_back("dbc");
        requiredDirs.emplace_back("maps");

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
                sLogger.info("Required dir {} found!", requiredPath.generic_string());
            }
            else
            {
                sLogger.failure("Directory {} not found. Shutting down.", requiredPath.generic_string());
                return false;
            }
        }
        return true;
    }

    void checkAdditionalDirs()
    {
        std::vector<std::string> additionalDirs;

        if (worldConfig.terrainCollision.isCollisionEnabled)
            additionalDirs.emplace_back("vmaps");

        if (worldConfig.terrainCollision.isPathfindingEnabled)
        {
            // Check that vertical maps are also enabled
            if (!worldConfig.terrainCollision.isCollisionEnabled)
            {
                sLogger.failure("Pathfinding is enabled but collision is disabled. Disabling pathfinding.");
                worldConfig.terrainCollision.isPathfindingEnabled = false;

                // Give user a chance to read the error message
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            }
            else
            {
                additionalDirs.emplace_back("mmaps");
            }
        }

        std::string dataDir = worldConfig.server.dataDir;
        dataDir.erase(0, 2); //remove ./ from string

        for (const auto& dir : additionalDirs)
        {
            fs::path additionalPath = fs::current_path();

            if (dataDir.empty() || dir == CONFDIR)
            {
                additionalPath /= dir;
            }
            else
            {
                additionalPath /= dataDir;
                additionalPath /= dir;
            }

            if (fs::exists(additionalPath))
            {
                sLogger.info("Required dir {} found!", additionalPath.generic_string());
            }
            else
            {
                if (dir == "mmaps")
                {
                    sLogger.failure("Movement maps in {} not found. Disabling pathfinding.", additionalPath.generic_string());
                    worldConfig.terrainCollision.isPathfindingEnabled = false;
                }
                else if (dir == "vmaps")
                {
                    sLogger.failure("Vertical maps in {} not found. Disabling collision.", additionalPath.generic_string());
                    worldConfig.terrainCollision.isCollisionEnabled = false;
                }
                else
                {
                    sLogger.failure("Required dir {} not found!", additionalPath.generic_string());
                }

                // Give user a chance to read the error message
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            }
        }

        // Check again that collision is also enabled if pathfinding is enabled
        // could happen if mmaps directory was found but vmaps directory wasn't found
        if (worldConfig.terrainCollision.isPathfindingEnabled && !worldConfig.terrainCollision.isCollisionEnabled)
        {
            sLogger.failure("Movement maps were found but collision was disabled. Disabling pathfinding.");
            worldConfig.terrainCollision.isPathfindingEnabled = false;

            // Give user a chance to read the error message
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
    }

    /////////////////////////////////////////////////////////////////////////////
}

void Master::onSignal(int s)
{
    switch (s)
    {
#ifndef _WIN32
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
            sMaster().stopEvent.store(true);
            break;
        default:
            // Explicitly ignore any other signals
            break;
    }

    (void)std::signal(s, onSignal);
}

Master& Master::getInstance()
{
    // Guaranteed to be destroyed and instantiated on first use. Thread-safe in C++11 and later.
    static Master mInstance; // NOLINT
    return mInstance;
}

Master::~Master() = default;

struct Addr
{
    unsigned short sa_family;
    /* sa_data */
    unsigned short Port;
    unsigned long IP; // inet_addr
    unsigned long unusedA;
    unsigned long unusedB;
};

bool Master::run(int /*argc*/, char** /*argv*/)
{
    std::string configFile = CONFDIR "/world.conf";

    // Thread-safe time initialization
    const auto now = std::chrono::system_clock::now();
    UNIXTIME = std::chrono::system_clock::to_time_t(now);
#ifdef _WIN32
    (void)localtime_s(&g_localTime, &UNIXTIME);
#else
    (void)localtime_r(&UNIXTIME, &g_localTime);
#endif

    sLogger.initalizeLogger("world");
    printBanner();

#if VERSION_STRING == Mop
    sLogger.info("Size of WoWObject {} / 8", static_cast<uint32_t>(sizeof(WoWObject) / sizeof(uint32_t)));
    sLogger.info("Size of WoWUnit {} / 160", static_cast<uint32_t>(sizeof(WoWUnit) / sizeof(uint32_t)));
    sLogger.info("Size of WoWPlayer {} / 1987", static_cast<uint32_t>(sizeof(WoWPlayer) / sizeof(uint32_t)));
    sLogger.info("Size of WoWGameObject {} / 20", static_cast<uint32_t>(sizeof(WoWGameObject) / sizeof(uint32_t)));
    sLogger.info("Size of WoWDynamicObject {} / 14", static_cast<uint32_t>(sizeof(WoWDynamicObject) / sizeof(uint32_t)));
    sLogger.info("Size of WoWItem {} / 69", static_cast<uint32_t>(sizeof(WoWItem) / sizeof(uint32_t)));
#endif

    sLogger.info("The key combination <Ctrl-C> will safely shut down the server.");

#ifndef _WIN32
    if (geteuid() == 0 || getegid() == 0)
        sLogger.warning("You are running AscEmu as root. This is not needed, and may be a possible security risk. It is advised to hit CTRL+C now and start as a non-privileged user.");
#endif

    if (!loadWorldConfiguration(configFile))
        return false;

    // Force early instantiation to ensure correct destruction order
    (void)EventMgr::getInstance();

    sWorld.initialize();
    sWorld.loadWorldConfigValues();

    sLogger.setMinimumMessageType(static_cast<AscEmu::Logging::MessageType>(worldConfig.logger.minimumMessageType));
    sLogger.setDebugFlags(static_cast<AscEmu::Logging::DebugFlags>(worldConfig.logger.debugFlags));

    createExtendedLogDir();
    if (!checkRequiredDirs())
    {
        sLogger.finalize();
        return false;
    }

    checkAdditionalDirs();
    openCheatLogFiles();

    if (!startDB())
    {
        Database::CleanupLibs();
        sLogger.finalize();
        return false;
    }

    // From here on, if we return false, we MUST clean up DB and ThreadPool!
    ThreadPool.Startup();
    auto startTime = Util::TimeNow();

    const std::string charDbName = worldConfig.charDb.dbName;
    DatabaseUpdater::initBaseIfNeeded(charDbName, "character", *databaseCharacter);
    DatabaseUpdater::checkAndApplyDBUpdatesIfNeeded("character", *databaseCharacter);

    const std::string worldDbName = worldConfig.worldDb.dbName;
    DatabaseUpdater::initBaseIfNeeded(worldDbName, "world", *databaseWorld);
    DatabaseUpdater::checkAndApplyDBUpdatesIfNeeded("world", *databaseWorld);

    if (!checkDBVersion())
    {
        ThreadPool.Shutdown(); // Prevent thread leak
        stopDB();
        sLogger.finalize();
        return false;
    }

    sOpcodeTables.initialize();
    WorldSession::registerOpcodeHandler();

    if (!sWorld.setInitialWorldSettings())
    {
        sLogger.failure("SetInitialWorldSettings() failed. Something went wrong? Exiting.");
        ThreadPool.Shutdown(); // Prevent thread leak
        stopDB();
        sLogger.finalize();
        return false;
    }

    sWorld.setWorldStartTime(static_cast<uint32_t>(UNIXTIME));

    worldRunnable = std::make_unique<WorldRunnable>();
    hookSignals();

    auto console = std::make_unique<ConsoleThread>();
    ThreadPool.ExecuteTask(console.get());

    startNetworkSubsystem();
    sSocketMgr.SpawnWorkerThreads();
    sScriptMgr.LoadScripts();
    sSpellMgr.loadSpellScripts();

    if (worldConfig.startup.enableSpellIdDump)
        sScriptMgr.DumpUnimplementedSpells();

    sLogger.info("Server : Ready for connections. Startup time: {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    sGameEventMgrThread.initialize();
    startRemoteConsole();
    writePidFile();

    sChannelMgr.initialize();
    sChannelMgr.setSeperatedChannels(!worldConfig.player.isInterfactionChannelEnabled);

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
    sMailSystem.StartMailSystem();

    //ThreadPool.Gobble();

    /* Connect to realmlist servers / logon servers */
    sLogonCommHandler.initialize();
    sLogonCommHandler.startLogonCommHandler();

    // Create listener
    auto listenSocket = std::make_unique<ListenSocket<WorldSocket>>(worldConfig.listen.listenHost.c_str(), worldConfig.listen.listenPort);
    bool isListenerOpen = listenSocket->IsOpen();
#ifdef _WIN32
    if (isListenerOpen)
        ThreadPool.ExecuteTask(listenSocket.get());
#endif

    shutdownThreadPools(isListenerOpen);

    // Begin server shutdown
    unhookSignals();

    worldRunnable->threadShutdown();
    worldRunnable.reset();

    ThreadPool.ShowStats();
    console->stopThread();

    shutdownLootSystem();

    sLogger.info("Database : Clearing all pending queries...");
    databaseCharacter->EndThreads();
    databaseWorld->EndThreads();

    listenSocket->Close();
    CloseConsoleListener();
    sWorld.saveAllPlayersToDb();

    sLogger.info("Network : Shutting down network subsystem.");
#ifdef _WIN32
    sSocketMgr.ShutdownThreads();
#endif
    sSocketMgr.CloseAll();

    serverShutdown.store(true);
    ThreadPool.Shutdown();

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

    sLogger.info("ChatHandler : ~ChatHandler()");
    sChatHandler.finalize();

    sLogger.info("World : ~World()");
    sWorld.finalize();

    sScriptMgr.UnloadScripts();

    sLogger.info("Database : Closing Connections...");
    stopDB();

    sLogger.info("Network : Deleting Network Subsystem...");
    sSocketMgr.finalize();
    sSocketGarbageCollector.finalize();

    gmCommandLog = nullptr;
    anticheatLog = nullptr;
    playerLog = nullptr;

    if (std::remove("worldserver.pid") != 0)
    {
        sLogger.failure("Error deleting file worldserver.pid");
    }
    else
    {
        sLogger.debug("File worldserver.pid successfully deleted");
    }

    sLogger.info("Shutdown : Shutdown complete.");
    sLogger.finalize();

#ifdef _WIN32
    WSACleanup();
#endif

    return true;
}

bool Master::checkDBVersion()
{
    auto worldQueryResult = databaseWorld->QueryNA("SELECT LastUpdate FROM world_db_version ORDER BY id DESC LIMIT 1;");
    if (!worldQueryResult)
    {
        sLogger.fatal("Database : World database is missing the table `world_db_version` OR the table doesn't contain any rows. Can't validate database version. Exiting.");
        sLogger.fatal("Database : You may need to update your database");
        return false;
    }

    Field* worldField = worldQueryResult->Fetch();
    std::string_view worldDbVersion = worldField->asCString();

    sLogger.info("Database : Last world database update: {}", worldDbVersion);

    if (worldDbVersion != REQUIRED_WORLD_DB_VERSION)
    {
        sLogger.fatal("Database : Last world database update doesn't match the required one which is {}.", REQUIRED_WORLD_DB_VERSION);

        if (worldDbVersion < REQUIRED_WORLD_DB_VERSION)
        {
            sLogger.fatal("Database : You need to apply the world update queries that are newer than {}. Exiting.", worldDbVersion);
            sLogger.fatal("Database : You can find the world update queries in the sql/world_updates sub-directory of your AscEmu source directory.");
        }
        else
        {
            sLogger.fatal("Database : Your world database is probably too new for this AscEmu version, you need to update your server. Exiting.");
        }

        return false;
    }

    auto charQueryResult = databaseCharacter->QueryNA("SELECT LastUpdate FROM character_db_version ORDER BY id DESC LIMIT 1;");
    if (!charQueryResult)
    {
        sLogger.fatal("Database : Character database is missing the table `character_db_version` OR the table doesn't contain any rows. Can't validate database version. Exiting.");
        sLogger.fatal("Database : You may need to update your database");
        return false;
    }

    Field* charField = charQueryResult->Fetch();
    std::string_view charDbVersion = charField->asCString();

    sLogger.info("Database : Last character database update: {}", charDbVersion);

    if (charDbVersion != REQUIRED_CHAR_DB_VERSION)
    {
        sLogger.fatal("Database : Last character database update doesn't match the required one which is {}.", REQUIRED_CHAR_DB_VERSION);
        if (charDbVersion < REQUIRED_CHAR_DB_VERSION)
        {
            sLogger.fatal("Database : You need to apply the character update queries that are newer than {}. Exiting.", charDbVersion);
            sLogger.fatal("Database : You can find the character update queries in the sql/character_updates sub-directory of your AscEmu source directory.");
        }
        else
            sLogger.fatal("Database : Your character database is too new for this AscEmu version, you need to update your server. Exiting.");

        return false;
    }

    sLogger.info("Database : Database successfully validated.");

    return true;
}

bool Master::startDB()
{
    databaseWorld.reset();
    databaseCharacter.reset();

    auto isValidConfig = [](const auto& dbConfig) -> bool
    {
        return !dbConfig.user.empty() &&
            !dbConfig.password.empty() &&
            !dbConfig.host.empty() &&
            !dbConfig.dbName.empty() &&
            dbConfig.port != 0;
    };

    if (!isValidConfig(worldConfig.worldDb))
    {
        sLogger.fatal("Configs : One or more parameters were missing for WorldDatabase connection.");
        return false;
    }

    databaseWorld = Database::CreateDatabaseInterface();

    // Initialize it
    if (!databaseWorld->Initialize(worldConfig.worldDb.host.c_str(),
                                   static_cast<uint32_t>(worldConfig.worldDb.port),
                                   worldConfig.worldDb.user.c_str(),
                                   worldConfig.worldDb.password.c_str(),
                                   worldConfig.worldDb.dbName.c_str(),
                                   worldConfig.worldDb.connections,
                                   16384,
                                   worldConfig.worldDb.isLegacyAuth))
    {
        sLogger.fatal("Configs : Connection to WorldDatabase failed. Check your database configurations!");
        return false;
    }

    if (!isValidConfig(worldConfig.charDb))
    {
        sLogger.fatal("Configs : One or more parameters were missing for CharacterDatabase connection.");
        return false;
    }

    databaseCharacter = Database::CreateDatabaseInterface();

    if (!databaseCharacter->Initialize(worldConfig.charDb.host.c_str(),
                                       static_cast<uint32_t>(worldConfig.charDb.port),
                                       worldConfig.charDb.user.c_str(),
                                       worldConfig.charDb.password.c_str(),
                                       worldConfig.charDb.dbName.c_str(),
                                       worldConfig.charDb.connections,
                                       16384,
                                       worldConfig.charDb.isLegacyAuth))
    {
        sLogger.fatal("Configs : Connection to CharacterDatabase failed. Check your database configurations!");
        return false;
    }

    return true;
}

void Master::stopDB()
{
    databaseWorld.reset();
    databaseCharacter.reset();
    Database::CleanupLibs();
}

void Master::hookSignals()
{
    (void)std::signal(SIGINT, onSignal);
    (void)std::signal(SIGTERM, onSignal);
    (void)std::signal(SIGABRT, onSignal);
#ifdef _WIN32
    (void)std::signal(SIGBREAK, onSignal);
#else
    (void)std::signal(SIGHUP, onSignal);
    (void)std::signal(SIGUSR1, onSignal);
#endif
}

void Master::unhookSignals()
{
    (void)std::signal(SIGINT, SIG_DFL);
    (void)std::signal(SIGTERM, SIG_DFL);
    (void)std::signal(SIGABRT, SIG_DFL);
#ifdef _WIN32
    (void)std::signal(SIGBREAK, SIG_DFL);
#else
    (void)std::signal(SIGHUP, SIG_DFL);
#endif
}

void Master::openCheatLogFiles()
{
    bool useTimeStamp = worldConfig.logger.enableTimeStamp;
    std::string logDir = worldConfig.logger.extendedLogsDir;

    anticheatLog = std::make_unique<SessionLog>(AscEmu::Logging::getFormattedFileName(logDir, "cheaters", useTimeStamp).c_str(), false);
    gmCommandLog = std::make_unique<SessionLog>(AscEmu::Logging::getFormattedFileName(logDir, "gmcommands", useTimeStamp).c_str(), false);
    playerLog = std::make_unique<SessionLog>(AscEmu::Logging::getFormattedFileName(logDir, "players", useTimeStamp).c_str(), false);

    // Helper lambda to deduplicate the open/close logic
    auto toggleLog = [](const std::unique_ptr<SessionLog>& log, bool isEnabled)
    {
        if (log->isSessionLogOpen())
        {
            if (!isEnabled) log->closeSessionLog();
        }
        else if (isEnabled)
        {
            log->openSessionLog();
        }
    };

    toggleLog(anticheatLog, worldConfig.logger.enableCheaterLog);
    toggleLog(gmCommandLog, worldConfig.logger.enableGmCommandLog);
    toggleLog(playerLog, worldConfig.logger.enablePlayerLog);
}

void Master::shutdownThreadPools(bool listenerSockCreate)
{
    uint32_t loopCounter = 0;
    auto lastTime = Util::TimeNow();
    uint32_t next_printout = Util::getMSTime();
    uint32_t next_send = Util::getMSTime();

    while (!stopEvent && listenerSockCreate)
    {
        auto begin = Util::TimeNow();
        auto diff = Util::GetTimeDifference(lastTime, begin);

        ++loopCounter;
        updatePeriodicStats(loopCounter);
        updateServerTime();

        sSocketGarbageCollector.Update();

        /* UPDATE */
        lastTime = Util::TimeNow();
        auto elapsedTime = Util::GetTimeDifference(begin, lastTime);
        if (processShutdownSequence(diff, next_printout, next_send))
        {
            break;
        }

        if (50 > elapsedTime)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50 - elapsedTime));
        }
    }
}

void Master::updatePeriodicStats(uint32_t currentLoop) const
{
    if (currentLoop % 10000 != 0) return; // Only run every ~5 mins

    ThreadPool.ShowStats();
    ThreadPool.IntegrityCheck();

#if !defined(_WIN32) && defined(__DEBUG__)
    if (std::ofstream uptimeFile{"worldserver.uptime"})
    {
        uptimeFile << sWorld.GetUptime() << " "
            << sWorld.GetSessionCount() << " "
            << sWorld.getPeakSessionCount() << " "
            << sWorld.getAcceptedConnections();
    }
#endif
}

void Master::updateServerTime() const
{
    const auto now = std::chrono::system_clock::now();
    std::time_t curTime = std::chrono::system_clock::to_time_t(now);

    if (UNIXTIME != curTime)
    {
        UNIXTIME = curTime;
#ifdef _WIN32
        (void)localtime_s(&g_localTime, &UNIXTIME);
#else
        (void)localtime_r(&UNIXTIME, &g_localTime);
#endif
    }
}

bool Master::processShutdownSequence(long long diff, uint32_t& next_printout, uint32_t& nextSend)
{
    if (!m_ShutdownEvent) return false;

    uint32_t currentMsTime = Util::getMSTime();

    // Logging
    if (currentMsTime >= next_printout)
    {
        if (m_ShutdownTimer >= 60000U && m_ShutdownTimer % 60000U == 0)
            sLogger.info("Server : Shutdown in {} minutes.", m_ShutdownTimer / 60000U);
        else if (m_ShutdownTimer < 60000U)
            sLogger.info("Server : Shutdown in {} seconds.", m_ShutdownTimer / 1000U);

        next_printout = currentMsTime + 500U;
    }

    // Broadcasting
    if (currentMsTime >= nextSend)
    {
        uint32_t timeLeft = m_ShutdownTimer / 1000U;
        if (timeLeft % 30U == 0 || timeLeft < 10U)
        {
            uint32_t messageType = m_restartEvent ? SERVER_MSG_RESTART_TIME : SERVER_MSG_SHUTDOWN_TIME;

            if (timeLeft > 0U)
            {
                uint32_t mins = timeLeft / 60U;
                uint32_t secs = timeLeft % 60U;
                std::string timeStr = std::format("{:02}:{:02}", mins, secs);

                sWorld.sendGlobalMessage(AscEmu::Packets::SmsgServerMessage(messageType, timeStr).serialise().get());
            }
        }
        nextSend = currentMsTime + 1000U;
    }

    // Timer Logic
    if (diff > 0)
    {
        if (std::cmp_greater_equal(diff, m_ShutdownTimer))
            return true; // Timer is done

        m_ShutdownTimer -= static_cast<uint32_t>(diff);
    }

    return false; // Continue looping
}

void Master::triggerShutdown(uint32_t timerMs, bool restart)
{
    m_ShutdownTimer = timerMs;
    m_ShutdownEvent = true;
    m_restartEvent = restart;
}

void Master::cancelShutdown()
{
    m_ShutdownEvent = false;
    m_ShutdownTimer = 5000;
    m_restartEvent = false;
}
