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

enum Rates
{
    RATE_HEALTH = 0, // hp regen
    RATE_POWER1,  // mp regen
    RATE_POWER2,  // rage (rate unused)
    RATE_POWER3,  // focus regen (pets)
    RATE_POWER4,  // energy regen
//    RATE_POWER5,  // happiness (pets; rate unused)
//    RATE_POWER6,  // what is this? (rate unused)
    RATE_POWER7,  // runic power (rate unused)
    RATE_DROP0, // separate rates for each quality level
    RATE_DROP1,
    RATE_DROP2,
    RATE_DROP3,
    RATE_DROP4,
    RATE_DROP5,
    RATE_DROP6,
    RATE_MONEY,
    RATE_XP,
    RATE_RESTXP,
    RATE_QUESTXP,
    RATE_EXPLOREXP,
    RATE_HONOR,
    RATE_QUESTREPUTATION,
    RATE_KILLREPUTATION,
    RATE_SKILLCHANCE,
    RATE_SKILLRATE,
    RATE_ARENAPOINTMULTIPLIER2X,
    RATE_ARENAPOINTMULTIPLIER3X,
    RATE_ARENAPOINTMULTIPLIER5X,
    RATE_VEHICLES_POWER_REGEN,
    MAX_RATES
};

enum IntRates
{
    INTRATE_SAVE = 0,
    INTRATE_COMPRESSION,
    INTRATE_PVPTIMER,
    MAX_INTRATES
};

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

