/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/Packets/CmsgAuctionListOwnerItems.h"
#include "Server/Packets/CmsgAuctionListItems.h"
#include "Server/Packets/CmsgAuctionRemoveItem.h"
#include "Server/Packets/SmsgAuctionCommandResult.h"
#include "Server/Packets/CmsgAuctionListIBidderItemse.h"
#include "Server/Packets/CmsgAuctionListIPendingSales.h"

using namespace AscEmu::Packets;

void WorldSession::handleAuctionListOwnerItems(WorldPacket& recvPacket)
{
    CmsgAuctionListOwnerItems recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_AUCTION_LIST_OWNER_ITEMS %u (guidLow)", recv_packet.guid.getGuidLow());

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLow());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    creature->auctionHouse->SendOwnerListPacket(GetPlayer(), nullptr);
}

void WorldSession::handleAuctionListItems(WorldPacket& recvPacket)
{
    CmsgAuctionListItems recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_AUCTION_LIST_OWNER_ITEMS %u (guidLow)", recv_packet.guid.getGuidLow());

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLow());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    creature->auctionHouse->SendAuctionList(GetPlayer(), &recvPacket);
}

void WorldSession::handleCancelAuction(WorldPacket& recvPacket)
{
    CmsgAuctionRemoveItem recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_AUCTION_REMOVE_ITEM %u (auctionId)", recv_packet.auctionId);

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLow());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    const auto auction = creature->auctionHouse->GetAuction(recv_packet.auctionId);
    if (auction == nullptr)
        return;

    creature->auctionHouse->QueueDeletion(auction, AUCTION_REMOVE_CANCELLED);

    SendPacket(SmsgAuctionCommandResult(recv_packet.auctionId, AUCTION_CANCEL, AUCTION_ERROR_NONE).serialise().get());

    creature->auctionHouse->SendOwnerListPacket(GetPlayer(), nullptr);
}

void WorldSession::handleAuctionListBidderItems(WorldPacket& recvPacket)
{
    CmsgAuctionListIBidderItemse recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_AUCTION_LIST_BIDDER_ITEMS %u (lowguid)", recv_packet.guid.getGuidLow());

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLow());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    creature->auctionHouse->SendBidListPacket(GetPlayer(), &recvPacket);
}

void WorldSession::handleAuctionListPendingSales(WorldPacket& recvPacket)
{
    CmsgAuctionListIPendingSales recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_AUCTION_LIST_PRENDING_SALES %u (lowguid)", recv_packet.guid.getGuidLow());

    //\todo SMSG_AUCTION_LIST_PENDING_SALES needs to be researched!
}
