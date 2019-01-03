/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
    CmsgAuctionListOwnerItems srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_AUCTION_LIST_OWNER_ITEMS %u (guidLow)", srlPacket.guid.getGuidLow());

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLow());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    creature->auctionHouse->SendOwnerListPacket(_player, nullptr);
}

void WorldSession::handleAuctionListItems(WorldPacket& recvPacket)
{
    CmsgAuctionListItems srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_AUCTION_LIST_OWNER_ITEMS %u (guidLow)", srlPacket.guid.getGuidLow());

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLow());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    creature->auctionHouse->SendAuctionList(_player, &recvPacket);
}

void WorldSession::handleCancelAuction(WorldPacket& recvPacket)
{
    CmsgAuctionRemoveItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_AUCTION_REMOVE_ITEM %u (auctionId)", srlPacket.auctionId);

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLow());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    const auto auction = creature->auctionHouse->GetAuction(srlPacket.auctionId);
    if (auction == nullptr)
        return;

    creature->auctionHouse->QueueDeletion(auction, AUCTION_REMOVE_CANCELLED);

    SendPacket(SmsgAuctionCommandResult(srlPacket.auctionId, AUCTION_CANCEL, AUCTION_ERROR_NONE).serialise().get());

    creature->auctionHouse->SendOwnerListPacket(_player, nullptr);
}

void WorldSession::handleAuctionListBidderItems(WorldPacket& recvPacket)
{
    CmsgAuctionListIBidderItemse srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_AUCTION_LIST_BIDDER_ITEMS %u (lowguid)", srlPacket.guid.getGuidLow());

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLow());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    creature->auctionHouse->SendBidListPacket(_player, &recvPacket);
}

void WorldSession::handleAuctionListPendingSales(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    CmsgAuctionListIPendingSales srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_AUCTION_LIST_PRENDING_SALES %u (lowguid)", srlPacket.guid.getGuidLow());

    //\todo SMSG_AUCTION_LIST_PENDING_SALES needs to be researched!
#endif
}

void WorldSession::handleAuctionSellItem(WorldPacket& recvPacket)
{
    CmsgAuctionSellItem srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_AUCTION_SELL_ITEM");

    if (!srlPacket.bidMoney || !srlPacket.expireTime)
        return;

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.auctioneerGuid.getGuidLow());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    const auto auctionHouse = creature->auctionHouse;
    srlPacket.expireTime *= MINUTE;

    switch (srlPacket.expireTime)
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

    for (uint32_t i = 0; i < srlPacket.itemsCount; ++i)
    {
        const auto item = _player->getItemInterface()->GetItemByGUID(srlPacket.itemGuids[i]);
        if (!item)
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_ITEM);
            return;
        }

        items[i] = item;
        finalCount += srlPacket.count[i];
    }

    if (!finalCount)
    {
        _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_INTERNAL);
        return;
    }

    for (uint32_t i = 0; i < srlPacket.itemsCount; ++i)
    {
        if (items[i] == nullptr || items[i]->getStackCount() < finalCount)
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_INTERNAL);
            return;
        }
    }

    for (uint32_t i = 0; i < srlPacket.itemsCount; ++i)
    {
        if (items[i] == nullptr)
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_INTERNAL);
            return;
        }

        const uint32_t item_worth = items[i]->getItemProperties()->SellPrice * items[i]->getStackCount();
        const uint32_t item_deposit = static_cast<uint32_t>(item_worth * auctionHouse->deposit_percent) * static_cast<uint32_t>(srlPacket.expireTime / 240.0f);

        if (!_player->hasEnoughCoinage(item_deposit))
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_MONEY);
            return;
        }

        _player->modCoinage(-int32(item_deposit));

        const auto item = _player->getItemInterface()->SafeRemoveAndRetreiveItemByGuid(srlPacket.itemGuids[i], false);
        if (!item)
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_ITEM);
            return;
        };

        if (item->IsInWorld())
            item->RemoveFromWorld();

        item->setOwner(nullptr);
        item->m_isDirty = true;
        item->SaveToDB(INVENTORY_SLOT_NOT_SET, 0, true, nullptr);

        const auto auction = new Auction;
        auction->BuyoutPrice = static_cast<uint32_t>(srlPacket.buyoutPrice);
        auction->ExpiryTime = static_cast<uint32_t>(UNIXTIME) + srlPacket.expireTime * MINUTE;
        auction->StartingPrice = static_cast<uint32_t>(srlPacket.bidMoney);
        auction->HighestBid = 0;
        auction->HighestBidder = 0;
        auction->Id = sAuctionMgr.GenerateAuctionId();
        auction->Owner = _player->getGuidLow();
        auction->pItem = item;
        auction->Deleted = false;
        auction->DeletedReason = 0;
        auction->DepositAmount = item_deposit;

        auctionHouse->AddAuction(auction);
        auction->SaveToDB(auctionHouse->GetID());

        _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_NONE);
    }

    auctionHouse->SendOwnerListPacket(_player, &recvPacket);
}

