/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Battleground/Battleground.h"
#include "Chat/ChatHandler.hpp"
#include "Server/WorldSession.h"

//.battleground forceinitqueue
bool ChatHandler::HandleBGForceInitQueueCommand(const char* /*args*/, WorldSession* m_session)
{
    BattlegroundManager.EventQueueUpdate(true);

    SystemMessage(m_session, "Forcing initialization of all battlegrounds. Done.");

    return true;
}

//.battleground getqueue
bool ChatHandler::HandleBGGetQueueCommand(const char* /*args*/, WorldSession* m_session)
{
    BattlegroundManager.HandleGetBattlegroundQueueCommand(m_session);

    SystemMessage(m_session, "Getting battleground queue. Done.");

    return true;
}

//.battleground info
bool ChatHandler::HandleBGInfoCommand(const char* /*args*/, WorldSession* m_session)
{
    RedSystemMessage(m_session, ".battleground info command not implemented yet!");

    return true;
}

//.battleground leave
bool ChatHandler::HandleBGLeaveCommand(const char* /*args*/, WorldSession* m_session)
{
    if (!m_session->GetPlayer()->m_bg)
    {
        RedSystemMessage(m_session, "You are not in a battleground.");
        return true;
    }

    m_session->GetPlayer()->m_bg->Close();

    return true;
}

//.battleground menu
bool ChatHandler::HandleBGMenuCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    uint32 type = atoi(args);
    if (type >= BATTLEGROUND_NUM_TYPES)
        return false;

    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    BattlegroundManager.HandleBattlegroundListPacket(selected_player->GetSession(), type);

    return true;
}

//.battleground pause
bool ChatHandler::HandleBGPauseCommand(const char* /*args*/, WorldSession* m_session)
{
    RedSystemMessage(m_session, ".battleground pause command not implemented yet!");

    return true;
}

//.battleground playsound
bool ChatHandler::HandleBGPlaySoundCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    if (!m_session->GetPlayer()->m_bg)
    {
        RedSystemMessage(m_session, "You are not in a battleground.");
        return true;
    }

    m_session->GetPlayer()->m_bg->PlaySoundToAll(atoi(args));
    return true;
}

//.battleground sendstatus
bool ChatHandler::HandleBGSendStatusCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    uint32 type = atoi(args);
    BattlegroundManager.SendBattlefieldStatus(m_session->GetPlayer(), BGSTATUS_INQUEUE, type, 0, 0, m_session->GetPlayer()->GetMapId(), 0);
    return true;
}

//.battleground setscore
bool ChatHandler::HandleBGSetScoreCommand(const char* /*args*/, WorldSession* m_session)
{
    RedSystemMessage(m_session, ".battleground setscore command not implemented yet!");

    return true;
}

//.battleground setworldstate
bool ChatHandler::HandleBGSetWorldStateCommand(const char* args, WorldSession* m_session)
{
    uint32 id, val;
    if (sscanf(args, "%u %u", &id, &val) != 2)
    {
        RedSystemMessage(m_session, ".battleground setworldstate rquires at least 2 values!");
        RedSystemMessage(m_session, " e.g. .battleground setworldstate <worldstate_id> <value>");
        return true;
    }

    if (m_session->GetPlayer()->m_bg)
        m_session->GetPlayer()->m_bg->SetWorldState(id, val);

    return true;
}

//.battleground setworldstates
bool ChatHandler::HandleBGSetWorldStatesCommand(const char* args, WorldSession* m_session)
{
    uint32 first, last, val;
    if (sscanf(args, "%u %u %u", &first, &last, &val) != 3)
    {
        RedSystemMessage(m_session, ".battleground setworldstates rquires at least 3 values!");
        RedSystemMessage(m_session, " e.g. .battleground setworldstates <start_worldstate_id> <end_worldstate_id> <value>");
        return true;
    }

    if (m_session->GetPlayer()->m_bg)
    {
        for (uint32 i = first; i < last; i++)
        {
            m_session->GetPlayer()->m_bg->SetWorldState(i, val);
        }
    }

    return true;
}

//.battleground start
bool ChatHandler::HandleBGStartCommand(const char* /*args*/, WorldSession* m_session)
{
    if (!m_session->GetPlayer()->m_bg)
    {
        RedSystemMessage(m_session, "You are not in a battleground.");
        return true;
    }

    m_session->GetPlayer()->m_bg->SendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0, m_session->LocalizedWorldSrv(49), m_session->LocalizedWorldSrv(m_session->GetPlayer()->m_bg->GetNameID()));

    sEventMgr.RemoveEvents(m_session->GetPlayer()->m_bg, EVENT_BATTLEGROUND_COUNTDOWN);

    m_session->GetPlayer()->m_bg->StartBattleground();

    return true;
}
