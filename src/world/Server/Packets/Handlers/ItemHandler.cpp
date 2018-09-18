/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgSwapItem.h"

using namespace AscEmu::Packets;

void WorldSession::handleSwapItemOpcode(WorldPacket& recvPacket)
{
    CmsgSwapItem recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LogDetail("CMSG_SWAP_ITEM: destInventorySlot %i destSlot %i srcInventorySlot %i srcInventorySlot %i", recv_packet.destInventorySlot, recv_packet.destSlot, recv_packet.srcInventorySlot, recv_packet.srcSlot);

    _player->GetItemInterface()->SwapItems(recv_packet.destInventorySlot, recv_packet.destSlot, recv_packet.srcInventorySlot, recv_packet.srcSlot);
}
