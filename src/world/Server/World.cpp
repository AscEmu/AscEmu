/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "WorldConf.h"
#include "Management/AddonMgr.h"
#include "Management/AuctionMgr.h"
#include "Management/CalendarMgr.h"
#include "Management/Item.h"
#include "Management/LFG/LFGMgr.h"
#include "Management/WordFilter.h"
#include "Management/WeatherMgr.h"
#include "Management/TaxiMgr.h"
#include "Management/ItemInterface.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "WorldSocket.h"
#include "Storage/MySQLDataStore.hpp"
#include <CrashHandler.h>
#include "Server/MainServerDefines.h"
#include "Config/Config.h"
#include "Map/MapCell.h"
#include "Spell/SpellMgr.h"
#include "Map/WorldCreator.h"
#include "Storage/DayWatcherThread.h"
#include "BroadcastMgr.h"
#include "World.Legacy.h"
#include "Spell/Customization/SpellCustomizations.hpp"

#if VERSION_STRING == Cata
#include "GameCata/Management/GuildMgr.h"
#include "GameCata/Management/GuildFinderMgr.h"
#endif

initialiseSingleton(World);

DayWatcherThread* dw = nullptr;

std::unique_ptr<BroadcastMgr> broadcastMgr = nullptr;

extern void ApplyNormalFixes();
extern void LoadGameObjectModelList(std::string const& dataPath);
void CleanupRandomNumberGenerators();

World::World()
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
    mEventableObjectHolder = new EventableObjectHolder(WORLD_INSTANCE);
    m_holder = mEventableObjectHolder;
    m_event_Instanceid = mEventableObjectHolder->GetInstanceID();

    //////////////////////////////////////////////////////////////////////////////////////////
    // GM Ticket System
    mGmTicketSystemEnabled = true;
}

World::~World()
{
    LogNotice("WorldLog : ~WorldLog()");
    delete WorldPacketLog::getSingletonPtr();

    LogNotice("ObjectMgr : ~ObjectMgr()");
    delete ObjectMgr::getSingletonPtr();

    LogNotice("LootMgr : ~LootMgr()");
    delete LootMgr::getSingletonPtr();

    LogNotice("LfgMgr : ~LfgMgr()");
    delete LfgMgr::getSingletonPtr();

    LogNotice("ChannelMgr : ~ChannelMgr()");
    delete ChannelMgr::getSingletonPtr();

    LogNotice("QuestMgr : ~QuestMgr()");
    delete QuestMgr::getSingletonPtr();

    LogNotice("WeatherMgr : ~WeatherMgr()");
    delete WeatherMgr::getSingletonPtr();

    LogNotice("TaxiMgr : ~TaxiMgr()");
    delete TaxiMgr::getSingletonPtr();

#if VERSION_STRING == Cata
    LogNotice("GuildMgr", "~GuildMgr()");
    delete GuildMgr::getSingletonPtr();

    LogNotice("GuildFinderMgr", "~GuildFinderMgr()");
    delete GuildFinderMgr::getSingletonPtr();
#endif

    LogNotice("BattlegroundMgr : ~BattlegroundMgr()");
    delete CBattlegroundManager::getSingletonPtr();

    LogNotice("InstanceMgr : ~InstanceMgr()");
    sInstanceMgr.Shutdown();

    LogNotice("WordFilter : ~WordFilter()");
    delete g_chatFilter;

    LogNotice("Rnd : ~Rnd()");
    CleanupRandomNumberGenerators();

    LogNotice("SpellProcMgr : ~SpellProcMgr()");
    delete SpellProcMgr::getSingletonPtr();

    LogNotice("SpellFactoryMgr : ~SpellFactoryMgr()");
    delete SpellFactoryMgr::getSingletonPtr();

    LogNotice("MySQLDataStore : ~MySQLDataStore()");
    delete MySQLDataStore::getSingletonPtr();

    delete mEventableObjectHolder;

    for (std::list<SpellInfo*>::iterator itr = dummySpellList.begin(); itr != dummySpellList.end(); ++itr)
        delete *itr;
}

//////////////////////////////////////////////////////////////////////////////////////////
// WorldConfig
void World::loadWorldConfigValues(bool reload /*false*/)
{
    settings.loadWorldConfigValues(reload);

    if (reload)
        Channel::LoadConfSettings();
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
    return (uint32_t)UNIXTIME - mStartTime;
}

