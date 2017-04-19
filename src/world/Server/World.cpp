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
 *
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
#include "Management/LocalizationMgr.h"
#include "Server/MainServerDefines.h"
#include "Config/Config.h"
#include "Map/MapCell.h"
#include "Spell/SpellMgr.h"
#include "Map/WorldCreator.h"
#include "Storage/DayWatcherThread.h"
#include "CommonScheduleThread.h"

initialiseSingleton(World);

DayWatcherThread* dw = nullptr;
CommonScheduleThread* cs = nullptr;

World::World()
{
    resetPlayerCount();
    m_gmTicketSystem = true;
    
    eventholder = new EventableObjectHolder(WORLD_INSTANCE);
    m_holder = eventholder;
    m_event_Instanceid = eventholder->GetInstanceID();

    PeakSessionCount = 0;
    mAcceptedConnections = 0;
    
    m_banTable = NULL;

    m_queueUpdateTimer = 5000;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Uptime
    mStartTime = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Player statistic
    mHordePlayersCount = 0;
    mAlliancePlayersCount = 0;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Traffic InfoCore
    mTotalTrafficInKB = 0.0;
    mTotalTrafficOutKB = 0.0;
    mLastTotalTrafficInKB = 0.0;
    mLastTotalTrafficOutKB = 0.0;
    mLastTrafficQuery = 0;
}

World::~World()
{
    LogNotice("LocalizationMgr : ~LocalizationMgr()");
    sLocalizationMgr.Shutdown();

    LogNotice("WorldLog : ~WorldLog()");
    delete WorldLog::getSingletonPtr();

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

    LogNotice("BattlegroundMgr : ~BattlegroundMgr()");
    delete CBattlegroundManager::getSingletonPtr();

    LogNotice("InstanceMgr : ~InstanceMgr()");
    sInstanceMgr.Shutdown();

    //LogDefault("Deleting Thread Manager..");
    //delete ThreadMgr::getSingletonPtr();
    LogNotice("WordFilter : ~WordFilter()");
    delete g_chatFilter;
    delete g_characterNameFilter;

    LogNotice("Rnd : ~Rnd()");
    CleanupRandomNumberGenerators();

    for (AreaTriggerMap::iterator i = m_AreaTrigger.begin(); i != m_AreaTrigger.end(); ++i)
    {
        delete i->second;
    }

    LogNotice("SpellProcMgr : ~SpellProcMgr()");
    delete SpellProcMgr::getSingletonPtr();

    LogNotice("SpellFactoryMgr : ~SpellFactoryMgr()");
    delete SpellFactoryMgr::getSingletonPtr();

    //eventholder = 0;
    delete eventholder;

    for (std::list<SpellInfo*>::iterator itr = dummyspells.begin(); itr != dummyspells.end(); ++itr)
        delete *itr;
}

