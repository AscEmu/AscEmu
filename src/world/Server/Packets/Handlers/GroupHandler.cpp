/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

void WorldSession::SendPartyCommandResult(Player* pPlayer, uint32 p1, std::string name, uint32 err)
{
    WorldPacket data(SMSG_PARTY_COMMAND_RESULT, name.size() + 18);   //guessed
    data << uint32(p1);     // 0 = invite, 1 = uninvite, 2 = leave, 3 = swap

    if (!name.length())
        data << uint8(0);
    else
        data << name.c_str();

    data << uint32(err);
    data << uint32(0);      // unk
    data << uint64(0);      // unk

    SendPacket(&data);
}

void WorldSession::SendEmptyGroupList(Player* player)
{
    WorldPacket data(SMSG_GROUP_LIST, 28);
    data << uint8(0x10);
    data << uint8(0);
    data << uint8(0);
    data << uint8(0);
    data << uint64(0);
    data << uint32(0);
    data << uint32(0);
    data << uint64(0);
    player->GetSession()->SendPacket(&data);
}

void WorldSession::HandleGroupInviteOpcode(WorldPacket& recv_data)
{
    std::string membername, realmname;
    ObjectGuid unk_guid;            // unused

    recv_data.read_skip<uint32>();
    recv_data.read_skip<uint32>();

    unk_guid[2] = recv_data.readBit();
    unk_guid[7] = recv_data.readBit();

    uint8 realmLen = recv_data.readBits(9);

    unk_guid[3] = recv_data.readBit();

    uint8 nameLen = recv_data.readBits(10);

    unk_guid[5] = recv_data.readBit();
    unk_guid[4] = recv_data.readBit();
    unk_guid[6] = recv_data.readBit();
    unk_guid[0] = recv_data.readBit();
    unk_guid[1] = recv_data.readBit();

    recv_data.ReadByteSeq(unk_guid[4]);
    recv_data.ReadByteSeq(unk_guid[7]);
    recv_data.ReadByteSeq(unk_guid[6]);

    membername = recv_data.ReadString(nameLen);
    realmname = recv_data.ReadString(realmLen);

    recv_data.ReadByteSeq(unk_guid[1]);
    recv_data.ReadByteSeq(unk_guid[0]);
    recv_data.ReadByteSeq(unk_guid[5]);
    recv_data.ReadByteSeq(unk_guid[3]);
    recv_data.ReadByteSeq(unk_guid[2]);

    if (_player->HasBeenInvited())
        return;

    Player* player = objmgr.GetPlayer(membername.c_str(), false);
    if (player == nullptr)
    {
        SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
        return;
    }

    if (_player == player)
        return;

    if (_player->InGroup() && !_player->IsGroupLeader())
    {
        SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        return;
    }

    Group* group = _player->GetGroup();
    if (group != nullptr)
    {
        if (group->IsFull())
        {
            SendPartyCommandResult(_player, 0, "", ERR_PARTY_IS_FULL);
            return;
        }
    }

    ObjectGuid invitedGuid = player->GetGUID();

    if (player->InGroup())
    {
        SendPartyCommandResult(_player, player->GetGroup()->GetGroupType(), membername, ERR_PARTY_ALREADY_IN_GROUP);
        WorldPacket data(SMSG_GROUP_INVITE, 45);
        data.writeBit(0);

        data.writeBit(invitedGuid[0]);
        data.writeBit(invitedGuid[3]);
        data.writeBit(invitedGuid[2]);

        data.writeBit(0);               //not in group

        data.writeBit(invitedGuid[6]);
        data.writeBit(invitedGuid[5]);

        data.writeBits(0, 9);

        data.writeBit(invitedGuid[4]);

        data.writeBits(strlen(GetPlayer()->GetName()), 7);

        data.writeBits(0, 24);
        data.writeBit(0);

        data.writeBit(invitedGuid[1]);
        data.writeBit(invitedGuid[7]);

        data.flushBits();

        data.WriteByteSeq(invitedGuid[1]);
        data.WriteByteSeq(invitedGuid[4]);

        data << int32(getMSTime());
        data << int32(0);
        data << int32(0);

        data.WriteByteSeq(invitedGuid[6]);
        data.WriteByteSeq(invitedGuid[0]);
        data.WriteByteSeq(invitedGuid[2]);
        data.WriteByteSeq(invitedGuid[3]);
        data.WriteByteSeq(invitedGuid[5]);
        data.WriteByteSeq(invitedGuid[7]);

        data.WriteString(GetPlayer()->GetName());

        data << int32(0);

        player->GetSession()->SendPacket(&data);
        return;
    }

    if (player->GetTeam() != _player->GetTeam() && _player->GetSession()->GetPermissionCount() == 0 && !sWorld.interfaction_group)
    {
        SendPartyCommandResult(_player, 0, membername, ERR_PARTY_WRONG_FACTION);
        return;
    }

    if (player->HasBeenInvited())
    {
        SendPartyCommandResult(_player, 0, membername, ERR_PARTY_ALREADY_IN_GROUP);
        return;
    }

    if (player->Social_IsIgnoring(_player->GetLowGUID()))
    {
        SendPartyCommandResult(_player, 0, membername, ERR_PARTY_IS_IGNORING_YOU);
        return;
    }

    if (player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) && !_player->GetSession()->HasPermissions())
    {
        SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
        return;
    }

    WorldPacket data(SMSG_GROUP_INVITE, 45);
    data.writeBit(0);

    data.writeBit(invitedGuid[0]);
    data.writeBit(invitedGuid[3]);
    data.writeBit(invitedGuid[2]);

    data.writeBit(1);               //not in group

    data.writeBit(invitedGuid[6]);
    data.writeBit(invitedGuid[5]);

    data.writeBits(0, 9);

    data.writeBit(invitedGuid[4]);

    data.writeBits(strlen(GetPlayer()->GetName()), 7);
    data.writeBits(0, 24);
    data.writeBit(0);

    data.writeBit(invitedGuid[1]);
    data.writeBit(invitedGuid[7]);

    data.flushBits();

    data.WriteByteSeq(invitedGuid[1]);
    data.WriteByteSeq(invitedGuid[4]);

    data << int32(getMSTime());
    data << int32(0);
    data << int32(0);

    data.WriteByteSeq(invitedGuid[6]);
    data.WriteByteSeq(invitedGuid[0]);
    data.WriteByteSeq(invitedGuid[2]);
    data.WriteByteSeq(invitedGuid[3]);
    data.WriteByteSeq(invitedGuid[5]);
    data.WriteByteSeq(invitedGuid[7]);

    data.WriteString(GetPlayer()->GetName());

    data << int32(0);

    player->GetSession()->SendPacket(&data);

    SendPartyCommandResult(_player, 0, membername, ERR_PARTY_NO_ERROR);

    player->SetInviter(_player->GetLowGUID());
}

