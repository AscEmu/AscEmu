/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum WorldConfigRates
{
    RATE_HEALTH = 0,    // hp
    RATE_POWER1,        // mp
    RATE_POWER2,        // rage
    RATE_POWER3,        // focus
    RATE_POWER4,        // energy
    //RATE_POWER5,
    //RATE_POWER6,
    RATE_POWER7,        // runic power
    RATE_DROP0,         // quality level
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

enum WorldConfigIntRates
{
    INTRATE_SAVE = 0,
    INTRATE_COMPRESSION,
    INTRATE_PVPTIMER,
    MAX_INTRATES
};

enum WorldConfigRealmTypes
{
    REALM_PVE = 0,
    REALM_PVP = 1
};

class WorldConfig
{
    public:

        WorldConfig();
        ~WorldConfig();

        void loadWorldConfigValues(bool reload = false);

        std::string getGmClientChannelName();

        void setMessageOfTheDay(std::string motd);
        std::string getMessageOfTheDay();

    private:

        float mFloatRates[MAX_RATES];
        uint32_t mIntRates[MAX_INTRATES];

    public:

        void setFloatRate(WorldConfigRates index, float value);
        float getFloatRate(WorldConfigRates index);

        void setIntRate(WorldConfigIntRates index, uint32_t value);
        uint32_t getIntRate(WorldConfigIntRates index);

        uint32_t getRealmType();

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
        } worldDb;

        struct CharacterDatabaseSettings
        {
            std::string host;
            std::string user;
            std::string password;
            std::string dbName;
            int port;
            //\todo add it to config or remove it from core
            int connections;        // not in configs
        } charDb;

        // world.conf - Listen Config
        struct ListenSettings
        {
            std::string listenHost;
            int listenPort;
        } listen;

        // world.conf - Log Level Setup
        struct LogLevelSettings
        {
            int fileLogLevel;
            int debugFlags;
            bool logWorldPacket;
            bool disableCrashdump;
        } logLevel;

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
        } server;

        // world.conf - Announce Configuration
        struct AnnounceSettings
        {
            std::string announceTag;
            bool enableGmAdminTag;
            bool showNameInAnnounce;
            bool showNameInWAnnounce;
            bool showAnnounceInConsoleOutput;
        } announce;

        // world.conf - Power regeneration multiplier setup
        struct RateSettings
        {
            uint32 arenaQueueDiff;
        } rate;

        // world.conf - GM Client Channel
        struct GMClientSettings
        {
            std::string gmClientChannelName;
        } gmClient;

        // world.conf - Terrain & Collision Settings
        struct TerrainCollisionSettings
        {
            std::string MapPath;
            std::string vMapPath;
            std::string mMapPath;
            bool unloadMapFiles;
            bool isCollisionEnabled;
            bool isPathfindingEnabled;
        } terrainCollision;

        // world.conf - Log Settings
        struct LogSettings
        {
            bool logCheaters;
            bool logGmCommands;
            //\todo remove it from core or add it to config
            bool logPlayers;       // not in config
            bool addTimeStampToFileName;
        } log;

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
        } mail;

        // world.conf - Startup Options
        struct StartupSettings
        {
            //\todo remove it from config or implement it
            bool isPreloadingCompleteWorldEnabled;  // not used by core
            bool isBackgroundLootLoadingEnabled;    // not used by core, not in config
            bool enableMultithreadedLoading;
            bool enableSpellIdDump;
            std::string additionalTableLoads;
        } startup;

        // world.conf - Flood Protection Setup
        struct FloodProtectionSettings
        {
            uint32 linesBeforeProtection;
            uint32 secondsBeforeProtectionReset;
            bool enableSendFloodProtectionMessage;
        } floodProtection;

        // world.conf - LogonServer Setup
        struct LogonServerSettings          // in realms.conf we have the same section...
        {
            bool disablePings;
            std::string remotePassword;
        } logonServer;

        // world.conf - AntiHack Setup
        struct AntiHackSettings
        {
            bool isTeleportHackCheckEnabled;
            bool isSpeedHackCkeckEnabled;
            bool isFallDamageHackCkeckEnabled;
            bool isFlyHackCkeckEnabled;
            uint32 flyHackThreshold;
            bool isAntiHackCheckDisabledForGm;
        } antiHack;

        // world.conf - Period Setup
        struct PeriodSettings
        {
            //\todo implement it
            std::string honorUpdate;        // not used by core
            std::string arenaUpdate;
            std::string dailyUpdate;
        } period;

        // world.conf - Channels Setup
        struct ChannelSettings
        {
            std::string bannedChannels;
            std::string minimumTalkLevel;
        } channel;

        // world.conf - Remote Console Setup
        struct RemoteConsoleSettings
        {
            bool isEnabled;
            std::string host;
            int port;
        } remoteConsole;

        // world.conf - Movement Setup
        struct MovementSettings
        {
            uint32 compressIntervalInMs;
            uint32 compressRate;
            float compressThresholdPlayers;
            float compressThresholdCreatures;
        } movement;

        // world.conf - Localization Setup
        struct LocalizationSettings
        {
            std::string localizedBindings;
        } localization;

        // world.conf - Dungeon / Instance Setup
        struct InstanceSettings
        {
            bool useGroupLeaderInstanceId;
            bool isRelativeExpirationEnabled;
            int relativeDailyHeroicInstanceResetHour;
            bool checkTriggerPrerequisitesOnEnter;
        } instance;

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
        } bg;

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
        } arena;

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
        } limit;

        // world.conf - MISSING in CONFIG!
        struct WorldSocketSettings
        {
            //\todo add it to config
            uint32 maxSocketSendBufSize;       // Section WorldSocket SendBuffSize
            uint32 maxSocketRecvBufSize;       // Section WorldSocket RecvBufSize
        } worldSocket;

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
        } optional;

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
        } interfaction;

        // optional.conf - Color Configuration
        struct ColorSettings
        {
            int tagColor;
            int tagGmColor;
            int nameColor;
            int msgColor;
        } color;

        // optional.conf - Game Master Configuration
        struct GameMasterSettings
        {
            bool isStartOnGmIslandEnabled;
            bool disableAchievements;
            bool listOnlyActiveGms;
            bool hidePermissions;
            bool worldAnnounceOnKickPlayer;
        } gm;

        // optional.conf - Common Schedule Configuration
        struct BroadcastSettings
        {
            bool isSystemEnabled;
            int interval;
            int triggerPercentCap;
            int orderMode;
        } broadcast;

        // optional.conf - Extra Class Configurations
        struct ExtraClassSettings
        {
            bool deathKnightPreReq;
            bool deathKnightLimit;
        } extraClass;

        // optional.conf - Gold Settings Configuration
        struct GoldSettings
        {
            bool isCapEnabled;
            uint32 limitAmount;
            uint32 startAmount;
        } gold;

        // optional.conf - Corpse Decay Settings
        struct CorpseDecaySettings
        {
            uint32 normalTimeInSeconds;
            uint32 rareTimeInSeconds;
            uint32 eliteTimeInSeconds;
            uint32 rareEliteTimeInSeconds;
            uint32 worldbossTimeInSeconds;
        } corpseDecay;

        //\todo move to one config file (world.conf)
        // realms.conf - LogonServer Section
        struct LogonServerSettings2
        {
            std::string address;
            int port;
            std::string name;
            int realmCount;
        } logonServer2;

        // realms.conf - Realm Section
        // handled in LogonCommHandler::LoadRealmConfiguration()
};
