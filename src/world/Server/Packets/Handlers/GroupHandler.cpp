/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgGroupInvite.h"
#include "Server/Packets/SmsgGroupInvite.h"
#include "Server/Packets/SmsgPartyCommandResult.h"
#include "Server/Packets/SmsgGroupDecline.h"
#include "Server/Packets/CmsgGroupUninvite.h"
#include "Server/Packets/CmsgGroupUninviteGuid.h"
#include "Server/Packets/MsgMinimapPing.h"
#include "Server/Packets/CmsgGroupSetLeader.h"
#include "Server/Packets/CmsgLootMethod.h"
#include "Server/Packets/MsgRaidTargetUpdate.h"
#include "Server/Packets/CmsgRequestPartyMemberStats.h"
#include "Server/Packets/SmsgPartyMemberStatsFull.h"
#include "Server/WorldSession.h"
#include "Objects/ObjectMgr.h"
#include "Map/MapMgr.h"
#include "Server/Packets/CmsgGroupChangeSubGroup.h"
#include "Server/Packets/CmsgGroupAssistantLeader.h"
#include "Server/Packets/MsgPartyAssign.h"
#include "Server/Packets/MsgRaidReadyCheck.h"

using namespace AscEmu::Packets;

#if VERSION_STRING >= Cata
void WorldSession::sendEmptyGroupList(Player* player)
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

void WorldSession::handleGroupInviteResponseOpcode(WorldPacket& recvPacket)
{
    recvPacket.readBit();                    //unk
    bool acceptInvite = recvPacket.readBit();

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
            _player->sendDungeonDifficultyPacket();
            return;
        }
        else
        {
            group = new Group(true);
            group->m_difficulty = group_inviter->iInstanceType;
            group->AddMember(group_inviter->m_playerInfo);
            group->AddMember(_player->m_playerInfo);
            _player->iInstanceType = group->m_difficulty;
            _player->sendDungeonDifficultyPacket();

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

        WorldPacket data(SMSG_GROUP_DECLINE, strlen(_player->getName().c_str()));
        data << _player->getName().c_str();
        group_inviter->GetSession()->SendPacket(&data);
    }
}

void WorldSession::handleGroupSetRolesOpcode(WorldPacket& recvPacket)
{
    uint32_t newRole;

    recvPacket >> newRole;

    ObjectGuid target_guid; // Target GUID
    ObjectGuid player_guid = _player->getGuid();

    target_guid[2] = recvPacket.readBit();
    target_guid[6] = recvPacket.readBit();
    target_guid[3] = recvPacket.readBit();
    target_guid[7] = recvPacket.readBit();
    target_guid[5] = recvPacket.readBit();
    target_guid[1] = recvPacket.readBit();
    target_guid[0] = recvPacket.readBit();
    target_guid[4] = recvPacket.readBit();

    recvPacket.ReadByteSeq(target_guid[6]);
    recvPacket.ReadByteSeq(target_guid[4]);
    recvPacket.ReadByteSeq(target_guid[1]);
    recvPacket.ReadByteSeq(target_guid[3]);
    recvPacket.ReadByteSeq(target_guid[0]);
    recvPacket.ReadByteSeq(target_guid[5]);
    recvPacket.ReadByteSeq(target_guid[2]);
    recvPacket.ReadByteSeq(target_guid[7]);

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

    if (_player->GetGroup())
        _player->GetGroup()->SendPacketToAll(&data);
    else
        SendPacket(&data);
}

void WorldSession::handleGroupRequestJoinUpdatesOpcode(WorldPacket& /*recvPacket*/)
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

