/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Logging/Logger.hpp"
#include "Management/Group.h"
#include "Server/Packets/MsgInspectHonorStats.h"
#include "Server/Packets/CmsgBattlemasterJoinArena.h"
#include "Server/Packets/CmsgBattlefieldPort.h"
#include "Server/Packets/CmsgBattlefieldList.h"
#include "Server/Packets/CmsgBattlemasterHello.h"
#include "Server/Packets/MsgBattlegroundPlayerPosition.h"
#include "Server/Packets/CmsgAreaSpiritHealerQueue.h"
#include "Server/Packets/CmsgAreaSpiritHealerQuery.h"
#include "Server/Packets/SmsgAreaSpiritHealerTime.h"
#include "Server/Packets/SmsgGroupJoinedBattleground.h"
#include "Server/WorldSession.h"
#include "Objects/Units/Players/Player.hpp"
#include "Management/Battleground/Battleground.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Management/ObjectMgr.hpp"
#include "Management/Battleground/BattlegroundMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Storage/MySQLDataStore.hpp"

#if VERSION_STRING >= Cata
#include "Server/Packets/CmsgRequestRatedBgInfo.h"
#include "Server/Packets/SmsgPvpOptionsEnabled.h"
#include "Server/Packets/SmsgRatedBgInfo.h"
#include "Server/Packets/SmsgRatedBgStats.h"
#include "Server/Packets/SmsgRequestPvpRewardsResponse.h"
#endif

using namespace AscEmu::Packets;

void WorldSession::handlePVPLogDataOpcode(WorldPacket& /*recvPacket*/)
{
    if (_player->m_bg != nullptr)
        _player->m_bg->sendPVPData(_player);
}

void WorldSession::handleInspectHonorStatsOpcode(WorldPacket& recvPacket)
{
    MsgInspectHonorStats srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_INSPECT_HONOR_STATS: {} (guidLow)", srlPacket.guid.getGuidLow());

    const auto player = _player->getWorldMap()->getPlayer(srlPacket.guid.getGuidLow());
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

    MsgInspectHonorStats sendPacket(player->getGuid(), honorCurrency, kills, todayContrib, yesterdayContrib, lifetimeKills);
    sendManagedPacket(sendPacket);
}

void WorldSession::handleArenaJoinOpcode(WorldPacket& recvPacket)
{
    if (_player->getGroup() && _player->getGroup()->m_isqueued)
    {
        systemMessage("You are already in a queued battleground group.");
        return;
    }

    if (_player->m_isQueuedForBg)
        sBattlegroundManager.removePlayerFromQueues(_player);

    CmsgBattlemasterJoinArena srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    uint32_t battlegroundType;

    switch (srlPacket.category)
    {
        case 0:
            battlegroundType = BattlegroundDef::TYPE_ARENA_2V2;
            break;
        case 1:
            battlegroundType = BattlegroundDef::TYPE_ARENA_3V3;
            break;
        case 2:
            battlegroundType = BattlegroundDef::TYPE_ARENA_5V5;
            break;
        default:
            sLogger.debugOpcode("Received CMSG_BATTLEMASTER_JOIN_ARENA: with invalid category ({})", srlPacket.category);
            battlegroundType = 0;
            break;
    }

    if (battlegroundType != 0)
        sBattlegroundManager.handleArenaJoin(this, battlegroundType, srlPacket.asGroup, srlPacket.ratedMatch);
}

void WorldSession::handleBattlefieldPortOpcode(WorldPacket& recvPacket)
{
    CmsgBattlefieldPort srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    if (srlPacket.action != 0)
    {
        if (_player->m_pendingBattleground)
            _player->m_pendingBattleground->portPlayer(_player);
    }
    else
    {
        sBattlegroundManager.removePlayerFromQueues(_player);
    }
}