//////////////////////////////////////////////////////////////////////////////////////////
// WorldConfig
void World::loadWorldConfigValues(bool reload /*false*/)
{
    settings.loadWorldConfigValues(reload);

    if (m_banTable != NULL)
        free(m_banTable);

    m_banTable = NULL;
    std::string s = settings.server.banTable;
    if (!s.empty())
        m_banTable = strdup(s.c_str());

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
    char str[300];
    time_t pTime = (time_t)UNIXTIME - mStartTime;
    tm* tmv = gmtime(&pTime);

    snprintf(str, 300, "%u days, %u hours, %u minutes, %u seconds.", tmv->tm_yday, tmv->tm_hour, tmv->tm_min, tmv->tm_sec);
    return std::string(str);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Traffic InfoCore
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

    for (std::unordered_map<uint32, Player*>::const_iterator itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        WorldSocket* socket = itr->second->GetSession()->GetSocket();
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

    if (mActiveSessionMapStore.size() > PeakSessionCount)
        PeakSessionCount = (uint32)mActiveSessionMapStore.size();

    worldSession->SendAccountDataTimes(GLOBAL_CACHE_MASK);

    mSessionLock.ReleaseWriteLock();
}

WorldSession* World::getSessionByAccountId(uint32_t accountId)
{
    mSessionLock.AcquireReadLock();

    WorldSession* worldSession = nullptr;

    activeSessionMap::const_iterator itr = mActiveSessionMapStore.find(accountId);
    if (itr != mActiveSessionMapStore.end())
        worldSession = itr->second;

    mSessionLock.ReleaseReadLock();

    return worldSession;
}

WorldSession* World::getSessionByAccountName(const char* Name)
{
    mSessionLock.AcquireReadLock();

    WorldSession* worldSession = nullptr;

    activeSessionMap::iterator itr = mActiveSessionMapStore.begin();
    for (; itr != mActiveSessionMapStore.end(); ++itr)
    {
        if (!stricmp(itr->second->GetAccountName().c_str(), Name))
        {
            worldSession = itr->second;
            break;
        }
    }

    mSessionLock.ReleaseReadLock();

    return worldSession;
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

    for (std::list<WorldSession*>::iterator itr = slist.begin(); itr != slist.end(); ++itr)
    {
        WorldSession* session = *itr;
        mActiveSessionMapStore.erase(session->GetAccountId());
    }

    mSessionLock.ReleaseWriteLock();

    for (std::list<WorldSession*>::iterator itr = slist.begin(); itr != slist.end(); ++itr)
    {
        WorldSession* session = *itr;
        delete session;
    }
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
    SessionSet::iterator itr, it2;
    WorldSession* session;
    int result;

    std::list<WorldSession*> ErasableSessions;

    globalSessionMutex.Acquire();

    for (itr = globalSessionSet.begin(); itr != globalSessionSet.end();)
    {
        session = (*itr);
        it2 = itr;
        ++itr;
        if (!session || session->GetInstance() != 0)
        {
            globalSessionSet.erase(it2);
            continue;
        }

        if ((result = session->Update(0)) != 0)
        {
            if (result == 1)
            {
                // complete deletion after relinquishing SessionMutex!
                // Otherwise Valgrind (probably falsely) reports a possible deadlock!
                ErasableSessions.push_back(session);
            }
            globalSessionSet.erase(it2);
        }
    }

    globalSessionMutex.Release();

    deleteSessions(ErasableSessions);
    ErasableSessions.clear();
}












void CleanupRandomNumberGenerators();

void World::LogoutPlayers()
{
    LogNotice("World : Logging out players...");
    for (activeSessionMap::iterator i = mActiveSessionMapStore.begin(); i != mActiveSessionMapStore.end(); ++i)
    {
        (i->second)->LogoutPlayer(true);
    }

    LogNotice("World", "Deleting sessions...");
    WorldSession* p;
    for (activeSessionMap::iterator i = mActiveSessionMapStore.begin(); i != mActiveSessionMapStore.end();)
    {
        p = i->second;
        ++i;

        deleteSession(p);
        //delete p;
    }
}

bool BasicTaskExecutor::run()
{
    /* Set thread priority, this is a bitch for multiplatform :P */
#ifdef WIN32
    switch (priority)
    {
    case BTE_PRIORITY_LOW:
        ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_LOWEST);
        break;

    case BTW_PRIORITY_HIGH:
        ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
        break;

    default: // BTW_PRIORITY_MED
        ::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_NORMAL);
        break;
    }
#else
    struct sched_param param;
    switch(priority)
    {
        case BTE_PRIORITY_LOW:
            param.sched_priority = 0;
            break;

        case BTW_PRIORITY_HIGH:
            param.sched_priority = 10;
            break;

        default:        // BTW_PRIORITY_MED
            param.sched_priority = 5;
            break;
    }
    pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
#endif

    // Execute the task in our new context.
    cb->execute();
#ifdef WIN32
    ::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#else
    param.sched_priority = 5;
    pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
#endif

    return true;
}

void ApplyNormalFixes();
extern void LoadGameObjectModelList(std::string const& dataPath);

