/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "WorldConf.h"
#include "Management/AddonMgr.h"
#include "Management/AuctionMgr.h"
#include "Management/CalendarMgr.h"
#include "Management/Item.h"
#include "Management/LFG/LFGMgr.hpp"
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
//#include "Config/Config.h"
//#include "Map/MapCell.h"
#include "Map/WorldCreator.h"
#include "Storage/DayWatcherThread.h"
#include "BroadcastMgr.h"
#include "World.Legacy.h"
#include "Spell/SpellMgr.h"
#include "Management/Guild/GuildMgr.hpp"
#include "Packets/SmsgPlaySound.h"
#include "Packets/SmsgAreaTriggerMessage.h"
#include "Packets/SmsgZoneUnderAttack.h"
#include "OpcodeTable.hpp"

#if VERSION_STRING == Cata
#include "GameCata/Management/GuildFinderMgr.h"
#elif VERSION_STRING == Mop
#include "GameMop/Management/GuildFinderMgr.h"
#endif

std::unique_ptr<DayWatcherThread> dw = nullptr;

std::unique_ptr<BroadcastMgr> broadcastMgr = nullptr;

extern void LoadGameObjectModelList(std::string const& dataPath);

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
    mEventableObjectHolder = new EventableObjectHolder(WORLD_INSTANCE);
    m_holder = mEventableObjectHolder;
    m_event_Instanceid = mEventableObjectHolder->GetInstanceID();

    //////////////////////////////////////////////////////////////////////////////////////////
    // GM Ticket System
    mGmTicketSystemEnabled = true;
}

void World::finalize()
{
    LogNotice("WorldLog : ~WorldLog()");
    sWorldPacketLog.finalize();

    LogNotice("ObjectMgr : ~ObjectMgr()");
    sObjectMgr.finalize();

    LogNotice("TicketMgr : ~TicketMgr()");
    sTicketMgr.finalize();

    LogNotice("LfgMgr : ~LfgMgr()");
    sLfgMgr.finalize();

    LogNotice("ChannelMgr : ~ChannelMgr()");
    sChannelMgr.finalize();

    LogNotice("QuestMgr : ~QuestMgr()");
    sQuestMgr.finalize();

    LogNotice("WeatherMgr : ~WeatherMgr()");
    sWeatherMgr.finalize();

    LogNotice("TaxiMgr : ~TaxiMgr()");
    sTaxiMgr.finalize();

#if VERSION_STRING >= Cata
    // todo: shouldn't this be deleted also on other versions?
    LogNotice("GuildMgr", "~GuildMgr()");
    sGuildMgr.finalize();
#endif

    LogNotice("InstanceMgr : ~InstanceMgr()");
    sInstanceMgr.Shutdown();

    LogNotice("WordFilter : ~WordFilter()");
    delete g_chatFilter;

    LogNotice("MySQLDataStore : ~MySQLDataStore()");
    sMySQLStore.finalize();

    LogNotice("OpcodeTables : finalize()");
    sOpcodeTables.finalize();

    delete mEventableObjectHolder;

    for (std::list<SpellInfo const*>::iterator itr = dummySpellList.begin(); itr != dummySpellList.end(); ++itr)
        delete *itr;
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
    time_t pTime = static_cast<time_t>(UNIXTIME) - mStartTime;
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

    sObjectMgr._playerslock.lock();

    for (auto playerStorage = sObjectMgr._players.begin(); playerStorage != sObjectMgr._players.end(); ++playerStorage)
    {
        WorldSocket* socket = playerStorage->second->GetSession()->GetSocket();
        if (!socket || !socket->IsConnected() || socket->IsDeleted())
            continue;

        socket->PollTraffic(&sent, &recieved);

        trafficIn += (static_cast<double>(recieved));
        trafficOut += (static_cast<double>(sent));
    }

    mTotalTrafficInKB += (trafficIn / 1024.0);
    mTotalTrafficOutKB += (trafficOut / 1024.0);

    sObjectMgr._playerslock.unlock();
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

    std::lock_guard<std::mutex> guard(mSessionLock);

    mActiveSessionMapStore[worldSession->GetAccountId()] = worldSession;

    if (static_cast<uint32_t>(mActiveSessionMapStore.size()) > getPeakSessionCount())
        setNewPeakSessionCount(static_cast<uint32_t>(mActiveSessionMapStore.size()));

#ifndef AE_TBC
    worldSession->sendAccountDataTimes(GLOBAL_CACHE_MASK);
#endif
}

WorldSession* World::getSessionByAccountId(uint32_t accountId)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    WorldSession* worldSession = nullptr;

    auto activeSessions = mActiveSessionMapStore.find(accountId);
    if (activeSessions != mActiveSessionMapStore.end())
        worldSession = activeSessions->second;


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
            worldSession = activeSessions->second;
            break;
        }
    }

    return worldSession;
}

