/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Master.hpp"

#include "BNetConf.hpp"
#include "BNetConfig.hpp"
#include "BNetServerDefines.hpp"
#include "BNetTlsContext.hpp"
#include "BNetSocket.hpp"
#include "BattleNetComm.hpp"
#include "Database/Database.hpp"
#include "Database/DatabaseUpdater.hpp"
#include "Database/Field.hpp"
#include "WebAuthSocket.hpp"
#include "Logging/Log.hpp"
#include "Logging/Logger.hpp"
#include "Network/Network.hpp"
#include "Threading/Thread.hpp"
#include "Threading/ThreadPool.hpp"

#include <csignal>
#include <cstring>
#include <ctime>

namespace AscEmu::Battlenet
{
    static constexpr char REQUIRED_LOGON_DB_VERSION[] = "20260924-00_battlenet_accounts";

    std::unique_ptr<Database> sBNetLogonSQL;
    std::unique_ptr<Database> sBNetCharacterSQL;

    namespace
    {
        Master* activeMaster = nullptr;
    }

    Master& Master::getInstance()
    {
        static Master instance;
        return instance;
    }

    bool Master::loadConfiguration()
    {
        if (!configManager.mainConfig.openAndLoadConfigFile(CONFDIR "/bnetserver.conf"))
        {
            sLogger.failure("Config : error occurred loading " CONFDIR "/bnetserver.conf");
            return false;
        }

        bnetConfig.load();
        sLogger.info("Config : " CONFDIR "/bnetserver.conf loaded");
        return true;
    }