void WorldSession::HandleGroupInviteResponseOpcode(WorldPacket& recv_data)
{
    bool accept_invite = false;

    recv_data.readBit();                    //unk
    accept_invite = recv_data.readBit();

    if (accept_invite)
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
            if (instance != nullptr && instance->m_creatorGuid == group_inviter->GetLowGUID())
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
        if (!group_inviter)
            return;

        group_inviter->SetInviter(0);
        _player->SetInviter(0);

        WorldPacket data(SMSG_GROUP_DECLINE, strlen(GetPlayer()->GetName()));
        data << GetPlayer()->GetName();
        group_inviter->GetSession()->SendPacket(&data);
    }
}

void WorldSession::HandleGroupSetRolesOpcode(WorldPacket& recvData)
{
    LOG_DEBUG("WORLD: Received CMSG_GROUP_SET_ROLES");

    uint32 newRole;

    ObjectGuid target_guid; // Target GUID

    ObjectGuid player_guid = GetPlayer()->GetGUID();

    recvData >> newRole;

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

    data << uint32(newRole);        // role

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

    data << uint32(0);              // unk

    if (GetPlayer()->GetGroup())
        GetPlayer()->GetGroup()->SendPacketToAll(&data);
    else
        SendPacket(&data);
}

void WorldSession::HandleGroupDisbandOpcode(WorldPacket& recv_data)
{
    Group* group = _player->GetGroup();
    if (group == nullptr)
        return;

    if (group->GetGroupType() & GROUP_TYPE_BG)
        return;

    group->RemovePlayer(_player->m_playerInfo);
}

