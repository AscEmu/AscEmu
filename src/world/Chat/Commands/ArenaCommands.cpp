/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatHandler.hpp"
#include "Management/ArenaTeam.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Players/Player.hpp"

uint8_t ChatHandler::GetArenaTeamInternalType(uint32_t type, WorldSession* m_session)
{
    uint8_t internal_type;
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
    uint32_t teamType;
    char teamName[1000];

    auto player = GetSelectedPlayer(m_session, true, true);
    if (sscanf(args, "%u %[^\n]", &teamType, teamName) != 2)
    {
        SystemMessage(m_session, "Invalid syntax. Usage: .arena createteam <type> <name>");
        return true;
    }

    uint8_t internalType = GetArenaTeamInternalType(teamType, m_session);
    if (internalType == 10)
        return true;

    if (player == nullptr)
    {
        SystemMessage(m_session, "Selected player not found!");
        return true;
    }

    if (player->isInArenaTeam(internalType))
    {
        RedSystemMessage(m_session, "Player: %s is already in an arena team of that type!", player->getName().c_str());
        return true;
    }

    ArenaTeamEmblem emblem{ .emblemStyle = 22, .emblemColour = 4292133532UL, .borderStyle = 1,
        .borderColour = 4294931722UL, .backgroundColour = 4284906803UL };

    if (auto* const arenaTeam = sObjectMgr.createArenaTeam(internalType, player, teamName, 1500, emblem))
    {
        player->setArenaTeam(arenaTeam->m_type, arenaTeam);
        GreenSystemMessage(m_session, "Arena team created for Player: %s Type: %u", player->getName().c_str(), teamType);
    }

    return true;
}

bool ChatHandler::HandleArenaSetTeamLeader(const char* args, WorldSession* m_session)
{
    uint32_t team_type;

    auto player = GetSelectedPlayer(m_session, true, true);
    if (sscanf(args, "%u", &team_type) != 1)
    {
        SystemMessage(m_session, "Invalid syntax. Usage: .arena setteamleader <type>");
        return true;
    }

    uint8_t internal_type = GetArenaTeamInternalType(team_type, m_session);
    if (internal_type == 10)
        return true;

    if (player == nullptr)
    {
        SystemMessage(m_session, "Selected player not found!");
        return true;
    }

    if (!player->isInArenaTeam(internal_type))
    {
        RedSystemMessage(m_session, "Player: %s is already in an arena team of that type!", player->getName().c_str());
        return true;
    }

    auto arena_team = player->getArenaTeam(internal_type);
    arena_team->setLeader(player->getPlayerInfo());

    GreenSystemMessage(m_session, "Player: %s is now arena team leader for type: %u", player->getName().c_str(), team_type);

    return true;
}

bool ChatHandler::HandleArenaTeamResetAllRatings(const char* /*args*/, WorldSession* /*m_session*/)
{
    sObjectMgr.resetArenaTeamRatings();

    return true;
}