void WorldSession::handleGroupRoleCheckBeginOpcode(WorldPacket& recvPacket)
{
    Group* group = _player->GetGroup();
    if (!group)
        return;

    if (recvPacket.isEmpty())
    {
        if (group->GetLeader()->guid != _player->getGuid() && group->GetMainAssist()->guid != _player->getGuid())
            return;

        ObjectGuid guid = _player->getGuid();

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
#endif

#if VERSION_STRING >= Cata
void WorldSession::handleGroupInviteOpcode(WorldPacket& recvPacket)
{
    ObjectGuid unk_guid;

    recvPacket.read_skip<uint32_t>();
    recvPacket.read_skip<uint32_t>();

    unk_guid[2] = recvPacket.readBit();
    unk_guid[7] = recvPacket.readBit();

    uint8_t realm_name_length = static_cast<uint8_t>(recvPacket.readBits(9));

    unk_guid[3] = recvPacket.readBit();

    uint8_t member_name_length = static_cast<uint8_t>(recvPacket.readBits(10));

    unk_guid[5] = recvPacket.readBit();
    unk_guid[4] = recvPacket.readBit();
    unk_guid[6] = recvPacket.readBit();
    unk_guid[0] = recvPacket.readBit();
    unk_guid[1] = recvPacket.readBit();

    recvPacket.ReadByteSeq(unk_guid[4]);
    recvPacket.ReadByteSeq(unk_guid[7]);
    recvPacket.ReadByteSeq(unk_guid[6]);

    std::string member_name = recvPacket.ReadString(member_name_length);
    std::string realm_name = recvPacket.ReadString(realm_name_length);

    recvPacket.ReadByteSeq(unk_guid[1]);
    recvPacket.ReadByteSeq(unk_guid[0]);
    recvPacket.ReadByteSeq(unk_guid[5]);
    recvPacket.ReadByteSeq(unk_guid[3]);
    recvPacket.ReadByteSeq(unk_guid[2]);

    if (_player->HasBeenInvited())
        return;

    Player* player = objmgr.GetPlayer(member_name.c_str(), false);
    if (player == nullptr)
    {
        SendPacket(SmsgPartyCommandResult(0, member_name, ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    if (_player == player)
        return;

    if (_player->InGroup() && !_player->IsGroupLeader())
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    Group* group = _player->GetGroup();
    if (group != nullptr)
    {
        if (group->IsFull())
        {
            SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_IS_FULL).serialise().get());
            return;
        }
    }

    ObjectGuid inviter_guid = player->getGuid();

    if (player->InGroup())
    {
        SendPacket(SmsgPartyCommandResult(player->GetGroup()->getGroupType(), member_name, ERR_PARTY_ALREADY_IN_GROUP).serialise().get());
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

        data.writeBits(strlen(_player->getName().c_str()), 7);

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

        data.WriteString(_player->getName().c_str());

        data << int32_t(0);

        player->GetSession()->SendPacket(&data);
        return;
    }

    if (player->getTeam() != _player->getTeam() && _player->GetSession()->GetPermissionCount() == 0 && !sWorld.settings.player.isInterfactionGroupEnabled)
    {
        SendPacket(SmsgPartyCommandResult(0, member_name, ERR_PARTY_WRONG_FACTION).serialise().get());
        return;
    }

    if (player->HasBeenInvited())
    {
        SendPacket(SmsgPartyCommandResult(0, member_name, ERR_PARTY_ALREADY_IN_GROUP).serialise().get());
        return;
    }

    if (player->Social_IsIgnoring(_player->getGuidLow()))
    {
        SendPacket(SmsgPartyCommandResult(0, member_name, ERR_PARTY_IS_IGNORING_YOU).serialise().get());
        return;
    }

    if (player->isGMFlagSet() && !_player->GetSession()->HasPermissions())
    {
        SendPacket(SmsgPartyCommandResult(0, member_name, ERR_PARTY_CANNOT_FIND).serialise().get());
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

    data.writeBits(strlen(_player->getName().c_str()), 7);
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

    data.WriteString(_player->getName().c_str());

    data << int32_t(0);

    player->GetSession()->SendPacket(&data);

    SendPacket(SmsgPartyCommandResult(0, member_name, ERR_PARTY_NO_ERROR).serialise().get());

    player->SetInviter(_player->getGuidLow());
}
#else
void WorldSession::handleGroupInviteOpcode(WorldPacket& recvPacket)
{
    CmsgGroupInvite srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    auto invitedPlayer = objmgr.GetPlayer(srlPacket.name.c_str(), false);
    if (invitedPlayer == nullptr)
    {
        SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    if (invitedPlayer == _player || _player->HasBeenInvited())
        return;

    if (_player->InGroup() && !_player->IsGroupLeader())
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    if (_player->GetGroup() != nullptr)
    {
        if (_player->GetGroup()->IsFull())
        {
            SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_IS_FULL).serialise().get());
            return;
        }
    }

    if (invitedPlayer->InGroup())
    {
        SendPacket(SmsgPartyCommandResult(invitedPlayer->GetGroup()->getGroupType(), srlPacket.name, ERR_PARTY_ALREADY_IN_GROUP).serialise().get());
        invitedPlayer->GetSession()->SendPacket(SmsgGroupInvite(0, _player->getName().c_str()).serialise().get());
        return;
    }

    if (invitedPlayer->getTeam() != _player->getTeam() && _player->GetSession()->GetPermissionCount() == 0 && !worldConfig.player.isInterfactionGroupEnabled)
    {
        SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_WRONG_FACTION).serialise().get());
        return;
    }

    if (invitedPlayer->HasBeenInvited())
    {
        SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_ALREADY_IN_GROUP).serialise().get());
        return;
    }

    if (invitedPlayer->Social_IsIgnoring(_player->getGuidLow()))
    {
        SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_IS_IGNORING_YOU).serialise().get());
        return;
    }

    if (invitedPlayer->isGMFlagSet() && !_player->GetSession()->HasPermissions())
    {
        SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    invitedPlayer->GetSession()->SendPacket(SmsgGroupInvite(1, _player->getName().c_str()).serialise().get());

    SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_NO_ERROR).serialise().get());

    invitedPlayer->SetInviter(_player->getGuidLow());
}
#endif