void WorldSession::HandleLootMethodOpcode(WorldPacket& recv_data)
{
    uint32 loot_method;
    uint64 loot_master;
    uint32 loot_threshold;

    recv_data >> loot_method;
    recv_data >> loot_master;
    recv_data >> loot_threshold;

    Group* target_group = _player->GetGroup();
    if (target_group == nullptr)
        return;

    if (loot_method > PARTY_LOOT_NBG)
        return;

    if (loot_threshold < ITEM_QUALITY_UNCOMMON_GREEN || loot_threshold > ITEM_QUALITY_ARTIFACT_LIGHT_YELLOW)
        return;

    if (loot_method == PARTY_LOOT_MASTER && !target_group->HasMember(objmgr.GetPlayer((uint32)loot_master)))
        return;

    if (target_group->GetLeader() != _player->getPlayerInfo())
    {
        SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        return;
    }

    Player* player = objmgr.GetPlayer((uint32)loot_master);
    if (player != nullptr)
        target_group->SetLooter(player, static_cast<uint8>(loot_method), static_cast<uint16>(loot_threshold));
    else
        target_group->SetLooter(_player, static_cast<uint8>(loot_method), static_cast<uint16>(loot_threshold));
}

/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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

//#include "StdAfx.h"

//////////////////////////////////////////////////////////////
/// This function handles CMSG_GROUP_INVITE
//////////////////////////////////////////////////////////////
//void WorldSession::HandleGroupInviteOpcode(WorldPacket& recv_data)
//{
//    CHECK_INWORLD_RETURN;
//    CHECK_PACKET_SIZE(recv_data, 1);
//    WorldPacket data(100);
//    std::string membername;
//    Group* group = NULL;
//
//    recv_data >> membername;
//    if (_player->HasBeenInvited())
//        return;
//
//    Player* player = objmgr.GetPlayer(membername.c_str(), false);
//
//    if (player == NULL)
//    {
//        SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
//        return;
//    }
//
//    if (player == _player)
//    {
//        return;
//    }
//
//    if (_player->InGroup() && !_player->IsGroupLeader())
//    {
//        SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
//        return;
//    }
//
//    group = _player->GetGroup();
//    if (group != NULL)
//    {
//        if (group->IsFull())
//        {
//            SendPartyCommandResult(_player, 0, "", ERR_PARTY_IS_FULL);
//            return;
//        }
//    }
//
//    if (player->InGroup())
//    {
//        SendPartyCommandResult(_player, player->GetGroup()->GetGroupType(), membername, ERR_PARTY_ALREADY_IN_GROUP);
//        data.SetOpcode(SMSG_GROUP_INVITE);
//        data << uint8(0);
//        data << GetPlayer()->GetName();
//        player->GetSession()->SendPacket(&data);
//        return;
//    }
//
//    if (player->GetTeam() != _player->GetTeam() && _player->GetSession()->GetPermissionCount() == 0 && !sWorld.interfaction_group)
//    {
//        SendPartyCommandResult(_player, 0, membername, ERR_PARTY_WRONG_FACTION);
//        return;
//    }
//
//    if (player->HasBeenInvited())
//    {
//        SendPartyCommandResult(_player, 0, membername, ERR_PARTY_ALREADY_IN_GROUP);
//        return;
//    }
//
//    if (player->Social_IsIgnoring(_player->GetLowGUID()))
//    {
//        SendPartyCommandResult(_player, 0, membername, ERR_PARTY_IS_IGNORING_YOU);
//        return;
//    }
//
//    if (player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM) && !_player->GetSession()->HasPermissions())
//    {
//        SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
//        return;
//    }
//
//    data.SetOpcode(SMSG_GROUP_INVITE);
//    data << uint8(1);
//    data << GetPlayer()->GetName();
//
//    player->GetSession()->SendPacket(&data);
//
//    SendPartyCommandResult(_player, 0, membername, ERR_PARTY_NO_ERROR);
//
//    // 16/08/06 - change to guid to prevent very unlikely event of a crash in deny, etc
//    player->SetInviter(_player->GetLowGUID());
//}

