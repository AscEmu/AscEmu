/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

#ifndef AUCTIONHOUSE_H
#define AUCTIONHOUSE_H

#include "Storage/DBC/DBCStructures.hpp"
#include "WorldConf.h"
#include "Item.h"

enum AuctionRemoveType
{
    AUCTION_REMOVE_EXPIRED,
    AUCTION_REMOVE_WON,
    AUCTION_REMOVE_CANCELLED
};

enum AUCTIONRESULT
{
    AUCTION_CREATE,
    AUCTION_CANCEL,
    AUCTION_BID,
    AUCTION_BUYOUT
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

struct Auction
{
    uint32 Id;

    uint32 Owner;
    uint32 HighestBidder;
    uint32 HighestBid;
    uint32 StartingPrice;
    uint32 BuyoutPrice;
    uint32 DepositAmount;

    uint32 ExpiryTime;
    Item* pItem;

    void DeleteFromDB();
    void SaveToDB(uint32 AuctionHouseId);
    void UpdateInDB();
#if VERSION_STRING < Cata
    void AddToPacket(WorldPacket& data);
#else
    uint32 GetAuctionOutBid();
    bool BuildAuctionInfo(WorldPacket & data);
#endif
    bool Deleted;
    uint32 DeletedReason;
};

class AuctionHouse
{
    public:

        AuctionHouse(uint32 ID);
        ~AuctionHouse();

        inline uint32 GetID() { return dbc->id; }
        void LoadAuctions();

        void UpdateAuctions();
        void UpdateDeletionQueue();

        void RemoveAuction(Auction* auct);
        void AddAuction(Auction* auct);
        Auction* GetAuction(uint32 Id);
        void QueueDeletion(Auction* auct, uint32 Reason);

        void SendOwnerListPacket(Player* plr, WorldPacket* packet);
        void UpdateOwner(uint32 oldGuid, uint32 newGuid);
        void SendBidListPacket(Player* plr, WorldPacket* packet);
        void SendAuctionBuyOutNotificationPacket(Auction* auct);
        void SendAuctionOutBidNotificationPacket(Auction* auct, uint64 newBidder, uint32 newHighestBid);
        void SendAuctionExpiredNotificationPacket(Auction* auct);
        void SendAuctionList(Player* plr, WorldPacket* packet);

    private:

        RWLock auctionLock;
        std::unordered_map<uint32, Auction*> auctions;

        Mutex removalLock;
        std::list<Auction*> removalList;

        DBC::Structures::AuctionHouseEntry const* dbc;

    public:

        float cut_percent;
        float deposit_percent;
        bool enabled;
};

#endif // AUCTIONHOUSE_H
