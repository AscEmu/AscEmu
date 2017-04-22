/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "WorldConfig.h"
#include "WorldConf.h"
#include "Server/MainServerDefines.h"
#include "Config/Config.h"
#include "Map/MapCell.h"
#include "Server/WorldSocket.h"
#include "Units/Players/PlayerDefines.hpp"
#include "shared/Log.hpp"


WorldConfig::WorldConfig()
{
    // world.conf - Mysql Database Section
    worldDb.port = 3306;
    worldDb.connections = 3;

    charDb.port = 3306;
    charDb.connections = 5;

    // world.conf - Listen Config
    listen.listenPort = 8129;

    // world.conf - Log Settings
    log.extendedLogsDir = "./";
    log.worldFileLogLevel = 0;
    log.worldDebugFlags = 0;
    log.enableWorldPacketLog = false;
    log.disableCrashdump = false;
    log.enableCheaterLog = false;
    log.enableGmCommandLog = false;
    log.enablePlayerLog = false;
    log.enableTimeStamp = false;
    log.enableSqlBanLog = false;

    // world.conf - Server Settings
    server.playerLimit = 1000;
    server.messageOfTheDay = "AscEmu Default MOTD";
    server.sendStatsOnJoin = true;
    server.enableBreathing = true;
    server.seperateChatChannels = false;
    server.compressionThreshold = 1000;
    server.queueUpdateInterval = 5000;
    server.secondsBeforeKickAFKPlayers = 0;
    server.secondsBeforeTimeOut = 180;
    server.realmType = false;
    server.enableAdjustPriority = false;
    server.requireAllSignatures = false;
    server.showGmInWhoList = true;
    server.mapUnloadTime = MAP_CELL_DEFAULT_UNLOAD_TIME;
    server.mapCellNumber = 1;
    server.enableLimitedNames = true;
    server.useAccountData = false;
    server.requireGmForCommands = false;
    server.enableLfgJoinForNonLfg = false;
    server.gmtTimeZone = 0;
    server.disableFearMovement = 0;
    server.saveExtendedCharData = false;
    server.dataDir = "./";

    // world.conf - Announce Configuration
    announce.enableGmAdminTag = true;
    announce.showNameInAnnounce = false;
    announce.showNameInWAnnounce = false;
    announce.showAnnounceInConsoleOutput = true;

    // world.conf - Power regeneration multiplier setup
    rate.arenaQueueDiff = 150;

    // world.conf - GM Client Channel
    gmClient.gmClientChannelName = "";

    // world.conf - Terrain & Collision Settings
    terrainCollision.unloadMapFiles = false;
    terrainCollision.isCollisionEnabled = false;
    terrainCollision.isPathfindingEnabled = false;

    // world.conf - Mail System Setup
    mail.reloadDelayInSeconds = 0;
    mail.isCostsForGmDisabled = false;
    mail.isCostsForEveryoneDisabled = false;
    mail.isDelayItemsDisabled = false;
    mail.isMessageExpiryDisabled = false;
    mail.isInterfactionMailEnabled = false;
    mail.isInterfactionMailForGmEnabled = false;

    // world.conf - Startup Options
    startup.enableMultithreadedLoading = false;
    startup.enableSpellIdDump = false;

    // world.conf - Flood Protection Setup
    floodProtection.linesBeforeProtection = 0;
    floodProtection.secondsBeforeProtectionReset = 0;
    floodProtection.enableSendFloodProtectionMessage = false;

    // world.conf - LogonServer Setup
    logonServer.disablePings = false;

    // world.conf - AntiHack Setup
    antiHack.isTeleportHackCheckEnabled = false;
    antiHack.isSpeedHackCkeckEnabled = false;
    antiHack.isFallDamageHackCkeckEnabled = false;
    antiHack.isFlyHackCkeckEnabled = false;
    antiHack.flyHackThreshold = 0;
    antiHack.isAntiHackCheckDisabledForGm = true;

    // world.conf - Period Setup
    // world.conf - Channels Setup
    // world.conf - Remote Console Setup
    remoteConsole.isEnabled = false;
    remoteConsole.port = 8092;

    // world.conf - Movement Setup
    movement.compressIntervalInMs = 1000;               // not used by core
    movement.compressRate = 1;                          // not used by core
    movement.compressThresholdCreatures = 15.0f;        // not used by core
    movement.compressThresholdPlayers = 25.0f;          // not used by core

    // world.conf - Localization Setup
    // world.conf - Dungeon / Instance Setup
    instance.useGroupLeaderInstanceId = false;
    instance.isRelativeExpirationEnabled = false;
    instance.relativeDailyHeroicInstanceResetHour = 5;
    instance.checkTriggerPrerequisitesOnEnter = true;

    // world.conf - BattleGround settings
    bg.minPlayerCountAlteracValley = 10;
    bg.maxPlayerCountAlteracValley = 40;
    bg.minPlayerCountArathiBasin = 5;
    bg.maxPlayerCountArathiBasin = 15;
    bg.minPlayerCountWarsongGulch = 5;
    bg.maxPlayerCountWarsongGulch = 10;
    bg.minPlayerCountEyeOfTheStorm = 5;
    bg.maxPlayerCountEyeOfTheStorm = 15;
    bg.minPlayerCountStrandOfTheAncients = 5;
    bg.maxPlayerCountStrandOfTheAncients = 15;
    bg.minPlayerCountIsleOfConquest = 10;
    bg.maxPlayerCountIsleOfConquest = 40;
    bg.firstRbgHonorValueToday = 30;
    bg.firstRbgArenaHonorValueToday = 25;
    bg.honorableKillsRbg = 15;
    bg.honorableArenaWinRbg = 0;
    bg.honorByLosingRbg = 5;
    bg.honorByLosingArenaRbg = 0;

    // world.conf - Arena Settings
    arena.arenaSeason = 8;
    arena.arenaProgress = 1;
    arena.minPlayerCount2V2 = 2;
    arena.maxPlayerCount2V2 = 2;
    arena.maxPlayerCount3V3 = 3;
    arena.minPlayerCount3V3 = 3;
    arena.maxPlayerCount5V5 = 5;
    arena.minPlayerCount5V5 = 5;

    // world.conf - Limits settings
    limit.isLimitSystemEnabled = true;
    limit.maxAutoAttackDamageCap = 10000;
    limit.maxSpellDamageCap = 30000;
    limit.maxHealthCap = 100000;
    limit.maxManaCap = 80000;
    limit.maxHonorPoints = 75000;
    limit.maxArenaPoints = 5000;
    limit.disconnectPlayerForExceedingLimits = false;
    limit.broadcastMessageToGmOnExceeding = true;

    // optional.conf - Optional Settings
    optional.playerStartingLevel = 1;
    optional.playerLevelCap = DBC_PLAYER_LEVEL_CAP;
    optional.playerGeneratedInformationByLevelCap = DBC_PLAYER_LEVEL_CAP;
    optional.allowTbcCharacters = true;
    optional.deactivateMasterLootNinja = false;
    optional.deathKnightStartTalentPoints = 0;
    optional.maxProfessions = 0;
    optional.skipCinematics = false;
    optional.enableInstantLogoutForAccessType = 1;
    optional.minDualSpecLevel = 40;
    optional.minTalentResetLevel = 10;
    optional.showAllVendorItems = false;

    // optional.conf - Inter-faction Options
    interfaction.isInterfactionChatEnabled = false;
    interfaction.isInterfactionGroupEnabled = false;
    interfaction.isInterfactionGuildEnabled = false;
    interfaction.isInterfactionTradeEnabled = false;
    interfaction.isInterfactionFriendsEnabled = false;
    interfaction.isInterfactionMiscEnabled = false;
    interfaction.isCrossoverCharsCreationEnabled = true;

    // optional.conf - Color Configuration
    color.tagColor = 2;
    color.tagGmColor = 1;
    color.nameColor = 4;
    color.msgColor = 6;

    // optional.conf - Game Master Configuration
    gm.isStartOnGmIslandEnabled = true;
    gm.disableAchievements = false;
    gm.listOnlyActiveGms = false;
    gm.hidePermissions = false;
    gm.worldAnnounceOnKickPlayer = true;

    // optional.conf - Common Schedule Configuration
    broadcast.isSystemEnabled = false;
    broadcast.interval = 10;
    broadcast.triggerPercentCap = 2;
    broadcast.orderMode = 0;

    // optional.conf - Extra Class Configurations
    extraClass.deathKnightPreReq = false;
    extraClass.deathKnightLimit = false;

    // optional.conf - Gold Settings Configuration
    gold.isCapEnabled = true;
    gold.limitAmount = 214748;
    gold.startAmount = 0;

    // optional.conf - Corpse Decay Settings
    corpseDecay.normalTimeInSeconds = 60000;
    corpseDecay.rareTimeInSeconds = 300000;
    corpseDecay.eliteTimeInSeconds = 300000;
    corpseDecay.rareEliteTimeInSeconds = 300000;
    corpseDecay.worldbossTimeInSeconds = 3600000;

    // realms.conf - LogonServer Section
    logonServer2.port = 8093;
    logonServer2.realmCount = 1;

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
        if (Config.MainConfig.SetSource(CONFDIR "/world.conf"))
        {
            LogDetail("Config : " CONFDIR "/world.conf reloaded");
        }
        else
        {
            LogError("Config : error occurred loading " CONFDIR "/world.conf");
            return;
        }

        if (Config.OptionalConfig.SetSource(CONFDIR "/optional.conf"))
        {
            LogDetail("Config : " CONFDIR "/optional.conf reloaded");
        }
        else
        {
            LOG_ERROR("Rehash: file optional.conf not available o.O !");
            return;
        }
    }

    // world.conf - Mysql Database Section
    worldDb.host = Config.MainConfig.GetStringDefault("WorldDatabase", "Hostname", "");
    worldDb.user = Config.MainConfig.GetStringDefault("WorldDatabase", "Username", "");
    worldDb.password = Config.MainConfig.GetStringDefault("WorldDatabase", "Password", "");
    worldDb.dbName = Config.MainConfig.GetStringDefault("WorldDatabase", "Name", "");
    worldDb.port = Config.MainConfig.GetIntDefault("WorldDatabase", "Port", 3306);
    worldDb.connections = Config.MainConfig.GetIntDefault("WorldDatabase", "Connections", 3);

    charDb.host = Config.MainConfig.GetStringDefault("CharacterDatabase", "Hostname", "");
    charDb.user = Config.MainConfig.GetStringDefault("CharacterDatabase", "Username", "");
    charDb.password = Config.MainConfig.GetStringDefault("CharacterDatabase", "Password", "");
    charDb.dbName = Config.MainConfig.GetStringDefault("CharacterDatabase", "Name", "");
    charDb.port = Config.MainConfig.GetIntDefault("CharacterDatabase", "Port", 3306);
    charDb.connections = Config.MainConfig.GetIntDefault("CharacterDatabase", "Connections", 5);

    // world.conf - Listen Config
    listen.listenHost = Config.MainConfig.GetStringDefault("Listen", "Host", "0.0.0.0");
    listen.listenPort = Config.MainConfig.GetIntDefault("Listen", "WorldServerPort", 8129);

    // world.conf - Log Settings
    log.worldFileLogLevel = Config.MainConfig.GetIntDefault("Log", "WorldFileLogLevel", 0);
    log.worldDebugFlags = Config.MainConfig.GetIntDefault("Log", "WorldDebugFlags", 0);
    log.enableWorldPacketLog = Config.MainConfig.GetBoolDefault("Log", "EnableWorldPacketLog", false);
    log.disableCrashdump = Config.MainConfig.GetBoolDefault("Log", "DisableCrashdumpReport", false);

    log.extendedLogsDir = Config.MainConfig.GetStringDefault("Log", "ExtendedLogDir", "./");
    if (log.extendedLogsDir.compare("./") != 0)
        log.extendedLogsDir = "./" + log.extendedLogsDir + "/";
    log.enableCheaterLog = Config.MainConfig.GetBoolDefault("Log", "EnableCheaterLog", false);
    log.enableGmCommandLog = Config.MainConfig.GetBoolDefault("Log", "EnableGMCommandLog", false);
    log.enablePlayerLog = Config.MainConfig.GetBoolDefault("Log", "EnablePlayerLog", false);
    log.enableTimeStamp = Config.MainConfig.GetBoolDefault("Log", "EnableTimeStamp", false);
    log.enableSqlBanLog = Config.MainConfig.GetBoolDefault("Log", "EnableSqlBanLog", false);

    // world.conf - Server Settings
    server.playerLimit = Config.MainConfig.GetIntDefault("Server", "PlayerLimit", 1000);
    server.messageOfTheDay = Config.MainConfig.GetStringDefault("Server", "Motd", "AscEmu Default MOTD");
    server.sendStatsOnJoin = Config.MainConfig.GetBoolDefault("Server", "SendStatsOnJoin", true);
    server.enableBreathing = Config.MainConfig.GetBoolDefault("Server", "EnableBreathing", true);
    server.seperateChatChannels = Config.MainConfig.GetBoolDefault("Server", "SeperateChatChannels", false);
    server.compressionThreshold = Config.MainConfig.GetIntDefault("Server", "CompressionThreshold", 1000);
    server.queueUpdateInterval = Config.MainConfig.GetIntDefault("Server", "QueueUpdateInterval", 5000);
    server.secondsBeforeKickAFKPlayers = Config.MainConfig.GetIntDefault("Server", "KickAFKPlayers", 0);
    server.secondsBeforeTimeOut = uint32_t(1000 * Config.MainConfig.GetIntDefault("Server", "ConnectionTimeout", 180));
    server.realmType = Config.MainConfig.GetBoolDefault("Server", "RealmType", false);
    server.enableAdjustPriority = Config.MainConfig.GetBoolDefault("Server", "AdjustPriority", false);
    server.requireAllSignatures = Config.MainConfig.GetBoolDefault("Server", "RequireAllSignatures", false);
    server.showGmInWhoList = Config.MainConfig.GetBoolDefault("Server", "ShowGMInWhoList", true);
    server.mapUnloadTime = Config.MainConfig.GetIntDefault("Server", "MapUnloadTime", MAP_CELL_DEFAULT_UNLOAD_TIME);
    server.mapCellNumber = Config.MainConfig.GetIntDefault("Server", "MapCellNumber", 1);
    server.enableLimitedNames = Config.MainConfig.GetBoolDefault("Server", "LimitedNames", true);
    server.useAccountData = Config.MainConfig.GetBoolDefault("Server", "UseAccountData", false);
    server.requireGmForCommands = !Config.MainConfig.GetBoolDefault("Server", "AllowPlayerCommands", false);
    server.enableLfgJoinForNonLfg = Config.MainConfig.GetBoolDefault("Server", "EnableLFGJoin", false);
    server.gmtTimeZone = Config.MainConfig.GetIntDefault("Server", "TimeZone", 0);
    server.disableFearMovement = Config.MainConfig.GetBoolDefault("Server", "DisableFearMovement", 0);
    server.saveExtendedCharData = Config.MainConfig.GetBoolDefault("Server", "SaveExtendedCharData", false);
    server.dataDir = Config.MainConfig.GetStringDefault("Server", "DataDir", "./");
    if (server.dataDir.compare("./") != 0)
        server.dataDir = "./" + server.dataDir + "/";

    if (server.mapUnloadTime == 0)
    {
        LOG_ERROR("MapUnloadTime is set to 0. This will NEVER unload MapCells!!! Overriding it to default value of %u", MAP_CELL_DEFAULT_UNLOAD_TIME);
        server.mapUnloadTime = MAP_CELL_DEFAULT_UNLOAD_TIME;
    }

    if (server.mapCellNumber == 0)
    {
        LOG_ERROR("MapCellNumber is set to 0. Congrats, no MapCells will be loaded. Overriding it to default value of 1");
        server.mapCellNumber = 1;
    }