bool World::SetInitialWorldSettings()
{
    Player::InitVisibleUpdateBits();

    CharacterDatabase.WaitExecute("UPDATE characters SET online = 0 WHERE online = 1");
    CharacterDatabase.WaitExecute("UPDATE characters SET banned= 0,banReason='' WHERE banned > 100 AND banned < %u", UNIXTIME);

    // Start
    uint32 start_time = getMSTime();

#if VERSION_STRING == Cata
    LoadDB2Stores();
#endif

    LogNotice("World : Loading DBC files...");
    if (!LoadDBCs())
    {
        AscLog.ConsoleLogMajorError("One or more of the DBC files are missing.", "These are absolutely necessary for the server to function.", "The server will not start without them.", "");
        return false;
    }

    new ObjectMgr;
    new QuestMgr;
    new LootMgr;
    new LfgMgr;
    new WeatherMgr;
    new TaxiMgr;
    new AddonMgr;
    new GameEventMgr;
    new CalendarMgr;
    new WorldLog;
    new ChatHandler;
    new SpellCustomizations;
    new SpellProcMgr;

    sSpellCustomizations.StartSpellCustomization();

    ApplyNormalFixes();

    LogNotice("GameObjectModel : Loading GameObject models...");
    LoadGameObjectModelList(worldConfig.terrainCollision.vMapPath);

    new SpellFactoryMgr;

    //Load tables
    new MySQLDataStore;

    sMySQLStore.LoadAdditionalTableConfig();

    sMySQLStore.LoadItemPagesTable();
    sMySQLStore.LoadItemPropertiesTable();
    sMySQLStore.LoadCreaturePropertiesTable();
    sMySQLStore.LoadGameObjectPropertiesTable();
    sMySQLStore.LoadQuestPropertiesTable();
    sMySQLStore.LoadGameObjectQuestItemBindingTable();
    sMySQLStore.LoadGameObjectQuestPickupBindingTable();

    sMySQLStore.LoadCreatureDifficultyTable();
    sMySQLStore.LoadDisplayBoundingBoxesTable();
    sMySQLStore.LoadVendorRestrictionsTable();
    sMySQLStore.LoadAreaTriggersTable();
    sMySQLStore.LoadNpcTextTable();
    sMySQLStore.LoadNpcScriptTextTable();
    sMySQLStore.LoadGossipMenuOptionTable();
    sMySQLStore.LoadGraveyardsTable();
    sMySQLStore.LoadTeleportCoordsTable();
    sMySQLStore.LoadFishingTable();
    sMySQLStore.LoadWorldMapInfoTable();
    sMySQLStore.LoadZoneGuardsTable();
    sMySQLStore.LoadBattleMastersTable();
    sMySQLStore.LoadTotemDisplayIdsTable();
    sMySQLStore.LoadSpellClickSpellsTable();

    sMySQLStore.LoadWorldStringsTable();
    sMySQLStore.LoadWorldBroadcastTable();
    sMySQLStore.LoadPointOfInterestTable();
    sMySQLStore.LoadItemSetLinkedSetBonusTable();
    sMySQLStore.LoadCreatureInitialEquipmentTable();

    sMySQLStore.LoadPlayerCreateInfoTable();
    sMySQLStore.LoadPlayerCreateInfoSkillsTable();
    sMySQLStore.LoadPlayerCreateInfoSpellsTable();
    sMySQLStore.LoadPlayerCreateInfoItemsTable();
    sMySQLStore.LoadPlayerXpToLevelTable();

    sMySQLStore.LoadSpellOverrideTable();

    sMySQLStore.LoadNpcGossipTextIdTable();
    sMySQLStore.LoadPetLevelAbilitiesTable();


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
    MAKE_TASK(ObjectMgr, LoadDefaultPetSpells);
    MAKE_TASK(ObjectMgr, LoadPetSpellCooldowns);
    MAKE_TASK(ObjectMgr, LoadGuildCharters);
    MAKE_TASK(ObjectMgr, LoadGMTickets);
    MAKE_TASK(ObjectMgr, SetHighestGuids);
    MAKE_TASK(ObjectMgr, LoadReputationModifiers);
    MAKE_TASK(ObjectMgr, LoadMonsterSay);
    MAKE_TASK(ObjectMgr, LoadGroups);
    MAKE_TASK(ObjectMgr, LoadCreatureAIAgents);
    MAKE_TASK(ObjectMgr, LoadArenaTeams);
    MAKE_TASK(ObjectMgr, LoadProfessionDiscoveries);
    MAKE_TASK(ObjectMgr, StoreBroadCastGroupKey);
    MAKE_TASK(ObjectMgr, LoadVehicleAccessories);
    MAKE_TASK(ObjectMgr, LoadWorldStateTemplates);
    MAKE_TASK(ObjectMgr, LoadAreaTrigger);

#if VERSION_STRING > TBC
    MAKE_TASK(ObjectMgr, LoadAchievementRewards);
#endif
    //LoadMonsterSay() must have finished before calling LoadExtraCreatureProtoStuff()
    tl.wait();

    MAKE_TASK(QuestMgr, LoadExtraQuestStuff);
    MAKE_TASK(SpellFactoryMgr, LoadSpellAreas);
    MAKE_TASK(ObjectMgr, LoadEventScripts);
    MAKE_TASK(WeatherMgr, LoadFromDB);
    MAKE_TASK(AddonMgr, LoadFromDB);
    MAKE_TASK(GameEventMgr, LoadFromDB);
    MAKE_TASK(CalendarMgr, LoadFromDB);

#undef MAKE_TASK

    // wait for all loading to complete.
    tl.wait();

    sLocalizationMgr.Reload(false);

    CommandTableStorage::getSingleton().Load();
    LogNotice("WordFilter : Loading...");

    g_characterNameFilter = new WordFilter();
    g_chatFilter = new WordFilter();
    g_characterNameFilter->Load("wordfilter_character_names");
    g_chatFilter->Load("wordfilter_chat");

    LogDetail("WordFilter : Done. Database loaded in %ums.", getMSTime() - start_time);

    // calling this puts all maps into our task list.
    sInstanceMgr.Load(&tl);

    // wait for the events to complete.
    tl.wait();

    // wait for them to exit, now.
    tl.kill();
    tl.waitForThreadsToExit();

    LogNotice("World : Object size: %u bytes", sizeof(Object));
    LogNotice("World : Unit size: %u bytes", sizeof(Unit) + sizeof(AIInterface));
    LogNotice("World : Creature size: %u bytes", sizeof(Creature) + sizeof(AIInterface));
    LogNotice("World : Player size: %u bytes", sizeof(Player) + sizeof(ItemInterface) + 50000 + 30000 + 1000 + sizeof(AIInterface));
    LogNotice("World : GameObject size: %u bytes", sizeof(GameObject));

    LogDetail("World : Starting Transport System...");
    objmgr.LoadTransports();

    //Start the Achievement system :D
    LogDetail("World : Starting Achievement System..");
#if VERSION_STRING > TBC
    objmgr.LoadAchievementCriteriaList();
#endif
    
    LogDetail("World : Starting Mail System...");
    new MailSystem;
    sMailSystem.StartMailSystem();

    LogDetail("World : Starting Auction System...");
    new AuctionMgr;
    sAuctionMgr.LoadAuctionHouses();

    LogDetail("World : Loading LFG rewards...");
    sLfgMgr.LoadRewards();

    m_queueUpdateTimer = settings.server.queueUpdateInterval;
    LogNotice("World : Loading loot data...");
    lootmgr.LoadLoot();

    Channel::LoadConfSettings();
    LogDetail("World : Starting CBattlegroundManager...");
    new CBattlegroundManager;

    dw = new DayWatcherThread();
    ThreadPool.ExecuteTask(dw);

    // commonschedule sys
    cs = new CommonScheduleThread();
    ThreadPool.ExecuteTask(cs);

    ThreadPool.ExecuteTask(new CharacterLoaderThread());

    sEventMgr.AddEvent(this, &World::CheckForExpiredInstances, EVENT_WORLD_UPDATEAUCTIONS, 120000, 0, 0);
    return true;
}


