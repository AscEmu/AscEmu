/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

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
                if (!sWorld.gamemaster_listOnlyActiveGMs)
                {
                    online_gm++;
                }
                else
                {
                    if (itr->second->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM))
                        online_gm++;
                }
            }
        }
    }
    objmgr._playerslock.ReleaseReadLock();

    uint32 active_sessions = uint32(sWorld.GetSessionCount());

    GreenSystemMessage(m_session, "Server Revision: |r%sAscEmu %s/%s-%s-%s %s(www.ascemu.org)", MSG_COLOR_WHITE, BUILD_HASH_STR, CONFIG, PLATFORM_TEXT, ARCH, MSG_COLOR_LIGHTBLUE);
    GreenSystemMessage(m_session, "Server Uptime: |r%s", sWorld.GetUptimeString().c_str());
    GreenSystemMessage(m_session, "Active Sessions: |r%u", active_sessions);
    GreenSystemMessage(m_session, "Current GMs: |r%u GMs", online_gm);
    GreenSystemMessage(m_session, "Current Players: |r%u (%u Peak)", online_gm > 0 ? (online_count - online_gm) : online_count, sWorld.PeakSessionCount);
    GreenSystemMessage(m_session, "Active Thread Count: |r%u", ThreadPool.GetActiveThreadCount());
    GreenSystemMessage(m_session, "Free Thread Count: |r%u", ThreadPool.GetFreeThreadCount());
    GreenSystemMessage(m_session, "Average Latency: |r%.3fms", online_count > 0 ? (latency_avg / online_count) : latency_avg);
    GreenSystemMessage(m_session, "CPU Usage: %3.2f %%", sWorld.GetCPUUsage());
    GreenSystemMessage(m_session, "RAM Usage: %6.2f MB", sWorld.GetRAMUsage());
    GreenSystemMessage(m_session, "SQL Query Cache Size (World): |r%u queries delayed", WorldDatabase.GetQueueSize());
    GreenSystemMessage(m_session, "SQL Query Cache Size (Character): |r%u queries delayed", CharacterDatabase.GetQueueSize());
    GreenSystemMessage(m_session, "Socket Count: |r%u", sSocketMgr.GetSocketCount());

    return true;
}

