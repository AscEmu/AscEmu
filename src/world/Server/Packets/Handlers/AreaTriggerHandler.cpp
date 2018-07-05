/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgAreatrigger.h"
#include "Server/WorldSession.h"

using namespace AscEmu::Packets;

void WorldSession::handleAreaTriggerOpcode(WorldPacket& recvPacket)
{
    CmsgAreatrigger recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_AREATRIGGER: %u (triggerId)", recv_packet.triggerId);

    _HandleAreaTriggerOpcode(recv_packet.triggerId);
}
