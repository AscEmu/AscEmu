/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
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
#include "Server/Packets/SmsgAuctionBidderNotification.h"
#include "Server/Packets/SmsgAuctionOwnerNotification.h"
#include "Server/Packets/SmsgAuctionOwnerListResult.h"
#include "Server/Packets/SmsgAuctionBidderListResult.h"
#include "Server/Packets/SmsgAuctionListResult.h"
#include "Server/Packets/CmsgAuctionListItems.h"

using namespace AscEmu::Packets;

void Auction::deleteFromDB()
{
    CharacterDatabase.WaitExecute("DELETE FROM auctions WHERE auctionId = %u", Id);
}

void Auction::saveToDB(uint32_t auctionHouseId)
{
    CharacterDatabase.Execute("INSERT INTO auctions VALUES(%u, %u, %u, %u, %u, %u, %u, %u, %u, %u)", 
        Id, auctionHouseId, auctionItem->getGuidLow(), ownerGuid.getGuidLow(), static_cast<uint32_t>(startPrice), static_cast<uint32_t>(buyoutPrice), expireTime, highestBidderGuid.getGuidLow(), 
        static_cast<uint32_t>(highestBid), depositAmount);
}

void Auction::updateInDB()
{
    CharacterDatabase.Execute("UPDATE auctions SET bidder = %u, bid = %u WHERE auctionId = %u", highestBidderGuid.getGuidLow(), static_cast<uint32_t>(highestBid), Id);
}

AuctionPacketList Auction::getListMember()
{
    AuctionPacketList auctionList;
    auctionList.Id = Id;
    auctionList.itemEntry = auctionItem->getEntry();

    for (uint8_t i = 0; i < MAX_INSPECTED_ENCHANTMENT_SLOT; ++i)
    {
        auctionList.itemEnchantment[i].Id = auctionItem->getEnchantmentId(i);
        auctionList.itemEnchantment[i].duration = auctionItem->getEnchantmentDuration(i);
        auctionList.itemEnchantment[i].charges = auctionItem->getEnchantmentCharges(i);
    }

    auctionList.propertiesId = auctionItem->getRandomPropertiesId();
    auctionList.propertySeed = auctionItem->getPropertySeed();
    auctionList.stackCount = auctionItem->getStackCount();
    auctionList.chargesLeft = auctionItem->GetChargesLeft();
    auctionList.unknown = 0;

    auctionList.ownerGuid = ownerGuid;

    auctionList.startPrice = startPrice;
    auctionList.outBid = (highestBid ? getAuctionOutBid() : 0);
    auctionList.buyoutPrice = buyoutPrice;

    auctionList.expireTime = static_cast<uint32_t>(((expireTime - UNIXTIME) * 1000));
    auctionList.highestBidderGuid = highestBidderGuid;
    auctionList.highestBid = highestBid;

    return auctionList;
}

/// the sum of outbid is (1% from current bid)*5, if bid is very small, it is 1c
#if VERSION_STRING < Cata
uint32_t Auction::getAuctionOutBid() const
{
    uint32_t outbid = highestBid * 5 / 100;
    return outbid ? outbid : 1;
}
#else
uint64_t Auction::getAuctionOutBid() const
{
    uint64_t outbid = highestBid * 5 / 100;
    return outbid ? outbid : 1;
}
#endif

AuctionHouse::AuctionHouse(uint32_t id)
{
    auctionHouseEntryDbc = sAuctionHouseStore.LookupEntry(id);
    if (auctionHouseEntryDbc)
    {
        cutPercent = auctionHouseEntryDbc->tax / 100.0f;
        depositPercent = auctionHouseEntryDbc->fee / 100.0f;
        isEnabled = true;
    }
    else
    {
        isEnabled = false;
    }
}

AuctionHouse::~AuctionHouse()
{
    for (auto itr = auctions.begin(); itr != auctions.end(); ++itr)
        delete itr->second;
}

uint32_t AuctionHouse::getId() const { return auctionHouseEntryDbc ? auctionHouseEntryDbc->id : 0; }

