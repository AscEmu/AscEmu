/*
* AscEmu Framework based on ArcEmu MMORPG Server
* Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
* Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
* Copyright (C) 2005-2007 Ascent Team
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "StdAfx.h"
#include "Management/AuctionHouse.h"
#include "Management/AuctionMgr.h"
#include "Management/Item.h"
#include "Management/ItemInterface.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Objects/ObjectMgr.h"
#include "Util.hpp"
#include "Server/Packets/SmsgAuctionCommandResult.h"

#if VERSION_STRING != Cata

using namespace AscEmu::Packets;

void WorldSession::HandleAuctionPlaceBid(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        uint64 guid;
    recv_data >> guid;

    uint32 auction_id, price;
    recv_data >> auction_id;
    recv_data >> price;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature || !pCreature->auctionHouse)
        return;

    // Find Item
    AuctionHouse* ah = pCreature->auctionHouse;
    Auction* auct = ah->GetAuction(auction_id);
    if (auct == 0 || !auct->Owner || !_player)
    {
        SendPacket(SmsgAuctionCommandResult(0, AUCTION_BID, AUCTION_ERROR_INTERNAL, 0).serialise().get());
        return;
    }

    if (auct->Owner == _player->getGuid())
    {
        SendPacket(SmsgAuctionCommandResult(0, AUCTION_BID, AUCTION_ERROR_BID_OWN_AUCTION, 0).serialise().get());
        return;
    }
    if (auct->HighestBid > price && price != auct->BuyoutPrice)
    {
        //HACK: Don't know the correct error code...
        SendPacket(SmsgAuctionCommandResult(0, AUCTION_BID, AUCTION_ERROR_INTERNAL, 0).serialise().get());
        return;
    }

    if (!_player->HasGold(price))
    {
        SendPacket(SmsgAuctionCommandResult(0, AUCTION_BID, AUCTION_ERROR_MONEY, 0).serialise().get());
        return;
    }

    _player->ModGold(-(int32)price);
    if (auct->HighestBidder != 0)
    {
        // Return the money to the last highest bidder.
        char subject[100];
        snprintf(subject, 100, "%u:0:0", (int)auct->pItem->getEntry());
        sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, ah->GetID(), auct->HighestBidder, subject, "", auct->HighestBid, 0, 0, MAIL_STATIONERY_AUCTION);

        // Do not send out bid notification, when current highest bidder and new bidder are the same player..
        if (auct->HighestBidder != (uint32)_player->getGuidLow())
            ah->SendAuctionOutBidNotificationPacket(auct, _player->getGuid(), price);
    }

    if (auct->BuyoutPrice == price)
    {
        auct->HighestBidder = _player->getGuidLow();
        auct->HighestBid = price;

        // we used buyout on the item.
        ah->QueueDeletion(auct, AUCTION_REMOVE_WON);

        SendPacket(SmsgAuctionCommandResult(auct->Id, AUCTION_BID, AUCTION_ERROR_NONE, 0).serialise().get());
        ah->SendAuctionBuyOutNotificationPacket(auct);
    }
    else
    {
        // update most recent bid
        auct->HighestBidder = _player->getGuidLow();
        auct->HighestBid = price;
        auct->UpdateInDB();

        SendPacket(SmsgAuctionCommandResult(auct->Id, AUCTION_BID, AUCTION_ERROR_NONE, 0).serialise().get());
    }
}

void WorldSession::HandleAuctionSellItem(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        uint64 guid;
    uint64 item;
    uint32 bid;
    uint32 buyout;
    uint32 etime;   // in minutes
    uint32 unk1;
    uint32 unk2;

    recv_data >> guid;
    recv_data >> unk1;
    recv_data >> item;
    recv_data >> unk2;
    recv_data >> bid;
    recv_data >> buyout;
    recv_data >> etime;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature || !pCreature->auctionHouse)
        return;        // NPC doesn't exist or isn't an auctioneer

                       // Get item
    Item* pItem = _player->GetItemInterface()->GetItemByGUID(item);
    if (!pItem || pItem->isSoulbound() || pItem->hasFlags(ITEM_FLAG_CONJURED))
    {
        SendPacket(SmsgAuctionCommandResult(0, AUCTION_CREATE, AUCTION_ERROR_ITEM).serialise().get());
        return;
    };

    AuctionHouse* ah = pCreature->auctionHouse;

    uint32 item_worth = pItem->getItemProperties()->SellPrice * pItem->getStackCount();
    uint32 item_deposit = (uint32)(item_worth * ah->deposit_percent) * (uint32)(etime / 240.0f); // deposit is per 4 hours

    if (!_player->HasGold(item_deposit))   // player cannot afford deposit
    {
        SendPacket(SmsgAuctionCommandResult(0, AUCTION_CREATE, AUCTION_ERROR_MONEY).serialise().get());
        return;
    }

    pItem = _player->GetItemInterface()->SafeRemoveAndRetreiveItemByGuid(item, false);
    if (!pItem)
    {
        SendPacket(SmsgAuctionCommandResult(0, AUCTION_CREATE, AUCTION_ERROR_ITEM).serialise().get());
        return;
    };

    if (pItem->IsInWorld())
    {
        pItem->RemoveFromWorld();
    }

    pItem->setOwner(NULL);
    pItem->m_isDirty = true;
    pItem->SaveToDB(INVENTORY_SLOT_NOT_SET, 0, true, NULL);

    // Create auction
    Auction* auct = new Auction;
    auct->BuyoutPrice = buyout;
    auct->ExpiryTime = (uint32)UNIXTIME + (etime * 60);
    auct->StartingPrice = bid;
    auct->HighestBid = 0;
    auct->HighestBidder = 0;    // hm
    auct->Id = sAuctionMgr.GenerateAuctionId();
    auct->Owner = _player->getGuidLow();
    auct->pItem = pItem;
    auct->Deleted = false;
    auct->DeletedReason = 0;
    auct->DepositAmount = item_deposit;

    // remove deposit
    _player->ModGold(-(int32)item_deposit);

    // Add and save auction to DB
    ah->AddAuction(auct);
    auct->SaveToDB(ah->GetID());

    // Send result packet
    SendPacket(SmsgAuctionCommandResult(auct->Id, AUCTION_CREATE, AUCTION_ERROR_NONE).serialise().get());
}
#endif