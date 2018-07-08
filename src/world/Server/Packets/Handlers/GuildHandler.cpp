/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgGuildQuery.h"
#include "Server/Packets/SmsgGuildCommandResult.h"
#include "Server/Packets/CmsgGuildInvite.h"

using namespace AscEmu::Packets;

void WorldSession::handleGuildQuery(WorldPacket& recvPacket)
{
    CmsgGuildQuery recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

#if VERSION_STRING != Cata
    const auto guild = objmgr.GetGuild(recv_packet.guildId);
    if (guild == nullptr)
        return;

    guild->SendGuildQuery(this);
#else
    const auto guild = sGuildMgr.getGuildById(uint32_t(recv_packet.guildId));
    if (guild == nullptr)
        return;

    if (guild->isMember(recv_packet.playerGuid))
        guild->handleQuery(this);
#endif
}

void WorldSession::HandleInviteToGuild(WorldPacket& recv_data)
{
    CmsgGuildInvite recv_packet;
    if (!recv_packet.deserialise(recv_data))
        return;

    sendGuildInvitePacket(recv_packet.name);
}