//\brief Not used for cata - the client sends a response
//       Check out handleGroupInviteResponseOpcode!
void WorldSession::handleGroupDeclineOpcode(WorldPacket& /*recvPacket*/)
{
    const auto inviter = objmgr.GetPlayer(_player->GetInviter());
    if (inviter == nullptr)
        return;

    inviter->SendPacket(SmsgGroupDecline(_player->getName()).serialise().get());
    inviter->SetInviter(0);
    _player->SetInviter(0);
}

void WorldSession::handleGroupAcceptOpcode(WorldPacket& /*recvPacket*/)
{
    if (_player->GetGroup())
        return;

    const auto player = objmgr.GetPlayer(_player->GetInviter());
    if (player == nullptr)
        return;

    _player->SetInviter(0);
    player->SetInviter(0);

    auto group = player->GetGroup();
    if (group == nullptr)
    {
        group = new Group(true);
        group->AddMember(player->getPlayerInfo());
        group->AddMember(_player->getPlayerInfo());
        group->m_difficulty = player->iInstanceType;
        _player->iInstanceType = player->iInstanceType;
        _player->sendDungeonDifficultyPacket();

        const auto instance = sInstanceMgr.GetInstanceByIds(player->GetMapId(), player->GetInstanceID());
        if (instance && instance->m_creatorGuid == player->getGuidLow())
        {
            group->m_instanceIds[instance->m_mapId][instance->m_difficulty] = instance->m_instanceId;
            instance->m_creatorGroup = group->GetID();
            instance->m_creatorGuid = 0;
            instance->SaveToDB();
        }
    }
    else
    {
        group->AddMember(_player->getPlayerInfo());
        _player->iInstanceType = group->m_difficulty;
        _player->sendDungeonDifficultyPacket();
    }
}

