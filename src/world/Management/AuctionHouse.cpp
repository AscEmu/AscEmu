/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Management/AuctionHouse.h"

#include "MailMgr.h"
#include "Logging/Log.hpp"
#include "Logging/Logger.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Objects/Item.hpp"
#include "Management/ItemInterface.h"
#include "Map/Management/MapMgr.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/WorldSession.h"
#include "Server/Packets/SmsgAuctionBidderNotification.h"
#include "Server/Packets/SmsgAuctionOwnerNotification.h"
#include "Server/Packets/SmsgAuctionOwnerListResult.h"
#include "Server/Packets/SmsgAuctionBidderListResult.h"
#include "Server/Packets/SmsgAuctionListResult.h"
#include "Server/Packets/CmsgAuctionListItems.h"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Narrow.hpp"
#include "Utilities/Strings.hpp"

using namespace AscEmu::Packets;

Auction::Auction() = default;
Auction::Auction(Field const* fields, std::unique_ptr<Item> pItem)
{
    Id = fields[0].asUint32();
    auctionItem = std::move(pItem);
    ownerGuid = fields[3].asUint32();
    startPrice = fields[4].asUint32();
    buyoutPrice = fields[5].asUint32();
    expireTime = fields[6].asUint32();
    highestBidderGuid = fields[7].asUint32();
    highestBid = fields[8].asUint32();
    depositAmount = fields[9].asUint32();

    removedType = AUCTION_REMOVE_EXPIRED;
    isRemoved = false;
}

void Auction::deleteFromDB()
{
    CharacterDatabase.WaitExecute("DELETE FROM auctions WHERE auctionId = %u", Id);
}

void Auction::saveToDB(uint32_t auctionHouseId)
{
    CharacterDatabase.Execute("INSERT INTO auctions VALUES(%u, %u, %u, %u, %u, %u, %u, %u, %u, %u)", 
        Id, auctionHouseId, auctionItem->getGuidLow(), ownerGuid.getGuidLow(), startPrice, buyoutPrice, expireTime, highestBidderGuid.getGuidLow(), 
        highestBid, depositAmount);
}

void Auction::updateInDB()
{
    CharacterDatabase.Execute("UPDATE auctions SET bidder = %u, bid = %u WHERE auctionId = %u", highestBidderGuid.getGuidLow(), highestBid, Id);
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
    auctionList.chargesLeft = auctionItem->getChargesLeft();
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
    auctionHouseEntryDbc = sAuctionHouseStore.lookupEntry(id);
    if (auctionHouseEntryDbc)
    {
        cutPercent = static_cast<float_t>(auctionHouseEntryDbc->tax) / 100.0f;
        depositPercent = static_cast<float_t>(auctionHouseEntryDbc->fee) / 100.0f;
        isEnabled = true;
    }
    else
    {
        isEnabled = false;
    }
}

AuctionHouse::~AuctionHouse() = default;

uint32_t AuctionHouse::getId() const { return auctionHouseEntryDbc ? auctionHouseEntryDbc->id : 0; }

void AuctionHouse::loadAuctionsFromDB()
{
    auto result = CharacterDatabase.Query("SELECT * FROM auctions WHERE auctionhouse =%u", getId());
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        const auto auctionId = fields[0].asUint32();

        auto pItem = sObjectMgr.loadItem(fields[2].asUint32());
        if (!pItem)
        {
            CharacterDatabase.Execute("DELETE FROM auctions WHERE auctionId=%u", auctionId);
            continue;
        }

        auctions.try_emplace(auctionId, std::make_unique<Auction>(fields, std::move(pItem)));
    }
    while (result->NextRow());
}

void AuctionHouse::updateAuctions()
{
    std::lock_guard guard(auctionLock);
    std::lock_guard lock(removalLock);

    auto const time = static_cast<uint32_t>(UNIXTIME);
    for (auto itr = auctions.begin(); itr != auctions.end();)
    {
        const auto& auction = itr->second;
        ++itr;

        if (auction->isRemoved)
            continue;

        if (time >= auction->expireTime)
        {
            if (auction->highestBidderGuid.getGuidLow() == 0)
            {
                auction->removedType = AUCTION_REMOVE_EXPIRED;
                this->sendAuctionExpiredNotificationPacket(auction.get());
            }
            else
            {
                auction->removedType = AUCTION_REMOVE_WON;
            }

            auction->isRemoved = true;
            removalList.push_back(auction.get());
        }
    }
}

void AuctionHouse::updateDeletionQueue()
{
    std::lock_guard lock(removalLock);

    for (auto auction : removalList)
    {
        removeAuction(auction);
    }

    removalList.clear();
}

