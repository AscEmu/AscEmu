/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgAuctionListIBidderItemse : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t listForm;
        uint32_t outbiddedCount;

        CmsgAuctionListIBidderItemse() : CmsgAuctionListIBidderItemse(0, 0, 0)
        {
        }

        CmsgAuctionListIBidderItemse(uint64_t guid, uint32_t listForm, uint32_t outbiddedCount) :
            ManagedPacket(CMSG_AUCTION_LIST_BIDDER_ITEMS, 0),
            guid(guid),
            listForm(listForm),
            outbiddedCount(outbiddedCount)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid >> listForm >> outbiddedCount;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}
