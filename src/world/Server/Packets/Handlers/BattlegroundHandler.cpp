/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
    if (_player->m_bg != nullptr)
        _player->m_bg->SendPVPData(_player);
}

void WorldSession::handleInspectHonorStatsOpcode(WorldPacket& recvPacket)
{
    MsgInspectHonorStats srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received MSG_INSPECT_HONOR_STATS: %u (guidLow)", srlPacket.guid.getGuidLow());

    const auto player = _player->GetMapMgr()->GetPlayer(srlPacket.guid.getGuidLow());
    if (player == nullptr)
        return;

    const uint8_t honorCurrency = static_cast<uint8_t>(player->GetHonorCurrency());
    const uint32_t lifetimeKills = player->getUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS);

    uint32_t kills = 0;
    uint32_t todayContrib = 0;
    uint32_t yesterdayContrib = 0;

#if VERSION_STRING != Classic
    kills = player->getUInt32Value(PLAYER_FIELD_KILLS);
#if VERSION_STRING < Cata
    todayContrib = player->getUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION);
    yesterdayContrib = player->getUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION);
#endif
#endif

    SendPacket(MsgInspectHonorStats(player->getGuid(), honorCurrency, kills, todayContrib, yesterdayContrib, lifetimeKills).serialise().get());
}

void WorldSession::handleArenaJoinOpcode(WorldPacket& recvPacket)
{
    if (_player->GetGroup() && _player->GetGroup()->m_isqueued)
    {
        SystemMessage("You are already in a queud group for battlegrounds.");
        return;
    }

    if (_player->m_bgIsQueued)
        BattlegroundManager.RemovePlayerFromQueues(_player);

    CmsgBattlemasterJoinArena srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    uint32_t battlegroundType;

    switch (srlPacket.category)
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
            LogDebugFlag(LF_OPCODE, "Received CMSG_BATTLEMASTER_JOIN_ARENA: with invalid category (%u)", srlPacket.category);
            battlegroundType = 0;
            break;
    }

    if (battlegroundType != 0)
        BattlegroundManager.HandleArenaJoin(this, battlegroundType, srlPacket.asGroup, srlPacket.ratedMatch);
}

void WorldSession::handleBattlefieldPortOpcode(WorldPacket& recvPacket)
{
    CmsgBattlefieldPort srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.action != 0)
    {
        if (_player->m_pendingBattleground)
            _player->m_pendingBattleground->PortPlayer(_player);
    }
    else
    {
        BattlegroundManager.RemovePlayerFromQueues(_player);
    }
}

void WorldSession::handleLeaveBattlefieldOpcode(WorldPacket& /*recvPacket*/)
{
    if (_player->m_bg && _player->IsInWorld())
        _player->m_bg->RemovePlayer(_player, false);
}

void WorldSession::handleBattlefieldListOpcode(WorldPacket& recvPacket)
{
    CmsgBattlefieldList srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_BATTLEFIELD_LIST: %u (bgType), %u (fromType)", srlPacket.bgType, srlPacket.fromType);

    BattlegroundManager.HandleBattlegroundListPacket(this, srlPacket.bgType, srlPacket.fromType);
}