void World::Update(unsigned long time_passed)
{
    sLfgMgr.Update((uint32)time_passed);
    eventholder->Update((uint32)time_passed);
    sAuctionMgr.Update();
    UpdateQueuedSessions((uint32)time_passed);
#ifdef SESSION_CAP
    if (GetSessionCount() >= SESSION_CAP)
        TerminateProcess(GetCurrentProcess(), 0);
#endif
}

void World::SendGlobalMessage(WorldPacket* packet, WorldSession* self)
{
    mSessionLock.AcquireReadLock();

    activeSessionMap::iterator itr;
    for (itr = mActiveSessionMapStore.begin(); itr != mActiveSessionMapStore.end(); ++itr)
    {
        if (itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld() && itr->second != self) // don't send to self!
        {
            itr->second->SendPacket(packet);
        }
    }

    mSessionLock.ReleaseReadLock();
}

void World::PlaySoundToAll(uint32 soundid)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << uint32(soundid);

    mSessionLock.AcquireWriteLock();

    for (activeSessionMap::iterator itr = mActiveSessionMapStore.begin(); itr != mActiveSessionMapStore.end(); ++itr)
    {
        WorldSession* session = itr->second;

        if ((session->GetPlayer() != NULL) && session->GetPlayer()->IsInWorld())
            session->SendPacket(&data);
    }

    mSessionLock.ReleaseWriteLock();
}