///////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_CANCEL:
///////////////////////////////////////////////////////////////
//void WorldSession::HandleGroupCancelOpcode(WorldPacket& recv_data)
//{
//    CHECK_INWORLD_RETURN
//
//        LOG_DEBUG("WORLD: got CMSG_GROUP_CANCEL.");
//}

////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_ACCEPT:
////////////////////////////////////////////////////////////////
//void WorldSession::HandleGroupAcceptOpcode(WorldPacket& recv_data)
//{
//    CHECK_INWORLD_RETURN;
//
//    // we are in group already
//    if (_player->GetGroup() != NULL)
//        return;
//
//    Player* player = objmgr.GetPlayer(_player->GetInviter());
//    if (!player)
//        return;
//
//    player->SetInviter(0);
//    _player->SetInviter(0);
//
//    Group* grp = player->GetGroup();
//    if (grp != NULL)
//    {
//        grp->AddMember(_player->m_playerInfo);
//        _player->iInstanceType = grp->m_difficulty;
//        _player->SendDungeonDifficulty();
//
//        //sInstanceSavingManager.ResetSavedInstancesForPlayer(_player);
//        return;
//    }
//
//    // If we're this far, it means we have no existing group, and have to make one.
//    grp = new Group(true);
//    grp->m_difficulty = player->iInstanceType;
//    grp->AddMember(player->m_playerInfo);        // add the inviter first, therefore he is the leader
//    grp->AddMember(_player->m_playerInfo);    // add us.
//    _player->iInstanceType = grp->m_difficulty;
//    _player->SendDungeonDifficulty();
//
//    Instance* instance = sInstanceMgr.GetInstanceByIds(player->GetMapId(), player->GetInstanceID());
//    if (instance != NULL && instance->m_creatorGuid == player->GetLowGUID())
//    {
//        grp->m_instanceIds[instance->m_mapId][instance->m_difficulty] = instance->m_instanceId;
//        instance->m_creatorGroup = grp->GetID();
//        instance->m_creatorGuid = 0;
//        instance->SaveToDB();
//    }
//
//    //sInstanceSavingManager.ResetSavedInstancesForPlayer(_player);
//
//    // Currentgroup and all that shit are set by addmember.
//}