void AuctionHouse::removeAuction(Auction* auction)
{
    sLogger.debug("{}: Removing auction {}, reason {}.", auctionHouseEntryDbc->id, auction->Id, auction->removedType);

    char subject[100];
    char body[200];
    switch (auction->removedType)
    {
        case AUCTION_REMOVE_EXPIRED:
        {
            // ItemEntry:0:3
            snprintf(subject, 100, "%u:0:3", auction->auctionItem->getEntry());

            // Auction expired, resend item, no money to owner.
            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, auctionHouseEntryDbc->id, auction->ownerGuid, subject, "", 0, 0, auction->auctionItem->getGuid(), MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);
        }
        break;
        case AUCTION_REMOVE_WON:
        {
            // ItemEntry:0:1
            snprintf(subject, 100, "%u:0:1", auction->auctionItem->getEntry());

            // <owner player guid>:bid:buyout
            snprintf(body, 200, "%X:%s:%s", auction->ownerGuid.getGuidLow(), std::to_string(auction->highestBid).c_str(), std::to_string(auction->buyoutPrice).c_str());

            // Auction won by highest bidder. He gets the item.
            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, auctionHouseEntryDbc->id, auction->highestBidderGuid, subject, body, 0, 0, auction->auctionItem->getGuid(), MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);

            // Send a mail to the owner with his cut of the price.
            const auto auction_cut = Util::float2int32(cutPercent * static_cast<float_t>(auction->highestBid));
            auto amount = auction->highestBid - auction_cut + auction->depositAmount;

            // ItemEntry:0:2
            snprintf(subject, 100, "%u:0:2", auction->auctionItem->getEntry());

            // <hex player guid>:bid:0:deposit:cut
            if (auction->highestBid == auction->buyoutPrice)       // Buyout
                snprintf(body, 200, "%X:%s:%s:%u:%u", auction->highestBidderGuid.getGuidLow(), std::to_string(auction->highestBid).c_str(), std::to_string(auction->buyoutPrice).c_str(), auction->depositAmount, (unsigned int)auction_cut);
            else
                snprintf(body, 200, "%X:%s:0:%u:%u", auction->highestBidderGuid.getGuidLow(), std::to_string(auction->highestBid).c_str(), auction->depositAmount, (unsigned int)auction_cut);

            // send message away.
            sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, auctionHouseEntryDbc->id, auction->ownerGuid, subject, body, static_cast<uint32_t>(amount), 0, 0, MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);

            // If it's not a buyout (otherwise the players has been already notified)
            if (auction->highestBid < auction->buyoutPrice || auction->buyoutPrice == 0)
            {
                this->sendAuctionBuyOutNotificationPacket(auction);
            }
        }
        break;
        case AUCTION_REMOVE_CANCELLED:
        {
            if (auction->auctionItem)
            {
                snprintf(subject, 100, "%u:0:5", auction->auctionItem->getEntry());
                const auto cut = Util::float2int32(cutPercent * static_cast<float_t>(auction->highestBid));
                Player* plr = sObjectMgr.getPlayer(auction->ownerGuid.getGuidLow());
                if (cut && plr && plr->hasEnoughCoinage(static_cast<uint32_t>(cut)))
                    plr->modCoinage(-cut);

                sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, getId(), auction->ownerGuid, subject, "", 0, 0, auction->auctionItem->getGuid(), MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);
            }
            // return bidders money
            if (auction->highestBidderGuid)
            {
                sMailSystem.SendAutomatedMessage(MAIL_TYPE_AUCTION, getId(), auction->highestBidderGuid, subject, "", static_cast<uint32_t>(auction->highestBid), 0, 0, MAIL_STATIONERY_AUCTION, MAIL_CHECK_MASK_COPIED);
            }

        }
        break;
    }

    // Destroy the item from memory (it still remains in the db)
    auction->auctionItem = nullptr;

    // Finally destroy the auction instance.
    auction->deleteFromDB();

    // Remove the auction from the hashmap.
    auctionLock.lock();
    auctions.erase(auction->Id);
    auctionLock.unlock();
}

Auction* AuctionHouse::addAuction(std::unique_ptr<Auction> auction)
{
    std::lock_guard<std::mutex> guard(auctionLock);

    const auto [itr, _] = auctions.try_emplace(auction->Id, std::move(auction));

    sLogger.debug("AuctionHouse : {}: Add auction {}, expire@ {}.", auctionHouseEntryDbc->id, itr->second->Id, itr->second->expireTime);
    return itr->second.get();
}

