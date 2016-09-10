/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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

void Auction::DeleteFromDB()
{
    CharacterDatabase.WaitExecute("DELETE FROM auctions WHERE auctionId = %u", Id);
}

void Auction::SaveToDB(uint32 AuctionHouseId)
{
    CharacterDatabase.Execute("INSERT INTO auctions VALUES(%u, %u, %u, %u, %u, %u, %u, %u, %u, %u)", Id, AuctionHouseId, pItem->GetLowGUID(), Owner, StartingPrice, BuyoutPrice, ExpiryTime, HighestBidder, HighestBid, DepositAmount);
}

void Auction::UpdateInDB()
{
    CharacterDatabase.Execute("UPDATE auctions SET bidder = %u, bid = %u WHERE auctionId = %u", HighestBidder, HighestBid, Id);
}

AuctionHouse::AuctionHouse(uint32 ID)
{
    dbc = sAuctionHouseStore.LookupEntry(ID);
    ARCEMU_ASSERT(dbc != NULL);

    cut_percent = dbc->tax / 100.0f;
    deposit_percent = dbc->fee / 100.0f;

    enabled = true;
}

AuctionHouse::~AuctionHouse()
{
    std::unordered_map<uint32, Auction*>::iterator itr = auctions.begin();
    for (; itr != auctions.end(); ++itr)
        delete itr->second;
}

void AuctionHouse::QueueDeletion(Auction* auct, uint32 Reason)
{
    if (auct->Deleted)
        return;

    auct->Deleted = true;
    auct->DeletedReason = Reason;
    removalLock.Acquire();
    removalList.push_back(auct);
    removalLock.Release();
}

void AuctionHouse::UpdateDeletionQueue()
{
    removalLock.Acquire();
    Auction* auct;

    std::list<Auction*>::iterator it = removalList.begin();
    for (; it != removalList.end(); ++it)
    {
        auct = *it;
        ARCEMU_ASSERT(auct->Deleted);
        RemoveAuction(auct);
    }

    removalList.clear();
    removalLock.Release();
}

void AuctionHouse::UpdateAuctions()
{
    auctionLock.AcquireReadLock();
    removalLock.Acquire();

    uint32 t = (uint32)UNIXTIME;
    std::unordered_map<uint32, Auction*>::iterator itr = auctions.begin();
    Auction* auct;
    for (; itr != auctions.end();)
    {
        auct = itr->second;
        ++itr;

        if (t >= auct->ExpiryTime)
        {
            if (auct->HighestBidder == 0)
            {
                auct->DeletedReason = AUCTION_REMOVE_EXPIRED;
                this->SendAuctionExpiredNotificationPacket(auct);
            }
            else
            {
                auct->DeletedReason = AUCTION_REMOVE_WON;
            }

            auct->Deleted = true;
            removalList.push_back(auct);
        }
    }

    removalLock.Release();
    auctionLock.ReleaseReadLock();
}

void AuctionHouse::AddAuction(Auction* auct)
{
    // add to the map
    auctionLock.AcquireWriteLock();
    auctions.insert(std::unordered_map<uint32, Auction*>::value_type(auct->Id, auct));
    auctionLock.ReleaseWriteLock();

    Log.Debug("AuctionHouse", "%u: Add auction %u, expire@ %u.", dbc->id, auct->Id, auct->ExpiryTime);
}

Auction* AuctionHouse::GetAuction(uint32 Id)
{
    Auction* ret;
    std::unordered_map<uint32, Auction*>::iterator itr;
    auctionLock.AcquireReadLock();
    itr = auctions.find(Id);
    ret = (itr == auctions.end()) ? 0 : itr->second;
    auctionLock.ReleaseReadLock();
    return ret;
}

