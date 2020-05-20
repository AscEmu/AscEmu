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

void Auction::DeleteFromDB()
{
    CharacterDatabase.WaitExecute("DELETE FROM auctions WHERE auctionId = %u", Id);
}

void Auction::SaveToDB(uint32 AuctionHouseId)
{
    CharacterDatabase.Execute("INSERT INTO auctions VALUES(%u, %u, %u, %u, %u, %u, %u, %u, %u, %u)", Id, AuctionHouseId, pItem->getGuidLow(), Owner, StartingPrice, BuyoutPrice, ExpiryTime, HighestBidder, HighestBid, DepositAmount);
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
    for (auto itr = auctions.begin(); itr != auctions.end(); ++itr)
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

    for (auto auct : removalList)
    {
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

    auto const time = static_cast<uint32>(UNIXTIME);
    for (auto itr = auctions.begin(); itr != auctions.end();)
    {
        auto auction = itr->second;
        ++itr;

        if (time >= auction->ExpiryTime)
        {
            if (auction->HighestBidder == 0)
            {
                auction->DeletedReason = AUCTION_REMOVE_EXPIRED;
                this->SendAuctionExpiredNotificationPacket(auction);
            }
            else
            {
                auction->DeletedReason = AUCTION_REMOVE_WON;
            }

            auction->Deleted = true;
            removalList.push_back(auction);
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

    LogDebug("AuctionHouse : %u: Add auction %u, expire@ %u.", dbc->id, auct->Id, auct->ExpiryTime);
}

Auction* AuctionHouse::GetAuction(uint32 Id)
{
    auctionLock.AcquireReadLock();
    const auto auctionsMap = auctions.find(Id);
    const auto auction = auctionsMap == auctions.end() ? nullptr : auctionsMap->second;
    auctionLock.ReleaseReadLock();

    return auction;
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
            const uint32 auction_cut = float2int32(cut_percent * auct->HighestBid);
            int32 amount = auct->HighestBid - auction_cut + auct->DepositAmount;
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
            const uint32 cut = float2int32(cut_percent * auct->HighestBid);
            Player* plr = sObjectMgr.GetPlayer(auct->Owner);
            if (cut && plr && plr->hasEnoughCoinage(cut))
                plr->modCoinage(-(int32)cut);

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

void AuctionHouse::UpdateOwner(uint32 oldGuid, uint32 newGuid)
{
    auctionLock.AcquireWriteLock();
    for (auto& itr : auctions)
    {
        auto auction = itr.second;
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

void AuctionHouse::SendAuctionOutBidNotificationPacket(Auction* auct, uint64 newBidder, uint32 newHighestBid)
{
    Player* bidder = sObjectMgr.GetPlayer(auct->HighestBidder);
    if (bidder && bidder->IsInWorld())
    {
        uint32 outbid = (auct->HighestBid / 100) * 5;
        if (!outbid)
            outbid = 1;

        WorldPacket data(SMSG_AUCTION_BIDDER_NOTIFICATION, 32);
        data << GetID();
        data << auct->Id;
        data << uint64(newBidder);
        data << uint32(newHighestBid);
        data << uint32(outbid);
        data << auct->pItem->getEntry();
        data << uint32(0);
        bidder->GetSession()->SendPacket(&data);
    }
}

void AuctionHouse::SendAuctionBuyOutNotificationPacket(Auction* auct)
{
    Player* bidder = sObjectMgr.GetPlayer(static_cast<uint32>(auct->HighestBidder));
    if (bidder && bidder->IsInWorld())
    {
        uint32 outbid = (auct->HighestBid / 100) * 5;
        if (!outbid)
            outbid = 1;

        WorldPacket data(SMSG_AUCTION_BIDDER_NOTIFICATION, 32);
        data << GetID();
        data << auct->Id;
        data << uint64(auct->HighestBidder);
#if VERSION_STRING < Cata
        data << uint32(0);
        data << uint32(outbid);
#else
        data << uint64(0);
        data << uint64(outbid);
#endif
        data << auct->pItem->getEntry();
        data << uint32(0);
        bidder->GetSession()->SendPacket(&data);
    }

    Player* owner = sObjectMgr.GetPlayer(static_cast<uint32>(auct->Owner));
    if (owner && owner->IsInWorld())
    {
        WorldPacket ownerData(SMSG_AUCTION_OWNER_NOTIFICATION, 28);
        ownerData << auct->Id;
#if VERSION_STRING < Cata
        ownerData << uint32(auct->HighestBid);
        ownerData << uint32(0);
        ownerData << uint32(0);
#else
        ownerData << uint64(auct->HighestBid);
        ownerData << uint64(0);
        ownerData << uint64(0);
#endif
        ownerData << auct->pItem->getEntry();
        ownerData << uint32(0);
 #if VERSION_STRING > TBC
        ownerData << float(0);
#endif
        owner->GetSession()->SendPacket(&ownerData);
    }
}

void AuctionHouse::SendAuctionExpiredNotificationPacket(Auction* /*auct*/)
{
    ///\todo I don't know the net code... so: TODO ;-)

    //Player* owner = sObjectMgr.GetPlayer((uint32)auct->Owner);
    //if (owner && owner->IsInWorld())
    //{
    //  WorldPacket data(SMSG_AUCTION_REMOVED_NOTIFICATION, ??);
    //  data << GetID();
    //  data << auct->Id;
    //  data << uint32(0);   // I don't have an active blizz account..so I can't get the netcode by myself.
    //  data << uint32(0);
    //  data << uint32(0);
    //  data << auct->pItem->getEntry();
    //  data << uint32(0);
    //  owner->GetSession()->SendPacket(&data);
    //}
}

void AuctionHouse::LoadAuctions()
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM auctions WHERE auctionhouse =%u", GetID());
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        auto auction = new Auction;
        auction->Id = fields[0].GetUInt32();

        Item* pItem = sObjectMgr.LoadItem(fields[2].GetUInt32());
        if (!pItem)
        {
            CharacterDatabase.Execute("DELETE FROM auctions WHERE auctionId=%u", auction->Id);
            delete auction;
            continue;
        }

        auction->pItem = pItem;
        auction->Owner = fields[3].GetUInt32();
        auction->StartingPrice = fields[4].GetUInt32();
        auction->BuyoutPrice = fields[5].GetUInt32();
        auction->ExpiryTime = fields[6].GetUInt32();
        auction->HighestBidder = fields[7].GetUInt32();
        auction->HighestBid = fields[8].GetUInt32();
        auction->DepositAmount = fields[9].GetUInt32();

        auction->DeletedReason = 0;
        auction->Deleted = false;

        auctions.insert(std::unordered_map<uint32, Auction*>::value_type(auction->Id, auction));
    }
    while (result->NextRow());
    delete result;
}

void AuctionHouse::SendAuctionList(Player* plr, AscEmu::Packets::CmsgAuctionListItems srlPacket)
{
    uint32_t count = 0;
    uint32_t totalcount = 0;

    // convert auction string to lowercase for faster parsing.
    if (srlPacket.searchedName.length() > 0)
    {
        for (uint32_t j = 0; j < srlPacket.searchedName.length(); ++j)
            srlPacket.searchedName[j] = static_cast<char>(tolower(srlPacket.searchedName[j]));
    }

    WorldPacket data(SMSG_AUCTION_LIST_RESULT, 7000);
    data << uint32_t(0); // count of items

    auctionLock.AcquireReadLock();
    for (auto& auction : auctions)
    {
        if (auction.second->Deleted)
            continue;

        ItemProperties const* proto = auction.second->pItem->getItemProperties();

        // Check the auction for parameters

        // inventory type
        if (srlPacket.auctionSlotId != 0xffffffff && srlPacket.auctionSlotId != proto->InventoryType)
            continue;

        // class
        if (srlPacket.auctionMainCategory != 0xffffffff && srlPacket.auctionMainCategory != proto->Class)
            continue;

        // subclass
        if (srlPacket.auctionSubCategory != 0xffffffff && srlPacket.auctionSubCategory != proto->SubClass)
            continue;

        // this is going to hurt. - name
        std::string proto_lower = proto->lowercase_name;
        if (srlPacket.searchedName.length() > 0 && !Util::findXinYString(srlPacket.searchedName, proto_lower))
            continue;

        // rarity
        if (srlPacket.quality != 0xffffffff && srlPacket.quality > proto->Quality)
            continue;

        // level range check - lower boundary
        if (srlPacket.levelMin && proto->RequiredLevel < srlPacket.levelMin)
            continue;

        // level range check - high boundary
        if (srlPacket.levelMax && proto->RequiredLevel > srlPacket.levelMax)
            continue;

        // usable check - this will hurt too :(
        if (srlPacket.usable)
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

        if (count < 50 && totalcount >= srlPacket.listFrom)
        {
            ++count;
            auction.second->AddToPacket(data);
        }

        ++totalcount;
    }

    // total count
    data.put<uint32>(0, count);
    data << uint32_t(totalcount);
    data << uint32_t(300);

    auctionLock.ReleaseReadLock();
    plr->GetSession()->SendPacket(&data);
}

void Auction::AddToPacket(WorldPacket& data)
{
    data << uint32(Id);
    data << uint32(pItem->getEntry());

    for (uint8 i = 0; i < MAX_INSPECTED_ENCHANTMENT_SLOT; i++)
    {
        data << uint32(pItem->getEnchantmentId(i));
        data << uint32(pItem->getEnchantmentDuration(i));
        data << uint32(pItem->getEnchantmentCharges(i));
    }

    data << pItem->getRandomPropertiesId();
    data << pItem->getPropertySeed();
    data << pItem->getStackCount();
    data << pItem->GetChargesLeft();
    data << uint32(0);
    data << uint64(Owner);

#if VERSION_STRING < Cata
    data << uint32(StartingPrice);
    data << uint32(HighestBid ? GetAuctionOutBid() : 0);
    data << uint32(BuyoutPrice);
#else
    data << uint64(StartingPrice);
    data << uint64(HighestBid ? GetAuctionOutBid() : 0);
    data << uint64(BuyoutPrice);
#endif

    data << uint32((ExpiryTime - UNIXTIME) * 1000);
    data << uint64(HighestBidder);

#if VERSION_STRING < Cata
    data << uint32(HighestBid);
#else
    data << uint64(HighestBid);
#endif
}

/// the sum of outbid is (1% from current bid)*5, if bid is very small, it is 1c
uint32 Auction::GetAuctionOutBid()
{
    uint32 outbid = HighestBid * 5 / 100;
    return outbid ? outbid : 1;
}

void AuctionHouse::SendBidListPacket(Player* plr, WorldPacket* /*packet*/)
{
    uint32 count = 0;
    uint32 totalcount = 0;

    WorldPacket data(SMSG_AUCTION_BIDDER_LIST_RESULT, 4 + 4 + 4);
    data << uint32(0);

    auctionLock.AcquireReadLock();
    for (auto itr = auctions.begin(); itr != auctions.end(); ++itr)
    {
        auto auction = itr->second;
        if (auction->HighestBidder == plr->getGuid())
        {
            if (auction->Deleted)
                continue;

            auction->AddToPacket(data);
            ++count;
            ++totalcount;
        }
    }

    data.put<uint32>(0, count);
    data << totalcount;
    data << uint32(300);
    auctionLock.ReleaseReadLock();
    plr->GetSession()->SendPacket(&data);
}

void AuctionHouse::SendOwnerListPacket(Player* plr, WorldPacket* /*packet*/)
{
    uint32 count = 0;
    uint32 totalcount = 0;

    WorldPacket data(SMSG_AUCTION_OWNER_LIST_RESULT, 4 + 4 + 4);
    data << uint32(0);

    auctionLock.AcquireReadLock();
    for (auto itr = auctions.begin(); itr != auctions.end(); ++itr)
    {
        auto auction = itr->second;
        if (auction->Owner == plr->getGuid())
        {
            if (auction->Deleted)
                continue;

            auction->AddToPacket(data);
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