void World::SendFactionMessage(WorldPacket* packet, uint8 teamId)
{
    mSessionLock.AcquireReadLock();
    activeSessionMap::iterator itr;
    Player* plr;
    for (itr = mActiveSessionMapStore.begin(); itr != mActiveSessionMapStore.end(); ++itr)
    {
        plr = itr->second->GetPlayer();
        if (!plr || !plr->IsInWorld())
            continue;

        if (plr->GetTeam() == teamId)
            itr->second->SendPacket(packet);
    }
    mSessionLock.ReleaseReadLock();
}

void World::SendGamemasterMessage(WorldPacket* packet, WorldSession* self)
{
    mSessionLock.AcquireReadLock();
    activeSessionMap::iterator itr;
    for (itr = mActiveSessionMapStore.begin(); itr != mActiveSessionMapStore.end(); ++itr)
    {
        if (itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld() && itr->second != self) // don't send to self!
        {
            if (itr->second->CanUseCommand('u'))
                itr->second->SendPacket(packet);
        }
    }
    mSessionLock.ReleaseReadLock();
}

void World::SendZoneMessage(WorldPacket* packet, uint32 zoneid, WorldSession* self)
{
    mSessionLock.AcquireReadLock();

    activeSessionMap::iterator itr;
    for (itr = mActiveSessionMapStore.begin(); itr != mActiveSessionMapStore.end(); ++itr)
    {
        if (itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld() && itr->second != self) // don't send to self!
        {
            if (itr->second->GetPlayer()->GetZoneId() == zoneid)
                itr->second->SendPacket(packet);
        }
    }

    mSessionLock.ReleaseReadLock();
}

void World::SendInstanceMessage(WorldPacket* packet, uint32 instanceid, WorldSession* self)
{
    mSessionLock.AcquireReadLock();

    activeSessionMap::iterator itr;
    for (itr = mActiveSessionMapStore.begin(); itr != mActiveSessionMapStore.end(); ++itr)
    {
        if (itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld() && itr->second != self) // don't send to self!
        {
            if (itr->second->GetPlayer()->GetInstanceID() == (int32)instanceid)
                itr->second->SendPacket(packet);
        }
    }

    mSessionLock.ReleaseReadLock();
}

void World::SendWorldText(const char* text, WorldSession* self)
{
    uint32 textLen = (uint32)strlen((char*)text) + 1;

    WorldPacket data(textLen + 40);
    data.Initialize(SMSG_MESSAGECHAT);
    data << uint8(CHAT_MSG_SYSTEM);
    data << uint32(LANG_UNIVERSAL);
    data << uint64(0); // Who cares about guid when there's no nickname displayed heh ?
    data << uint32(0);
    data << uint64(0);
    data << textLen;
    data << text;
    data << uint8(0);
    SendGlobalMessage(&data, self);

    if (settings.announce.showAnnounceInConsoleOutput)
    {
        LogDetail("WORLD : SendWorldText %s", text);
    }
}

void World::SendGMWorldText(const char* text, WorldSession* self)
{
    uint32 textLen = (uint32)strlen((char*)text) + 1;

    WorldPacket data(textLen + 40);
    data.Initialize(SMSG_MESSAGECHAT);
    data << uint8(CHAT_MSG_SYSTEM);
    data << uint32(LANG_UNIVERSAL);
    data << uint64(0);
    data << uint32(0);
    data << uint64(0);
    data << textLen;
    data << text;
    data << uint8(0);
    SendGamemasterMessage(&data, self);
}

void World::SendDamageLimitTextToGM(const char* playername, const char* dmglog)
{
    std::string gm_ann(MSG_COLOR_GREEN);

    gm_ann += "|Hplayer:";
    gm_ann += playername;
    gm_ann += "|h[";
    gm_ann += playername;
    gm_ann += "]|h: ";
    gm_ann += MSG_COLOR_YELLOW;
    gm_ann += dmglog;

    sWorld.SendGMWorldText(gm_ann.c_str());
}

void World::SendWorldWideScreenText(const char* text, WorldSession* self)
{
    WorldPacket data(256);
    data.Initialize(SMSG_AREA_TRIGGER_MESSAGE);
    data << uint32(0);
    data << text;
    data << uint8(0x00);
    SendGlobalMessage(&data, self);
}

uint32 World::AddQueuedSocket(WorldSocket* Socket)
{
    queueMutex.Acquire();
    mQueuedSessions.push_back(Socket);
    queueMutex.Release();

    return (uint32)mQueuedSessions.size();
}

