/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WorldConfig.h"

WorldConfig::WorldConfig()
{
    // world.conf - Mysql Database Section
    worldDbSettings.port = 3306;
    worldDbSettings.connections = 3;

    charDbSettings.port = 3306;
    charDbSettings.connections = 5;

    // world.conf - Listen Config
    listenSettings.listenPort = 8129;

    // world.conf - Log Level Setup
    logLevelSettings.fileLogLevel = 0;
    logLevelSettings.debugFlags = 0;
    logLevelSettings.logWorldPacket = false;
    logLevelSettings.disableCrashdump = false;

    // world.conf - Server Settings
    serverSettings.playerLimit = 1000;
    serverSettings.messageOfTheDay = "AscEmu Default MOTD";
    serverSettings.sendStatsOnJoin = true;
    serverSettings.enableBreathing = true;
    serverSettings.seperateChatChannels = false;
    serverSettings.compressionThreshold = 1000;
    serverSettings.queueUpdateInterval = 5000;
    serverSettings.secondsBeforeKickAFKPlayers = 0;
    serverSettings.secondsBeforeTimeOut = 180;
    serverSettings.realmType = false;
    serverSettings.enableAdjustPriority = false;
    serverSettings.requireAllSignatures = false;
    serverSettings.showGmInWhoList = true;
    serverSettings.mapUnloadTime = MAP_CELL_DEFAULT_UNLOAD_TIME;
    serverSettings.mapCellNumber = 1;
    serverSettings.enableLimitedNames = true;
    serverSettings.useAccountData = false;
    serverSettings.requireGmForCommands = false;
    serverSettings.enableLfgJoinForNonLfg = false;
    serverSettings.gmtTimeZone = 0;
    serverSettings.disableFearMovement = 0;
    serverSettings.saveExtendedCharData = false;
    serverSettings.skipAttunementForGm = true;
    serverSettings.clientCacheVersion = 12340;
    serverSettings.banTable = "";

    // world.conf - Announce Configuration
    announceSettings.enableGmAdminTag = true;
    announceSettings.showNameInAnnounce = false;
    announceSettings.showNameInWAnnounce = false;
    announceSettings.showAnnounceInConsoleOutput = true;

    // world.conf - Power regeneration multiplier setup
    rateSettings.arenaQueueDiff = 150;

    // world.conf - GM Client Channel
    gmClientSettings.gmClientChannelName = "";

    // world.conf - Terrain & Collision Settings
    terrainCollisionSettings.unloadMapFiles = false;
    terrainCollisionSettings.isCollisionEnabled = false;
    terrainCollisionSettings.isPathfindingEnabled = false;

    // world.conf - Log Settings
    logSettings.logCheaters = false;
    logSettings.logGmCommands = false;
    logSettings.logPlayers = false;
    logSettings.addTimeStampToFileName = false;

    // world.conf - Mail System Setup
    mailSettings.reloadDelayInSeconds = 0;
    mailSettings.isCostsForGmDisabled = false;
    mailSettings.isCostsForEveryoneDisabled = false;
    mailSettings.isDelayItemsDisabled = false;
    mailSettings.isMessageExpiryDisabled = false;
    mailSettings.isInterfactionMailEnabled = false;
    mailSettings.isInterfactionMailForGmEnabled = false;

    // world.conf - Startup Options
    startupSettings.isPreloadingCompleteWorldEnabled = false;
    startupSettings.isBackgroundLootLoadingEnabled = false;
    startupSettings.enableMultithreadedLoading = false;
    startupSettings.enableSpellIdDump = false;

    // world.conf - Flood Protection Setup
    floodProtectionSettings.linesBeforeProtection = 0;
    floodProtectionSettings.secondsBeforeProtectionReset = 0;
    floodProtectionSettings.enableSendFloodProtectionMessage = false;

    // world.conf - LogonServer Setup
    logonServerSettings.disablePings = false;

    // world.conf - AntiHack Setup
    antiHackSettings.isTeleportHackCheckEnabled = false;
    antiHackSettings.isSpeedHackCkeckEnabled = false;
    antiHackSettings.isFallDamageHackCkeckEnabled = false;
    antiHackSettings.isFlyHackCkeckEnabled = false;
    antiHackSettings.flyHackThreshold = 0;
    antiHackSettings.isAntiHackCheckDisabledForGm = true;

    // world.conf - Period Setup
    // world.conf - Channels Setup
    // world.conf - Remote Console Setup
    remoteConsoleSettings.isEnabled = false;
    remoteConsoleSettings.port = 8092;

    // world.conf - Movement Setup
    movementSettings.compressIntervalInMs = 1000;             // not used by core
    movementSettings.compressRate = 1;                    // not used by core
    movementSettings.compressThresholdCreatures = 15.0f;  // not used by core
    movementSettings.compressThresholdPlayers = 25.0f;           // not used by core

                                                                 // world.conf - Localization Setup
                                                                 // world.conf - Dungeon / Instance Setup
    instanceSettings.useGroupLeaderInstanceId = false;
    instanceSettings.isRelativeExpirationEnabled = false;
    instanceSettings.relativeDailyHeroicInstanceResetHour = 5;
    instanceSettings.checkTriggerPrerequisitesOnEnter = true;

    // world.conf - BattleGround settings
    bgSettings.minPlayerCountAlteracValley = 10;
    bgSettings.maxPlayerCountAlteracValley = 40;
    bgSettings.minPlayerCountArathiBasin = 5;
    bgSettings.maxPlayerCountArathiBasin = 15;
    bgSettings.minPlayerCountWarsongGulch = 5;
    bgSettings.maxPlayerCountWarsongGulch = 10;
    bgSettings.minPlayerCountEyeOfTheStorm = 5;
    bgSettings.maxPlayerCountEyeOfTheStorm = 15;
    bgSettings.minPlayerCountStrandOfTheAncients = 5;
    bgSettings.maxPlayerCountStrandOfTheAncients = 15;
    bgSettings.minPlayerCountIsleOfConquest = 10;
    bgSettings.maxPlayerCountIsleOfConquest = 40;
    bgSettings.firstRbgHonorValueToday = 30;
    bgSettings.firstRbgArenaHonorValueToday = 25;
    bgSettings.honorableKillsRbg = 15;
    bgSettings.honorableArenaWinRbg = 0;
    bgSettings.honorByLosingRbg = 5;
    bgSettings.honorByLosingArenaRbg = 0;

    // world.conf - Arena Settings
    arenaSettings.arenaSeason = 8;
    arenaSettings.arenaProgress = 1;
    arenaSettings.minPlayerCount2V2 = 2;
    arenaSettings.maxPlayerCount2V2 = 2;
    arenaSettings.maxPlayerCount3V3 = 3;
    arenaSettings.minPlayerCount3V3 = 3;
    arenaSettings.maxPlayerCount5V5 = 5;
    arenaSettings.minPlayerCount5V5 = 5;

    // world.conf - Limits settings
    limitSettings.isLimitSystemEnabled = true;
    limitSettings.maxAutoAttackDamageCap = 10000;
    limitSettings.maxSpellDamageCap = 30000;
    limitSettings.maxHealthCap = 100000;
    limitSettings.maxManaCap = 80000;
    limitSettings.maxHonorPoints = 75000;
    limitSettings.maxArenaPoints = 5000;
    limitSettings.disconnectPlayerForExceedingLimits = false;
    limitSettings.broadcastMessageToGmOnExceeding = true;

    // world.conf - MISSING in CONFIG!
    worldSocketSettings.maxSocketSendBufSize = WORLDSOCKET_SENDBUF_SIZE;
    worldSocketSettings.maxSocketRecvBufSize = WORLDSOCKET_RECVBUF_SIZE;

    // optional.conf - Optional Settings
    optionalSettings.playerStartingLevel = 1;
    optionalSettings.playerLevelCap = DBC_PLAYER_LEVEL_CAP;
    optionalSettings.playerGeneratedInformationByLevelCap = DBC_PLAYER_LEVEL_CAP; //! no delete
    optionalSettings.allowTbcCharacters = true;
    optionalSettings.deactivateMasterLootNinja = false;
    optionalSettings.loadAdditionalFunScripts = false;
    optionalSettings.deathKnightStartTalentPoints = 0;
    //unstuck - Not loaded by core
    //unstuckcooldown - Not loaded by core
    //unstucktobind - Not loaded by core
    optionalSettings.maxProfessions = 0;
    optionalSettings.skipCinematics = false;
    optionalSettings.enableInstantLogoutForAccessType = 1;
    optionalSettings.minDualSpecLevel = 40;
    optionalSettings.minTalentResetLevel = 10;
    optionalSettings.showAllVendorItems = false;

    // optional.conf - Inter-faction Options
    interfactionSettings.isInterfactionChatEnabled = false;
    interfactionSettings.isInterfactionGroupEnabled = false;
    interfactionSettings.isInterfactionGuildEnabled = false;
    interfactionSettings.isInterfactionTradeEnabled = false;
    interfactionSettings.isInterfactionFriendsEnabled = false;
    interfactionSettings.isInterfactionMiscEnabled = false;
    interfactionSettings.isCrossoverCharsCreationEnabled = true;

    // optional.conf - Color Configuration
    colorSettings.tagColor = 2;
    colorSettings.tagGmColor = 1;
    colorSettings.nameColor = 4;
    colorSettings.msgColor = 6;

    // optional.conf - Game Master Configuration
    gmSettings.isStartOnGmIslandEnabled = true;
    gmSettings.disableAchievements = false;
    gmSettings.listOnlyActiveGms = false;
    gmSettings.hidePermissions = false;
    gmSettings.worldAnnounceOnKickPlayer = true;

    // optional.conf - Common Schedule Configuration
    broadcastSettings.isSystemEnabled = false;
    broadcastSettings.interval = 10;
    broadcastSettings.triggerPercentCap = 2;
    broadcastSettings.orderMode = 0;

    // optional.conf - Extra Class Configurations
    extraClassSettings.deathKnightPreReq = false;
    extraClassSettings.deathKnightLimit = false;

    // optional.conf - Gold Settings Configuration
    goldSettings.isCapEnabled = true;
    goldSettings.limitAmount = 214748;
    goldSettings.startAmount = 0;

    // optional.conf - Corpse Decay Settings
    corpseDecaySettings.normalTimeInSeconds = 60000;
    corpseDecaySettings.rareTimeInSeconds = 300000;
    corpseDecaySettings.eliteTimeInSeconds = 300000;
    corpseDecaySettings.rareEliteTimeInSeconds = 300000;
    corpseDecaySettings.worldbossTimeInSeconds = 3600000;

    // realms.conf - LogonServer Section
    logonServerSettings2.port = 8093;
    logonServerSettings2.realmCount = 1;

    // realms.conf - Realm Section
    // handled in LogonCommHandler::LoadRealmConfiguration()
}

