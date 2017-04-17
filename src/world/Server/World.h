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

#ifndef __WORLD_H
#define __WORLD_H

#include "EventableObject.h"
#include "IUpdatable.h"
#include "Definitions.h"
#include "Storage/DBC/DBCStores.h"
#include "Server/Packets/Handlers/AreaTrigger.h"
#include "WorldSession.h"
#include "WorldConfig.h"

#include <set>
#include <string>
#include <vector>

#define IS_INSTANCE(a) ((a > 1) && (a != 530) && (a != 571))

class Object;
class WorldPacket;
class WorldSession;
class Unit;
class Creature;
class GameObject;
class DynamicObject;
class Player;
class EventableObjectHolder;
class MapMgr;
class Battleground;
struct DatabaseConnection;

enum EnviromentalDamage
{
    DAMAGE_EXHAUSTED = 0,
    DAMAGE_DROWNING = 1,
    DAMAGE_FALL = 2,
    DAMAGE_LAVA = 3,
    DAMAGE_SLIME = 4,
    DAMAGE_FIRE = 5
};

// ServerMessages.dbc
enum ServerMessageType
{
    SERVER_MSG_SHUTDOWN_TIME            = 1,
    SERVER_MSG_RESTART_TIME             = 2,
    SERVER_MSG_STRING                   = 3,
    SERVER_MSG_SHUTDOWN_CANCELLED       = 4,
    SERVER_MSG_RESTART_CANCELLED        = 5,
    SERVER_MSG_BATTLEGROUND_SHUTDOWN    = 6,
    SERVER_MSG_BATTLEGROUND_RESTART     = 7,
    SERVER_MSG_INSTANCE_SHUTDOWN        = 8,
    SERVER_MSG_INSTANCE_RESTART         = 9
};

enum WorldMapInfoFlag
{
    WMI_INSTANCE_ENABLED            = 0x001,
    WMI_INSTANCE_WELCOME            = 0x002,
    WMI_INSTANCE_ARENA              = 0x004,
    WMI_INSTANCE_XPACK_01           = 0x008, //The Burning Crusade expansion
    WMI_INSTANCE_XPACK_02           = 0x010, //Wrath of the Lich King expansion
    WMI_INSTANCE_HAS_NORMAL_10MEN   = 0x020,
    WMI_INSTANCE_HAS_NORMAL_25MEN   = 0x040,
    WMI_INSTANCE_HAS_HEROIC_10MEN   = 0x080,
    WMI_INSTANCE_HAS_HEROIC_25MEN   = 0x100
};

enum AccountFlags
{
    ACCOUNT_FLAG_VIP            = 0x1,
    ACCOUNT_FLAG_NO_AUTOJOIN    = 0x2,
    //ACCOUNT_FLAG_XTEND_INFO   = 0x4,
    ACCOUNT_FLAG_XPACK_01       = 0x8,
    ACCOUNT_FLAG_XPACK_02       = 0x10,
    ACCOUNT_FLAG_XPACK_03       = 0x20
};

#pragma pack(push,1)
struct MapInfo
{
    uint32 mapid;
    uint32 screenid;
    uint32 type;
    uint32 playerlimit;
    uint32 minlevel;
    uint32 minlevel_heroic;
    float repopx;
    float repopy;
    float repopz;
    uint32 repopmapid;
    std::string name;
    uint32 flags;
    uint32 cooldown;
    uint32 lvl_mod_a;
    uint32 required_quest_A;
    uint32 required_quest_H;
    uint32 required_item;
    uint32 heroic_key_1;
    uint32 heroic_key_2;
    float update_distance;
    uint32 checkpoint_id;

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Tells if the map has this particular flag
    /// \param  uint32 flag  -  flag to check
    /// \return true if the map has the flag, otherwise false if the map doesn't have the flag.
    //////////////////////////////////////////////////////////////////////////////////////////
    bool HasFlag(uint32 flag) const
    {
        if ((flags & flag) != 0)
            return true;
        else
            return false;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Tells if the map has a particular raid difficulty.
    /// Valid difficulties are in the RAID_MODE enum.
    /// \param    uint32 difficulty  -  difficulty to check
    /// \return   true if the map has this difficulty, otherwise false.
    //////////////////////////////////////////////////////////////////////////////////////////
    bool HasDifficulty(uint32 difficulty) const
    {
        if (difficulty > uint32(TOTAL_RAID_MODES))
            return false;

        return HasFlag(uint32(WMI_INSTANCE_HAS_NORMAL_10MEN) << difficulty);
    }
};

#pragma pack(pop)


class BasicTaskExecutor : public ThreadBase
{
    CallbackBase* cb;
    uint32 priority;

