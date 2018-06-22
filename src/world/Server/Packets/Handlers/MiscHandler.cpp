/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Item.h"
#include "Management/WeatherMgr.h"
#include "Management/ItemInterface.h"
#include "Management/Battleground/Battleground.h"
#include "Server/WorldSocket.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Server/MainServerDefines.h"
#include "zlib.h"
#include "Map/MapMgr.h"
#include "Spell/SpellMgr.h"
#include "Server/Packets/SmsgLogoutResponse.h"
#include "Server/Packets/CmsgStandStateChange.h"
#include "Server/Packets/CmsgWho.h"
#include "Server/Packets/CmsgSetSelection.h"
#include "Server/Packets/CmsgTutorialFlag.h"
#include "Server/Packets/CmsgSetSheathed.h"
#if VERSION_STRING == Cata
#include "GameCata/Management/GuildMgr.h"
#endif

using namespace AscEmu::Packets;

void WorldSession::handleStandStateChangeOpcode(WorldPacket& recvPacket)
{
    CmsgStandStateChange recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    _player->setStandState(recv_packet.state);
}

void WorldSession::handleWhoOpcode(WorldPacket& recv_data)
{
    CmsgWho recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    bool cname = false;
    bool gname = false;

    if (recv_packet.player_name.length() > 0)
        cname = true;

    if (recv_packet.guild_name.length() > 0)
        gname = true;

    LOG_DEBUG("WORLD: Recvd CMSG_WHO Message with %u zones and %u names", recv_packet.zone_count, recv_packet.name_count);

    uint32_t team = _player->GetTeam();

    uint32_t sent_count = 0;
    uint32_t total_count = 0;

    WorldPacket data;
    data.SetOpcode(SMSG_WHO);
    data << uint64_t(0);

    objmgr._playerslock.AcquireReadLock();
    PlayerStorageMap::const_iterator iend = objmgr._players.end();
    PlayerStorageMap::const_iterator itr = objmgr._players.begin();
    while (itr != iend && sent_count < 49)
    {
        Player* plr = itr->second;
        ++itr;

        if (!plr->GetSession() || !plr->IsInWorld())
            continue;

        if (!worldConfig.server.showGmInWhoList && !HasGMPermissions())
        {
            if (plr->GetSession()->HasGMPermissions())
                continue;
        }

        // Team check
        if (!HasGMPermissions() && plr->GetTeam() != team && !plr->GetSession()->HasGMPermissions() && !worldConfig.player.isInterfactionMiscEnabled)
            continue;

        ++total_count;

        // Add by default, if we don't have any checks
        bool add = true;

        // Chat name
        if (cname && recv_packet.player_name.compare(plr->getName()) != 0)
            continue;

        // Guild name
        if (gname)
        {
#if VERSION_STRING != Cata
            if (!plr->GetGuild() || recv_packet.guild_name != plr->GetGuild()->getGuildName())
                continue;
#else
            if (!plr->GetGuild() || recv_packet.guild_name.compare(plr->GetGuild()->getName()) != 0)
                continue;
#endif
        }

        // Level check
        if (recv_packet.min_level && recv_packet.max_level)
        {
            // skip players outside of level range
            if (plr->getLevel() < recv_packet.min_level || plr->getLevel() > recv_packet.max_level)
                continue;
        }

        // Zone id compare
        if (recv_packet.zone_count)
        {
            // people that fail the zone check don't get added
            add = false;
            for (uint32_t i = 0; i < recv_packet.zone_count; ++i)
            {
                if (recv_packet.zones[i] == plr->GetZoneId())
                {
                    add = true;
                    break;
                }
            }
        }

        if (!((recv_packet.class_mask >> 1) & plr->getClassMask()) || !((recv_packet.race_mask >> 1) & plr->getRaceMask()))
            add = false;

        // skip players that fail zone check
        if (!add)
            continue;

        if (recv_packet.name_count)
        {
            // people that fail name check don't get added
            add = false;
            for (uint32_t i = 0; i < recv_packet.name_count; ++i)
            {
                if (!strnicmp(recv_packet.names[i].c_str(), plr->getName().c_str(), recv_packet.names[i].length()))
                {
                    add = true;
                    break;
                }
            }
        }

        if (!add)
            continue;

        // if we're here, it means we've passed all tests
        data << plr->getName().c_str();

#if VERSION_STRING != Cata
        if (plr->m_playerInfo->guild)
            data << plr->m_playerInfo->guild->getGuildName();
        else
            data << uint8_t(0);
#else
        if (plr->m_playerInfo->m_guild)
            data << sGuildMgr.getGuildById(plr->m_playerInfo->m_guild)->getName().c_str();
        else
            data << uint8_t(0);
#endif

        data << plr->getLevel();
        data << uint32_t(plr->getClass());
        data << uint32_t(plr->getRace());
        data << plr->getGender();
        data << uint32_t(plr->GetZoneId());
        ++sent_count;
    }
    objmgr._playerslock.ReleaseReadLock();
    data.wpos(0);
    data << sent_count;
    data << sent_count;

    SendPacket(&data);
}

