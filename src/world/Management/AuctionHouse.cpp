/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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

#if VERSION_STRING < Cata
void Auction::DeleteFromDB()
{
    CharacterDatabase.WaitExecute("DELETE FROM auctions WHERE auctionId = %u", Id);
}

void Auction::SaveToDB(uint32_t AuctionHouseId)
{
    CharacterDatabase.Execute("INSERT INTO auctions VALUES(%u, %u, %u, %u, %u, %u, %u, %u, %u, %u)", Id, AuctionHouseId, pItem->getGuidLow(), Owner, StartingPrice, BuyoutPrice, ExpiryTime, HighestBidder, HighestBid, DepositAmount);
}

void Auction::UpdateInDB()
{
    CharacterDatabase.Execute("UPDATE auctions SET bidder = %u, bid = %u WHERE auctionId = %u", HighestBidder, HighestBid, Id);
}

AuctionHouse::AuctionHouse(uint32_t ID)
{
    dbc = sAuctionHouseStore.LookupEntry(ID);
    ARCEMU_ASSERT(dbc != NULL);

    cut_percent = dbc->tax / 100.0f;
    deposit_percent = dbc->fee / 100.0f;

    enabled = true;
}

AuctionHouse::~AuctionHouse()
{
    std::unordered_map<uint32_t, Auction*>::iterator itr = auctions.begin();
    for (; itr != auctions.end(); ++itr)
        delete itr->second;
}

void AuctionHouse::QueueDeletion(Auction* auct, uint32_t Reason)
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

    uint32_t t = (uint32_t)UNIXTIME;
    std::unordered_map<uint32_t, Auction*>::iterator itr = auctions.begin();
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
    auctions.insert(std::unordered_map<uint32_t, Auction*>::value_type(auct->Id, auct));
    auctionLock.ReleaseWriteLock();

    LOG_DEBUG("%u: Add auction %u, expire@ %u.", dbc->id, auct->Id, auct->ExpiryTime);
}

Auction* AuctionHouse::GetAuction(uint32_t Id)
{
    Auction* ret;
    std::unordered_map<uint32_t, Auction*>::iterator itr;
    auctionLock.AcquireReadLock();
    itr = auctions.find(Id);
    ret = (itr == auctions.end()) ? 0 : itr->second;
    auctionLock.ReleaseReadLock();
    return ret;
}

void AuctionHouse::RemoveAuction(Auction* auct)
{
    LOG_DEBUG("%u: Removing auction %u, reason %u.", dbc->id, auct->Id, auct->DeletedReason);

    char subject[100];
    char body[200];
    switch (auct->DeletedReason)
    {
        case AUCTION_REMOVE_EXPIRED:
        {
            // ItemEntry:0:3
            snprintf(subject, 100, "%u:0:3", (unsigned int)auct->pItem->getEntry());

            // Auction expired, resend item, no money to owner.
            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, dbc->id, auct->Owner, subject, "", 0, 0, auct->pItem->getGuid(), MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);
        }
        break;

        case AUCTION_REMOVE_WON:
        {
            // ItemEntry:0:1
            snprintf(subject, 100, "%u:0:1", (unsigned int)auct->pItem->getEntry());

            // <owner player guid>:bid:buyout
            snprintf(body, 200, "%X:%u:%u", (unsigned int)auct->Owner, (unsigned int)auct->HighestBid, (unsigned int)auct->BuyoutPrice);

            // Auction won by highest bidder. He gets the item.
            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, dbc->id, auct->HighestBidder, subject, body, 0, 0, auct->pItem->getGuid(), MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);

            // Send a mail to the owner with his cut of the price.
            uint32_t auction_cut = float2int32(cut_percent * auct->HighestBid);
            int32_t amount = auct->HighestBid - auction_cut + auct->DepositAmount;
            if (amount < 0)
                amount = 0;

            // ItemEntry:0:2
            snprintf(subject, 100, "%u:0:2", (unsigned int)auct->pItem->getEntry());

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
            snprintf(subject, 100, "%u:0:5", (unsigned int)auct->pItem->getEntry());
            uint32_t cut = float2int32(cut_percent * auct->HighestBid);
            Player* plr = sObjectMgr.GetPlayer(auct->Owner);
            if (cut && plr && plr->hasEnoughCoinage(cut))
                plr->modCoinage(-(int32_t)cut);

            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, GetID(), auct->Owner, subject, "", 0, 0, auct->pItem->getGuid(), MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);

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

