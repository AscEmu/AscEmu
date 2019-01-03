/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/WorldSocket.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Server/Master.h"

//.server info
bool ChatHandler::HandleServerInfoCommand(const char* /*args*/, WorldSession* m_session)
{
    uint16 online_gm = 0;
    uint16 online_count = 0;
    float latency_avg = 0;

    objmgr._playerslock.AcquireReadLock();
    for (PlayerStorageMap::const_iterator itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        if (itr->second->GetSession())
        {
            online_count++;
            latency_avg += itr->second->GetSession()->GetLatency();
            if (itr->second->GetSession()->GetPermissionCount())
            {
                if (!worldConfig.gm.listOnlyActiveGms)
                {
                    online_gm++;
                }
                else
                {
                    if (itr->second->isGMFlagSet())
                        online_gm++;
                }
            }
        }
    }
    objmgr._playerslock.ReleaseReadLock();

    uint32 active_sessions = uint32(sWorld.getSessionCount());

    GreenSystemMessage(m_session, "Server Revision: |r%sAscEmu %s/%s-%s-%s %s(www.ascemu.org)", MSG_COLOR_WHITE, BUILD_HASH_STR, CONFIG, PLATFORM_TEXT, ARCH, MSG_COLOR_LIGHTBLUE);
    GreenSystemMessage(m_session, "Server Uptime: |r%s", sWorld.getWorldUptimeString().c_str());
    GreenSystemMessage(m_session, "Active Sessions: |r%u", active_sessions);
    GreenSystemMessage(m_session, "Current GMs: |r%u GMs", online_gm);
    GreenSystemMessage(m_session, "Current Players: |r%u (%u Peak)", online_gm > 0 ? (online_count - online_gm) : online_count, sWorld.getPeakSessionCount());
    GreenSystemMessage(m_session, "Active Thread Count: |r%u", ThreadPool.GetActiveThreadCount());
    GreenSystemMessage(m_session, "Free Thread Count: |r%u", ThreadPool.GetFreeThreadCount());
    GreenSystemMessage(m_session, "Average Latency: |r%.3fms", online_count > 0 ? (latency_avg / online_count) : latency_avg);
    GreenSystemMessage(m_session, "CPU Usage: %3.2f %%", sWorld.getCPUUsage());
    GreenSystemMessage(m_session, "RAM Usage: %6.2f MB", sWorld.getRAMUsage());
    GreenSystemMessage(m_session, "SQL Query Cache Size (World): |r%u queries delayed", WorldDatabase.GetQueueSize());
    GreenSystemMessage(m_session, "SQL Query Cache Size (Character): |r%u queries delayed", CharacterDatabase.GetQueueSize());
    GreenSystemMessage(m_session, "Socket Count: |r%u", sSocketMgr.GetSocketCount());

    return true;
}

//.server rehash
bool ChatHandler::HandleServerRehashCommand(const char* /*args*/, WorldSession* m_session)
{
    std::stringstream teamAnnounce;
    teamAnnounce << MSG_COLOR_RED << "[Team]" << MSG_COLOR_GREEN << " |Hplayer:" << m_session->GetPlayer()->getName().c_str();
    teamAnnounce << "|h[" << m_session->GetPlayer()->getName().c_str() << "]|h:" << MSG_COLOR_YELLOW << " is rehashing config file.";

    sWorld.sendMessageToOnlineGms(teamAnnounce.str());

    sWorld.loadWorldConfigValues(true);

    return true;
}

//.server save
bool ChatHandler::HandleServerSaveCommand(const char* args, WorldSession* m_session)
{
    Player* player_target = nullptr;

    if (!args)
    {
        player_target = GetSelectedPlayer(m_session, false, false);
        if (player_target == nullptr)
        {
            RedSystemMessage(m_session, "You need to target or name a player!");
            RedSystemMessage(m_session, "Use: .server save (on a targeted player)");
            RedSystemMessage(m_session, "or: .server save <playername>");
            return true;
        }
    }
    else
    {
        player_target = objmgr.GetPlayer(args, false);
        if (player_target == nullptr)
        {
            RedSystemMessage(m_session, "A player with name %s is not online / does not exist!", args);
            return true;
        }
    }


    if (player_target->m_nextSave < 180000)
    {
        player_target->SaveToDB(false);
        GreenSystemMessage(m_session, "Player %s saved to DB", player_target->getName().c_str());
    }
    else
    {
        RedSystemMessage(m_session, "You can only save once every 3 minutes.");
    }

    return true;
}