enum REALM_TYPE
{
    REALM_PVE = 0,
    REALM_PVP = 1
};

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
    //////////////////////////////////////////////////////////////////////////////////////////
    // Config values
    public:

        // world.conf - Mysql Database Section
        struct WorldDatabaseSettings
        {
            std::string host;
            std::string user;
            std::string password;
            std::string dbName;
            int port;
            //\todo add it to config or remove it from core
            int connections;        // not in configs
        } worldDbSettings;

        struct CharacterDatabaseSettings
        {
            std::string host;
            std::string user;
            std::string password;
            std::string dbName;
            int port;
            //\todo add it to config or remove it from core
            int connections;        // not in configs
        } charDbSettings;

        // world.conf - Listen Config
        struct ListenSettings
        {
            std::string listenHost;
            int listenPort;
        } listenSettings;

        // world.conf - Log Level Setup
        struct LogLevelSettings
        {
            int fileLogLevel;
            int debugFlags;
            bool logWorldPacket;
            bool disableCrashdump;
        } logLevelSettings;

        // world.conf - Server Settings
        struct ServerSettings
        {
            uint32 playerLimit;
            std::string messageOfTheDay;
            bool sendStatsOnJoin;
            bool enableBreathing;
            bool seperateChatChannels;
            uint32 compressionThreshold;
            uint32 queueUpdateInterval;
            uint32 secondsBeforeKickAFKPlayers;
            uint32 secondsBeforeTimeOut;
            uint32 realmType;
            bool enableAdjustPriority;
            bool requireAllSignatures;
            bool showGmInWhoList;
            uint32 mapUnloadTime;
            uint8 mapCellNumber;
            bool enableLimitedNames;
            bool useAccountData;
            bool requireGmForCommands;
            bool enableLfgJoinForNonLfg;
            int gmtTimeZone;
            bool disableFearMovement;
            bool saveExtendedCharData;
            //\todo add it to config or remove it from core
            bool skipAttunementForGm;           //not in config file!
            uint32 clientCacheVersion;          //not in config file!
            std::string banTable;               //not in config file!
        } serverSettings;

        // world.conf - Announce Configuration
        struct AnnounceSettings
        {
            std::string announceTag;
            bool enableGmAdminTag;
            bool showNameInAnnounce;
            bool showNameInWAnnounce;
            bool showAnnounceInConsoleOutput;
        } announceSettings;

        // world.conf - Power regeneration multiplier setup
        struct RateSettings
        {
            uint32 arenaQueueDiff;
        } rateSettings;

        // world.conf - GM Client Channel
        struct GMClientSettings
        {
            std::string gmClientChannelName;
        } gmClientSettings;

        // world.conf - Terrain & Collision Settings
        struct TerrainCollisionSettings
        {
            std::string MapPath;
            std::string vMapPath;
            std::string mMapPath;
            bool unloadMapFiles;
            bool isCollisionEnabled;
            bool isPathfindingEnabled;
        } terrainCollisionSettings;

        // world.conf - Log Settings
        struct LogSettings
        {
            bool logCheaters;
            bool logGmCommands;
            //\todo remove it from core or add it to config
            bool logPlayers;       // not in config
            bool addTimeStampToFileName;
        } logSettings;

        // world.conf - Mail System Setup
        struct MailSettings
        {
            //\todo remove it from config or implement it
            int reloadDelayInSeconds;                // not used by core
            bool isCostsForGmDisabled;
            bool isCostsForEveryoneDisabled;
            bool isDelayItemsDisabled;
            bool isMessageExpiryDisabled;
            bool isInterfactionMailEnabled;
            bool isInterfactionMailForGmEnabled;
        } mailSettings;

        // world.conf - Startup Options
        struct StartupSettings
        {
            //\todo remove it from config or implement it
            bool isPreloadingCompleteWorldEnabled;  // not used by core
            bool isBackgroundLootLoadingEnabled;    // not used by core, not in config
            bool enableMultithreadedLoading;
            bool enableSpellIdDump;
            std::string additionalTableLoads;
        } startupSettings;

        // world.conf - Flood Protection Setup
        struct FloodProtectionSettings
        {
            uint32 linesBeforeProtection;
            uint32 secondsBeforeProtectionReset;
            bool enableSendFloodProtectionMessage;
        } floodProtectionSettings;

        // world.conf - LogonServer Setup
        struct LogonServerSettings          // in realms.conf we have the same section...
        {
            bool disablePings;
            std::string remotePassword;
        } logonServerSettings;

        // world.conf - AntiHack Setup
        struct AntiHackSettings
        {
            bool isTeleportHackCheckEnabled;
            bool isSpeedHackCkeckEnabled;
            bool isFallDamageHackCkeckEnabled;
            bool isFlyHackCkeckEnabled;
            uint32 flyHackThreshold;
            bool isAntiHackCheckDisabledForGm;
        } antiHackSettings;

        // world.conf - Period Setup
        struct PeriodSettings
        {
            //\todo implement it
            std::string honorUpdate;        // not used by core
            std::string arenaUpdate;
            std::string dailyUpdate;
        } periodSettings;

        // world.conf - Channels Setup
        struct ChannelSettings
        {
            std::string bannedChannels;
            std::string minimumTalkLevel;
        } channelSettings;

        // world.conf - Remote Console Setup
        struct RemoteConsoleSettings
        {
            bool isEnabled;
            std::string host;
            int port;
        } remoteConsoleSettings;

        // world.conf - Movement Setup
        struct MovementSettings
        {
            uint32 compressIntervalInMs;
            uint32 compressRate;
            float compressThresholdPlayers;
            float compressThresholdCreatures;
        } movementSettings;

        // world.conf - Localization Setup
        struct LocalizationSettings
        {
            std::string localizedBindings;
        } localizationSettings;

        // world.conf - Dungeon / Instance Setup
        struct InstanceSettings
        {
            bool useGroupLeaderInstanceId;
            bool isRelativeExpirationEnabled;
            int relativeDailyHeroicInstanceResetHour;
            bool checkTriggerPrerequisitesOnEnter;
        } instanceSettings;

        // world.conf - BattleGround settings
        struct BattleGroundSettings
        {
            uint32 minPlayerCountAlteracValley;
            uint32 maxPlayerCountAlteracValley;
            uint32 minPlayerCountArathiBasin;
            uint32 maxPlayerCountArathiBasin;
            uint32 minPlayerCountWarsongGulch;
            uint32 maxPlayerCountWarsongGulch;
            uint32 minPlayerCountEyeOfTheStorm;
            uint32 maxPlayerCountEyeOfTheStorm;
            uint32 minPlayerCountStrandOfTheAncients;
            uint32 maxPlayerCountStrandOfTheAncients;
            uint32 minPlayerCountIsleOfConquest;
            uint32 maxPlayerCountIsleOfConquest;
            uint32 firstRbgHonorValueToday;
            uint32 firstRbgArenaHonorValueToday;
            uint32 honorableKillsRbg;
            uint32 honorableArenaWinRbg;
            uint32 honorByLosingRbg;
            uint32 honorByLosingArenaRbg;
        } bgSettings;

        // world.conf - Arena Settings
        struct ArenaSettings
        {
            int arenaSeason;
            int arenaProgress;
            uint32 minPlayerCount2V2;
            uint32 maxPlayerCount2V2;
            uint32 minPlayerCount3V3;
            uint32 maxPlayerCount3V3;
            uint32 minPlayerCount5V5;
            uint32 maxPlayerCount5V5;
        } arenaSettings;

        // world.conf - Limits settings
        struct LimitSettings
        {
            bool isLimitSystemEnabled;
            uint32 maxAutoAttackDamageCap;
            uint32 maxSpellDamageCap;
            uint32 maxHealthCap;
            uint32 maxManaCap;
            uint32 maxHonorPoints;
            uint32 maxArenaPoints;
            bool disconnectPlayerForExceedingLimits;
            bool broadcastMessageToGmOnExceeding;
        } limitSettings;

        // world.conf - MISSING in CONFIG!
        struct WorldSocketSettings
        {
            //\todo add it to config
            uint32 maxSocketSendBufSize;       // Section WorldSocket SendBuffSize
            uint32 maxSocketRecvBufSize;       // Section WorldSocket RecvBufSize
        } worldSocketSettings;

        // optional.conf - Optional Settings
        struct OptionalSettings
        {
            int32 playerStartingLevel;
            uint32 playerLevelCap;
            uint32 playerGeneratedInformationByLevelCap;
            bool allowTbcCharacters;
            bool deactivateMasterLootNinja;
            bool loadAdditionalFunScripts;
            uint32 deathKnightStartTalentPoints;
            //\todo remove it from config or implement in core
            //unstuck - Not loaded by core
            //unstuckcooldown - Not loaded by core
            //unstucktobind - Not loaded by core
            uint32 maxProfessions;
            bool skipCinematics;
            uint8 enableInstantLogoutForAccessType;
            uint32 minDualSpecLevel;
            uint32 minTalentResetLevel;
            bool showAllVendorItems;
        } optionalSettings;

        // optional.conf - Inter-faction Options
        struct InterfactionSettings
        {
            bool isInterfactionChatEnabled;
            bool isInterfactionGroupEnabled;
            bool isInterfactionGuildEnabled;
            bool isInterfactionTradeEnabled;
            bool isInterfactionFriendsEnabled;
            bool isInterfactionMiscEnabled;
            bool isCrossoverCharsCreationEnabled;
        } interfactionSettings;

        // optional.conf - Color Configuration
        struct ColorSettings
        {
            int tagColor;
            int tagGmColor;
            int nameColor;
            int msgColor;
        } colorSettings;

        // optional.conf - Game Master Configuration
        struct GameMasterSettings
        {
            bool isStartOnGmIslandEnabled;
            bool disableAchievements;
            bool listOnlyActiveGms;
            bool hidePermissions;
            bool worldAnnounceOnKickPlayer;
        } gmSettings;

        // optional.conf - Common Schedule Configuration
        struct BroadcastSettings
        {
            bool isSystemEnabled;
            int interval;
            int triggerPercentCap;
            int orderMode;
        } broadcastSettings;

        // optional.conf - Extra Class Configurations
        struct ExtraClassSettings
        {
            bool deathKnightPreReq;
            bool deathKnightLimit;
        } extraClassSettings;

        // optional.conf - Gold Settings Configuration
        struct GoldSettings
        {
            bool isCapEnabled;
            uint32 limitAmount;
            uint32 startAmount;
        } goldSettings;

        // optional.conf - Corpse Decay Settings
        struct CorpseDecaySettings
        {
            uint32 normalTimeInSeconds;
            uint32 rareTimeInSeconds;
            uint32 eliteTimeInSeconds;
            uint32 rareEliteTimeInSeconds;
            uint32 worldbossTimeInSeconds;
        } corpseDecaySettings;

        //\todo move to one config file (world.conf)
        // realms.conf - LogonServer Section
        struct LogonServerSettings2
        {
            std::string address;
            int port;
            std::string name;
            int realmCount;
        } logonServerSettings2;

        // realms.conf - Realm Section
        // handled in LogonCommHandler::LoadRealmConfiguration()

    private:
    //MIT End
    //AGPL Start

        uint32 HordePlayers;
        uint32 AlliancePlayers;

    public:

        inline uint32 getHordePlayerCount() { return HordePlayers; }
        inline uint32 getAlliancePlayerCount() { return AlliancePlayers; }
        inline uint32 getPlayerCount() { return (HordePlayers + AlliancePlayers); }
        inline void resetPlayerCount() { HordePlayers = AlliancePlayers = 0; }
        inline void incrementPlayerCount(uint32 faction)
        {
            if (faction == 1)
                HordePlayers++;
            else
                AlliancePlayers++;
        }
        inline void decrementPlayerCount(uint32 faction)
        {
            if (faction == 1)
                HordePlayers--;
            else
                AlliancePlayers--;
        }

        ///\todo Encapsulate below this point
    public:

        World();
        ~World();

