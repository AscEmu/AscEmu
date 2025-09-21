/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <sstream>

#include "Common.hpp"
#include "git_version.hpp"
#include "Chat/ChatDefines.hpp"
#include "Chat/ChatCommandHandler.hpp"
#include "Chat/CommandTableStorage.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/Master.h"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Server/WorldSocket.h"
#include "Server/Packets/SmsgServerMessage.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Threading/LegacyThreading.h"
#include "Utilities/Util.hpp"

#include <openssl/opensslv.h>
#include <openssl/crypto.h>

//.server info
bool ChatCommandHandler::HandleServerInfoCommand(const char* /*args*/, WorldSession* m_session)
{
    uint16_t online_gm = 0;
    uint16_t online_count = 0;
    float latency_avg = 0;

    std::lock_guard guard(sObjectMgr.m_playerLock);
    for (const auto playerPair : sObjectMgr.getPlayerStorage())
    {
        Player* player = playerPair.second;
        if (player->getSession())
        {
            online_count++;
            latency_avg += player->getSession()->GetLatency();
            if (player->getSession()->hasPermissions())
            {
                if (!worldConfig.gm.listOnlyActiveGms)
                    online_gm++;
                else
                    if (player->isGMFlagSet())
                        online_gm++;
            }
        }
    }

    uint32_t active_sessions = uint32_t(sWorld.getSessionCount());

    greenSystemMessage(m_session, "Info: |r{}AscEmu {}/{}-{}-{} {}(www.ascemu.org)", MSG_COLOR_WHITE, AE_BUILD_HASH, CONFIG, AE_PLATFORM, AE_ARCHITECTURE, MSG_COLOR_LIGHTBLUE);
    greenSystemMessage(m_session, "Using {}/Library {}", OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION));
    greenSystemMessage(m_session, "Uptime: |r{}", sWorld.getWorldUptimeString().c_str());
    greenSystemMessage(m_session, "Active Branch: |r{}", AE_BUILD_BRANCH);
    greenSystemMessage(m_session, "Active Sessions: |r{}", active_sessions);
    greenSystemMessage(m_session, "Current GMs: |r{} GMs", online_gm);
    greenSystemMessage(m_session, "Current Players: |r{} ({} Peak)", online_gm > 0 ? (online_count - online_gm) : online_count, sWorld.getPeakSessionCount());
    greenSystemMessage(m_session, "Active Thread Count: |r{}", ThreadPool.GetActiveThreadCount());
    greenSystemMessage(m_session, "Free Thread Count: |r{}", ThreadPool.GetFreeThreadCount());
    greenSystemMessage(m_session, "Average Latency: |r{}ms", online_count > 0 ? (latency_avg / online_count) : latency_avg);
    greenSystemMessage(m_session, "CPU Usage: {}%", sWorld.getCPUUsage());
    greenSystemMessage(m_session, "RAM Usage: {} MB", sWorld.getRAMUsage());
    greenSystemMessage(m_session, "SQL Query Cache Size (World): |r{} queries delayed", WorldDatabase.GetQueueSize());
    greenSystemMessage(m_session, "SQL Query Cache Size (Character): |r{} queries delayed", CharacterDatabase.GetQueueSize());
    greenSystemMessage(m_session, "Socket Count: |r{}", sSocketMgr.GetSocketCount());

    return true;
}

//.server rehash
bool ChatCommandHandler::HandleServerRehashCommand(const char* /*args*/, WorldSession* m_session)
{
    std::stringstream teamAnnounce;
    teamAnnounce << MSG_COLOR_RED << "[Team]" << MSG_COLOR_GREEN << " |Hplayer:" << m_session->GetPlayer()->getName().c_str();
    teamAnnounce << "|h[" << m_session->GetPlayer()->getName().c_str() << "]|h:" << MSG_COLOR_YELLOW << " is rehashing config file.";

    sWorld.sendMessageToOnlineGms(teamAnnounce.str());

    sWorld.loadWorldConfigValues(true);

    return true;
}