//.server saveall
bool ChatHandler::HandleServerSaveAllCommand(const char* /*args*/, WorldSession* m_session)
{
    auto start_time = Util::TimeNow();
    uint32 online_count = 0;

    objmgr._playerslock.AcquireReadLock();
    for (PlayerStorageMap::const_iterator itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        if (itr->second->GetSession())
        {
            itr->second->SaveToDB(false);
            online_count++;
        }
    }

    objmgr._playerslock.ReleaseReadLock();

    std::stringstream teamAnnounce;
    teamAnnounce << MSG_COLOR_RED << "[Team]" << MSG_COLOR_GREEN << " |Hplayer:" << m_session->GetPlayer()->getName().c_str() << "|h[";
    teamAnnounce << m_session->GetPlayer()->getName().c_str() << "]|h:" << MSG_COLOR_YELLOW << " saved all online players (" << online_count;
    teamAnnounce << ") in " << Util::GetTimeDifferenceToNow(start_time) << " msec.";

    sWorld.sendMessageToOnlineGms(teamAnnounce.str());

    sGMLog.writefromsession(m_session, "saved all online players");

    return true;
}

//.server setmotd
bool ChatHandler::HandleServerSetMotdCommand(const char* args, WorldSession* m_session)
{
    if (!args || strlen(args) < 5)
    {
        RedSystemMessage(m_session, "You must specify a message.");
        RedSystemMessage(m_session, ".server setmotd <message>");
        return true;
    }

    GreenSystemMessage(m_session, "Motd has been set to: %s", args);
    sGMLog.writefromsession(m_session, "Set MOTD to %s", args);
    worldConfig.setMessageOfTheDay(args);

    return true;
}