std::string World::getWorldUptimeString()
{
    time_t pTime = (time_t)UNIXTIME - mStartTime;
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

    double TrafficIn = 0;
    double TrafficOut = 0;

    mLastTrafficQuery = UNIXTIME;
    mLastTotalTrafficInKB = mTotalTrafficInKB;
    mLastTotalTrafficOutKB = mTotalTrafficOutKB;

    objmgr._playerslock.AcquireReadLock();

    for (auto playerStorage = objmgr._players.begin(); playerStorage != objmgr._players.end(); ++playerStorage)
    {
        WorldSocket* socket = playerStorage->second->GetSession()->GetSocket();
        if (!socket || !socket->IsConnected() || socket->IsDeleted())
            continue;

        socket->PollTraffic(&sent, &recieved);

        TrafficIn += (static_cast<double>(recieved));
        TrafficOut += (static_cast<double>(sent));
    }

    mTotalTrafficInKB += (TrafficIn / 1024.0);
    mTotalTrafficOutKB += (TrafficOut / 1024.0);

    objmgr._playerslock.ReleaseReadLock();
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
void World::addSession(WorldSession* worldSession)
{
    ARCEMU_ASSERT(worldSession != NULL);

    mSessionLock.AcquireWriteLock();

    mActiveSessionMapStore[worldSession->GetAccountId()] = worldSession;

    if ((uint32_t)mActiveSessionMapStore.size() > getPeakSessionCount())
        setNewPeakSessionCount((uint32_t)mActiveSessionMapStore.size());

    worldSession->SendAccountDataTimes(GLOBAL_CACHE_MASK);

    mSessionLock.ReleaseWriteLock();
}

WorldSession* World::getSessionByAccountId(uint32_t accountId)
{
    mSessionLock.AcquireReadLock();

    WorldSession* worldSession = nullptr;

    auto activeSessions = mActiveSessionMapStore.find(accountId);
    if (activeSessions != mActiveSessionMapStore.end())
        worldSession = activeSessions->second;

    mSessionLock.ReleaseReadLock();

    return worldSession;
}

WorldSession* World::getSessionByAccountName(std::string accountName)
{
    mSessionLock.AcquireReadLock();

    WorldSession* worldSession = nullptr;

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (accountName.compare(activeSessions->second->GetAccountName()) == 0)
        {
            worldSession = activeSessions->second;
            break;
        }
    }

    mSessionLock.ReleaseReadLock();

    return worldSession;
}

void World::sendCharacterEnumToAccountSession(QueryResultVector& results, uint32_t accountId)
{
    WorldSession* worldSession = getSessionByAccountId(accountId);
    if (worldSession != nullptr)
        worldSession->CharacterEnumProc(results[0].result);
}

void World::loadAccountDataProcForId(QueryResultVector& results, uint32_t accountId)
{
    WorldSession* worldSession = getSessionByAccountId(accountId);
    if (worldSession != nullptr)
        worldSession->LoadAccountDataProc(results[0].result);
}

size_t World::getSessionCount()
{
    mSessionLock.AcquireReadLock();
    size_t ssize = mActiveSessionMapStore.size();
    mSessionLock.ReleaseReadLock();

    return ssize;
}

void World::deleteSession(WorldSession* worldSession)
{
    mSessionLock.AcquireWriteLock();
    mActiveSessionMapStore.erase(worldSession->GetAccountId());
    mSessionLock.ReleaseWriteLock();
    delete worldSession;
}

void World::deleteSessions(std::list<WorldSession*>& slist)
{
    mSessionLock.AcquireWriteLock();

    for (auto sessionList = slist.begin(); sessionList != slist.end(); ++sessionList)
    {
        WorldSession* session = *sessionList;
        mActiveSessionMapStore.erase(session->GetAccountId());
    }

    mSessionLock.ReleaseWriteLock();

    for (auto sessionList = slist.begin(); sessionList != slist.end(); ++sessionList)
    {
        WorldSession* session = *sessionList;
        delete session;
    }
}