//.server save
bool ChatCommandHandler::HandleServerSaveCommand(const char* args, WorldSession* m_session)
{
    Player* player_target = nullptr;

    if (!args)
    {
        player_target = GetSelectedPlayer(m_session, false, false);
        if (player_target == nullptr)
        {
            redSystemMessage(m_session, "You need to target or name a player!");
            redSystemMessage(m_session, "Use: .server save (on a targeted player)");
            redSystemMessage(m_session, "or: .server save <playername>");
            return true;
        }
    }
    else
    {
        player_target = sObjectMgr.getPlayer(args, false);
        if (player_target == nullptr)
        {
            redSystemMessage(m_session, "A player with name {} is not online / does not exist!", args);
            return true;
        }
    }

    if (player_target->m_nextSave < 180000)
    {
        player_target->saveToDB(false);
        greenSystemMessage(m_session, "Player {} saved to DB", player_target->getName());
    }
    else
    {
        redSystemMessage(m_session, "You can only save once every 3 minutes.");
    }

    return true;
}

//.server saveall
bool ChatCommandHandler::HandleServerSaveAllCommand(const char* /*args*/, WorldSession* m_session)
{
    auto start_time = Util::TimeNow();
    uint32_t online_count = 0;

    std::lock_guard guard(sObjectMgr.m_playerLock);
    for (const auto playerPair : sObjectMgr.getPlayerStorage())
    {
        Player* player = playerPair.second;
        if (player->getSession())
        {
            player->saveToDB(false);
            online_count++;
        }
    }

    std::stringstream teamAnnounce;
    teamAnnounce << MSG_COLOR_RED << "[Team]" << MSG_COLOR_GREEN << " |Hplayer:" << m_session->GetPlayer()->getName().c_str() << "|h[";
    teamAnnounce << m_session->GetPlayer()->getName().c_str() << "]|h:" << MSG_COLOR_YELLOW << " saved all online players (" << online_count;
    teamAnnounce << ") in " << Util::GetTimeDifferenceToNow(start_time) << " msec.";

    sWorld.sendMessageToOnlineGms(teamAnnounce.str());

    sGMLog.writefromsession(m_session, "saved all online players");

    return true;
}

//.server setmotd
bool ChatCommandHandler::HandleServerSetMotdCommand(const char* args, WorldSession* m_session)
{
    if (!args || strlen(args) < 5)
    {
        redSystemMessage(m_session, "You must specify a message.");
        redSystemMessage(m_session, ".server setmotd <message>");
        return true;
    }

    greenSystemMessage(m_session, "Motd has been set to: {}", args);
    sGMLog.writefromsession(m_session, "Set MOTD to %s", args);
    worldConfig.setMessageOfTheDay(args);

    return true;
}

//.server shutdown
bool ChatCommandHandler::HandleServerShutdownCommand(const char* args, WorldSession* m_session)
{
    uint32_t shutdowntime;
    if (!args)
        shutdowntime = 30;
    else
        shutdowntime = std::stoul(args);

    if (shutdowntime < 30)
        shutdowntime = 30;

    std::stringstream teamAnnounce;
    teamAnnounce << MSG_COLOR_RED << "[Team]" << MSG_COLOR_GREEN << " |Hplayer:" << m_session->GetPlayer()->getName().c_str();
    teamAnnounce << "|h[" << m_session->GetPlayer()->getName().c_str() << "]|h:" << MSG_COLOR_YELLOW << " initiated server shutdown timer " << shutdowntime << " sec";

    sWorld.sendMessageToOnlineGms(teamAnnounce.str());

    sGMLog.writefromsession(m_session, "initiated server shutdown timer %u sec", shutdowntime);

    std::stringstream worldAnnounce;
    worldAnnounce << "Server is shutting down in " << shutdowntime << " seconds.";

    sWorld.sendMessageToAll(worldAnnounce.str());
    sWorld.sendAreaTriggerMessage(worldAnnounce.str());

    shutdowntime *= 1000;
    sMaster.m_ShutdownTimer = shutdowntime;
    sMaster.m_ShutdownEvent = true;
    sMaster.m_restartEvent = false;

    return true;
}