///////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_DECLINE:
//////////////////////////////////////////////////////////////////////////////////////
//void WorldSession::HandleGroupDeclineOpcode(WorldPacket& recv_data)
//{
//    CHECK_INWORLD_RETURN;
//    WorldPacket data(SMSG_GROUP_DECLINE, 100);
//
//    Player* player = objmgr.GetPlayer(_player->GetInviter());
//    if (!player)
//        return;
//
//    data << GetPlayer()->GetName();
//
//    player->GetSession()->SendPacket(&data);
//    player->SetInviter(0);
//    _player->SetInviter(0);
//}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_UNINVITE(unused since 3.1.3):
//////////////////////////////////////////////////////////////////////////////////////////
//void WorldSession::HandleGroupUninviteOpcode(WorldPacket& recv_data)
//{
//    CHECK_INWORLD_RETURN;
//    CHECK_PACKET_SIZE(recv_data, 1);
//    std::string membername;
//    Group* group;
//    Player* player;
//    PlayerInfo* info;
//
//    recv_data >> membername;
//
//    player = objmgr.GetPlayer(membername.c_str(), false);
//    info = objmgr.GetPlayerInfoByName(membername.c_str());
//    if (player == NULL && info == NULL)
//    {
//        SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
//        return;
//    }
//
//    if (!_player->InGroup() || (info != NULL && info->m_Group != _player->GetGroup()))
//    {
//        SendPartyCommandResult(_player, 0, membername, ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
//        return;
//    }
//
//    if (!_player->IsGroupLeader())
//    {
//        if (player == NULL)
//        {
//            SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
//            return;
//        }
//        else if (_player != player)
//        {
//            SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
//            return;
//        }
//    }
//
//    group = _player->GetGroup();
//
//    if (group != NULL)
//    {
//        group->RemovePlayer(info);
//    }
//}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_UNINVITE_GUID(used since 3.1.3):
//////////////////////////////////////////////////////////////////////////////////////////
//void WorldSession::HandleGroupUninviteGuidOpcode(WorldPacket& recv_data)
//{
//    CHECK_INWORLD_RETURN;
//    CHECK_PACKET_SIZE(recv_data, 1);
//    uint64 PlayerGUID;
//    std::string membername = "unknown";
//    Group* group;
//    Player* player;
//    PlayerInfo* info;
//
//    recv_data >> PlayerGUID;
//
//    player = objmgr.GetPlayer(Arcemu::Util::GUID_LOPART(PlayerGUID));
//    info = objmgr.GetPlayerInfo(Arcemu::Util::GUID_LOPART(PlayerGUID));
//    // If both conditions match the player gets thrown out of the group by the server since this means the character is deleted
//    if (player == NULL && info == NULL)
//    {
//        SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
//        return;
//    }
//
//    membername = player ? player->GetName() : info->name;
//
//    if (!_player->InGroup() || (info != NULL && info->m_Group != _player->GetGroup()))
//    {
//        SendPartyCommandResult(_player, 0, membername, ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
//        return;
//    }
//
//    if (!_player->IsGroupLeader())
//    {
//        if (player == NULL)
//        {
//            SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
//            return;
//        }
//        else if (_player != player)
//        {
//            SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
//            return;
//        }
//    }
//
//    group = _player->GetGroup();
//    if (group != NULL)
//    {
//        group->RemovePlayer(info);
//    }
//}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_SET_LEADER:
//////////////////////////////////////////////////////////////////////////////////////////
//void WorldSession::HandleGroupSetLeaderOpcode(WorldPacket& recv_data)
//{
//    CHECK_INWORLD_RETURN;
//    // important note _player->GetName() can be wrong.
//    CHECK_PACKET_SIZE(recv_data, 1);
//    WorldPacket data;
//    uint64 MemberGuid;
//    Player* player;
//
//    recv_data >> MemberGuid;
//
//    player = objmgr.GetPlayer((uint32)MemberGuid);
//
//    if (player == NULL)
//    {
//        //SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
//        SendPartyCommandResult(_player, 0, _player->GetName(), ERR_PARTY_CANNOT_FIND);
//        return;
//    }
//
//    if (!_player->IsGroupLeader())
//    {
//        SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
//        return;
//    }
//
//    if (player->GetGroup() != _player->GetGroup())
//    {
//        //SendPartyCommandResult(_player, 0, membername, ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
//        SendPartyCommandResult(_player, 0, _player->GetName(), ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
//        return;
//    }
//
//    Group* pGroup = _player->GetGroup();
//    if (pGroup)
//        pGroup->SetLeader(player, false);
//}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_GROUP_DISBAND:
//////////////////////////////////////////////////////////////////////////////////////////
//void WorldSession::HandleGroupDisbandOpcode(WorldPacket& recv_data)
//{
//    // this is actually leaving a party, disband is not possible anymore
//    CHECK_INWORLD_RETURN;
//    Group* pGroup = _player->GetGroup();
//    if (pGroup == NULL)
//        return;
//
//    // cant leave a battleground group (blizzlike 3.3.3)
//    if (pGroup->GetGroupType() & GROUP_TYPE_BG)
//        return;
//
//    pGroup->RemovePlayer(_player->m_playerInfo);
//}

