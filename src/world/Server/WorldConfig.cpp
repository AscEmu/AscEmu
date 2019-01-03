/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "WorldConfig.h"
#include <utility>
#include "WorldConf.h"
#include "Server/MainServerDefines.h"
#include "Config/Config.h"
#include "Map/MapCell.h"
//#include "Server/WorldSocket.h"
#include "Units/Players/PlayerDefines.hpp"
#include "shared/Log.hpp"


WorldConfig::WorldConfig(): mFloatRates{}, mIntRates{}
{
    // world.conf - Mysql Database Section
    worldDb.port = 3306;
    worldDb.connections = 3;

    charDb.port = 3306;
    charDb.connections = 5;

    // world.conf - LogonServer Settings
    logonServer.address = "127.0.0.1";
    logonServer.port = 8093;
    logonServer.name = "Default Logon";
    logonServer.realmCount = 1;
    logonServer.disablePings = false;

    // world.conf - Listen Config
    listen.listenPort = 8129;

    // world.conf - Log Settings
    log.extendedLogsDir = "./";
    log.worldFileLogLevel = 1;
    log.worldDebugFlags = 0;
    log.enableWorldPacketLog = false;
    log.disableCrashdump = false;
    log.enableCheaterLog = false;
    log.enableGmCommandLog = false;
    log.enablePlayerLog = false;
    log.enableTimeStamp = false;
    log.enableSqlBanLog = false;

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
    corpseDecay.normalTimeInSeconds = 300000;
    corpseDecay.rareTimeInSeconds = 900000;
    corpseDecay.eliteTimeInSeconds = 300000;
    corpseDecay.rareEliteTimeInSeconds = 900000;
    corpseDecay.worldbossTimeInSeconds = 3600000;

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
            LogDetail("Config : " CONFDIR "/world.conf reloaded");
        }
        else
        {
            LogError("Config : error occurred loading " CONFDIR "/world.conf");
            return;
        }
    }

    // world.conf - Mysql Database Section
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("WorldDatabase", "Hostname", &worldDb.host));
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("WorldDatabase", "Username", &worldDb.user));
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("WorldDatabase", "Password", &worldDb.password));
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("WorldDatabase", "Name", &worldDb.dbName));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("WorldDatabase", "Port", &worldDb.port));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("WorldDatabase", "Connections", &worldDb.connections));

    ARCEMU_ASSERT(Config.MainConfig.tryGetString("CharacterDatabase", "Hostname", &charDb.host));
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("CharacterDatabase", "Username", &charDb.user));
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("CharacterDatabase", "Password", &charDb.password));
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("CharacterDatabase", "Name", &charDb.dbName));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("CharacterDatabase", "Port", &charDb.port));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("CharacterDatabase", "Connections", &charDb.connections));

    // world.conf - LogonServer Settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("LogonServer", "Address", &logonServer.address));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("LogonServer", "Port", &logonServer.port));
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("LogonServer", "Name", &logonServer.name));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("LogonServer", "RealmCount", &logonServer.realmCount));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("LogonServer", "DisablePings", &logonServer.disablePings));
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("LogonServer", "RemotePassword", &logonServer.remotePassword));

    // world.conf - Realm Section

    // world.conf - Listen Config
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("Listen", "Host", &listen.listenHost));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Listen", "WorldServerPort", &listen.listenPort));

    // world.conf - Log Settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Log", "WorldFileLogLevel", &log.worldFileLogLevel));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Log", "WorldDebugFlags", &log.worldDebugFlags));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Log", "EnableWorldPacketLog", &log.enableWorldPacketLog));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Log", "DisableCrashdumpReport", &log.disableCrashdump));

    ARCEMU_ASSERT(Config.MainConfig.tryGetString("Log", "ExtendedLogDir", &log.extendedLogsDir));
    if (log.extendedLogsDir != "./")
        log.extendedLogsDir = "./" + log.extendedLogsDir + "/";

    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Log", "EnableCheaterLog", &log.enableCheaterLog));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Log", "EnableGMCommandLog", &log.enableGmCommandLog));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Log", "EnablePlayerLog", &log.enablePlayerLog));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Log", "EnableTimeStamp", &log.enableTimeStamp));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Log", "EnableSqlBanLog", &log.enableSqlBanLog));

    // world.conf - Server Settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Server", "PlayerLimit", &server.playerLimit));
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("Server", "Motd", &server.messageOfTheDay));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Server", "SendStatsOnJoin", &server.sendStatsOnJoin));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Server", "TimeZone", &server.gmtTimeZone));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Server", "CompressionThreshold", &server.compressionThreshold));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Server", "AdjustPriority", &server.enableAdjustPriority));
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
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Server", "MapUnloadTime", &server.mapUnloadTime));
    if (server.mapUnloadTime == 0)
    {
        LOG_ERROR("MapUnloadTime is set to 0. This will NEVER unload MapCells!!! Overriding it to default value of %u", MAP_CELL_DEFAULT_UNLOAD_TIME);
        server.mapUnloadTime = MAP_CELL_DEFAULT_UNLOAD_TIME;
    }
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Server", "MapCellNumber", &server.mapCellNumber));
    if (server.mapCellNumber == 0)
    {
        LOG_ERROR("MapCellNumber is set to 0. Congrats, no MapCells will be loaded. Overriding it to default value of 1");
        server.mapCellNumber = 1;
    }
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Server", "KickAFKPlayers", &server.secondsBeforeKickAFKPlayers));
    server.secondsBeforeKickAFKPlayers *= 1000;
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Server", "QueueUpdateInterval", &server.queueUpdateInterval));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Server", "EnableBreathing", &server.enableBreathing));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Server", "LimitedNames", &server.enableLimitedNames));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Server", "UseAccountData", &server.useAccountData));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Server", "AllowPlayerCommands", &server.requireGmForCommands));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Server", "SaveExtendedCharData", &server.saveExtendedCharData));
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("Server", "DataDir", &server.dataDir));
    if (server.dataDir == "")
        server.dataDir = "./";
    else if (server.dataDir != "./")
        server.dataDir = "./" + server.dataDir + "/";

    // world.conf - Player Settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Player", "StartingLevel", &player.playerStartingLevel));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Player", "LevelCap", &player.playerLevelCap));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Player", "GenLevelCap", &player.playerGeneratedInformationByLevelCap)); //! no delete
    if (player.playerStartingLevel > static_cast<int32_t>(player.playerLevelCap))
        player.playerStartingLevel = static_cast<int32_t>(player.playerLevelCap);
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Player", "DKStartingTalents", &player.deathKnightStartTalentPoints));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "DKPreReq", &player.deathKnightPreReq));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "DKLimit", &player.deathKnightLimit));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Player", "MaxProfessions", &player.maxProfessions));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "SkipCinematic", &player.skipCinematics));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Player", "InstantLogout", &player.enableInstantLogoutForAccessType));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Player", "MinDualSpecLevel", &player.minDualSpecLevel));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Player", "MinTalentResetLevel", &player.minTalentResetLevel));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "ShowAllVendorItems", &player.showAllVendorItems));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "InterfactionChat", &player.isInterfactionChatEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "InterfactionChannel", &player.isInterfactionChannelEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "InterfactionGroup", &player.isInterfactionGroupEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "InterfactionGuild", &player.isInterfactionGuildEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "InterfactionTrade", &player.isInterfactionTradeEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "InterfactionFriends", &player.isInterfactionFriendsEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "InterfactionMail", &player.isInterfactionMailEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "InterfactionMisc", &player.isInterfactionMiscEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "CrossOverCharacters", &player.isCrossoverCharsCreationEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "EnableGoldCap", &player.isGoldCapEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Player", "MaximumGold", &player.limitGoldAmount));
    if (player.limitGoldAmount > 0)
        player.limitGoldAmount *= 10000;
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Player", "StartingGold", &player.startGoldAmount));
    if (player.startGoldAmount > 0)
        player.startGoldAmount *= 10000;
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "AntiMasterLootNinja", &player.deactivateMasterLootNinja));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Player", "EnablePvPToken", &player.enablePvPToken));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Player", "PvPTokenID", &player.pvpTokenId));
    if (!player.enablePvPToken || player.pvpTokenId == 0)
        player.enablePvPToken = player.pvpTokenId = 0;

    // world.conf - Guild Settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Guild", "CharterCost", &guild.charterCost));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Guild", "RequireAllSignatures", &guild.requireAllSignatures));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Guild", "MaxLevel", &guild.maxLevel));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Guild", "MaxMembers", &guild.maxMembers));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Guild", "MaxXpPerDay", &guild.maxXpPerDay));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Guild", "MaxRepPerWeek", &guild.maxRepPerWeek));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Guild", "LevelingEnabled", &guild.levelingEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Guild", "UndeletableLevel", &guild.undeletableLevel));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Guild", "EventLogCount", &guild.eventLogCount));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Guild", "NewsLogCount", &guild.newsLogCount));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Guild", "BankLogCount", &guild.bankLogCount));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Guild", "SaveInterval", &guild.saveInterval));

    // world.conf - Announce Settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("Announce", "Tag", &announce.announceTag));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Announce", "GMAdminTag", &announce.enableGmAdminTag));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Announce", "NameinAnnounce", &announce.showNameInAnnounce));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Announce", "NameinWAnnounce", &announce.showNameInWAnnounce));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Announce", "ShowInConsole", &announce.showAnnounceInConsoleOutput));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Announce", "AnnTagColor", &announce.tagColor));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Announce", "AnnGMTagColor", &announce.tagGmColor));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Announce", "AnnNameColor", &announce.nameColor));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Announce", "AnnMsgColor", &announce.msgColor));

    // world.conf - GameMaster Settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("GameMaster", "StartOnGMIsland", &gm.isStartOnGmIslandEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("GameMaster", "DisableAchievements", &gm.disableAchievements));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("GameMaster", "ListOnlyActiveGMs", &gm.listOnlyActiveGms));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("GameMaster", "HidePermissions", &gm.hidePermissions));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("GameMaster", "AnnounceKick", &gm.worldAnnounceOnKickPlayer));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("GameMaster", "ShowGMInWhoList", &gm.showGmInWhoList));
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("GameMaster", "GmClientChannel", &gm.gmClientChannelName));

    // world.conf - Broadcast Settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Broadcast", "EnableSystem", &broadcast.isSystemEnabled));

    // world.conf - Rate Settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "Health", &mFloatRates[RATE_HEALTH]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "Mana", &mFloatRates[RATE_POWER1]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "Rage", &mFloatRates[RATE_POWER2]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "Focus", &mFloatRates[RATE_POWER3]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "Energy", &mFloatRates[RATE_POWER4]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "RunicPower", &mFloatRates[RATE_POWER7]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "VehiclePower", &mFloatRates[RATE_VEHICLES_POWER_REGEN]));

    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "DropGrey", &mFloatRates[RATE_DROP0]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "DropWhite", &mFloatRates[RATE_DROP1]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "DropGreen", &mFloatRates[RATE_DROP2]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "DropBlue", &mFloatRates[RATE_DROP3]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "DropPurple", &mFloatRates[RATE_DROP4]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "DropOrange", &mFloatRates[RATE_DROP5]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "DropArtifact", &mFloatRates[RATE_DROP6]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "DropMoney", &mFloatRates[RATE_MONEY]));

    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "XP", &mFloatRates[RATE_XP]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "RestXP", &mFloatRates[RATE_RESTXP]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "QuestXP", &mFloatRates[RATE_QUESTXP]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "ExploreXP", &mFloatRates[RATE_EXPLOREXP]));

    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "QuestReputation", &mFloatRates[RATE_QUESTREPUTATION]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "KillReputation", &mFloatRates[RATE_KILLREPUTATION]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "SkillChance", &mFloatRates[RATE_SKILLCHANCE]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "SkillRate", &mFloatRates[RATE_SKILLRATE]));

    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "Honor", &mFloatRates[RATE_HONOR]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Rates", "PvPTimer", &mIntRates[INTRATE_PVPTIMER]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Rates", "ArenaQueueDiff", &rate.arenaQueueDiff));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "ArenaMultiplier2x", &mFloatRates[RATE_ARENAPOINTMULTIPLIER2X]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "ArenaMultiplier3x", &mFloatRates[RATE_ARENAPOINTMULTIPLIER3X]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetFloat("Rates", "ArenaMultiplier5x", &mFloatRates[RATE_ARENAPOINTMULTIPLIER5X]));

    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Rates", "Save", &mIntRates[INTRATE_SAVE]));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Rates", "Compression", &mIntRates[INTRATE_COMPRESSION]));

    // world.conf - Corpse Decay Settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("CorpseDecaySettings", "DecayNormal", &corpseDecay.normalTimeInSeconds));
    corpseDecay.normalTimeInSeconds *= 1000;
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("CorpseDecaySettings", "DecayRare", &corpseDecay.rareTimeInSeconds));
    corpseDecay.rareTimeInSeconds *= 1000;
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("CorpseDecaySettings", "DecayElite", &corpseDecay.eliteTimeInSeconds));
    corpseDecay.eliteTimeInSeconds *= 1000;
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("CorpseDecaySettings", "DecayRareElite", &corpseDecay.rareEliteTimeInSeconds));
    corpseDecay.rareEliteTimeInSeconds *= 1000;
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("CorpseDecaySettings", "DecayWorldboss", &corpseDecay.worldbossTimeInSeconds));
    corpseDecay.worldbossTimeInSeconds *= 1000;

    // world.conf - Terrain & Collision Settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Terrain", "Collision", &terrainCollision.isCollisionEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Terrain", "Pathfinding", &terrainCollision.isPathfindingEnabled));
    if (terrainCollision.isPathfindingEnabled && !terrainCollision.isCollisionEnabled)
        terrainCollision.isPathfindingEnabled = false;

    // world.conf - Mail Settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Mail", "DisablePostageCostsForGM", &mail.isCostsForGmDisabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Mail", "DisablePostageCosts", &mail.isCostsForEveryoneDisabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Mail", "DisablePostageDelayItems", &mail.isDelayItemsDisabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Mail", "DisableMessageExpiry", &mail.isMessageExpiryDisabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Mail", "EnableInterfactionMailForGM", &mail.isInterfactionMailForGmEnabled));

    // world.conf - Startup Options
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Startup", "EnableMultithreadedLoading", &startup.enableMultithreadedLoading));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Startup", "EnableSpellIDDump", &startup.enableSpellIdDump));
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("Startup", "LoadAdditionalTables", &startup.additionalTableLoads));

    // world.conf - AntiHack Setup
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("AntiHack", "Teleport", &antiHack.isTeleportHackCheckEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("AntiHack", "Speed", &antiHack.isSpeedHackCkeckEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("AntiHack", "DisableOnGM", &antiHack.isAntiHackCheckDisabledForGm));

    // world.conf - Period Setup
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("Periods", "ArenaUpdate", &period.arenaUpdate));
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("Periods", "DailyUpdate", &period.dailyUpdate));

    // world.conf - Chat Settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("Chat", "BannedChannels", &chat.bannedChannels));
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("Chat", "MinimumLevel", &chat.minimumTalkLevel));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Chat", "FloodLines", &chat.linesBeforeProtection));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Chat", "FloodSeconds", &chat.secondsBeforeProtectionReset));
    if (!chat.linesBeforeProtection || !chat.secondsBeforeProtectionReset)
        chat.linesBeforeProtection = chat.secondsBeforeProtectionReset = 0;
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Chat", "FloodSendMessage", &chat.enableSendFloodProtectionMessage));

    // world.conf - Remote Console Setup
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("RemoteConsole", "Enabled", &remoteConsole.isEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetString("RemoteConsole", "Host", &remoteConsole.host));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("RemoteConsole", "Port", &remoteConsole.port));

    // world.conf - Dungeon / Instance Setup
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("InstanceHandling", "TakeGroupLeaderID", &instance.useGroupLeaderInstanceId));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("InstanceHandling", "SlidingExpiration", &instance.isRelativeExpirationEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("InstanceHandling", "DailyHeroicInstanceResetHour", &instance.relativeDailyHeroicInstanceResetHour));
    if (instance.relativeDailyHeroicInstanceResetHour < 0)
        instance.relativeDailyHeroicInstanceResetHour = 0;
    if (instance.relativeDailyHeroicInstanceResetHour > 23)
        instance.relativeDailyHeroicInstanceResetHour = 23;
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("InstanceHandling", "CheckTriggerPrerequisites", &instance.checkTriggerPrerequisitesOnEnter));

    // world.conf - BattleGround settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "AV_MIN", &bg.minPlayerCountAlteracValley));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "AV_MAX", &bg.maxPlayerCountAlteracValley));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "AB_MIN", &bg.minPlayerCountArathiBasin));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "AB_MAX", &bg.maxPlayerCountArathiBasin));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "WSG_MIN", &bg.minPlayerCountWarsongGulch));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "WSG_MAX", &bg.maxPlayerCountWarsongGulch));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "EOTS_MIN", &bg.minPlayerCountEyeOfTheStorm));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "EOTS_MAX", &bg.maxPlayerCountEyeOfTheStorm));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "SOTA_MIN", &bg.minPlayerCountStrandOfTheAncients));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "SOTA_MAX", &bg.maxPlayerCountStrandOfTheAncients));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "IOC_MIN", &bg.minPlayerCountIsleOfConquest));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "IOC_MAX", &bg.maxPlayerCountIsleOfConquest));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "RBG_FIRST_WIN_HONOR", &bg.firstRbgHonorValueToday));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "RBG_FIRST_WIN_ARENA", &bg.firstRbgArenaHonorValueToday));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "RBG_WIN_HONOR", &bg.honorableKillsRbg));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "RBG_WIN_ARENA", &bg.honorableArenaWinRbg));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "RBG_LOSE_HONOR", &bg.honorByLosingRbg));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Battleground", "RBG_LOSE_ARENA", &bg.honorByLosingArenaRbg));

    // world.conf - Arena Settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Arena", "Season", &arena.arenaSeason));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Arena", "Progress", &arena.arenaProgress));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Arena", "2V2_COST", &arena.charterCost2v2));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Arena", "3V3_COST", &arena.charterCost3v3));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Arena", "5V5_COST", &arena.charterCost5v5));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Arena", "2V2_MIN", &arena.minPlayerCount2V2));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Arena", "2V2_MAX", &arena.maxPlayerCount2V2));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Arena", "3V3_MIN", &arena.minPlayerCount3V3));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Arena", "3V3_MAX", &arena.maxPlayerCount3V3));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Arena", "5V5_MIN", &arena.minPlayerCount5V5));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Arena", "5V5_MAX", &arena.maxPlayerCount5V5));

    // world.conf - Limits settings
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Limits", "Enable", &limit.isLimitSystemEnabled));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Limits", "AutoAttackDmg", &limit.maxAutoAttackDamageCap));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Limits", "SpellDmg", &limit.maxSpellDamageCap));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Limits", "Health", &limit.maxHealthCap));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Limits", "Mana", &limit.maxManaCap));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Limits", "Honor", &limit.maxHonorPoints));
    ARCEMU_ASSERT(Config.MainConfig.tryGetInt("Limits", "Arena", &limit.maxArenaPoints));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Limits", "Disconnect", &limit.disconnectPlayerForExceedingLimits));
    ARCEMU_ASSERT(Config.MainConfig.tryGetBool("Limits", "BroadcastGMs", &limit.broadcastMessageToGmOnExceeding));
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

