/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/WorldSession.h"
#include "Management/AuctionHouse.h"
#include "Management/AuctionMgr.h"
#include "Server/MainServerDefines.h"
#include "Log.hpp"
#include "Management/MailMgr.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Objects/ObjectMgr.h"
#include "Management/ItemInterface.h"
#include "Management/Item.h"
#include "Map/MapMgr.h"


void WorldSession::HandleAuctionListBidderItems(WorldPacket& recv_data)
{
    uint64_t guid;
    uint32_t listfrom;
    uint32_t outbiddedCount;
    
    recv_data >> guid;
    recv_data >> listfrom;              // not used
    recv_data >> outbiddedCount;        // not used

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature || !pCreature->auctionHouse)
        return;

    pCreature->auctionHouse->SendBidListPacket(_player, &recv_data);
}

void WorldSession::SendAuctionPlaceBidResultPacket(uint32_t itemId, uint32_t error)
{
    WorldPacket data(SMSG_AUCTION_COMMAND_RESULT, 16);
    data << itemId;
    data << uint32_t(AUCTION_BID);
    data << error;
    data << uint32_t(0);
    SendPacket(&data);
}

void WorldSession::HandleAuctionPlaceBid(WorldPacket& recv_data)
{
    uint64_t guid;
    uint32_t auction_id;
    uint32_t price;

    recv_data >> guid;
    recv_data >> auction_id;
    recv_data >> price;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature || !pCreature->auctionHouse)
        return;

    AuctionHouse* ah = pCreature->auctionHouse;
    Auction* auct = ah->GetAuction(auction_id);
    if (!auct || !auct->Owner || !_player)
    {
        SendAuctionPlaceBidResultPacket(0, AUCTION_ERR_INTERNAL_DB);
        return;
    }

    if (auct->Owner == _player->GetGUID())
    {
        SendAuctionPlaceBidResultPacket(0, AUCTION_ERR_BID_OWN_AUCTION);
        return;
    }
    if (auct->HighestBid > price && price != auct->BuyoutPrice)
    {
        SendAuctionPlaceBidResultPacket(0, AUCTION_ERR_INTERNAL_DB);
        return;
    }

    if (!_player->HasGold(price))
    {
        SendAuctionPlaceBidResultPacket(0, AUCTION_ERR_NOT_ENOUGH_MONEY);
        return;
    }

    _player->ModGold(-(int32)price);
    if (auct->HighestBidder != 0)
    {
        char subject[100];
        snprintf(subject, 100, "%u:0:0", (int)auct->pItem->GetEntry());
        sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, ah->GetID(), auct->HighestBidder, subject, "", auct->HighestBid, 0, 0, MAIL_STATIONERY_AUCTION);

        if (auct->HighestBidder != (uint32_t)_player->GetLowGUID())
            ah->SendAuctionOutBidNotificationPacket(auct, _player->GetGUID(), price);
    }

    if (auct->BuyoutPrice == price)
    {
        auct->HighestBidder = _player->GetLowGUID();
        auct->HighestBid = price;

        ah->QueueDeletion(auct, AUCTION_REMOVE_WON);

        SendAuctionPlaceBidResultPacket(auct->Id, AUCTION_ERR_NONE);
        ah->SendAuctionBuyOutNotificationPacket(auct);
    }
    else
    {
        auct->HighestBidder = _player->GetLowGUID();
        auct->HighestBid = price;
        auct->UpdateInDB();

        SendAuctionPlaceBidResultPacket(auct->Id, AUCTION_ERR_NONE);
    }

    ah->SendAuctionList(_player, &recv_data);
}

void WorldSession::HandleCancelAuction(WorldPacket& recv_data)
{
    uint64_t guid;
    uint32_t auction_id;

    recv_data >> guid;
    recv_data >> auction_id;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature || !pCreature->auctionHouse)
        return;

    Auction* auct = pCreature->auctionHouse->GetAuction(auction_id);
    if (!auct)
        return;

    pCreature->auctionHouse->QueueDeletion(auct, AUCTION_REMOVE_CANCELLED);

    WorldPacket data(SMSG_AUCTION_COMMAND_RESULT, 8);
    data << auction_id;
    data << uint32_t(AUCTION_CANCEL);
    data << uint32_t(0);
    SendPacket(&data);

    pCreature->auctionHouse->SendOwnerListPacket(_player, 0);
}