    public:

        BasicTaskExecutor(CallbackBase* Callback, uint32 Priority) : cb(Callback), priority(Priority) {}
        ~BasicTaskExecutor() { delete cb; }
        bool run();
};

class Task
{
    CallbackBase* _cb;

    public:

        Task(CallbackBase* cb) : _cb(cb), completed(false), in_progress(false) {}
        ~Task() { delete _cb; }
        bool completed;
        bool in_progress;
        void execute();
};

struct CharacterLoaderThread : public ThreadBase
{
    Arcemu::Threading::ConditionVariable cond;

    bool running;

    public:

        CharacterLoaderThread();
        ~CharacterLoaderThread();
        void OnShutdown();
        bool run();
};

class TaskList
{
    std::set<Task*> tasks;
    Mutex queueLock;

    public:

        TaskList() : thread_count(0), running(false) {};
        Task* GetTask();
        void AddTask(Task* task);
        void RemoveTask(Task* task)
        {
            queueLock.Acquire();
            tasks.erase(task);
            queueLock.Release();
        }

        void spawn();
        void kill();

        void wait();
        void waitForThreadsToExit();
        Arcemu::Threading::AtomicCounter thread_count;
        bool running;
};

enum BasicTaskExecutorPriorities
{
    BTE_PRIORITY_LOW        = 0,
    BTE_PRIORITY_MED        = 1,
    BTW_PRIORITY_HIGH       = 2
};

class TaskExecutor : public ThreadBase
{
    TaskList* starter;

    public:

        TaskExecutor(TaskList* l) : starter(l) { ++l->thread_count; }
        ~TaskExecutor() { --starter->thread_count; }

        bool run();
};

class WorldSocket;

// Slow for remove in middle, oh well, wont get done much.
typedef std::list<WorldSocket*> QueueSet;
typedef std::set<WorldSession*> SessionSet;

// AGPL End
// MIT Start

class SERVER_DECL World : public Singleton<World>, public EventableObject, public IUpdatable
{
    public:

        World();
        ~World();

    private:

    //////////////////////////////////////////////////////////////////////////////////////////
    // WorldConfig
    public:

        WorldConfig settings;

        void loadWorldConfigValues(bool reload = false);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Player statistic
    private:
    
        uint32_t mHordePlayersCount;
        uint32_t mAlliancePlayersCount;

    public:

        uint32_t getPlayerCount();
        void resetPlayerCount();
        void incrementPlayerCount(uint32_t team);
        void decrementPlayerCount(uint32_t team);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Uptime
    private:

        uint32_t mStartTime;

    public:

        void setWorldStartTime(uint32_t start_time);
        uint32_t getWorldStartTime();
        uint32_t getWorldUptime();
        std::string getWorldUptimeString();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Traffic InfoCore
    private:

        double mTotalTrafficInKB;
        double mTotalTrafficOutKB;
        double mLastTotalTrafficInKB;
        double mLastTotalTrafficOutKB;
        time_t mLastTrafficQuery;

        void updateAllTrafficTotals();

    public:

        void setTotalTraffic(double* totalin, double* totalout);
        void setLastTotalTraffic(double* totalin, double* totalout);
        float getCPUUsage();
        float getRAMUsage();

    //MIT End
    //AGPL Start
        ///\todo Encapsulate below this point
    public:

        //session
        WorldSession* FindSession(uint32 id);
        WorldSession* FindSessionByName(const char*);
        void AddSession(WorldSession* s);
        void RemoveSession(uint32 id);

        void AddGlobalSession(WorldSession* session);
        void RemoveGlobalSession(WorldSession* session);
        void DeleteSession(WorldSession* session);
        void DeleteSessions(std::list< WorldSession* > &slist);

        size_t GetSessionCount()
        {
            m_sessionlock.AcquireReadLock();
            size_t ssize = m_sessions.size();
            m_sessionlock.ReleaseReadLock();

            return ssize;
        }

        // queue
        inline size_t GetQueueCount() { return mQueuedSessions.size(); }
        void GetStats(uint32* GMCount, float* AverageLatency);

        //movement
        inline bool getAllowMovement() const { return m_allowMovement; }
        void SetAllowMovement(bool allow) { m_allowMovement = allow; }

