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
            _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_ITEM);
            return;
        }

        items[i] = item;
        finalCount += count[i];
    }

    if (!finalCount)
    {
        _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_INTERNAL);
        return;
    }

    for (uint32_t i = 0; i < itemsCount; ++i)
    {
        Item* item = items[i];

        if (item->getStackCount() < finalCount)
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_INTERNAL);
            return;
        }
    }

    for (uint32_t i = 0; i < itemsCount; ++i)
    {
        Item* item = items[i];

        AuctionHouse* auctionHouse = pCreature->auctionHouse;

        uint32_t item_worth = item->getItemProperties()->SellPrice * item->getStackCount();
        uint32_t item_deposit = (uint32_t)(item_worth * auctionHouse->deposit_percent) * (uint32_t)(etime / 240.0f);

        if (!_player->HasGold((uint64_t)item_deposit))
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_MONEY);
            return;
        }

        _player->ModGold(-int32(item_deposit));

        item = _player->GetItemInterface()->SafeRemoveAndRetreiveItemByGuid(itemGUIDs[i], false);
        if (!item)
        {
            _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_ITEM);
            return;
        };

        if (item->IsInWorld())
        {
            item->RemoveFromWorld();
        }

        item->setOwner(nullptr);
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
        auct->Owner = _player->getGuidLow();
        auct->pItem = item;
        auct->Deleted = false;
        auct->DeletedReason = 0;
        auct->DepositAmount = item_deposit;

        auctionHouse->AddAuction(auct);
        auct->SaveToDB(auctionHouse->GetID());

        _player->sendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERROR_NONE);
    }

    ah->SendOwnerListPacket(_player, &recvData);
}