void AuctionHouse::RemoveAuction(Auction* auct)
{
    Log.Debug("AuctionHouse", "%u: Removing auction %u, reason %u.", dbc->id, auct->Id, auct->DeletedReason);

    char subject[100];
    char body[200];
    switch (auct->DeletedReason)
    {
        case AUCTION_REMOVE_EXPIRED:
        {
            // ItemEntry:0:3
            snprintf(subject, 100, "%u:0:3", (unsigned int)auct->pItem->GetEntry());

            // Auction expired, resend item, no money to owner.
            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, dbc->id, auct->Owner, subject, "", 0, 0, auct->pItem->GetGUID(), MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);
        }
        break;

        case AUCTION_REMOVE_WON:
        {
            // ItemEntry:0:1
            snprintf(subject, 100, "%u:0:1", (unsigned int)auct->pItem->GetEntry());

            // <owner player guid>:bid:buyout
            snprintf(body, 200, "%X:%u:%u", (unsigned int)auct->Owner, (unsigned int)auct->HighestBid, (unsigned int)auct->BuyoutPrice);

            // Auction won by highest bidder. He gets the item.
            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, dbc->id, auct->HighestBidder, subject, body, 0, 0, auct->pItem->GetGUID(), MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);

            // Send a mail to the owner with his cut of the price.
            uint32 auction_cut = float2int32(cut_percent * auct->HighestBid);
            int32 amount = auct->HighestBid - auction_cut + auct->DepositAmount;
            if (amount < 0)
                amount = 0;

            // ItemEntry:0:2
            snprintf(subject, 100, "%u:0:2", (unsigned int)auct->pItem->GetEntry());

            // <hex player guid>:bid:0:deposit:cut
            if (auct->HighestBid == auct->BuyoutPrice)       // Buyout
                snprintf(body, 200, "%X:%u:%u:%u:%u", (unsigned int)auct->HighestBidder, (unsigned int)auct->HighestBid, (unsigned int)auct->BuyoutPrice, (unsigned int)auct->DepositAmount, (unsigned int)auction_cut);
            else
                snprintf(body, 200, "%X:%u:0:%u:%u", (unsigned int)auct->HighestBidder, (unsigned int)auct->HighestBid, (unsigned int)auct->DepositAmount, (unsigned int)auction_cut);

            // send message away.
            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, dbc->id, auct->Owner, subject, body, amount, 0, 0, MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);

            // If it's not a buyout (otherwise the players has been already notified)
            if (auct->HighestBid < auct->BuyoutPrice || auct->BuyoutPrice == 0)
            {
                this->SendAuctionBuyOutNotificationPacket(auct);
            }
        }
        break;
        case AUCTION_REMOVE_CANCELLED:
        {
            snprintf(subject, 100, "%u:0:5", (unsigned int)auct->pItem->GetEntry());
            uint32 cut = float2int32(cut_percent * auct->HighestBid);
            Player* plr = objmgr.GetPlayer(auct->Owner);
            if (cut && plr && plr->HasGold(cut))
                plr->ModGold(-(int32)cut);

            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, GetID(), auct->Owner, subject, "", 0, 0, auct->pItem->GetGUID(), MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);

            // return bidders money
            if (auct->HighestBidder)
            {
                sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, GetID(), auct->HighestBidder, subject, "", auct->HighestBid, 0, 0, MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);
            }

        }
        break;
    }

    // Remove the auction from the hashmap.
    auctionLock.AcquireWriteLock();
    auctions.erase(auct->Id);
    auctionLock.ReleaseWriteLock();

    // Destroy the item from memory (it still remains in the db)
    if (auct->pItem)
        auct->pItem->DeleteMe();

    // Finally destroy the auction instance.
    auct->DeleteFromDB();
    delete auct;
}

void WorldSession::HandleAuctionListBidderItems(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 guid;                        //NPC guid
    uint32 listfrom;                    //page of auctions
    uint32 outbiddedCount;              //count of outbidded auctions
    
    recv_data >> guid;
    recv_data >> listfrom;              // not used in fact (this list not have page control in client)
    recv_data >> outbiddedCount;        // not used in fact (this is The Count of Items)

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature || !pCreature->auctionHouse)
        return;

    pCreature->auctionHouse->SendBidListPacket(_player, &recv_data);
}

