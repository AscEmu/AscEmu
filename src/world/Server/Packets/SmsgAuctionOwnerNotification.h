/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgAuctionOwnerNotification : public ManagedPacket
    {
    public:
        uint32_t auctionId;

#if VERSION_STRING < Cata
        uint32_t newHighestBid;
        uint32_t unknown1 = 0;
        uint32_t unknown2 = 0;
#else
        uint64_t newHighestBid;
        uint64_t unknown1 = 0;
        uint64_t unknown2 = 0;
#endif
        uint32_t itemEntry;

        uint32_t unknown = 0;
        float unknownFloat = 0;

        SmsgAuctionOwnerNotification() : SmsgAuctionOwnerNotification(0, 0, 0)
        {
        }

        SmsgAuctionOwnerNotification(uint32_t auctionId, uint32_t newHighestBid, uint32_t itemEntry) :
            ManagedPacket(SMSG_AUCTION_OWNER_NOTIFICATION, 24),
            auctionId(auctionId),
            newHighestBid(newHighestBid),
            itemEntry(itemEntry)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << auctionId << newHighestBid;
            packet << unknown1 << unknown2;
            packet << itemEntry << unknown;
#if VERSION_STRING > TBC
            packet << unknownFloat;
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