void WorldSession::HandleAuctionSellItem(WorldPacket& recvData)
{
    uint64_t auctioneer;
    uint64_t bid;
    uint64_t buyout;
    uint32_t itemsCount;
    uint32_t etime;

    recvData >> auctioneer;
    recvData >> itemsCount;

    uint64_t itemGUIDs[MAX_AUCTION_ITEMS];
    uint32_t count[MAX_AUCTION_ITEMS];
    
    for (uint32_t i = 0; i < itemsCount; ++i)
    {
        recvData >> itemGUIDs[i];
        recvData >> count[i];

        if (!itemGUIDs[i] || !count[i] || count[i] > 1000)
            return;
    }

    recvData >> bid;
    recvData >> buyout;
    recvData >> etime;

    if (!bid || !etime)
        return;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(auctioneer));
    if (!pCreature || !pCreature->auctionHouse)
        return;

    AuctionHouse* ah = pCreature->auctionHouse;
    etime *= MINUTE;

    switch (etime)
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

    for (uint32_t i = 0; i < itemsCount; ++i)
    {
        Item* item = _player->GetItemInterface()->GetItemByGUID(itemGUIDs[i]);
        if (!item)
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERR_ITEM_NOT_FOUND);
            return;
        }

        items[i] = item;
        finalCount += count[i];
    }

    if (!finalCount)
    {
        _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERR_INTERNAL_DB);
        return;
    }

    for (uint32_t i = 0; i < itemsCount; ++i)
    {
        Item* item = items[i];

        if (item->GetStackCount() < finalCount)
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERR_INTERNAL_DB);
            return;
        }
    }

    for (uint32_t i = 0; i < itemsCount; ++i)
    {
        Item* item = items[i];

        AuctionHouse* auctionHouse = pCreature->auctionHouse;

        uint32_t item_worth = item->GetItemProperties()->SellPrice * item->GetStackCount();
        uint32_t item_deposit = (uint32_t)(item_worth * auctionHouse->deposit_percent) * (uint32_t)(etime / 240.0f);

        if (!_player->HasGold((uint64_t)item_deposit))
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERR_NOT_ENOUGH_MONEY);
            return;
        }

        _player->TakeGold(-int32(item_deposit));

        item = _player->GetItemInterface()->SafeRemoveAndRetreiveItemByGuid(itemGUIDs[i], false);
        if (!item)
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERR_ITEM_NOT_FOUND);
            return;
        };

        if (item->IsInWorld())
        {
            item->RemoveFromWorld();
        }

        item->SetOwner(nullptr);
        item->m_isDirty = true;
        item->SaveToDB(INVENTORY_SLOT_NOT_SET, 0, true, nullptr);

        // Create auction
        Auction* auct = new Auction;
        auct->BuyoutPrice = static_cast<uint32_t>(buyout);
        auct->ExpiryTime = (uint32_t)UNIXTIME + (etime * 60);
        auct->StartingPrice = static_cast<uint32_t>(bid);
        auct->HighestBid = 0;
        auct->HighestBidder = 0;
        auct->Id = sAuctionMgr.GenerateAuctionId();
        auct->Owner = _player->GetLowGUID();
        auct->pItem = item;
        auct->Deleted = false;
        auct->DeletedReason = 0;
        auct->DepositAmount = item_deposit;

        auctionHouse->AddAuction(auct);
        auct->SaveToDB(auctionHouse->GetID());

        _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERR_NONE);
    }

    ah->SendOwnerListPacket(_player, &recvData);
}

void WorldSession::HandleAuctionListOwnerItems(WorldPacket& recv_data)
{
    uint64_t guid;
    uint32_t listfrom;

    recv_data >> guid;
    recv_data >> listfrom;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature || !pCreature->auctionHouse)
        return;

    pCreature->auctionHouse->SendOwnerListPacket(_player, &recv_data);
}

void WorldSession::HandleAuctionListItems(WorldPacket& recv_data)
{
    uint64_t guid;
    recv_data >> guid;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature || !pCreature->auctionHouse)
        return;

    pCreature->auctionHouse->SendAuctionList(_player, &recv_data);
}

void WorldSession::HandleAuctionListPendingSales(WorldPacket& recv_data)
{
    uint64_t guid;
    recv_data >> guid;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature || !pCreature->auctionHouse)
        return;

    WorldPacket data(SMSG_AUCTION_LIST_PENDING_SALES, 4);
    data << uint32_t(0);
    SendPacket(&data);
}