void AuctionHouse::SendBidListPacket(Player* plr, WorldPacket* packet)
{
    uint32 count = 0;
    uint32 totalcount = 0;

    WorldPacket data(SMSG_AUCTION_BIDDER_LIST_RESULT, 4 + 4 + 4);
    data << uint32(0);                  // Placeholder

    auctionLock.AcquireReadLock();
    std::unordered_map<uint32, Auction*>::iterator itr = auctions.begin();
    for (; itr != auctions.end(); ++itr)
    {
        Auction* auct = itr->second;
        if (auct->HighestBidder == plr->GetGUID())
        {
            if (auct->Deleted) continue;

            if (auct->BuildAuctionInfo(data))
            {
                ++count;
                ++totalcount;
            }
        }
    }

    data.put<uint32>(0, count);         // add count to placeholder
    data << totalcount;
    data << uint32(300);                //unk 2.3.0
    auctionLock.ReleaseReadLock();
    plr->GetSession()->SendPacket(&data);
}

void AuctionHouse::UpdateOwner(uint32 oldGuid, uint32 newGuid)
{
    auctionLock.AcquireWriteLock();
    std::unordered_map<uint32, Auction*>::iterator itr = auctions.begin();
    Auction* auction;
    for (; itr != auctions.end(); ++itr)
    {
        auction = itr->second;
        if (auction->Owner == oldGuid)
            auction->Owner = newGuid;
        if (auction->HighestBidder == oldGuid)
        {
            auction->HighestBidder = newGuid;
            auction->UpdateInDB();
        }
    }
    auctionLock.ReleaseWriteLock();
}

void AuctionHouse::SendOwnerListPacket(Player* plr, WorldPacket* packet)
{
    uint32 count = 0;
    uint32 totalcount = 0;

    WorldPacket data(SMSG_AUCTION_OWNER_LIST_RESULT, 4 + 4 + 4);
    data << uint32(0);                       // Placeholder

    Auction* auct;
    auctionLock.AcquireReadLock();
    std::unordered_map<uint32, Auction*>::iterator itr = auctions.begin();
    for (; itr != auctions.end(); ++itr)
    {
        auct = itr->second;
        if (auct->Owner == plr->GetGUID())
        {
            if (auct->Deleted)
                continue;

            if (auct->BuildAuctionInfo(data))
                ++count;

            ++totalcount;
        }
    }

    data.put<uint32>(0, count);
    data << uint32(totalcount);
    data << uint32(0);
    auctionLock.ReleaseReadLock();
    plr->GetSession()->SendPacket(&data);
}

bool Auction::BuildAuctionInfo(WorldPacket& data)
{
    if (!pItem)
    {
        Log.Error("AuctionHouse", "AuctionEntry::BuildAuctionInfo: Auction %u has a non-existent item: %u", Id, pItem->GetEntry());
        return false;
    }
    data << uint32(Id);
    data << uint32(pItem->GetEntry());

    for (uint8 i = 0; i < PROP_ENCHANTMENT_SLOT_0; ++i) // PROP_ENCHANTMENT_SLOT_0 = 10
    {
        data << uint32(pItem->GetEnchantmentId(EnchantmentSlot(i)));
        data << uint32(pItem->GetEnchantmentDuration(EnchantmentSlot(i)));
        data << uint32(pItem->GetEnchantmentCharges(EnchantmentSlot(i)));
    }

    data << int32(pItem->GetItemRandomPropertyId());                // Random item property id
    data << uint32(pItem->GetItemRandomSuffixFactor());             // SuffixFactor
    data << uint32(pItem->GetStackCount());                         // item->count
    data << uint32(pItem->GetChargesLeft());                        // item->charge FFFFFFF
    data << uint32(0);                                              // Unknown
    data << uint64(Owner);                                          // Auction->owner
    data << uint64(StartingPrice);                                  // Auction->startbid (not sure if useful)
    data << uint64(HighestBid ? GetAuctionOutBid() : 0);
    // Minimal outbid
    data << uint64(BuyoutPrice);                                    // Auction->buyout
    data << uint32((ExpiryTime - time(NULL)) * IN_MILLISECONDS);    // time left
    data << uint64(HighestBidder);                                  // auction->bidder current
    data << uint64(HighestBid);                                     // current bid
    return true;
}

