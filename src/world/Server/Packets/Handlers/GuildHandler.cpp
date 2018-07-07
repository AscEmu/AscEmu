/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgGuildQuery.h"

using namespace AscEmu::Packets;

#if VERSION_STRING != Cata

void WorldSession::handleGuildQuery(WorldPacket& recvPacket)
{
    CmsgGuildQuery recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto guild = objmgr.GetGuild(recv_packet.guildId);
    if (guild == nullptr)
        return;

    guild->SendGuildQuery(this);
}

#endif
