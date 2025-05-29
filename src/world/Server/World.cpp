/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "World.h"

#include "AEVersion.hpp"
#include "Management/AddonMgr.h"
#include "Management/AuctionMgr.hpp"
#include "Management/CalendarMgr.hpp"
#include "Management/LFG/LFGMgr.hpp"
#include "Management/Loot/LootMgr.hpp"
#include "Management/WordFilter.hpp"
#include "Management/WeatherMgr.hpp"
#include "Management/TaxiMgr.hpp"
#include "Management/ItemInterface.h"
#include "Chat/ChannelMgr.hpp"
#include "WorldSocket.h"
#include "Storage/MySQLDataStore.hpp"
#include "Debugging/CrashHandler.h"
#include "Storage/DayWatcherThread.h"
#include "BroadcastMgr.h"
#include "DatabaseDefinition.hpp"
#include "Spell/SpellMgr.hpp"
#include "Management/Guild/GuildMgr.hpp"
#include "Packets/SmsgPlaySound.h"
#include "Packets/SmsgAreaTriggerMessage.h"
#include "Packets/SmsgZoneUnderAttack.h"
#include "OpcodeTable.hpp"
#include "Chat/ChatHandler.hpp"
#include "Management/GameEventMgr.hpp"
#include "Objects/Units/Creatures/CreatureGroups.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Movement/WaypointManager.h"
#include "Packets/SmsgMessageChat.h"
#include "Map/Management/MapMgr.hpp"
#include "Chat/CommandTableStorage.hpp"
#include "VMapFactory.h"
#include "VMapManager2.h"
#include "WorldSession.h"
#include "Management/MailMgr.h"
#include "Management/ObjectMgr.hpp"
#include "Management/QuestMgr.h"
#include "Management/TransporterHandler.hpp"
#include "Management/Battleground/BattlegroundMgr.hpp"
#include "Management/Tickets/TicketMgr.hpp"
#include "Movement/MovementGenerator.h"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Spell/SpellInfo.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"

#if VERSION_STRING >= Cata
#include "Management/Guild/GuildFinderMgr.hpp"
#endif

std::unique_ptr<DayWatcherThread> dw = nullptr;

std::unique_ptr<BroadcastMgr> broadcastMgr = nullptr;

extern void LoadGameObjectModelList(std::string const& dataPath);

World::World() = default;
World::~World() = default;

World& World::getInstance()
{
    static World mInstance;
    return mInstance;
}

void World::initialize()
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Uptime
    mStartTime = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Player statistic
    mHordePlayersCount = 0;
    mAlliancePlayersCount = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // InfoCore
    mTotalTrafficInKB = 0.0;
    mTotalTrafficOutKB = 0.0;
    mLastTotalTrafficInKB = 0.0;
    mLastTotalTrafficOutKB = 0.0;
    mLastTrafficQuery = 0;
    mAcceptedConnections = 0;
    mPeakSessionCount = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Session queue
    mQueueUpdateTimer = 5000;

    //////////////////////////////////////////////////////////////////////////////////////////
    // General Functions
    mEventableObjectHolder = std::make_unique<EventableObjectHolder>(WORLD_INSTANCE);
    m_holder = mEventableObjectHolder.get();
    m_event_Instanceid = mEventableObjectHolder->GetInstanceID();

    //////////////////////////////////////////////////////////////////////////////////////////
    // GM Ticket System
    mGmTicketSystemEnabled = true;
}