void World::sendCharacterEnumToAccountSession(QueryResultVector& results, uint32_t accountId)
{
    WorldSession* worldSession = getSessionByAccountId(accountId);
    if (worldSession != nullptr)
        worldSession->characterEnumProc(results[0].result);
}

void World::loadAccountDataProcForId(QueryResultVector& results, uint32_t accountId)
{
    WorldSession* worldSession = getSessionByAccountId(accountId);
    if (worldSession != nullptr)
        worldSession->loadAccountDataProc(results[0].result);
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

    delete worldSession;
}

void World::deleteSessions(std::list<WorldSession*>& slist)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto sessionList = slist.begin(); sessionList != slist.end(); ++sessionList)
    {
        WorldSession* session = *sessionList;
        mActiveSessionMapStore.erase(session->GetAccountId());
    }

    for (auto sessionList = slist.begin(); sessionList != slist.end(); ++sessionList)
    {
        WorldSession* session = *sessionList;
        delete session;
    }
}

void World::disconnectSessionByAccountName(const std::string& accountName, WorldSession* worldSession)
{
    bool isUserFound = false;

    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        WorldSession* session = activeSessions->second;
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
        WorldSession* session = activeSessions->second;
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
        WorldSession* session = activeSessions->second;
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
    ARCEMU_ASSERT(worldSession != NULL);

    globalSessionMutex.Acquire();
    globalSessionSet.insert(worldSession);
    globalSessionMutex.Release();
}

void World::updateGlobalSession(uint32_t /*diff*/)
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

uint32_t World::addQueuedSocket(WorldSocket* socket)
{
    queueMutex.Acquire();
    mQueuedSessions.push_back(socket);
    queueMutex.Release();

    return getQueuedSessions();
}

void World::removeQueuedSocket(WorldSocket* socket)
{
    queueMutex.Acquire();

    for (QueuedWorldSocketList::iterator iter = mQueuedSessions.begin(); iter != mQueuedSessions.end(); ++iter)
    {
        if ((*iter) == socket)
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
void World::sendMessageToOnlineGms(const std::string& message, WorldSession* sendToSelf /*nullptr*/)
{
    const auto data = AscEmu::Packets::SmsgMessageChat(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, 0, message).serialise().get();

    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (activeSessions->second->GetPlayer() && activeSessions->second->GetPlayer()->IsInWorld() && activeSessions->second != sendToSelf)
        {
            if (activeSessions->second->CanUseCommand('u'))
                activeSessions->second->SendPacket(data);
        }
    }
}

void World::sendMessageToAll(const std::string& message, WorldSession* sendToSelf /*nullptr*/)
{
    const auto data = AscEmu::Packets::SmsgMessageChat(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, 0, message).serialise().get();

    sendGlobalMessage(data, sendToSelf);

    if (settings.announce.showAnnounceInConsoleOutput)
    {
        LogDetail("WORLD : SendWorldText %s", message.c_str());
    }
}

void World::sendAreaTriggerMessage(const std::string& message, WorldSession* sendToSelf /*nullptr*/)
{
    sendGlobalMessage(AscEmu::Packets::SmsgAreaTriggerMessage(0, message.c_str(), 0).serialise().get(), sendToSelf);
}

void World::sendGlobalMessage(WorldPacket* worldPacket, WorldSession* sendToSelf /*nullptr*/, int32_t team /*-1*/)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (activeSessions->second->GetPlayer() && activeSessions->second->GetPlayer()->IsInWorld()
            && activeSessions->second != sendToSelf && (team == -1 || activeSessions->second->GetPlayer()->GetTeam() == static_cast<uint32_t>(team)))
            activeSessions->second->SendPacket(worldPacket);
    }
}

void World::sendZoneMessage(WorldPacket* worldPacket, uint32_t zoneId, WorldSession* sendToSelf /*nullptr*/)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (activeSessions->second->GetPlayer() && activeSessions->second->GetPlayer()->IsInWorld() && activeSessions->second != sendToSelf)
        {
            if (activeSessions->second->GetPlayer()->GetZoneId() == zoneId)
                activeSessions->second->SendPacket(worldPacket);
        }
    }
}

