/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/MsgInspectArenaTeams.h"
#include "Server/Packets/MsgInspectHonorStats.h"
#include "Server/Packets/CmsgBattlemasterJoinArena.h"
#include "Server/Packets/CmsgBattlefieldPort.h"
#include "Server/Packets/CmsgBattlefieldList.h"
#include "Server/Packets/CmsgBattlemasterHello.h"
#include "Server/Packets/MsgBattlegroundPlayerPosition.h"

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

void WorldSession::handleBattlefieldPortOpcode(WorldPacket& recvPacket)
{
    CmsgBattlefieldPort recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (recv_packet.action != 0)
    {
        if (GetPlayer()->m_pendingBattleground)
            GetPlayer()->m_pendingBattleground->PortPlayer(GetPlayer());
    }
    else
    {
        BattlegroundManager.RemovePlayerFromQueues(GetPlayer());
    }
}

void WorldSession::handleLeaveBattlefieldOpcode(WorldPacket& /*recvPacket*/)
{
    if (GetPlayer()->m_bg && GetPlayer()->IsInWorld())
        GetPlayer()->m_bg->RemovePlayer(GetPlayer(), false);
}

void WorldSession::handleBattlefieldListOpcode(WorldPacket& recvPacket)
{
    CmsgBattlefieldList recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_BATTLEFIELD_LIST: %u (bgType), %u (fromType)", recv_packet.bgType, recv_packet.fromType);

    BattlegroundManager.HandleBattlegroundListPacket(this, recv_packet.bgType, recv_packet.fromType);
}

void WorldSession::handleReadyForAccountDataTimes(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "Received CMSG_READY_FOR_ACCOUNT_DATA_TIMES");

    SendAccountDataTimes(GLOBAL_CACHE_MASK);
}

void WorldSession::handleBattleMasterHelloOpcode(WorldPacket& recvPacket)
{
    CmsgBattlemasterHello recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_BATTLEMASTER_HELLO: %u (guidLow)", recv_packet.guid.getGuidLow());

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLow());
    if (creature == nullptr || !creature->isBattleMaster())
        return;

    SendBattlegroundList(creature, 0);
}

void WorldSession::handleBattlegroundPlayerPositionsOpcode(WorldPacket& /*recvPacket*/)
{
    const auto cBattleground = _player->m_bg;
    if (cBattleground == nullptr)
        return;

    uint32_t flagHolders = 0;

    const auto alliancePlayer = objmgr.GetPlayer(static_cast<uint32_t>(cBattleground->GetFlagHolderGUID(TEAM_ALLIANCE)));
    if (alliancePlayer)
        ++flagHolders;

    const auto hordePlayer = objmgr.GetPlayer(static_cast<uint32_t>(cBattleground->GetFlagHolderGUID(TEAM_HORDE)));
    if (hordePlayer)
        ++flagHolders;

    SendPacket(MsgBattlegroundPlayerPosition(0, flagHolders, alliancePlayer, hordePlayer).serialise().get());
}
