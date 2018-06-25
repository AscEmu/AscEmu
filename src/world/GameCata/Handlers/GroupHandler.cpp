/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Map/MapMgr.h"
#include "Spell/SpellAuras.h"
#include "Map/WorldCreator.h"
#include "Objects/ObjectMgr.h"
#include "Server/Packets/SmsgPartyCommandResult.h"

using namespace AscEmu::Packets;

void WorldSession::SendEmptyGroupList(Player* player)
{
    WorldPacket data(SMSG_GROUP_LIST, 28);
    data << uint8_t(0x10);
    data << uint8_t(0);
    data << uint8_t(0);
    data << uint8_t(0);
    data << uint64_t(0);
    data << uint32_t(0);
    data << uint32_t(0);
    data << uint64_t(0);
    player->GetSession()->SendPacket(&data);
}

void WorldSession::HandleGroupInviteResponseOpcode(WorldPacket& recvData)
{
    recvData.readBit();                    //unk
    bool acceptInvite = recvData.readBit();

    if (acceptInvite)
    {
        if (_player->GetGroup() != nullptr)
            return;

        Player* group_inviter = objmgr.GetPlayer(_player->GetInviter());
        if (!group_inviter)
            return;

        group_inviter->SetInviter(0);
        _player->SetInviter(0);

        Group* group = group_inviter->GetGroup();
        if (group != nullptr)
        {
            group->AddMember(_player->m_playerInfo);
            _player->iInstanceType = group->m_difficulty;
            _player->SendDungeonDifficulty();
            return;
        }
        else
        {
            group = new Group(true);
            group->m_difficulty = group_inviter->iInstanceType;
            group->AddMember(group_inviter->m_playerInfo);
            group->AddMember(_player->m_playerInfo);
            _player->iInstanceType = group->m_difficulty;
            _player->SendDungeonDifficulty();

            Instance* instance = sInstanceMgr.GetInstanceByIds(group_inviter->GetMapId(), group_inviter->GetInstanceID());
            if (instance != nullptr && instance->m_creatorGuid == group_inviter->getGuidLow())
            {
                group->m_instanceIds[instance->m_mapId][instance->m_difficulty] = instance->m_instanceId;
                instance->m_creatorGroup = group->GetID();
                instance->m_creatorGuid = 0;
                instance->SaveToDB();
            }
        }
    }
    else
    {
        Player* group_inviter = objmgr.GetPlayer(_player->GetInviter());
        if (group_inviter == nullptr)
            return;

        group_inviter->SetInviter(0);
        _player->SetInviter(0);

        WorldPacket data(SMSG_GROUP_DECLINE, strlen(GetPlayer()->getName().c_str()));
        data << GetPlayer()->getName().c_str();
        group_inviter->GetSession()->SendPacket(&data);
    }
}

void WorldSession::HandleGroupSetRolesOpcode(WorldPacket& recvData)
{
    uint32_t newRole;

    recvData >> newRole;

    ObjectGuid target_guid; // Target GUID
    ObjectGuid player_guid = GetPlayer()->getGuid();

    target_guid[2] = recvData.readBit();
    target_guid[6] = recvData.readBit();
    target_guid[3] = recvData.readBit();
    target_guid[7] = recvData.readBit();
    target_guid[5] = recvData.readBit();
    target_guid[1] = recvData.readBit();
    target_guid[0] = recvData.readBit();
    target_guid[4] = recvData.readBit();

    recvData.ReadByteSeq(target_guid[6]);
    recvData.ReadByteSeq(target_guid[4]);
    recvData.ReadByteSeq(target_guid[1]);
    recvData.ReadByteSeq(target_guid[3]);
    recvData.ReadByteSeq(target_guid[0]);
    recvData.ReadByteSeq(target_guid[5]);
    recvData.ReadByteSeq(target_guid[2]);
    recvData.ReadByteSeq(target_guid[7]);

    WorldPacket data(SMSG_GROUP_SET_ROLE, 24);

    data.writeBit(player_guid[1]);

    data.writeBit(target_guid[0]);
    data.writeBit(target_guid[2]);
    data.writeBit(target_guid[4]);
    data.writeBit(target_guid[7]);
    data.writeBit(target_guid[3]);

    data.writeBit(player_guid[7]);

    data.writeBit(target_guid[5]);

    data.writeBit(player_guid[5]);
    data.writeBit(player_guid[4]);
    data.writeBit(player_guid[3]);

    data.writeBit(target_guid[6]);

    data.writeBit(player_guid[2]);
    data.writeBit(player_guid[6]);

    data.writeBit(target_guid[1]);

    data.writeBit(player_guid[0]);

    data.WriteByteSeq(player_guid[7]);

    data.WriteByteSeq(target_guid[3]);

    data.WriteByteSeq(player_guid[6]);

    data.WriteByteSeq(target_guid[4]);
    data.WriteByteSeq(target_guid[0]);

    data << uint32_t(newRole);        // role

    data.WriteByteSeq(target_guid[6]);
    data.WriteByteSeq(target_guid[2]);

    data.WriteByteSeq(player_guid[0]);
    data.WriteByteSeq(player_guid[4]);

    data.WriteByteSeq(target_guid[1]);

    data.WriteByteSeq(player_guid[3]);
    data.WriteByteSeq(player_guid[5]);
    data.WriteByteSeq(player_guid[2]);

    data.WriteByteSeq(target_guid[5]);
    data.WriteByteSeq(target_guid[7]);

    data.WriteByteSeq(player_guid[1]);

    data << uint32_t(0);              // unk

    if (GetPlayer()->GetGroup())
        GetPlayer()->GetGroup()->SendPacketToAll(&data);
    else
        SendPacket(&data);
}

void WorldSession::HandleGroupRequestJoinUpdatesOpcode(WorldPacket& /*recvData*/)
{
    Group* group = _player->GetGroup();
    if (group != nullptr)
    {
        WorldPacket data(SMSG_REAL_GROUP_UPDATE, 13);
        data << uint8_t(group->getGroupType());
        data << uint32_t(group->GetMembersCount());
        data << uint64_t(0);  // unk
        SendPacket(&data);
    }
}

void WorldSession::HandleGroupRoleCheckBeginOpcode(WorldPacket& recvData)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    if (recvData.isEmpty())
    {
        if (group->GetLeader()->guid != GetPlayer()->getGuid() && group->GetMainAssist()->guid != GetPlayer()->getGuid())
            return;

        ObjectGuid guid = GetPlayer()->getGuid();

        WorldPacket data(SMSG_ROLE_CHECK_BEGIN, 8);
        data.writeBit(guid[1]);
        data.writeBit(guid[5]);
        data.writeBit(guid[7]);
        data.writeBit(guid[3]);
        data.writeBit(guid[2]);
        data.writeBit(guid[4]);
        data.writeBit(guid[0]);
        data.writeBit(guid[6]);

        data.WriteByteSeq(guid[4]);
        data.WriteByteSeq(guid[7]);
        data.WriteByteSeq(guid[0]);
        data.WriteByteSeq(guid[5]);
        data.WriteByteSeq(guid[1]);
        data.WriteByteSeq(guid[6]);
        data.WriteByteSeq(guid[2]);
        data.WriteByteSeq(guid[3]);

        group->SendPacketToAll(&data);
    }
}
