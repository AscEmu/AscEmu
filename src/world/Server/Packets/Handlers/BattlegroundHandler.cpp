/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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
#include "Server/Packets/CmsgRequestRatedBgInfo.h"
#include "Server/Packets/SmsgRatedBgInfo.h"
#include "Server/Packets/SmsgRatedBgStats.h"
#include "Server/Packets/SmsgPvpOptionsEnabled.h"

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

    sLogger.debug("Received MSG_INSPECT_HONOR_STATS: %u (guidLow)", srlPacket.guid.getGuidLow());

    const auto player = _player->GetMapMgr()->GetPlayer(srlPacket.guid.getGuidLow());
    if (player == nullptr)
        return;

    uint8_t honorCurrency = 0;

#if VERSION_STRING > Classic
#if VERSION_STRING < Cata
    honorCurrency = static_cast<uint8_t>(player->getHonorCurrency());
#endif
#endif

    const uint32_t lifetimeKills = player->getLifetimeHonorableKills();

    uint32_t kills = 0;
    uint32_t todayContrib = 0;
    uint32_t yesterdayContrib = 0;

#if VERSION_STRING != Classic
    kills = player->getFieldKills();
#if VERSION_STRING < Cata
    todayContrib = player->getContributionToday();
    yesterdayContrib = player->getContributionYesterday();
#endif
#endif

    SendPacket(MsgInspectHonorStats(player->getGuid(), honorCurrency, kills, todayContrib, yesterdayContrib, lifetimeKills).serialise().get());
}

void WorldSession::handleArenaJoinOpcode(WorldPacket& recvPacket)
{
    if (_player->getGroup() && _player->getGroup()->m_isqueued)
    {
        SystemMessage("You are already in a queud group for battlegrounds.");
        return;
    }

    if (_player->m_bgIsQueued)
        sBattlegroundManager.RemovePlayerFromQueues(_player);

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
            sLogger.debug("Received CMSG_BATTLEMASTER_JOIN_ARENA: with invalid category (%u)", srlPacket.category);
            battlegroundType = 0;
            break;
    }

    if (battlegroundType != 0)
        sBattlegroundManager.HandleArenaJoin(this, battlegroundType, srlPacket.asGroup, srlPacket.ratedMatch);
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
        sBattlegroundManager.RemovePlayerFromQueues(_player);
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

    sLogger.debug("Received CMSG_BATTLEFIELD_LIST: %u (bgType), %u (fromType)", srlPacket.bgType, srlPacket.fromType);

    sBattlegroundManager.HandleBattlegroundListPacket(this, srlPacket.bgType, srlPacket.fromType);
}

void WorldSession::handleBattleMasterHelloOpcode(WorldPacket& recvPacket)
{
    CmsgBattlemasterHello srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debug("Received CMSG_BATTLEMASTER_HELLO: %u (guidLowPart)", srlPacket.guid.getGuidLowPart());

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

    const auto alliancePlayer = sObjectMgr.GetPlayer(static_cast<uint32_t>(cBattleground->GetFlagHolderGUID(TEAM_ALLIANCE)));
    if (alliancePlayer)
        ++flagHolders;

    const auto hordePlayer = sObjectMgr.GetPlayer(static_cast<uint32_t>(cBattleground->GetFlagHolderGUID(TEAM_HORDE)));
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

    sLogger.debug("Received CMSG_AREA_SPIRIT_HEALER_QUEUE: %u (guidLowPart)", srlPacket.guid.getGuidLowPart());

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

    sLogger.debug("Received CMSG_AREA_SPIRIT_HEALER_QUEUE: %u (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto spiritHealer = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLowPart());
    if (spiritHealer == nullptr)
        return;

    uint32_t restTime = cBattleground->GetLastResurrect() + 30;
    if (static_cast<uint32_t>(UNIXTIME) > restTime)
        restTime = 1000;
    else
        restTime = (restTime - static_cast<uint32_t>(UNIXTIME)) * 1000;

    SendPacket(SmsgAreaSpiritHealerTime(srlPacket.guid.getRawGuid(), restTime).serialise().get());
}

void WorldSession::handleBattlefieldStatusOpcode(WorldPacket& /*recvPacket*/)
{
    const auto pendingBattleground = _player->m_pendingBattleground;
    const auto cBattleground = _player->m_bg;

    if (cBattleground)
        sBattlegroundManager.SendBattlefieldStatus(_player, BGSTATUS_TIME, cBattleground->GetType(), cBattleground->GetId(), static_cast<uint32_t>(UNIXTIME) - cBattleground->GetStartTime(), _player->GetMapId(), cBattleground->Rated());
    else if (pendingBattleground)
        sBattlegroundManager.SendBattlefieldStatus(_player, BGSTATUS_READY, pendingBattleground->GetType(), pendingBattleground->GetId(), 120000, 0, pendingBattleground->Rated());
    else
        sBattlegroundManager.SendBattlefieldStatus(_player, BGSTATUS_NOFLAGS, 0, 0, 0, 0, 0);
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

    if (_player->getGroup() && _player->getGroup()->m_isqueued)
    {
        SystemMessage("You are already in a group and queued for a battleground or inside a battleground. Leave this first.");
        return;
    }

    if (_player->m_bgIsQueued)
        sBattlegroundManager.RemovePlayerFromQueues(_player);

    if (_player->IsInWorld())
        sBattlegroundManager.HandleBattlegroundJoin(this, recvPacket);
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

    sBattlegroundManager.HandleBattlegroundListPacket(this, battlegroundType);
}

#if VERSION_STRING >= Cata
void WorldSession::handleRequestRatedBgInfoOpcode(WorldPacket & recvPacket)
{
    CmsgRequestRatedBgInfo srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debug("Received CMSG_REQUEST_RATED_BG_INFO received with unk_type = %u", srlPacket.type);

    SendPacket(SmsgRatedBgInfo(0).serialise().get());
}

void WorldSession::handleRequestRatedBgStatsOpcode(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("Received CMSG_REQUEST_RATED_BG_STATS received");

    SendPacket(SmsgRatedBgStats(3).serialise().get());
}

void WorldSession::handleRequestPvPRewardsOpcode(WorldPacket& /*recvPacket*/)
{
    sLogger.debug("Received CMSG_REQUEST_RATED_BG_STATS received");

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
    sLogger.debug("Received CMSG_REQUEST_RATED_BG_STATS received");

    SendPacket(SmsgPvpOptionsEnabled(true, true, true).serialise().get());
}
#endif
