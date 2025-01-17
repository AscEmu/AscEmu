/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "WorldConfig.h"
#include "AEVersion.hpp"
#include "ConfigMgr.hpp"
#include "WorldConf.h"
#include "Config/Config.h"
#include "Map/Cells/MapCell.hpp"
#include "Logging/Logger.hpp"
#include "Macros/MapsMacros.hpp"
#include "Macros/PlayerMacros.hpp"

#include <utility>

WorldConfig::WorldConfig(): mFloatRates{}, mIntRates{}
{
    // world.conf - Mysql Database Section
    worldDb.port = 3306;
    worldDb.connections = 3;
    worldDb.isLegacyAuth = false;

    charDb.port = 3306;
    charDb.connections = 5;
    charDb.isLegacyAuth = false;

    // world.conf - LogonServer Settings
    logonServer.address = "127.0.0.1";
    logonServer.port = 8093;
    logonServer.name = "Default Logon";
    logonServer.realmCount = 1;
    logonServer.disablePings = false;

    // world.conf - Listen Config
    listen.listenPort = 8129;

    // world.conf - Logger Settings
    logger.extendedLogsDir = "./";
    logger.minimumMessageType = 2;
    logger.enableWorldPacketLog = false;
    logger.enableCheaterLog = false;
    logger.enableGmCommandLog = false;
    logger.enablePlayerLog = false;
    logger.enableTimeStamp = false;
    logger.enableSqlBanLog = false;

    // world.conf - Server Settings
    server.playerLimit = 100;
    server.messageOfTheDay = "Welcome to the World of Warcraft!";
    server.sendStatsOnJoin = true;
    server.gmtTimeZone = 0;
    server.compressionThreshold = 1000;
    server.enableAdjustPriority = false;
    server.mapUnloadTime = MAP_CELL_DEFAULT_UNLOAD_TIME;
    server.mapCellNumber = 1;
    server.secondsBeforeKickAFKPlayers = 0;
    server.queueUpdateInterval = 5000;
    server.enableBreathing = true;
    server.enableLimitedNames = true;
    server.useAccountData = false;
    server.requireGmForCommands = false;
    server.saveExtendedCharData = false;
    server.dataDir = "";

    // world.conf - Player Settings
    player.playerStartingLevel = 1;
    player.playerLevelCap = DBC_PLAYER_LEVEL_CAP;
    player.playerGeneratedInformationByLevelCap = DBC_PLAYER_LEVEL_CAP;
    player.deathKnightStartTalentPoints = 0;
    player.deathKnightPreReq = false;
    player.deathKnightLimit = false;
    player.maxProfessions = 0;
    player.skipCinematics = false;
    player.enableInstantLogoutForAccessType = 1;
    player.minDualSpecLevel = 40;
    player.minTalentResetLevel = 10;
    player.showAllVendorItems = false;
    player.isInterfactionChatEnabled = false;
    player.isInterfactionChannelEnabled = false;
    player.isInterfactionGroupEnabled = false;
    player.isInterfactionGuildEnabled = false;
    player.isInterfactionTradeEnabled = false;
    player.isInterfactionFriendsEnabled = false;
    player.isInterfactionMailEnabled = false;
    player.isInterfactionMiscEnabled = false;
    player.isCrossoverCharsCreationEnabled = false;
    player.isGoldCapEnabled = true;
    player.limitGoldAmount = 214748;
    player.startGoldAmount = 0;
    player.deactivateMasterLootNinja = false;
    player.enablePvPToken = false;
    player.pvpTokenId = 0;

    // world.conf - Guild Settings
    guild.charterCost = 1000;
    guild.requireAllSignatures = true;
    guild.maxLevel = 25;
    guild.maxMembers = 0;
    guild.maxXpPerDay = 0;
    guild.maxRepPerWeek = 0;
    guild.levelingEnabled = false;
    guild.undeletableLevel = 0;
    guild.eventLogCount = 0;
    guild.newsLogCount = 0;
    guild.bankLogCount = 0;
    guild.saveInterval = 300;

    // world.conf - Announce Settings
    announce.enableGmAdminTag = true;
    announce.showNameInAnnounce = false;
    announce.showNameInWAnnounce = false;
    announce.showAnnounceInConsoleOutput = true;
    announce.tagColor = 2;
    announce.tagGmColor = 1;
    announce.nameColor = 4;
    announce.msgColor = 6;

    // world.conf - GameMaster Settings
    gm.isStartOnGmIslandEnabled = true;
    gm.disableAchievements = false;
    gm.listOnlyActiveGms = false;
    gm.hidePermissions = false;
    gm.worldAnnounceOnKickPlayer = true;
    gm.showGmInWhoList = true;
    gm.gmClientChannelName = "";

    // world.conf - Broadcast Settings
    broadcast.isSystemEnabled = false;

    // world.conf - Rate Settings
    rate.arenaQueueDiff = 150;

    // world.conf - Corpse Decay Settings
    corpseDecay.normalTimeInSeconds = 3000;
    corpseDecay.rareTimeInSeconds = 900;
    corpseDecay.eliteTimeInSeconds = 300;
    corpseDecay.rareEliteTimeInSeconds = 900;
    corpseDecay.worldbossTimeInSeconds = 3600;

    // world.conf - Terrain & Collision Settings
    terrainCollision.isCollisionEnabled = false;
    terrainCollision.isPathfindingEnabled = false;

    // world.conf - Mail Settings
    mail.isCostsForGmDisabled = false;
    mail.isCostsForEveryoneDisabled = false;
    mail.isDelayItemsDisabled = false;
    mail.isMessageExpiryDisabled = false;
    mail.isInterfactionMailForGmEnabled = false;

    // world.conf - Startup Options
    startup.enableMultithreadedLoading = false;
    startup.enableSpellIdDump = false;

    // world.conf - AntiHack Setup
    antiHack.isTeleportHackCheckEnabled = false;
    antiHack.isSpeedHackCkeckEnabled = false;
    antiHack.isAntiHackCheckDisabledForGm = true;

    // world.conf - Period Setup
    // world.conf - Chat Setttings
    chat.linesBeforeProtection = 0;
    chat.secondsBeforeProtectionReset = 0;
    chat.enableSendFloodProtectionMessage = false;

    // world.conf - Remote Console Setup
    remoteConsole.isEnabled = false;
    remoteConsole.port = 8092;

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
    arena.charterCost2v2 = 800000;
    arena.charterCost3v3 = 1200000;
    arena.charterCost5v5 = 2000000;
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
}

