/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#include <string>
#include <cstdint>

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

class SERVER_DECL WorldConfig
{
    public:

        WorldConfig();
        ~WorldConfig();

        void loadWorldConfigValues(bool reload = false);

        // world.conf - Mysql Database Section
        struct WorldDatabaseSettings
        {
            std::string host;
            std::string user;
            std::string password;
            std::string dbName;
            int port;
            int connections;
        } worldDb;

        struct CharacterDatabaseSettings
        {
            std::string host;
            std::string user;
            std::string password;
            std::string dbName;
            int port;
            int connections;
        } charDb;

        // world.conf - Listen Config
        struct ListenSettings
        {
            std::string listenHost;
            int listenPort;
        } listen;

        // world.conf - Log Settings
        struct LogSettings
        {
            int worldFileLogLevel;
            int worldDebugFlags;
            bool enableWorldPacketLog;
            bool disableCrashdump;
            std::string extendedLogsDir;
            bool enableCheaterLog;
            bool enableGmCommandLog;
            bool enablePlayerLog;
            bool enableTimeStamp;
            bool enableSqlBanLog;
        } log;

        // world.conf - LogonServer Settings
        struct LogonServerSettings
        {
            std::string address;
            int port;
            std::string name;
            int realmCount;
            bool disablePings;
            std::string remotePassword;
        } logonServer;

        // world.conf - Server Settings
        struct ServerSettings
        {
            uint32_t playerLimit;
            std::string messageOfTheDay;
            bool sendStatsOnJoin;
            bool enableBreathing;
            bool seperateChatChannels;
            uint32_t compressionThreshold;
            uint32_t queueUpdateInterval;
            uint32_t secondsBeforeKickAFKPlayers;
            uint32_t secondsBeforeTimeOut;
            uint32_t realmType;
            bool enableAdjustPriority;
            bool requireAllSignatures;
            bool showGmInWhoList;
            uint32_t mapUnloadTime;
            uint8_t mapCellNumber;
            bool enableLimitedNames;
            bool useAccountData;
            bool requireGmForCommands;
            bool enableLfgJoinForNonLfg;
            int gmtTimeZone;
            bool disableFearMovement;
            bool saveExtendedCharData;
            std::string dataDir;
        } server;

        uint32_t getPlayerLimit();

        void setMessageOfTheDay(std::string motd);
        std::string getMessageOfTheDay();

        uint32_t getKickAFKPlayerTime();

        uint32_t getRealmType();

        // world.conf - Player Settings
        struct PlayerSettings
        {
            int32_t playerStartingLevel;
            uint32_t playerLevelCap;
            uint32_t playerGeneratedInformationByLevelCap;
            bool allowTbcCharacters;
            bool deactivateMasterLootNinja;
            uint32_t deathKnightStartTalentPoints;
            bool deathKnightPreReq;
            bool deathKnightLimit;
            uint32_t maxProfessions;
            bool skipCinematics;
            uint8_t enableInstantLogoutForAccessType;
            uint32_t minDualSpecLevel;
            uint32_t minTalentResetLevel;
            bool showAllVendorItems;
            bool isInterfactionChatEnabled;
            bool isInterfactionGroupEnabled;
            bool isInterfactionGuildEnabled;
            bool isInterfactionTradeEnabled;
            bool isInterfactionFriendsEnabled;
            bool isInterfactionMiscEnabled;
            bool isCrossoverCharsCreationEnabled;
            bool isGoldCapEnabled;
            uint32_t limitGoldAmount;
            uint32_t startGoldAmount;
        } player;

        // world.conf - Announce Settings
        struct AnnounceSettings
        {
            std::string announceTag;
            bool enableGmAdminTag;
            bool showNameInAnnounce;
            bool showNameInWAnnounce;
            bool showAnnounceInConsoleOutput;
            int tagColor;
            int tagGmColor;
            int nameColor;
            int msgColor;
        } announce;

        std::string getColorStringForNumber(int color);

        // world.conf - GameMaster Settings
        struct GameMasterSettings
        {
            bool isStartOnGmIslandEnabled;
            bool disableAchievements;
            bool listOnlyActiveGms;
            bool hidePermissions;
            bool worldAnnounceOnKickPlayer;
            std::string gmClientChannelName;
        } gm;