#ifdef WIN32
    DWORD current_priority_class = GetPriorityClass(GetCurrentProcess());
    bool high = server.enableAdjustPriority;

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

    // world.conf - Announce Configuration
    announce.announceTag = Config.MainConfig.GetStringDefault("Announce", "Tag", "Staff");
    announce.enableGmAdminTag = Config.MainConfig.GetBoolDefault("Announce", "GMAdminTag", false);
    announce.showNameInAnnounce = Config.MainConfig.GetBoolDefault("Announce", "NameinAnnounce", true);
    announce.showNameInWAnnounce = Config.MainConfig.GetBoolDefault("Announce", "NameinWAnnounce", true);
    announce.showAnnounceInConsoleOutput = Config.MainConfig.GetBoolDefault("Announce", "ShowInConsole", true);

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
    rate.arenaQueueDiff = Config.MainConfig.GetIntDefault("Rates", "ArenaQueueDiff", 150);
    setFloatRate(RATE_ARENAPOINTMULTIPLIER2X, Config.MainConfig.GetFloatDefault("Rates", "ArenaMultiplier2x", 1.0f));
    setFloatRate(RATE_ARENAPOINTMULTIPLIER3X, Config.MainConfig.GetFloatDefault("Rates", "ArenaMultiplier3x", 1.0f));
    setFloatRate(RATE_ARENAPOINTMULTIPLIER5X, Config.MainConfig.GetFloatDefault("Rates", "ArenaMultiplier5x", 1.0f));

    // world.conf - GM Client Channel
    gmClient.gmClientChannelName = Config.MainConfig.GetStringDefault("GMClient", "GmClientChannel", "");

    // world.conf - Terrain & Collision Settings
    terrainCollision.unloadMapFiles = Config.MainConfig.GetBoolDefault("Terrain", "UnloadMapFiles", true);
    terrainCollision.isCollisionEnabled = Config.MainConfig.GetBoolDefault("Terrain", "Collision", false);
    terrainCollision.isPathfindingEnabled = Config.MainConfig.GetBoolDefault("Terrain", "Pathfinding", false);

    // world.conf - Mail System Setup
    mail.reloadDelayInSeconds = Config.MainConfig.GetIntDefault("Mail", "ReloadDelay", 0);
    mail.isCostsForGmDisabled = Config.MainConfig.GetBoolDefault("Mail", "DisablePostageCostsForGM", true);
    mail.isCostsForEveryoneDisabled = Config.MainConfig.GetBoolDefault("Mail", "DisablePostageCosts", false);
    mail.isDelayItemsDisabled = Config.MainConfig.GetBoolDefault("Mail", "DisablePostageDelayItems", true);
    mail.isMessageExpiryDisabled = Config.MainConfig.GetBoolDefault("Mail", "DisableMessageExpiry", false);
    mail.isInterfactionMailEnabled = Config.MainConfig.GetBoolDefault("Mail", "EnableInterfactionMail", true);
    mail.isInterfactionMailForGmEnabled = Config.MainConfig.GetBoolDefault("Mail", "EnableInterfactionForGM", true);

    // world.conf - Startup Options
    //startupSettings.Preloading;                    // not used
    //startupSettings.BackgroundLootLoading;         // not used, not in config
    startup.enableMultithreadedLoading = Config.MainConfig.GetBoolDefault("Startup", "EnableMultithreadedLoading", true);
    startup.enableSpellIdDump = Config.MainConfig.GetBoolDefault("Startup", "EnableSpellIDDump", false);
    startup.additionalTableLoads = Config.MainConfig.GetStringDefault("Startup", "LoadAdditionalTables", "");

    // world.conf - Flood Protection Setup
    floodProtection.linesBeforeProtection = Config.MainConfig.GetIntDefault("FloodProtection", "Lines", 0);
    floodProtection.secondsBeforeProtectionReset = Config.MainConfig.GetIntDefault("FloodProtection", "Seconds", 0);
    floodProtection.enableSendFloodProtectionMessage = Config.MainConfig.GetBoolDefault("FloodProtection", "SendMessage", false);

    if (!floodProtection.linesBeforeProtection || !floodProtection.secondsBeforeProtectionReset)
        floodProtection.linesBeforeProtection = floodProtection.secondsBeforeProtectionReset = 0;

    // world.conf - LogonServer Setup
    logonServer.disablePings = Config.MainConfig.GetBoolDefault("LogonServer", "DisablePings", false);
    logonServer.remotePassword = Config.MainConfig.GetStringDefault("LogonServer", "RemotePassword", "r3m0t3");

    // world.conf - AntiHack Setup
    antiHack.isTeleportHackCheckEnabled = Config.MainConfig.GetBoolDefault("AntiHack", "Teleport", true);
    antiHack.isSpeedHackCkeckEnabled = Config.MainConfig.GetBoolDefault("AntiHack", "Speed", true);
    antiHack.isFallDamageHackCkeckEnabled = Config.MainConfig.GetBoolDefault("AntiHack", "FallDamage", true);
    antiHack.isFlyHackCkeckEnabled = Config.MainConfig.GetBoolDefault("AntiHack", "Flight", true);
    antiHack.flyHackThreshold = Config.MainConfig.GetIntDefault("AntiHack", "FlightThreshold", 8);
    antiHack.isAntiHackCheckDisabledForGm = Config.MainConfig.GetBoolDefault("AntiHack", "DisableOnGM", true);

    // world.conf - Period Setup
    period.arenaUpdate = Config.MainConfig.GetStringDefault("Periods", "ArenaUpdate", "weekly");
    period.dailyUpdate = Config.MainConfig.GetStringDefault("Periods", "DailyUpdate", "daily");

    // world.conf - Channels Setup
    channel.bannedChannels = Config.MainConfig.GetStringDefault("Channels", "BannedChannels", "");
    channel.minimumTalkLevel = Config.MainConfig.GetStringDefault("Channels", "MinimumLevel", "");

    // world.conf - Remote Console Setup
    remoteConsole.isEnabled = Config.MainConfig.GetBoolDefault("RemoteConsole", "Enabled", false);
    remoteConsole.host = Config.MainConfig.GetStringDefault("RemoteConsole", "Host", "0.0.0.0");
    remoteConsole.port = Config.MainConfig.GetIntDefault("RemoteConsole", "Port", 8092);

    // world.conf - Movement Setup
    movement.compressIntervalInMs = Config.MainConfig.GetIntDefault("Movement", "FlushInterval", 1000);
    movement.compressRate = Config.MainConfig.GetIntDefault("Movement", "CompressRate", 1);

    movement.compressThresholdCreatures = Config.MainConfig.GetFloatDefault("Movement", "CompressThresholdCreatures", 15.0f);
    movement.compressThresholdCreatures *= movement.compressThresholdCreatures;

    movement.compressThresholdPlayers = Config.MainConfig.GetFloatDefault("Movement", "CompressThreshold", 25.0f);
    movement.compressThresholdPlayers *= movement.compressThresholdPlayers;

    // world.conf - Localization Setup
    localization.localizedBindings = Config.MainConfig.GetStringDefault("Localization", "LocaleBindings", "");

    // world.conf - Dungeon / Instance Setup
    instance.useGroupLeaderInstanceId = Config.MainConfig.GetBoolDefault("InstanceHandling", "TakeGroupLeaderID", true);
    instance.isRelativeExpirationEnabled = Config.MainConfig.GetBoolDefault("InstanceHandling", "SlidingExpiration", false);
    instance.relativeDailyHeroicInstanceResetHour = Config.MainConfig.GetIntDefault("InstanceHandling", "DailyHeroicInstanceResetHour", 5);
    instance.checkTriggerPrerequisitesOnEnter = Config.MainConfig.GetBoolDefault("InstanceHandling", "CheckTriggerPrerequisites", true);

    if (instance.relativeDailyHeroicInstanceResetHour < 0)
        instance.relativeDailyHeroicInstanceResetHour = 0;
    if (instance.relativeDailyHeroicInstanceResetHour > 23)
        instance.relativeDailyHeroicInstanceResetHour = 23;

    // world.conf - BattleGround settings
    bg.minPlayerCountAlteracValley = Config.MainConfig.GetIntDefault("Battleground", "AV_MIN", 10);
    bg.maxPlayerCountAlteracValley = Config.MainConfig.GetIntDefault("Battleground", "AV_MAX", 40);
    bg.minPlayerCountArathiBasin = Config.MainConfig.GetIntDefault("Battleground", "AB_MIN", 4);
    bg.maxPlayerCountArathiBasin = Config.MainConfig.GetIntDefault("Battleground", "AB_MAX", 15);
    bg.minPlayerCountWarsongGulch = Config.MainConfig.GetIntDefault("Battleground", "WSG_MIN", 2);
    bg.maxPlayerCountWarsongGulch = Config.MainConfig.GetIntDefault("Battleground", "WSG_MAX", 10);
    bg.minPlayerCountEyeOfTheStorm = Config.MainConfig.GetIntDefault("Battleground", "EOTS_MIN", 4);
    bg.maxPlayerCountEyeOfTheStorm = Config.MainConfig.GetIntDefault("Battleground", "EOTS_MAX", 15);
    bg.minPlayerCountStrandOfTheAncients = Config.MainConfig.GetIntDefault("Battleground", "SOTA_MIN", 10);
    bg.maxPlayerCountStrandOfTheAncients = Config.MainConfig.GetIntDefault("Battleground", "SOTA_MAX", 15);
    bg.minPlayerCountIsleOfConquest = Config.MainConfig.GetIntDefault("Battleground", "IOC_MIN", 10);
    bg.maxPlayerCountIsleOfConquest = Config.MainConfig.GetIntDefault("Battleground", "IOC_MAX", 15);
    bg.firstRbgHonorValueToday = Config.MainConfig.GetIntDefault("Battleground", "RBG_FIRST_WIN_HONOR", 30);
    bg.firstRbgArenaHonorValueToday = Config.MainConfig.GetIntDefault("Battleground", "RBG_FIRST_WIN_ARENA", 25);
    bg.honorableKillsRbg = Config.MainConfig.GetIntDefault("Battleground", "RBG_WIN_HONOR", 15);
    bg.honorableArenaWinRbg = Config.MainConfig.GetIntDefault("Battleground", "RBG_WIN_ARENA", 0);
    bg.honorByLosingRbg = Config.MainConfig.GetIntDefault("Battleground", "RBG_LOSE_HONOR", 5);
    bg.honorByLosingArenaRbg = Config.MainConfig.GetIntDefault("Battleground", "RBG_LOSE_ARENA", 0);

    // world.conf - Arena Settings
    arena.arenaSeason = Config.MainConfig.GetIntDefault("Arena", "Season", 1);
    arena.arenaProgress = Config.MainConfig.GetIntDefault("Arena", "Progress", 1);
    arena.minPlayerCount2V2 = Config.MainConfig.GetIntDefault("Arena", "2V2_MIN", 2);
    arena.maxPlayerCount2V2 = Config.MainConfig.GetIntDefault("Arena", "2V2_MAX", 2);
    arena.minPlayerCount3V3 = Config.MainConfig.GetIntDefault("Arena", "3V3_MIN", 3);
    arena.maxPlayerCount3V3 = Config.MainConfig.GetIntDefault("Arena", "3V3_MAX", 3);
    arena.minPlayerCount5V5 = Config.MainConfig.GetIntDefault("Arena", "5V5_MIN", 5);
    arena.maxPlayerCount5V5 = Config.MainConfig.GetIntDefault("Arena", "5V5_MAX", 5);

    // world.conf - Limits settings
    limit.isLimitSystemEnabled = Config.MainConfig.GetBoolDefault("Limits", "Enable", true);
    limit.maxAutoAttackDamageCap = (uint32_t)Config.MainConfig.GetIntDefault("Limits", "AutoAttackDmg", 10000);
    limit.maxSpellDamageCap = (uint32_t)Config.MainConfig.GetIntDefault("Limits", "SpellDmg", 30000);
    limit.maxHealthCap = (uint32_t)Config.MainConfig.GetIntDefault("Limits", "Health", 80000);
    limit.maxManaCap = (uint32_t)Config.MainConfig.GetIntDefault("Limits", "Mana", 80000);
    limit.maxHonorPoints = (uint32_t)Config.MainConfig.GetIntDefault("Limits", "Honor", 75000);
    limit.maxArenaPoints = (uint32_t)Config.MainConfig.GetIntDefault("Limits", "Arena", 5000);
    limit.disconnectPlayerForExceedingLimits = Config.MainConfig.GetBoolDefault("Limits", "Disconnect", false);
    limit.broadcastMessageToGmOnExceeding = Config.MainConfig.GetBoolDefault("Limits", "BroadcastGMs", true);

    // optional.conf - Optional Settings
    optional.playerStartingLevel = Config.OptionalConfig.GetIntDefault("Optional", "StartingLevel", 1);
    optional.playerLevelCap = Config.OptionalConfig.GetIntDefault("Optional", "LevelCap", DBC_PLAYER_LEVEL_CAP);
    optional.playerGeneratedInformationByLevelCap = Config.OptionalConfig.GetIntDefault("Optional", "GenLevelCap", DBC_PLAYER_LEVEL_CAP); //! no delete
    if (optional.playerStartingLevel > static_cast<int32_t>(optional.playerLevelCap))
        optional.playerStartingLevel = static_cast<int32_t>(optional.playerLevelCap);

    optional.allowTbcCharacters = Config.OptionalConfig.GetBoolDefault("Optional", "AllowTBC", true);
    optional.deactivateMasterLootNinja = Config.OptionalConfig.GetBoolDefault("Optional", "AntiMasterLootNinja", false);
    optional.deathKnightStartTalentPoints = Config.OptionalConfig.GetIntDefault("Optional", "DKStartingTalents", 0);
    optional.maxProfessions = (uint32_t)Config.OptionalConfig.GetIntDefault("Optional", "MaxProfessions", 2);
    optional.skipCinematics = Config.OptionalConfig.GetBoolDefault("Optional", "SkipCinematic", false);
    optional.enableInstantLogoutForAccessType = Config.OptionalConfig.GetIntDefault("Optional", "InstantLogout", 1);
    optional.minDualSpecLevel = Config.OptionalConfig.GetIntDefault("Optional", "MinDualSpecLevel", 40);
    optional.minTalentResetLevel = Config.OptionalConfig.GetIntDefault("Optional", "MinTalentResetLevel", 10);
    optional.showAllVendorItems = Config.OptionalConfig.GetBoolDefault("Optional", "ShowAllVendorItems", false);

    // optional.conf - Inter-faction Options
    interfaction.isInterfactionChatEnabled = Config.OptionalConfig.GetBoolDefault("Interfaction", "InterfactionChat", false);
    interfaction.isInterfactionGroupEnabled = Config.OptionalConfig.GetBoolDefault("Interfaction", "InterfactionGroup", false);
    interfaction.isInterfactionGuildEnabled = Config.OptionalConfig.GetBoolDefault("Interfaction", "InterfactionGuild", false);
    interfaction.isInterfactionTradeEnabled = Config.OptionalConfig.GetBoolDefault("Interfaction", "InterfactionTrade", false);
    interfaction.isInterfactionFriendsEnabled = Config.OptionalConfig.GetBoolDefault("Interfaction", "InterfactionFriends", false);
    interfaction.isInterfactionMiscEnabled = Config.OptionalConfig.GetBoolDefault("Interfaction", "InterfactionMisc", false);
    interfaction.isCrossoverCharsCreationEnabled = Config.OptionalConfig.GetBoolDefault("Interfaction", "CrossOverCharacters", false);

    // optional.conf - Color Configuration
    color.tagColor = Config.OptionalConfig.GetIntDefault("Color", "AnnTagColor", 2);
    color.tagGmColor = Config.OptionalConfig.GetIntDefault("Color", "AnnGMTagColor", 1);
    color.nameColor = Config.OptionalConfig.GetIntDefault("Color", "AnnNameColor", 4);
    color.msgColor = Config.OptionalConfig.GetIntDefault("Color", "AnnMsgColor", 10);

    // optional.conf - Game Master Configuration
    gm.isStartOnGmIslandEnabled = Config.OptionalConfig.GetBoolDefault("GameMaster", "StartOnGMIsland", false);
    gm.disableAchievements = Config.OptionalConfig.GetBoolDefault("GameMaster", "DisableAchievements", false);
    gm.listOnlyActiveGms = Config.OptionalConfig.GetBoolDefault("GameMaster", "ListOnlyActiveGMs", false);
    gm.hidePermissions = Config.OptionalConfig.GetBoolDefault("GameMaster", "HidePermissions", false);
    gm.worldAnnounceOnKickPlayer = Config.OptionalConfig.GetBoolDefault("GameMaster", "AnnounceKick", true);

    // optional.conf - Common Schedule Configuration
    broadcast.triggerPercentCap = Config.OptionalConfig.GetIntDefault("CommonSchedule", "BroadCastTriggerPercentCap", 100);
    broadcast.interval = Config.OptionalConfig.GetIntDefault("CommonSchedule", "BroadCastInterval", 1);
    broadcast.isSystemEnabled = Config.OptionalConfig.GetBoolDefault("CommonSchedule", "AutoBroadCast", false);
    broadcast.orderMode = Config.OptionalConfig.GetIntDefault("CommonSchedule", "BroadCastOrderMode", 0);

    if (broadcast.interval < 10)
        broadcast.interval = 10;
    else if (broadcast.interval > 1440)
        broadcast.interval = 1440;

    if (broadcast.triggerPercentCap >= 99)
        broadcast.triggerPercentCap = 98;
    else if (broadcast.triggerPercentCap <= 1)
        broadcast.triggerPercentCap = 0;

    if (broadcast.orderMode < 0)
        broadcast.orderMode = 0;
    else if (broadcast.orderMode > 1)
        broadcast.orderMode = 1;

    // optional.conf - Extra Class Configurations
    extraClass.deathKnightPreReq = Config.OptionalConfig.GetBoolDefault("ClassOptions", "DeathKnightPreReq", false);
    extraClass.deathKnightLimit = Config.OptionalConfig.GetBoolDefault("ClassOptions", "DeathKnightLimit", true);

    // optional.conf - Gold Settings Configuration
    gold.isCapEnabled = Config.OptionalConfig.GetBoolDefault("GoldSettings", "EnableGoldCap", true);
    gold.limitAmount = Config.OptionalConfig.GetIntDefault("GoldSettings", "MaximumGold", 214000);
    if (gold.limitAmount)
        gold.limitAmount *= 10000;

    gold.startAmount = Config.OptionalConfig.GetIntDefault("GoldSettings", "StartingGold", 0);
    if (gold.startAmount)
        gold.startAmount *= 10000;

    // optional.conf - Corpse Decay Settings
    corpseDecay.normalTimeInSeconds = (1000 * (Config.OptionalConfig.GetIntDefault("CorpseDecaySettings", "DecayNormal", 60)));
    corpseDecay.rareTimeInSeconds = (1000 * (Config.OptionalConfig.GetIntDefault("CorpseDecaySettings", "DecayRare", 300)));
    corpseDecay.eliteTimeInSeconds = (1000 * (Config.OptionalConfig.GetIntDefault("CorpseDecaySettings", "DecayElite", 300)));
    corpseDecay.rareEliteTimeInSeconds = (1000 * (Config.OptionalConfig.GetIntDefault("CorpseDecaySettings", "DecayRareElite", 300)));
    corpseDecay.worldbossTimeInSeconds = (1000 * (Config.OptionalConfig.GetIntDefault("CorpseDecaySettings", "DecayWorldboss", 3600)));

    // realms.conf - LogonServer Section
    logonServer2.address = Config.RealmConfig.GetStringDefault("LogonServer", "Address", "127.0.0.1");
    logonServer2.port = Config.RealmConfig.GetIntDefault("LogonServer", "Port", 8093);
    logonServer2.name = Config.RealmConfig.GetStringDefault("LogonServer", "Name", "UnkLogon");
    logonServer2.realmCount = Config.RealmConfig.GetIntDefault("LogonServer", "RealmCount", 1);

    // realms.conf - Realm Section
    // handled in LogonCommHandler::LoadRealmConfiguration()
}


