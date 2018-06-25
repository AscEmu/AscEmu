/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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

using namespace AscEmu::Packets;

#if VERSION_STRING == Cata
void WorldSession::handleGroupInviteOpcode(WorldPacket& recvData)
{
    ObjectGuid unk_guid;

    recvData.read_skip<uint32_t>();
    recvData.read_skip<uint32_t>();

    unk_guid[2] = recvData.readBit();
    unk_guid[7] = recvData.readBit();

    uint8_t realm_name_length = static_cast<uint8_t>(recvData.readBits(9));

    unk_guid[3] = recvData.readBit();

    uint8_t member_name_length = static_cast<uint8_t>(recvData.readBits(10));

    unk_guid[5] = recvData.readBit();
    unk_guid[4] = recvData.readBit();
    unk_guid[6] = recvData.readBit();
    unk_guid[0] = recvData.readBit();
    unk_guid[1] = recvData.readBit();

    recvData.ReadByteSeq(unk_guid[4]);
    recvData.ReadByteSeq(unk_guid[7]);
    recvData.ReadByteSeq(unk_guid[6]);

    std::string member_name = recvData.ReadString(member_name_length);
    std::string realm_name = recvData.ReadString(realm_name_length);

    recvData.ReadByteSeq(unk_guid[1]);
    recvData.ReadByteSeq(unk_guid[0]);
    recvData.ReadByteSeq(unk_guid[5]);
    recvData.ReadByteSeq(unk_guid[3]);
    recvData.ReadByteSeq(unk_guid[2]);

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

        data.writeBits(strlen(GetPlayer()->getName().c_str()), 7);

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

        data.WriteString(GetPlayer()->getName().c_str());

        data << int32_t(0);

        player->GetSession()->SendPacket(&data);
        return;
    }

    if (player->GetTeam() != _player->GetTeam() && _player->GetSession()->GetPermissionCount() == 0 && !sWorld.settings.player.isInterfactionGroupEnabled)
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

    data.writeBits(strlen(GetPlayer()->getName().c_str()), 7);
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

    data.WriteString(GetPlayer()->getName().c_str());

    data << int32_t(0);

    player->GetSession()->SendPacket(&data);

    SendPacket(SmsgPartyCommandResult(0, member_name, ERR_PARTY_NO_ERROR).serialise().get());

    player->SetInviter(_player->getGuidLow());
}
#else
void WorldSession::handleGroupInviteOpcode(WorldPacket& recvPacket)
{
    CmsgGroupInvite recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    auto invitedPlayer = objmgr.GetPlayer(recv_packet.name.c_str(), false);
    if (invitedPlayer == nullptr)
    {
        SendPacket(SmsgPartyCommandResult(0, recv_packet.name, ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    if (invitedPlayer == GetPlayer() || GetPlayer()->HasBeenInvited())
        return;

    if (GetPlayer()->InGroup() && !GetPlayer()->IsGroupLeader())
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    if (GetPlayer()->GetGroup() != nullptr)
    {
        if (GetPlayer()->GetGroup()->IsFull())
        {
            SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_IS_FULL).serialise().get());
            return;
        }
    }

    if (invitedPlayer->InGroup())
    {
        SendPacket(SmsgPartyCommandResult(invitedPlayer->GetGroup()->getGroupType(), recv_packet.name, ERR_PARTY_ALREADY_IN_GROUP).serialise().get());
        invitedPlayer->GetSession()->SendPacket(SmsgGroupInvite(0, GetPlayer()->getName().c_str()).serialise().get());
        return;
    }

    if (invitedPlayer->GetTeam() != GetPlayer()->GetTeam() && GetPlayer()->GetSession()->GetPermissionCount() == 0 && !worldConfig.player.isInterfactionGroupEnabled)
    {
        SendPacket(SmsgPartyCommandResult(0, recv_packet.name, ERR_PARTY_WRONG_FACTION).serialise().get());
        return;
    }

    if (invitedPlayer->HasBeenInvited())
    {
        SendPacket(SmsgPartyCommandResult(0, recv_packet.name, ERR_PARTY_ALREADY_IN_GROUP).serialise().get());
        return;
    }

    if (invitedPlayer->Social_IsIgnoring(GetPlayer()->getGuidLow()))
    {
        SendPacket(SmsgPartyCommandResult(0, recv_packet.name, ERR_PARTY_IS_IGNORING_YOU).serialise().get());
        return;
    }

    if (invitedPlayer->isGMFlagSet() && !GetPlayer()->GetSession()->HasPermissions())
    {
        SendPacket(SmsgPartyCommandResult(0, recv_packet.name, ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    invitedPlayer->GetSession()->SendPacket(SmsgGroupInvite(1, GetPlayer()->getName().c_str()).serialise().get());

    SendPacket(SmsgPartyCommandResult(0, recv_packet.name, ERR_PARTY_NO_ERROR).serialise().get());

    invitedPlayer->SetInviter(GetPlayer()->getGuidLow());
}
#endif

//\brief Not used for cata - the client sends a response
//       Check out HandleGroupInviteResponseOpcode!
void WorldSession::handleGroupDeclineOpcode(WorldPacket& /*recvPacket*/)
{
    const auto inviter = objmgr.GetPlayer(GetPlayer()->GetInviter());
    if (inviter == nullptr)
        return;

    inviter->SendPacket(SmsgGroupDecline(GetPlayer()->getName()).serialise().get());
    inviter->SetInviter(0);
    GetPlayer()->SetInviter(0);
}

void WorldSession::handleGroupUninviteOpcode(WorldPacket& recvPacket)
{
    CmsgGroupUninvite recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_GROUP_UNINVITE: %s (name)", recv_packet.name.c_str());

    const auto uninvitePlayer = objmgr.GetPlayer(recv_packet.name.c_str(), false);
    if (uninvitePlayer == nullptr)
    {
        SendPacket(SmsgPartyCommandResult(0, recv_packet.name, ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    if (!GetPlayer()->InGroup() || uninvitePlayer->getPlayerInfo()->m_Group != GetPlayer()->GetGroup())
    {
        SendPacket(SmsgPartyCommandResult(0, recv_packet.name, ERR_PARTY_IS_NOT_IN_YOUR_PARTY).serialise().get());
        return;
    }

    if (!GetPlayer()->IsGroupLeader())
    {
        if (GetPlayer() != uninvitePlayer)
        {
            SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
            return;
        }
    }

    const auto group = GetPlayer()->GetGroup();
    if (group)
        group->RemovePlayer(uninvitePlayer->getPlayerInfo());
}

void WorldSession::handleGroupUninviteGuidOpcode(WorldPacket& recvPacket)
{
    CmsgGroupUninviteGuid recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_GROUP_UNINVITE_GUID: %u (guidLow)", recv_packet.guid.getGuidLow());

    const auto uninvitePlayer = objmgr.GetPlayer(recv_packet.guid.getGuidLow());
    if (uninvitePlayer == nullptr)
    {
        SendPacket(SmsgPartyCommandResult(0, "unknown", ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    const std::string name = uninvitePlayer->getName();

    if (!GetPlayer()->InGroup() || uninvitePlayer->getPlayerInfo()->m_Group != GetPlayer()->GetGroup())
    {
        SendPacket(SmsgPartyCommandResult(0, name, ERR_PARTY_IS_NOT_IN_YOUR_PARTY).serialise().get());
        return;
    }

    if (!GetPlayer()->IsGroupLeader())
    {
        if (GetPlayer() != uninvitePlayer)
        {
            SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
            return;
        }
    }

    const auto group = GetPlayer()->GetGroup();
    if (group)
        group->RemovePlayer(uninvitePlayer->getPlayerInfo());
}

void WorldSession::handleGroupDisbandOpcode(WorldPacket& /*recvPacket*/)
{
    const auto group = GetPlayer()->GetGroup();
    if (group == nullptr)
        return;

    if (group->getGroupType() & GROUP_TYPE_BG)
        return;

    group->RemovePlayer(GetPlayer()->getPlayerInfo());
}

void WorldSession::handleMinimapPingOpcode(WorldPacket& recvPacket)
{
    MsgMinimapPing recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received MSG_MINIMAP_PING: %f (x), %f (y)", recv_packet.posX, recv_packet.posY);

    if (!GetPlayer()->InGroup())
        return;

    const auto group = GetPlayer()->GetGroup();
    if (group == nullptr)
        return;

    group->SendPacketToAllButOne(MsgMinimapPing(GetPlayer()->getGuid(), recv_packet.posX, recv_packet.posY).serialise().get(), GetPlayer());
}

void WorldSession::handleGroupSetLeaderOpcode(WorldPacket& recvPacket)
{
    CmsgGroupSetLeader recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_GROUP_SET_LEADER: %u (guidLow)", recv_packet.guid.getGuidLow());

    const auto targetPlayer = objmgr.GetPlayer(recv_packet.guid.getGuidLow());
    if (targetPlayer == nullptr)
    {
        SendPacket(SmsgPartyCommandResult(0, GetPlayer()->getName(), ERR_PARTY_CANNOT_FIND).serialise().get());
        return;
    }

    if (!GetPlayer()->IsGroupLeader())
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    if (targetPlayer->GetGroup() != GetPlayer()->GetGroup())
    {
        SendPacket(SmsgPartyCommandResult(0, GetPlayer()->getName(), ERR_PARTY_IS_NOT_IN_YOUR_PARTY).serialise().get());
        return;
    }

    const auto group = GetPlayer()->GetGroup();
    if (group)
        group->SetLeader(targetPlayer, false);
}

void WorldSession::handleLootMethodOpcode(WorldPacket& recvPacket)
{
    CmsgLootMethod recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_LOOT_METHOD: %u (method), %u (guidLow), %u (theshold)", recv_packet.method, recv_packet.guid.getGuidLow(), recv_packet.threshold);

    if (!GetPlayer()->IsGroupLeader())
    {
        SendPacket(SmsgPartyCommandResult(0, "", ERR_PARTY_YOU_ARE_NOT_LEADER).serialise().get());
        return;
    }

    const auto group = GetPlayer()->GetGroup();
    if (group == nullptr)
        return;

    const auto lootMasterPlayer = objmgr.GetPlayer(recv_packet.guid.getGuidLow());
    if (lootMasterPlayer == nullptr)
        group->SetLooter(GetPlayer(), static_cast<uint8_t>(recv_packet.method), static_cast<uint16_t>(recv_packet.threshold));
    else
        group->SetLooter(lootMasterPlayer, static_cast<uint8_t>(recv_packet.method), static_cast<uint16_t>(recv_packet.threshold));

}