void World::disconnectSessionByAccountName(std::string accountName, WorldSession* worldSession)
{
    bool isUserFound = false;

    mSessionLock.AcquireReadLock();

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        WorldSession* session = activeSessions->second;
        if (accountName.compare(session->GetAccountName()) == 0)
        {
            isUserFound = true;
            worldSession->SystemMessage("Disconnecting user with account `%s` IP `%s` Player `%s`.", session->GetAccountNameS(),
                session->GetSocket() ? session->GetSocket()->GetRemoteIP().c_str() : "noip", session->GetPlayer() ? session->GetPlayer()->GetName() : "noplayer");

            session->Disconnect();
        }
    }

    mSessionLock.ReleaseReadLock();

    if (isUserFound == false)
        worldSession->SystemMessage("There is nobody online with account [%s]", accountName.c_str());
}

void World::disconnectSessionByIp(std::string ipString, WorldSession* worldSession)
{
    bool isUserFound = false;

    mSessionLock.AcquireReadLock();

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        WorldSession* session = activeSessions->second;
        if (!session->GetSocket())
            continue;

        std::string ip2 = session->GetSocket()->GetRemoteIP().c_str();
        if (ipString.compare(ip2) == 0)
        {
            isUserFound = true;
            worldSession->SystemMessage("Disconnecting user with account `%s` IP `%s` Player `%s`.", session->GetAccountNameS(),
                ip2.c_str(), session->GetPlayer() ? session->GetPlayer()->GetName() : "noplayer");

            session->Disconnect();
        }
    }

    mSessionLock.ReleaseReadLock();

    if (isUserFound == false)
        worldSession->SystemMessage("There is nobody online with ip [%s]", ipString.c_str());
}

void World::disconnectSessionByPlayerName(std::string playerName, WorldSession* worldSession)
{
    bool isUserFound = false;

    mSessionLock.AcquireReadLock();

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        WorldSession* session = activeSessions->second;
        if (!session->GetPlayer())
            continue;

        if (playerName.compare(session->GetPlayer()->GetName()) == 0)
        {
            isUserFound = true;
            worldSession->SystemMessage("Disconnecting user with account `%s` IP `%s` Player `%s`.", session->GetAccountNameS(),
                session->GetSocket() ? session->GetSocket()->GetRemoteIP().c_str() : "noip", session->GetPlayer() ? session->GetPlayer()->GetName() : "noplayer");

            session->Disconnect();
        }
    }

    mSessionLock.ReleaseReadLock();

    if (isUserFound == false)
        worldSession->SystemMessage("There is no body online with the name [%s]", playerName.c_str());
}

//////////////////////////////////////////////////////////////////////////////////////////
// GlobalSession functions - not used?
void World::addGlobalSession(WorldSession* worldSession)
{
    ARCEMU_ASSERT(worldSession != NULL);

    globalSessionMutex.Acquire();
    globalSessionSet.insert(worldSession);
    globalSessionMutex.Release();
}

void World::updateGlobalSession(uint32_t diff)
{
    std::list<WorldSession*> ErasableSessions;

    globalSessionMutex.Acquire();

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

    globalSessionMutex.Release();

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
        queueMutex.Acquire();

        if (getQueuedSessions() == 0)
        {
            queueMutex.Release();
            return;
        }

        while (mActiveSessionMapStore.size() < settings.getPlayerLimit() && getQueuedSessions())
        {
            QueuedWorldSocketList::iterator iter = mQueuedSessions.begin();
            WorldSocket* QueuedSocket = *iter;
            mQueuedSessions.erase(iter);

            if (QueuedSocket->GetSession())
            {
                QueuedSocket->GetSession()->deleteMutex.Acquire();
                QueuedSocket->Authenticate();
                QueuedSocket->GetSession()->deleteMutex.Release();
            }
        }

        if (getQueuedSessions() == 0)
        {
            queueMutex.Release();
            return;
        }

        QueuedWorldSocketList::iterator iter = mQueuedSessions.begin();
        uint32_t queuPosition = 1;
        while (iter != mQueuedSessions.end())
        {
            (*iter)->UpdateQueuePosition(queuPosition++);
            if (iter == mQueuedSessions.end())
                break;
            else
                ++iter;
        }
        queueMutex.Release();
    }
    else
    {
        mQueueUpdateTimer -= diff;
    }
}

uint32_t World::addQueuedSocket(WorldSocket* Socket)
{
    queueMutex.Acquire();
    mQueuedSessions.push_back(Socket);
    queueMutex.Release();

    return getQueuedSessions();
}