void Auction::AddToPacket(WorldPacket& data)
{
    data << uint32_t(Id);
    data << uint32_t(pItem->getEntry());

    for (uint8_t i = 0; i < MAX_INSPECTED_ENCHANTMENT_SLOT; i++)
    {
        data << uint32_t(pItem->getEnchantmentId(i));              // Enchantment ID
        data << uint32_t(pItem->GetEnchantmentApplytime(i));       // Unknown / maybe ApplyTime
        data << uint32_t(pItem->getEnchantmentCharges(i));         // charges
    }

    data << pItem->getRandomPropertiesId();                        // -ItemRandomSuffix / random property : If the value is negative its ItemRandomSuffix if its possitive its RandomItemProperty
    data << pItem->getPropertySeed();                              // when ItemRandomSuffix is used this is the modifier

    /******************** ItemRandomSuffix***************************
    * For what I have seen ItemRandomSuffix is like RandomItemProperty
    * The only difference is has is that it has a modifier.
    * That is the result of jewelcrafting, the effect is that the
    * enchantment is variable. That means that a enchantment can be +1 and
    * with more Jem's +12 or so.
    * Description for lookup: You get the enchantmentSuffixID and search the
    * DBC for the last 1 - 3 value's(depending on the enchantment).
    * That value is what I call EnchantmentValue. You guys might find a
    * better name but for now its good enough. The formula to calculate
    * The ingame "GAIN" is:
    * (Modifier / 10000) * enchantmentvalue = EnchantmentGain;
    */

    data << pItem->getStackCount();                                // Amount
    data << pItem->GetChargesLeft();                               // Charges Left
    data << uint32_t(0);                                           // Unknown
    data << uint64_t(Owner);                                       // Owner guid
    data << uint32_t(StartingPrice);                               // Starting bid
    // If there's no bid yet, we should start at starting bid
    data << uint32_t((HighestBid > 0 ? 50 : 0));                   // Next bid value modifier, like current bid + this value
    data << uint32_t(BuyoutPrice);                                 // Buyout
    data << uint32_t((ExpiryTime - UNIXTIME) * 1000);              // Time left
    data << uint64_t(HighestBidder);                               // Last bidder
    data << uint32_t(HighestBid);                                  // The bid of the last bidder

}

void AuctionHouse::SendBidListPacket(Player* plr, WorldPacket* /*packet*/)
{
    uint32_t count = 0;

    WorldPacket data(SMSG_AUCTION_BIDDER_LIST_RESULT, 1024);
    data << uint32_t(0); // Placeholder

    Auction* auct;
    auctionLock.AcquireReadLock();
    std::unordered_map<uint32_t, Auction*>::iterator itr = auctions.begin();
    for (; itr != auctions.end(); ++itr)
    {
        auct = itr->second;
        if (auct->HighestBidder == plr->getGuid())
        {
            if (auct->Deleted) continue;

            auct->AddToPacket(data);
            (*(uint32_t*)&data.contents()[0])++;
            ++count;
        }
    }

    data << count;
    auctionLock.ReleaseReadLock();
    plr->GetSession()->SendPacket(&data);
}

