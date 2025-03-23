/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Server/WorldSession.h"
#include "Management/ObjectMgr.hpp"
#include "Server/Packets/MsgInspectArenaTeams.h"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Packets/CmsgArenaTeamQuery.h"
#include "Server/Packets/CmsgArenaTeamInvite.h"
#include "Server/Packets/CmsgArenaTeamRemove.h"
#include "Server/Packets/CmsgArenaTeamLeave.h"
#include "Server/Packets/CmsgArenaTeamDisband.h"
#include "Server/Packets/CmsgArenaTeamLeader.h"
#include "Server/Packets/CmsgArenaTeamRoster.h"
#include "Server/Packets/SmsgArenaTeamStats.h"
#include "Server/Packets/SmsgArenaTeamQueryResponse.h"
#include "Server/Packets/SmsgArenaTeamInvite.h"
#include "Server/Packets/SmsgArenaTeamRooster.h"
#include "Server/Packets/SmsgMessageChat.h"

using namespace AscEmu::Packets;

void WorldSession::handleArenaTeamQueryOpcode(WorldPacket& recvPacket)
{
    CmsgArenaTeamQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (auto arenaTeam = sObjectMgr.getArenaTeamById(srlPacket.teamId))
    {
        SendPacket(SmsgArenaTeamQueryResponse(arenaTeam->m_id, arenaTeam->m_name,
            arenaTeam->getPlayersPerTeam(), arenaTeam->m_emblem).serialise().get());

        SendPacket(SmsgArenaTeamStats(arenaTeam->m_id, arenaTeam->m_stats).serialise().get());
    }
}

void WorldSession::handleArenaTeamAddMemberOpcode(WorldPacket& recvPacket)
{
    CmsgArenaTeamInvite srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    auto arenaTeam = sObjectMgr.getArenaTeamById(srlPacket.teamId);
    if (arenaTeam == nullptr)
        return;

    if (!arenaTeam->isMember(GetPlayer()->getGuidLow()))
    {
        GetPlayer()->softDisconnect();
        return;
    }

    auto player = sObjectMgr.getPlayer(srlPacket.playerName.c_str(), false);
    if (player == nullptr)
    {
        SystemMessage("Player `%s` is non-existent or not online.", srlPacket.playerName.c_str());
        return;
    }

    if (arenaTeam->m_leader != _player->getGuidLow())
    {
        SystemMessage("You are not the captain of this arena team.");
        return;
    }

    if (player->getLevel() < PLAYER_ARENA_MIN_LEVEL)
    {
        SystemMessage("Player must be level %u to join an arena team.", PLAYER_ARENA_MIN_LEVEL);
        return;
    }

    if (player->getArenaTeam(arenaTeam->m_type) != nullptr)
    {
        SystemMessage("That player is already in an arena team of this type.");
        return;
    }

    if (player->getInviteArenaTeamId() != 0)
    {
        SystemMessage("That player is already invited to an arena team");
        return;
    }

    if (player->getTeam() != _player->getTeam() && !HasGMPermissions())
    {
        SystemMessage("That player is a member of a different faction.");
        return;
    }

    player->setInviteArenaTeamId(_player->getArenaTeam(arenaTeam->m_type)->m_id);

    player->sendPacket(SmsgArenaTeamInvite(_player->getName(), _player->getArenaTeam(arenaTeam->m_type)->m_name).serialise().get());
}