        std::string getGmClientChannelName();

        // world.conf - Broadcast Settings
        struct BroadcastSettings
        {
            bool isSystemEnabled;
            int interval;
            int triggerPercentCap;
            int orderMode;
        } broadcast;

        // world.conf - Rate Settings
        struct RateSettings
        {
            uint32_t arenaQueueDiff;
        } rate;

        float mFloatRates[MAX_RATES];
        uint32_t mIntRates[MAX_INTRATES];

        void setFloatRate(uint32_t index, float value);
        float getFloatRate(uint32_t index);

        void setIntRate(uint32_t index, uint32_t value);
        uint32_t getIntRate(uint32_t index);

        // world.conf - Terrain & Collision Settings
        struct TerrainCollisionSettings
        {
            bool unloadMapFiles;
            bool isCollisionEnabled;
            bool isPathfindingEnabled;
        } terrainCollision;

        // world.conf - Mail Settings
        struct MailSettings
        {
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
            bool enableMultithreadedLoading;
            bool enableSpellIdDump;
            std::string additionalTableLoads;
        } startup;

        // world.conf - AntiHack Setup
        struct AntiHackSettings
        {
            bool isTeleportHackCheckEnabled;
            bool isSpeedHackCkeckEnabled;
            bool isFallDamageHackCkeckEnabled;
            bool isFlyHackCkeckEnabled;
            uint32_t flyHackThreshold;
            bool isAntiHackCheckDisabledForGm;
        } antiHack;

        // world.conf - Period Setup
        struct PeriodSettings
        {
            std::string arenaUpdate;
            std::string dailyUpdate;
        } period;

        // world.conf - Chat Settings
        struct ChatSettings
        {
            std::string bannedChannels;
            std::string minimumTalkLevel;
            uint32_t linesBeforeProtection;
            uint32_t secondsBeforeProtectionReset;
            bool enableSendFloodProtectionMessage;
        } chat;

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
            uint32_t compressIntervalInMs;
            uint32_t compressRate;
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
            uint32_t minPlayerCountAlteracValley;
            uint32_t maxPlayerCountAlteracValley;
            uint32_t minPlayerCountArathiBasin;
            uint32_t maxPlayerCountArathiBasin;
            uint32_t minPlayerCountWarsongGulch;
            uint32_t maxPlayerCountWarsongGulch;
            uint32_t minPlayerCountEyeOfTheStorm;
            uint32_t maxPlayerCountEyeOfTheStorm;
            uint32_t minPlayerCountStrandOfTheAncients;
            uint32_t maxPlayerCountStrandOfTheAncients;
            uint32_t minPlayerCountIsleOfConquest;
            uint32_t maxPlayerCountIsleOfConquest;
            uint32_t firstRbgHonorValueToday;
            uint32_t firstRbgArenaHonorValueToday;
            uint32_t honorableKillsRbg;
            uint32_t honorableArenaWinRbg;
            uint32_t honorByLosingRbg;
            uint32_t honorByLosingArenaRbg;
        } bg;

        // world.conf - Arena Settings
        struct ArenaSettings
        {
            int arenaSeason;
            int arenaProgress;
            uint32_t minPlayerCount2V2;
            uint32_t maxPlayerCount2V2;
            uint32_t minPlayerCount3V3;
            uint32_t maxPlayerCount3V3;
            uint32_t minPlayerCount5V5;
            uint32_t maxPlayerCount5V5;
        } arena;

        // world.conf - Limits settings
        struct LimitSettings
        {
            bool isLimitSystemEnabled;
            uint32_t maxAutoAttackDamageCap;
            uint32_t maxSpellDamageCap;
            uint32_t maxHealthCap;
            uint32_t maxManaCap;
            uint32_t maxHonorPoints;
            uint32_t maxArenaPoints;
            bool disconnectPlayerForExceedingLimits;
            bool broadcastMessageToGmOnExceeding;
        } limit;

        // world.conf - Corpse Decay Settings
        struct CorpseDecaySettings
        {
            uint32_t normalTimeInSeconds;
            uint32_t rareTimeInSeconds;
            uint32_t eliteTimeInSeconds;
            uint32_t rareEliteTimeInSeconds;
            uint32_t worldbossTimeInSeconds;
        } corpseDecay;

};