void World::finalize()
{
    sLogger.info("WorldLog : ~WorldLog()");
    sWorldPacketLog.finalize();
#if VERSION_STRING <= Cata  // support MOP
    sLogger.info("TransportHandler : unload()");
    sTransportHandler.unload();
#endif

    sLogger.info("TicketMgr : ~TicketMgr()");
    sTicketMgr.finalize();

    sLogger.info("LfgMgr : ~LfgMgr()");
    sLfgMgr.finalize();

    sLogger.info("ChannelMgr : ~ChannelMgr()");
    sChannelMgr.finalize();

    sLogger.info("QuestMgr : ~QuestMgr()");
    sQuestMgr.finalize();

    sLogger.info("WeatherMgr : ~WeatherMgr()");
    sWeatherMgr.finalize();

    sLogger.info("GuildMgr", "~GuildMgr()");
    sGuildMgr.finalize();

    sLogger.info("MapMgr : ~MapMgr()");
    sMapMgr.shutdown();

    sLogger.info("WordFilter : ~WordFilter()");
    g_chatFilter = nullptr;

    sLogger.info("SpellMgr : ~SpellMgr()");
    sSpellMgr.finalize();

    sLogger.info("MySQLDataStore : ~MySQLDataStore()");
    sMySQLStore.finalize();

    sLogger.info("OpcodeTables : finalize()");
    sOpcodeTables.finalize();

    broadcastMgr.reset();

    mEventableObjectHolder = nullptr;

    dummySpellList.clear();

    // Finalizing ObjectMgr must be last
    sLogger.info("ObjectMgr : ~ObjectMgr()");
    sObjectMgr.finalize();
}

//////////////////////////////////////////////////////////////////////////////////////////
// WorldConfig
void World::loadWorldConfigValues(bool reload /*false*/)
{
    settings.loadWorldConfigValues(reload);

    if (reload)
        sChannelMgr.loadConfigSettings();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Player statistic
uint32_t World::getPlayerCount()
{
    return (mHordePlayersCount + mAlliancePlayersCount);
}

void World::resetPlayerCount()
{
    mHordePlayersCount = 0;
    mAlliancePlayersCount = 0;
}

void World::incrementPlayerCount(uint32_t team)
{
    if (team == 1)
        mHordePlayersCount++;
    else
        mAlliancePlayersCount++;
}
void World::decrementPlayerCount(uint32_t team)
{
    if (team == 1)
        mHordePlayersCount--;
    else
        mAlliancePlayersCount--;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Uptime
void World::setWorldStartTime(uint32_t start_time)
{
    mStartTime = start_time;
}

uint32_t World::getWorldStartTime()
{
    return mStartTime;
}

uint32_t World::getWorldUptime()
{
    return static_cast<uint32_t>(UNIXTIME) - mStartTime;
}

std::string World::getWorldUptimeString()
{
    time_t pTime = UNIXTIME - mStartTime;
    tm* tmv = gmtime(&pTime);

    std::stringstream uptimeStream;
    uptimeStream << tmv->tm_yday << " days, " << tmv->tm_hour << " hours, " << tmv->tm_min << " minutes, " << tmv->tm_sec << " seconds.";

    return uptimeStream.str();
}

//////////////////////////////////////////////////////////////////////////////////////////
// InfoCore
void World::updateAllTrafficTotals()
{
    unsigned long sent = 0;
    unsigned long recieved = 0;

    double trafficIn = 0;
    double trafficOut = 0;

    mLastTrafficQuery = UNIXTIME;
    mLastTotalTrafficInKB = mTotalTrafficInKB;
    mLastTotalTrafficOutKB = mTotalTrafficOutKB;

    std::lock_guard guard(sObjectMgr.m_playerLock);
    for (const auto playerStorage : sObjectMgr.getPlayerStorage())
    {
        WorldSocket* socket = playerStorage.second->getSession()->GetSocket();
        if (!socket || !socket->IsConnected() || socket->IsDeleted())
            continue;

        socket->PollTraffic(&sent, &recieved);

        trafficIn += (static_cast<double>(recieved));
        trafficOut += (static_cast<double>(sent));
    }

    mTotalTrafficInKB += (trafficIn / 1024.0);
    mTotalTrafficOutKB += (trafficOut / 1024.0);
}

void World::setTotalTraffic(double* totalin, double* totalout)
{
    if (mLastTrafficQuery == 0 || mLastTrafficQuery <= (UNIXTIME - 10))
        updateAllTrafficTotals();

    *totalin = mTotalTrafficInKB;
    *totalout = mTotalTrafficOutKB;
}

void World::setLastTotalTraffic(double* totalin, double* totalout)
{
    *totalin = mLastTotalTrafficInKB;
    *totalout = mLastTotalTrafficOutKB;
}

float World::getCPUUsage()
{
    return perfcounter.GetCurrentCPUUsage();
}

float World::getRAMUsage()
{
    return perfcounter.GetCurrentRAMUsage();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Session functions
void World::addSession(std::unique_ptr<WorldSession> sessionHolder)
{
    if (sessionHolder)
    {
        std::lock_guard<std::mutex> guard(mSessionLock);

        auto* worldSession = sessionHolder.get();
        mActiveSessionMapStore[sessionHolder->GetAccountId()] = std::move(sessionHolder);

        if (static_cast<uint32_t>(mActiveSessionMapStore.size()) > getPeakSessionCount())
            setNewPeakSessionCount(static_cast<uint32_t>(mActiveSessionMapStore.size()));

#ifndef AE_TBC
        worldSession->sendAccountDataTimes(GLOBAL_CACHE_MASK);
#endif
    }
}

WorldSession* World::getSessionByAccountId(uint32_t accountId)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    WorldSession* worldSession = nullptr;

    auto activeSessions = mActiveSessionMapStore.find(accountId);
    if (activeSessions != mActiveSessionMapStore.end())
        worldSession = activeSessions->second.get();


    return worldSession;
}

WorldSession* World::getSessionByAccountName(const std::string& accountName)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    WorldSession* worldSession = nullptr;

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (accountName == activeSessions->second->GetAccountName())
        {
            worldSession = activeSessions->second.get();
            break;
        }
    }

    return worldSession;
}