//.server cancelshutdown
bool ChatCommandHandler::HandleServerCancelShutdownCommand(const char* /*args*/, WorldSession* m_session)
{
    if (!sMaster.m_ShutdownEvent)
    {
        redSystemMessage(m_session, "There is no Shutdown/Restart to cancel!");
        return true;
    }

    std::stringstream teamAnnounce;
    if (sMaster.m_restartEvent)
    {
        teamAnnounce << MSG_COLOR_RED << "[Team]" << MSG_COLOR_GREEN << " |Hplayer:" << m_session->GetPlayer()->getName().c_str();
        teamAnnounce << "|h[" << m_session->GetPlayer()->getName().c_str() << "]|h:" << MSG_COLOR_YELLOW << " canceled server restart!";
    }
    else
    {
        teamAnnounce << MSG_COLOR_RED << "[Team]" << MSG_COLOR_GREEN << " |Hplayer:" << m_session->GetPlayer()->getName().c_str();
        teamAnnounce << "|h[" << m_session->GetPlayer()->getName().c_str() << "]|h:" << MSG_COLOR_YELLOW << " canceled server shutdown!";
    }
    
    sWorld.sendMessageToOnlineGms(teamAnnounce.str());
    sGMLog.writefromsession(m_session, "canceled server shutdown");

    sWorld.sendGlobalMessage(AscEmu::Packets::SmsgServerMessage(sMaster.m_restartEvent ? SERVER_MSG_RESTART_CANCELLED : SERVER_MSG_SHUTDOWN_CANCELLED).serialise().get());

    sMaster.m_ShutdownTimer = 5000;
    sMaster.m_ShutdownEvent = false;
    sMaster.m_restartEvent = false;

    return true;
}

