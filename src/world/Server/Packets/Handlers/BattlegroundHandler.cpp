/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/MsgInspectArenaTeams.h"
#include "Server/Packets/MsgInspectHonorStats.h"
#include "Server/Packets/CmsgBattlemasterJoinArena.h"

using namespace AscEmu::Packets;

void WorldSession::handlePVPLogDataOpcode(WorldPacket& /*recvPacket*/)
{
    if (GetPlayer()->m_bg != nullptr)
        GetPlayer()->m_bg->SendPVPData(GetPlayer());
}

void WorldSession::handleInspectArenaStatsOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING != Classic
    MsgInspectArenaTeams recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received MSG_INSPECT_ARENA_STATS: %u (guidLow)", recv_packet.guid.getGuidLow());

    const auto player = GetPlayer()->GetMapMgr()->GetPlayer(recv_packet.guid.getGuidLow());
    if (player == nullptr)
        return;

    std::vector<ArenaTeamsList> arenaTeamList;
    ArenaTeamsList tempList;

    for (uint8_t offset = 0; offset < 3; ++offset)
    {
        const uint32_t teamId = player->getUInt32Value(PLAYER_FIELD_ARENA_TEAM_INFO_1_1 + (offset * 7));
        if (teamId > 0)
        {
            const auto team = objmgr.GetArenaTeamById(teamId);
            if (team != nullptr)
            {
                tempList.playerGuid = player->getGuid();
                tempList.teamType = team->m_type;
                tempList.teamId = team->m_id;
                tempList.teamRating = team->m_stat_rating;
                tempList.playedWeek = team->m_stat_gamesplayedweek;
                tempList.wonWeek = team->m_stat_gameswonweek;
                tempList.playedSeason = team->m_stat_gamesplayedseason;

                arenaTeamList.push_back(tempList);
            }
        }
    }

    if (!arenaTeamList.empty())
        SendPacket(MsgInspectArenaTeams(0, arenaTeamList).serialise().get());
#endif
}

void WorldSession::handleInspectHonorStatsOpcode(WorldPacket& recvPacket)
{
    MsgInspectHonorStats recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received MSG_INSPECT_HONOR_STATS: %u (guidLow)", recv_packet.guid.getGuidLow());

    const auto player = GetPlayer()->GetMapMgr()->GetPlayer(recv_packet.guid.getGuidLow());
    if (player == nullptr)
        return;

    const uint8_t honorCurrency = static_cast<uint8_t>(player->GetHonorCurrency());
    const uint32_t lifetimeKills = player->getUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS);

    uint32_t kills = 0;
    uint32_t todayContrib = 0;
    uint32_t yesterdayContrib = 0;

#if VERSION_STRING != Classic
    kills = player->getUInt32Value(PLAYER_FIELD_KILLS);
#if VERSION_STRING != Cata
    todayContrib = player->getUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION);
    yesterdayContrib = player->getUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION);
#endif
#endif

    SendPacket(MsgInspectHonorStats(player->getGuid(), honorCurrency, kills, todayContrib, yesterdayContrib, lifetimeKills).serialise().get());
}

void WorldSession::handleArenaJoinOpcode(WorldPacket& recvPacket)
{
    if (GetPlayer()->GetGroup() && GetPlayer()->GetGroup()->m_isqueued)
    {
        SystemMessage("You are already in a queud group for battlegrounds.");
        return;
    }

    if (GetPlayer()->m_bgIsQueued)
        BattlegroundManager.RemovePlayerFromQueues(GetPlayer());

    CmsgBattlemasterJoinArena recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    uint32_t battlegroundType;

    switch (recv_packet.category)
    {
        case 0:
            battlegroundType = BATTLEGROUND_ARENA_2V2;
            break;
        case 1:
            battlegroundType = BATTLEGROUND_ARENA_3V3;
            break;
        case 2:
            battlegroundType = BATTLEGROUND_ARENA_5V5;
            break;
        default:
            LOG_DEBUG("Received CMSG_BATTLEMASTER_JOIN_ARENA: with invalid category (%u)", recv_packet.category);
            battlegroundType = 0;
            break;
    }

    if (battlegroundType != 0)
        BattlegroundManager.HandleArenaJoin(this, battlegroundType, recv_packet.asGroup, recv_packet.ratedMatch);
}