void WorldSession::handleBattleMasterHelloOpcode(WorldPacket& recvPacket)
{
    CmsgBattlemasterHello srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_BATTLEMASTER_HELLO: %u (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr || !creature->isBattleMaster())
        return;

    sendBattlegroundList(creature, 0);
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

void WorldSession::handleAreaSpiritHealerQueueOpcode(WorldPacket& recvPacket)
{
    const auto cBattleground = _player->m_bg;
    if (cBattleground == nullptr)
        return;

    CmsgAreaSpiritHealerQueue srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_AREA_SPIRIT_HEALER_QUEUE: %u (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto spiritHealer = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLowPart());
    if (spiritHealer == nullptr)
        return;

    cBattleground->QueuePlayerForResurrect(_player, spiritHealer);
    _player->castSpell(_player, 2584, true);
}

void WorldSession::handleAreaSpiritHealerQueryOpcode(WorldPacket& recvPacket)
{
    const auto cBattleground = _player->m_bg;
    if (cBattleground == nullptr)
        return;

    CmsgAreaSpiritHealerQuery srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_AREA_SPIRIT_HEALER_QUEUE: %u (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto spiritHealer = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLowPart());
    if (spiritHealer == nullptr)
        return;

    uint32_t restTime = cBattleground->GetLastResurrect() + 30;
    if (static_cast<uint32_t>(UNIXTIME) > restTime)
        restTime = 1000;
    else
        restTime = (restTime - static_cast<uint32_t>(UNIXTIME)) * 1000;

    SendPacket(SmsgAreaSpiritHealerTime(srlPacket.guid.GetOldGuid(), restTime).serialise().get());
}

void WorldSession::handleBattlefieldStatusOpcode(WorldPacket& /*recvPacket*/)
{
    const auto pendingBattleground = _player->m_pendingBattleground;
    const auto cBattleground = _player->m_bg;

    if (cBattleground)
        BattlegroundManager.SendBattlefieldStatus(_player, BGSTATUS_TIME, cBattleground->GetType(), cBattleground->GetId(), static_cast<uint32_t>(UNIXTIME) - cBattleground->GetStartTime(), _player->GetMapId(), cBattleground->Rated());
    else if (pendingBattleground)
        BattlegroundManager.SendBattlefieldStatus(_player, BGSTATUS_READY, pendingBattleground->GetType(), pendingBattleground->GetId(), 120000, 0, pendingBattleground->Rated());
    else
        BattlegroundManager.SendBattlefieldStatus(_player, BGSTATUS_NOFLAGS, 0, 0, 0, 0, 0);
}

void WorldSession::handleBattleMasterJoinOpcode(WorldPacket& recvPacket)
{
    if (_player->HasAura(BG_DESERTER))
    {
        WorldPacket data(SMSG_GROUP_JOINED_BATTLEGROUND, 4);
        data << uint32_t(0xFFFFFFFE);
        _player->GetSession()->SendPacket(&data);
        return;
    }

    if (_player->GetGroup() && _player->GetGroup()->m_isqueued)
    {
        SystemMessage("You are already in a group and queued for a battleground or inside a battleground. Leave this first.");
        return;
    }

    if (_player->m_bgIsQueued)
        BattlegroundManager.RemovePlayerFromQueues(_player);

    if (_player->IsInWorld())
        BattlegroundManager.HandleBattlegroundJoin(this, recvPacket);
}

void WorldSession::sendBattlegroundList(Creature* creature, uint32_t mapId)
{
    if (creature == nullptr)
        return;

    uint32_t battlegroundType = BATTLEGROUND_WARSONG_GULCH;
    if (mapId == 0)
    {
        if (creature->GetCreatureProperties()->SubName != "Arena")
        {
            battlegroundType = BATTLEGROUND_ARENA_2V2;
        }
        else
        {
            if (const auto battleMaster = sMySQLStore.getBattleMaster(creature->GetCreatureProperties()->Id))
                battlegroundType = battleMaster->battlegroundId;
        }
    }
    else
    {
        battlegroundType = mapId;
    }

    BattlegroundManager.HandleBattlegroundListPacket(this, battlegroundType);
}

#if VERSION_STRING >= Cata
void WorldSession::handleRequestRatedBgInfoOpcode(WorldPacket & recvPacket)
{
    uint8_t unk_type;
    recvPacket >> unk_type;

    LogDebugFlag(LF_OPCODE, "Received CMSG_REQUEST_RATED_BG_INFO received with unk_type = %u", unk_type);

    WorldPacket data(SMSG_RATED_BG_INFO, 72);
    for (int i = 0; i < 18; ++i)
    {
        data << uint32_t(0);    // unknown
    }

    SendPacket(&data);
}

void WorldSession::handleRequestRatedBgStatsOpcode(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "Received CMSG_REQUEST_RATED_BG_STATS received");

    WorldPacket data(SMSG_RATED_BG_STATS, 29);
    data << uint32_t(0);    // unknown
    data << uint8_t(3);     // unknown - always 3?... type?
    data << uint32_t(0);    // unknown
    data << uint32_t(0);    // unknown
    data << uint32_t(0);    // unknown
    data << uint32_t(0);    // unknown
    data << uint32_t(0);    // unknown
    data << uint32_t(0);    // unknown

    SendPacket(&data);
}

void WorldSession::handleRequestPvPRewardsOpcode(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "Received CMSG_REQUEST_RATED_BG_STATS received");

    WorldPacket packet(SMSG_REQUEST_PVP_REWARDS_RESPONSE, 24);
    packet << uint32_t(0);    // unknown currency week cap conquest points
    packet << uint32_t(0);    // unknown currency on week conquest points
    packet << uint32_t(0);    // unknown currency week cap conquest arena
    packet << uint32_t(0);    // unknown currency on week conquest random baattleground
    packet << uint32_t(0);    // unknown currency on week conquest arena
    packet << uint32_t(0);    // unknown currency week cap conquest points

    SendPacket(&packet);
}

void WorldSession::handleRequestPvpOptionsOpcode(WorldPacket& /*recvPacket*/)
{
    LogDebugFlag(LF_OPCODE, "Received CMSG_REQUEST_RATED_BG_STATS received");

    WorldPacket data(SMSG_PVP_OPTIONS_ENABLED, 1);
    data.writeBit(1);       // unknown 
    data.writeBit(1);       // unknown wargames enabled
    data.writeBit(1);       // unknown 
    data.writeBit(1);       // unknown rated battlegrounds enabled
    data.writeBit(1);       // unknown rated arenas enabled

    data.flushBits();

    SendPacket(&data);
}
#endif