//.server restart
bool ChatCommandHandler::HandleServerRestartCommand(const char* args, WorldSession* m_session)
{
    uint32_t shutdowntime;
    if (!args)
        shutdowntime = 30;
    else
        shutdowntime = std::stoul(args);

    if (shutdowntime < 30)
        shutdowntime = 30;

    std::stringstream teamAnnounce;
    teamAnnounce << MSG_COLOR_RED << "[Team]" << MSG_COLOR_GREEN << " |Hplayer:" << m_session->GetPlayer()->getName().c_str() << "|h[" << m_session->GetPlayer()->getName().c_str();
    teamAnnounce << "]|h:" << MSG_COLOR_YELLOW << " initiated server restart timer " << shutdowntime << " sec";

    sWorld.sendMessageToOnlineGms(teamAnnounce.str());
    sGMLog.writefromsession(m_session, "initiated server restart timer %u sec", shutdowntime);

    std::stringstream worldAnnounce;
    worldAnnounce << "Server is restarting in " << shutdowntime << " seconds.";

    sWorld.sendMessageToAll(worldAnnounce.str());
    sWorld.sendAreaTriggerMessage(worldAnnounce.str());

    shutdowntime *= 1000;
    sMaster.m_ShutdownTimer = shutdowntime;
    sMaster.m_ShutdownEvent = true;
    sMaster.m_restartEvent = true;

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// .server reload commands
//.server reload gameobjects
bool ChatCommandHandler::HandleReloadGameobjectsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadGameObjectPropertiesTable();
    greenSystemMessage(m_session, "WorldDB gameobjects tables reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload creatures
bool ChatCommandHandler::HandleReloadCreaturesCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadCreaturePropertiesTable();
    greenSystemMessage(m_session, "WorldDB creature tables reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload areatriggers
bool ChatCommandHandler::HandleReloadAreaTriggersCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadAreaTriggerTable();
    greenSystemMessage(m_session, "WorldDB table 'areatriggers' reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload command_overrides
bool ChatCommandHandler::HandleReloadCommandOverridesCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sCommandTableStorage.loadOverridePermission();

    greenSystemMessage(m_session, "CharactersDB 'command_overrides' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload fishing
bool ChatCommandHandler::HandleReloadFishingCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadFishingTable();
    greenSystemMessage(m_session, "WorldDB 'fishing' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload gossip_menu_option
bool ChatCommandHandler::HandleReloadGossipMenuOptionCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadGossipMenuOptionTable();
    greenSystemMessage(m_session, "WorldDB 'gossip_menu_option' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload graveyards
bool ChatCommandHandler::HandleReloadGraveyardsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadGraveyardsTable();
    greenSystemMessage(m_session, "WorldDB 'graveyards' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload items
bool ChatCommandHandler::HandleReloadItemsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadItemPropertiesTable();
    sMySQLStore.loadItemPropertiesSpellsTable();
    sMySQLStore.loadItemPropertiesStatsTable();
    greenSystemMessage(m_session, "WorldDB table 'items' reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload itempages
bool ChatCommandHandler::HandleReloadItempagesCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadItemPagesTable();
    greenSystemMessage(m_session, "WorldDB 'itempages' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload npc_script_text
bool ChatCommandHandler::HandleReloadNpcScriptTextCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadNpcScriptTextTable();
    greenSystemMessage(m_session, "WorldDB 'npc_script_text' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload npc_gossip_text
bool ChatCommandHandler::HandleReloadNpcTextCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadNpcTextTable();
    greenSystemMessage(m_session, "WorldDB 'npc_gossip_text' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload pet_level_abilities
bool ChatCommandHandler::HandleReloadPetLevelAbilitiesCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadPetLevelAbilitiesTable();
    greenSystemMessage(m_session, "WorldDB 'pet_level_abilities' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload player_xp_for_level
bool ChatCommandHandler::HandleReloadPlayerXpForLevelCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadPlayerXpToLevelTable();
    greenSystemMessage(m_session, "WorldDB 'player_xp_for_level' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload points_of_interest
bool ChatCommandHandler::HandleReloadPointsOfInterestCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadPointsOfInterestTable();
    greenSystemMessage(m_session, "WorldDB 'points_of_interest' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload quests
bool ChatCommandHandler::HandleReloadQuestsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadQuestPropertiesTable();
    greenSystemMessage(m_session, "WorldDB 'quest_properties' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload teleport_coords
bool ChatCommandHandler::HandleReloadTeleportCoordsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadTeleportCoordsTable();
    greenSystemMessage(m_session, "WorldDB 'teleport_coords' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload worldbroadcast
bool ChatCommandHandler::HandleReloadWorldbroadcastCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadBroadcastTable();
    greenSystemMessage(m_session, "WorldDB 'worldbroadcast' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload worldmap_info
bool ChatCommandHandler::HandleReloadWorldmapInfoCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadWorldMapInfoTable();
    greenSystemMessage(m_session, "WorldDB 'worldmap_info' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload worldstring_tables
bool ChatCommandHandler::HandleReloadWorldstringTablesCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadWorldStringsTable();
    greenSystemMessage(m_session, "WorldDB 'worldstring_tables' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reload zoneguards
bool ChatCommandHandler::HandleReloadZoneguardsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadZoneGuardsTable();
    greenSystemMessage(m_session, "WorldDB 'zoneguards' table reloaded in {} ms", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.server reloadscripts
bool ChatCommandHandler::HandleServerReloadScriptsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sScriptMgr.ReloadScriptEngines();
    greenSystemMessage(m_session, "Scripts reloaded in {} my.", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}