void World::removeQueuedSocket(WorldSocket* Socket)
{
    queueMutex.Acquire();

    for (QueuedWorldSocketList::iterator iter = mQueuedSessions.begin(); iter != mQueuedSessions.end(); ++iter)
    {
        if ((*iter) == Socket)
        {
            mQueuedSessions.erase(iter);
            queueMutex.Release();
            return;
        }
    }

    queueMutex.Release();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Send Messages
void World::sendMessageToOnlineGms(std::string message, WorldSession* sendToSelf /*nullptr*/)
{
    uint32_t textLength = (uint32_t)message.size() + 1;

    WorldPacket data(SMSG_MESSAGECHAT, textLength + 40);
    data << uint8_t(CHAT_MSG_SYSTEM);
    data << uint32_t(LANG_UNIVERSAL);
    data << uint64_t(0);
    data << uint32_t(0);
    data << uint64_t(0);
    data << textLength;
    data << message.c_str();
    data << uint8_t(0);

    mSessionLock.AcquireReadLock();

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (activeSessions->second->GetPlayer() && activeSessions->second->GetPlayer()->IsInWorld() && activeSessions->second != sendToSelf)
        {
            if (activeSessions->second->CanUseCommand('u'))
                activeSessions->second->SendPacket(&data);
        }
    }
    mSessionLock.ReleaseReadLock();
}

void World::sendMessageToAll(std::string message, WorldSession* sendToSelf /*nullptr*/)
{
    uint32_t textLength = (uint32_t)message.size() + 1;

    WorldPacket data(SMSG_MESSAGECHAT, textLength + 40);
    data << uint8_t(CHAT_MSG_SYSTEM);
    data << uint32_t(LANG_UNIVERSAL);
    data << uint64_t(0);
    data << uint32_t(0);
    data << uint64_t(0);
    data << textLength;
    data << message.c_str();
    data << uint8_t(0);

    sendGlobalMessage(&data, sendToSelf);

    if (settings.announce.showAnnounceInConsoleOutput)
    {
        LogDetail("WORLD : SendWorldText %s", message.c_str());
    }
}

void World::sendAreaTriggerMessage(std::string message, WorldSession* sendToSelf /*nullptr*/)
{
    WorldPacket data(SMSG_AREA_TRIGGER_MESSAGE, 256);
    data << uint32_t(0);
    data << message.c_str();
    data << uint8_t(0);

    sendGlobalMessage(&data, sendToSelf);
}

void World::sendGlobalMessage(WorldPacket* worldPacket, WorldSession* sendToSelf /*nullptr*/)
{
    mSessionLock.AcquireReadLock();

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (activeSessions->second->GetPlayer() && activeSessions->second->GetPlayer()->IsInWorld() && activeSessions->second != sendToSelf)
            activeSessions->second->SendPacket(worldPacket);
    }

    mSessionLock.ReleaseReadLock();
}

void World::sendZoneMessage(WorldPacket* worldPacket, uint32_t zoneId, WorldSession* sendToSelf /*nullptr*/)
{
    mSessionLock.AcquireReadLock();

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (activeSessions->second->GetPlayer() && activeSessions->second->GetPlayer()->IsInWorld() && activeSessions->second != sendToSelf)
        {
            if (activeSessions->second->GetPlayer()->GetZoneId() == zoneId)
                activeSessions->second->SendPacket(worldPacket);
        }
    }

    mSessionLock.ReleaseReadLock();
}

void World::sendInstanceMessage(WorldPacket* worldPacket, uint32_t instanceId, WorldSession* sendToSelf /*nullptr*/)
{
    mSessionLock.AcquireReadLock();

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (activeSessions->second->GetPlayer() && activeSessions->second->GetPlayer()->IsInWorld() && activeSessions->second != sendToSelf)
        {
            if (activeSessions->second->GetPlayer()->GetInstanceID() == (int32)instanceId)
                activeSessions->second->SendPacket(worldPacket);
        }
    }

    mSessionLock.ReleaseReadLock();
}

void World::sendZoneUnderAttackMessage(uint32_t areaId, uint8_t teamId)
{
    WorldPacket data(SMSG_ZONE_UNDER_ATTACK, 4);
    data << uint32_t(areaId);

    mSessionLock.AcquireReadLock();

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        Player* player = activeSessions->second->GetPlayer();
        if (player != nullptr && player->IsInWorld())
        {
            if (player->GetTeam() == teamId)
                activeSessions->second->SendPacket(&data);
        }
    }

    mSessionLock.ReleaseReadLock();
}

