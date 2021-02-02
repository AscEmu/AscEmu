/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Storage/DBC/DBCStructures.hpp"
#include "WorldConf.h"
#include "Item.h"

namespace AscEmu::Packets
{
    class CmsgAuctionListItems;
}

enum AuctionRemoveType
{
    AUCTION_REMOVE_EXPIRED,
    AUCTION_REMOVE_WON,
    AUCTION_REMOVE_CANCELLED
};

enum AuctionAction
{
    AUCTION_ACTION_CREATE,
    AUCTION_ACTION_CANCEL,
    AUCTION_ACTION_BID,
    AUCTION_ACTION_BUYOUT
};

#define MIN_AUCTION_TIME (12 * HOUR)
#define MAX_AUCTION_ITEMS 160

enum AuctionResultError
{
    AUCTION_ERROR_NONE = 0,
    AUCTION_ERROR_INVENTORY = 1,
    AUCTION_ERROR_INTERNAL = 2,
    AUCTION_ERROR_MONEY = 3,
    AUCTION_ERROR_ITEM = 4,
    AUCTION_ERROR_HIGHER_BID = 5,
    AUCTION_ERROR_BID_INCREMENT = 7,
    AUCTION_ERROR_BID_OWN_AUCTION = 10,
    AUCTION_ERROR_RESTRICTED_ACCOUNT = 13
};

enum AuctionMailResult
{
    AUCTION_OUTBID,
    AUCTION_WON,
    AUCTION_SOLD,
    AUCTION_EXPIRED,
    AUCTION_EXPIRED2,
    AUCTION_CANCELLED
};

struct AuctionPacketItemEnchantment
{
    uint32_t Id;
    uint32_t duration;
    uint32_t charges;
};

struct AuctionPacketList
{
    uint32_t Id;
    uint32_t itemEntry;

    AuctionPacketItemEnchantment itemEnchantment[MAX_INSPECTED_ENCHANTMENT_SLOT];

    uint32_t propertiesId;
    uint32_t propertySeed;
    uint32_t stackCount;
    uint32_t chargesLeft;
    uint32_t unknown;

    uint64_t ownerGuid;

#if VERSION_STRING < Cata
    uint32_t startPrice;
    uint32_t outBid;
    uint32_t buyoutPrice;
#else
    uint64_t startPrice;
    uint64_t outBid;
    uint64_t buyoutPrice;
#endif

    uint32_t expireTime;

    uint64_t highestBidderGuid;

#if VERSION_STRING < Cata
    uint32_t highestBid;
#else
    uint64_t highestBid;
#endif
};

struct Auction
{
    uint32_t Id;

    WoWGuid ownerGuid;
    WoWGuid highestBidderGuid;

 #if VERSION_STRING < Cata
    uint32_t highestBid;
    uint32_t startPrice;
    uint32_t buyoutPrice;
#else
    uint64_t highestBid;
    uint64_t startPrice;
    uint64_t buyoutPrice;
#endif

    uint32_t depositAmount;

    uint32_t expireTime;
    Item* auctionItem;

    void deleteFromDB();
    void saveToDB(uint32_t auctionHouseId);
    void updateInDB();

#if VERSION_STRING < Cata
    uint32_t getAuctionOutBid() const;
#else
    uint64_t getAuctionOutBid() const;
#endif

    bool isRemoved;
    uint32_t removedType;

    AuctionPacketList getListMember();
};

class AuctionHouse
{
public:
    AuctionHouse(uint32_t id);
    ~AuctionHouse();

    uint32_t getId() const;
    void loadAuctionsFromDB();

    void updateAuctions();
    void updateDeletionQueue();

    void removeAuction(Auction* auction);
    void addAuction(Auction* auction);
    Auction* getAuction(uint32_t id);
    void queueDeletion(Auction* auction, uint32_t reasonType);

    void sendOwnerListPacket(Player* player, WorldPacket* packet);
    void updateOwner(uint32_t oldGuid, uint32_t newGuid);
    void sendBidListPacket(Player* player, WorldPacket* packet);
    void sendAuctionBuyOutNotificationPacket(Auction* auction);
    void sendAuctionOutBidNotificationPacket(Auction* auction, uint64_t newBidder, uint32_t newHighestBid);
    void sendAuctionExpiredNotificationPacket(Auction* auction);
    void sendAuctionList(Player* player, AscEmu::Packets::CmsgAuctionListItems srlPacket);

private:
    std::mutex auctionLock;
    std::unordered_map<uint32_t, Auction*> auctions;

    Mutex removalLock;
    std::list<Auction*> removalList;

    DBC::Structures::AuctionHouseEntry const* auctionHouseEntryDbc;

public:
    float cutPercent;
    float depositPercent;
    bool isEnabled;
};