void World::RemoveQueuedSocket(WorldSocket* Socket)
{
    queueMutex.Acquire();

    // Find socket in list
    QueueSet::iterator iter = mQueuedSessions.begin();
    for (; iter != mQueuedSessions.end(); ++iter)
    {
        if ((*iter) == Socket)
        {
            // Remove from the queue and abort.
            // This will be slow (Removing from middle of a vector!) but it won't
            // get called very much, so it's not really a big deal.

            mQueuedSessions.erase(iter);
            queueMutex.Release();
            return;
        }
    }
    queueMutex.Release();
}

void World::UpdateQueuedSessions(uint32 diff)
{
    if (diff >= m_queueUpdateTimer)
    {
        m_queueUpdateTimer = settings.server.queueUpdateInterval;
        queueMutex.Acquire();

        if (mQueuedSessions.size() == 0)
        {
            queueMutex.Release();
            return;
        }

        while (mActiveSessionMapStore.size() < settings.getPlayerLimit() && mQueuedSessions.size())
        {
            // Yay. We can let another player in now.
            // Grab the first fucker from the queue, but guard of course, since
            // this is in a different thread again.

            QueueSet::iterator iter = mQueuedSessions.begin();
            WorldSocket* QueuedSocket = *iter;
            mQueuedSessions.erase(iter);

            // Welcome, sucker.
            if (QueuedSocket->GetSession())
            {
                QueuedSocket->GetSession()->deleteMutex.Acquire();
                QueuedSocket->Authenticate();
                QueuedSocket->GetSession()->deleteMutex.Release();
            }
        }

        if (mQueuedSessions.size() == 0)
        {
            queueMutex.Release();
            return;
        }

        // Update the remaining queue members.
        QueueSet::iterator iter = mQueuedSessions.begin();
        uint32 Position = 1;
        while (iter != mQueuedSessions.end())
        {
            (*iter)->UpdateQueuePosition(Position++);
            if (iter == mQueuedSessions.end())
                break;
            else
                ++iter;
        }
        queueMutex.Release();
    }
    else
    {
        m_queueUpdateTimer -= diff;
    }
}

void World::SaveAllPlayers()
{
    if (!(ObjectMgr::getSingletonPtr()))
        return;

    LogDefault("Saving all players to database...");
    uint32 count = 0;
    uint32 save_start_time;

    objmgr._playerslock.AcquireReadLock();

    PlayerStorageMap::const_iterator itr;
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        if (itr->second->GetSession())
        {
            save_start_time = getMSTime();
            itr->second->SaveToDB(false);
            LogDetail("Saved player `%s` (level %u) in %ums.", itr->second->GetName(), itr->second->getLevel(), getMSTime() - save_start_time);
            ++count;
        }
    }
    objmgr._playerslock.ReleaseReadLock();
    LogDetail("Saved %u players.", count);
}

void TaskList::AddTask(Task* task)
{
    queueLock.Acquire();
    tasks.insert(task);
    queueLock.Release();
}

Task* TaskList::GetTask()
{
    queueLock.Acquire();

    Task* t = 0;

    for (std::set<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); ++itr)
    {
        if (!(*itr)->in_progress)
        {
            t = (*itr);
            t->in_progress = true;
            break;
        }
    }

    queueLock.Release();

    return t;
}

void TaskList::spawn()
{
    running = true;
    thread_count.SetVal(0);

    uint32 threadcount;
    if (worldConfig.startup.enableMultithreadedLoading)
    {
        // get processor count
#ifndef WIN32
#if UNIX_FLAVOUR == UNIX_FLAVOUR_LINUX
#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__)
        threadcount = 2;
#else
        long affmask;
        sched_getaffinity(0, 4, (cpu_set_t*)&affmask);
        threadcount = (BitCount8(affmask)) * 2;
        if (threadcount > 8) threadcount = 8;
        else if (threadcount <= 0) threadcount = 1;
#endif
#else
        threadcount = 2;
#endif
#else
        SYSTEM_INFO s;
        GetSystemInfo(&s);
        threadcount = s.dwNumberOfProcessors * 2;
        if (threadcount > 8)
            threadcount = 8;
#endif
    }
    else
        threadcount = 1;

    LogNotice("World : Beginning %s server startup with %u threads.", (threadcount == 1) ? "progressive" : "parallel", threadcount);

    for (uint32 x = 0; x < threadcount; ++x)
        ThreadPool.ExecuteTask(new TaskExecutor(this));
}

