/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Map/MapMgr.h"
#include "Spell/SpellAuras.h"
#include "Map/WorldCreator.h"
#include "Objects/ObjectMgr.h"


void WorldSession::SendPartyCommandResult(Player* /*pPlayer*/, uint32_t p1, std::string name, uint32_t err)
{
    WorldPacket data(SMSG_PARTY_COMMAND_RESULT, name.size() + 18);
    data << uint32_t(p1);     // 0 invite, 1 uninvite, 2 leave, 3 swap

    if (!name.length())
        data << uint8_t(0);
    else
        data << name.c_str();

    data << uint32_t(err);
    data << uint32_t(0);      // unk
    data << uint64_t(0);      // unk

    SendPacket(&data);
}

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

void WorldSession::HandleGroupInviteOpcode(WorldPacket& recv_data)
{
    std::string member_name;
    std::string realm_name;
    ObjectGuid unk_guid;

    recv_data.read_skip<uint32_t>();
    recv_data.read_skip<uint32_t>();

    unk_guid[2] = recv_data.readBit();
    unk_guid[7] = recv_data.readBit();

    uint8_t realm_name_length = recv_data.readBits(9);

    unk_guid[3] = recv_data.readBit();

    uint8_t member_name_length = recv_data.readBits(10);

    unk_guid[5] = recv_data.readBit();
    unk_guid[4] = recv_data.readBit();
    unk_guid[6] = recv_data.readBit();
    unk_guid[0] = recv_data.readBit();
    unk_guid[1] = recv_data.readBit();

    recv_data.ReadByteSeq(unk_guid[4]);
    recv_data.ReadByteSeq(unk_guid[7]);
    recv_data.ReadByteSeq(unk_guid[6]);

    member_name = recv_data.ReadString(member_name_length);
    realm_name = recv_data.ReadString(realm_name_length);

    recv_data.ReadByteSeq(unk_guid[1]);
    recv_data.ReadByteSeq(unk_guid[0]);
    recv_data.ReadByteSeq(unk_guid[5]);
    recv_data.ReadByteSeq(unk_guid[3]);
    recv_data.ReadByteSeq(unk_guid[2]);

    if (_player->HasBeenInvited())
        return;

    Player* player = objmgr.GetPlayer(member_name.c_str(), false);
    if (player == nullptr)
    {
        SendPartyCommandResult(_player, 0, member_name, ERR_PARTY_CANNOT_FIND);
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

    ObjectGuid inviter_guid = player->GetGUID();

    if (player->InGroup())
    {
        SendPartyCommandResult(_player, player->GetGroup()->GetGroupType(), member_name, ERR_PARTY_ALREADY_IN_GROUP);
        WorldPacket data(SMSG_GROUP_INVITE, 45);
        data.writeBit(0);

        data.writeBit(inviter_guid[0]);
        data.writeBit(inviter_guid[3]);
        data.writeBit(inviter_guid[2]);

        data.writeBit(0);                   //not in group

        data.writeBit(inviter_guid[6]);
        data.writeBit(inviter_guid[5]);

        data.writeBits(0, 9);

        data.writeBit(inviter_guid[4]);

        data.writeBits(strlen(GetPlayer()->GetName()), 7);

        data.writeBits(0, 24);
        data.writeBit(0);

        data.writeBit(inviter_guid[1]);
        data.writeBit(inviter_guid[7]);

        data.flushBits();

        data.WriteByteSeq(inviter_guid[1]);
        data.WriteByteSeq(inviter_guid[4]);

        data << int32_t(Util::getMSTime());
        data << int32_t(0);
        data << int32_t(0);

        data.WriteByteSeq(inviter_guid[6]);
        data.WriteByteSeq(inviter_guid[0]);
        data.WriteByteSeq(inviter_guid[2]);
        data.WriteByteSeq(inviter_guid[3]);
        data.WriteByteSeq(inviter_guid[5]);
        data.WriteByteSeq(inviter_guid[7]);

        data.WriteString(GetPlayer()->GetName());

        data << int32_t(0);

        player->GetSession()->SendPacket(&data);
        return;
    }

    if (player->GetTeam() != _player->GetTeam() && _player->GetSession()->GetPermissionCount() == 0 && !sWorld.settings.player.isInterfactionGroupEnabled)
    {
        SendPartyCommandResult(_player, 0, member_name, ERR_PARTY_WRONG_FACTION);
        return;
    }

    if (player->HasBeenInvited())
    {
        SendPartyCommandResult(_player, 0, member_name, ERR_PARTY_ALREADY_IN_GROUP);
        return;
    }

    if (player->Social_IsIgnoring(_player->GetLowGUID()))
    {
        SendPartyCommandResult(_player, 0, member_name, ERR_PARTY_IS_IGNORING_YOU);
        return;
    }

    if (player->isGMFlagSet() && !_player->GetSession()->HasPermissions())
    {
        SendPartyCommandResult(_player, 0, member_name, ERR_PARTY_CANNOT_FIND);
        return;
    }

    WorldPacket data(SMSG_GROUP_INVITE, 45);
    data.writeBit(0);

    data.writeBit(inviter_guid[0]);
    data.writeBit(inviter_guid[3]);
    data.writeBit(inviter_guid[2]);

    data.writeBit(1);                   //not in group

    data.writeBit(inviter_guid[6]);
    data.writeBit(inviter_guid[5]);

    data.writeBits(0, 9);

    data.writeBit(inviter_guid[4]);

    data.writeBits(strlen(GetPlayer()->GetName()), 7);
    data.writeBits(0, 24);
    data.writeBit(0);

    data.writeBit(inviter_guid[1]);
    data.writeBit(inviter_guid[7]);

    data.flushBits();

    data.WriteByteSeq(inviter_guid[1]);
    data.WriteByteSeq(inviter_guid[4]);

    data << int32_t(Util::getMSTime());
    data << int32_t(0);
    data << int32_t(0);

    data.WriteByteSeq(inviter_guid[6]);
    data.WriteByteSeq(inviter_guid[0]);
    data.WriteByteSeq(inviter_guid[2]);
    data.WriteByteSeq(inviter_guid[3]);
    data.WriteByteSeq(inviter_guid[5]);
    data.WriteByteSeq(inviter_guid[7]);

    data.WriteString(GetPlayer()->GetName());

    data << int32_t(0);

    player->GetSession()->SendPacket(&data);

    SendPartyCommandResult(_player, 0, member_name, ERR_PARTY_NO_ERROR);

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
        if (group_inviter == nullptr)
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
    uint32_t new_role;

    recvData >> new_role;

    ObjectGuid target_guid; // Target GUID
    ObjectGuid player_guid = GetPlayer()->GetGUID();

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

    data << uint32_t(new_role);        // role

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

void WorldSession::HandleGroupDisbandOpcode(WorldPacket& /*recvData*/)
{
    Group* group = _player->GetGroup();
    if (group == nullptr)
        return;

    if (group->GetGroupType() & GROUP_TYPE_BG)
        return;

    group->RemovePlayer(_player->m_playerInfo);
}

void WorldSession::HandleLootMethodOpcode(WorldPacket& recvData)
{
    uint32_t loot_method;
    uint64_t loot_master;
    uint32_t loot_threshold;

    recvData >> loot_method;
    recvData >> loot_master;
    recvData >> loot_threshold;

    Group* target_group = _player->GetGroup();
    if (target_group == nullptr)
        return;

    if (loot_method > PARTY_LOOT_NBG)
        return;

    if (loot_threshold < ITEM_QUALITY_UNCOMMON_GREEN || loot_threshold > ITEM_QUALITY_ARTIFACT_LIGHT_YELLOW)
        return;

    if (loot_method == PARTY_LOOT_MASTER && !target_group->HasMember(objmgr.GetPlayer((uint32_t)loot_master)))
        return;

    if (target_group->GetLeader() != _player->getPlayerInfo())
    {
        SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        return;
    }

    Player* player = objmgr.GetPlayer((uint32_t)loot_master);
    if (player != nullptr)
        target_group->SetLooter(player, static_cast<uint8_t>(loot_method), static_cast<uint16_t>(loot_threshold));
    else
        target_group->SetLooter(_player, static_cast<uint8_t>(loot_method), static_cast<uint16_t>(loot_threshold));
}

void WorldSession::HandleConvertGroupToRaidOpcode(WorldPacket& recv_data)
{
    bool convert_to_raid = false;
    recv_data >> convert_to_raid;

    Group* group = _player->GetGroup();
    if (group == nullptr)
        return;

    if (group->GetLeader() != _player->getPlayerInfo())
    {
        SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        return;
    }

    //\todo convert back to party
    /*if (convert_to_raid)*/
    group->ExpandToRaid();
    /*else
    group->ReduceToParty();*/

    SendPartyCommandResult(_player, 0, "", ERR_PARTY_NO_ERROR);
}

void WorldSession::HandleGroupRequestJoinUpdatesOpcode(WorldPacket& /*recvData*/)
{
    Group* group = _player->GetGroup();
    if (group != nullptr)
    {
        WorldPacket data(SMSG_REAL_GROUP_UPDATE, 13);
        data << uint8_t(group->GetGroupType());
        data << uint32_t(group->GetMembersCount());
        data << uint64_t(0);  // unk
        SendPacket(&data);
    }
}

void WorldSession::HandleGroupRoleCheckBeginOpcode(WorldPacket& recv_data)
{
    Group* group = GetPlayer()->GetGroup();
    if (!group)
        return;

    if (recv_data.isEmpty())
    {
        if (group->GetLeader()->guid != GetPlayer()->GetGUID() && group->GetMainAssist()->guid != GetPlayer()->GetGUID())
            return;

        ObjectGuid guid = GetPlayer()->GetGUID();

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

void WorldSession::HandleGroupSetLeaderOpcode(WorldPacket& recv_data)
{
    uint64_t memberGuid;
    recv_data >> memberGuid;

    Player* player = objmgr.GetPlayer((uint32)memberGuid);

    if (player == nullptr)
    {
        SendPartyCommandResult(_player, 0, _player->GetName(), ERR_PARTY_CANNOT_FIND);
        return;
    }

    if (!_player->IsGroupLeader())
    {
        SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
        return;
    }

    if (player->GetGroup() != _player->GetGroup())
    {
        SendPartyCommandResult(_player, 0, _player->GetName(), ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
        return;
    }

    Group* pGroup = _player->GetGroup();
    if (pGroup)
    {
        pGroup->SetLeader(player, false);
    }
}

//\TODO handle reason - send it to player.
void WorldSession::HandleGroupUninviteGuidOpcode(WorldPacket& recv_data)
{
    uint64_t memberGuid;
    std::string reason;

    recv_data >> memberGuid;
    recv_data >> reason;

    Player* player = objmgr.GetPlayer(Arcemu::Util::GUID_LOPART(memberGuid));
    PlayerInfo* info = objmgr.GetPlayerInfo(Arcemu::Util::GUID_LOPART(memberGuid));
    if (player == nullptr && info == nullptr)
    {
        SendPartyCommandResult(_player, 0, "", ERR_PARTY_CANNOT_FIND);
        return;
    }

    std::string membername = "unknown";
    membername = player ? player->GetName() : info->name;

    if (!_player->InGroup() || (info != nullptr && info->m_Group != _player->GetGroup()))
    {
        SendPartyCommandResult(_player, 0, membername, ERR_PARTY_IS_NOT_IN_YOUR_PARTY);
        return;
    }

    if (!_player->IsGroupLeader())
    {
        if (player == nullptr)
        {
            SendPartyCommandResult(_player, 0, membername, ERR_PARTY_CANNOT_FIND);
            return;
        }
        else if (_player != player)
        {
            SendPartyCommandResult(_player, 0, "", ERR_PARTY_YOU_ARE_NOT_LEADER);
            return;
        }
    }

    Group* group = _player->GetGroup();
    if (group != nullptr)
    {
        group->RemovePlayer(info);
    }
}