void WorldSession::handleArenaTeamRemoveMemberOpcode(WorldPacket& recvPacket)
{
    CmsgArenaTeamRemove srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    auto arenaTeam = sObjectMgr.getArenaTeamById(srlPacket.teamId);
    if (arenaTeam == nullptr)
    {
        GetPlayer()->softDisconnect();
        return;
    }

    const auto slot = arenaTeam->m_type;

    if ((arenaTeam = _player->getArenaTeam(slot)) == nullptr)
    {
        SystemMessage("You are not in an arena team of this type.");
        return;
    }

    if (arenaTeam->m_leader != _player->getGuidLow())
    {
        SystemMessage("You are not the leader of this team.");
        return;
    }

    const auto playerInfo = sObjectMgr.getCachedCharacterInfoByName(srlPacket.playerName);
    if (playerInfo == nullptr)
    {
        SystemMessage("That player cannot be found.");
        return;
    }

    if (!arenaTeam->isMember(playerInfo->guid))
    {
        SystemMessage("That player is not in your arena team.");
        return;
    }

    if (arenaTeam->removeMember(playerInfo))
    {
        char buffer[1024];
        snprintf(buffer, 1024, "%s was removed from the arena team '%s'.", playerInfo->name.c_str(), arenaTeam->m_name.c_str());

        arenaTeam->sendPacket(SmsgMessageChat(SystemMessagePacket(buffer)).serialise().get());

        SystemMessage("Removed %s from the arena team '%s'.", playerInfo->name.c_str(), arenaTeam->m_name.c_str());
    }
}

void WorldSession::handleArenaTeamInviteAcceptOpcode(WorldPacket& /*recvPacket*/)
{
    if (_player->getInviteArenaTeamId() == 0)
    {
        SystemMessage("You have not been invited into another arena team.");
        return;
    }

    auto arenaTeam = sObjectMgr.getArenaTeamById(_player->getInviteArenaTeamId());
    if (arenaTeam == nullptr)
    {
        SystemMessage("That arena team no longer exists.");
        return;
    }

    if (arenaTeam->m_memberCount >= arenaTeam->m_slots)
    {
        SystemMessage("That team is now full.");
        return;
    }

    if (_player->getArenaTeam(arenaTeam->m_type) != nullptr)
    {
        SystemMessage("You have already been in an arena team of that size.");
        return;
    }

    if (arenaTeam->addMember(_player->m_playerInfo))
    {
        _player->setArenaTeam(arenaTeam->m_type, arenaTeam);

        char buffer[1024];
        snprintf(buffer, 1024, "%s joined the arena team, '%s'.", _player->getName().c_str(), arenaTeam->m_name.c_str());

        arenaTeam->sendPacket(SmsgMessageChat(SystemMessagePacket(buffer)).serialise().get());
    }
    else
    {
        SendNotification("Internal error.");
    }
}

void WorldSession::handleArenaTeamInviteDenyOpcode(WorldPacket& /*recvPacket*/)
{
    if (_player->getInviteArenaTeamId() == 0)
    {
        SystemMessage("You were not invited.");
        return;
    }

    const auto* team = sObjectMgr.getArenaTeamById(_player->getInviteArenaTeamId());
    if (team == nullptr)
        return;

    if (const auto player = sObjectMgr.getPlayer(team->m_leader))
        player->getSession()->SystemMessage("%s denied your arena team invitation for %s.", _player->getName().c_str(), team->m_name.c_str());
}

void WorldSession::handleArenaTeamLeaveOpcode(WorldPacket& recvPacket)
{
    CmsgArenaTeamLeave srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    auto arenaTeam = sObjectMgr.getArenaTeamById(srlPacket.teamId);
    if (arenaTeam == nullptr)
    {
        GetPlayer()->softDisconnect();
        return;
    }

    if ((arenaTeam = _player->getArenaTeam(arenaTeam->m_type)) == nullptr)
    {
        SystemMessage("You are not in an arena team of this type.");
        return;
    }

    if (arenaTeam->m_leader == _player->getGuidLow() && arenaTeam->m_memberCount == 1)
    {
        arenaTeam->destroy();
        return;
    }

    if (arenaTeam->m_leader == _player->getGuidLow())
    {
        SystemMessage("You cannot leave the team yet, promote someone else to captain first.");
        return;
    }

    if (arenaTeam->removeMember(_player->m_playerInfo))
    {
        char buffer[1024];
        snprintf(buffer, 1024, "%s left the arena team, '%s'.", _player->getName().c_str(), arenaTeam->m_name.c_str());

        arenaTeam->sendPacket(SmsgMessageChat(SystemMessagePacket(buffer)).serialise().get());

        SystemMessage("You have left the arena team, '%s'.", arenaTeam->m_name.c_str());
    }
}