void AuctionHouse::loadAuctionsFromDB()
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM auctions WHERE auctionhouse =%u", getId());
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

        auction->auctionItem = pItem;
        auction->ownerGuid = fields[3].GetUInt32();
        auction->startPrice = fields[4].GetUInt32();
        auction->buyoutPrice = fields[5].GetUInt32();
        auction->expireTime = fields[6].GetUInt32();
        auction->highestBidderGuid = fields[7].GetUInt32();
        auction->highestBid = fields[8].GetUInt32();
        auction->depositAmount = fields[9].GetUInt32();

        auction->removedType = AUCTION_REMOVE_EXPIRED;
        auction->isRemoved = false;

        auctions.insert(std::unordered_map<uint32_t, Auction*>::value_type(auction->Id, auction));
    }
    while (result->NextRow());
    delete result;
}

void AuctionHouse::updateAuctions()
{
    std::lock_guard<std::mutex> guard(auctionLock);

    removalLock.Acquire();

    auto const time = static_cast<uint32_t>(UNIXTIME);
    for (auto itr = auctions.begin(); itr != auctions.end();)
    {
        auto auction = itr->second;
        ++itr;

        if (time >= auction->expireTime)
        {
            if (auction->highestBidderGuid.getGuidLow() == 0)
            {
                auction->removedType = AUCTION_REMOVE_EXPIRED;
                this->sendAuctionExpiredNotificationPacket(auction);
            }
            else
            {
                auction->removedType = AUCTION_REMOVE_WON;
            }

            auction->isRemoved = true;
            removalList.push_back(auction);
        }
    }

    removalLock.Release();
}

void AuctionHouse::updateDeletionQueue()
{
    removalLock.Acquire();

    for (auto auction : removalList)
    {
        ARCEMU_ASSERT(auction->isRemoved);
        removeAuction(auction);
    }

    removalList.clear();
    removalLock.Release();
}

void AuctionHouse::removeAuction(Auction* auction)
{
    LOG_DEBUG("%u: Removing auction %u, reason %u.", auctionHouseEntryDbc->id, auction->Id, auction->removedType);

    char subject[100];
    char body[200];
    switch (auction->removedType)
    {
        case AUCTION_REMOVE_EXPIRED:
        {
            // ItemEntry:0:3
            snprintf(subject, 100, "%u:0:3", (unsigned int)auction->auctionItem->getEntry());

            // Auction expired, resend item, no money to owner.
            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, auctionHouseEntryDbc->id, auction->ownerGuid, subject, "", 0, 0, auction->auctionItem->getGuid(), MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);
        }
        break;

        case AUCTION_REMOVE_WON:
        {
            // ItemEntry:0:1
            snprintf(subject, 100, "%u:0:1", (unsigned int)auction->auctionItem->getEntry());

            // <owner player guid>:bid:buyout
            snprintf(body, 200, "%X:%u:%u", (unsigned int)auction->ownerGuid.getGuidLow(), (unsigned int)auction->highestBid, (unsigned int)auction->buyoutPrice);

            // Auction won by highest bidder. He gets the item.
            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, auctionHouseEntryDbc->id, auction->highestBidderGuid, subject, body, 0, 0, auction->auctionItem->getGuid(), MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);

            // Send a mail to the owner with his cut of the price.
            const uint32_t auction_cut = float2int32(cutPercent * auction->highestBid);
            auto amount = auction->highestBid - auction_cut + auction->depositAmount;
            if (amount < 0)
                amount = 0;

            // ItemEntry:0:2
            snprintf(subject, 100, "%u:0:2", (unsigned int)auction->auctionItem->getEntry());

            // <hex player guid>:bid:0:deposit:cut
            if (auction->highestBid == auction->buyoutPrice)       // Buyout
                snprintf(body, 200, "%X:%u:%u:%u:%u", (unsigned int)auction->highestBidderGuid.getGuidLow(), (unsigned int)auction->highestBid, (unsigned int)auction->buyoutPrice, (unsigned int)auction->depositAmount, (unsigned int)auction_cut);
            else
                snprintf(body, 200, "%X:%u:0:%u:%u", (unsigned int)auction->highestBidderGuid.getGuidLow(), (unsigned int)auction->highestBid, (unsigned int)auction->depositAmount, (unsigned int)auction_cut);

            // send message away.
            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, auctionHouseEntryDbc->id, auction->ownerGuid, subject, body, amount, 0, 0, MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);

            // If it's not a buyout (otherwise the players has been already notified)
            if (auction->highestBid < auction->buyoutPrice || auction->buyoutPrice == 0)
            {
                this->sendAuctionBuyOutNotificationPacket(auction);
            }
        }
        break;
        case AUCTION_REMOVE_CANCELLED:
        {
            snprintf(subject, 100, "%u:0:5", (unsigned int)auction->auctionItem->getEntry());
            const uint32_t cut = float2int32(cutPercent * auction->highestBid);
            Player* plr = sObjectMgr.GetPlayer(auction->ownerGuid.getGuidLow());
            if (cut && plr && plr->hasEnoughCoinage(cut))
                plr->modCoinage(-(int32)cut);

            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, getId(), auction->ownerGuid, subject, "", 0, 0, auction->auctionItem->getGuid(), MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);

            // return bidders money
            if (auction->highestBidderGuid)
            {
                sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, getId(), auction->highestBidderGuid, subject, "", auction->highestBid, 0, 0, MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);
            }

        }
        break;
    }

    // Remove the auction from the hashmap.
    auctionLock.lock();
    auctions.erase(auction->Id);
    auctionLock.unlock();

    // Destroy the item from memory (it still remains in the db)
    if (auction->auctionItem)
        auction->auctionItem->DeleteMe();

    // Finally destroy the auction instance.
    auction->deleteFromDB();
    delete auction;
}