//.server shutdown
bool ChatHandler::HandleServerShutdownCommand(const char* args, WorldSession* m_session)
{
    uint32 shutdowntime;
    if (!args)
        shutdowntime = 30;
    else
        shutdowntime = atol(args);

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
bool ChatHandler::HandleServerCancelShutdownCommand(const char* /*args*/, WorldSession* m_session)
{
    if (sMaster.m_ShutdownEvent == false)
    {
        RedSystemMessage(m_session, "There is no Shutdown/Restart to cancel!");
        return true;
    }
    else
    {
        std::stringstream teamAnnounce;
        teamAnnounce << MSG_COLOR_RED << "[Team]" << MSG_COLOR_GREEN << " |Hplayer:" << m_session->GetPlayer()->getName().c_str();
        teamAnnounce << "|h[" << m_session->GetPlayer()->getName().c_str() << "]|h:" << MSG_COLOR_YELLOW << " canceled server shutdown!";

        sWorld.sendMessageToOnlineGms(teamAnnounce.str());
        sGMLog.writefromsession(m_session, "canceled server shutdown");

        std::stringstream worldAnnounce;
        worldAnnounce << "Server " << (sMaster.m_restartEvent ? "Restart" : "Shutdown") << " cancelled.";

        sWorld.sendMessageToAll(worldAnnounce.str());
        sWorld.sendAreaTriggerMessage(worldAnnounce.str());

        sMaster.m_ShutdownTimer = 5000;
        sMaster.m_ShutdownEvent = false;
        sMaster.m_restartEvent = false;
    }

    return true;
}

//.server restart
bool ChatHandler::HandleServerRestartCommand(const char* args, WorldSession* m_session)
{
    uint32 shutdowntime;
    if (!args)
        shutdowntime = 30;
    else
        shutdowntime = atol(args);

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
bool ChatHandler::HandleReloadGameobjectsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadGameObjectPropertiesTable();
    GreenSystemMessage(m_session, "WorldDB gameobjects tables reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload creatures
bool ChatHandler::HandleReloadCreaturesCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadCreaturePropertiesTable();
    GreenSystemMessage(m_session, "WorldDB creature tables reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload areatriggers
bool ChatHandler::HandleReloadAreaTriggersCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadAreaTriggerTable();
    GreenSystemMessage(m_session, "WorldDB table 'areatriggers' reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload command_overrides
bool ChatHandler::HandleReloadCommandOverridesCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sCommandTableStorag.Dealloc();
    sCommandTableStorag.Init();
    sCommandTableStorag.Load();
    GreenSystemMessage(m_session, "CharactersDB 'command_overrides' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload fishing
bool ChatHandler::HandleReloadFishingCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadFishingTable();
    GreenSystemMessage(m_session, "WorldDB 'fishing' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload gossip_menu_option
bool ChatHandler::HandleReloadGossipMenuOptionCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadGossipMenuOptionTable();
    GreenSystemMessage(m_session, "WorldDB 'gossip_menu_option' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload graveyards
bool ChatHandler::HandleReloadGraveyardsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadGraveyardsTable();
    GreenSystemMessage(m_session, "WorldDB 'graveyards' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload items
bool ChatHandler::HandleReloadItemsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadItemPropertiesTable();
    GreenSystemMessage(m_session, "WorldDB table 'items' reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload itempages
bool ChatHandler::HandleReloadItempagesCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadItemPagesTable();
    GreenSystemMessage(m_session, "WorldDB 'itempages' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload npc_script_text
bool ChatHandler::HandleReloadNpcScriptTextCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadNpcScriptTextTable();
    GreenSystemMessage(m_session, "WorldDB 'npc_script_text' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload npc_text
bool ChatHandler::HandleReloadNpcTextCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadNpcTextTable();
    GreenSystemMessage(m_session, "WorldDB 'npc_text' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload pet_level_abilities
bool ChatHandler::HandleReloadPetLevelAbilitiesCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadPetLevelAbilitiesTable();
    GreenSystemMessage(m_session, "WorldDB 'pet_level_abilities' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload player_xp_for_level
bool ChatHandler::HandleReloadPlayerXpForLevelCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadPlayerXpToLevelTable();
    GreenSystemMessage(m_session, "WorldDB 'player_xp_for_level' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload points_of_interest
bool ChatHandler::HandleReloadPointsOfInterestCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadPointsOfInterestTable();
    GreenSystemMessage(m_session, "WorldDB 'points_of_interest' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload quests
bool ChatHandler::HandleReloadQuestsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadQuestPropertiesTable();
    GreenSystemMessage(m_session, "WorldDB 'quest_properties' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload teleport_coords
bool ChatHandler::HandleReloadTeleportCoordsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadTeleportCoordsTable();
    GreenSystemMessage(m_session, "WorldDB 'teleport_coords' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload worldbroadcast
bool ChatHandler::HandleReloadWorldbroadcastCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadBroadcastTable();
    GreenSystemMessage(m_session, "WorldDB 'worldbroadcast' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload worldmap_info
bool ChatHandler::HandleReloadWorldmapInfoCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadWorldMapInfoTable();
    GreenSystemMessage(m_session, "WorldDB 'worldmap_info' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload worldstring_tables
bool ChatHandler::HandleReloadWorldstringTablesCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadWorldStringsTable();
    GreenSystemMessage(m_session, "WorldDB 'worldstring_tables' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reload zoneguards
bool ChatHandler::HandleReloadZoneguardsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sMySQLStore.loadZoneGuardsTable();
    GreenSystemMessage(m_session, "WorldDB 'zoneguards' table reloaded in %u ms", Util::GetTimeDifferenceToNow(startTime));
    return true;
}

//.server reloadscripts
bool ChatHandler::HandleServerReloadScriptsCommand(const char* /*args*/, WorldSession* m_session)
{
    auto startTime = Util::TimeNow();
    sScriptMgr.ReloadScriptEngines();
    GreenSystemMessage(m_session, "Scripts reloaded in %u my.", Util::GetTimeDifferenceToNow(startTime));
    return true;
}