WorldConfig::~WorldConfig() {}

//////////////////////////////////////////////////////////////////////////////////////////
// Config functions
void WorldConfig::loadWorldConfigValues(bool reload /*false*/)
{
    if (reload)
    {
        // This will only happen if someone deleted/renamed the con-files after the server started...
        if (!Config.MainConfig.SetSource(CONFDIR "/world.conf", true))
        {
            LOG_ERROR("Rehash: file world.conf not available o.O !");
            return;
        }
        if (!Config.OptionalConfig.SetSource(CONFDIR "/optional.conf", true))
        {
            LOG_ERROR("Rehash: file optional.conf not available o.O !");
            return;
        }
    }

    if (!ChannelMgr::getSingletonPtr())
        new ChannelMgr;

    if (!MailSystem::getSingletonPtr())
        new MailSystem;

    // world.conf - Mysql Database Section
    worldDbSettings.host = Config.MainConfig.GetStringDefault("WorldDatabase", "Hostname", "");
    worldDbSettings.user = Config.MainConfig.GetStringDefault("WorldDatabase", "Username", "");
    worldDbSettings.password = Config.MainConfig.GetStringDefault("WorldDatabase", "Password", "");
    worldDbSettings.dbName = Config.MainConfig.GetStringDefault("WorldDatabase", "Name", "");
    worldDbSettings.port = Config.MainConfig.GetIntDefault("WorldDatabase", "Port", 3306);
    worldDbSettings.connections = Config.MainConfig.GetIntDefault("WorldDatabase", "ConnectionCount", 3);

    charDbSettings.host = Config.MainConfig.GetStringDefault("CharacterDatabase", "Hostname", "");
    charDbSettings.user = Config.MainConfig.GetStringDefault("CharacterDatabase", "Username", "");
    charDbSettings.password = Config.MainConfig.GetStringDefault("CharacterDatabase", "Password", "");
    charDbSettings.dbName = Config.MainConfig.GetStringDefault("CharacterDatabase", "Name", "");
    charDbSettings.port = Config.MainConfig.GetIntDefault("CharacterDatabase", "Port", 3306);
    charDbSettings.connections = Config.MainConfig.GetIntDefault("CharacterDatabase", "ConnectionCount", 5);

    // world.conf - Listen Config
    listenSettings.listenHost = Config.MainConfig.GetStringDefault("Listen", "Host", "0.0.0.0");
    listenSettings.listenPort = Config.MainConfig.GetIntDefault("Listen", "WorldServerPort", 8129);

    // world.conf - Log Level Setup
    logLevelSettings.fileLogLevel = Config.MainConfig.GetIntDefault("LogLevel", "File", 0);
    logLevelSettings.debugFlags = Config.MainConfig.GetIntDefault("LogLevel", "DebugFlags", 0);
    logLevelSettings.logWorldPacket = Config.MainConfig.GetBoolDefault("LogLevel", "World", false);
    logLevelSettings.disableCrashdump = Config.MainConfig.GetBoolDefault("LogLevel", "DisableCrashdumpReport", false);

    AscLog.SetFileLoggingLevel(logLevelSettings.fileLogLevel);
    AscLog.SetDebugFlags(logLevelSettings.debugFlags);

    // world.conf - Server Settings
    serverSettings.playerLimit = Config.MainConfig.GetIntDefault("Server", "PlayerLimit", 1000);
    serverSettings.messageOfTheDay = Config.MainConfig.GetStringDefault("Server", "Motd", "AscEmu Default MOTD");
    serverSettings.sendStatsOnJoin = Config.MainConfig.GetBoolDefault("Server", "SendStatsOnJoin", true);
    serverSettings.enableBreathing = Config.MainConfig.GetBoolDefault("Server", "EnableBreathing", true);
    serverSettings.seperateChatChannels = Config.MainConfig.GetBoolDefault("Server", "SeperateChatChannels", false);
    serverSettings.compressionThreshold = Config.MainConfig.GetIntDefault("Server", "CompressionThreshold", 1000);
    serverSettings.queueUpdateInterval = Config.MainConfig.GetIntDefault("Server", "QueueUpdateInterval", 5000);
    serverSettings.secondsBeforeKickAFKPlayers = Config.MainConfig.GetIntDefault("Server", "KickAFKPlayers", 0);
    serverSettings.secondsBeforeTimeOut = uint32(1000 * Config.MainConfig.GetIntDefault("Server", "ConnectionTimeout", 180));
    serverSettings.realmType = Config.MainConfig.GetBoolDefault("Server", "RealmType", false);
    serverSettings.enableAdjustPriority = Config.MainConfig.GetBoolDefault("Server", "AdjustPriority", false);
    serverSettings.requireAllSignatures = Config.MainConfig.GetBoolDefault("Server", "RequireAllSignatures", false);
    serverSettings.showGmInWhoList = Config.MainConfig.GetBoolDefault("Server", "ShowGMInWhoList", true);
    serverSettings.mapUnloadTime = Config.MainConfig.GetIntDefault("Server", "MapUnloadTime", MAP_CELL_DEFAULT_UNLOAD_TIME);
    serverSettings.mapCellNumber = Config.MainConfig.GetIntDefault("Server", "MapCellNumber", 1);
    serverSettings.enableLimitedNames = Config.MainConfig.GetBoolDefault("Server", "LimitedNames", true);
    serverSettings.useAccountData = Config.MainConfig.GetBoolDefault("Server", "UseAccountData", false);
    serverSettings.requireGmForCommands = !Config.MainConfig.GetBoolDefault("Server", "AllowPlayerCommands", false);
    serverSettings.enableLfgJoinForNonLfg = Config.MainConfig.GetBoolDefault("Server", "EnableLFGJoin", false);
    serverSettings.gmtTimeZone = Config.MainConfig.GetIntDefault("Server", "TimeZone", 0);
    serverSettings.disableFearMovement = Config.MainConfig.GetBoolDefault("Server", "DisableFearMovement", 0);
    serverSettings.saveExtendedCharData = Config.MainConfig.GetBoolDefault("Server", "SaveExtendedCharData", false);
    serverSettings.skipAttunementForGm = Config.MainConfig.GetBoolDefault("Server", "SkipAttunementsForGM", true);
    serverSettings.clientCacheVersion = uint32(Config.MainConfig.GetIntDefault("Server", "CacheVersion", 12340));
    serverSettings.banTable = Config.MainConfig.GetStringDefault("Server", "BanTable", "");

    /*SetPlayerLimit(serverSettings.playerLimit);
    SetKickAFKPlayerTime(serverSettings.secondsBeforeKickAFKPlayers);

    if (m_banTable != NULL)
        free(m_banTable);

    m_banTable = NULL;
    std::string s = serverSettings.banTable;
    if (!s.empty())
        m_banTable = strdup(s.c_str());*/

    if (serverSettings.mapUnloadTime == 0)
    {
        LOG_ERROR("MapUnloadTime is set to 0. This will NEVER unload MapCells!!! Overriding it to default value of %u", MAP_CELL_DEFAULT_UNLOAD_TIME);
        serverSettings.mapUnloadTime = MAP_CELL_DEFAULT_UNLOAD_TIME;
    }

    if (serverSettings.mapCellNumber == 0)
    {
        LOG_ERROR("MapCellNumber is set to 0. Congrats, no MapCells will be loaded. Overriding it to default value of 1");
        serverSettings.mapCellNumber = 1;
    }

#ifdef WIN32
    DWORD current_priority_class = GetPriorityClass(GetCurrentProcess());
    bool high = serverSettings.enableAdjustPriority;

    if (high)
    {
        if (current_priority_class != HIGH_PRIORITY_CLASS)
            SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    }
    else
    {
        if (current_priority_class != NORMAL_PRIORITY_CLASS)
            SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
    }
#endif

    channelmgr.seperatechannels = serverSettings.seperateChatChannels;

    // world.conf - Announce Configuration
    announceSettings.announceTag = Config.MainConfig.GetStringDefault("Announce", "Tag", "Staff");
    announceSettings.enableGmAdminTag = Config.MainConfig.GetBoolDefault("Announce", "GMAdminTag", false);
    announceSettings.showNameInAnnounce = Config.MainConfig.GetBoolDefault("Announce", "NameinAnnounce", true);
    announceSettings.showNameInWAnnounce = Config.MainConfig.GetBoolDefault("Announce", "NameinWAnnounce", true);
    announceSettings.showAnnounceInConsoleOutput = Config.MainConfig.GetBoolDefault("Announce", "ShowInConsole", true);

    // world.conf - Power regeneration multiplier setup
    setFloatRate(RATE_HEALTH, Config.MainConfig.GetFloatDefault("Rates", "Health", 1)); // health
    setFloatRate(RATE_POWER1, Config.MainConfig.GetFloatDefault("Rates", "Power1", 1)); // mana
    setFloatRate(RATE_POWER2, Config.MainConfig.GetFloatDefault("Rates", "Power2", 1)); // rage
    setFloatRate(RATE_POWER3, Config.MainConfig.GetFloatDefault("Rates", "Power3", 1)); // focus
    setFloatRate(RATE_POWER4, Config.MainConfig.GetFloatDefault("Rates", "Power4", 1)); // energy
    setFloatRate(RATE_POWER7, Config.MainConfig.GetFloatDefault("Rates", "Power7", 1)); // runic power (rate unused)
    setFloatRate(RATE_VEHICLES_POWER_REGEN, Config.MainConfig.GetFloatDefault("Rates", "VehiclePower", 1.0f)); // Vehicle power regeneration
    setFloatRate(RATE_DROP0, Config.MainConfig.GetFloatDefault("Rates", "DropGrey", 1));
    setFloatRate(RATE_DROP1, Config.MainConfig.GetFloatDefault("Rates", "DropWhite", 1));
    setFloatRate(RATE_DROP2, Config.MainConfig.GetFloatDefault("Rates", "DropGreen", 1));
    setFloatRate(RATE_DROP3, Config.MainConfig.GetFloatDefault("Rates", "DropBlue", 1));
    setFloatRate(RATE_DROP4, Config.MainConfig.GetFloatDefault("Rates", "DropPurple", 1));
    setFloatRate(RATE_DROP5, Config.MainConfig.GetFloatDefault("Rates", "DropOrange", 1));
    setFloatRate(RATE_DROP6, Config.MainConfig.GetFloatDefault("Rates", "DropArtifact", 1));
    setFloatRate(RATE_XP, Config.MainConfig.GetFloatDefault("Rates", "XP", 1));
    setFloatRate(RATE_RESTXP, Config.MainConfig.GetFloatDefault("Rates", "RestXP", 1));
    setFloatRate(RATE_QUESTXP, Config.MainConfig.GetFloatDefault("Rates", "QuestXP", 1));
    setFloatRate(RATE_EXPLOREXP, Config.MainConfig.GetFloatDefault("Rates", "ExploreXP", 1));
    setIntRate(INTRATE_SAVE, Config.MainConfig.GetIntDefault("Rates", "Save", 1));
    setFloatRate(RATE_MONEY, Config.MainConfig.GetFloatDefault("Rates", "DropMoney", 1.0f));
    setFloatRate(RATE_QUESTREPUTATION, Config.MainConfig.GetFloatDefault("Rates", "QuestReputation", 1.0f));
    setFloatRate(RATE_KILLREPUTATION, Config.MainConfig.GetFloatDefault("Rates", "KillReputation", 1.0f));
    setFloatRate(RATE_HONOR, Config.MainConfig.GetFloatDefault("Rates", "Honor", 1.0f));
    setFloatRate(RATE_SKILLCHANCE, Config.MainConfig.GetFloatDefault("Rates", "SkillChance", 1.0f));
    setFloatRate(RATE_SKILLRATE, Config.MainConfig.GetFloatDefault("Rates", "SkillRate", 1.0f));
    setIntRate(INTRATE_COMPRESSION, Config.MainConfig.GetIntDefault("Rates", "Compression", 1));
    setIntRate(INTRATE_PVPTIMER, Config.MainConfig.GetIntDefault("Rates", "PvPTimer", 300000));
    rateSettings.arenaQueueDiff = Config.MainConfig.GetIntDefault("Rates", "ArenaQueueDiff", 150);
    setFloatRate(RATE_ARENAPOINTMULTIPLIER2X, Config.MainConfig.GetFloatDefault("Rates", "ArenaMultiplier2x", 1.0f));
    setFloatRate(RATE_ARENAPOINTMULTIPLIER3X, Config.MainConfig.GetFloatDefault("Rates", "ArenaMultiplier3x", 1.0f));
    setFloatRate(RATE_ARENAPOINTMULTIPLIER5X, Config.MainConfig.GetFloatDefault("Rates", "ArenaMultiplier5x", 1.0f));

    // world.conf - GM Client Channel
    gmClientSettings.gmClientChannelName = Config.MainConfig.GetStringDefault("GMClient", "GmClientChannel", "");

    // world.conf - Terrain & Collision Settings
    terrainCollisionSettings.MapPath = Config.MainConfig.GetStringDefault("Terrain", "MapPath", "maps");
    terrainCollisionSettings.vMapPath = Config.MainConfig.GetStringDefault("Terrain", "vMapPath", "vmaps");
    terrainCollisionSettings.mMapPath = Config.MainConfig.GetStringDefault("Terrain", "mMapPath", "mmaps");
    terrainCollisionSettings.unloadMapFiles = Config.MainConfig.GetBoolDefault("Terrain", "UnloadMapFiles", true);
    terrainCollisionSettings.isCollisionEnabled = Config.MainConfig.GetBoolDefault("Terrain", "Collision", false);
    terrainCollisionSettings.isPathfindingEnabled = Config.MainConfig.GetBoolDefault("Terrain", "Pathfinding", false);

    // world.conf - Log Settings
    logSettings.logCheaters = Config.MainConfig.GetBoolDefault("Log", "Cheaters", false);
    logSettings.logGmCommands = Config.MainConfig.GetBoolDefault("Log", "GMCommands", false);
    logSettings.logPlayers = Config.MainConfig.GetBoolDefault("Log", "Player", false);
    logSettings.addTimeStampToFileName = Config.MainConfig.GetBoolDefault("log", "TimeStamp", false);

    // world.conf - Mail System Setup
    mailSettings.reloadDelayInSeconds = Config.MainConfig.GetIntDefault("Mail", "ReloadDelay", 0);
    mailSettings.isCostsForGmDisabled = Config.MainConfig.GetBoolDefault("Mail", "DisablePostageCostsForGM", true);
    mailSettings.isCostsForEveryoneDisabled = Config.MainConfig.GetBoolDefault("Mail", "DisablePostageCosts", false);
    mailSettings.isDelayItemsDisabled = Config.MainConfig.GetBoolDefault("Mail", "DisablePostageDelayItems", true);
    mailSettings.isMessageExpiryDisabled = Config.MainConfig.GetBoolDefault("Mail", "DisableMessageExpiry", false);
    mailSettings.isInterfactionMailEnabled = Config.MainConfig.GetBoolDefault("Mail", "EnableInterfactionMail", true);
    mailSettings.isInterfactionMailForGmEnabled = Config.MainConfig.GetBoolDefault("Mail", "EnableInterfactionForGM", true);

    uint32 config_flags = 0;

    if (mailSettings.isCostsForGmDisabled)
        config_flags |= MAIL_FLAG_NO_COST_FOR_GM;

    if (mailSettings.isCostsForEveryoneDisabled)
        config_flags |= MAIL_FLAG_DISABLE_POSTAGE_COSTS;

    if (mailSettings.isDelayItemsDisabled)
        config_flags |= MAIL_FLAG_DISABLE_HOUR_DELAY_FOR_ITEMS;

    if (mailSettings.isMessageExpiryDisabled)
        config_flags |= MAIL_FLAG_NO_EXPIRY;

    if (mailSettings.isInterfactionMailEnabled)
        config_flags |= MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION;

    if (mailSettings.isInterfactionMailForGmEnabled)
        config_flags |= MAIL_FLAG_CAN_SEND_TO_OPPOSITE_FACTION_GM;

    sMailSystem.config_flags = config_flags;

    // world.conf - Startup Options
    //startupSettings.Preloading;                    // not used
    //startupSettings.BackgroundLootLoading;         // not used, not in config
    startupSettings.enableMultithreadedLoading = Config.MainConfig.GetBoolDefault("Startup", "EnableMultithreadedLoading", true);
    startupSettings.enableSpellIdDump = Config.MainConfig.GetBoolDefault("Startup", "EnableSpellIDDump", false);
    startupSettings.additionalTableLoads = Config.MainConfig.GetStringDefault("Startup", "LoadAdditionalTables", "");

    // world.conf - Flood Protection Setup
    floodProtectionSettings.linesBeforeProtection = Config.MainConfig.GetIntDefault("FloodProtection", "Lines", 0);
    floodProtectionSettings.secondsBeforeProtectionReset = Config.MainConfig.GetIntDefault("FloodProtection", "Seconds", 0);
    floodProtectionSettings.enableSendFloodProtectionMessage = Config.MainConfig.GetBoolDefault("FloodProtection", "SendMessage", false);

    if (!floodProtectionSettings.linesBeforeProtection || !floodProtectionSettings.secondsBeforeProtectionReset)
        floodProtectionSettings.linesBeforeProtection = floodProtectionSettings.secondsBeforeProtectionReset = 0;

    // world.conf - LogonServer Setup
    logonServerSettings.disablePings = Config.MainConfig.GetBoolDefault("LogonServer", "DisablePings", false);
    logonServerSettings.remotePassword = Config.MainConfig.GetStringDefault("LogonServer", "RemotePassword", "r3m0t3");

    // world.conf - AntiHack Setup
    antiHackSettings.isTeleportHackCheckEnabled = Config.MainConfig.GetBoolDefault("AntiHack", "Teleport", true);
    antiHackSettings.isSpeedHackCkeckEnabled = Config.MainConfig.GetBoolDefault("AntiHack", "Speed", true);
    antiHackSettings.isFallDamageHackCkeckEnabled = Config.MainConfig.GetBoolDefault("AntiHack", "FallDamage", true);
    antiHackSettings.isFlyHackCkeckEnabled = Config.MainConfig.GetBoolDefault("AntiHack", "Flight", true);
    antiHackSettings.flyHackThreshold = Config.MainConfig.GetIntDefault("AntiHack", "FlightThreshold", 8);
    antiHackSettings.isAntiHackCheckDisabledForGm = Config.MainConfig.GetBoolDefault("AntiHack", "DisableOnGM", true);

    // world.conf - Period Setup
    periodSettings.honorUpdate = Config.MainConfig.GetStringDefault("Periods", "HonorUpdate", "daily");
    periodSettings.arenaUpdate = Config.MainConfig.GetStringDefault("Periods", "ArenaUpdate", "weekly");
    periodSettings.dailyUpdate = Config.MainConfig.GetStringDefault("Periods", "DailyUpdate", "daily");

    // world.conf - Channels Setup
    channelSettings.bannedChannels = Config.MainConfig.GetStringDefault("Channels", "BannedChannels", "");
    channelSettings.minimumTalkLevel = Config.MainConfig.GetStringDefault("Channels", "MinimumLevel", "");

    // world.conf - Remote Console Setup
    remoteConsoleSettings.isEnabled = Config.MainConfig.GetBoolDefault("RemoteConsole", "Enabled", false);
    remoteConsoleSettings.host = Config.MainConfig.GetStringDefault("RemoteConsole", "Host", "0.0.0.0");
    remoteConsoleSettings.port = Config.MainConfig.GetIntDefault("RemoteConsole", "Port", 8092);

    // world.conf - Movement Setup
    movementSettings.compressIntervalInMs = Config.MainConfig.GetIntDefault("Movement", "FlushInterval", 1000);
    movementSettings.compressRate = Config.MainConfig.GetIntDefault("Movement", "CompressRate", 1);

    movementSettings.compressThresholdCreatures = Config.MainConfig.GetFloatDefault("Movement", "CompressThresholdCreatures", 15.0f);
    movementSettings.compressThresholdCreatures *= movementSettings.compressThresholdCreatures;

    movementSettings.compressThresholdPlayers = Config.MainConfig.GetFloatDefault("Movement", "CompressThreshold", 25.0f);
    movementSettings.compressThresholdPlayers *= movementSettings.compressThresholdPlayers; // square it to avoid sqrt() on checks

                                                                                            // world.conf - Localization Setup
    localizationSettings.localizedBindings = Config.MainConfig.GetStringDefault("Localization", "LocaleBindings", "");

    // world.conf - Dungeon / Instance Setup
    instanceSettings.useGroupLeaderInstanceId = Config.MainConfig.GetBoolDefault("InstanceHandling", "TakeGroupLeaderID", true);
    instanceSettings.isRelativeExpirationEnabled = Config.MainConfig.GetBoolDefault("InstanceHandling", "SlidingExpiration", false);
    instanceSettings.relativeDailyHeroicInstanceResetHour = Config.MainConfig.GetIntDefault("InstanceHandling", "DailyHeroicInstanceResetHour", 5);
    instanceSettings.checkTriggerPrerequisitesOnEnter = Config.MainConfig.GetBoolDefault("InstanceHandling", "CheckTriggerPrerequisites", true);

    if (instanceSettings.relativeDailyHeroicInstanceResetHour < 0)
        instanceSettings.relativeDailyHeroicInstanceResetHour = 0;
    if (instanceSettings.relativeDailyHeroicInstanceResetHour > 23)
        instanceSettings.relativeDailyHeroicInstanceResetHour = 23;

    // world.conf - BattleGround settings
    bgSettings.minPlayerCountAlteracValley = Config.MainConfig.GetIntDefault("Battleground", "AV_MIN", 10);
    bgSettings.maxPlayerCountAlteracValley = Config.MainConfig.GetIntDefault("Battleground", "AV_MAX", 40);
    bgSettings.minPlayerCountArathiBasin = Config.MainConfig.GetIntDefault("Battleground", "AB_MIN", 4);
    bgSettings.maxPlayerCountArathiBasin = Config.MainConfig.GetIntDefault("Battleground", "AB_MAX", 15);
    bgSettings.minPlayerCountWarsongGulch = Config.MainConfig.GetIntDefault("Battleground", "WSG_MIN", 2);
    bgSettings.maxPlayerCountWarsongGulch = Config.MainConfig.GetIntDefault("Battleground", "WSG_MAX", 10);
    bgSettings.minPlayerCountEyeOfTheStorm = Config.MainConfig.GetIntDefault("Battleground", "EOTS_MIN", 4);
    bgSettings.maxPlayerCountEyeOfTheStorm = Config.MainConfig.GetIntDefault("Battleground", "EOTS_MAX", 15);
    bgSettings.minPlayerCountStrandOfTheAncients = Config.MainConfig.GetIntDefault("Battleground", "SOTA_MIN", 10);
    bgSettings.maxPlayerCountStrandOfTheAncients = Config.MainConfig.GetIntDefault("Battleground", "SOTA_MAX", 15);
    bgSettings.minPlayerCountIsleOfConquest = Config.MainConfig.GetIntDefault("Battleground", "IOC_MIN", 10);
    bgSettings.maxPlayerCountIsleOfConquest = Config.MainConfig.GetIntDefault("Battleground", "IOC_MAX", 15);
    bgSettings.firstRbgHonorValueToday = Config.MainConfig.GetIntDefault("Battleground", "RBG_FIRST_WIN_HONOR", 30);
    bgSettings.firstRbgArenaHonorValueToday = Config.MainConfig.GetIntDefault("Battleground", "RBG_FIRST_WIN_ARENA", 25);
    bgSettings.honorableKillsRbg = Config.MainConfig.GetIntDefault("Battleground", "RBG_WIN_HONOR", 15);
    bgSettings.honorableArenaWinRbg = Config.MainConfig.GetIntDefault("Battleground", "RBG_WIN_ARENA", 0);
    bgSettings.honorByLosingRbg = Config.MainConfig.GetIntDefault("Battleground", "RBG_LOSE_HONOR", 5);
    bgSettings.honorByLosingArenaRbg = Config.MainConfig.GetIntDefault("Battleground", "RBG_LOSE_ARENA", 0);

    // world.conf - Arena Settings
    arenaSettings.arenaSeason = Config.MainConfig.GetIntDefault("Arena", "Season", 1);
    arenaSettings.arenaProgress = Config.MainConfig.GetIntDefault("Arena", "Progress", 1);
    arenaSettings.minPlayerCount2V2 = Config.MainConfig.GetIntDefault("Arena", "2V2_MIN", 2);
    arenaSettings.maxPlayerCount2V2 = Config.MainConfig.GetIntDefault("Arena", "2V2_MAX", 2);
    arenaSettings.minPlayerCount3V3 = Config.MainConfig.GetIntDefault("Arena", "3V3_MIN", 3);
    arenaSettings.maxPlayerCount3V3 = Config.MainConfig.GetIntDefault("Arena", "3V3_MAX", 3);
    arenaSettings.minPlayerCount5V5 = Config.MainConfig.GetIntDefault("Arena", "5V5_MIN", 5);
    arenaSettings.maxPlayerCount5V5 = Config.MainConfig.GetIntDefault("Arena", "5V5_MAX", 5);

    // world.conf - Limits settings
    limitSettings.isLimitSystemEnabled = Config.MainConfig.GetBoolDefault("Limits", "Enable", true);
    limitSettings.maxAutoAttackDamageCap = (uint32)Config.MainConfig.GetIntDefault("Limits", "AutoAttackDmg", 10000);
    limitSettings.maxSpellDamageCap = (uint32)Config.MainConfig.GetIntDefault("Limits", "SpellDmg", 30000);
    limitSettings.maxHealthCap = (uint32)Config.MainConfig.GetIntDefault("Limits", "Health", 80000);
    limitSettings.maxManaCap = (uint32)Config.MainConfig.GetIntDefault("Limits", "Mana", 80000);
    limitSettings.maxHonorPoints = (uint32)Config.MainConfig.GetIntDefault("Limits", "Honor", 75000);
    limitSettings.maxArenaPoints = (uint32)Config.MainConfig.GetIntDefault("Limits", "Arena", 5000);
    limitSettings.disconnectPlayerForExceedingLimits = Config.MainConfig.GetBoolDefault("Limits", "Disconnect", false);
    limitSettings.broadcastMessageToGmOnExceeding = Config.MainConfig.GetBoolDefault("Limits", "BroadcastGMs", true);

    // world.conf - MISSING in CONFIG!
    worldSocketSettings.maxSocketRecvBufSize = Config.MainConfig.GetIntDefault("WorldSocket", "RecvBufSize", WORLDSOCKET_RECVBUF_SIZE);
    worldSocketSettings.maxSocketSendBufSize = Config.MainConfig.GetIntDefault("WorldSocket", "SendBufSize", WORLDSOCKET_SENDBUF_SIZE);

    // optional.conf - Optional Settings
    optionalSettings.playerStartingLevel = Config.OptionalConfig.GetIntDefault("Optional", "StartingLevel", 1);
    optionalSettings.playerLevelCap = Config.OptionalConfig.GetIntDefault("Optional", "LevelCap", DBC_PLAYER_LEVEL_CAP);
    optionalSettings.playerGeneratedInformationByLevelCap = Config.OptionalConfig.GetIntDefault("Optional", "GenLevelCap", DBC_PLAYER_LEVEL_CAP); //! no delete
    if (optionalSettings.playerStartingLevel > static_cast<int32>(optionalSettings.playerLevelCap))
        optionalSettings.playerStartingLevel = static_cast<int32>(optionalSettings.playerLevelCap);

    optionalSettings.allowTbcCharacters = Config.OptionalConfig.GetBoolDefault("Optional", "AllowTBC", true);
    optionalSettings.deactivateMasterLootNinja = Config.OptionalConfig.GetBoolDefault("Optional", "AntiMasterLootNinja", false);
    optionalSettings.deathKnightStartTalentPoints = Config.OptionalConfig.GetIntDefault("Optional", "DKStartingTalents", 0);
    optionalSettings.maxProfessions = (uint32)Config.OptionalConfig.GetIntDefault("Optional", "MaxProfessions", 2);
    //unstuck - Not loaded by core
    //unstuckcooldown - Not loaded by core
    //unstucktobind - Not loaded by core
    optionalSettings.skipCinematics = Config.OptionalConfig.GetBoolDefault("Optional", "SkipCinematic", false);
    optionalSettings.enableInstantLogoutForAccessType = Config.OptionalConfig.GetIntDefault("Optional", "InstantLogout", 1);
    optionalSettings.minDualSpecLevel = Config.OptionalConfig.GetIntDefault("Optional", "MinDualSpecLevel", 40);
    optionalSettings.minTalentResetLevel = Config.OptionalConfig.GetIntDefault("Optional", "MinTalentResetLevel", 10);
    optionalSettings.showAllVendorItems = Config.OptionalConfig.GetBoolDefault("Optional", "ShowAllVendorItems", false);

    // optional.conf - Inter-faction Options
    interfactionSettings.isInterfactionChatEnabled = Config.OptionalConfig.GetBoolDefault("Interfaction", "InterfactionChat", false);
    interfactionSettings.isInterfactionGroupEnabled = Config.OptionalConfig.GetBoolDefault("Interfaction", "InterfactionGroup", false);
    interfactionSettings.isInterfactionGuildEnabled = Config.OptionalConfig.GetBoolDefault("Interfaction", "InterfactionGuild", false);
    interfactionSettings.isInterfactionTradeEnabled = Config.OptionalConfig.GetBoolDefault("Interfaction", "InterfactionTrade", false);
    interfactionSettings.isInterfactionFriendsEnabled = Config.OptionalConfig.GetBoolDefault("Interfaction", "InterfactionFriends", false);
    interfactionSettings.isInterfactionMiscEnabled = Config.OptionalConfig.GetBoolDefault("Interfaction", "InterfactionMisc", false);
    interfactionSettings.isCrossoverCharsCreationEnabled = Config.OptionalConfig.GetBoolDefault("Interfaction", "CrossOverCharacters", false);

    // optional.conf - Color Configuration
    colorSettings.tagColor = Config.OptionalConfig.GetIntDefault("Color", "AnnTagColor", 2);
    colorSettings.tagGmColor = Config.OptionalConfig.GetIntDefault("Color", "AnnGMTagColor", 1);
    colorSettings.nameColor = Config.OptionalConfig.GetIntDefault("Color", "AnnNameColor", 4);
    colorSettings.msgColor = Config.OptionalConfig.GetIntDefault("Color", "AnnMsgColor", 10);
    //AnnounceColorChooser(colorSettings.tagColor, colorSettings.tagGmColor, colorSettings.nameColor, colorSettings.msgColor);

    // optional.conf - Game Master Configuration
    gmSettings.isStartOnGmIslandEnabled = Config.OptionalConfig.GetBoolDefault("GameMaster", "StartOnGMIsland", false);
    gmSettings.disableAchievements = Config.OptionalConfig.GetBoolDefault("GameMaster", "DisableAchievements", false);
    gmSettings.listOnlyActiveGms = Config.OptionalConfig.GetBoolDefault("GameMaster", "ListOnlyActiveGMs", false);
    gmSettings.hidePermissions = Config.OptionalConfig.GetBoolDefault("GameMaster", "HidePermissions", false);
    gmSettings.worldAnnounceOnKickPlayer = Config.OptionalConfig.GetBoolDefault("GameMaster", "AnnounceKick", true);

    // optional.conf - Common Schedule Configuration
    broadcastSettings.triggerPercentCap = Config.OptionalConfig.GetIntDefault("CommonSchedule", "BroadCastTriggerPercentCap", 100);
    broadcastSettings.interval = Config.OptionalConfig.GetIntDefault("CommonSchedule", "BroadCastInterval", 1);
    broadcastSettings.isSystemEnabled = Config.OptionalConfig.GetBoolDefault("CommonSchedule", "AutoBroadCast", false);
    broadcastSettings.orderMode = Config.OptionalConfig.GetIntDefault("CommonSchedule", "BroadCastOrderMode", 0);

    if (broadcastSettings.interval < 10)
        broadcastSettings.interval = 10;
    else if (broadcastSettings.interval > 1440)
        broadcastSettings.interval = 1440;

    if (broadcastSettings.triggerPercentCap >= 99)
        broadcastSettings.triggerPercentCap = 98;
    else if (broadcastSettings.triggerPercentCap <= 1)
        broadcastSettings.triggerPercentCap = 0;

    if (broadcastSettings.orderMode < 0)
        broadcastSettings.orderMode = 0;
    else if (broadcastSettings.orderMode > 1)
        broadcastSettings.orderMode = 1;

    // optional.conf - Extra Class Configurations
    extraClassSettings.deathKnightPreReq = Config.OptionalConfig.GetBoolDefault("ClassOptions", "DeathKnightPreReq", false);
    extraClassSettings.deathKnightLimit = Config.OptionalConfig.GetBoolDefault("ClassOptions", "DeathKnightLimit", true);

    // optional.conf - Gold Settings Configuration
    goldSettings.isCapEnabled = Config.OptionalConfig.GetBoolDefault("GoldSettings", "EnableGoldCap", true);
    goldSettings.limitAmount = Config.OptionalConfig.GetIntDefault("GoldSettings", "MaximumGold", 214000);
    if (goldSettings.limitAmount)
        goldSettings.limitAmount *= 10000; // Convert into gsc (gold, silver, copper)
    goldSettings.startAmount = Config.OptionalConfig.GetIntDefault("GoldSettings", "StartingGold", 0);
    if (goldSettings.startAmount)
        goldSettings.startAmount *= 10000;

    // optional.conf - Corpse Decay Settings
    corpseDecaySettings.normalTimeInSeconds = (1000 * (Config.OptionalConfig.GetIntDefault("CorpseDecaySettings", "DecayNormal", 60)));
    corpseDecaySettings.rareTimeInSeconds = (1000 * (Config.OptionalConfig.GetIntDefault("CorpseDecaySettings", "DecayRare", 300)));
    corpseDecaySettings.eliteTimeInSeconds = (1000 * (Config.OptionalConfig.GetIntDefault("CorpseDecaySettings", "DecayElite", 300)));
    corpseDecaySettings.rareEliteTimeInSeconds = (1000 * (Config.OptionalConfig.GetIntDefault("CorpseDecaySettings", "DecayRareElite", 300)));
    corpseDecaySettings.worldbossTimeInSeconds = (1000 * (Config.OptionalConfig.GetIntDefault("CorpseDecaySettings", "DecayWorldboss", 3600)));

    // realms.conf - LogonServer Section
    logonServerSettings2.address = Config.RealmConfig.GetStringDefault("LogonServer", "Address", "127.0.0.1");
    logonServerSettings2.port = Config.RealmConfig.GetIntDefault("LogonServer", "Port", 8093);
    logonServerSettings2.name = Config.RealmConfig.GetStringDefault("LogonServer", "Name", "UnkLogon");
    logonServerSettings2.realmCount = Config.RealmConfig.GetIntDefault("LogonServer", "RealmCount", 1);

    // realms.conf - Realm Section
    // handled in LogonCommHandler::LoadRealmConfiguration()

    if (reload)
        Channel::LoadConfSettings();
}

std::string WorldConfig::getGmClientChannelName()
{
    return gmClientSettings.gmClientChannelName;
}

void WorldConfig::setMessageOfTheDay(std::string motd)
{
    serverSettings.messageOfTheDay = motd;
}

std::string WorldConfig::getMessageOfTheDay()
{
    return serverSettings.messageOfTheDay;
}

void WorldConfig::setFloatRate(WorldConfigRates index, float value)
{
    mFloatRates[index] = value;
}

float WorldConfig::getFloatRate(WorldConfigRates index)
{
    return mFloatRates[index];
}

void WorldConfig::setIntRate(WorldConfigIntRates index, uint32_t value)
{
    mIntRates[index] = value;
}

uint32_t WorldConfig::getIntRate(WorldConfigIntRates index)
{
    return mIntRates[index];
}

uint32_t WorldConfig::getRealmType()
{
    return serverSettings.realmType;
}