void TaskList::wait()
{
    bool has_tasks = true;
    while (has_tasks)
    {
        queueLock.Acquire();
        has_tasks = false;
        for (std::set<Task*>::iterator itr = tasks.begin(); itr != tasks.end(); ++itr)
        {
            if (!(*itr)->completed)
            {
                has_tasks = true;
                break;
            }
        }
        queueLock.Release();
        Arcemu::Sleep(20);
    }
}

void TaskList::kill()
{
    running = false;
}

void Task::execute()
{
    _cb->execute();
}

bool TaskExecutor::run()
{
    Task* t;

    THREAD_TRY_EXECUTION
        while (starter->running)
        {
            t = starter->GetTask();
            if (t)
            {
                t->execute();
                t->completed = true;
                starter->RemoveTask(t);
                delete t;
            }
            else
                Arcemu::Sleep(20);
        }

        THREAD_HANDLE_CRASH

    return true;
}

void TaskList::waitForThreadsToExit()
{
    while (thread_count.GetVal())
    {
        Arcemu::Sleep(20);
    }
}

void World::DeleteObject(Object* obj)
{
    delete obj;
}

void World::CharacterEnumProc(QueryResultVector& results, uint32 AccountId)
{
    WorldSession* s = getSessionByAccountId(AccountId);
    if (s == NULL)
        return;

    s->CharacterEnumProc(results[0].result);
}

void World::LoadAccountDataProc(QueryResultVector& results, uint32 AccountId)
{
    WorldSession* session = getSessionByAccountId(AccountId);
    if (session == NULL)
        return;

    session->LoadAccountDataProc(results[0].result);
}

void World::CheckForExpiredInstances()
{
    sInstanceMgr.CheckForExpiredInstances();
}

struct insert_playeritem
{
    uint32 ownerguid;
    uint32 entry;
    uint32 wrapped_item_id;
    uint32 wrapped_creator;
    uint32 creator;
    uint32 count;
    uint32 charges;
    uint32 flags;
    uint32 randomprop;
    uint32 randomsuffix;
    uint32 itemtext;
    uint32 durability;
    int32 containerslot;
    int32 slot;
    std::string enchantments;
};

#define LOAD_THREAD_SLEEP 180

void CharacterLoaderThread::OnShutdown()
{
    running = false;
    cond.Signal();
}

CharacterLoaderThread::CharacterLoaderThread()
{
    running = false;
}

CharacterLoaderThread::~CharacterLoaderThread()
{
}

bool CharacterLoaderThread::run()
{
    running = true;
    for (;;)
    {
        // Get a single connection to maintain for the whole process.
        DatabaseConnection* con = CharacterDatabase.GetFreeConnection();

        con->Busy.Release();

        cond.Wait(LOAD_THREAD_SLEEP * 1000);

        if (!running)
            break;
    }

    return true;
}

void World::DisconnectUsersWithAccount(const char* account, WorldSession* m_session)
{
    activeSessionMap::iterator itr;
    WorldSession* session;
    mSessionLock.AcquireReadLock();
    bool FoundUser = false;
    for (itr = mActiveSessionMapStore.begin(); itr != mActiveSessionMapStore.end();)
    {
        session = itr->second;
        ++itr;

        if (!stricmp(account, session->GetAccountNameS()))
        {
            FoundUser = true;
            m_session->SystemMessage("Disconnecting user with account `%s` IP `%s` Player `%s`.", session->GetAccountNameS(),
                                     session->GetSocket() ? session->GetSocket()->GetRemoteIP().c_str() : "noip", session->GetPlayer() ? session->GetPlayer()->GetName() : "noplayer");

            session->Disconnect();
        }
    }
    mSessionLock.ReleaseReadLock();
    if (FoundUser == false)
        m_session->SystemMessage("There is nobody online with account [%s]", account);
}

void World::DisconnectUsersWithIP(const char* ip, WorldSession* m_session)
{
    activeSessionMap::iterator itr;
    WorldSession* session;
    mSessionLock.AcquireReadLock();
    bool FoundUser = false;
    for (itr = mActiveSessionMapStore.begin(); itr != mActiveSessionMapStore.end();)
    {
        session = itr->second;
        ++itr;

        if (!session->GetSocket())
            continue;

        std::string ip2 = session->GetSocket()->GetRemoteIP().c_str();
        if (!stricmp(ip, ip2.c_str()))
        {
            FoundUser = true;
            m_session->SystemMessage("Disconnecting user with account `%s` IP `%s` Player `%s`.", session->GetAccountNameS(),
                                     ip2.c_str(), session->GetPlayer() ? session->GetPlayer()->GetName() : "noplayer");

            session->Disconnect();
        }
    }
    if (FoundUser == false)
        m_session->SystemMessage("There is nobody online with ip [%s]", ip);
    mSessionLock.ReleaseReadLock();
}