void WorldSession::handleGroupUninviteOpcode(WorldPacket& recvPacket)
{
    CmsgGroupUninvite srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_GROUP_UNINVITE: %s (name)", srlPacket.name.c_str());

    const auto uninvitePlayer = objmgr.GetPlayer(srlPacket.name.c_str(), false);
    if (uninvitePlayer == nullptr)
    {
        SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    if (!_player->InGroup() || uninvitePlayer->getPlayerInfo()->m_Group != _player->GetGroup())
    {
        SendPacket(SmsgPartyCommandResult(0, srlPacket.name, ERR_PARTY_IS_NOT_IN_YOUR_PARTY).serialise().get());
        return;
    }

    if (!_player->IsGroupLeader())
    {
        if (_player != uninvitePlayer)
        {
            SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
            return;
        }
    }

    const auto group = _player->GetGroup();
    if (group)
        group->RemovePlayer(uninvitePlayer->getPlayerInfo());
}

void WorldSession::handleGroupUninviteGuidOpcode(WorldPacket& recvPacket)
{
    CmsgGroupUninviteGuid srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_GROUP_UNINVITE_GUID: %u (guidLow)", srlPacket.guid.getGuidLow());

    const auto uninvitePlayer = objmgr.GetPlayer(srlPacket.guid.getGuidLow());
    if (uninvitePlayer == nullptr)
    {
        SendPacket(SmsgPartyCommandResult(0, "unknown", ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    const std::string name = uninvitePlayer->getName();

    if (!_player->InGroup() || uninvitePlayer->getPlayerInfo()->m_Group != _player->GetGroup())
    {
        SendPacket(SmsgPartyCommandResult(0, name, ERR_PARTY_IS_NOT_IN_YOUR_PARTY).serialise().get());
        return;
    }

    if (!_player->IsGroupLeader())
    {
        if (_player != uninvitePlayer)
        {
            SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
            return;
        }
    }

    const auto group = _player->GetGroup();
    if (group)
        group->RemovePlayer(uninvitePlayer->getPlayerInfo());
}

void WorldSession::handleGroupDisbandOpcode(WorldPacket& /*recvPacket*/)
{
    const auto group = _player->GetGroup();
    if (group == nullptr)
        return;

    if (group->getGroupType() & GROUP_TYPE_BG)
        return;

    group->RemovePlayer(_player->getPlayerInfo());
}

void WorldSession::handleMinimapPingOpcode(WorldPacket& recvPacket)
{
    MsgMinimapPing srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received MSG_MINIMAP_PING: %f (x), %f (y)", srlPacket.posX, srlPacket.posY);

    if (!_player->InGroup())
        return;

    const auto group = _player->GetGroup();
    if (group == nullptr)
        return;

    group->SendPacketToAllButOne(MsgMinimapPing(_player->getGuid(), srlPacket.posX, srlPacket.posY).serialise().get(), _player);
}

void WorldSession::handleGroupSetLeaderOpcode(WorldPacket& recvPacket)
{
    CmsgGroupSetLeader srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_GROUP_SET_LEADER: %u (guidLow)", srlPacket.guid.getGuidLow());

    const auto targetPlayer = objmgr.GetPlayer(srlPacket.guid.getGuidLow());
    if (targetPlayer == nullptr)
    {
        SendPacket(SmsgPartyCommandResult(0, _player->getName(), ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    if (!_player->IsGroupLeader())
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    if (targetPlayer->GetGroup() != _player->GetGroup())
    {
        SendPacket(SmsgPartyCommandResult(0, _player->getName(), ERR_PARTY_IS_NOT_IN_YOUR_PARTY).serialise().get());
        return;
    }

    const auto group = _player->GetGroup();
    if (group)
        group->SetLeader(targetPlayer, false);
}

void WorldSession::handleLootMethodOpcode(WorldPacket& recvPacket)
{
    CmsgLootMethod srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_LOOT_METHOD: %u (method), %u (guidLow), %u (theshold)", srlPacket.method, srlPacket.guid.getGuidLow(), srlPacket.threshold);

    if (!_player->IsGroupLeader())
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    const auto group = _player->GetGroup();
    if (group == nullptr)
        return;

    const auto lootMasterPlayer = objmgr.GetPlayer(srlPacket.guid.getGuidLow());
    if (lootMasterPlayer == nullptr)
        group->SetLooter(_player, static_cast<uint8_t>(srlPacket.method), static_cast<uint16_t>(srlPacket.threshold));
    else
        group->SetLooter(lootMasterPlayer, static_cast<uint8_t>(srlPacket.method), static_cast<uint16_t>(srlPacket.threshold));

}

void WorldSession::handleSetPlayerIconOpcode(WorldPacket& recvPacket)
{
    MsgRaidTargetUpdate srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received MSG_RAID_TARGET_UPDATE: %u (icon)", srlPacket.icon);

    const auto group = _player->GetGroup();
    if (group == nullptr)
        return;

    if (srlPacket.icon == 0xFF)
    {
        SendPacket(MsgRaidTargetUpdate(1, 0, 0, 0, group).serialise().get());
    }
    else if (_player->IsGroupLeader())
    {
        if (srlPacket.icon >= iconCount)
            return;

        for (uint8_t i = 0; i < iconCount; ++i)
        {
            if (group->m_targetIcons[i] == srlPacket.guid)
            {
                group->m_targetIcons[i] = 0;
                group->SendPacketToAll(MsgRaidTargetUpdate(0, 0, i, 0, nullptr).serialise().get());
            }
        }

        group->SendPacketToAll(MsgRaidTargetUpdate(0, _player->getGuid(), srlPacket.icon, srlPacket.guid, nullptr).serialise().get());
        group->m_targetIcons[srlPacket.icon] = srlPacket.guid;
    }
}

//\brief: Not used on Cata
void WorldSession::handlePartyMemberStatsOpcode(WorldPacket& recvPacket)
{
    CmsgRequestPartyMemberStats srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_REQUEST_PARTY_MEMBER_STATS: %u (guidLow)", srlPacket.guid.getGuidLow());

    if (_player->GetMapMgr() == nullptr)
    {
        LogDebugFlag(LF_OPCODE, "Received CMSG_REQUEST_PARTY_MEMBER_STATS: But MapMgr is not ready!");
        return;
    }

    const auto requestedPlayer = _player->GetMapMgr()->GetPlayer(srlPacket.guid.getGuidLow());
    if (_player->GetGroup() == nullptr || requestedPlayer == nullptr)
    {
        SendPacket(SmsgPartyMemberStatsFull(srlPacket.guid, nullptr).serialise().get());
        return;
    }

    if (!_player->GetGroup()->HasMember(requestedPlayer))
        return;

    if (_player->IsVisible(requestedPlayer->getGuid()))
        return;

    SendPacket(SmsgPartyMemberStatsFull(requestedPlayer->getGuid(), requestedPlayer).serialise().get());
}

void WorldSession::handleConvertGroupToRaidOpcode(WorldPacket& /*recvPacket*/)
{
    auto const group = _player->GetGroup();
    if (group == nullptr)
        return;

    if (group->GetLeader() != _player->m_playerInfo)
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    group->ExpandToRaid();
    SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_NO_ERROR).serialise().get());
}

void WorldSession::handleRequestRaidInfoOpcode(WorldPacket& /*recvPacket*/)
{
    sInstanceMgr.BuildRaidSavedInstancesForPlayer(_player);
}

void WorldSession::handleGroupChangeSubGroup(WorldPacket& recvPacket)
{
    CmsgGroupChangeSubGroup srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto playerInfo = objmgr.GetPlayerInfoByName(srlPacket.name.c_str());
    if (playerInfo == nullptr || playerInfo->m_Group == nullptr)
        return;

    if (playerInfo->m_Group != _player->GetGroup())
        return;

    _player->GetGroup()->MovePlayer(playerInfo, srlPacket.subGroup);
}

void WorldSession::handleGroupAssistantLeader(WorldPacket& recvPacket)
{
    const auto group = _player->GetGroup();
    if (group == nullptr)
        return;

    if (group->GetLeader() != _player->getPlayerInfo())
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    CmsgGroupAssistantLeader srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (srlPacket.isActivated)
    {
        const auto playerInfo = objmgr.GetPlayerInfo(srlPacket.guid.getGuidLow());
        if (playerInfo == nullptr)
        {
            group->SetAssistantLeader(nullptr);
        }
        else
        {
            if (group->HasMember(playerInfo))
                group->SetAssistantLeader(playerInfo);
        }
    }
}

void WorldSession::handleGroupPromote(WorldPacket& recvPacket)
{
    const auto group = _player->GetGroup();
    if (group == nullptr)
        return;

    if (group->GetLeader() != _player->getPlayerInfo())
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    MsgPartyAssign srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    PlayerInfo* playerInfo = nullptr;

    if (srlPacket.isActivated)
        playerInfo = objmgr.GetPlayerInfo(srlPacket.guid.getGuidLow());

    if (srlPacket.promoteType == 1)
        group->SetMainAssist(playerInfo);
    else if (srlPacket.promoteType == 0)
        group->SetMainTank(playerInfo);
}

void WorldSession::handleReadyCheckOpcode(WorldPacket& recvPacket)
{
    const auto group = _player->GetGroup();
    if (group == nullptr)
        return;

    if (recvPacket.isEmpty())
    {
        if (group->GetLeader() == _player->getPlayerInfo() || group->GetAssistantLeader() == _player->getPlayerInfo())
            group->SendPacketToAll(MsgRaidReadyCheck(_player->getGuid(), 0, true).serialise().get());
        else
            SendNotification("You do not have permission to perform that function.");
    }
    else
    {
        MsgRaidReadyCheck srlPacket;
        if (!srlPacket.deserialise(recvPacket))
            return;

        if (group->GetLeader() && group->GetLeader()->m_loggedInPlayer)
            group->GetLeader()->m_loggedInPlayer->SendPacket(MsgRaidReadyCheck(_player->getGuid(), srlPacket.isReady, false).serialise().get());

        if (group->GetAssistantLeader() && group->GetAssistantLeader()->m_loggedInPlayer)
            group->GetAssistantLeader()->m_loggedInPlayer->SendPacket(MsgRaidReadyCheck(_player->getGuid(), srlPacket.isReady, false).serialise().get());
    }
}