void World::sendCharacterEnumToAccountSession(QueryResultVector& results, uint32_t accountId)
{
    WorldSession* worldSession = getSessionByAccountId(accountId);
    if (worldSession != nullptr)
        worldSession->characterEnumProc(results[0].result.get());
}

void World::loadAccountDataProcForId(QueryResultVector& results, uint32_t accountId)
{
    WorldSession* worldSession = getSessionByAccountId(accountId);
    if (worldSession != nullptr)
        worldSession->loadAccountDataProc(results[0].result.get());
}

size_t World::getSessionCount()
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    size_t ssize = mActiveSessionMapStore.size();

    return ssize;
}

void World::deleteSession(WorldSession* worldSession)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    mActiveSessionMapStore.erase(worldSession->GetAccountId());
}

void World::deleteSessions(std::list<WorldSession*>& slist)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto sessionList = slist.begin(); sessionList != slist.end(); ++sessionList)
    {
        WorldSession* session = *sessionList;
        mActiveSessionMapStore.erase(session->GetAccountId());
    }
}

void World::disconnectSessionByAccountName(const std::string& accountName, WorldSession* worldSession)
{
    bool isUserFound = false;

    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        WorldSession* session = activeSessions->second.get();
        if (accountName == session->GetAccountName())
        {
            isUserFound = true;
            worldSession->SystemMessage("Disconnecting user with account `%s` IP `%s` Player `%s`.", session->GetAccountNameS(),
                session->GetSocket() ? session->GetSocket()->GetRemoteIP().c_str() : "noip", session->GetPlayer() ? session->GetPlayer()->getName().c_str() : "noplayer");

            session->Disconnect();
        }
    }

    if (!isUserFound)
        worldSession->SystemMessage("There is nobody online with account [%s]", accountName.c_str());
}

void World::disconnectSessionByIp(const std::string& ipString, WorldSession* worldSession)
{
    bool isUserFound = false;

    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        WorldSession* session = activeSessions->second.get();
        if (!session->GetSocket())
            continue;

        std::string ip2 = session->GetSocket()->GetRemoteIP();
        if (ipString == ip2)
        {
            isUserFound = true;
            worldSession->SystemMessage("Disconnecting user with account `%s` IP `%s` Player `%s`.", session->GetAccountNameS(),
                ip2.c_str(), session->GetPlayer() ? session->GetPlayer()->getName().c_str() : "noplayer");

            session->Disconnect();
        }
    }

    if (!isUserFound)
        worldSession->SystemMessage("There is nobody online with ip [%s]", ipString.c_str());
}

