/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgAuctionCommandResult : public ManagedPacket
    {
    public:
        uint32_t auctionId;
        uint32_t command;
        uint32_t error;

#if VERSION_STRING >= Cata
        uint64_t outBid;
        uint64_t highestBid;
#else
        uint32_t outBid;
        uint32_t highestBid;
#endif
        uint32_t bidError;
        uint64_t highestBidderGuid;

        SmsgAuctionCommandResult() : SmsgAuctionCommandResult(0, 0, 0, 0, 0, 0, 0)
        {
        }

        SmsgAuctionCommandResult(uint32_t auctionId, uint32_t command, uint32_t error, uint32_t outBid = 0, uint32_t highestBid = 0, uint32_t bidError = 0, uint64_t highestBidderGuid = 0) :
            ManagedPacket(SMSG_AUCTION_COMMAND_RESULT, 12),
            auctionId(auctionId),
            command(command),
            error(error),
            outBid(outBid),
            highestBid(highestBid),
            bidError(bidError),
            highestBidderGuid(highestBidderGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << auctionId << command << error;

            switch (error)
            {
                case 0:     // AUCTION_ERROR_NONE
                    if (command == 2)   // AUCTION_ACTION_BID
                        packet << outBid;
                    break;
                case 1:     // AUCTION_ERROR_INVENTORY
                    packet << bidError;
                    break;
                case 5:     // AUCTION_ERROR_HIGHER_BID
                    packet << highestBidderGuid << highestBid << outBid;
                    break;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