void AuctionHouse::UpdateOwner(uint32_t oldGuid, uint32_t newGuid)
{
    auctionLock.AcquireWriteLock();
    std::unordered_map<uint32_t, Auction*>::iterator itr = auctions.begin();
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

//\todo remove unused function member.
void AuctionHouse::SendOwnerListPacket(Player* plr, WorldPacket* /*packet*/)
{
    uint32_t count = 0;

    WorldPacket data(SMSG_AUCTION_OWNER_LIST_RESULT, 1024);
    data << uint32_t(0);                                          // Placeholder

    Auction* auct;
    auctionLock.AcquireReadLock();
    std::unordered_map<uint32_t, Auction*>::iterator itr = auctions.begin();
    for (; itr != auctions.end(); ++itr)
    {
        auct = itr->second;
        if (auct->Owner == plr->getGuid())
        {
            if (auct->Deleted) continue;

            auct->AddToPacket(data);
            (*(uint32_t*)&data.contents()[0])++;
            ++count;
        }
    }
    data << count;
    auctionLock.ReleaseReadLock();
    plr->GetSession()->SendPacket(&data);
}

void AuctionHouse::SendAuctionOutBidNotificationPacket(Auction* auct, uint64_t newBidder, uint32_t newHighestBid)
{
    Player* bidder = sObjectMgr.GetPlayer(auct->HighestBidder);
    if (bidder != NULL && bidder->IsInWorld())
    {
        uint32_t outbid = (auct->HighestBid / 100) * 5;
        if (!outbid)
            outbid = 1;

        ///\todo Check this code, when a user has been bid out by instant buy out
        WorldPacket data(SMSG_AUCTION_BIDDER_NOTIFICATION, 32);
        data << GetID();
        data << auct->Id;
        data << uint64_t(newBidder);
        data << uint32_t(newHighestBid);
        data << uint32_t(outbid);
        data << auct->pItem->getEntry();
        data << uint32_t(0);
        bidder->GetSession()->SendPacket(&data);
    }
}

void AuctionHouse::SendAuctionBuyOutNotificationPacket(Auction* auct)
{
    Player* bidder = sObjectMgr.GetPlayer((uint32_t)auct->HighestBidder);
    if (bidder != NULL && bidder->IsInWorld())
    {
        uint32_t outbid = (auct->HighestBid / 100) * 5;
        if (!outbid)
            outbid = 1;

        WorldPacket data(SMSG_AUCTION_BIDDER_NOTIFICATION, 32);
        data << GetID();
        data << auct->Id;
        data << uint64_t(auct->HighestBidder);
        data << uint32_t(0);
        data << uint32_t(outbid);
        data << auct->pItem->getEntry();
        data << uint32_t(0);
        bidder->GetSession()->SendPacket(&data);
    }

    Player* owner = sObjectMgr.GetPlayer((uint32_t)auct->Owner);
    if (owner != NULL && owner->IsInWorld())
    {
        WorldPacket ownerData(SMSG_AUCTION_OWNER_NOTIFICATION, 28);
        ownerData << GetID();
        ownerData << auct->Id;
        ownerData << uint32_t(0);
        ownerData << uint32_t(0);
        ownerData << uint32_t(0);
        ownerData << auct->pItem->getEntry();
        ownerData << uint32_t(0);
        owner->GetSession()->SendPacket(&ownerData);
    }
}

void AuctionHouse::SendAuctionExpiredNotificationPacket(Auction* /*auct*/)
{
    ///\todo I don't know the net code... so: TODO ;-)

    //Player* owner = sObjectMgr.GetPlayer((uint32_t)auct->Owner);
    //if (owner != NULL && owner->IsInWorld())
    //{
    //  WorldPacket data(SMSG_AUCTION_REMOVED_NOTIFICATION, ??);
    //  data << GetID();
    //  data << auct->Id;
    //  data << uint32_t(0); // I don't have an active blizz account..so I can't get the netcode by myself.
    //  data << uint32_t(0);
    //  data << uint32_t(0);
    //  data << auct->pItem->getEntry();
    //  data << uint32_t(0);
    //  owner->GetSession()->SendPacket(&data);
    //}
}

//\todo: eeeeeeek packet read outside of the packet class o.O
void AuctionHouse::SendAuctionList(Player* plr, WorldPacket* packet)
{
    uint32_t start_index, current_index = 0;
    uint32_t counted_items = 0;
    std::string auctionString;
    uint8_t levelRange1, levelRange2, usableCheck;
    int32_t inventory_type, itemclass, itemsubclass, rarityCheck;

    *packet >> start_index;
    *packet >> auctionString;
    *packet >> levelRange1 >> levelRange2;
    *packet >> inventory_type >> itemclass >> itemsubclass;
    *packet >> rarityCheck >> usableCheck;

    // convert auction string to lowercase for faster parsing.
    if (auctionString.length() > 0)
    {
        for (uint32_t j = 0; j < auctionString.length(); ++j)
            auctionString[j] = static_cast<char>(tolower(auctionString[j]));
    }

    WorldPacket data(SMSG_AUCTION_LIST_RESULT, 7000);
    data << uint32_t(0); // count of items

    auctionLock.AcquireReadLock();
    std::unordered_map<uint32_t, Auction*>::iterator itr = auctions.begin();
    ItemProperties const* proto;
    for (; itr != auctions.end(); ++itr)
    {
        if (itr->second->Deleted) continue;
        proto = itr->second->pItem->getItemProperties();

        // Check the auction for parameters

        // inventory type
        if (inventory_type != -1 && inventory_type != (int32_t)proto->InventoryType)
            continue;

        // class
        if (itemclass != -1 && itemclass != (int32_t)proto->Class)
            continue;

        // subclass
        if (itemsubclass != -1 && itemsubclass != (int32_t)proto->SubClass)
            continue;

        // this is going to hurt. - name
        std::string proto_lower = proto->lowercase_name;
        if (auctionString.length() > 0 && Util::findXinYString(auctionString, proto_lower) == false)
            continue;

        // rarity
        if (rarityCheck != -1 && rarityCheck > (int32_t)proto->Quality)
            continue;

        // level range check - lower boundary
        if (levelRange1 && proto->RequiredLevel < levelRange1)
            continue;

        // level range check - high boundary
        if (levelRange2 && proto->RequiredLevel > levelRange2)
            continue;

        // usable check - this will hurt too :(
        if (usableCheck)
        {
            // allowed class
            if (proto->AllowableClass && !(plr->getClassMask() & proto->AllowableClass))
                continue;

            if (proto->RequiredLevel && proto->RequiredLevel > plr->getLevel())
                continue;

            if (proto->AllowableRace && !(plr->getRaceMask() & proto->AllowableRace))
                continue;

            if (proto->Class == 4 && proto->SubClass && !(plr->GetArmorProficiency() & (((uint32_t)(1)) << proto->SubClass)))
                continue;

            if (proto->Class == 2 && proto->SubClass && !(plr->GetWeaponProficiency() & (((uint32_t)(1)) << proto->SubClass)))
                continue;

            if (proto->RequiredSkill && (!plr->_HasSkillLine(proto->RequiredSkill) || proto->RequiredSkillRank > plr->_GetSkillLineCurrent(proto->RequiredSkill, true)))
                continue;
        }

        // Page system.
        ++counted_items;
        if (counted_items >= start_index + 50)
            continue;
        current_index++;
        if (start_index && current_index < start_index) continue;

        // all checks passed -> add to packet.
        itr->second->AddToPacket(data);
        (*(uint32_t*)&data.contents()[0])++;
    }

    // total count
    data << uint32_t(1 + counted_items);
    data << uint32_t(300);

    auctionLock.ReleaseReadLock();
    plr->GetSession()->SendPacket(&data);
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

        Item* pItem = sObjectMgr.LoadItem(fields[2].GetUInt32());
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

        auctions.insert(std::unordered_map<uint32_t, Auction*>::value_type(auct->Id, auct));
    }
    while (result->NextRow());
    delete result;
}

