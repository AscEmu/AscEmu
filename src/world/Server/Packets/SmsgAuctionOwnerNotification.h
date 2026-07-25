/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgAuctionOwnerNotification : public ManagedPacket
    {
    public:
        uint32_t auctionId;
        uint64_t newHighestBid;
        uint64_t unknown1 = 0;
        uint64_t unknown2 = 0;
        uint32_t itemEntry;
        uint32_t unknown = 0;
        float unknownFloat = 0;

        SmsgAuctionOwnerNotification() : SmsgAuctionOwnerNotification(0, 0, 0)
        {
        }

        SmsgAuctionOwnerNotification(uint32_t auctionId, uint64_t newHighestBid, uint32_t itemEntry) :
            ManagedPacket(SMSG_AUCTION_OWNER_NOTIFICATION, 0),
            auctionId(auctionId),
            newHighestBid(newHighestBid),
            itemEntry(itemEntry)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
                return m_protocol.expansion <= WoW::Expansion::_TBC ? 24 : 28;

            return 40;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << auctionId;

            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << static_cast<uint32_t>(newHighestBid);
                packet << static_cast<uint32_t>(unknown1);
                packet << static_cast<uint32_t>(unknown2);
            }
            else
            {
                packet << newHighestBid << unknown1 << unknown2;
            }

            packet << itemEntry << unknown;

            if (m_protocol.expansion > WoW::Expansion::_TBC)
                packet << unknownFloat;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
