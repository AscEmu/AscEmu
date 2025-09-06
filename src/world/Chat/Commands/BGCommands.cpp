/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatDefines.hpp"
#include "Chat/ChatCommandHandler.hpp"
#include "Management/Battleground/Battleground.hpp"
#include "Management/Battleground/BattlegroundMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Server/WorldSession.h"
#include "Storage/WorldStrings.h"

//.battleground forceinitqueue
bool ChatCommandHandler::HandleBGForceInitQueueCommand(const char* /*args*/, WorldSession* m_session)
{
    sBattlegroundManager.eventQueueUpdate(true);

    systemMessage(m_session, "Forcing initialization of all battlegrounds. Done.");

    return true;
}

//.battleground getqueue
bool ChatCommandHandler::HandleBGGetQueueCommand(const char* /*args*/, WorldSession* m_session)
{
    sBattlegroundManager.handleGetBattlegroundQueueCommand(m_session);

    systemMessage(m_session, "Getting battleground queue. Done.");

    return true;
}

//.battleground info
bool ChatCommandHandler::HandleBGInfoCommand(const char* /*args*/, WorldSession* m_session)
{
    redSystemMessage(m_session, ".battleground info command not implemented yet!");

    return true;
}

//.battleground leave
bool ChatCommandHandler::HandleBGLeaveCommand(const char* /*args*/, WorldSession* m_session)
{
    if (!m_session->GetPlayer()->getBattleground())
    {
        redSystemMessage(m_session, "You are not in a battleground.");
        return true;
    }

    m_session->GetPlayer()->getBattleground()->close();

    return true;
}

//.battleground menu
bool ChatCommandHandler::HandleBGMenuCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    uint32_t type = atoi(args);
    if (type >= BATTLEGROUND_NUM_TYPES)
        return false;

    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

#if VERSION_STRING <= WotLK
    sBattlegroundManager.handleBattlegroundListPacket(selected_player->getSession(), type);
#else
    WoWGuid guid;
    guid.Init(uint64_t(0));
    sBattlegroundManager.handleBattlegroundListPacket(guid, selected_player->getSession(), type);
#endif

    return true;
}

//.battleground pause
bool ChatCommandHandler::HandleBGPauseCommand(const char* /*args*/, WorldSession* m_session)
{
    redSystemMessage(m_session, ".battleground pause command not implemented yet!");

    return true;
}

//.battleground playsound
bool ChatCommandHandler::HandleBGPlaySoundCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    if (!m_session->GetPlayer()->getBattleground())
    {
        redSystemMessage(m_session, "You are not in a battleground.");
        return true;
    }

    m_session->GetPlayer()->getBattleground()->playSoundToAll(atoi(args));
    return true;
}

//.battleground sendstatus
bool ChatCommandHandler::HandleBGSendStatusCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    uint32_t type = atoi(args);
    sBattlegroundManager.sendBattlefieldStatus(m_session->GetPlayer(), BattlegroundDef::STATUS_INQUEUE, type, 0, 0, m_session->GetPlayer()->GetMapId(), 0);
    return true;
}

//.battleground setscore
bool ChatCommandHandler::HandleBGSetScoreCommand(const char* /*args*/, WorldSession* m_session)
{
    redSystemMessage(m_session, ".battleground setscore command not implemented yet!");

    return true;
}

//.battleground setworldstate
bool ChatCommandHandler::HandleBGSetWorldStateCommand(const char* args, WorldSession* m_session)
{
    uint32_t id, val;
    if (sscanf(args, "%u %u", &id, &val) != 2)
    {
        redSystemMessage(m_session, ".battleground setworldstate rquires at least 2 values!");
        redSystemMessage(m_session, " e.g. .battleground setworldstate <worldstate_id> <value>");
        return true;
    }

    if (m_session->GetPlayer()->getBattleground())
        m_session->GetPlayer()->getBattleground()->setWorldState(id, val);

    return true;
}

//.battleground setworldstates
bool ChatCommandHandler::HandleBGSetWorldStatesCommand(const char* args, WorldSession* m_session)
{
    uint32_t first, last, val;
    if (sscanf(args, "%u %u %u", &first, &last, &val) != 3)
    {
        redSystemMessage(m_session, ".battleground setworldstates rquires at least 3 values!");
        redSystemMessage(m_session, " e.g. .battleground setworldstates <start_worldstate_id> <end_worldstate_id> <value>");
        return true;
    }

    if (m_session->GetPlayer()->getBattleground())
        for (uint32_t i = first; i < last; i++)
            m_session->GetPlayer()->getBattleground()->setWorldState(i, val);

    return true;
}

//.battleground start
bool ChatCommandHandler::HandleBGStartCommand(const char* /*args*/, WorldSession* m_session)
{
    if (!m_session->GetPlayer()->getBattleground())
    {
        redSystemMessage(m_session, "You are not in a battleground.");
        return true;
    }

    m_session->GetPlayer()->getBattleground()->sendChatMessage(CHAT_MSG_BG_EVENT_NEUTRAL, 0,
        m_session->LocalizedWorldSrv(SS_THE_BATTLE_FOR_HAS_BEGUN),
        m_session->LocalizedWorldSrv(m_session->GetPlayer()->getBattleground()->GetNameID()));

    sEventMgr.RemoveEvents(m_session->GetPlayer()->getBattleground(), EVENT_BATTLEGROUND_COUNTDOWN);

    m_session->GetPlayer()->getBattleground()->startBattleground();

    return true;
}