uint32_t WorldConfig::getRealmType() { return server.realmType; }
uint32_t WorldConfig::getPlayerLimit() { return server.playerLimit; }
uint32_t WorldConfig::getKickAFKPlayerTime() { return server.secondsBeforeKickAFKPlayers; }

void WorldConfig::setMessageOfTheDay(std::string motd) { server.messageOfTheDay = motd; }
std::string WorldConfig::getMessageOfTheDay() { return server.messageOfTheDay; }

void WorldConfig::setFloatRate(uint32_t index, float value) { mFloatRates[index] = value; }
float WorldConfig::getFloatRate(uint32_t index) { return mFloatRates[index]; }

void WorldConfig::setIntRate(uint32_t index, uint32_t value) { mIntRates[index] = value; }
uint32_t WorldConfig::getIntRate(uint32_t index) { return mIntRates[index]; }

std::string WorldConfig::getGmClientChannelName() { return gmClient.gmClientChannelName; }

std::string WorldConfig::getColorStringForNumber(int color)
{
    switch (color)
    {
        case 1:
            return "|cffff6060"; //lightred
        case 2:
            return "|cff00ccff"; //lightblue
        case 3:
            return "|cff0000ff"; //blue
        case 4:
            return "|cff00ff00"; //green
        case 5:
            return "|cffff0000"; //red
        case 6:
            return "|cffffcc00"; //gold
        case 7:
            return "|cff888888"; //grey
        case 8:
            return "|cffffffff"; //white
        case 9:
            return "|cffff00ff"; //magenta
        case 10:
            return "|cffffff00"; //yellow
        default:
            return "|cffffffff"; //white
    }
}