//////////////////////////////////////////////////////////////////////////////////////////
///This function handles CMSG_LOOT_METHOD:
//////////////////////////////////////////////////////////////////////////////////////////
//void WorldSession::HandleLootMethodOpcode(WorldPacket& recv_data)
//{
//    CHECK_INWORLD_RETURN;
//    CHECK_PACKET_SIZE(recv_data, 16);
//
//    uint32 lootMethod;
//    uint64 lootMaster;
//    uint32 threshold;
//
//    recv_data >> lootMethod;
//    recv_data >> lootMaster;
//    recv_data >> threshold;
//
//    if (!_player->IsGroupLeader())
//    {
//        SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
//        return;
//    }
//
//    Group* pGroup = _player->GetGroup();
//
//    if (pGroup == NULL)
//        return;
//
//    Player* plr = objmgr.GetPlayer((uint32)lootMaster);
//    if (plr != NULL)
//    {
//        pGroup->SetLooter(plr, static_cast<uint8>(lootMethod), static_cast<uint16>(threshold));
//    }
//    else
//    {
//        pGroup->SetLooter(_player, static_cast<uint8>(lootMethod), static_cast<uint16>(threshold));
//    }
//
//}

//void WorldSession::HandleMinimapPingOpcode(WorldPacket& recv_data)
//{
//    CHECK_INWORLD_RETURN;
//    CHECK_PACKET_SIZE(recv_data, 8);
//    if (!_player->InGroup())
//        return;
//    Group* party = _player->GetGroup();
//    if (!party)return;
//
//    float x, y;
//    recv_data >> x;
//    recv_data >> y;
//
//    WorldPacket data;
//    data.SetOpcode(MSG_MINIMAP_PING);
//    data << _player->GetGUID();
//    data << x;
//    data << y;
//    party->SendPacketToAllButOne(&data, _player);
//}

//void WorldSession::HandleSetPlayerIconOpcode(WorldPacket& recv_data)
//{
//    CHECK_INWORLD_RETURN;
//    uint64 guid;
//    uint8 icon;
//    Group* pGroup = _player->GetGroup();
//    if (pGroup == NULL)
//        return;
//
//    recv_data >> icon;
//    if (icon == 0xFF)
//    {
//        // client request
//        WorldPacket data(MSG_RAID_TARGET_UPDATE, 73);
//        data << uint8(1);
//        for (uint8 i = 0; i < 8; ++i)
//            data << i << pGroup->m_targetIcons[i];
//
//        SendPacket(&data);
//    }
//    else if (_player->IsGroupLeader())
//    {
//        recv_data >> guid;
//        if (icon > 7)
//            return;            // whoops, buffer overflow :p
//
//        //removing other icon
//        for (uint8 i = 0; i < 8; ++i)
//        {
//            if (pGroup->m_targetIcons[i] == guid)
//            {
//                WorldPacket data(MSG_RAID_TARGET_UPDATE, 10);
//                data << uint8(0);
//                data << uint64(0);
//                data << uint8(i);
//                data << uint64(0);
//                pGroup->SendPacketToAll(&data);
//
//                pGroup->m_targetIcons[i] = 0;
//                break;
//            }
//        }
//        // setting icon
//        WorldPacket data(MSG_RAID_TARGET_UPDATE, 10);
//        data << uint8(0);
//        data << uint64(GetPlayer()->GetGUID());
//        data << icon;
//        data << guid;
//        pGroup->SendPacketToAll(&data);
//
//        pGroup->m_targetIcons[icon] = guid;
//    }
//}

