/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/TaxiMgr.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/Packets/CmsgTaxiQueryAvailableNodes.h"
#include "Server/Packets/CmsgEnabletaxi.h"
#include "Server/Packets/SmsgTaxinodeStatus.h"
#include "Server/Packets/CmsgTaxinodeStatusQuery.h"

using namespace AscEmu::Packets;

uint8_t isTaximaskKnown(Player* player, uint32_t nearestNode)
{
    const auto field = static_cast<uint8_t>((nearestNode - 1) / 32);
    const uint32_t submask = 1 << ((nearestNode - 1) % 32);

    if ((player->GetTaximask(field) & submask) != submask)
        return 0;

    return 1;
}

void WorldSession::handleTaxiNodeStatusQueryOpcode(WorldPacket& recvPacket)
{
    CHECK_INWORLD_RETURN

    CmsgTaxinodeStatusQuery recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "WORLD: Received CMSG_TAXINODE_STATUS_QUERY");

    const auto nearestNode = sTaxiMgr.getNearestNodeForPlayer(_player);
    if (!nearestNode)
        return;

    uint8_t status = isTaximaskKnown(_player, nearestNode);
    SendPacket(SmsgTaxinodeStatus(recv_packet.guid, status).serialise().get());
}

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
