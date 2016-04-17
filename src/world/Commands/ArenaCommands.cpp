/*
Copyright (c) 2015 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

bool ChatHandler::HandleArenaCreateTeam(const char* args, WorldSession* m_session)
{
    uint8 team_type;
    uint8 internal_type;
    char team_name[1000];

    auto player = getSelectedChar(m_session, true);
    if (sscanf(args, "%u %[^\n]", &team_type, team_name) != 2)
    {
        SystemMessage(m_session, "Invalid syntax. Usage: .arena createteam <type> <name>");
        return true;
    }

    switch (team_type)
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
            SystemMessage(m_session, "Invalid arena team type specified! Valid types: 2, 3 and 5.");
            return true;
    }

    if (player == nullptr)
    {
        SystemMessage(m_session, "Selected player not found!");
        return true;
    }

    if ( player->m_arenaTeams[internal_type] != NULL)
    {
        SystemMessage(m_session, "Player: %s is already in an arena team of that type!", player->GetName());
        return true;
    }

    auto arena_team = new ArenaTeam(internal_type, objmgr.GenerateArenaTeamId());
    arena_team->m_emblemStyle = 22;
    arena_team->m_emblemColour = 4292133532UL;
    arena_team->m_borderColour = 4294931722UL;
    arena_team->m_borderStyle = 1;
    arena_team->m_backgroundColour = 4284906803UL;
    arena_team->m_leader = player->GetLowGUID();
    arena_team->m_name = std::string(team_name);
    arena_team->AddMember(player->getPlayerInfo());

    objmgr.AddArenaTeam(arena_team);

    SystemMessage(m_session, "Arena team created for Player: %s Type: %u", player->GetName(), team_type);

    return true;
}
