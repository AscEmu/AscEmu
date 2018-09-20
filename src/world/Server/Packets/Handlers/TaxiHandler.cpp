/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/TaxiMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/Packets/CmsgTaxiQueryAvailableNodes.h"
#include "Server/Packets/CmsgEnabletaxi.h"

using namespace AscEmu::Packets;

void WorldSession::handleTaxiQueryAvaibleNodesOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgTaxiQueryAvailableNodes recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "WORLD: Received CMSG_TAXIQUERYAVAILABLENODES");

    if (const auto creature = _player->GetMapMgr()->GetCreature(recv_packet.creatureGuid.getGuidLowPart()))
        SendTaxiList(creature);
}

void WorldSession::handleEnabletaxiOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgEnabletaxi recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "WORLD: Received CMSG_ENABLETAXI");

    if (const auto creature = _player->GetMapMgr()->GetCreature(recv_packet.creatureGuid.getGuidLowPart()))
        SendTaxiList(creature);
}