        //tickets
        inline bool getGMTicketStatus() { return m_gmTicketSystem; }
        bool toggleGMTicketStatus()
        {
            m_gmTicketSystem = !m_gmTicketSystem;
            return m_gmTicketSystem;
        }

        // world settings
        bool SetInitialWorldSettings();

        //messages
        void SendWorldText(const char* text, WorldSession* self = 0);
        void SendWorldWideScreenText(const char* text, WorldSession* self = 0);
        void SendGlobalMessage(WorldPacket* packet, WorldSession* self = 0);
        void PlaySoundToAll(uint32 soundid);
        void SendZoneMessage(WorldPacket* packet, uint32 zoneid, WorldSession* self = 0);
        void SendInstanceMessage(WorldPacket* packet, uint32 instanceid, WorldSession* self = 0);
        void SendFactionMessage(WorldPacket* packet, uint8 teamId);
        void SendGamemasterMessage(WorldPacket* packet, WorldSession* self = 0);
        void SendGMWorldText(const char* text, WorldSession* self = 0);
        void SendDamageLimitTextToGM(const char* playername, const char* dmglog);
        void SendBCMessageByID(uint32 id);
        void SendLocalizedWorldText(bool wide, const char* format, ...);
        void SendZoneUnderAttackMsg(uint32 areaid, uint8 team);

        // cebernic: textfilter,no fast,but works:D ...
        inline std::string SessionLocalizedTextFilter(WorldSession* _session, const char* text)
        {
            std::string opstr = std::string(text);
            std::string::iterator t = opstr.begin();
            std::string temp;
            int found = 0;
            std::string num;
            while(t != opstr.end())
            {
                if ((char)(*t) == '{' && strlen((char*) & (*t)) > 1)    // find and no end :D
                {
                    found++;
                    ++t;
                    continue;
                }
                if (found == 1)
                {
                    if ((char)(*t) == '}') found++;
                    else num.push_back(*t);
                }
                if (found)    // get the flag and doing my work and skip pushback.
                {
                    if (found == 2)
                    {
                        temp += _session->LocalizedWorldSrv((uint32) atoi((char*)num.c_str()));
                        found = 0;
                        num.clear();
                    }
                }
                else temp.push_back(*t);
                ++t;
            }
            return temp;
        }


        // update the world server every frame
        void Update(unsigned long time_passed);
        void CheckForExpiredInstances();
        void UpdateSessions(uint32 diff);


        //queue
        uint32 AddQueuedSocket(WorldSocket* Socket);
        void RemoveQueuedSocket(WorldSocket* Socket);
        uint32 GetQueuePos(WorldSocket* Socket);
        void UpdateQueuedSessions(uint32 diff);
        Mutex queueMutex;

        void SaveAllPlayers();

        uint32 mAcceptedConnections;
        
        uint32 PeakSessionCount;
        
        SessionSet gmList;
        
        void ShutdownClasses();
        void DeleteObject(Object* obj);
        

        std::string ann_namecolor;
        std::string ann_gmtagcolor;
        std::string ann_tagcolor;
        std::string ann_msgcolor;
        void AnnounceColorChooser(int tagcolor, int gmtagcolor, int namecolor, int msgcolor);

        void CharacterEnumProc(QueryResultVector & results, uint32 AccountId);
        void LoadAccountDataProc(QueryResultVector & results, uint32 AccountId);

        void DisconnectUsersWithAccount(const char* account, WorldSession* session);
        void DisconnectUsersWithIP(const char* ip, WorldSession* session);
        void DisconnectUsersWithPlayerName(const char* plr, WorldSession* session);

        void LogoutPlayers();

        typedef std::unordered_map<uint32, WorldSession*> SessionMap;
        SessionMap m_sessions;
        RWLock m_sessionlock;

    private:

        EventableObjectHolder* eventholder;
        //! Timers
        typedef std::unordered_map<uint32, AreaTrigger*> AreaTriggerMap;
        AreaTriggerMap m_AreaTrigger;

        Arcemu::PerformanceCounter perfcounter;

    protected:

        Mutex SessionsMutex;    //FOR GLOBAL !
        SessionSet Sessions;

        bool m_allowMovement;
        bool m_gmTicketSystem;

        
        uint32 m_queueUpdateTimer;

        QueueSet mQueuedSessions;

    public:

        std::list<SpellInfo*> dummyspells;

        char* m_banTable;
};

#define sWorld World::getSingleton()
#define worldConfig sWorld.settings

#endif      //__WORLD_H
