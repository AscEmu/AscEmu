/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"
#include "Management/AuctionHouse.h"

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

            for (auto itemList : auctionPacketList)
            {
                packet << itemList.Id << itemList.itemEntry;

                for (auto enchantments : itemList.itemEnchantment)
                    packet << enchantments.Id << enchantments.duration << enchantments.charges;

                packet << itemList.propertiesId << itemList. propertySeed << itemList.stackCount << itemList.chargesLeft;
                packet << itemList.unknown << itemList.ownerGuid << itemList.startPrice << itemList.outBid << itemList.buyoutPrice;
                packet << itemList.expireTime << itemList.highestBidderGuid << itemList.highestBid;
            }

            packet << totalCount;
            packet << unknown;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