void AuctionHouse::addAuction(Auction* auction)
{
    std::lock_guard<std::mutex> guard(auctionLock);

    auctions.insert(std::unordered_map<uint32_t, Auction*>::value_type(auction->Id, auction));

    LogDebug("AuctionHouse : %u: Add auction %u, expire@ %u.", auctionHouseEntryDbc->id, auction->Id, auction->expireTime);
}

Auction* AuctionHouse::getAuction(uint32_t id)
{
    std::lock_guard<std::mutex> guard(auctionLock);

    const auto auctionsMap = auctions.find(id);
    const auto auction = auctionsMap == auctions.end() ? nullptr : auctionsMap->second;

    return auction;
}

void AuctionHouse::queueDeletion(Auction* auction, uint32_t reasonType)
{
    if (auction->isRemoved)
        return;

    auction->isRemoved = true;
    auction->removedType = reasonType;
    removalLock.Acquire();
    removalList.push_back(auction);
    removalLock.Release();
}

void AuctionHouse::sendOwnerListPacket(Player* player, WorldPacket* /*packet*/)
{
    std::vector<AuctionPacketList> auctionPacketList{};

    std::lock_guard<std::mutex> guard(auctionLock);

    for (auto& itr : auctions)
    {
        auto auction = itr.second;
        if (auction->ownerGuid == player->getGuid())
        {
            if (auction->isRemoved)
                continue;

            auctionPacketList.push_back(auction->getListMember());
        }
    }

    player->SendPacket(SmsgAuctionOwnerListResult(static_cast<uint32_t>(auctionPacketList.size()), auctionPacketList, static_cast<uint32_t>(auctionPacketList.size())).serialise().get());
}

void AuctionHouse::updateOwner(uint32_t oldGuid, uint32_t newGuid)
{
    std::lock_guard<std::mutex> guard(auctionLock);

    for (auto& itr : auctions)
    {
        auto auction = itr.second;
        if (auction->ownerGuid.getGuidLow() == oldGuid)
            auction->ownerGuid = newGuid;

        if (auction->highestBidderGuid.getGuidLow() == oldGuid)
        {
            auction->highestBidderGuid = newGuid;
            auction->updateInDB();
        }
    }
}

void AuctionHouse::sendBidListPacket(Player* player, WorldPacket* /*packet*/)
{
    std::vector<AuctionPacketList> auctionPacketList{};

    std::lock_guard<std::mutex> guard(auctionLock);

    for (auto itr = auctions.begin(); itr != auctions.end(); ++itr)
    {
        auto auction = itr->second;
        if (auction->highestBidderGuid == player->getGuid())
        {
            if (auction->isRemoved)
                continue;

            auctionPacketList.push_back(auction->getListMember());
        }
    }

    player->SendPacket(SmsgAuctionBidderListResult(static_cast<uint32_t>(auctionPacketList.size()), auctionPacketList, static_cast<uint32_t>(auctionPacketList.size()), 300).serialise().get());
}