void World::sendBroadcastMessageById(uint32_t broadcastId)
{
    mSessionLock.AcquireReadLock();

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (activeSessions->second->GetPlayer() && activeSessions->second->GetPlayer()->IsInWorld())
        {
            const char* text = activeSessions->second->LocalizedBroadCast(broadcastId);
            uint32_t textLen = (uint32_t)strlen(text) + 1;

            WorldPacket data(SMSG_MESSAGECHAT, textLen + 40);
            data << uint8_t(CHAT_MSG_SYSTEM);
            data << uint32_t(LANG_UNIVERSAL);
            data << uint64_t(0);
            data << uint32_t(0);
            data << uint64_t(0);
            data << textLen;
            data << text;
            data << uint8_t(0);

            activeSessions->second->SendPacket(&data);
        }
    }

    mSessionLock.ReleaseReadLock();
}

//////////////////////////////////////////////////////////////////////////////////////////
// General Functions
bool World::setInitialWorldSettings()
{
    uint32_t start_time = Util::getMSTime();

    Player::InitVisibleUpdateBits();

    resetCharacterLoginBannState();

    if (!loadDbcDb2Stores())
        return false;

    new TaxiMgr;
#if VERSION_STRING == Cata
    new GuildFinderMgr;
    new GuildMgr;
#endif
    new ChatHandler;
    new SpellProcMgr;

    new WorldPacketLog;
    sWorldPacketLog.initWorldPacketLog(worldConfig.log.enableWorldPacketLog);

    new SpellCustomizations;
    sSpellCustomizations.StartSpellCustomization();

    ApplyNormalFixes();

    LogNotice("GameObjectModel : Loading GameObject models...");
    std::string vmapPath = worldConfig.server.dataDir + "vmaps";
    LoadGameObjectModelList(vmapPath);

    loadMySQLStores();
    loadMySQLTablesByTask();
    logEntitySize();

#if VERSION_STRING > TBC
    LogDetail("World : Starting Achievement System..");
    objmgr.LoadAchievementCriteriaList();
#endif

    LogDetail("World : Starting Transport System...");
    objmgr.LoadTransports();

    LogDetail("World : Starting Mail System...");
    new MailSystem;
    sMailSystem.StartMailSystem();

    LogDetail("World : Starting Auction System...");
    new AuctionMgr;
    sAuctionMgr.LoadAuctionHouses();

    LogDetail("World : Loading LFG rewards...");
    new LfgMgr;
    sLfgMgr.LoadRewards();

#if VERSION_STRING == Cata
    sGuildMgr.loadGuildXpForLevelFromDB();
    sGuildMgr.loadGuildRewardsFromDB();
    sGuildMgr.loadGuildDataFromDB();

    sGuildFinderMgr.loadGuildFinderDataFromDB();
#endif

    mQueueUpdateTimer = settings.server.queueUpdateInterval;

    LogNotice("World : Loading loot data...");
    new LootMgr;
    lootmgr.LoadLoot();

    Channel::LoadConfSettings();

    LogDetail("World : Starting CBattlegroundManager...");
    new CBattlegroundManager;

    dw = new DayWatcherThread();
    ThreadPool.ExecuteTask(dw);

    broadcastMgr = std::move(std::make_unique<BroadcastMgr>());

    ThreadPool.ExecuteTask(new CharacterLoaderThread());

    sEventMgr.AddEvent(this, &World::checkForExpiredInstances, EVENT_WORLD_UPDATEAUCTIONS, 120000, 0, 0);
    return true;
}

void World::resetCharacterLoginBannState()
{
    CharacterDatabase.WaitExecute("UPDATE characters SET online = 0 WHERE online = 1");
    CharacterDatabase.WaitExecute("UPDATE characters SET banned= 0,banReason='' WHERE banned > 100 AND banned < %u", UNIXTIME);
}

bool World::loadDbcDb2Stores()
{
#if VERSION_STRING == Cata
    LoadDB2Stores();
#endif

    LogNotice("World : Loading DBC files...");
    if (!LoadDBCs())
    {
        AscLog.ConsoleLogMajorError("One or more of the DBC files are missing.", "These are absolutely necessary for the server to function.", "The server will not start without them.", "");
        return false;
    }

    return true;
}