void World::sendInstanceMessage(WorldPacket* worldPacket, uint32_t instanceId, WorldSession* sendToSelf /*nullptr*/)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    for (auto activeSessions = mActiveSessionMapStore.begin(); activeSessions != mActiveSessionMapStore.end(); ++activeSessions)
    {
        if (activeSessions->second->GetPlayer() && activeSessions->second->GetPlayer()->IsInWorld() && activeSessions->second != sendToSelf)
        {
            if (activeSessions->second->GetPlayer()->GetInstanceID() == static_cast<int32>(instanceId))
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

            const auto data = AscEmu::Packets::SmsgMessageChat(CHAT_MSG_SYSTEM, LANG_UNIVERSAL, 0, text).serialise().get();

            activeSessions->second->SendPacket(data);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// General Functions
bool World::setInitialWorldSettings()
{
    auto startTime = Util::TimeNow();

    Player::InitVisibleUpdateBits();

    resetCharacterLoginBannState();

    if (!loadDbcDb2Stores())
        return false;

    sTaxiMgr.initialize();
    sChatHandler.initialize();
    sSpellProcMgr.initialize();

    sWorldPacketLog.initialize();
    sWorldPacketLog.initWorldPacketLog(worldConfig.log.enableWorldPacketLog);

    LogNotice("World : Loading SpellInfo data...");
    sSpellMgr.startSpellMgr();

    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        LogNotice("GameObjectModel : Loading GameObject models...");
        std::string vmapPath = worldConfig.server.dataDir + "vmaps";
        LoadGameObjectModelList(vmapPath);
    }

    loadMySQLStores();

    LogNotice("World : Loading loot data...");
    sLootMgr.initialize();
    sLootMgr.LoadLoot();

    loadMySQLTablesByTask();
    logEntitySize();

    sSpellMgr.loadSpellDataFromDatabase();
    sSpellMgr.calculateSpellCoefficients();

#if VERSION_STRING > TBC
    LogDetail("World : Starting Achievement System...");
    sObjectMgr.LoadAchievementCriteriaList();
#endif

    LogDetail("World : Starting Transport System...");
    sTransportHandler.loadTransportTemplates();
    sTransportHandler.spawnContinentTransports();

    LogDetail("World : Starting Mail System...");
    sMailSystem.StartMailSystem();

    LogDetail("World : Starting Auction System...");
    sAuctionMgr.initialize();
    sAuctionMgr.LoadAuctionHouses();

    LogDetail("World : Loading LFG rewards...");
    sLfgMgr.initialize();
    sLfgMgr.LoadRewards();

    sGuildMgr.loadGuildDataFromDB();

#if VERSION_STRING >= Cata
    sGuildMgr.loadGuildXpForLevelFromDB();
    sGuildMgr.loadGuildRewardsFromDB();

    sGuildFinderMgr.loadGuildFinderDataFromDB();
#endif

    mQueueUpdateTimer = settings.server.queueUpdateInterval;

    sChannelMgr.loadConfigSettings();

    LogDetail("World : Starting BattlegroundManager...");
    sBattlegroundManager.initialize();

    dw = std::move(std::make_unique<DayWatcherThread>());

    broadcastMgr = std::move(std::make_unique<BroadcastMgr>());

    ThreadPool.ExecuteTask(new CharacterLoaderThread());

    sEventMgr.AddEvent(this, &World::checkForExpiredInstances, EVENT_WORLD_UPDATEAUCTIONS, 120000, 0, 0);

    LogDetail("World: init in %u ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    return true;
}

void World::resetCharacterLoginBannState()
{
    CharacterDatabase.WaitExecute("UPDATE characters SET online = 0 WHERE online = 1");
    CharacterDatabase.WaitExecute("UPDATE characters SET banned= 0,banReason='' WHERE banned > 100 AND banned < %u", UNIXTIME);
}

bool World::loadDbcDb2Stores()
{
#if VERSION_STRING >= Cata
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

    sMySQLStore.loadTransportDataTable();
    sMySQLStore.loadTransportEntrys();
    sMySQLStore.loadGossipMenuItemsTable();
    sMySQLStore.loadRecallTable();
}

void World::loadMySQLTablesByTask()
{
    auto startTime = Util::TimeNow();

    sObjectMgr.initialize();
    sAddonMgr.initialize();
    sTicketMgr.initialize();
    sGameEventMgr.initialize();

#define MAKE_TASK(sp, ptr) tl.AddTask(new Task(new CallbackP0<sp>(&sp::getInstance(), &sp::ptr)))
#define MAKE_TASK2(sp, ptr, value) tl.AddTask(new Task(new CallbackP1<sp, uint8_t>(&sp::getInstance(), &sp::ptr, value)))
    // Fill the task list with jobs to do.
    TaskList tl;

    // spawn worker threads (2 * number of cpus)
    tl.spawn();

    // storage stuff has to be loaded first
    tl.wait();

    MAKE_TASK(ObjectMgr, GenerateLevelUpInfo);
    MAKE_TASK(ObjectMgr, LoadPlayersInfo);
    tl.wait();

    MAKE_TASK(MySQLDataStore, loadCreatureSpawns);
    MAKE_TASK(MySQLDataStore, loadGameobjectSpawns);

    MAKE_TASK(ObjectMgr, LoadInstanceEncounters);

    MAKE_TASK(ObjectMgr, LoadCreatureWaypoints);
    MAKE_TASK(ObjectMgr, LoadCreatureTimedEmotes);
    MAKE_TASK(ObjectMgr, LoadTrainers);
    MAKE_TASK(ObjectMgr, LoadSpellSkills);
    MAKE_TASK(ObjectMgr, LoadVendors);
    MAKE_TASK(ObjectMgr, LoadSpellTargetConstraints);
#if VERSION_STRING >= Cata
    MAKE_TASK(ObjectMgr, LoadSpellRequired);
    MAKE_TASK(ObjectMgr, LoadSkillLineAbilityMap);
#endif
    MAKE_TASK(ObjectMgr, LoadPetSpellCooldowns);
    MAKE_TASK(ObjectMgr, LoadGuildCharters);
    MAKE_TASK(TicketMgr, loadGMTickets);
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

    MAKE_TASK2(LootMgr, loadAndGenerateLoot, 0);
    MAKE_TASK2(LootMgr, loadAndGenerateLoot, 1);
    MAKE_TASK2(LootMgr, loadAndGenerateLoot, 2);
    MAKE_TASK2(LootMgr, loadAndGenerateLoot, 3);
    MAKE_TASK2(LootMgr, loadAndGenerateLoot, 4);
    MAKE_TASK2(LootMgr, loadAndGenerateLoot, 5);

    MAKE_TASK(QuestMgr, LoadExtraQuestStuff);
    MAKE_TASK(ObjectMgr, LoadEventScripts);
    MAKE_TASK(WeatherMgr, LoadFromDB);
    MAKE_TASK(AddonMgr, LoadFromDB);
    MAKE_TASK(GameEventMgr, LoadFromDB);
    MAKE_TASK(CalendarMgr, LoadFromDB);

#undef MAKE_TASK
#undef MAKE_TASK2

    // wait for tasks above
    tl.wait();

    sCommandTableStorage.Load();
    LogNotice("WordFilter : Loading...");

    g_chatFilter = new WordFilter();

    LogDetail("Done. Database loaded in %u ms.", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    // calling this puts all maps into our task list.
    sInstanceMgr.Load();

    // wait for the events to complete.
    tl.wait();

    // wait for them to exit, now.
    tl.kill();
    tl.waitForThreadsToExit();
}

void World::logEntitySize()
{
    LogNotice("World : Object size: %lu bytes", sizeof(Object));
    LogNotice("World : Unit size: %lu bytes", sizeof(Unit) + sizeof(AIInterface));
    LogNotice("World : Creature size: %lu bytes", sizeof(Creature) + sizeof(AIInterface));
    LogNotice("World : Player size: %lu bytes", sizeof(Player) + sizeof(ItemInterface) + 50000 + 30000 + 1000 + sizeof(AIInterface));
    LogNotice("World : GameObject size: %lu bytes", sizeof(GameObject));
}

void World::Update(unsigned long timePassed)
{
    sLfgMgr.Update(static_cast<uint32_t>(timePassed));
    mEventableObjectHolder->Update(static_cast<uint32_t>(timePassed));
    sAuctionMgr.Update();
    updateQueuedSessions(static_cast<uint32_t>(timePassed));

    sGuildMgr.update(static_cast<uint32>(timePassed));
}

void World::saveAllPlayersToDb()
{
    LogDefault("Saving all players to database...");

    uint32_t count = 0;

    sObjectMgr._playerslock.lock();

    for (PlayerStorageMap::const_iterator itr = sObjectMgr._players.begin(); itr != sObjectMgr._players.end(); ++itr)
    {
        if (itr->second->GetSession())
        {
            auto startTime = Util::TimeNow();
            itr->second->SaveToDB(false);
            LogDetail("Saved player `%s` (level %u) in %u ms.", itr->second->getName().c_str(), itr->second->getLevel(), static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
            ++count;
        }
    }

    sObjectMgr._playerslock.unlock();
    LogDetail("Saved %u players.", count);
}

void World::playSoundToAllPlayers(uint32_t soundId)
{
    std::lock_guard<std::mutex> guard(mSessionLock);

    for (activeSessionMap::iterator itr = mActiveSessionMapStore.begin(); itr != mActiveSessionMapStore.end(); ++itr)
    {
        WorldSession* worldSession = itr->second;
        if ((worldSession->GetPlayer() != nullptr) && worldSession->GetPlayer()->IsInWorld())
            worldSession->SendPacket(AscEmu::Packets::SmsgPlaySound(soundId).serialise().get());
    }
}

void World::logoutAllPlayers()
{
    LogNotice("World : Logging out players...");
    for (activeSessionMap::iterator i = mActiveSessionMapStore.begin(); i != mActiveSessionMapStore.end(); ++i)
        (i->second)->LogoutPlayer(true);

    LogNotice("World : Deleting sessions...");
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