void WorldSession::handleLeaveBattlefieldOpcode(WorldPacket& /*recvPacket*/)
{
    if (_player->m_bg && _player->IsInWorld())
        _player->m_bg->removePlayer(_player, false);
}

void WorldSession::handleBattlefieldListOpcode(WorldPacket& recvPacket)
{
    CmsgBattlefieldList srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_BATTLEFIELD_LIST: {} (bgType), {} (fromType)", srlPacket.bgType, srlPacket.fromType);

#if VERSION_STRING <= WotLK
    sBattlegroundManager.handleBattlegroundListPacket(this, srlPacket.bgType, srlPacket.fromType);
 #else
    WoWGuid guid;
    guid.init(uint64_t(0));
    sBattlegroundManager.handleBattlegroundListPacket(guid, this, srlPacket.bgType);
#endif
}

void WorldSession::handleBattleMasterHelloOpcode(WorldPacket& recvPacket)
{
    CmsgBattlemasterHello srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_BATTLEMASTER_HELLO: {} (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto creature = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (creature == nullptr || !creature->isBattleMaster())
        return;

    sendBattlegroundList(creature, 0);
}

void WorldSession::handleBattlegroundPlayerPositionsOpcode(WorldPacket& /*recvPacket*/)
{
    const auto Battleground = _player->m_bg;
    if (Battleground == nullptr)
        return;

    uint32_t flagHolders = 0;

    const auto alliancePlayer = sObjectMgr.getPlayer(static_cast<uint32_t>(Battleground->GetFlagHolderGUID(TEAM_ALLIANCE)));
    if (alliancePlayer)
        ++flagHolders;

    const auto hordePlayer = sObjectMgr.getPlayer(static_cast<uint32_t>(Battleground->GetFlagHolderGUID(TEAM_HORDE)));
    if (hordePlayer)
        ++flagHolders;

    MsgBattlegroundPlayerPosition sendPacket(0, flagHolders, alliancePlayer, hordePlayer);
    sendManagedPacket(sendPacket);
}

void WorldSession::handleAreaSpiritHealerQueueOpcode(WorldPacket& recvPacket)
{
    const auto Battleground = _player->m_bg;
    if (Battleground == nullptr)
        return;

    CmsgAreaSpiritHealerQueue srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_AREA_SPIRIT_HEALER_QUEUE: {} (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto spiritHealer = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (spiritHealer == nullptr)
        return;

    Battleground->queuePlayerForResurrect(_player, spiritHealer);
    _player->castSpell(_player, 2584, true);
}

void WorldSession::handleAreaSpiritHealerQueryOpcode(WorldPacket& recvPacket)
{
    const auto Battleground = _player->m_bg;
    if (Battleground == nullptr)
        return;

    CmsgAreaSpiritHealerQuery srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_AREA_SPIRIT_HEALER_QUEUE: {} (guidLowPart)", srlPacket.guid.getGuidLowPart());

    const auto spiritHealer = _player->getWorldMap()->getCreature(srlPacket.guid.getGuidLowPart());
    if (spiritHealer == nullptr)
        return;

    uint32_t restTime = Battleground->getLastResurrect() + 30;
    if (static_cast<uint32_t>(UNIXTIME) > restTime)
        restTime = 1000;
    else
        restTime = (restTime - static_cast<uint32_t>(UNIXTIME)) * 1000;

    SmsgAreaSpiritHealerTime managedPacket(srlPacket.guid.getRawGuid(), restTime);
    sendManagedPacket(managedPacket);
}

void WorldSession::handleBattlefieldStatusOpcode(WorldPacket& /*recvPacket*/)
{
    const auto pendingBattleground = _player->m_pendingBattleground;
    const auto Battleground = _player->m_bg;

    if (Battleground)
        sBattlegroundManager.sendBattlefieldStatus(_player, BattlegroundDef::STATUS_TIME, Battleground->getType(), Battleground->getId(), static_cast<uint32_t>(UNIXTIME) - Battleground->getStartTime(), _player->GetMapId(), Battleground->Rated());
    else if (pendingBattleground)
        sBattlegroundManager.sendBattlefieldStatus(_player, BattlegroundDef::STATUS_READY, pendingBattleground->getType(), pendingBattleground->getId(), 120000, 0, pendingBattleground->Rated());
    else
        sBattlegroundManager.sendBattlefieldStatus(_player, BattlegroundDef::STATUS_NOFLAGS, 0, 0, 0, 0, 0);
}

void WorldSession::handleBattleMasterJoinOpcode(WorldPacket& recvPacket)
{
    if (_player->hasAurasWithId(BattlegroundDef::DESERTER))
    {
        SmsgGroupJoinedBattleground managedPacket(static_cast<int32_t>(0xFFFFFFFE));
        if (const auto session = _player->getSession())
            session->sendManagedPacket(managedPacket);
        return;
    }

    if (_player->getGroup() && _player->getGroup()->m_isqueued)
    {
        systemMessage("You are already in a group queued for a battleground or are currently inside a battleground. Leave it first.");
        return;
    }

    if (_player->m_isQueuedForBg)
        sBattlegroundManager.removePlayerFromQueues(_player);

    if (_player->IsInWorld())
        sBattlegroundManager.handleBattlegroundJoin(this, recvPacket);
}

void WorldSession::sendBattlegroundList(Creature* creature, uint32_t mapId)
{
    if (creature == nullptr)
        return;

    WoWGuid guid;

    uint32_t battlegroundType = BattlegroundDef::TYPE_WARSONG_GULCH;
    if (mapId == 0)
    {
        if (creature->GetCreatureProperties()->SubName != "Arena")
        {
            battlegroundType = BattlegroundDef::TYPE_ARENA_2V2;
        }
        else
        {
            if (const auto battleMaster = sMySQLStore.getBattleMaster(creature->GetCreatureProperties()->Id))
            {
                battlegroundType = battleMaster->battlegroundId;
                guid.init(creature->getGuid());
            }
        }
    }
    else
    {
        battlegroundType = mapId;
    }

#if VERSION_STRING <= WotLK
    sBattlegroundManager.handleBattlegroundListPacket(this, battlegroundType);
#else
    sBattlegroundManager.handleBattlegroundListPacket(guid, this, battlegroundType);
#endif
}

void WorldSession::handleRequestRatedBgInfoOpcode([[maybe_unused]] WorldPacket& recvPacket)
{
#if VERSION_STRING >= Cata
    CmsgRequestRatedBgInfo srlPacket;
    if (!parsePacket(recvPacket, srlPacket))
        return;

    sLogger.debugOpcode("Received CMSG_REQUEST_RATED_BG_INFO received with unk_type = {}", srlPacket.type);

    SmsgRatedBgInfo managedPacket(0);
    sendManagedPacket(managedPacket);
#endif
}

void WorldSession::handleRequestRatedBgStatsOpcode(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING >= Cata
    sLogger.debugOpcode("Received CMSG_REQUEST_RATED_BG_STATS received");

    SmsgRatedBgStats managedPacket(3);
    sendManagedPacket(managedPacket);
#endif
}

void WorldSession::handleRequestPvPRewardsOpcode(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING >= Cata
    sLogger.debugOpcode("Received CMSG_REQUEST_RATED_BG_STATS received");

    SmsgRequestPvpRewardsResponse managedPacket(0, 0, 0, 0, 0, 0);
    sendManagedPacket(managedPacket);
#endif
}

void WorldSession::handleRequestPvpOptionsOpcode(WorldPacket& /*recvPacket*/)
{
#if VERSION_STRING >= Cata
    sLogger.debugOpcode("Received CMSG_REQUEST_RATED_BG_STATS received");

    SmsgPvpOptionsEnabled managedPacket(true, true, true);
    sendManagedPacket(managedPacket);
#endif
}

