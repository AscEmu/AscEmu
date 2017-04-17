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
    m_allowMovement = true;
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

    AnnounceColorChooser(settings.color.tagColor, settings.color.tagGmColor, settings.color.nameColor, settings.color.msgColor);
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

void CleanupRandomNumberGenerators();

void World::LogoutPlayers()
{
    LogNotice("World : Logging out players...");
    for (SessionMap::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i)
    {
        (i->second)->LogoutPlayer(true);
    }

    LogNotice("World", "Deleting sessions...");
    WorldSession* p;
    for (SessionMap::iterator i = m_sessions.begin(); i != m_sessions.end();)
    {
        p = i->second;
        ++i;

        DeleteSession(p);
        //delete p;
    }
}

WorldSession* World::FindSession(uint32 id)
{
    m_sessionlock.AcquireReadLock();
    WorldSession* ret = 0;
    SessionMap::const_iterator itr = m_sessions.find(id);

    if (itr != m_sessions.end())
        ret = itr->second;

    m_sessionlock.ReleaseReadLock();

    return ret;
}

void World::RemoveSession(uint32 id)
{
    m_sessionlock.AcquireWriteLock();

    SessionMap::iterator itr = m_sessions.find(id);

    if (itr != m_sessions.end())
    {
        delete itr->second;
        m_sessions.erase(itr);
    }

    m_sessionlock.ReleaseWriteLock();
}

void World::AddSession(WorldSession* s)
{
    ARCEMU_ASSERT(s != NULL);

    m_sessionlock.AcquireWriteLock();

    m_sessions[s->GetAccountId()] = s;

    if (m_sessions.size() > PeakSessionCount)
        PeakSessionCount = (uint32)m_sessions.size();

    s->SendAccountDataTimes(GLOBAL_CACHE_MASK);

    m_sessionlock.ReleaseWriteLock();
}

void World::AddGlobalSession(WorldSession* session)
{
    ARCEMU_ASSERT(session != NULL);

    SessionsMutex.Acquire();
    Sessions.insert(session);
    SessionsMutex.Release();
}

void World::RemoveGlobalSession(WorldSession* session)
{
    ARCEMU_ASSERT(session != NULL);

    SessionsMutex.Acquire();
    Sessions.erase(session);
    SessionsMutex.Release();
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

    LoadInspectTalentTab();

    sEventMgr.AddEvent(this, &World::CheckForExpiredInstances, EVENT_WORLD_UPDATEAUCTIONS, 120000, 0, 0);
    return true;
}

void World::LoadInspectTalentTab()
{
    // Preload and compile talent and talent tab data to speed up talent inspect
    // Zyres: It looks like missplaced and should be moved right after dbc loading
#define MAX_TALENT_CLASS 12
    uint32 talent_max_rank;
    uint32 talent_pos;
    uint32 talent_class;

    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        auto talent_info = sTalentStore.LookupEntry(i);
        if (talent_info == nullptr)
            continue;

        // Don't add invalid talents or Hunter Pet talents (trees 409, 410 and 411) to the inspect table
        if (talent_info->TalentTree == 409 || talent_info->TalentTree == 410 || talent_info->TalentTree == 411)
            continue;

        auto talent_tab = sTalentTabStore.LookupEntry(talent_info->TalentTree);
        if (talent_tab == nullptr)
            continue;

        talent_max_rank = 0;
        for (uint32 j = 5; j > 0; --j)
        {
            if (talent_info->RankID[j - 1])
            {
                talent_max_rank = j;
                break;
            }
        }

        InspectTalentTabBit[(talent_info->Row << 24) + (talent_info->Col << 16) + talent_info->TalentID] = talent_max_rank;
        InspectTalentTabSize[talent_info->TalentTree] += talent_max_rank;
    }

    for (uint32 i = 0; i < sTalentTabStore.GetNumRows(); ++i)
    {
        auto talent_tab = sTalentTabStore.LookupEntry(i);
        if (talent_tab == nullptr)
            continue;

        // Don't add Hunter Pet TalentTabs (ClassMask == 0) to the InspectTalentTabPages
        if (talent_tab->ClassMask == 0)
            continue;

        talent_pos = 0;

        for (talent_class = 0; talent_class < MAX_TALENT_CLASS; ++talent_class)
        {
            if (talent_tab->ClassMask & (1 << talent_class))
                break;
        }
        // Zyres: prevent Out-of-bounds read
        if (talent_class > 0 && talent_class < MAX_TALENT_CLASS)
            InspectTalentTabPages[talent_class][talent_tab->TabPage] = talent_tab->TalentTabID;

        for (std::map<uint32, uint32>::iterator itr = InspectTalentTabBit.begin(); itr != InspectTalentTabBit.end(); ++itr)
        {
            uint32 talent_id = itr->first & 0xFFFF;
            auto talent_info = sTalentStore.LookupEntry(talent_id);
            if (talent_info == nullptr)
                continue;

            if (talent_info->TalentTree != talent_tab->TalentTabID)
                continue;

            InspectTalentTabPos[talent_id] = talent_pos;
            talent_pos += itr->second;
        }
    }
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
    m_sessionlock.AcquireReadLock();

    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld() && itr->second != self) // don't send to self!
        {
            itr->second->SendPacket(packet);
        }
    }

    m_sessionlock.ReleaseReadLock();
}