void AuctionHouse::SendAuctionOutBidNotificationPacket(Auction* auct, uint64 newBidder, uint32 newHighestBid)
{
    Player* bidder = objmgr.GetPlayer(auct->HighestBidder);
    if (bidder != NULL && bidder->IsInWorld())
    {
        uint32 outbid = (auct->HighestBid / 100) * 5;
        if (!outbid)
            outbid = 1;

        ///\todo Check this code, when a user has been bid out by instant buy out
        WorldPacket data(SMSG_AUCTION_BIDDER_NOTIFICATION, 32);
        data << GetID();
        data << auct->Id;
        data << uint64(newBidder);
        data << uint32(newHighestBid);
        data << uint32(outbid);
        data << auct->pItem->GetEntry();
        data << uint32(0);
        bidder->GetSession()->SendPacket(&data);
    }
}

void AuctionHouse::SendAuctionBuyOutNotificationPacket(Auction* auct)
{
    Player* bidder = objmgr.GetPlayer((uint32)auct->HighestBidder);
    if (bidder != NULL && bidder->IsInWorld())
    {
        uint32 outbid = (auct->HighestBid / 100) * 5;
        if (!outbid)
            outbid = 1;

        WorldPacket data(SMSG_AUCTION_BIDDER_NOTIFICATION, 32);
        data << GetID();
        data << auct->Id;
        data << uint64(auct->HighestBidder);
        data << uint32(0);
        data << uint32(outbid);
        data << auct->pItem->GetEntry();
        data << uint32(0);
        bidder->GetSession()->SendPacket(&data);
    }

    Player* owner = objmgr.GetPlayer((uint32)auct->Owner);
    if (owner != NULL && owner->IsInWorld())
    {
        WorldPacket ownerData(SMSG_AUCTION_OWNER_NOTIFICATION, 28);
        ownerData << GetID();
        ownerData << auct->Id;
        ownerData << uint32(0);
        ownerData << uint32(0);
        ownerData << uint32(0);
        ownerData << auct->pItem->GetEntry();
        ownerData << uint32(0);
        owner->GetSession()->SendPacket(&ownerData);
    }
}

void AuctionHouse::SendAuctionExpiredNotificationPacket(Auction* auct)
{
    //todo danko
    ///\todo I don't know the net code... so: TODO ;-)

    //Player* owner = objmgr.GetPlayer((uint32)auct->Owner);
    //if (owner != NULL && owner->IsInWorld())
    //{
    //  WorldPacket data(SMSG_AUCTION_REMOVED_NOTIFICATION, ??);
    //  data << GetID();
    //  data << auct->Id;
    //  data << uint32(0);   // I don't have an active blizz account..so I can't get the netcode by myself.
    //  data << uint32(0);
    //  data << uint32(0);
    //  data << auct->pItem->GetEntry();
    //  data << uint32(0);
    //  owner->GetSession()->SendPacket(&data);
    //}
}

void WorldSession::SendAuctionPlaceBidResultPacket(uint32 itemId, uint32 error)
{
    WorldPacket data(SMSG_AUCTION_COMMAND_RESULT, 16);
    data << itemId;
    data << uint32(AUCTION_BID);
    data << error;
    data << uint32(0);
    SendPacket(&data);
}

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
        //HACK: Don't know the correct error code...
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
        // Return the money to the last highest bidder.
        char subject[100];
        snprintf(subject, 100, "%u:0:0", (int)auct->pItem->GetEntry());
        sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, ah->GetID(), auct->HighestBidder, subject, "", auct->HighestBid, 0, 0, MAIL_STATIONERY_AUCTION);

        // Do not send out bid notification, when current highest bidder and new bidder are the same player..
        if (auct->HighestBidder != (uint32)_player->GetLowGUID())
            ah->SendAuctionOutBidNotificationPacket(auct, _player->GetGUID(), price);
    }

    if (auct->BuyoutPrice == price)
    {
        auct->HighestBidder = _player->GetLowGUID();
        auct->HighestBid = price;

        // we used buyout on the item.
        ah->QueueDeletion(auct, AUCTION_REMOVE_WON);

        SendAuctionPlaceBidResultPacket(auct->Id, AUCTION_ERR_NONE);
        ah->SendAuctionBuyOutNotificationPacket(auct);
    }
    else
    {
        // update most recent bid
        auct->HighestBidder = _player->GetLowGUID();
        auct->HighestBid = price;
        auct->UpdateInDB();

        SendAuctionPlaceBidResultPacket(auct->Id, AUCTION_ERR_NONE);
    }

    ah->SendAuctionList(_player, &recv_data);
}

