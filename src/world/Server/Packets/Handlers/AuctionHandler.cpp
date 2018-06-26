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
#include "Server/Packets/CmsgAuctionPlaceBid.h"
#include "Server/Packets/CmsgAuctionSellItem.h"
#include "Server/WorldSession.h"
#include "Units/Players/Player.h"
#include "Map/MapMgr.h"
#include "Units/Creatures/Creature.h"
#include "Management/AuctionMgr.h"
#include "Management/ItemInterface.h"

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
#if VERSION_STRING > TBC
    CmsgAuctionListIPendingSales recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_AUCTION_LIST_PRENDING_SALES %u (lowguid)", recv_packet.guid.getGuidLow());

    //\todo SMSG_AUCTION_LIST_PENDING_SALES needs to be researched!
#endif
}

void WorldSession::handleAuctionSellItem(WorldPacket& recvPacket)
{
    CmsgAuctionSellItem recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_AUCTION_SELL_ITEM");

    if (!recv_packet.bidMoney || !recv_packet.expireTime)
        return;

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.auctioneerGuid.getGuidLow());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    const auto auctionHouse = creature->auctionHouse;
    recv_packet.expireTime *= MINUTE;

    switch (recv_packet.expireTime)
    {
        case 1 * MIN_AUCTION_TIME:
        case 2 * MIN_AUCTION_TIME:
        case 4 * MIN_AUCTION_TIME:
            break;
        default:
            return;
    }

    Item* items[MAX_AUCTION_ITEMS];

    uint32_t finalCount = 0;

    for (uint32_t i = 0; i < recv_packet.itemsCount; ++i)
    {
        const auto item = GetPlayer()->GetItemInterface()->GetItemByGUID(recv_packet.itemGuids[i]);
        if (!item)
        {
            GetPlayer()->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_ITEM);
            return;
        }

        items[i] = item;
        finalCount += recv_packet.count[i];
    }

    if (!finalCount)
    {
        GetPlayer()->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_INTERNAL);
        return;
    }

    for (uint32_t i = 0; i < recv_packet.itemsCount; ++i)
    {
        if (items[i] == nullptr || items[i]->getStackCount() < finalCount)
        {
            GetPlayer()->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_INTERNAL);
            return;
        }
    }

    for (uint32_t i = 0; i < recv_packet.itemsCount; ++i)
    {
        if (items[i] == nullptr)
        {
            GetPlayer()->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_INTERNAL);
            return;
        }

        const uint32_t item_worth = items[i]->getItemProperties()->SellPrice * items[i]->getStackCount();
        const uint32_t item_deposit = static_cast<uint32_t>(item_worth * auctionHouse->deposit_percent) * static_cast<uint32_t>(recv_packet.expireTime / 240.0f);

        if (!GetPlayer()->HasGold(item_deposit))
        {
            GetPlayer()->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_MONEY);
            return;
        }

        GetPlayer()->ModGold(-int32(item_deposit));

        const auto item = GetPlayer()->GetItemInterface()->SafeRemoveAndRetreiveItemByGuid(recv_packet.itemGuids[i], false);
        if (!item)
        {
            GetPlayer()->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_ITEM);
            return;
        };

        if (item->IsInWorld())
            item->RemoveFromWorld();

        item->setOwner(nullptr);
        item->m_isDirty = true;
        item->SaveToDB(INVENTORY_SLOT_NOT_SET, 0, true, nullptr);

        const auto auction = new Auction;
        auction->BuyoutPrice = static_cast<uint32_t>(recv_packet.buyoutPrice);
        auction->ExpiryTime = static_cast<uint32_t>(UNIXTIME) + recv_packet.expireTime * MINUTE;
        auction->StartingPrice = static_cast<uint32_t>(recv_packet.bidMoney);
        auction->HighestBid = 0;
        auction->HighestBidder = 0;
        auction->Id = sAuctionMgr.GenerateAuctionId();
        auction->Owner = GetPlayer()->getGuidLow();
        auction->pItem = item;
        auction->Deleted = false;
        auction->DeletedReason = 0;
        auction->DepositAmount = item_deposit;

        auctionHouse->AddAuction(auction);
        auction->SaveToDB(auctionHouse->GetID());

        GetPlayer()->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_NONE);
    }

    auctionHouse->SendOwnerListPacket(GetPlayer(), &recvPacket);
}

void WorldSession::handleAuctionPlaceBid(WorldPacket& recvPacket)
{
    CmsgAuctionPlaceBid recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_AUCTION_PLACE_BID: %u (auctionId), %u (price)", recv_packet.auctionId, recv_packet.price);

    const auto creature = GetPlayer()->GetMapMgr()->GetCreature(recv_packet.guid.getGuidLow());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    const auto auctionHouse = creature->auctionHouse;
    const auto auction = auctionHouse->GetAuction(recv_packet.auctionId);
    if (auction == nullptr || !auction->Owner || GetPlayer() == nullptr)
    {
        SendPacket(SmsgAuctionCommandResult(0, AUCTION_BID, AUCTION_ERROR_INTERNAL, 0).serialise().get());
        return;
    }

    if (auction->Owner == GetPlayer()->getGuidLow())
    {
        SendPacket(SmsgAuctionCommandResult(0, AUCTION_BID, AUCTION_ERROR_BID_OWN_AUCTION, 0).serialise().get());
        return;
    }

    if (auction->HighestBid > recv_packet.price && recv_packet.price != auction->BuyoutPrice)
    {
        SendPacket(SmsgAuctionCommandResult(0, AUCTION_BID, AUCTION_ERROR_INTERNAL, 0).serialise().get());
        return;
    }

    if (!GetPlayer()->HasGold(recv_packet.price))
    {
        SendPacket(SmsgAuctionCommandResult(0, AUCTION_BID, AUCTION_ERROR_MONEY, 0).serialise().get());
        return;
    }

    GetPlayer()->ModGold(-static_cast<int32_t>(recv_packet.price));
    if (auction->HighestBidder != 0)
    {
        char subject[100];
        snprintf(subject, 100, "%u:0:0", static_cast<int>(auction->pItem->getEntry()));
        sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, auctionHouse->GetID(), auction->HighestBidder, subject, "", auction->HighestBid, 0, 0, MAIL_STATIONERY_AUCTION);

        if (auction->HighestBidder != GetPlayer()->getGuidLow())
            auctionHouse->SendAuctionOutBidNotificationPacket(auction, GetPlayer()->getGuid(), recv_packet.price);
    }

    if (auction->BuyoutPrice == recv_packet.price)
    {
        auction->HighestBidder = GetPlayer()->getGuidLow();
        auction->HighestBid = recv_packet.price;

        auctionHouse->QueueDeletion(auction, AUCTION_REMOVE_WON);

        SendPacket(SmsgAuctionCommandResult(auction->Id, AUCTION_BID, AUCTION_ERROR_NONE, 0).serialise().get());
        auctionHouse->SendAuctionBuyOutNotificationPacket(auction);
    }
    else
    {
        auction->HighestBidder = GetPlayer()->getGuidLow();
        auction->HighestBid = recv_packet.price;
        auction->UpdateInDB();

        SendPacket(SmsgAuctionCommandResult(auction->Id, AUCTION_BID, AUCTION_ERROR_NONE, 0).serialise().get());
    }
}