void WorldSession::handleAuctionPlaceBid(WorldPacket& recvPacket)
{
    CmsgAuctionPlaceBid srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    LogDebugFlag(LF_OPCODE, "Received CMSG_AUCTION_PLACE_BID: %u (auctionId), %u (price)", srlPacket.auctionId, srlPacket.price);

    const auto creature = _player->GetMapMgr()->GetCreature(srlPacket.guid.getGuidLow());
    if (creature == nullptr || creature->auctionHouse == nullptr)
        return;

    const auto auctionHouse = creature->auctionHouse;
    const auto auction = auctionHouse->GetAuction(srlPacket.auctionId);
    if (auction == nullptr || !auction->Owner || _player == nullptr)
    {
        SendPacket(SmsgAuctionCommandResult(0, AUCTION_BID, AUCTION_ERROR_INTERNAL, 0).serialise().get());
        return;
    }

    if (auction->Owner == _player->getGuidLow())
    {
        SendPacket(SmsgAuctionCommandResult(0, AUCTION_BID, AUCTION_ERROR_BID_OWN_AUCTION, 0).serialise().get());
        return;
    }

    if (auction->HighestBid > srlPacket.price && srlPacket.price != auction->BuyoutPrice)
    {
        SendPacket(SmsgAuctionCommandResult(0, AUCTION_BID, AUCTION_ERROR_INTERNAL, 0).serialise().get());
        return;
    }

    if (!_player->hasEnoughCoinage(srlPacket.price))
    {
        SendPacket(SmsgAuctionCommandResult(0, AUCTION_BID, AUCTION_ERROR_MONEY, 0).serialise().get());
        return;
    }

    _player->modCoinage(-static_cast<int32_t>(srlPacket.price));
    if (auction->HighestBidder != 0)
    {
        char subject[100];
        snprintf(subject, 100, "%u:0:0", static_cast<int>(auction->pItem->getEntry()));
        sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, auctionHouse->GetID(), auction->HighestBidder, subject, "", auction->HighestBid, 0, 0, MAIL_STATIONERY_AUCTION);

        if (auction->HighestBidder != _player->getGuidLow())
            auctionHouse->SendAuctionOutBidNotificationPacket(auction, _player->getGuid(), srlPacket.price);
    }

    if (auction->BuyoutPrice == srlPacket.price)
    {
        auction->HighestBidder = _player->getGuidLow();
        auction->HighestBid = srlPacket.price;

        auctionHouse->QueueDeletion(auction, AUCTION_REMOVE_WON);

        SendPacket(SmsgAuctionCommandResult(auction->Id, AUCTION_BID, AUCTION_ERROR_NONE, 0).serialise().get());
        auctionHouse->SendAuctionBuyOutNotificationPacket(auction);
    }
    else
    {
        auction->HighestBidder = _player->getGuidLow();
        auction->HighestBid = srlPacket.price;
        auction->UpdateInDB();

        SendPacket(SmsgAuctionCommandResult(auction->Id, AUCTION_BID, AUCTION_ERROR_NONE, 0).serialise().get());
    }
}