#else
void Auction::DeleteFromDB()
{
    CharacterDatabase.WaitExecute("DELETE FROM auctions WHERE auctionId = %u", Id);
}

void Auction::SaveToDB(uint32_t AuctionHouseId)
{
    CharacterDatabase.Execute("INSERT INTO auctions VALUES(%u, %u, %u, %u, %u, %u, %u, %u, %u, %u)", Id, AuctionHouseId, pItem->getGuidLow(), Owner, StartingPrice, BuyoutPrice, ExpiryTime, HighestBidder, HighestBid, DepositAmount);
}

void Auction::UpdateInDB()
{
    CharacterDatabase.Execute("UPDATE auctions SET bidder = %u, bid = %u WHERE auctionId = %u", HighestBidder, HighestBid, Id);
}

AuctionHouse::AuctionHouse(uint32_t ID)
{
    dbc = sAuctionHouseStore.LookupEntry(ID);
    ARCEMU_ASSERT(dbc != NULL);

    cut_percent = dbc->tax / 100.0f;
    deposit_percent = dbc->fee / 100.0f;

    enabled = true;
}

AuctionHouse::~AuctionHouse()
{
    std::unordered_map<uint32_t, Auction*>::iterator itr = auctions.begin();
    for (; itr != auctions.end(); ++itr)
        delete itr->second;
}

void AuctionHouse::QueueDeletion(Auction* auct, uint32_t Reason)
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

    uint32_t t = (uint32_t)UNIXTIME;
    std::unordered_map<uint32_t, Auction*>::iterator itr = auctions.begin();
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
    auctions.insert(std::unordered_map<uint32_t, Auction*>::value_type(auct->Id, auct));
    auctionLock.ReleaseWriteLock();

    LogDebug("AuctionHouse : %u: Add auction %u, expire@ %u.", dbc->id, auct->Id, auct->ExpiryTime);
}