WorldConfig::~WorldConfig() = default;

//////////////////////////////////////////////////////////////////////////////////////////
// Config functions
void WorldConfig::loadWorldConfigValues(bool reload /*false*/)
{
    if (reload)
    {
        // This will only happen if someone deleted/renamed the conf after the server started...
        if (Config.MainConfig.openAndLoadConfigFile(CONFDIR "/world.conf"))
        {
            sLogger.info("Config : " CONFDIR "/world.conf reloaded");
        }
        else
        {
            sLogger.failure("Config : error occurred loading " CONFDIR "/world.conf");
            return;
        }
    }

    // world.conf - Mysql Database Section
    Config.MainConfig.tryGetString("WorldDatabase", "Hostname", &worldDb.host);
    Config.MainConfig.tryGetString("WorldDatabase", "Username", &worldDb.user);
    Config.MainConfig.tryGetString("WorldDatabase", "Password", &worldDb.password);
    Config.MainConfig.tryGetString("WorldDatabase", "Name", &worldDb.dbName);
    Config.MainConfig.tryGetInt("WorldDatabase", "Port", &worldDb.port);
    Config.MainConfig.tryGetInt("WorldDatabase", "Connections", &worldDb.connections);
    Config.MainConfig.tryGetBool("WorldDatabase", "LegacyAuth", &worldDb.isLegacyAuth);

    Config.MainConfig.tryGetString("CharacterDatabase", "Hostname", &charDb.host);
    Config.MainConfig.tryGetString("CharacterDatabase", "Username", &charDb.user);
    Config.MainConfig.tryGetString("CharacterDatabase", "Password", &charDb.password);
    Config.MainConfig.tryGetString("CharacterDatabase", "Name", &charDb.dbName);
    Config.MainConfig.tryGetInt("CharacterDatabase", "Port", &charDb.port);
    Config.MainConfig.tryGetInt("CharacterDatabase", "Connections", &charDb.connections);
    Config.MainConfig.tryGetBool("CharacterDatabase", "LegacyAuth", &charDb.isLegacyAuth);

    // world.conf - LogonServer Settings
    Config.MainConfig.tryGetString("LogonServer", "Address", &logonServer.address);
    Config.MainConfig.tryGetInt("LogonServer", "Port", &logonServer.port);
    Config.MainConfig.tryGetString("LogonServer", "Name", &logonServer.name);
    Config.MainConfig.tryGetInt("LogonServer", "RealmCount", &logonServer.realmCount);
    Config.MainConfig.tryGetBool("LogonServer", "DisablePings", &logonServer.disablePings);
    Config.MainConfig.tryGetString("LogonServer", "RemotePassword", &logonServer.remotePassword);

    // world.conf - Realm Section

    // world.conf - Listen Config
    Config.MainConfig.tryGetString("Listen", "Host", &listen.listenHost);
    Config.MainConfig.tryGetInt("Listen", "WorldServerPort", &listen.listenPort);

    // world.conf - Logger Settings
    Config.MainConfig.tryGetInt("Logger", "MinimumMessageType", &logger.minimumMessageType);
    Config.MainConfig.tryGetInt("Logger", "DebugFlags", &logger.debugFlags);
    Config.MainConfig.tryGetBool("Logger", "EnableWorldPacketLog", &logger.enableWorldPacketLog);

    Config.MainConfig.tryGetString("Logger", "ExtendedLogDir", &logger.extendedLogsDir);
    if (logger.extendedLogsDir != "./")
        logger.extendedLogsDir = "./" + logger.extendedLogsDir + "/";

    Config.MainConfig.tryGetBool("Logger", "EnableCheaterLog", &logger.enableCheaterLog);
    Config.MainConfig.tryGetBool("Logger", "EnableGMCommandLog", &logger.enableGmCommandLog);
    Config.MainConfig.tryGetBool("Logger", "EnablePlayerLog", &logger.enablePlayerLog);
    Config.MainConfig.tryGetBool("Logger", "EnableTimeStamp", &logger.enableTimeStamp);
    Config.MainConfig.tryGetBool("Logger", "EnableSqlBanLog", &logger.enableSqlBanLog);

    // world.conf - Server Settings
    Config.MainConfig.tryGetInt("Server", "PlayerLimit", &server.playerLimit);
    Config.MainConfig.tryGetString("Server", "Motd", &server.messageOfTheDay);
    Config.MainConfig.tryGetBool("Server", "SendStatsOnJoin", &server.sendStatsOnJoin);
    Config.MainConfig.tryGetInt("Server", "TimeZone", &server.gmtTimeZone);
    Config.MainConfig.tryGetInt("Server", "CompressionThreshold", &server.compressionThreshold);
    Config.MainConfig.tryGetBool("Server", "AdjustPriority", &server.enableAdjustPriority);
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
    Config.MainConfig.tryGetInt("Server", "MapUnloadTime", &server.mapUnloadTime);
    if (server.mapUnloadTime == 0)
    {
        sLogger.failure("MapUnloadTime is set to 0. This will NEVER unload MapCells!!! Overriding it to default value of {}", MAP_CELL_DEFAULT_UNLOAD_TIME);
        server.mapUnloadTime = MAP_CELL_DEFAULT_UNLOAD_TIME;
    }
    Config.MainConfig.tryGetInt("Server", "MapCellNumber", &server.mapCellNumber);
    if (server.mapCellNumber == 0)
    {
        sLogger.failure("MapCellNumber is set to 0. Congrats, no MapCells will be loaded. Overriding it to default value of 1");
        server.mapCellNumber = 1;
    }
    Config.MainConfig.tryGetInt("Server", "KickAFKPlayers", &server.secondsBeforeKickAFKPlayers);
    server.secondsBeforeKickAFKPlayers *= 1000;
    Config.MainConfig.tryGetInt("Server", "QueueUpdateInterval", &server.queueUpdateInterval);
    Config.MainConfig.tryGetBool("Server", "EnableBreathing", &server.enableBreathing);
    Config.MainConfig.tryGetBool("Server", "LimitedNames", &server.enableLimitedNames);
    Config.MainConfig.tryGetBool("Server", "UseAccountData", &server.useAccountData);
    Config.MainConfig.tryGetBool("Server", "AllowPlayerCommands", &server.requireGmForCommands);
    Config.MainConfig.tryGetBool("Server", "SaveExtendedCharData", &server.saveExtendedCharData);
    Config.MainConfig.tryGetString("Server", "DataDir", &server.dataDir);
    if (server.dataDir == "")
        server.dataDir = "./";
    else if (server.dataDir != "./")
        server.dataDir = "./" + server.dataDir + "/";

    // world.conf - Player Settings
    Config.MainConfig.tryGetInt("Player", "StartingLevel", &player.playerStartingLevel);
    Config.MainConfig.tryGetInt("Player", "LevelCap", &player.playerLevelCap);
    Config.MainConfig.tryGetInt("Player", "GenLevelCap", &player.playerGeneratedInformationByLevelCap); //! no delete
    if (player.playerStartingLevel > static_cast<int32_t>(player.playerLevelCap))
        player.playerStartingLevel = static_cast<int32_t>(player.playerLevelCap);
    Config.MainConfig.tryGetInt("Player", "DKStartingTalents", &player.deathKnightStartTalentPoints);
    Config.MainConfig.tryGetBool("Player", "DKPreReq", &player.deathKnightPreReq);
    Config.MainConfig.tryGetBool("Player", "DKLimit", &player.deathKnightLimit);
    Config.MainConfig.tryGetInt("Player", "MaxProfessions", &player.maxProfessions);
    Config.MainConfig.tryGetBool("Player", "SkipCinematic", &player.skipCinematics);
    Config.MainConfig.tryGetInt("Player", "InstantLogout", &player.enableInstantLogoutForAccessType);
    Config.MainConfig.tryGetInt("Player", "MinDualSpecLevel", &player.minDualSpecLevel);
    Config.MainConfig.tryGetInt("Player", "MinTalentResetLevel", &player.minTalentResetLevel);
    Config.MainConfig.tryGetBool("Player", "ShowAllVendorItems", &player.showAllVendorItems);
    Config.MainConfig.tryGetBool("Player", "InterfactionChat", &player.isInterfactionChatEnabled);
    Config.MainConfig.tryGetBool("Player", "InterfactionChannel", &player.isInterfactionChannelEnabled);
    Config.MainConfig.tryGetBool("Player", "InterfactionGroup", &player.isInterfactionGroupEnabled);
    Config.MainConfig.tryGetBool("Player", "InterfactionGuild", &player.isInterfactionGuildEnabled);
    Config.MainConfig.tryGetBool("Player", "InterfactionTrade", &player.isInterfactionTradeEnabled);
    Config.MainConfig.tryGetBool("Player", "InterfactionFriends", &player.isInterfactionFriendsEnabled);
    Config.MainConfig.tryGetBool("Player", "InterfactionMail", &player.isInterfactionMailEnabled);
    Config.MainConfig.tryGetBool("Player", "InterfactionMisc", &player.isInterfactionMiscEnabled);
    Config.MainConfig.tryGetBool("Player", "CrossOverCharacters", &player.isCrossoverCharsCreationEnabled);
    Config.MainConfig.tryGetBool("Player", "EnableGoldCap", &player.isGoldCapEnabled);
    Config.MainConfig.tryGetInt("Player", "MaximumGold", &player.limitGoldAmount);
    if (player.limitGoldAmount > 0)
        player.limitGoldAmount *= 10000;
    Config.MainConfig.tryGetInt("Player", "StartingGold", &player.startGoldAmount);
    if (player.startGoldAmount > 0)
        player.startGoldAmount *= 10000;
    Config.MainConfig.tryGetBool("Player", "AntiMasterLootNinja", &player.deactivateMasterLootNinja);
    Config.MainConfig.tryGetBool("Player", "EnablePvPToken", &player.enablePvPToken);
    Config.MainConfig.tryGetInt("Player", "PvPTokenID", &player.pvpTokenId);
    if (!player.enablePvPToken || player.pvpTokenId == 0)
        player.enablePvPToken = player.pvpTokenId = 0;

    // world.conf - Guild Settings
    Config.MainConfig.tryGetInt("Guild", "CharterCost", &guild.charterCost);
    Config.MainConfig.tryGetBool("Guild", "RequireAllSignatures", &guild.requireAllSignatures);
    Config.MainConfig.tryGetInt("Guild", "MaxLevel", &guild.maxLevel);
    Config.MainConfig.tryGetInt("Guild", "MaxMembers", &guild.maxMembers);
    Config.MainConfig.tryGetInt("Guild", "MaxXpPerDay", &guild.maxXpPerDay);
    Config.MainConfig.tryGetInt("Guild", "MaxRepPerWeek", &guild.maxRepPerWeek);
    Config.MainConfig.tryGetBool("Guild", "LevelingEnabled", &guild.levelingEnabled);
    Config.MainConfig.tryGetInt("Guild", "UndeletableLevel", &guild.undeletableLevel);
    Config.MainConfig.tryGetInt("Guild", "EventLogCount", &guild.eventLogCount);
    Config.MainConfig.tryGetInt("Guild", "NewsLogCount", &guild.newsLogCount);
    Config.MainConfig.tryGetInt("Guild", "BankLogCount", &guild.bankLogCount);
    Config.MainConfig.tryGetInt("Guild", "SaveInterval", &guild.saveInterval);

    // world.conf - Announce Settings
    Config.MainConfig.tryGetString("Announce", "Tag", &announce.announceTag);
    Config.MainConfig.tryGetBool("Announce", "GMAdminTag", &announce.enableGmAdminTag);
    Config.MainConfig.tryGetBool("Announce", "NameinAnnounce", &announce.showNameInAnnounce);
    Config.MainConfig.tryGetBool("Announce", "NameinWAnnounce", &announce.showNameInWAnnounce);
    Config.MainConfig.tryGetBool("Announce", "ShowInConsole", &announce.showAnnounceInConsoleOutput);
    Config.MainConfig.tryGetInt("Announce", "AnnTagColor", &announce.tagColor);
    Config.MainConfig.tryGetInt("Announce", "AnnGMTagColor", &announce.tagGmColor);
    Config.MainConfig.tryGetInt("Announce", "AnnNameColor", &announce.nameColor);
    Config.MainConfig.tryGetInt("Announce", "AnnMsgColor", &announce.msgColor);

    // world.conf - GameMaster Settings
    Config.MainConfig.tryGetBool("GameMaster", "StartOnGMIsland", &gm.isStartOnGmIslandEnabled);
    Config.MainConfig.tryGetBool("GameMaster", "DisableAchievements", &gm.disableAchievements);
    Config.MainConfig.tryGetBool("GameMaster", "ListOnlyActiveGMs", &gm.listOnlyActiveGms);
    Config.MainConfig.tryGetBool("GameMaster", "HidePermissions", &gm.hidePermissions);
    Config.MainConfig.tryGetBool("GameMaster", "AnnounceKick", &gm.worldAnnounceOnKickPlayer);
    Config.MainConfig.tryGetBool("GameMaster", "ShowGMInWhoList", &gm.showGmInWhoList);
    Config.MainConfig.tryGetString("GameMaster", "GmClientChannel", &gm.gmClientChannelName);

    // world.conf - Broadcast Settings
    Config.MainConfig.tryGetBool("Broadcast", "EnableSystem", &broadcast.isSystemEnabled);

    // world.conf - Rate Settings
    Config.MainConfig.tryGetFloat("Rates", "Health", &mFloatRates[RATE_HEALTH]);
    Config.MainConfig.tryGetFloat("Rates", "Mana", &mFloatRates[RATE_POWER1]);
    Config.MainConfig.tryGetFloat("Rates", "Rage", &mFloatRates[RATE_POWER2]);
    Config.MainConfig.tryGetFloat("Rates", "Focus", &mFloatRates[RATE_POWER3]);
    Config.MainConfig.tryGetFloat("Rates", "Energy", &mFloatRates[RATE_POWER4]);
    Config.MainConfig.tryGetFloat("Rates", "RunicPower", &mFloatRates[RATE_POWER7]);
    Config.MainConfig.tryGetFloat("Rates", "VehiclePower", &mFloatRates[RATE_VEHICLES_POWER_REGEN]);

    Config.MainConfig.tryGetFloat("Rates", "DropGrey", &mFloatRates[RATE_DROP0]);
    Config.MainConfig.tryGetFloat("Rates", "DropWhite", &mFloatRates[RATE_DROP1]);
    Config.MainConfig.tryGetFloat("Rates", "DropGreen", &mFloatRates[RATE_DROP2]);
    Config.MainConfig.tryGetFloat("Rates", "DropBlue", &mFloatRates[RATE_DROP3]);
    Config.MainConfig.tryGetFloat("Rates", "DropPurple", &mFloatRates[RATE_DROP4]);
    Config.MainConfig.tryGetFloat("Rates", "DropOrange", &mFloatRates[RATE_DROP5]);
    Config.MainConfig.tryGetFloat("Rates", "DropArtifact", &mFloatRates[RATE_DROP6]);
    Config.MainConfig.tryGetFloat("Rates", "DropMoney", &mFloatRates[RATE_MONEY]);

    Config.MainConfig.tryGetFloat("Rates", "XP", &mFloatRates[RATE_XP]);
    Config.MainConfig.tryGetFloat("Rates", "RestXP", &mFloatRates[RATE_RESTXP]);
    Config.MainConfig.tryGetFloat("Rates", "QuestXP", &mFloatRates[RATE_QUESTXP]);
    Config.MainConfig.tryGetFloat("Rates", "ExploreXP", &mFloatRates[RATE_EXPLOREXP]);

    Config.MainConfig.tryGetFloat("Rates", "QuestReputation", &mFloatRates[RATE_QUESTREPUTATION]);
    Config.MainConfig.tryGetFloat("Rates", "KillReputation", &mFloatRates[RATE_KILLREPUTATION]);
    Config.MainConfig.tryGetFloat("Rates", "SkillChance", &mFloatRates[RATE_SKILLCHANCE]);
    Config.MainConfig.tryGetFloat("Rates", "SkillRate", &mFloatRates[RATE_SKILLRATE]);

    Config.MainConfig.tryGetFloat("Rates", "Honor", &mFloatRates[RATE_HONOR]);
    Config.MainConfig.tryGetInt("Rates", "PvPTimer", &mIntRates[INTRATE_PVPTIMER]);
    Config.MainConfig.tryGetInt("Rates", "ArenaQueueDiff", &rate.arenaQueueDiff);
    Config.MainConfig.tryGetFloat("Rates", "ArenaMultiplier2x", &mFloatRates[RATE_ARENAPOINTMULTIPLIER2X]);
    Config.MainConfig.tryGetFloat("Rates", "ArenaMultiplier3x", &mFloatRates[RATE_ARENAPOINTMULTIPLIER3X]);
    Config.MainConfig.tryGetFloat("Rates", "ArenaMultiplier5x", &mFloatRates[RATE_ARENAPOINTMULTIPLIER5X]);

    Config.MainConfig.tryGetInt("Rates", "Save", &mIntRates[INTRATE_SAVE]);
    Config.MainConfig.tryGetInt("Rates", "Compression", &mIntRates[INTRATE_COMPRESSION]);

    // world.conf - Corpse Decay Settings
    Config.MainConfig.tryGetInt("CorpseDecaySettings", "DecayNormal", &corpseDecay.normalTimeInSeconds);
    Config.MainConfig.tryGetInt("CorpseDecaySettings", "DecayRare", &corpseDecay.rareTimeInSeconds);
    Config.MainConfig.tryGetInt("CorpseDecaySettings", "DecayElite", &corpseDecay.eliteTimeInSeconds);
    Config.MainConfig.tryGetInt("CorpseDecaySettings", "DecayRareElite", &corpseDecay.rareEliteTimeInSeconds);
    Config.MainConfig.tryGetInt("CorpseDecaySettings", "DecayWorldboss", &corpseDecay.worldbossTimeInSeconds);

    // world.conf - Terrain & Collision Settings
    Config.MainConfig.tryGetBool("Terrain", "Collision", &terrainCollision.isCollisionEnabled);
    Config.MainConfig.tryGetBool("Terrain", "Pathfinding", &terrainCollision.isPathfindingEnabled);

    // world.conf - Mail Settings
    Config.MainConfig.tryGetBool("Mail", "DisablePostageCostsForGM", &mail.isCostsForGmDisabled);
    Config.MainConfig.tryGetBool("Mail", "DisablePostageCosts", &mail.isCostsForEveryoneDisabled);
    Config.MainConfig.tryGetBool("Mail", "DisablePostageDelayItems", &mail.isDelayItemsDisabled);
    Config.MainConfig.tryGetBool("Mail", "DisableMessageExpiry", &mail.isMessageExpiryDisabled);
    Config.MainConfig.tryGetBool("Mail", "EnableInterfactionMailForGM", &mail.isInterfactionMailForGmEnabled);

    // world.conf - Startup Options
    Config.MainConfig.tryGetBool("Startup", "EnableMultithreadedLoading", &startup.enableMultithreadedLoading);
    Config.MainConfig.tryGetBool("Startup", "EnableSpellIDDump", &startup.enableSpellIdDump);
    Config.MainConfig.tryGetString("Startup", "LoadAdditionalTables", &startup.additionalTableLoads);

    // world.conf - AntiHack Setup
    Config.MainConfig.tryGetBool("AntiHack", "Teleport", &antiHack.isTeleportHackCheckEnabled);
    Config.MainConfig.tryGetBool("AntiHack", "Speed", &antiHack.isSpeedHackCkeckEnabled);
    Config.MainConfig.tryGetBool("AntiHack", "DisableOnGM", &antiHack.isAntiHackCheckDisabledForGm);

    // world.conf - Period Setup
    Config.MainConfig.tryGetString("Periods", "ArenaUpdate", &period.arenaUpdate);
    Config.MainConfig.tryGetString("Periods", "DailyUpdate", &period.dailyUpdate);

    // world.conf - Chat Settings
    Config.MainConfig.tryGetString("Chat", "BannedChannels", &chat.bannedChannels);
    Config.MainConfig.tryGetString("Chat", "MinimumLevel", &chat.minimumTalkLevel);
    Config.MainConfig.tryGetInt("Chat", "FloodLines", &chat.linesBeforeProtection);
    Config.MainConfig.tryGetInt("Chat", "FloodSeconds", &chat.secondsBeforeProtectionReset);
    if (!chat.linesBeforeProtection || !chat.secondsBeforeProtectionReset)
        chat.linesBeforeProtection = chat.secondsBeforeProtectionReset = 0;
    Config.MainConfig.tryGetBool("Chat", "FloodSendMessage", &chat.enableSendFloodProtectionMessage);

    // world.conf - Remote Console Setup
    Config.MainConfig.tryGetBool("RemoteConsole", "Enabled", &remoteConsole.isEnabled);
    Config.MainConfig.tryGetString("RemoteConsole", "Host", &remoteConsole.host);
    Config.MainConfig.tryGetInt("RemoteConsole", "Port", &remoteConsole.port);

    // world.conf - Dungeon / Instance Setup
    Config.MainConfig.tryGetBool("InstanceHandling", "TakeGroupLeaderID", &instance.useGroupLeaderInstanceId);
    Config.MainConfig.tryGetBool("InstanceHandling", "SlidingExpiration", &instance.isRelativeExpirationEnabled);
    Config.MainConfig.tryGetInt("InstanceHandling", "DailyHeroicInstanceResetHour", &instance.relativeDailyHeroicInstanceResetHour);
    if (instance.relativeDailyHeroicInstanceResetHour < 0)
        instance.relativeDailyHeroicInstanceResetHour = 0;
    if (instance.relativeDailyHeroicInstanceResetHour > 23)
        instance.relativeDailyHeroicInstanceResetHour = 23;
    Config.MainConfig.tryGetBool("InstanceHandling", "CheckTriggerPrerequisites", &instance.checkTriggerPrerequisitesOnEnter);

    // world.conf - BattleGround settings
    Config.MainConfig.tryGetInt("Battleground", "AV_MIN", &bg.minPlayerCountAlteracValley);
    Config.MainConfig.tryGetInt("Battleground", "AV_MAX", &bg.maxPlayerCountAlteracValley);
    Config.MainConfig.tryGetInt("Battleground", "AB_MIN", &bg.minPlayerCountArathiBasin);
    Config.MainConfig.tryGetInt("Battleground", "AB_MAX", &bg.maxPlayerCountArathiBasin);
    Config.MainConfig.tryGetInt("Battleground", "WSG_MIN", &bg.minPlayerCountWarsongGulch);
    Config.MainConfig.tryGetInt("Battleground", "WSG_MAX", &bg.maxPlayerCountWarsongGulch);
    Config.MainConfig.tryGetInt("Battleground", "EOTS_MIN", &bg.minPlayerCountEyeOfTheStorm);
    Config.MainConfig.tryGetInt("Battleground", "EOTS_MAX", &bg.maxPlayerCountEyeOfTheStorm);
    Config.MainConfig.tryGetInt("Battleground", "SOTA_MIN", &bg.minPlayerCountStrandOfTheAncients);
    Config.MainConfig.tryGetInt("Battleground", "SOTA_MAX", &bg.maxPlayerCountStrandOfTheAncients);
    Config.MainConfig.tryGetInt("Battleground", "IOC_MIN", &bg.minPlayerCountIsleOfConquest);
    Config.MainConfig.tryGetInt("Battleground", "IOC_MAX", &bg.maxPlayerCountIsleOfConquest);
    Config.MainConfig.tryGetInt("Battleground", "RBG_FIRST_WIN_HONOR", &bg.firstRbgHonorValueToday);
    Config.MainConfig.tryGetInt("Battleground", "RBG_FIRST_WIN_ARENA", &bg.firstRbgArenaHonorValueToday);
    Config.MainConfig.tryGetInt("Battleground", "RBG_WIN_HONOR", &bg.honorableKillsRbg);
    Config.MainConfig.tryGetInt("Battleground", "RBG_WIN_ARENA", &bg.honorableArenaWinRbg);
    Config.MainConfig.tryGetInt("Battleground", "RBG_LOSE_HONOR", &bg.honorByLosingRbg);
    Config.MainConfig.tryGetInt("Battleground", "RBG_LOSE_ARENA", &bg.honorByLosingArenaRbg);

    // world.conf - Arena Settings
    Config.MainConfig.tryGetInt("Arena", "Season", &arena.arenaSeason);
    Config.MainConfig.tryGetInt("Arena", "Progress", &arena.arenaProgress);
    Config.MainConfig.tryGetInt("Arena", "2V2_COST", &arena.charterCost2v2);
    Config.MainConfig.tryGetInt("Arena", "3V3_COST", &arena.charterCost3v3);
    Config.MainConfig.tryGetInt("Arena", "5V5_COST", &arena.charterCost5v5);
    Config.MainConfig.tryGetInt("Arena", "2V2_MIN", &arena.minPlayerCount2V2);
    Config.MainConfig.tryGetInt("Arena", "2V2_MAX", &arena.maxPlayerCount2V2);
    Config.MainConfig.tryGetInt("Arena", "3V3_MIN", &arena.minPlayerCount3V3);
    Config.MainConfig.tryGetInt("Arena", "3V3_MAX", &arena.maxPlayerCount3V3);
    Config.MainConfig.tryGetInt("Arena", "5V5_MIN", &arena.minPlayerCount5V5);
    Config.MainConfig.tryGetInt("Arena", "5V5_MAX", &arena.maxPlayerCount5V5);

    // world.conf - Limits settings
    Config.MainConfig.tryGetBool("Limits", "Enable", &limit.isLimitSystemEnabled);
    Config.MainConfig.tryGetInt("Limits", "AutoAttackDmg", &limit.maxAutoAttackDamageCap);
    Config.MainConfig.tryGetInt("Limits", "SpellDmg", &limit.maxSpellDamageCap);
    Config.MainConfig.tryGetInt("Limits", "Health", &limit.maxHealthCap);
    Config.MainConfig.tryGetInt("Limits", "Mana", &limit.maxManaCap);
    Config.MainConfig.tryGetInt("Limits", "Honor", &limit.maxHonorPoints);
    Config.MainConfig.tryGetInt("Limits", "Arena", &limit.maxArenaPoints);
    Config.MainConfig.tryGetBool("Limits", "Disconnect", &limit.disconnectPlayerForExceedingLimits);
    Config.MainConfig.tryGetBool("Limits", "BroadcastGMs", &limit.broadcastMessageToGmOnExceeding);
}

uint32_t WorldConfig::getPlayerLimit() const
{
    return server.playerLimit;
}

uint32_t WorldConfig::getKickAFKPlayerTime() const
{
    return server.secondsBeforeKickAFKPlayers;
}

void WorldConfig::setMessageOfTheDay(std::string motd)
{
    server.messageOfTheDay = std::move(motd);
}

std::string WorldConfig::getMessageOfTheDay() const
{
    return server.messageOfTheDay;
}

void WorldConfig::setFloatRate(uint32_t index, float value)
{
    mFloatRates[index] = value;
}

float WorldConfig::getFloatRate(uint32_t index) const
{
    return mFloatRates[index];
}

void WorldConfig::setIntRate(uint32_t index, uint32_t value)
{
    mIntRates[index] = value;
}

uint32_t WorldConfig::getIntRate(uint32_t index) const
{
    return mIntRates[index];
}

std::string WorldConfig::getGmClientChannelName() const
{
    return gm.gmClientChannelName;
}

std::string WorldConfig::getColorStringForNumber(int color) const
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