Auction* AuctionHouse::getAuction(uint32_t id)
{
    std::lock_guard<std::mutex> guard(auctionLock);

    const auto auctionsMap = auctions.find(id);
    const auto auction = auctionsMap == auctions.end() ? nullptr : auctionsMap->second.get();

    return auction;
}

void AuctionHouse::queueDeletion(Auction* auction, uint32_t reasonType)
{
    if (auction->isRemoved)
        return;

    auction->isRemoved = true;
    auction->removedType = reasonType;

    std::lock_guard lock(removalLock);

    removalList.push_back(auction);
}

void AuctionHouse::sendOwnerListPacket(Player* player, WorldPacket* /*packet*/)
{
    std::vector<AuctionPacketList> auctionPacketList{};

    std::lock_guard<std::mutex> guard(auctionLock);

    for (auto& itr : auctions)
    {
        const auto& auction = itr.second;
        if (auction->ownerGuid == player->getGuid())
        {
            if (auction->isRemoved)
                continue;

            auctionPacketList.push_back(auction->getListMember());
        }
    }

    player->sendPacket(SmsgAuctionOwnerListResult(static_cast<uint32_t>(auctionPacketList.size()), auctionPacketList, static_cast<uint32_t>(auctionPacketList.size())).serialise().get());
}

void AuctionHouse::updateOwner(uint32_t oldGuid, uint32_t newGuid)
{
    std::lock_guard<std::mutex> guard(auctionLock);

    for (auto& itr : auctions)
    {
        const auto& auction = itr.second;
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
        const auto& auction = itr->second;
        if (auction->highestBidderGuid == player->getGuid())
        {
            if (auction->isRemoved)
                continue;

            auctionPacketList.push_back(auction->getListMember());
        }
    }

    player->sendPacket(SmsgAuctionBidderListResult(static_cast<uint32_t>(auctionPacketList.size()), auctionPacketList, static_cast<uint32_t>(auctionPacketList.size()), 300).serialise().get());
}

void AuctionHouse::sendAuctionBuyOutNotificationPacket(Auction* auction)
{
    Player* bidder = sObjectMgr.getPlayer(auction->highestBidderGuid.getGuidLow());
    if (bidder && bidder->IsInWorld())
    {
        auto outbid = (auction->highestBid / 100) * 5;
        if (!outbid)
            outbid = 1;

        bidder->getSession()->SendPacket(SmsgAuctionBidderNotification(getId(), auction->Id, auction->highestBidderGuid, 0, outbid, auction->auctionItem->getEntry()).serialise().get());
    }

    Player* owner = sObjectMgr.getPlayer(auction->ownerGuid.getGuidLow());
    if (owner && owner->IsInWorld())
    {
        owner->getSession()->SendPacket(SmsgAuctionOwnerNotification(auction->Id, static_cast<uint32_t>(auction->highestBid), auction->auctionItem->getEntry()).serialise().get());
    }
}

void AuctionHouse::sendAuctionOutBidNotificationPacket(Auction* auction, uint64_t newBidder, uint32_t newHighestBid)
{
    Player* bidder = sObjectMgr.getPlayer(auction->highestBidderGuid.getGuidLow());
    if (bidder && bidder->IsInWorld())
    {
        auto outbid = (auction->highestBid / 100) * 5;
        if (!outbid)
            outbid = 1;

        bidder->getSession()->SendPacket(SmsgAuctionBidderNotification(getId(), auction->Id, newBidder, newHighestBid, outbid, auction->auctionItem->getEntry()).serialise().get());
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
    //  owner->getSession()->sendPacket(&data);
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
        if (srlPacket.searchedName.length() > 0 && !AscEmu::Util::Strings::contains(srlPacket.searchedName, proto_lower))
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

            if (proto->Class == 4 && proto->SubClass && !(player->getArmorProficiency() & (((uint32_t)(1)) << proto->SubClass)))
                continue;

            if (proto->Class == 2 && proto->SubClass && !(player->getWeaponProficiency() & (((uint32_t)(1)) << proto->SubClass)))
                continue;

            if (proto->RequiredSkill && (!player->hasSkillLine(proto->RequiredSkill) || proto->RequiredSkillRank > player->getSkillLineCurrent(proto->RequiredSkill, true)))
                continue;
        }

        if (count < 50 && totalcount >= srlPacket.listFrom)
        {
            ++count;

            auctionPacketList.push_back(auction.second->getListMember());
        }

        ++totalcount;
    }

    player->sendPacket(SmsgAuctionListResult(static_cast<uint32_t>(auctionPacketList.size()), auctionPacketList, totalcount, 300).serialise().get());
}