void WorldSession::handleArenaTeamDisbandOpcode(WorldPacket& recvPacket)
{
    CmsgArenaTeamDisband srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    auto arenaTeam = sObjectMgr.getArenaTeamById(srlPacket.teamId);
    if (arenaTeam == nullptr)
    {
        GetPlayer()->softDisconnect();
        return;
    }

    if ((arenaTeam = _player->getArenaTeam(arenaTeam->m_type)) == nullptr)
    {
        SystemMessage("You are not in an arena team of this type.");
        return;
    }

    if (arenaTeam->m_leader != _player->getGuidLow())
    {
        SystemMessage("You aren't the captain of this team.");
        return;
    }

    arenaTeam->destroy();
}

void WorldSession::handleArenaTeamPromoteOpcode(WorldPacket& recvPacket)
{
    CmsgArenaTeamLeader srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    auto arenaTeam = sObjectMgr.getArenaTeamById(srlPacket.teamId);
    if (arenaTeam == nullptr)
    {
        GetPlayer()->softDisconnect();
        return;
    }

    const auto slot = arenaTeam->m_type;

    if (slot >= NUM_ARENA_TEAM_TYPES)
        return;

    if ((arenaTeam = _player->getArenaTeam(slot)) == nullptr)
    {
        SystemMessage("You are not in an arena team of this type.");
        return;
    }

    if (arenaTeam->m_leader != _player->getGuidLow())
    {
        SystemMessage("You aren't the captain of this team.");
        return;
    }

    const auto playerInfo = sObjectMgr.getCachedCharacterInfoByName(srlPacket.playerName);
    if (playerInfo == nullptr)
    {
        SystemMessage("That player cannot be found.");
        return;
    }

    if (!arenaTeam->isMember(playerInfo->guid))
    {
        SystemMessage("That player is not a member of your arena team.");
        return;
    }

    arenaTeam->setLeader(playerInfo);
}

void WorldSession::handleArenaTeamRosterOpcode(WorldPacket& recvPacket)
{
    CmsgArenaTeamRoster srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (auto arenaTeam = sObjectMgr.getArenaTeamById(srlPacket.teamId))
    {
        const auto memberList = arenaTeam->getRoosterMembers();
        SendPacket(SmsgArenaTeamRooster(arenaTeam->m_id, static_cast<uint32_t>(memberList.size()), arenaTeam->getPlayersPerTeam(), memberList).serialise().get());
    }
}

void WorldSession::handleInspectArenaStatsOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING != Classic
    MsgInspectArenaTeams srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_INSPECT_ARENA_STATS: {} (guidLow)", srlPacket.guid.getGuidLow());

    const auto player = _player->getWorldMap()->getPlayer(srlPacket.guid.getGuidLow());
    if (player == nullptr)
        return;

    std::vector<ArenaTeamsList> arenaTeamList;
    ArenaTeamsList tempList{};

    for (uint8_t offset = 0; offset < NUM_ARENA_TEAM_TYPES; ++offset)
    {
        const uint32_t teamId = player->getArenaTeamId(offset);
        if (teamId > 0)
        {
            const auto arenaTeam = sObjectMgr.getArenaTeamById(teamId);
            if (arenaTeam != nullptr)
            {
                tempList.playerGuid = player->getGuid();
                tempList.teamType = arenaTeam->m_type;
                tempList.teamId = arenaTeam->m_id;
                tempList.teamRating = arenaTeam->m_stats.rating;
                tempList.playedWeek = arenaTeam->m_stats.played_week;
                tempList.wonWeek = arenaTeam->m_stats.won_week;
                tempList.playedSeason = arenaTeam->m_stats.played_season;

                arenaTeamList.push_back(tempList);
            }
        }
    }

    if (!arenaTeamList.empty())
        SendPacket(MsgInspectArenaTeams(0, arenaTeamList).serialise().get());
#endif
}