void WorldSession::HandleCancelAuction(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 guid;
    uint32 auction_id;

    recv_data >> guid;
    recv_data >> auction_id;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature || !pCreature->auctionHouse)
        return;

    // Find Item
    Auction* auct = pCreature->auctionHouse->GetAuction(auction_id);
    if (!auct)
        return;

    pCreature->auctionHouse->QueueDeletion(auct, AUCTION_REMOVE_CANCELLED);

    // Send response packet.
    WorldPacket data(SMSG_AUCTION_COMMAND_RESULT, 8);
    data << auction_id;
    data << uint32(AUCTION_CANCEL);
    data << uint32(0);
    SendPacket(&data);

    // Re-send the owner list.
    pCreature->auctionHouse->SendOwnerListPacket(_player, 0);
}

void WorldSession::HandleAuctionSellItem(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 auctioneer;
    uint64 bid;
    uint64 buyout;
    uint32 itemsCount;
    uint32 etime;
    recv_data >> auctioneer;
    recv_data >> itemsCount;

    uint64 itemGUIDs[MAX_AUCTION_ITEMS]; // 160 slot = 4x 36 slot bag + backpack 16 slot
    uint32 count[MAX_AUCTION_ITEMS];
    
    for (uint32 i = 0; i < itemsCount; ++i)
    {
        recv_data >> itemGUIDs[i];
        recv_data >> count[i];

        if (!itemGUIDs[i] || !count[i] || count[i] > 1000)
            return;
    }

    recv_data >> bid;
    recv_data >> buyout;
    recv_data >> etime;

    if (!bid || !etime)
        return;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(auctioneer));
    if (!pCreature || !pCreature->auctionHouse)
        return;        // NPC doesn't exist or isn't an auctioneer

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

    uint32 finalCount = 0;

    for (uint32 i = 0; i < itemsCount; ++i)
    {
        Item* item = _player->GetItemInterface()->GetItemByGUID(itemGUIDs[i]);
        if (!item)
        {
            _player->SendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERR_ITEM_NOT_FOUND);
            return;
        }

        items[i] = item;
        finalCount += count[i];
    }

    if (!finalCount)
    {
        _player->SendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERR_INTERNAL_DB);
        return;
    }

    for (uint32 i = 0; i < itemsCount; ++i)
    {
        Item* item = items[i];

        if (item->GetStackCount() < finalCount)
        {
            _player->SendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERR_INTERNAL_DB);
            return;
        }
    }

    for (uint32 i = 0; i < itemsCount; ++i)
    {
        Item* item = items[i];
        uint32 auctionTime = uint32(etime);

        AuctionHouse* ah = pCreature->auctionHouse;

        uint32 item_worth = item->GetItemProperties()->SellPrice * item->GetStackCount();
        uint32 item_deposit = (uint32)(item_worth * ah->deposit_percent) * (uint32)(etime / 240.0f); // deposit is per 4 hours

        if (!_player->HasGold((uint64)item_deposit))
        {
            _player->SendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERR_NOT_ENOUGH_MONEY);
            return;
        }

        _player->TakeGold(-int32(item_deposit));

        item = _player->GetItemInterface()->SafeRemoveAndRetreiveItemByGuid(itemGUIDs[i], false);
        if (!item)
        {
            _player->SendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERR_ITEM_NOT_FOUND);
            return;
        };

        if (item->IsInWorld())
        {
            item->RemoveFromWorld();
        }

        item->SetOwner(NULL);
        item->m_isDirty = true;
        item->SaveToDB(INVENTORY_SLOT_NOT_SET, 0, true, NULL);

        // Create auction
        Auction* auct = new Auction;
        auct->BuyoutPrice = buyout;
        auct->ExpiryTime = (uint32)UNIXTIME + (etime * 60);
        auct->StartingPrice = bid;
        auct->HighestBid = 0;
        auct->HighestBidder = 0;    // hm
        auct->Id = sAuctionMgr.GenerateAuctionId();
        auct->Owner = _player->GetLowGUID();
        auct->pItem = item;
        auct->Deleted = false;
        auct->DeletedReason = 0;
        auct->DepositAmount = item_deposit;

        // Add and save auction to DB
        ah->AddAuction(auct);
        auct->SaveToDB(ah->GetID());

        // Send result packet
        _player->SendAuctionCommandResult(nullptr, AUCTION_CREATE, AUCTION_ERR_NONE);
    }

    ah->SendOwnerListPacket(_player, &recv_data);
}

