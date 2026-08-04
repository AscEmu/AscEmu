/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgAuctionRemovedNotification : public ManagedPacket
    {
    public:
        uint32_t auctionId;
        uint32_t itemEntry;
        uint32_t itemRandomPropertyId;

        SmsgAuctionRemovedNotification(uint32_t auctionId, uint32_t itemEntry, uint32_t itemRandomPropertyId) :
            ManagedPacket(SMSG_AUCTION_REMOVED_NOTIFICATION, 0),
            auctionId(auctionId),
            itemEntry(itemEntry),
            itemRandomPropertyId(itemRandomPropertyId)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + 4 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << auctionId;
            packet << itemEntry;
            packet << itemRandomPropertyId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
