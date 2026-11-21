/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/AuctionHouse.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgAuctionOwnerListResult : public ManagedPacket
    {
    public:
        uint32_t listItemCount;
        std::vector<AuctionPacketList> auctionPacketList;
        uint32_t totalCount;
        uint32_t unknown;

        SmsgAuctionOwnerListResult() : SmsgAuctionOwnerListResult(0, {}, 0, 0)
        {
        }

        SmsgAuctionOwnerListResult(uint32_t listItemCount, std::vector<AuctionPacketList> auctionPacketList, uint32_t totalCount, uint32_t unknown = 0) :
            ManagedPacket(SMSG_AUCTION_OWNER_LIST_RESULT, 24),
            listItemCount(listItemCount),
            auctionPacketList(std::move(auctionPacketList)),
            totalCount(totalCount),
            unknown(unknown)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << listItemCount;

            for (const auto& itemList : auctionPacketList)
                itemList.serialise(packet, m_protocol);

            packet << totalCount;
            packet << unknown;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