    void Master::run()
    {
        UNIXTIME = std::time(nullptr);
        sLogger.initializeLogger("bnetserver");
        sLogger.info("AscEmu Battle.net server starting");

        if (!loadConfiguration())
        {
            sLogger.finalize();
            return;
        }

        sLogger.setMinimumMessageType(static_cast<AscEmu::Logging::MessageType>(bnetConfig.logger.minimumMessageType));

        sBNetLogonSQL = Database::createDatabaseInterface();
        if (!sBNetLogonSQL || !sBNetLogonSQL->initialize(bnetConfig.logonDatabase.host.c_str(), bnetConfig.logonDatabase.port, bnetConfig.logonDatabase.user.c_str(), bnetConfig.logonDatabase.password.c_str(), bnetConfig.logonDatabase.name.c_str(), bnetConfig.logonDatabase.connections, 16384, bnetConfig.logonDatabase.legacyAuth))
        {
            sLogger.failure("BNet: logon database initialization failed");
            sBNetLogonSQL.reset();
            sLogger.finalize();
            return;
        }

        sLogger.info("BNet: connected to logon database '{}@{}:{}/{}'", bnetConfig.logonDatabase.user, bnetConfig.logonDatabase.host, bnetConfig.logonDatabase.port, bnetConfig.logonDatabase.name);

        DatabaseUpdater::initBaseIfNeeded(bnetConfig.logonDatabase.name, "logon", *sBNetLogonSQL);
        DatabaseUpdater::checkAndApplyDBUpdatesIfNeeded("logon", *sBNetLogonSQL);

        auto logonVersionResult = sBNetLogonSQL->queryNA("SELECT LastUpdate FROM logon_db_version ORDER BY id DESC LIMIT 1;");
        if (!logonVersionResult)
        {
            sLogger.failure("Database : logon database is missing the table `logon_db_version` OR the table doesn't contain any rows. " "Can't validate database version. Exiting.");
            sBNetLogonSQL->shutdown();
            sBNetLogonSQL.reset();
            sLogger.finalize();
            return;
        }

        Field* logonVersionFields = logonVersionResult->fetch();
        const char* logonDbVersion = logonVersionFields[0].asCString();
        sLogger.info("Database : Last logon database update: {}", logonDbVersion);

        const int logonVersionCompare = std::strcmp(logonDbVersion, REQUIRED_LOGON_DB_VERSION);
        if (logonVersionCompare != 0)
        {
            sLogger.failure("Database : Last logon database update doesn't match the required one which is {}.", REQUIRED_LOGON_DB_VERSION);

            if (logonVersionCompare < 0)
            {
                sLogger.failure("Database : You need to apply the logon update queries that are newer than {}. Exiting.", logonDbVersion);
                sLogger.failure("Database : You can find the logon update queries in the sql/logon/updates sub-directory of your AscEmu source directory.");
            }
            else
            {
                sLogger.failure("Database : Your logon database is too new for this AscEmu version, you need to update your server. Exiting.");
            }

            sBNetLogonSQL->shutdown();
            sBNetLogonSQL.reset();
            sLogger.finalize();
            return;
        }

        sLogger.info("Database : Logon database successfully validated.");

        sBNetCharacterSQL = Database::createDatabaseInterface();
        if (!sBNetCharacterSQL || !sBNetCharacterSQL->initialize(bnetConfig.characterDatabase.host.c_str(), bnetConfig.characterDatabase.port, bnetConfig.characterDatabase.user.c_str(), bnetConfig.characterDatabase.password.c_str(), bnetConfig.characterDatabase.name.c_str(), bnetConfig.characterDatabase.connections, 16384, bnetConfig.characterDatabase.legacyAuth))
        {
            sLogger.failure("BNet: character database initialization failed");
            sBNetCharacterSQL.reset();
            sBNetLogonSQL->shutdown();
            sBNetLogonSQL.reset();
            sLogger.finalize();
            return;
        }

        sLogger.info("BNet: connected to character database '{}@{}:{}/{}'", bnetConfig.characterDatabase.user, bnetConfig.characterDatabase.host, bnetConfig.characterDatabase.port, bnetConfig.characterDatabase.name);

        if (!BNetTlsContext::getInstance().initialize())
        {
            sLogger.failure("BNet: TLS initialization failed");
            if (sBNetCharacterSQL)
            {
                sBNetCharacterSQL->shutdown();
                sBNetCharacterSQL.reset();
            }
            sBNetLogonSQL->shutdown();
            sBNetLogonSQL.reset();
            sLogger.finalize();
            return;
        }

        m_threadPool = std::make_unique<AscEmu::Threading::AEThreadPool>("BNetServer", 1, 4, 8);
        m_threadPool->start();

        sSocketMgr.initialize();
        sSocketMgr.SetThreadPool(*m_threadPool);

        auto listener = std::make_unique<ListenSocket<BNetSocket>>(bnetConfig.listen.host.c_str(), bnetConfig.listen.port);

        if (!listener->IsOpen())
        {
            sLogger.failure("BNet: failed to listen on {}:{}", bnetConfig.listen.host, bnetConfig.listen.port);
            sSocketMgr.finalize();
            BNetTlsContext::getInstance().finalize();
            m_threadPool->shutdown();
            m_threadPool->join();
            m_threadPool.reset();
            if (sBNetCharacterSQL)
            {
                sBNetCharacterSQL->shutdown();
                sBNetCharacterSQL.reset();
            }
            sBNetLogonSQL->shutdown();
            sBNetLogonSQL.reset();
            sLogger.finalize();
            return;
        }

        auto webAuthListener = std::make_unique<ListenSocket<WebAuthSocket>>(bnetConfig.webAuth.host.c_str(), bnetConfig.webAuth.port);

        if (!webAuthListener->IsOpen())
        {
            sLogger.failure("BNet WebAuth: failed to listen on {}:{}", bnetConfig.webAuth.host, bnetConfig.webAuth.port);
            listener->Close();
            sSocketMgr.finalize();
            BNetTlsContext::getInstance().finalize();
            m_threadPool->shutdown();
            m_threadPool->join();
            m_threadPool.reset();
            if (sBNetCharacterSQL)
            {
                sBNetCharacterSQL->shutdown();
                sBNetCharacterSQL.reset();
            }
            sBNetLogonSQL->shutdown();
            sBNetLogonSQL.reset();
            sLogger.finalize();
            return;
        }

        auto battleNetCommListener = std::make_unique<ListenSocket<BattleNetCommServerSocket>>(bnetConfig.battleNetComm.host.c_str(), bnetConfig.battleNetComm.port);

        if (!battleNetCommListener->IsOpen())
        {
            sLogger.failure("BattleNetComm: failed to listen on {}:{}", bnetConfig.battleNetComm.host, bnetConfig.battleNetComm.port);
            webAuthListener->Close();
            listener->Close();
            sSocketMgr.finalize();
            BNetTlsContext::getInstance().finalize();
            m_threadPool->shutdown();
            m_threadPool->join();
            m_threadPool.reset();
            if (sBNetCharacterSQL)
            {
                sBNetCharacterSQL->shutdown();
                sBNetCharacterSQL.reset();
            }
            sBNetLogonSQL->shutdown();
            sBNetLogonSQL.reset();
            sLogger.finalize();
            return;
        }

        sSocketMgr.SpawnWorkerThreads();

#ifdef WIN32
        m_threadPool->addDedicatedThread("BNetListenSocket", [socket = listener.get()](AscEmu::Threading::AEThread&) { static_cast<void>(socket->runThread()); });

        m_threadPool->addDedicatedThread("BNetWebAuthListenSocket", [socket = webAuthListener.get()](AscEmu::Threading::AEThread&) { static_cast<void>(socket->runThread()); });

        m_threadPool->addDedicatedThread("BattleNetCommListenSocket", [socket = battleNetCommListener.get()](AscEmu::Threading::AEThread&) { static_cast<void>(socket->runThread()); });
#endif

        hookSignals();
        sLogger.info("BNet: listening on {}:{}", bnetConfig.listen.host, bnetConfig.listen.port);
        sLogger.info("BNet WebAuth: listening on https://{}:{}", bnetConfig.webAuth.host, bnetConfig.webAuth.port);
        sLogger.info("BattleNetComm: listening on {}:{}", bnetConfig.battleNetComm.host, bnetConfig.battleNetComm.port);

        while (m_running.load())
        {
            UNIXTIME = std::time(nullptr);
            sSocketGarbageCollector.Update();
            AscEmu::Threading::sleep(1000);
        }

        unhookSignals();

        battleNetCommListener->Close();
        webAuthListener->Close();
        listener->Close();
        sSocketMgr.CloseAll();
        sSocketMgr.ShutdownThreads();
        sSocketMgr.finalize();
        sSocketGarbageCollector.finalize();

        m_threadPool->shutdown();
        m_threadPool->join();
        m_threadPool.reset();

        if (sBNetCharacterSQL)
        {
            sBNetCharacterSQL->shutdown();
            sBNetCharacterSQL.reset();
        }

        if (sBNetLogonSQL)
        {
            sBNetLogonSQL->shutdown();
            sBNetLogonSQL.reset();
        }

        BNetTlsContext::getInstance().finalize();

        sLogger.info("BNet: shutdown complete");
        sLogger.finalize();
    }

    void Master::stop()
    {
        m_running.store(false);
    }

    void Master::hookSignals()
    {
        activeMaster = this;
        std::signal(SIGINT, &Master::onSignal);
        std::signal(SIGTERM, &Master::onSignal);
    }

    void Master::unhookSignals()
    {
        std::signal(SIGINT, SIG_DFL);
        std::signal(SIGTERM, SIG_DFL);
        activeMaster = nullptr;
    }

    void Master::onSignal(int /*signal*/)
    {
        if (activeMaster != nullptr)
            activeMaster->stop();
    }
}