void Player::SendAuctionCommandResult(Auction* auction, uint32 action, uint32 errorCode, uint32 bidError)
{
    WorldPacket data(SMSG_AUCTION_COMMAND_RESULT);
    data << uint32(auction ? auction->Id : 0);
    data << uint32(action);
    data << uint32(errorCode);

    switch (errorCode)
    {
        case AUCTION_ERR_NONE:
            if (action == AUCTION_BID)
                data << uint64(auction->HighestBid? auction->GetAuctionOutBid() : 0);
            break;
        case AUCTION_ERR_INVENTORY:
            data << uint32(bidError);
            break;
        case AUCTION_ERR_HIGHER_BID:
            data << uint64(auction->HighestBidder);
            data << uint64(auction->HighestBid);
            data << uint64(auction->HighestBid ? auction->GetAuctionOutBid() : 0);
            break;
    }

    SendPacket(&data);
}

/// the sum of outbid is (1% from current bid)*5, if bid is very small, it is 1c
uint32 Auction::GetAuctionOutBid()
{
    uint32 outbid = HighestBid * 5 / 100;
    return outbid ? outbid : 1;
}

void WorldSession::HandleAuctionListOwnerItems(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 guid;
    uint32 listfrom;

    recv_data >> guid;
    recv_data >> listfrom;      // not used in fact (this list not have page control in client)

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature || !pCreature->auctionHouse)
        return;

    pCreature->auctionHouse->SendOwnerListPacket(_player, &recv_data);
}

