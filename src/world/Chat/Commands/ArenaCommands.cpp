/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatHandler.hpp"
#include "Management/ArenaTeam.hpp"
#include "Management/ObjectMgr.h"

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

    auto arenaTeam = std::make_shared<ArenaTeam>(internalType, sObjectMgr.GenerateArenaTeamId());
    arenaTeam->m_emblem.emblemStyle = 22;
    arenaTeam->m_emblem.emblemColour = 4292133532UL;
    arenaTeam->m_emblem.borderColour = 4294931722UL;
    arenaTeam->m_emblem.borderStyle = 1;
    arenaTeam->m_emblem.backgroundColour = 4284906803UL;
    arenaTeam->m_leader = player->getGuidLow();
    arenaTeam->m_name = std::string(teamName);
    arenaTeam->addMember(player->getPlayerInfo());

    sObjectMgr.addArenaTeam(arenaTeam);

    GreenSystemMessage(m_session, "Arena team created for Player: %s Type: %u", player->getName().c_str(), teamType);

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