void World::disconnectSessionByPlayerName(const std::string& playerName, WorldSession* worldSession)
{
    bool isUserFound = false;

    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        WorldSession* session = activeSessions->second.get();
        if (!session->GetPlayer())
            continue;

        if (playerName == session->GetPlayer()->getName())
        {
            isUserFound = true;
            worldSession->SystemMessage("Disconnecting user with account `%s` IP `%s` Player `%s`.", session->GetAccountNameS(),
                session->GetSocket() ? session->GetSocket()->GetRemoteIP().c_str() : "noip", session->GetPlayer() ? session->GetPlayer()->getName().c_str() : "noplayer");

            session->Disconnect();
        }
    }

    if (!isUserFound)
        worldSession->SystemMessage("There is no body online with the name [%s]", playerName.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////
// GlobalSession functions - not used?
void World::addGlobalSession(WorldSession* worldSession)
{
    if (worldSession)
        globalSessionSet.insert(worldSession);
}

void World::updateGlobalSession(uint32_t /*diff*/)
{
    std::list<WorldSession*> ErasableSessions;

    for (SessionSet::iterator itr = globalSessionSet.begin(); itr != globalSessionSet.end();)
    {
        WorldSession* session = (*itr);
        SessionSet::iterator it2 = itr;
        ++itr;
        if (!session || session->GetInstance() != 0)
        {
            globalSessionSet.erase(it2);
            continue;
        }

        if (int result = session->Update(0))
        {
            if (result == 1)
                ErasableSessions.push_back(session);

            globalSessionSet.erase(it2);
        }
    }

    deleteSessions(ErasableSessions);
    ErasableSessions.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Session queue
void World::updateQueuedSessions(uint32_t diff)
{
    if (diff >= getQueueUpdateTimer())
    {
        mQueueUpdateTimer = settings.server.queueUpdateInterval;

        std::lock_guard lock(queueMutex);

        if (getQueuedSessions() == 0)
        {
            return;
        }

        while (mActiveSessionMapStore.size() < settings.getPlayerLimit() && getQueuedSessions())
        {
            QueuedWorldSocketList::iterator iter = mQueuedSessions.begin();
            WorldSocket* QueuedSocket = (*iter).first;
            auto&& sessionHolder = std::move((*iter).second);
            mQueuedSessions.erase(iter);

            if (QueuedSocket->GetSession() && sessionHolder != nullptr)
            {
                std::lock_guard guard(QueuedSocket->GetSession()->deleteMutex);
                QueuedSocket->Authenticate(std::move(sessionHolder));
            }
        }

        if (getQueuedSessions() == 0)
        {
            return;
        }

        QueuedWorldSocketList::iterator iter = mQueuedSessions.begin();
        uint32_t queuPosition = 1;
        while (iter != mQueuedSessions.end())
        {
            (*iter).first->UpdateQueuePosition(queuPosition++);
            if (iter == mQueuedSessions.end())
                break;

            ++iter;
        }
    }
    else
    {
        mQueueUpdateTimer -= diff;
    }
}

uint32_t World::addQueuedSocket(WorldSocket* socket, std::unique_ptr<WorldSession> sessionHolder)
{
    std::lock_guard lock(queueMutex);

    mQueuedSessions.emplace_back(socket, std::move(sessionHolder));

    return getQueuedSessions();
}

void World::removeQueuedSocket(WorldSocket* socket)
{
    std::lock_guard lock(queueMutex);

    for (QueuedWorldSocketList::iterator iter = mQueuedSessions.begin(); iter != mQueuedSessions.end(); ++iter)
    {
        if ((*iter).first == socket)
        {
            mQueuedSessions.erase(iter);
            return;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Send Messages
void World::sendMessageToOnlineGms(const std::string& message, WorldSession* sendToSelf /*nullptr*/)
{
    const auto data = AscEmu::Packets::SmsgMessageChat(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, 0, message).serialise();

    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (activeSessions->second->GetPlayer() && activeSessions->second->GetPlayer()->IsInWorld() && activeSessions->second.get() != sendToSelf)
        {
            if (activeSessions->second->CanUseCommand('u'))
                activeSessions->second->SendPacket(data.get());
        }
    }
}

void World::sendMessageToAll(const std::string& message, WorldSession* sendToSelf /*nullptr*/)
{
    const auto data = AscEmu::Packets::SmsgMessageChat(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, 0, message).serialise();

    sendGlobalMessage(data.get(), sendToSelf);

    if (settings.announce.showAnnounceInConsoleOutput)
    {
        sLogger.info("WORLD : SendWorldText {}", message);
    }
}

void World::sendAreaTriggerMessage(const std::string& message, WorldSession* sendToSelf /*nullptr*/)
{
    sendGlobalMessage(AscEmu::Packets::SmsgAreaTriggerMessage(0, message.c_str(), 0).serialise().get(), sendToSelf);
}

void World::sendGlobalMessage(WorldPacket* worldPacket, WorldSession* sendToSelf /*nullptr*/, uint32_t team /*3*/)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (activeSessions->second->GetPlayer() && activeSessions->second->GetPlayer()->IsInWorld()
            && activeSessions->second.get() != sendToSelf && (team == 3 || activeSessions->second->GetPlayer()->GetTeam() == team))
            activeSessions->second->SendPacket(worldPacket);
    }
}

void World::sendZoneMessage(WorldPacket* worldPacket, uint32_t zoneId, WorldSession* sendToSelf /*nullptr*/)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (activeSessions->second->GetPlayer() && activeSessions->second->GetPlayer()->IsInWorld() && activeSessions->second.get() != sendToSelf)
        {
            if (activeSessions->second->GetPlayer()->getZoneId() == zoneId)
                activeSessions->second->SendPacket(worldPacket);
        }
    }
}

