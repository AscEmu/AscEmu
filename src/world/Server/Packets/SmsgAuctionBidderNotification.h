/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgAuctionBidderNotification : public ManagedPacket
    {
    public:
        uint32_t houseId;
        uint32_t auctionId;
        uint64_t newBidderGuid;

#if VERSION_STRING < Cata
        uint32_t newHighestBid;
        uint32_t outBid;
#else
        uint64_t newHighestBid;
        uint64_t outBid;
#endif
        uint32_t itemEntry;

        uint32_t unknown = 0;

        SmsgAuctionBidderNotification() : SmsgAuctionBidderNotification(0, 0, 0, 0, 0, 0)
        {
        }

        SmsgAuctionBidderNotification(uint32_t houseId, uint32_t auctionId, uint64_t newBidderGuid, uint32_t newHighestBid, uint32_t outBid, uint32_t itemEntry) :
            ManagedPacket(SMSG_AUCTION_BIDDER_NOTIFICATION, 28),
            houseId(houseId),
            auctionId(auctionId),
            newBidderGuid(newBidderGuid),
            newHighestBid(newHighestBid),
            outBid(outBid),
            itemEntry(itemEntry)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << houseId << auctionId << newBidderGuid << newHighestBid << outBid << itemEntry << unknown;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
