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
#include "Server/Packets/CmsgAreaSpiritHealerQueue.h"
#include "Server/Packets/CmsgAreaSpiritHealerQuery.h"
#include "Server/Packets/SmsgAreaSpiritHealerTime.h"
#include "Server/WorldSession.h"
#include "Units/Players/Player.h"
#include "Management/Battleground/Battleground.h"
#include "Map/MapMgr.h"
#include "Objects/ObjectMgr.h"
#include "Storage/MySQLDataStore.hpp"

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

    LOG_DEBUG("Received CMSG_BATTLEMASTER_HELLO: %u (guidLowPart)", recv_packet.guid.getGuidLowPart());

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLowPart());
    if (creature == nullptr || !creature->isBattleMaster())
        return;

    SendBattlegroundList(creature, 0);
}

void WorldSession::handleBattlegroundPlayerPositionsOpcode(WorldPacket& /*recvPacket*/)
{
    const auto cBattleground = GetPlayer()->m_bg;
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

void WorldSession::handleAreaSpiritHealerQueueOpcode(WorldPacket& recvPacket)
{
    const auto cBattleground = GetPlayer()->m_bg;
    if (cBattleground == nullptr)
        return;

    CmsgAreaSpiritHealerQueue recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_AREA_SPIRIT_HEALER_QUEUE: %u (guidLowPart)", recv_packet.guid.getGuidLowPart());

    const auto spiritHealer = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLowPart());
    if (spiritHealer == nullptr)
        return;

    cBattleground->QueuePlayerForResurrect(GetPlayer(), spiritHealer);
    GetPlayer()->CastSpell(GetPlayer(), 2584, true);
}

void WorldSession::handleAreaSpiritHealerQueryOpcode(WorldPacket& recvPacket)
{
    const auto cBattleground = GetPlayer()->m_bg;
    if (cBattleground == nullptr)
        return;

    CmsgAreaSpiritHealerQuery recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_AREA_SPIRIT_HEALER_QUEUE: %u (guidLowPart)", recv_packet.guid.getGuidLowPart());

    const auto spiritHealer = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLowPart());
    if (spiritHealer == nullptr)
        return;

    uint32_t restTime = cBattleground->GetLastResurrect() + 30;
    if (static_cast<uint32_t>(UNIXTIME) > restTime)
        restTime = 1000;
    else
        restTime = (restTime - static_cast<uint32_t>(UNIXTIME)) * 1000;

    SendPacket(SmsgAreaSpiritHealerTime(recv_packet.guid.GetOldGuid(), restTime).serialise().get());
}

void WorldSession::handleBattlefieldStatusOpcode(WorldPacket& /*recvPacket*/)
{
    const auto pendingBattleground = GetPlayer()->m_pendingBattleground;
    const auto cBattleground = GetPlayer()->m_bg;

    if (cBattleground)
        BattlegroundManager.SendBattlefieldStatus(GetPlayer(), BGSTATUS_TIME, cBattleground->GetType(), cBattleground->GetId(), static_cast<uint32_t>(UNIXTIME) - cBattleground->GetStartTime(), GetPlayer()->GetMapId(), cBattleground->Rated());
    else if (pendingBattleground)
        BattlegroundManager.SendBattlefieldStatus(GetPlayer(), BGSTATUS_READY, pendingBattleground->GetType(), pendingBattleground->GetId(), 120000, 0, pendingBattleground->Rated());
    else
        BattlegroundManager.SendBattlefieldStatus(GetPlayer(), BGSTATUS_NOFLAGS, 0, 0, 0, 0, 0);
}

void WorldSession::handleBattleMasterJoinOpcode(WorldPacket& recvPacket)
{
    if (GetPlayer()->HasAura(BG_DESERTER))
    {
        WorldPacket data(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
        data << uint32_t(0xFFFFFFFE);
        GetPlayer()->GetSession()->SendPacket(&data);
        return;
    }

    if (GetPlayer()->GetGroup() && GetPlayer()->GetGroup()->m_isqueued)
    {
        SystemMessage("You are already in a group and queued for a battleground or inside a battleground. Leave this first.");
        return;
    }

    if (GetPlayer()->m_bgIsQueued)
        BattlegroundManager.RemovePlayerFromQueues(GetPlayer());

    if (GetPlayer()->IsInWorld())
        BattlegroundManager.HandleBattlegroundJoin(this, recvPacket);
}

void WorldSession::SendBattlegroundList(Creature* pCreature, uint32_t mapId)
{
    if (pCreature == nullptr)
        return;

    uint32_t battlegroundType = BATTLEGROUND_WARSONG_GULCH;
    if (mapId == 0)
    {
        if (pCreature->GetCreatureProperties()->SubName != "Arena")
        {
            battlegroundType = BATTLEGROUND_ARENA_2V2;
        }
        else
        {
            const auto battlemaster = sMySQLStore.getBattleMaster(pCreature->GetCreatureProperties()->Id);
            if (battlemaster)
                battlegroundType = battlemaster->battlegroundId;
        }
    }
    else
    {
        battlegroundType = mapId;
    }

    BattlegroundManager.HandleBattlegroundListPacket(this, battlegroundType);
}
