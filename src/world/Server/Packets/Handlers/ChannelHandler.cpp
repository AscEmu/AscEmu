/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

    const auto channel = channelmgr.GetCreateChannel(srlPacket.channelName.c_str(), _player, srlPacket.dbcId);
    if (channel == nullptr)
        return;

    channel->AttemptJoin(_player, srlPacket.password.c_str());
    LogDebugFlag(LF_OPCODE, "ChannelJoin %s", srlPacket.channelName.c_str());
}

void WorldSession::handleGetChannelMemberCount(WorldPacket& recvPacket)
{
    CmsgGetChannelMemberCount srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    if (channel)
        SendPacket(SmgsChannelMemberCount(srlPacket.name, channel->m_flags, uint32_t(channel->GetNumMembers())).serialise().get());
}

void WorldSession::handleChannelLeave(WorldPacket& recvPacket)
{
    CmsgLeaveChannel srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    if (channel)
        channel->Part(_player);
}

void WorldSession::handleChannelList(WorldPacket& recvPacket)
{
    CmsgChannelList srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    if (channel)
        channel->List(_player);
}

void WorldSession::handleChannelPassword(WorldPacket& recvPacket)
{
    CmsgChannelPassword srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    if (channel)
        channel->Password(_player, srlPacket.password.c_str());
}

void WorldSession::handleChannelSetOwner(WorldPacket& recvPacket)
{
    CmsgChannelSetOwner srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    const auto player = objmgr.GetPlayer(srlPacket.setName.c_str(), false);
    if (channel && player)
        channel->SetOwner(_player, player);
}

void WorldSession::handleChannelOwner(WorldPacket& recvPacket)
{
    CmsgChannelOwner srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    if (channel)
        channel->GetOwner(_player);
}

void WorldSession::handleChannelModerator(WorldPacket& recvPacket)
{
    CmsgChannelModerator srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    const auto player = objmgr.GetPlayer(srlPacket.modName.c_str(), false);
    if (channel && player)
        channel->GiveModerator(_player, player);
}

void WorldSession::handleChannelUnmoderator(WorldPacket& recvPacket)
{
    CmsgChannelUnmoderator srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    const auto player = objmgr.GetPlayer(srlPacket.unmodName.c_str(), false);
    if (channel && player)
        channel->TakeModerator(_player, player);
}

void WorldSession::handleChannelMute(WorldPacket& recvPacket)
{
    CmsgChannelMute srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    const auto player = objmgr.GetPlayer(srlPacket.muteName.c_str(), false);
    if (channel && player)
        channel->Mute(_player, player);
}

void WorldSession::handleChannelUnmute(WorldPacket& recvPacket)
{
    CmsgChannelUnmute srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    const auto player = objmgr.GetPlayer(srlPacket.unmuteName.c_str(), false);
    if (channel && player)
        channel->Unmute(_player, player);
}

void WorldSession::handleChannelInvite(WorldPacket& recvPacket)
{
    CmsgChannelInvite srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    const auto player = objmgr.GetPlayer(srlPacket.inviteName.c_str(), false);
    if (channel && player)
        channel->Invite(_player, player);
}

void WorldSession::handleChannelKick(WorldPacket& recvPacket)
{
    CmsgChannelKick srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    const auto player = objmgr.GetPlayer(srlPacket.kickName.c_str(), false);
    if (channel && player)
        channel->Kick(_player, player, false);
}

void WorldSession::handleChannelBan(WorldPacket& recvPacket)
{
    CmsgChannelBan srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    const auto player = objmgr.GetPlayer(srlPacket.banName.c_str(), false);
    if (channel && player)
        channel->Kick(_player, player, true);
}

void WorldSession::handleChannelUnban(WorldPacket& recvPacket)
{
    CmsgChannelUnban srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    const auto playerInfo = objmgr.GetPlayerInfoByName(srlPacket.unbanName.c_str());
    if (channel && playerInfo)
        channel->Unban(_player, playerInfo);
}

void WorldSession::handleChannelAnnounce(WorldPacket& recvPacket)
{
    CmsgChannelAnnouncements srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    if (channel)
        channel->Announce(_player);
}

void WorldSession::handleChannelModerate(WorldPacket& recvPacket)
{
    CmsgChannelModerate srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    if (channel)
        channel->Moderate(_player);
}

void WorldSession::handleChannelRosterQuery(WorldPacket& recvPacket)
{
    CmsgChannelDisplayList srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto channel = channelmgr.GetChannel(srlPacket.name.c_str(), _player);
    if (channel)
        channel->List(_player);
}
