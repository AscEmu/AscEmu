/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgGuildQuery.h"
#include "Server/Packets/SmsgGuildCommandResult.h"
#include "Server/Packets/CmsgGuildInvite.h"
#include "Management/GuildMgr.h"
#include "Objects/ObjectMgr.h"
#include "Server/Packets/SmsgGuildInfo.h"
#include "Server/Packets/MsgSaveGuildEmblem.h"

using namespace AscEmu::Packets;

void WorldSession::handleGuildQuery(WorldPacket& recvPacket)
{
    CmsgGuildQuery recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto guild = sGuildMgr.getGuildById(uint32_t(recv_packet.guildId));
    if (guild == nullptr)
        return;

#if VERSION_STRING != Cata
    guild->handleQuery(this);
#else

    if (guild->isMember(recv_packet.playerGuid))
        guild->handleQuery(this);
#endif
}

void WorldSession::handleInviteToGuild(WorldPacket& recvPacket)
{
    CmsgGuildInvite recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    sendGuildInvitePacket(recv_packet.name);
}

#if VERSION_STRING != Cata
void WorldSession::handleGuildInfo(WorldPacket& /*recvPacket*/)
{
    if (const auto guild = GetPlayer()->GetGuild())
        SendPacket(SmsgGuildInfo(guild->getName(), guild->getCreatedDate(), guild->getMembersCount(), guild->getAccountCount()).serialise().get());
}
#endif

void WorldSession::handleSaveGuildEmblem(WorldPacket& recvPacket)
{
    MsgSaveGuildEmblem recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "MSG_SAVE_GUILD_EMBLEM %s: vendorGuid: %u style: %u, color: %u, borderStyle: %u, borderColor: %u, backgroundColor: %u",
        _player->getName().c_str(), recv_packet.guid.getGuidLow(), recv_packet.emblemInfo.getStyle(), recv_packet.emblemInfo.getColor(),
        recv_packet.emblemInfo.getBorderStyle(), recv_packet.emblemInfo.getBorderColor(), recv_packet.emblemInfo.getBackgroundColor());

    Guild* guild = _player->GetGuild();
    if (guild == nullptr)
    {
        SendPacket(MsgSaveGuildEmblem(GEM_ERROR_NOGUILD).serialise().get());
        return;
    }

    if (guild->getLeaderGUID() != _player->getGuid())
    {
        SendPacket(MsgSaveGuildEmblem(GEM_ERROR_NOTGUILDMASTER).serialise().get());
        return;
    }

    guild->handleSetEmblem(this, recv_packet.emblemInfo);
}