void World::loadMySQLStores()
{
    new MySQLDataStore;

    sMySQLStore.loadAdditionalTableConfig();

    sMySQLStore.loadItemPagesTable();
    sMySQLStore.loadItemPropertiesTable();
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
    sMySQLStore.loadPlayerCreateInfoSkillsTable();
    sMySQLStore.loadPlayerCreateInfoSpellsTable();
    sMySQLStore.loadPlayerCreateInfoItemsTable();
    sMySQLStore.loadPlayerXpToLevelTable();

    sMySQLStore.loadSpellOverrideTable();

    sMySQLStore.loadNpcGossipTextIdTable();
    sMySQLStore.loadPetLevelAbilitiesTable();
    sMySQLStore.loadBroadcastTable();

    sMySQLStore.loadAreaTriggerTable();
    sMySQLStore.loadWordFilterCharacterNames();
    sMySQLStore.loadWordFilterChat();
    sMySQLStore.loadCreatureFormationsTable();

    sMySQLStore.loadLocalesCreature();
    sMySQLStore.loadLocalesGameobject();
    sMySQLStore.loadLocalesGossipMenuOption();
    sMySQLStore.loadLocalesItem();
    sMySQLStore.loadLocalesItemPages();
    sMySQLStore.loadLocalesNPCMonstersay();
    sMySQLStore.loadLocalesNpcScriptText();
    sMySQLStore.loadLocalesNpcText();
    sMySQLStore.loadLocalesQuest();
    sMySQLStore.loadLocalesWorldbroadcast();
    sMySQLStore.loadLocalesWorldmapInfo();
    sMySQLStore.loadLocalesWorldStringTable();

    sMySQLStore.loadNpcMonstersayTable();
    //sMySQLStore.loadDefaultPetSpellsTable();      Zyres 2017/07/16 not used
    sMySQLStore.loadProfessionDiscoveriesTable();

    sMySQLStore.loadTransportCreaturesTable();
    sMySQLStore.loadTransportDataTable();
    sMySQLStore.loadGossipMenuItemsTable();
}

void World::loadMySQLTablesByTask()
{
    auto startTime = Util::TimeNow();

    new ObjectMgr;
    new QuestMgr;
    new SpellFactoryMgr;
    new WeatherMgr;
    new AddonMgr;
    new GameEventMgr;
    new CalendarMgr;

#define MAKE_TASK(sp, ptr) tl.AddTask(new Task(new CallbackP0<sp>(sp::getSingletonPtr(), &sp::ptr)))
    // Fill the task list with jobs to do.
    TaskList tl;

    // spawn worker threads (2 * number of cpus)
    tl.spawn();

    // storage stuff has to be loaded first
    tl.wait();

    MAKE_TASK(ObjectMgr, GenerateLevelUpInfo);
    MAKE_TASK(ObjectMgr, LoadPlayersInfo);
    tl.wait();

    MAKE_TASK(ObjectMgr, LoadInstanceBossInfos);
    MAKE_TASK(ObjectMgr, LoadCreatureWaypoints);
    MAKE_TASK(ObjectMgr, LoadCreatureTimedEmotes);
    MAKE_TASK(ObjectMgr, LoadTrainers);
    MAKE_TASK(ObjectMgr, LoadSpellSkills);
    MAKE_TASK(ObjectMgr, LoadVendors);
    MAKE_TASK(ObjectMgr, LoadAIThreatToSpellId);
    MAKE_TASK(ObjectMgr, LoadSpellEffectsOverride);
    MAKE_TASK(ObjectMgr, LoadSpellTargetConstraints);
#if VERSION_STRING == Cata
    MAKE_TASK(ObjectMgr, LoadSpellRequired);
    MAKE_TASK(ObjectMgr, LoadSkillLineAbilityMap);
#endif
    MAKE_TASK(ObjectMgr, LoadPetSpellCooldowns);
    MAKE_TASK(ObjectMgr, LoadGuildCharters);
    MAKE_TASK(ObjectMgr, LoadGMTickets);
    MAKE_TASK(ObjectMgr, SetHighestGuids);
    MAKE_TASK(ObjectMgr, LoadReputationModifiers);
    MAKE_TASK(ObjectMgr, LoadGroups);
    MAKE_TASK(ObjectMgr, LoadCreatureAIAgents);
    MAKE_TASK(ObjectMgr, LoadArenaTeams);
    MAKE_TASK(ObjectMgr, LoadVehicleAccessories);
    MAKE_TASK(ObjectMgr, LoadWorldStateTemplates);

#if VERSION_STRING > TBC
    MAKE_TASK(ObjectMgr, LoadAchievementRewards);
#endif

    tl.wait();

    MAKE_TASK(QuestMgr, LoadExtraQuestStuff);
    MAKE_TASK(SpellFactoryMgr, LoadSpellAreas);
    MAKE_TASK(ObjectMgr, LoadEventScripts);
    MAKE_TASK(WeatherMgr, LoadFromDB);
    MAKE_TASK(AddonMgr, LoadFromDB);
    MAKE_TASK(GameEventMgr, LoadFromDB);
    MAKE_TASK(CalendarMgr, LoadFromDB);

#undef MAKE_TASK

    tl.wait();

    CommandTableStorage::getSingleton().Load();
    LogNotice("WordFilter : Loading...");

    g_chatFilter = new WordFilter();

    LogDetail("Done. Database loaded in %u ms.", Util::GetTimeDifferenceToNow(startTime));

    // calling this puts all maps into our task list.
    sInstanceMgr.Load(&tl);

    // wait for the events to complete.
    tl.wait();

    // wait for them to exit, now.
    tl.kill();
    tl.waitForThreadsToExit();
}