void World::PlaySoundToAll(uint32 soundid)
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << uint32(soundid);

    m_sessionlock.AcquireWriteLock();

    for (SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        WorldSession* session = itr->second;

        if ((session->GetPlayer() != NULL) && session->GetPlayer()->IsInWorld())
            session->SendPacket(&data);
    }

    m_sessionlock.ReleaseWriteLock();
}

void World::SendFactionMessage(WorldPacket* packet, uint8 teamId)
{
    m_sessionlock.AcquireReadLock();
    SessionMap::iterator itr;
    Player* plr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        plr = itr->second->GetPlayer();
        if (!plr || !plr->IsInWorld())
            continue;

        if (plr->GetTeam() == teamId)
            itr->second->SendPacket(packet);
    }
    m_sessionlock.ReleaseReadLock();
}

void World::SendGamemasterMessage(WorldPacket* packet, WorldSession* self)
{
    m_sessionlock.AcquireReadLock();
    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld() && itr->second != self) // don't send to self!
        {
            if (itr->second->CanUseCommand('u'))
                itr->second->SendPacket(packet);
        }
    }
    m_sessionlock.ReleaseReadLock();
}

void World::SendZoneMessage(WorldPacket* packet, uint32 zoneid, WorldSession* self)
{
    m_sessionlock.AcquireReadLock();

    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld() && itr->second != self) // don't send to self!
        {
            if (itr->second->GetPlayer()->GetZoneId() == zoneid)
                itr->second->SendPacket(packet);
        }
    }

    m_sessionlock.ReleaseReadLock();
}

void World::SendInstanceMessage(WorldPacket* packet, uint32 instanceid, WorldSession* self)
{
    m_sessionlock.AcquireReadLock();

    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld() && itr->second != self) // don't send to self!
        {
            if (itr->second->GetPlayer()->GetInstanceID() == (int32)instanceid)
                itr->second->SendPacket(packet);
        }
    }

    m_sessionlock.ReleaseReadLock();
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

void World::UpdateSessions(uint32 diff)
{
    SessionSet::iterator itr, it2;
    WorldSession* session;
    int result;

    std::list<WorldSession*> ErasableSessions;

    SessionsMutex.Acquire();

    for (itr = Sessions.begin(); itr != Sessions.end();)
    {
        session = (*itr);
        it2 = itr;
        ++itr;
        if (!session || session->GetInstance() != 0)
        {
            Sessions.erase(it2);
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
            Sessions.erase(it2);
        }
    }

    SessionsMutex.Release();

    DeleteSessions(ErasableSessions);
    ErasableSessions.clear();
}

void World::DeleteSession(WorldSession* session)
{
    m_sessionlock.AcquireWriteLock();
    m_sessions.erase(session->GetAccountId());
    m_sessionlock.ReleaseWriteLock();
    delete session;
}

