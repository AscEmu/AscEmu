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
 */

#include "StdAfx.h"
#include "Management/Channel.h"
#include "Management/ChannelMgr.h"
#include "Server/WorldSession.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Objects/ObjectMgr.h"
#include "Server/Packets/CmsgJoinChannel.h"
#include "Server/Packets/SmsgChannelMemberCount.h"

initialiseSingleton(ChannelMgr);

//MIT
using namespace AscEmu::Packets;

#if VERSION_STRING != Cata
void WorldSession::handleChannelJoin(WorldPacket& recvPacket)
{
    CmsgJoinChannel recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    if (!recv_packet.channel_name.compare(worldConfig.getGmClientChannelName()) && !GetPermissionCount())
        return;

    auto channel = channelmgr.GetCreateChannel(recv_packet.channel_name.c_str(), _player, recv_packet.dbc_id);
    if (channel == nullptr)
        return;

    channel->AttemptJoin(_player, recv_packet.password.c_str());
    LogDebugFlag(LF_OPCODE, "CMSG_JOIN_CHANNEL %s", recv_packet.channel_name.c_str());
}

void WorldSession::handleGetChannelMemberCount(WorldPacket& recvPacket)
{
    std::string name;
    recvPacket >> name;

    auto channel = channelmgr.GetChannel(name.c_str(), _player);
    if (channel)
        SendPacket(SmgsChannelMemberCount(name, channel->m_flags, uint32_t(channel->GetNumMembers())).serialise().get());
}
#endif
//MIT end


void WorldSession::HandleChannelLeave(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname;
    uint32 code = 0;
    Channel* chn;

    recvPacket >> code;
    recvPacket >> channelname;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    if (chn == NULL)
        return;

    chn->Part(_player);
}

void WorldSession::HandleChannelList(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname;
    Channel* chn;

    recvPacket >> channelname;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    if (chn != NULL)
        chn->List(_player);
}

void WorldSession::HandleChannelPassword(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, pass;
    Channel* chn;

    recvPacket >> channelname;
    recvPacket >> pass;
    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    if (chn)
        chn->Password(_player, pass.c_str());
}

void WorldSession::HandleChannelSetOwner(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;
    Channel* chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->SetOwner(_player, plr);
}

void WorldSession::HandleChannelOwner(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, pass;
    Channel* chn;

    recvPacket >> channelname;
    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    if (chn)
        chn->GetOwner(_player);
}

void WorldSession::HandleChannelModerator(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;
    Channel* chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->GiveModerator(_player, plr);
}

void WorldSession::HandleChannelUnmoderator(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;
    Channel* chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->TakeModerator(_player, plr);
}

void WorldSession::HandleChannelMute(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;
    Channel* chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->Mute(_player, plr);
}

void WorldSession::HandleChannelUnmute(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;
    Channel* chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->Unmute(_player, plr);
}

void WorldSession::HandleChannelInvite(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;
    Channel* chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->Invite(_player, plr);
}
void WorldSession::HandleChannelKick(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;
    Channel* chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->Kick(_player, plr, false);
}

void WorldSession::HandleChannelBan(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;
    Channel* chn;
    Player* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayer(newp.c_str(), false);
    if (chn && plr)
        chn->Kick(_player, plr, true);
}

void WorldSession::HandleChannelUnban(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname, newp;
    Channel* chn;
    PlayerInfo* plr;

    recvPacket >> channelname;
    recvPacket >> newp;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    plr = objmgr.GetPlayerInfoByName(newp.c_str());
    if (chn && plr)
        chn->Unban(_player, plr);
}

void WorldSession::HandleChannelAnnounce(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname;
    Channel* chn;
    recvPacket >> channelname;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    if (chn)
        chn->Announce(_player);
}

void WorldSession::HandleChannelModerate(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recvPacket, 1);
    std::string channelname;
    Channel* chn;
    recvPacket >> channelname;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    if (chn)
        chn->Moderate(_player);
}

void WorldSession::HandleChannelRosterQuery(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    std::string channelname;
    Channel* chn;
    recvPacket >> channelname;

    chn = channelmgr.GetChannel(channelname.c_str(), _player);
    if (chn)
        chn->List(_player);
}
