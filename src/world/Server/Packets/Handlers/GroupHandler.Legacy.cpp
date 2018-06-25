/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "Map/MapMgr.h"
#include "Spell/SpellAuras.h"
#include "Map/WorldCreator.h"
#include "Objects/ObjectMgr.h"
#include "Units/Creatures/Pet.h"
#include "Server/Packets/CmsgGroupInvite.h"
#include "Server/Packets/SmsgGroupInvite.h"
#include "Server/Packets/CmsgRequestPartyMemberStats.h"
#include "Server/Packets/SmsgPartyCommandResult.h"

using namespace AscEmu::Packets;

#if VERSION_STRING != Cata

////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_ACCEPT:
////////////////////////////////////////////////////////////////
void WorldSession::HandleGroupAcceptOpcode(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN;

    // we are in group already
    if (_player->GetGroup() != NULL)
        return;

    Player* player = objmgr.GetPlayer(_player->GetInviter());
    if (!player)
        return;

    player->SetInviter(0);
    _player->SetInviter(0);

    Group* grp = player->GetGroup();
    if (grp != NULL)
    {
        grp->AddMember(_player->m_playerInfo);
        _player->iInstanceType = grp->m_difficulty;
        _player->SendDungeonDifficulty();

        //sInstanceSavingManager.ResetSavedInstancesForPlayer(_player);
        return;
    }

    // If we're this far, it means we have no existing group, and have to make one.
    grp = new Group(true);
    grp->m_difficulty = player->iInstanceType;
    grp->AddMember(player->m_playerInfo);        // add the inviter first, therefore he is the leader
    grp->AddMember(_player->m_playerInfo);    // add us.
    _player->iInstanceType = grp->m_difficulty;
    _player->SendDungeonDifficulty();

    Instance* instance = sInstanceMgr.GetInstanceByIds(player->GetMapId(), player->GetInstanceID());
    if (instance != NULL && instance->m_creatorGuid == player->getGuidLow())
    {
        grp->m_instanceIds[instance->m_mapId][instance->m_difficulty] = instance->m_instanceId;
        instance->m_creatorGroup = grp->GetID();
        instance->m_creatorGuid = 0;
        instance->SaveToDB();
    }
}

void WorldSession::HandleSetPlayerIconOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN;
    uint64 guid;
    uint8 icon;
    Group* pGroup = _player->GetGroup();
    if (pGroup == NULL)
        return;

    recv_data >> icon;
    if (icon == 0xFF)
    {
        // client request
        WorldPacket data(MSG_RAID_TARGET_UPDATE, 73);
        data << uint8(1);
        for (uint8 i = 0; i < 8; ++i)
            data << i << pGroup->m_targetIcons[i];

        SendPacket(&data);
    }
    else if (_player->IsGroupLeader())
    {
        recv_data >> guid;
        if (icon > 7)
            return;            // whoops, buffer overflow :p

        //removing other icon
        for (uint8 i = 0; i < 8; ++i)
        {
            if (pGroup->m_targetIcons[i] == guid)
            {
                WorldPacket data(MSG_RAID_TARGET_UPDATE, 10);
                data << uint8(0);
                data << uint64(0);
                data << uint8(i);
                data << uint64(0);
                pGroup->SendPacketToAll(&data);

                pGroup->m_targetIcons[i] = 0;
                break;
            }
        }
        // setting icon
        WorldPacket data(MSG_RAID_TARGET_UPDATE, 10);
        data << uint8(0);
        data << uint64(GetPlayer()->getGuid());
        data << icon;
        data << guid;
        pGroup->SendPacketToAll(&data);

        pGroup->m_targetIcons[icon] = guid;
    }
}

void WorldSession::HandlePartyMemberStatsOpcode(WorldPacket& recvPacket)
{
    CmsgRequestPartyMemberStats recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_REQUEST_PARTY_MEMBER_STATS: %u (guidLow)", recv_packet.guid.getGuidLow());

    const auto requestedPlayer = GetPlayer()->GetMapMgr()->GetPlayer(recv_packet.guid.getGuidLow());

    if (GetPlayer()->GetGroup() == nullptr || requestedPlayer == nullptr)
    {
        WorldPacket data(SMSG_PARTY_MEMBER_STATS_FULL, 3 + 4 + 2);
        data << uint8(0);                                   // only for SMSG_PARTY_MEMBER_STATS_FULL, probably arena/bg related
        data.appendPackGUID(recv_packet.guid);
        data << uint32(GROUP_UPDATE_FLAG_STATUS);
        data << uint16(MEMBER_STATUS_OFFLINE);
        SendPacket(&data);
        return;
    }

    if (!GetPlayer()->GetGroup()->HasMember(requestedPlayer))
        return;

    if (GetPlayer()->IsVisible(requestedPlayer->getGuid()))
        return;

    Pet* pet = requestedPlayer->GetSummon();

    WorldPacket data(SMSG_PARTY_MEMBER_STATS_FULL, 4 + 2 + 2 + 2 + 1 + 2 * 6 + 8 + 1 + 8);
    data << uint8(0);                                       // only for SMSG_PARTY_MEMBER_STATS_FULL, probably arena/bg related
    data.append(requestedPlayer->GetNewGUID());

    uint32 mask1 = 0x00040BFF;                              // common mask, real flags used 0x000040BFF
    if (pet)
        mask1 = 0x7FFFFFFF;                                 // for hunters and other classes with pets

    uint8 powerType = requestedPlayer->getPowerType();
    data << uint32(mask1);
    data << uint16(MEMBER_STATUS_ONLINE);
    data << uint32(requestedPlayer->getHealth());
    data << uint32(requestedPlayer->getMaxHealth());
    data << uint8(powerType);
    data << uint16(requestedPlayer->GetPower(powerType));
    data << uint16(requestedPlayer->GetMaxPower(powerType));
    data << uint16(requestedPlayer->getLevel());
    data << uint16(requestedPlayer->GetZoneId());
    data << uint16(requestedPlayer->GetPositionX());
    data << uint16(requestedPlayer->GetPositionY());

    uint64 auramask = 0;
    size_t maskPos = data.wpos();
    data << uint64(auramask);
    for (uint8 i = 0; i < 64; ++i)
    {
        if (Aura * aurApp = requestedPlayer->GetAuraWithSlot(i))
        {
            auramask |= (uint64(1) << i);
            data << uint32(aurApp->GetSpellId());
            data << uint8(1);
        }
    }
    data.put<uint64>(maskPos, auramask);

    if (pet)
    {
        uint8 petpowertype = pet->getPowerType();
        data << uint64(pet->getGuid());
        data << pet->GetName();
        data << uint16(pet->getDisplayId());
        data << uint32(pet->getHealth());
        data << uint32(pet->getMaxHealth());
        data << uint8(petpowertype);
        data << uint16(pet->GetPower(petpowertype));
        data << uint16(pet->GetMaxPower(petpowertype));

        uint64 petauramask = 0;
        size_t petMaskPos = data.wpos();
        data << uint64(petauramask);
        for (uint8 i = 0; i < 64; ++i)
        {
            if (Aura * auraApp = pet->GetAuraWithSlot(i))
            {
                petauramask |= (uint64(1) << i);
                data << uint32(auraApp->GetSpellId());
                data << uint8(1);
            }
        }
        data.put<uint64>(petMaskPos, petauramask);
    }
    else
    {
        data << uint8(0);      // GROUP_UPDATE_FLAG_PET_NAME
        data << uint64(0);     // GROUP_UPDATE_FLAG_PET_AURAS
    }

    SendPacket(&data);
}
#endif