#define DAMAGE(sp)     sp->OTspell_coef_override = sp->fixed_dddhcoef = sp->fixed_hotdotcoef = 0

        // Reloads the config and sets all of the setting variables
        void Rehash(bool load);

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
        uint32 GetNonGmSessionCount();
        inline size_t GetQueueCount() { return mQueuedSessions.size(); }
        void GetStats(uint32* GMCount, float* AverageLatency);

        inline uint32 GetPlayerLimit() const { return m_playerLimit; }
        void SetPlayerLimit(uint32 limit) { m_playerLimit = limit; }

        inline bool getAllowMovement() const { return m_allowMovement; }
        void SetAllowMovement(bool allow) { m_allowMovement = allow; }
        inline bool getGMTicketStatus() { return m_gmTicketSystem; }
        bool toggleGMTicketStatus()
        {
            m_gmTicketSystem = !m_gmTicketSystem;
            return m_gmTicketSystem;
        }

        inline std::string getGmClientChannel() { return gmClientSettings.gmClientChannelName; }

        void SetMotd(const char* motd) { serverSettings.messageOfTheDay = motd; }
        inline const char* GetMotd() const { return serverSettings.messageOfTheDay.c_str(); }

        bool SetInitialWorldSettings();

        void SendWorldText(const char* text, WorldSession* self = 0);
        void SendWorldWideScreenText(const char* text, WorldSession* self = 0);
        void SendGlobalMessage(WorldPacket* packet, WorldSession* self = 0);


        //////////////////////////////////////////////////////////////////////////////////////////
        /// Plays the sound to everyone logged in and in the world
        /// \param uint32 soundid  -  Identifier of the sound to play
        /// \return none
        //////////////////////////////////////////////////////////////////////////////////////////
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

        inline void SetStartTime(uint32 val) { m_StartTime = val; }
        inline uint32 GetUptime(void) { return (uint32)UNIXTIME - m_StartTime; }
        inline uint32 GetStartTime(void) { return m_StartTime; }
        std::string GetUptimeString();

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

        inline void setRate(int index, float value)
        {
            regen_values[index] = value;
        }

        inline float getRate(int index)
        {
            return regen_values[index];
        }

        inline uint32 getIntRate(int index)
        {
            return int_rates[index];
        }

        inline void setIntRate(int index, uint32 value)
        {
            int_rates[index] = value;
        }

        // talent inspection lookup tables
        std::map< uint32, uint32 > InspectTalentTabPos;
        std::map< uint32, uint32 > InspectTalentTabSize;
        std::map< uint32, uint32 > InspectTalentTabBit;
        uint32 InspectTalentTabPages[12][3];


        inline uint32 GetTimeOut() {return serverSettings.secondsBeforeTimeOut;}

        struct NameGenData
        {
            std::string name;
            uint32 type;
        };
        std::vector<NameGenData> _namegendata[3];
        void LoadNameGenData();

        std::string GenerateName(uint32 type = 0);

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

        void    SetKickAFKPlayerTime(uint32 idletimer) {m_KickAFKPlayers = idletimer;}
        uint32    GetKickAFKPlayerTime() {return m_KickAFKPlayers;}

        uint32 GetRealmType() { return serverSettings.realmType; }

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

        float regen_values[MAX_RATES];
        uint32 int_rates[MAX_INTRATES];

        uint32 m_playerLimit;
        bool m_allowMovement;
        bool m_gmTicketSystem;

        uint32 m_StartTime;
        uint32 m_queueUpdateTimer;

        QueueSet mQueuedSessions;

        uint32 m_KickAFKPlayers;

    public:

        std::list<SpellInfo*> dummyspells;

        char* m_banTable;

    protected:

        //Traffic meter stuff
        double TotalTrafficInKB;
        double TotalTrafficOutKB;
        double LastTotalTrafficInKB;
        double LastTotalTrafficOutKB;
        time_t LastTrafficQuery;

        void UpdateTotalTraffic();

    public:

        void QueryTotalTraffic(double* totalin, double* totalout)
        {
            // We don't want to spam this
            if (LastTrafficQuery == 0 || LastTrafficQuery <= (UNIXTIME - 10))
                UpdateTotalTraffic();

            *totalin = TotalTrafficInKB;
            *totalout = TotalTrafficOutKB;
        }

        void QueryLastTotalTraffic(double* totalin, double* totalout)
        {
            *totalin = LastTotalTrafficInKB;
            *totalout = LastTotalTrafficOutKB;
        }

        float GetCPUUsage()
        {
            return perfcounter.GetCurrentCPUUsage();
        }

        float GetRAMUsage()
        {
            return perfcounter.GetCurrentRAMUsage();
        }
};

#define sWorld World::getSingleton()

#endif      //__WORLD_H