void World::sendInstanceMessage(WorldPacket* worldPacket, uint32_t instanceId, WorldSession* sendToSelf /*nullptr*/)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (activeSessions->second->GetPlayer() && activeSessions->second->GetPlayer()->IsInWorld() && activeSessions->second.get() != sendToSelf)
        {
            if (activeSessions->second->GetPlayer()->GetInstanceID() == static_cast<int32_t>(instanceId))
                activeSessions->second->SendPacket(worldPacket);
        }
    }
}

void World::sendZoneUnderAttackMessage(uint32_t areaId, uint8_t teamId)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        Player* player = activeSessions->second->GetPlayer();
        if (player != nullptr && player->IsInWorld())
        {
            if (player->getTeam() == teamId)
                activeSessions->second->SendPacket(AscEmu::Packets::SmsgZoneUnderAttack(areaId).serialise().get());
        }
    }
}

void World::sendBroadcastMessageById(uint32_t broadcastId)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (activeSessions->second->GetPlayer() && activeSessions->second->GetPlayer()->IsInWorld())
        {
            const char* text = activeSessions->second->LocalizedBroadCast(broadcastId);

            const auto data = AscEmu::Packets::SmsgMessageChat(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, 0, text).serialise();

            activeSessions->second->SendPacket(data.get());
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// General Functions
bool World::setInitialWorldSettings()
{
    auto startTime = Util::TimeNow();

    (new IdleMovementFactory())->registerSelf();
    (new RandomMovementFactory())->registerSelf();
    (new WaypointMovementFactory())->registerSelf();

    Player::initVisibleUpdateBits();

    resetCharacterLoginBannState();

    if (!loadDbcDb2Stores())
        return false;

#if VERSION_STRING < Cata
    loadDbcLocaleLanguage();
    auto localeString = Util::getLanguagesStringFromId(mDbcLocaleId);
    if (mDbcLocaleId == 0)
        localeString.append("/enUS");
    else if (mDbcLocaleId == 10)
        localeString.append("/ptPT");

    sLogger.info("World : Using {} DBC locale", localeString);
#endif

    sTaxiMgr.initialize();
    sChatHandler.initialize();
    sSpellProcMgr.initialize();

    sWorldPacketLog.initialize();
    sWorldPacketLog.initWorldPacketLog(worldConfig.logger.enableWorldPacketLog);

    sLogger.info("World : Loading SpellInfo data...");
    sSpellMgr.initialize();

    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        sLogger.info("GameObjectModel : Loading GameObject models...");
        std::string vmapPath = worldConfig.server.dataDir + "vmaps";
        LoadGameObjectModelList(vmapPath);
    }

    // Initialize Vmaps Liquid
    VMAP::VMapManager2* vmmgr2 = VMAP::VMapFactory::createOrGetVMapManager();
    vmmgr2->GetLiquidFlagsPtr = &getLiquidFlags;

    sObjectMgr.initialize();
    sInstanceMgr.loadInstances();
    loadMySQLStores();

    sLogger.info("World : Loading loot data...");
    sLootMgr.initialize();
    sLootMgr.loadLoot();

    loadMySQLTablesByTask();

    sMapMgr.initialize();

    logEntitySize();

    sSpellMgr.loadSpellDataFromDatabase();
    sSpellMgr.calculateSpellCoefficients();

#if VERSION_STRING > TBC
    sLogger.info("World : Starting Achievement System...");
    sObjectMgr.loadAchievementCriteriaList();
#endif

#if VERSION_STRING <= Cata // support MOP
    sLogger.info("World : Starting Transport System...");
    sTransportHandler.loadTransportTemplates();
    sTransportHandler.spawnContinentTransports();

    sLogger.info("World : Starting Mail System...");
    sMailSystem.StartMailSystem();

    sLogger.info("World : Starting Auction System...");
    sAuctionMgr.initialize();
    sAuctionMgr.loadAuctionHouses();

    sLogger.info("World : Loading LFG rewards...");
    sLfgMgr.initialize();
    sLfgMgr.LoadRewards();
#endif

    sGuildMgr.loadGuildDataFromDB();

#if VERSION_STRING >= Cata
    sGuildMgr.loadGuildXpForLevelFromDB();
    sGuildMgr.loadGuildRewardsFromDB();

    sGuildFinderMgr.loadGuildFinderDataFromDB();
#endif

    mQueueUpdateTimer = settings.server.queueUpdateInterval;

    sChannelMgr.loadConfigSettings();

    sLogger.info("World : Starting BattlegroundManager...");
    sBattlegroundManager.initialize();

    dw = std::move(std::make_unique<DayWatcherThread>());

    broadcastMgr = std::move(std::make_unique<BroadcastMgr>());

    sLogger.info("World: init in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    return true;
}

void World::resetCharacterLoginBannState()
{
    CharacterDatabase.WaitExecute("UPDATE characters SET online = 0 WHERE online = 1");
    CharacterDatabase.WaitExecute("UPDATE characters SET banned= 0,banReason='' WHERE banned > 100 AND banned < %u", UNIXTIME);
}

bool World::loadDbcDb2Stores()
{
    sLogger.info("World : Loading DBC files...");
    if (!loadDBCs())
    {
        sLogger.fatal("One or more of the DBC files are missing.", "These are absolutely necessary for the server to function.", "The server will not start without them.", "");
        return false;
    }

    return true;
}

#if VERSION_STRING < Cata
void World::loadDbcLocaleLanguage()
{
    // Read names from warrior class in ChrClasses DBC file to get used locale language
    const auto warr = sChrClassesStore.lookupEntry(1);
#if VERSION_STRING == Classic
    for (uint8_t i = 0; i < 8; ++i)
#else
    for (uint8_t i = 0; i < 16; ++i)
#endif
    {
        std::string name(warr->name[i]);
        if (!name.empty())
        {
            mDbcLocaleId = i;
            break;
        }
    }
}

uint8_t World::getDbcLocaleLanguageId() const
{
    return mDbcLocaleId;
}
#endif

void World::loadMySQLStores()
{
    auto startTime = Util::TimeNow();

    sMySQLStore.loadAdditionalTableConfig();

    sMySQLStore.loadItemPagesTable();
    sMySQLStore.loadItemPropertiesTable();
    sMySQLStore.loadCreaturePropertiesMovementTable();
    sMySQLStore.loadCreaturePropertiesTable();
    sMySQLStore.loadGameObjectPropertiesTable();
    sMySQLStore.loadQuestPropertiesTable();
    sMySQLStore.loadGameObjectQuestItemBindingTable();
    sMySQLStore.loadGameObjectQuestPickupBindingTable();

    sMySQLStore.loadCreatureDifficultyTable();
    sMySQLStore.loadDisplayBoundingBoxesTable();
    sMySQLStore.loadVendorRestrictionsTable();

    sMySQLStore.loadNpcTextTable();
    sMySQLStore.loadNpcScriptTextTable();
    sMySQLStore.loadGossipMenuOptionTable();
    sMySQLStore.loadGraveyardsTable();
    sMySQLStore.loadTeleportCoordsTable();
    sMySQLStore.loadFishingTable();
    sMySQLStore.loadWorldMapInfoTable();
    sMySQLStore.loadZoneGuardsTable();
    sMySQLStore.loadBattleMastersTable();
    sMySQLStore.loadTotemDisplayIdsTable();
    sMySQLStore.loadSpellClickSpellsTable();

    sMySQLStore.loadWorldStringsTable();
    sMySQLStore.loadPointsOfInterestTable();
    sMySQLStore.loadItemSetLinkedSetBonusTable();
    sMySQLStore.loadCreatureInitialEquipmentTable();

    sMySQLStore.loadPlayerCreateInfoTable();
    sMySQLStore.loadPlayerCreateInfoBars();
    sMySQLStore.loadPlayerCreateInfoItems();
    sMySQLStore.loadPlayerCreateInfoSkills();
    sMySQLStore.loadPlayerCreateInfoSpellLearn();
    sMySQLStore.loadPlayerCreateInfoSpellCast();
    sMySQLStore.loadPlayerCreateInfoLevelstats();
    sMySQLStore.loadPlayerCreateInfoClassLevelstats();
    sMySQLStore.loadPlayerXpToLevelTable();

    sMySQLStore.loadSpellOverrideTable();

    sMySQLStore.loadNpcGossipTextIdTable();
    sMySQLStore.loadPetLevelAbilitiesTable();
    sMySQLStore.loadBroadcastTable();

    sMySQLStore.loadAreaTriggerTable();
    sMySQLStore.loadWordFilterCharacterNames();
    sMySQLStore.loadWordFilterChat();

    sMySQLStore.loadLocalesCreature();
    sMySQLStore.loadLocalesGameobject();
    sMySQLStore.loadLocalesGossipMenuOption();
    sMySQLStore.loadLocalesItem();
    sMySQLStore.loadLocalesItemPages();
    sMySQLStore.loadLocalesNpcScriptText();
    sMySQLStore.loadLocalesNpcText();
    sMySQLStore.loadLocalesQuest();
    sMySQLStore.loadLocalesWorldbroadcast();
    sMySQLStore.loadLocalesWorldmapInfo();
    sMySQLStore.loadLocalesWorldStringTable();

    //sMySQLStore.loadDefaultPetSpellsTable();      Zyres 2017/07/16 not used
    sMySQLStore.loadProfessionDiscoveriesTable();

    sMySQLStore.loadTransportDataTable();
    sMySQLStore.loadTransportEntrys();
    sMySQLStore.loadGossipMenuItemsTable();
    sMySQLStore.loadRecallTable();
    sMySQLStore.loadCreatureAIScriptsTable();
    sMySQLStore.loadSpawnGroupIds();

    sLogger.info("Done. MySQLStore loaded in {} ms.", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    sFormationMgr->loadCreatureFormations();
    sWaypointMgr->load();
    sWaypointMgr->loadCustomWaypoints();
    sObjectMgr.loadCreatureMovementOverrides();
}

void World::loadMySQLTablesByTask()
{
    auto startTime = Util::TimeNow();

    sAddonMgr.initialize();
    sTicketMgr.initialize();
    sGameEventMgr.initialize();


    sObjectMgr.generateLevelUpInfo();
    sObjectMgr.loadCharacters();

    sMySQLStore.loadCreatureSpawns();
    sMySQLStore.loadGameobjectSpawns();
    sMySQLStore.loadGameObjectSpawnsExtraTable();
    sMySQLStore.loadGameObjectSpawnsOverrideTable();

    sMySQLStore.loadCreatureGroupSpawns();
    sMySQLStore.loadCreatureSplineChains();

    sObjectMgr.loadInstanceEncounters();
    sObjectMgr.loadCreatureTimedEmotes();
    sObjectMgr.loadVendors();
    sObjectMgr.loadTrainerSpellSets();
    sObjectMgr.loadTrainers();
    sObjectMgr.loadPetSpellCooldowns();
    sObjectMgr.loadCharters();
    sTicketMgr.loadGMTickets();
    sObjectMgr.setHighestGuids();
    sObjectMgr.loadReputationModifiers();
    sObjectMgr.loadGroups();
    sObjectMgr.loadGroupInstances();
    sObjectMgr.loadArenaTeams();
#ifdef FT_VEHICLES
    sObjectMgr.loadVehicleAccessories();
    sObjectMgr.loadVehicleSeatAddon();
#endif
    sObjectMgr.loadWorldStateTemplates();

#if VERSION_STRING > TBC
    sObjectMgr.loadAchievementRewards();
#endif

    sLootMgr.loadAndGenerateLoot(0);
    sLootMgr.loadAndGenerateLoot(1);
    sLootMgr.loadAndGenerateLoot(2);
    sLootMgr.loadAndGenerateLoot(3);
    sLootMgr.loadAndGenerateLoot(4);
    sLootMgr.loadAndGenerateLoot(5);

    sQuestMgr.LoadExtraQuestStuff();
    sObjectMgr.loadEventScripts();
    sWeatherMgr.loadFromDB();
    sAddonMgr.LoadFromDB();
    sGameEventMgr.LoadFromDB();
    sCalendarMgr.loadFromDB();

    sCommandTableStorage.Load();
    sLogger.info("WordFilter : Loading...");

    g_chatFilter = std::make_unique<WordFilter>();

    sLogger.info("Done. Database loaded in {} ms.", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void World::logEntitySize()
{
    sLogger.info("World : Object size: {} bytes", sizeof(Object));
    sLogger.info("World : Unit size: {} bytes", sizeof(Unit) + sizeof(AIInterface));
    sLogger.info("World : Creature size: {} bytes", sizeof(Creature) + sizeof(AIInterface));
    sLogger.info("World : Player size: {} bytes", sizeof(Player) + sizeof(ItemInterface) + 50000 + 30000 + 1000 + sizeof(AIInterface));
    sLogger.info("World : GameObject size: {} bytes", sizeof(GameObject));
}

void World::Update(unsigned long timePassed)
{
    sLfgMgr.Update(static_cast<uint32_t>(timePassed));
    mEventableObjectHolder->Update(static_cast<uint32_t>(timePassed));
    sAuctionMgr.update();
    updateQueuedSessions(static_cast<uint32_t>(timePassed));
    sMapMgr.update();
    sInstanceMgr.update();
    sGuildMgr.update(static_cast<uint32_t>(timePassed));
}

void World::saveAllPlayersToDb()
{
    sLogger.info("Saving all players to database...");

    uint32_t count = 0;

    std::lock_guard guard(sObjectMgr.m_playerLock);
    for (const auto playerPair : sObjectMgr.getPlayerStorage())
    {
        Player* player = playerPair.second;
        if (player->getSession())
        {
            const auto startTime = Util::TimeNow();
            player->saveToDB(false);
            sLogger.info("Saved player `{}` (level {}) in {} ms.", player->getName(), player->getLevel(), static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
            ++count;
        }
    }

    sLogger.info("Saved {} players.", count);
}

void World::playSoundToAllPlayers(uint32_t soundId)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    for (activeSessionMap::iterator itr = mActiveSessionMapStore.begin(); itr != mActiveSessionMapStore.end(); ++itr)
    {
        WorldSession* worldSession = itr->second.get();
        if ((worldSession->GetPlayer() != nullptr) && worldSession->GetPlayer()->IsInWorld())
            worldSession->SendPacket(AscEmu::Packets::SmsgPlaySound(soundId).serialise().get());
    }
}

void World::logoutAllPlayers()
{
    sLogger.info("World : Logging out players...");
    for (activeSessionMap::iterator i = mActiveSessionMapStore.begin(); i != mActiveSessionMapStore.end(); ++i)
        (i->second)->LogoutPlayer(true);

    sLogger.info("World : Deleting sessions...");
    for (activeSessionMap::iterator i = mActiveSessionMapStore.begin(); i != mActiveSessionMapStore.end();)
    {
        WorldSession* worldSession = i->second.get();
        ++i;
        deleteSession(worldSession);
    }
}

void World::deleteObject(Object* object)
{
    delete object;
}

//////////////////////////////////////////////////////////////////////////////////////////
// GM Ticket System
bool World::getGmTicketStatus()
{
    return mGmTicketSystemEnabled;
}

void World::toggleGmTicketStatus()
{
    mGmTicketSystemEnabled = !mGmTicketSystemEnabled;
}