void World::DisconnectUsersWithPlayerName(const char* plr, WorldSession* m_session)
{
    activeSessionMap::iterator itr;
    WorldSession* session;
    mSessionLock.AcquireReadLock();
    bool FoundUser = false;
    for (itr = mActiveSessionMapStore.begin(); itr != mActiveSessionMapStore.end();)
    {
        session = itr->second;
        ++itr;

        if (!session->GetPlayer())
            continue;

        if (!stricmp(plr, session->GetPlayer()->GetName()))
        {
            FoundUser = true;
            m_session->SystemMessage("Disconnecting user with account `%s` IP `%s` Player `%s`.", session->GetAccountNameS(),
                                     session->GetSocket() ? session->GetSocket()->GetRemoteIP().c_str() : "noip", session->GetPlayer() ? session->GetPlayer()->GetName() : "noplayer");

            session->Disconnect();
        }
    }
    if (FoundUser == false)
        m_session->SystemMessage("There is no body online with the name [%s]", plr);
    mSessionLock.ReleaseReadLock();
}

void World::SendBCMessageByID(uint32 id)
{
    mSessionLock.AcquireReadLock();
    activeSessionMap::iterator itr;
    for (itr = mActiveSessionMapStore.begin(); itr != mActiveSessionMapStore.end(); ++itr)
    {
        if (itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld())
        {
            const char* text = itr->second->LocalizedBroadCast(id);
            uint32 textLen = (uint32)strlen(text) + 1;

            WorldPacket data(textLen + 40);
            data.Initialize(SMSG_MESSAGECHAT);
            data << uint8(CHAT_MSG_SYSTEM);
            data << uint32(LANG_UNIVERSAL);
            data << uint64(0); // Who cares about guid when there's no nickname displayed heh ?
            data << uint32(0);
            data << uint64(0);
            data << textLen;
            data << text;
            data << uint8(0);
            itr->second->SendPacket(&data);
        }
    }
    mSessionLock.ReleaseReadLock();
}

// cebernic:2008-10-19
// Format as SendLocalizedWorldText("forcing english & {5}{12} %s","test");
// 5,12 are ids from worldstring_table
//\todo Zyres: Not called! 2017-04-18
//void World::SendLocalizedWorldText(bool wide, const char* format, ...) // May not optimized,just for works.
//{
//    m_sessionlock.AcquireReadLock();
//    SessionMap::iterator itr;
//    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
//    {
//        if (itr->second->GetPlayer() &&
//            itr->second->GetPlayer()->IsInWorld())
//        {
//            std::string temp = SessionLocalizedTextFilter(itr->second, format);
//            // parsing
//            format = (char*)temp.c_str();
//            char buffer[1024];
//            va_list ap;
//            va_start(ap, format);
//            vsnprintf(buffer, 1024, format, ap);
//            va_end(ap);
//            // again,we need parse args
//            temp = SessionLocalizedTextFilter(itr->second, buffer);
//            memset(buffer, 0, temp.length() + 1);
//            memcpy(buffer, temp.c_str(), temp.length() + 1); // full done
//
//            uint32 textLen = (uint32)strlen(buffer) + 1;
//            WorldPacket data(textLen + 40);
//
//            if (wide)
//            {
//                data.Initialize(SMSG_AREA_TRIGGER_MESSAGE);
//                data << uint32(0);
//                data << (char*)buffer;
//                data << uint8(0);
//            }
//            else
//            {
//                data.Initialize(SMSG_MESSAGECHAT);
//                data << uint8(CHAT_MSG_SYSTEM);
//                data << uint32(LANG_UNIVERSAL);
//                data << uint64(0); // Who cares about guid when there's no nickname displayed heh ?
//                data << uint32(0);
//                data << uint64(0);
//                data << uint32(textLen);
//                data << buffer;
//                data << uint8(0);
//            }
//            itr->second->SendPacket(&data);
//        }
//    }
//    m_sessionlock.ReleaseReadLock();
//}

void World::SendZoneUnderAttackMsg(uint32 areaid, uint8 team)
{
    WorldPacket data(SMSG_ZONE_UNDER_ATTACK, 4);
    data << uint32(areaid);

    SendFactionMessage(&data, team);
}