void World::logEntitySize()
{
    LogNotice("World : Object size: %u bytes", sizeof(Object));
    LogNotice("World : Unit size: %u bytes", sizeof(Unit) + sizeof(AIInterface));
    LogNotice("World : Creature size: %u bytes", sizeof(Creature) + sizeof(AIInterface));
    LogNotice("World : Player size: %u bytes", sizeof(Player) + sizeof(ItemInterface) + 50000 + 30000 + 1000 + sizeof(AIInterface));
    LogNotice("World : GameObject size: %u bytes", sizeof(GameObject));
}

void World::Update(unsigned long time_passed)
{
    sLfgMgr.Update((uint32_t)time_passed);
    mEventableObjectHolder->Update((uint32_t)time_passed);
    sAuctionMgr.Update();
    updateQueuedSessions((uint32_t)time_passed);
#ifdef SESSION_CAP
    if (GetSessionCount() >= SESSION_CAP)
        TerminateProcess(GetCurrentProcess(), 0);
#endif

#if VERSION_STRING == Cata
    sGuildMgr.update((uint32)time_passed);
#endif
}

void World::saveAllPlayersToDb()
{
    if (!(ObjectMgr::getSingletonPtr()))
        return;

    LogDefault("Saving all players to database...");

    uint32_t count = 0;
    uint32_t save_start_time;

    objmgr._playerslock.AcquireReadLock();

    for (PlayerStorageMap::const_iterator itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        if (itr->second->GetSession())
        {
            auto startTime = Util::TimeNow();
            itr->second->SaveToDB(false);
            LogDetail("Saved player `%s` (level %u) in %u ms.", itr->second->GetName(), itr->second->getLevel(), Util::GetTimeDifferenceToNow(startTime));
            ++count;
        }
    }

    objmgr._playerslock.ReleaseReadLock();
    LogDetail("Saved %u players.", count);
}

void World::playSoundToAllPlayers(uint32_t soundId)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << uint32_t(soundId);

    mSessionLock.AcquireWriteLock();

    for (activeSessionMap::iterator itr = mActiveSessionMapStore.begin(); itr != mActiveSessionMapStore.end(); ++itr)
    {
        WorldSession* worldSession = itr->second;
        if ((worldSession->GetPlayer() != nullptr) && worldSession->GetPlayer()->IsInWorld())
            worldSession->SendPacket(&data);
    }

    mSessionLock.ReleaseWriteLock();
}

void World::logoutAllPlayers()
{
    LogNotice("World : Logging out players...");
    for (activeSessionMap::iterator i = mActiveSessionMapStore.begin(); i != mActiveSessionMapStore.end(); ++i)
        (i->second)->LogoutPlayer(true);

    LogNotice("World", "Deleting sessions...");
    for (activeSessionMap::iterator i = mActiveSessionMapStore.begin(); i != mActiveSessionMapStore.end();)
    {
        WorldSession* worldSession = i->second;
        ++i;
        deleteSession(worldSession);
    }
}

void World::checkForExpiredInstances()
{
    sInstanceMgr.CheckForExpiredInstances();
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