void World::DeleteSessions(std::list<WorldSession*>& slist)
{
    m_sessionlock.AcquireWriteLock();

    for (std::list<WorldSession*>::iterator itr = slist.begin(); itr != slist.end(); ++itr)
    {
        WorldSession* session = *itr;
        m_sessions.erase(session->GetAccountId());
    }

    m_sessionlock.ReleaseWriteLock();

    for (std::list<WorldSession*>::iterator itr = slist.begin(); itr != slist.end(); ++itr)
    {
        WorldSession* session = *itr;
        delete session;
    }
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

uint32 World::GetQueuePos(WorldSocket* Socket)
{
    queueMutex.Acquire();

    // Find socket in list
    QueueSet::iterator iter = mQueuedSessions.begin();
    uint32 QueuePos = 1;
    for (; iter != mQueuedSessions.end(); ++iter , ++QueuePos)
    {
        if ((*iter) == Socket)
        {
            queueMutex.Release();
            // Return our queue position.
            return QueuePos;
        }
    }
    queueMutex.Release();
    // We shouldn't get here..
    return 1;
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

        while (m_sessions.size() < settings.getPlayerLimit() && mQueuedSessions.size())
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

WorldSession* World::FindSessionByName(const char* Name) //case insensitive
{
    m_sessionlock.AcquireReadLock();

    // loop sessions, see if we can find him
    SessionMap::iterator itr = m_sessions.begin();
    for (; itr != m_sessions.end(); ++itr)
    {
        if (!stricmp(itr->second->GetAccountName().c_str(), Name))
        {
            m_sessionlock.ReleaseReadLock();
            return itr->second;
        }
    }
    m_sessionlock.ReleaseReadLock();
    return 0;
}

void World::ShutdownClasses()
{
    LogNotice("AddonMgr : ~AddonMgr()");
    sAddonMgr.SaveToDB();
    delete AddonMgr::getSingletonPtr();

    LogNotice("AuctionMgr : ~AuctionMgr()");
    delete AuctionMgr::getSingletonPtr();
    LogNotice("LootMgr : ~LootMgr()");
    delete LootMgr::getSingletonPtr();

    LogNotice("MailSystem : ~MailSystem()");
    delete MailSystem::getSingletonPtr();
}

void World::GetStats(uint32* GMCount, float* AverageLatency)
{
    int gm = 0;
    int count = 0;
    int avg = 0;
    PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        if (itr->second->GetSession())
        {
            count++;
            avg += itr->second->GetSession()->GetLatency();
            if (itr->second->GetSession()->GetPermissionCount())
                gm++;
        }
    }
    objmgr._playerslock.ReleaseReadLock();

    *AverageLatency = count ? ((float)avg / (float)count) : 0;
    *GMCount = gm;
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
    WorldSession* s = FindSession(AccountId);
    if (s == NULL)
        return;

    s->CharacterEnumProc(results[0].result);
}

void World::AnnounceColorChooser(int tagcolor, int gmtagcolor, int namecolor, int msgcolor)
{
    switch (tagcolor)
    {
    case 1:
        ann_tagcolor = "|cffff6060"; //lightred
        break;
    case 2:
        ann_tagcolor = "|cff00ccff"; //lightblue
        break;
    case 3:
        ann_tagcolor = "|cff0000ff"; //blue
        break;
    case 4:
        ann_tagcolor = "|cff00ff00"; //green
        break;
    case 5:
        ann_tagcolor = "|cffff0000"; //red
        break;
    case 6:
        ann_tagcolor = "|cffffcc00"; //gold
        break;
    case 7:
        ann_tagcolor = "|cff888888"; //grey
        break;
    case 8:
        ann_tagcolor = "|cffffffff"; //white
        break;
    case 9:
        ann_tagcolor = "|cffff00ff"; //magenta
        break;
    case 10:
        ann_tagcolor = "|cffffff00"; //yellow
        break;
    }
    switch (gmtagcolor)
    {
    case 1:
        ann_gmtagcolor = "|cffff6060"; //lightred
        break;
    case 2:
        ann_gmtagcolor = "|cff00ccff"; //lightblue
        break;
    case 3:
        ann_gmtagcolor = "|cff0000ff"; //blue
        break;
    case 4:
        ann_gmtagcolor = "|cff00ff00"; //green
        break;
    case 5:
        ann_gmtagcolor = "|cffff0000"; //red
        break;
    case 6:
        ann_gmtagcolor = "|cffffcc00"; //gold
        break;
    case 7:
        ann_gmtagcolor = "|cff888888"; //grey
        break;
    case 8:
        ann_gmtagcolor = "|cffffffff"; //white
        break;
    case 9:
        ann_gmtagcolor = "|cffff00ff"; //magenta
        break;
    case 10:
        ann_gmtagcolor = "|cffffff00"; //yellow
        break;
    }
    switch (namecolor)
    {
    case 1:
        ann_namecolor = "|cffff6060"; //lightred
        break;
    case 2:
        ann_namecolor = "|cff00ccff"; //lightblue
        break;
    case 3:
        ann_namecolor = "|cff0000ff"; //blue
        break;
    case 4:
        ann_namecolor = "|cff00ff00"; //green
        break;
    case 5:
        ann_namecolor = "|cffff0000"; //red
        break;
    case 6:
        ann_namecolor = "|cffffcc00"; //gold
        break;
    case 7:
        ann_namecolor = "|cff888888"; //grey
        break;
    case 8:
        ann_namecolor = "|cffffffff"; //white
        break;
    case 9:
        ann_namecolor = "|cffff00ff"; //magenta
        break;
    case 10:
        ann_namecolor = "|cffffff00"; //yellow
        break;
    }
    switch (msgcolor)
    {
    case 1:
        ann_msgcolor = "|cffff6060"; //lightred
        break;
    case 2:
        ann_msgcolor = "|cff00ccff"; //lightblue
        break;
    case 3:
        ann_msgcolor = "|cff0000ff"; //blue
        break;
    case 4:
        ann_msgcolor = "|cff00ff00"; //green
        break;
    case 5:
        ann_msgcolor = "|cffff0000"; //red
        break;
    case 6:
        ann_msgcolor = "|cffffcc00"; //gold
        break;
    case 7:
        ann_msgcolor = "|cff888888"; //grey
        break;
    case 8:
        ann_msgcolor = "|cffffffff"; //white
        break;
    case 9:
        ann_msgcolor = "|cffff00ff"; //magenta
        break;
    case 10:
        ann_msgcolor = "|cffffff00"; //yellow
        break;
    }
    LOG_BASIC("Announce colors initialized.");
}

void World::LoadAccountDataProc(QueryResultVector& results, uint32 AccountId)
{
    WorldSession* session = FindSession(AccountId);
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
    SessionMap::iterator itr;
    WorldSession* session;
    m_sessionlock.AcquireReadLock();
    bool FoundUser = false;
    for (itr = m_sessions.begin(); itr != m_sessions.end();)
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
    m_sessionlock.ReleaseReadLock();
    if (FoundUser == false)
        m_session->SystemMessage("There is nobody online with account [%s]", account);
}

void World::DisconnectUsersWithIP(const char* ip, WorldSession* m_session)
{
    SessionMap::iterator itr;
    WorldSession* session;
    m_sessionlock.AcquireReadLock();
    bool FoundUser = false;
    for (itr = m_sessions.begin(); itr != m_sessions.end();)
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
    m_sessionlock.ReleaseReadLock();
}

void World::DisconnectUsersWithPlayerName(const char* plr, WorldSession* m_session)
{
    SessionMap::iterator itr;
    WorldSession* session;
    m_sessionlock.AcquireReadLock();
    bool FoundUser = false;
    for (itr = m_sessions.begin(); itr != m_sessions.end();)
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
    m_sessionlock.ReleaseReadLock();
}

void World::SendBCMessageByID(uint32 id)
{
    m_sessionlock.AcquireReadLock();
    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
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
    m_sessionlock.ReleaseReadLock();
}

// cebernic:2008-10-19
// Format as SendLocalizedWorldText("forcing english & {5}{12} %s","test");
// 5,12 are ids from worldstring_table
void World::SendLocalizedWorldText(bool wide, const char* format, ...) // May not optimized,just for works.
{
    m_sessionlock.AcquireReadLock();
    SessionMap::iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (itr->second->GetPlayer() &&
            itr->second->GetPlayer()->IsInWorld())
        {
            std::string temp = SessionLocalizedTextFilter(itr->second, format);
            // parsing
            format = (char*)temp.c_str();
            char buffer[1024];
            va_list ap;
            va_start(ap, format);
            vsnprintf(buffer, 1024, format, ap);
            va_end(ap);
            // again,we need parse args
            temp = SessionLocalizedTextFilter(itr->second, buffer);
            memset(buffer, 0, temp.length() + 1);
            memcpy(buffer, temp.c_str(), temp.length() + 1); // full done

            uint32 textLen = (uint32)strlen(buffer) + 1;
            WorldPacket data(textLen + 40);

            if (wide)
            {
                data.Initialize(SMSG_AREA_TRIGGER_MESSAGE);
                data << uint32(0);
                data << (char*)buffer;
                data << uint8(0);
            }
            else
            {
                data.Initialize(SMSG_MESSAGECHAT);
                data << uint8(CHAT_MSG_SYSTEM);
                data << uint32(LANG_UNIVERSAL);
                data << uint64(0); // Who cares about guid when there's no nickname displayed heh ?
                data << uint32(0);
                data << uint64(0);
                data << uint32(textLen);
                data << buffer;
                data << uint8(0);
            }
            itr->second->SendPacket(&data);
        }
    }
    m_sessionlock.ReleaseReadLock();
}

void World::SendZoneUnderAttackMsg(uint32 areaid, uint8 team)
{
    WorldPacket data(SMSG_ZONE_UNDER_ATTACK, 4);
    data << uint32(areaid);

    SendFactionMessage(&data, team);
}