void AuctionHouse::sendAuctionBuyOutNotificationPacket(Auction* auction)
{
    Player* bidder = sObjectMgr.GetPlayer(auction->highestBidderGuid.getGuidLow());
    if (bidder && bidder->IsInWorld())
    {
        auto outbid = (auction->highestBid / 100) * 5;
        if (!outbid)
            outbid = 1;

        bidder->GetSession()->SendPacket(SmsgAuctionBidderNotification(getId(), auction->Id, auction->highestBidderGuid, 0, outbid, auction->auctionItem->getEntry()).serialise().get());
    }

    Player* owner = sObjectMgr.GetPlayer(auction->ownerGuid.getGuidLow());
    if (owner && owner->IsInWorld())
    {
        owner->GetSession()->SendPacket(SmsgAuctionOwnerNotification(auction->Id, auction->highestBid, auction->auctionItem->getEntry()).serialise().get());
    }
}

void AuctionHouse::sendAuctionOutBidNotificationPacket(Auction* auction, uint64_t newBidder, uint32_t newHighestBid)
{
    Player* bidder = sObjectMgr.GetPlayer(auction->highestBidderGuid.getGuidLow());
    if (bidder && bidder->IsInWorld())
    {
        auto outbid = (auction->highestBid / 100) * 5;
        if (!outbid)
            outbid = 1;

        bidder->GetSession()->SendPacket(SmsgAuctionBidderNotification(getId(), auction->Id, newBidder, newHighestBid, outbid, auction->auctionItem->getEntry()).serialise().get());
    }
}

void AuctionHouse::sendAuctionExpiredNotificationPacket(Auction* /*auct*/)
{
    ///\todo I don't know the net code... so: TODO ;-)

    //Player* owner = sObjectMgr.GetPlayer(auct->ownerGuid.getGuidLow());
    //if (owner && owner->IsInWorld())
    //{
    //  WorldPacket data(SMSG_AUCTION_REMOVED_NOTIFICATION, ??);
    //  data << GetID();
    //  data << auct->Id;
    //  data << uint32_t(0);   // I don't have an active blizz account..so I can't get the netcode by myself.
    //  data << uint32_t(0);
    //  data << uint32_t(0);
    //  data << auct->pItem->getEntry();
    //  data << uint32_t(0);
    //  owner->GetSession()->SendPacket(&data);
    //}
}

void AuctionHouse::sendAuctionList(Player* player, AscEmu::Packets::CmsgAuctionListItems srlPacket)
{
    std::vector<AuctionPacketList> auctionPacketList{};

    uint32_t count = 0;
    uint32_t totalcount = 0;

    // convert auction string to lowercase for faster parsing.
    if (srlPacket.searchedName.length() > 0)
    {
        for (uint32_t j = 0; j < srlPacket.searchedName.length(); ++j)
            srlPacket.searchedName[j] = static_cast<char>(tolower(srlPacket.searchedName[j]));
    }

    std::lock_guard<std::mutex> guard(auctionLock);

    for (auto& auction : auctions)
    {
        if (auction.second->isRemoved)
            continue;

        ItemProperties const* proto = auction.second->auctionItem->getItemProperties();

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
            if (proto->AllowableClass && !(player->getClassMask() & proto->AllowableClass))
                continue;

            if (proto->RequiredLevel && proto->RequiredLevel > player->getLevel())
                continue;

            if (proto->AllowableRace && !(player->getRaceMask() & proto->AllowableRace))
                continue;

            if (proto->Class == 4 && proto->SubClass && !(player->GetArmorProficiency() & (((uint32_t)(1)) << proto->SubClass)))
                continue;

            if (proto->Class == 2 && proto->SubClass && !(player->GetWeaponProficiency() & (((uint32_t)(1)) << proto->SubClass)))
                continue;

            if (proto->RequiredSkill && (!player->_HasSkillLine(proto->RequiredSkill) || proto->RequiredSkillRank > player->_GetSkillLineCurrent(proto->RequiredSkill, true)))
                continue;
        }

        if (count < 50 && totalcount >= srlPacket.listFrom)
        {
            ++count;

            auctionPacketList.push_back(auction.second->getListMember());
        }

        ++totalcount;
    }

    player->SendPacket(SmsgAuctionListResult(static_cast<uint32_t>(auctionPacketList.size()), auctionPacketList, static_cast<uint32_t>(auctionPacketList.size()), 300).serialise().get());
}