Auction* AuctionHouse::GetAuction(uint32_t Id)
{
    Auction* ret;
    std::unordered_map<uint32_t, Auction*>::iterator itr;
    auctionLock.AcquireReadLock();
    itr = auctions.find(Id);
    ret = (itr == auctions.end()) ? 0 : itr->second;
    auctionLock.ReleaseReadLock();
    return ret;
}

void AuctionHouse::RemoveAuction(Auction* auct)
{
    LogDebug("AuctionHouse : %u: Removing auction %u, reason %u.", dbc->id, auct->Id, auct->DeletedReason);

    char subject[100];
    char body[200];
    switch (auct->DeletedReason)
    {
        case AUCTION_REMOVE_EXPIRED:
        {
            // ItemEntry:0:3
            snprintf(subject, 100, "%u:0:3", (unsigned int)auct->pItem->getEntry());

            // Auction expired, resend item, no money to owner.
            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, dbc->id, auct->Owner, subject, "", 0, 0, auct->pItem->getGuid(), MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);
        }
        break;

        case AUCTION_REMOVE_WON:
        {
            // ItemEntry:0:1
            snprintf(subject, 100, "%u:0:1", (unsigned int)auct->pItem->getEntry());

            // <owner player guid>:bid:buyout
            snprintf(body, 200, "%X:%u:%u", (unsigned int)auct->Owner, (unsigned int)auct->HighestBid, (unsigned int)auct->BuyoutPrice);

            // Auction won by highest bidder. He gets the item.
            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, dbc->id, auct->HighestBidder, subject, body, 0, 0, auct->pItem->getGuid(), MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);

            // Send a mail to the owner with his cut of the price.
            uint32_t auction_cut = float2int32(cut_percent * auct->HighestBid);
            int32_t amount = auct->HighestBid - auction_cut + auct->DepositAmount;
            if (amount < 0)
                amount = 0;

            // ItemEntry:0:2
            snprintf(subject, 100, "%u:0:2", (unsigned int)auct->pItem->getEntry());

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
            snprintf(subject, 100, "%u:0:5", (unsigned int)auct->pItem->getEntry());
            uint32_t cut = float2int32(cut_percent * auct->HighestBid);
            Player* plr = sObjectMgr.GetPlayer(auct->Owner);
            if (cut && plr && plr->hasEnoughCoinage(cut))
                plr->modCoinage(-(int32_t)cut);

            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, GetID(), auct->Owner, subject, "", 0, 0, auct->pItem->getGuid(), MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);

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

void AuctionHouse::SendBidListPacket(Player* plr, WorldPacket* /*packet*/)
{
    uint32_t count = 0;
    uint32_t totalcount = 0;

    WorldPacket data(SMSG_AUCTION_BIDDER_LIST_RESULT, 4 + 4 + 4);
    data << uint32_t(0); // Placeholder

    auctionLock.AcquireReadLock();
    std::unordered_map<uint32_t, Auction*>::iterator itr = auctions.begin();
    for (; itr != auctions.end(); ++itr)
    {
        Auction* auct = itr->second;
        if (auct->HighestBidder == plr->getGuid())
        {
            if (auct->Deleted) continue;

            if (auct->BuildAuctionInfo(data))
            {
                ++count;
                ++totalcount;
            }
        }
    }

    data.put<uint32_t>(0, count); // add count to placeholder
    data << totalcount;
    data << uint32_t(300); //unk 2.3.0
    auctionLock.ReleaseReadLock();
    plr->GetSession()->SendPacket(&data);
}