void WorldSession::handleSetSelectionOpcode(WorldPacket& recvPacket)
{
    CmsgSetSelection recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    _player->SetSelection(recv_packet.guid);

    if (_player->m_comboPoints)
        _player->UpdateComboPoints();

    _player->setTargetGuid(recv_packet.guid);
    if (recv_packet.guid == 0)
    {
        if (_player->IsInWorld())
            _player->CombatStatusHandler_ResetPvPTimeout();
    }
}

void WorldSession::handleTogglePVPOpcode(WorldPacket& /*recvPacket*/)
{
    _player->PvPToggle();
}

void WorldSession::handleTutorialFlag(WorldPacket& recvPacket)
{
    CmsgTutorialFlag recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const uint32_t tutorial_index = (recv_packet.flag / 32);
    const uint32_t tutorial_status = (recv_packet.flag % 32);

    if (tutorial_index >= 7)
    {
        Disconnect();
        return;
    }

    uint32_t tutorial_flag = GetPlayer()->GetTutorialInt(tutorial_index);
    tutorial_flag |= (1 << tutorial_status);
    GetPlayer()->SetTutorialInt(tutorial_index, tutorial_flag);

    LOG_DEBUG("Received Tutorial flag: (%u).", recv_packet.flag);
}

void WorldSession::handleTutorialClear(WorldPacket& /*recvPacket*/)
{
    for (uint32_t id = 0; id < 8; ++id)
        GetPlayer()->SetTutorialInt(id, 0xFFFFFFFF);
}

void WorldSession::handleTutorialReset(WorldPacket& /*recvPacket*/)
{
    for (uint32_t id = 0; id < 8; ++id)
        GetPlayer()->SetTutorialInt(id, 0x00000000);
}

void WorldSession::handleLogoutRequestOpcode(WorldPacket& /*recvPacket*/)
{
    CHECK_INWORLD_RETURN

    auto player = GetPlayer();
    if (player)
    {
        if (!sHookInterface.OnLogoutRequest(player))
        {
            SendPacket(SmsgLogoutResponse(true).serialise().get());
            return;
        }

        if (GetPermissionCount() == 0)
        {
            if (player->CombatStatus.IsInCombat() || player->DuelingWith != nullptr)
            {
                SendPacket(SmsgLogoutResponse(true).serialise().get());
                return;
            }

            if (player->m_isResting || player->isOnTaxi() || worldConfig.player.enableInstantLogoutForAccessType == 2)
            {
                SetLogoutTimer(1);
                return;
            }
        }

        if (GetPermissionCount() > 0)
        {
            if (player->m_isResting || player->isOnTaxi() || worldConfig.player.enableInstantLogoutForAccessType > 0)
            {
                SetLogoutTimer(1);
                return;
            }
        }

        SendPacket(SmsgLogoutResponse(false).serialise().get());

        player->setMoveRoot(true);
        LoggingOut = true;

        player->addUnitFlags(UNIT_FLAG_LOCK_PLAYER);

        player->setStandState(STANDSTATE_SIT);
        SetLogoutTimer(20000);
    }
}

void WorldSession::handleSetSheathedOpcode(WorldPacket& recvPacket)
{
    CmsgSetSheathed recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    GetPlayer()->setSheathType(static_cast<uint8_t>(recv_packet.type));
}