//.server rehash
bool ChatHandler::HandleServerRehashCommand(const char* /*args*/, WorldSession* m_session)
{
    char TeamAnnounce[512];
    snprintf(TeamAnnounce, 512, MSG_COLOR_RED "[Team]" MSG_COLOR_GREEN " |Hplayer:%s|h[%s]|h:" MSG_COLOR_YELLOW " is rehashing config file.", m_session->GetPlayer()->GetName(), m_session->GetPlayer()->GetName());
    sWorld.SendGMWorldText(TeamAnnounce);

    sWorld.Rehash(true);

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
        GreenSystemMessage(m_session, "Player %s saved to DB", player_target->GetName());
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
    uint32 start_time = now();
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

    char TeamAnnounce[512];
    snprintf(TeamAnnounce, 512, MSG_COLOR_RED "[Team]" MSG_COLOR_GREEN " |Hplayer:%s|h[%s]|h:" MSG_COLOR_YELLOW " saved all online players (%u) in %u msec.", m_session->GetPlayer()->GetName(), m_session->GetPlayer()->GetName(), online_count, now() - start_time);
    sWorld.SendGMWorldText(TeamAnnounce);
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
    sWorld.SetMotd(args);

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

    char TeamAnnounce[512];
    snprintf(TeamAnnounce, 512, MSG_COLOR_RED "[Team]" MSG_COLOR_GREEN " |Hplayer:%s|h[%s]|h:" MSG_COLOR_YELLOW " initiated server shutdown timer %u sec", m_session->GetPlayer()->GetName(), m_session->GetPlayer()->GetName(), shutdowntime);
    sWorld.SendGMWorldText(TeamAnnounce);
    sGMLog.writefromsession(m_session, "initiated server shutdown timer %u sec", shutdowntime);

    char msg[500];
    snprintf(msg, 500, "Server is shutting down in %u seconds.", (unsigned int)shutdowntime);
    sWorld.SendWorldText(msg);
    sWorld.SendWorldWideScreenText(msg);

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
        char TeamAnnounce[512];
        snprintf(TeamAnnounce, 512, MSG_COLOR_RED "[Team]" MSG_COLOR_GREEN " |Hplayer:%s|h[%s]|h:" MSG_COLOR_YELLOW " canceled server shutdown!", m_session->GetPlayer()->GetName(), m_session->GetPlayer()->GetName());
        sWorld.SendGMWorldText(TeamAnnounce);
        sGMLog.writefromsession(m_session, "canceled server shutdown");

        char msg[500];
        snprintf(msg, 500, "Server %s cancelled.", (sMaster.m_restartEvent ? "Restart" : "Shutdown"));
        sWorld.SendWorldText(msg);
        sWorld.SendWorldWideScreenText(msg);

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

    char TeamAnnounce[512];
    snprintf(TeamAnnounce, 512, MSG_COLOR_RED "[Team]" MSG_COLOR_GREEN " |Hplayer:%s|h[%s]|h:" MSG_COLOR_YELLOW " initiated server restart timer %u sec", m_session->GetPlayer()->GetName(), m_session->GetPlayer()->GetName(), shutdowntime);
    sWorld.SendGMWorldText(TeamAnnounce);
    sGMLog.writefromsession(m_session, "initiated server restart timer %u sec", shutdowntime);

    char msg[500];
    snprintf(msg, 500, "Server is restarting in %u seconds.", (unsigned int)shutdowntime);
    sWorld.SendWorldText(msg);
    sWorld.SendWorldWideScreenText(msg);

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
    uint32 start_time = getMSTime();
    sMySQLStore.LoadGameObjectPropertiesTable();
    GreenSystemMessage(m_session, "WorldDB gameobjects tables reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload creatures
bool ChatHandler::HandleReloadCreaturesCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadCreaturePropertiesTable();
    GreenSystemMessage(m_session, "WorldDB creature tables reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload areatriggers
bool ChatHandler::HandleReloadAreaTriggersCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadAreaTriggersTable();
    GreenSystemMessage(m_session, "WorldDB table 'areatriggers' reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload command_overrides
bool ChatHandler::HandleReloadCommandOverridesCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sCommandTableStorag.Dealloc();
    sCommandTableStorag.Init();
    sCommandTableStorag.Load();
    GreenSystemMessage(m_session, "CharactersDB 'command_overrides' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload fishing
bool ChatHandler::HandleReloadFishingCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadFishingTable();
    GreenSystemMessage(m_session, "WorldDB 'fishing' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload gossip_menu_option
bool ChatHandler::HandleReloadGossipMenuOptionCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadGossipMenuOptionTable();
    GreenSystemMessage(m_session, "WorldDB 'gossip_menu_option' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload graveyards
bool ChatHandler::HandleReloadGraveyardsCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadGraveyardsTable();
    GreenSystemMessage(m_session, "WorldDB 'graveyards' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload items
bool ChatHandler::HandleReloadItemsCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadItemPropertiesTable();
    GreenSystemMessage(m_session, "WorldDB table 'items' reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload itempages
bool ChatHandler::HandleReloadItempagesCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadItemPagesTable();
    GreenSystemMessage(m_session, "WorldDB 'itempages' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload npc_script_text
bool ChatHandler::HandleReloadNpcScriptTextCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadNpcScriptTextTable();
    GreenSystemMessage(m_session, "WorldDB 'npc_script_text' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload npc_text
bool ChatHandler::HandleReloadNpcTextCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadNpcTextTable();
    GreenSystemMessage(m_session, "WorldDB 'npc_text' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload player_xp_for_level
bool ChatHandler::HandleReloadPlayerXpForLevelCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    objmgr.LoadXpToLevelTable();
    GreenSystemMessage(m_session, "WorldDB 'player_xp_for_level' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload points_of_interest
bool ChatHandler::HandleReloadPointsOfInterestCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadPointOfInterestTable();
    GreenSystemMessage(m_session, "WorldDB 'points_of_interest' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload quests
bool ChatHandler::HandleReloadQuestsCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadQuestPropertiesTable();
    GreenSystemMessage(m_session, "WorldDB 'quest_properties' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload teleport_coords
bool ChatHandler::HandleReloadTeleportCoordsCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadTeleportCoordsTable();
    GreenSystemMessage(m_session, "WorldDB 'teleport_coords' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload worldbroadcast
bool ChatHandler::HandleReloadWorldbroadcastCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadWorldBroadcastTable();
    GreenSystemMessage(m_session, "WorldDB 'worldbroadcast' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload worldmap_info
bool ChatHandler::HandleReloadWorldmapInfoCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadWorldMapInfoTable();
    GreenSystemMessage(m_session, "WorldDB 'worldmap_info' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload worldstring_tables
bool ChatHandler::HandleReloadWorldstringTablesCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadWorldStringsTable();
    GreenSystemMessage(m_session, "WorldDB 'worldstring_tables' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reload zoneguards
bool ChatHandler::HandleReloadZoneguardsCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 start_time = getMSTime();
    sMySQLStore.LoadZoneGuardsTable();
    GreenSystemMessage(m_session, "WorldDB 'zoneguards' table reloaded in %u ms", getMSTime() - start_time);
    return true;
}

//.server reloadscripts
bool ChatHandler::HandleServerReloadScriptsCommand(const char* /*args*/, WorldSession* m_session)
{
    sScriptMgr.ReloadScriptEngines();
    GreenSystemMessage(m_session, "Scripts reloaded!");
    return true;
}