//void WorldSession::HandlePartyMemberStatsOpcode(WorldPacket& recv_data)
//{
//    CHECK_INWORLD_RETURN
//
//    uint64 guid;
//    recv_data >> guid;
//
//    Player* plr = _player->GetMapMgr()->GetPlayer((uint32)guid);
//
//    if (!_player->GetGroup() || !plr)
//    {
//        WorldPacket data(SMSG_PARTY_MEMBER_STATS_FULL, 3 + 4 + 2);
//        data << uint8(0);                                   // only for SMSG_PARTY_MEMBER_STATS_FULL, probably arena/bg related
//        data.appendPackGUID(guid);
//        data << uint32(GROUP_UPDATE_FLAG_STATUS);
//        data << uint16(MEMBER_STATUS_OFFLINE);
//        SendPacket(&data);
//        return;
//    }
//
//    if (!_player->GetGroup()->HasMember(plr))
//        return;
//
//    if (_player->IsVisible(plr->GetGUID()))
//        return;
//
//    Pet* pet = plr->GetSummon();
//
//    WorldPacket data(SMSG_PARTY_MEMBER_STATS_FULL, 4 + 2 + 2 + 2 + 1 + 2 * 6 + 8 + 1 + 8);
//    data << uint8(0);                                       // only for SMSG_PARTY_MEMBER_STATS_FULL, probably arena/bg related
//    data.append(plr->GetNewGUID());
//
//    uint32 mask1 = 0x00040BFF;                              // common mask, real flags used 0x000040BFF
//    if (pet)
//        mask1 = 0x7FFFFFFF;                                 // for hunters and other classes with pets
//
//    uint8 powerType = plr->GetPowerType();
//    data << uint32(mask1);
//    data << uint16(MEMBER_STATUS_ONLINE);
//    data << uint32(plr->GetHealth());
//    data << uint32(plr->GetMaxHealth());
//    data << uint8(powerType);
//    data << uint16(plr->GetPower(powerType));
//    data << uint16(plr->GetMaxPower(powerType));
//    data << uint16(plr->getLevel());
//    data << uint16(plr->GetZoneId());
//    data << uint16(plr->GetPositionX());
//    data << uint16(plr->GetPositionY());
//
//    uint64 auramask = 0;
//    size_t maskPos = data.wpos();
//    data << uint64(auramask);
//    for (uint8 i = 0; i < 64; ++i)
//    {
//        if (Aura * aurApp = plr->GetAuraWithSlot(i))
//        {
//            auramask |= (uint64(1) << i);
//            data << uint32(aurApp->GetSpellId());
//            data << uint8(1);
//        }
//    }
//    data.put<uint64>(maskPos, auramask);
//
//    if (pet)
//    {
//        uint8 petpowertype = pet->GetPowerType();
//        data << uint64(pet->GetGUID());
//        data << pet->GetName();
//        data << uint16(pet->GetDisplayId());
//        data << uint32(pet->GetHealth());
//        data << uint32(pet->GetMaxHealth());
//        data << uint8(petpowertype);
//        data << uint16(pet->GetPower(petpowertype));
//        data << uint16(pet->GetMaxPower(petpowertype));
//
//        uint64 petauramask = 0;
//        size_t petMaskPos = data.wpos();
//        data << uint64(petauramask);
//        for (uint8 i = 0; i < 64; ++i)
//        {
//            if (Aura * auraApp = pet->GetAuraWithSlot(i))
//            {
//                petauramask |= (uint64(1) << i);
//                data << uint32(auraApp->GetSpellId());
//                data << uint8(1);
//            }
//        }
//        data.put<uint64>(petMaskPos, petauramask);
//    }
//    else
//    {
//        data << uint8(0);      // GROUP_UPDATE_FLAG_PET_NAME
//        data << uint64(0);     // GROUP_UPDATE_FLAG_PET_AURAS
//    }
//
//    SendPacket(&data);
//}