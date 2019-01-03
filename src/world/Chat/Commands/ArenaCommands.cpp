/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Chat/ChatHandler.hpp"
#include "Objects/ObjectMgr.h"

uint8 ChatHandler::GetArenaTeamInternalType(uint32 type, WorldSession* m_session)
{
    uint8 internal_type;
    switch (type)
    {
        case 2:
            internal_type = 0;
            break;
        case 3:
            internal_type = 1;
            break;
        case 5:
            internal_type = 2;
            break;
        default:
        {
            internal_type = 10;
            RedSystemMessage(m_session, "Invalid arena team type specified! Valid types: 2, 3 and 5.");
        }break;
    }

    return internal_type;
}

bool ChatHandler::HandleArenaCreateTeam(const char* args, WorldSession* m_session)
{
    uint32 team_type;
    char team_name[1000];

    auto player = GetSelectedPlayer(m_session, true, true);
    if (sscanf(args, "%u %[^\n]", &team_type, team_name) != 2)
    {
        SystemMessage(m_session, "Invalid syntax. Usage: .arena createteam <type> <name>");
        return true;
    }

    uint8 internal_type = GetArenaTeamInternalType(team_type, m_session);
    if (internal_type == 10)
        return true;

    if (player == nullptr)
    {
        SystemMessage(m_session, "Selected player not found!");
        return true;
    }

    if (player->m_arenaTeams[internal_type] != NULL)
    {
        RedSystemMessage(m_session, "Player: %s is already in an arena team of that type!", player->getName().c_str());
        return true;
    }

    auto arena_team = new ArenaTeam(uint32(internal_type), objmgr.GenerateArenaTeamId());
    arena_team->m_emblemStyle = 22;
    arena_team->m_emblemColour = 4292133532UL;
    arena_team->m_borderColour = 4294931722UL;
    arena_team->m_borderStyle = 1;
    arena_team->m_backgroundColour = 4284906803UL;
    arena_team->m_leader = player->getGuidLow();
    arena_team->m_name = std::string(team_name);
    arena_team->AddMember(player->getPlayerInfo());

    objmgr.AddArenaTeam(arena_team);

    GreenSystemMessage(m_session, "Arena team created for Player: %s Type: %u", player->getName().c_str(), team_type);

    return true;
}

bool ChatHandler::HandleArenaSetTeamLeader(const char* args, WorldSession* m_session)
{
    uint32 team_type;

    auto player = GetSelectedPlayer(m_session, true, true);
    if (sscanf(args, "%u", &team_type) != 1)
    {
        SystemMessage(m_session, "Invalid syntax. Usage: .arena setteamleader <type>");
        return true;
    }

    uint8 internal_type = GetArenaTeamInternalType(team_type, m_session);
    if (internal_type == 10)
        return true;

    if (player == nullptr)
    {
        SystemMessage(m_session, "Selected player not found!");
        return true;
    }

    if (player->m_arenaTeams[internal_type] == NULL)
    {
        RedSystemMessage(m_session, "Player: %s is already in an arena team of that type!", player->getName().c_str());
        return true;
    }

    auto arena_team = player->m_arenaTeams[internal_type];
    arena_team->SetLeader(player->getPlayerInfo());

    GreenSystemMessage(m_session, "Player: %s is now arena team leader for type: %u", player->getName().c_str(), team_type);

    return true;
}

bool ChatHandler::HandleArenaTeamResetAllRatings(const char* /*args*/, WorldSession* /*m_session*/)
{
    objmgr.ResetArenaTeamRatings();

    return true;
}