void AuctionHouse::UpdateOwner(uint32_t oldGuid, uint32_t newGuid)
{
    auctionLock.AcquireWriteLock();
    std::unordered_map<uint32_t, Auction*>::iterator itr = auctions.begin();
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

void AuctionHouse::SendOwnerListPacket(Player* plr, WorldPacket* /*packet*/)
{
    uint32_t count = 0;
    uint32_t totalcount = 0;

    WorldPacket data(SMSG_AUCTION_OWNER_LIST_RESULT, 4 + 4 + 4);
    data << uint32_t(0); // Placeholder

    Auction* auct;
    auctionLock.AcquireReadLock();
    std::unordered_map<uint32_t, Auction*>::iterator itr = auctions.begin();
    for (; itr != auctions.end(); ++itr)
    {
        auct = itr->second;
        if (auct->Owner == plr->getGuid())
        {
            if (auct->Deleted)
                continue;

            if (auct->BuildAuctionInfo(data))
                ++count;

            ++totalcount;
        }
    }

    data.put<uint32_t>(0, count);
    data << uint32_t(totalcount);
    data << uint32_t(0);
    auctionLock.ReleaseReadLock();
    plr->GetSession()->SendPacket(&data);
}

bool Auction::BuildAuctionInfo(WorldPacket& data)
{
    if (!pItem)
    {
        LOG_ERROR("Auction %u has a non-existent item: %u", Id, pItem->getEntry());
        return false;
    }
    data << uint32_t(Id);
    data << uint32_t(pItem->getEntry());

    for (uint8_t i = 0; i < PROP_ENCHANTMENT_SLOT_0; ++i) // PROP_ENCHANTMENT_SLOT_0 = 10
    {
        data << uint32_t(pItem->getEnchantmentId(EnchantmentSlot(i)));
        data << uint32_t(pItem->getEnchantmentDuration(EnchantmentSlot(i)));
        data << uint32_t(pItem->getEnchantmentCharges(EnchantmentSlot(i)));
    }

    data << int32_t(pItem->getRandomPropertiesId());                  // Random item property id
    data << uint32_t(pItem->getPropertySeed());                       // SuffixFactor
    data << uint32_t(pItem->getStackCount());                         // item->count
    data << uint32_t(pItem->GetChargesLeft());                        // item->charge FFFFFFF
    data << uint32_t(0);                                              // Unknown
    data << uint64_t(Owner);                                          // Auction->owner
    data << uint64_t(StartingPrice);                                  // Auction->startbid (not sure if useful)
    data << uint64_t(HighestBid ? GetAuctionOutBid() : 0);
    // Minimal outbid
    data << uint64_t(BuyoutPrice);                                    // Auction->buyout
    data << uint32_t((ExpiryTime - time(NULL)) * IN_MILLISECONDS);    // time left
    data << uint64_t(HighestBidder);                                  // auction->bidder current
    data << uint64_t(HighestBid);                                     // current bid
    return true;
}

void AuctionHouse::SendAuctionOutBidNotificationPacket(Auction* auct, uint64_t newBidder, uint32_t newHighestBid)
{
    Player* bidder = sObjectMgr.GetPlayer(auct->HighestBidder);
    if (bidder != NULL && bidder->IsInWorld())
    {
        uint32_t outbid = (auct->HighestBid / 100) * 5;
        if (!outbid)
            outbid = 1;

        ///\todo Check this code, when a user has been bid out by instant buy out
        WorldPacket data(SMSG_AUCTION_BIDDER_NOTIFICATION, 32);
        data << GetID();
        data << auct->Id;
        data << uint64_t(newBidder);
        data << uint32_t(newHighestBid);
        data << uint32_t(outbid);
        data << auct->pItem->getEntry();
        data << uint32_t(0);
        bidder->GetSession()->SendPacket(&data);
    }
}

void AuctionHouse::SendAuctionBuyOutNotificationPacket(Auction* auct)
{
    Player* bidder = sObjectMgr.GetPlayer((uint32_t)auct->HighestBidder);
    if (bidder != NULL && bidder->IsInWorld())
    {
        uint32_t outbid = (auct->HighestBid / 100) * 5;
        if (!outbid)
            outbid = 1;

        WorldPacket data(SMSG_AUCTION_BIDDER_NOTIFICATION, 32);
        data << GetID();
        data << auct->Id;
        data << uint64_t(auct->HighestBidder);
        data << uint32_t(0);
        data << uint32_t(outbid);
        data << auct->pItem->getEntry();
        data << uint32_t(0);
        bidder->GetSession()->SendPacket(&data);
    }

    Player* owner = sObjectMgr.GetPlayer((uint32_t)auct->Owner);
    if (owner != NULL && owner->IsInWorld())
    {
        WorldPacket ownerData(SMSG_AUCTION_OWNER_NOTIFICATION, 28);
        ownerData << GetID();
        ownerData << auct->Id;
        ownerData << uint32_t(0);
        ownerData << uint32_t(0);
        ownerData << uint32_t(0);
        ownerData << auct->pItem->getEntry();
        ownerData << uint32_t(0);
        owner->GetSession()->SendPacket(&ownerData);
    }
}

void AuctionHouse::SendAuctionExpiredNotificationPacket(Auction* /*auct*/)
{
    //todo danko
    ///\todo I don't know the net code... so: TODO ;-)

    //Player* owner = sObjectMgr.GetPlayer((uint32_t)auct->Owner);
    //if (owner != NULL && owner->IsInWorld())
    //{
    //  WorldPacket data(SMSG_AUCTION_REMOVED_NOTIFICATION, ??);
    //  data << GetID();
    //  data << auct->Id;
    //  data << uint32_t(0); // I don't have an active blizz account..so I can't get the netcode by myself.
    //  data << uint32_t(0);
    //  data << uint32_t(0);
    //  data << auct->pItem->getEntry();
    //  data << uint32_t(0);
    //  owner->GetSession()->SendPacket(&data);
    //}
}

/// the sum of outbid is (1% from current bid)*5, if bid is very small, it is 1c
uint32_t Auction::GetAuctionOutBid()
{
    uint32_t outbid = HighestBid * 5 / 100;
    return outbid ? outbid : 1;
}

void AuctionHouse::SendAuctionList(Player* plr, WorldPacket* packet)
{
    std::string searchedname;
    uint8_t levelmin;
    uint8_t levelmax;
    uint8_t usable;
    uint32_t listfrom;
    uint32_t auctionSlotID;
    uint32_t auctionMainCategory;
    uint32_t auctionSubCategory;
    uint32_t quality;

    *packet >> listfrom; // start, used for page control listing by 50 elements
    *packet >> searchedname;
    *packet >> levelmin;
    *packet >> levelmax;
    *packet >> auctionSlotID;
    *packet >> auctionMainCategory;
    *packet >> auctionSubCategory;
    *packet >> quality;
    *packet >> usable;

    packet->read_skip<uint8_t>();
    packet->read_skip<uint8_t>();

    // this block looks like it uses some lame byte packing or similar...
    for (uint8_t i = 0; i < 15; ++i)
        packet->read_skip<uint8_t>();

    WorldPacket data(SMSG_AUCTION_LIST_RESULT, 7000);
    uint32_t count = 0;
    uint32_t totalcount = 0;
    data << uint32_t(0);

    // convert auction string to lowercase for faster parsing.
    if (searchedname.length() > 0)
    {
        for (uint32_t j = 0; j < searchedname.length(); ++j)
            searchedname[j] = static_cast<char>(tolower(searchedname[j]));
    }

    auctionLock.AcquireReadLock();
    std::unordered_map<uint32_t, Auction*>::iterator itr = auctions.begin();
    ItemProperties const* proto;
    for (; itr != auctions.end(); ++itr)
    {
        if (itr->second->Deleted) continue;
        proto = itr->second->pItem->getItemProperties();

        // Check the auction for parameters

        // inventory type
        if (auctionSlotID != -1 && auctionSlotID != (int32_t)proto->InventoryType)
            continue;

        // class
        if (auctionMainCategory != 0xffffffff && auctionMainCategory != (int32_t)proto->Class)
            continue;

        // subclass
        if (auctionSubCategory != 0xffffffff && auctionSubCategory != (int32_t)proto->SubClass)
            continue;

        // this is going to hurt. - name
        std::string proto_lower = proto->lowercase_name;
        if (searchedname.length() > 0 && Util::findXinYString(searchedname, proto_lower) == false)
            continue;

        // rarity
        if (quality != 0xffffffff && quality > (int32_t)proto->Quality)
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

            if (proto->Class == 4 && proto->SubClass && !(plr->GetArmorProficiency() & (((uint32_t)(1)) << proto->SubClass)))
                continue;

            if (proto->Class == 2 && proto->SubClass && !(plr->GetWeaponProficiency() & (((uint32_t)(1)) << proto->SubClass)))
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
    data.put<uint32_t>(0, count);
    data << uint32_t(totalcount);
    data << uint32_t(300);

    auctionLock.ReleaseReadLock();
    plr->GetSession()->SendPacket(&data);
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

        Item* pItem = sObjectMgr.LoadItem(fields[2].GetUInt32());
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

        auctions.insert(std::unordered_map<uint32_t, Auction*>::value_type(auct->Id, auct));
    }
    while (result->NextRow());
    delete result;
}
#endif