void AuctionHouse::SendAuctionList(Player* plr, WorldPacket* packet)
{
    std::string searchedname;
    uint8 levelmin;
    uint8 levelmax;
    uint8 usable;
    uint32 listfrom;
    uint32 auctionSlotID;
    uint32 auctionMainCategory;
    uint32 auctionSubCategory;
    uint32 quality;

    *packet >> listfrom;                // start, used for page control listing by 50 elements
    *packet >> searchedname;
    *packet >> levelmin;
    *packet >> levelmax;
    *packet >> auctionSlotID;
    *packet >> auctionMainCategory;
    *packet >> auctionSubCategory;
    *packet >> quality;
    *packet >> usable;

    packet->read_skip<uint8>();
    packet->read_skip<uint8>();

    // this block looks like it uses some lame byte packing or similar...
    for (uint8 i = 0; i < 15; ++i)
        packet->read_skip<uint8>();

    WorldPacket data(SMSG_AUCTION_LIST_RESULT, 7000);
    uint32 count = 0;
    uint32 totalcount = 0;
    data << uint32(0);

    // convert auction string to lowercase for faster parsing.
    if (searchedname.length() > 0)
    {
        for (uint32 j = 0; j < searchedname.length(); ++j)
            searchedname[j] = static_cast<char>(tolower(searchedname[j]));
    }

    auctionLock.AcquireReadLock();
    std::unordered_map<uint32, Auction*>::iterator itr = auctions.begin();
    ItemProperties const* proto;
    for (; itr != auctions.end(); ++itr)
    {
        if (itr->second->Deleted) continue;
        proto = itr->second->pItem->GetItemProperties();

        // Check the auction for parameters

        // inventory type
        if (auctionSlotID != -1 && auctionSlotID != (int32)proto->InventoryType)
            continue;

        // class
        if (auctionMainCategory != 0xffffffff && auctionMainCategory != (int32)proto->Class)
            continue;

        // subclass
        if (auctionSubCategory != 0xffffffff && auctionSubCategory != (int32)proto->SubClass)
            continue;

        // this is going to hurt. - name
        std::string proto_lower = proto->lowercase_name;
        if (searchedname.length() > 0 && !FindXinYString(searchedname, proto_lower))
            continue;

        // rarity
        if (quality != 0xffffffff && quality > (int32)proto->Quality)
            continue;

        // level range check - lower boundary
        if (levelmin && proto->RequiredLevel < levelmin)
            continue;

        // level range check - high boundary
        if (levelmax && proto->RequiredLevel > levelmax)
            continue;

        // usable check - this will hurt too :(
        if (quality)
        {
            // allowed class
            if (proto->AllowableClass && !(plr->getClassMask() & proto->AllowableClass))
                continue;

            if (proto->RequiredLevel && proto->RequiredLevel > plr->getLevel())
                continue;

            if (proto->AllowableRace && !(plr->getRaceMask() & proto->AllowableRace))
                continue;

            if (proto->Class == 4 && proto->SubClass && !(plr->GetArmorProficiency() & (((uint32)(1)) << proto->SubClass)))
                continue;

            if (proto->Class == 2 && proto->SubClass && !(plr->GetWeaponProficiency() & (((uint32)(1)) << proto->SubClass)))
                continue;

            if (proto->RequiredSkill && (!plr->_HasSkillLine(proto->RequiredSkill) || proto->RequiredSkillRank > plr->_GetSkillLineCurrent(proto->RequiredSkill, true)))
                continue;
        }

        // Add the item if no search term or if entered search term was found
        if (count < 50 && totalcount >= listfrom)
        {
            ++count;
            itr->second->BuildAuctionInfo(data);
        }

        ++totalcount;
    }

    // total count
    data.put<uint32>(0, count);
    data << uint32(totalcount);
    data << uint32(300);

    auctionLock.ReleaseReadLock();
    plr->GetSession()->SendPacket(&data);
}

void WorldSession::HandleAuctionListItems(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN;

    uint64 guid;
    recv_data >> guid;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature || !pCreature->auctionHouse)
        return;

    pCreature->auctionHouse->SendAuctionList(_player, &recv_data);
}

void WorldSession::HandleAuctionListPendingSales(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    uint64 guid;
    recv_data >> guid;

    Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
    if (!pCreature || !pCreature->auctionHouse)
        return;

    LOG_DEBUG("WORLD: Received CMSG_AUCTION_LIST_PENDING_SALES");

    WorldPacket data(SMSG_AUCTION_LIST_PENDING_SALES, 4);
    data << uint32(0);      // count
    SendPacket(&data);
}

void AuctionHouse::LoadAuctions()
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM auctions WHERE auctionhouse =%u", GetID());

    if (!result)
        return;

    Auction* auct;
    Field* fields;

    do
    {
        fields = result->Fetch();
        auct = new Auction;
        auct->Id = fields[0].GetUInt32();

        Item* pItem = objmgr.LoadItem(fields[2].GetUInt32());
        if (!pItem)
        {
            CharacterDatabase.Execute("DELETE FROM auctions WHERE auctionId=%u", auct->Id);
            delete auct;
            continue;
        }
        auct->pItem = pItem;
        auct->Owner = fields[3].GetUInt32();
        auct->StartingPrice = fields[4].GetUInt32();
        auct->BuyoutPrice = fields[5].GetUInt32();
        auct->ExpiryTime = fields[6].GetUInt32();
        auct->HighestBidder = fields[7].GetUInt32();
        auct->HighestBid = fields[8].GetUInt32();
        auct->DepositAmount = fields[9].GetUInt32();

        auct->DeletedReason = 0;
        auct->Deleted = false;

        auctions.insert(std::unordered_map<uint32, Auction*>::value_type(auct->Id, auct));
    }
    while (result->NextRow());
    delete result;
}
