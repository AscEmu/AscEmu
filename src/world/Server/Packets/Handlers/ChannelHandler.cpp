/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Server/Packets/CmsgJoinChannel.h"
#include "Server/Packets/SmsgChannelMemberCount.h"
#include "Server/Packets/CmsgChannelDisplayList.h"
#include "Server/Packets/CmsgChannelModerate.h"
#include "Server/Packets/CmsgChannelAnnouncements.h"
#include "Server/Packets/CmsgChannelUnban.h"
#include "Server/Packets/CmsgChannelBan.h"
#include "Server/Packets/CmsgChannelKick.h"
#include "Server/Packets/CmsgChannelInvite.h"
#include "Server/Packets/CmsgChannelUnmute.h"
#include "Server/Packets/CmsgChannelMute.h"
#include "Server/Packets/CmsgChannelUnmoderator.h"
#include "Server/Packets/CmsgChannelModerator.h"
#include "Server/Packets/CmsgChannelOwner.h"
#include "Server/Packets/CmsgChannelSetOwner.h"
#include "Server/Packets/CmsgChannelPassword.h"
#include "Server/Packets/CmsgChannelList.h"
#include "Server/Packets/CmsgLeaveChannel.h"
#include "Server/Packets/CmsgGetChannelMemberCount.h"
#include "Server/WorldSession.h"
#include "Server/World.h"
#include "Objects/ObjectMgr.h"

initialiseSingleton(ChannelMgr);

using namespace AscEmu::Packets;

void WorldSession::handleChannelJoin(WorldPacket& recvPacket)
{
    CmsgJoinChannel srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    if (!sWorld.settings.gm.gmClientChannelName.empty() && !stricmp(sWorld.settings.gm.gmClientChannelName.c_str(), srlPacket.channelName.c_str()) && !GetPermissionCount())
        return;

    const auto channel = channelmgr.GetCreateChannel(srlPacket.channelName.c_str(), GetPlayer(), srlPacket.dbcId);
    if (channel == nullptr)
        return;

    channel->AttemptJoin(GetPlayer(), srlPacket.password.c_str());
    LogDebugFlag(LF_OPCODE, "ChannelJoin %s", srlPacket.channelName.c_str());
}

void WorldSession::handleGetChannelMemberCount(WorldPacket& recvPacket)
{
    CmsgGetChannelMemberCount recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    if (channel)
        SendPacket(SmgsChannelMemberCount(recv_packet.name, channel->m_flags, uint32_t(channel->GetNumMembers())).serialise().get());
}

void WorldSession::handleChannelLeave(WorldPacket& recvPacket)
{
    CmsgLeaveChannel recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    if (channel)
        channel->Part(GetPlayer());
}

void WorldSession::handleChannelList(WorldPacket& recvPacket)
{
    CmsgChannelList recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    if (channel)
        channel->List(GetPlayer());
}

void WorldSession::handleChannelPassword(WorldPacket& recvPacket)
{
    CmsgChannelPassword recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    if (channel)
        channel->Password(GetPlayer(), recv_packet.password.c_str());
}

void WorldSession::handleChannelSetOwner(WorldPacket& recvPacket)
{
    CmsgChannelSetOwner recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    const auto player = objmgr.GetPlayer(recv_packet.setName.c_str(), false);
    if (channel && player)
        channel->SetOwner(GetPlayer(), player);
}

void WorldSession::handleChannelOwner(WorldPacket& recvPacket)
{
    CmsgChannelOwner recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    if (channel)
        channel->GetOwner(GetPlayer());
}

void WorldSession::handleChannelModerator(WorldPacket& recvPacket)
{
    CmsgChannelModerator recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    const auto player = objmgr.GetPlayer(recv_packet.modName.c_str(), false);
    if (channel && player)
        channel->GiveModerator(GetPlayer(), player);
}

void WorldSession::handleChannelUnmoderator(WorldPacket& recvPacket)
{
    CmsgChannelUnmoderator recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    const auto player = objmgr.GetPlayer(recv_packet.unmodName.c_str(), false);
    if (channel && player)
        channel->TakeModerator(GetPlayer(), player);
}

void WorldSession::handleChannelMute(WorldPacket& recvPacket)
{
    CmsgChannelMute recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    const auto player = objmgr.GetPlayer(recv_packet.muteName.c_str(), false);
    if (channel && player)
        channel->Mute(GetPlayer(), player);
}

void WorldSession::handleChannelUnmute(WorldPacket& recvPacket)
{
    CmsgChannelUnmute recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    const auto player = objmgr.GetPlayer(recv_packet.unmuteName.c_str(), false);
    if (channel && player)
        channel->Unmute(GetPlayer(), player);
}

void WorldSession::handleChannelInvite(WorldPacket& recvPacket)
{
    CmsgChannelInvite recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    const auto player = objmgr.GetPlayer(recv_packet.inviteName.c_str(), false);
    if (channel && player)
        channel->Invite(GetPlayer(), player);
}

void WorldSession::handleChannelKick(WorldPacket& recvPacket)
{
    CmsgChannelKick recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    const auto player = objmgr.GetPlayer(recv_packet.kickName.c_str(), false);
    if (channel && player)
        channel->Kick(GetPlayer(), player, false);
}

void WorldSession::handleChannelBan(WorldPacket& recvPacket)
{
    CmsgChannelBan recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    const auto player = objmgr.GetPlayer(recv_packet.banName.c_str(), false);
    if (channel && player)
        channel->Kick(GetPlayer(), player, true);
}

void WorldSession::handleChannelUnban(WorldPacket& recvPacket)
{
    CmsgChannelUnban recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    const auto playerInfo = objmgr.GetPlayerInfoByName(recv_packet.unbanName.c_str());
    if (channel && playerInfo)
        channel->Unban(GetPlayer(), playerInfo);
}

void WorldSession::handleChannelAnnounce(WorldPacket& recvPacket)
{
    CmsgChannelAnnouncements recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    if (channel)
        channel->Announce(GetPlayer());
}

void WorldSession::handleChannelModerate(WorldPacket& recvPacket)
{
    CmsgChannelModerate recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    if (channel)
        channel->Moderate(GetPlayer());
}

void WorldSession::handleChannelRosterQuery(WorldPacket& recvPacket)
{
    CmsgChannelDisplayList recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(recv_packet.name.c_str(), GetPlayer());
    if (channel)
        channel->List(GetPlayer());
}
