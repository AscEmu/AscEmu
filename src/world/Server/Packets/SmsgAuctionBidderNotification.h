/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgAuctionBidderNotification : public ManagedPacket
    {
    public:
        uint32_t houseId;
        uint32_t auctionId;
        uint64_t newBidderGuid;
        uint64_t newHighestBid;
        uint64_t outBid;
        uint32_t itemEntry;
        uint32_t unknown = 0;

        SmsgAuctionBidderNotification() : SmsgAuctionBidderNotification(0, 0, 0, 0, 0, 0)
        {
        }

        SmsgAuctionBidderNotification(uint32_t houseId, uint32_t auctionId, uint64_t newBidderGuid,
            uint64_t newHighestBid, uint64_t outBid, uint32_t itemEntry) :
            ManagedPacket(SMSG_AUCTION_BIDDER_NOTIFICATION, 0),
            houseId(houseId),
            auctionId(auctionId),
            newBidderGuid(newBidderGuid),
            newHighestBid(newHighestBid),
            outBid(outBid),
            itemEntry(itemEntry)
        {
        }

    protected:
        bool uses64BitGold() const noexcept
        {
            return m_protocol.isContentAtLeast(WoW::Expansion::_Cata);
        }

        size_t expectedSize() const override
        {
            return uses64BitGold() ? 32 : 40;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << houseId << auctionId << newBidderGuid;

            if (uses64BitGold())
            {
                packet << static_cast<uint32_t>(newHighestBid);
                packet << static_cast<uint32_t>(outBid);
            }
            else
            {
                packet << newHighestBid << outBid;
            }

            packet << itemEntry << unknown;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
