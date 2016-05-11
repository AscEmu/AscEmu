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
