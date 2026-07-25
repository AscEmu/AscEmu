/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgAuctionCommandResult : public ManagedPacket
    {
    public:
        uint32_t auctionId;
        uint32_t command;
        uint32_t error;
        uint64_t outBid;
        uint64_t highestBid;
        uint32_t bidError;
        uint64_t highestBidderGuid;

        SmsgAuctionCommandResult() : SmsgAuctionCommandResult(0, 0, 0, 0, 0, 0, 0)
        {
        }

        SmsgAuctionCommandResult(uint32_t auctionId, uint32_t command, uint32_t error,
            uint64_t outBid = 0, uint64_t highestBid = 0, uint32_t bidError = 0,
            uint64_t highestBidderGuid = 0) :
            ManagedPacket(SMSG_AUCTION_COMMAND_RESULT, 0),
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
        size_t expectedSize() const override
        {
            return m_protocol.expansion < WoW::Expansion::_Cata ? 28 : 36;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << auctionId << command << error;

            const auto writeBidAmount = [this, &packet](uint64_t amount)
            {
                if (m_protocol.expansion < WoW::Expansion::_Cata)
                    packet << static_cast<uint32_t>(amount);
                else
                    packet << amount;
            };

            switch (error)
            {
                case 0:     // AUCTION_ERROR_NONE
                    if (command == 2)   // AUCTION_ACTION_BID
                        writeBidAmount(outBid);
                    break;
                case 1:     // AUCTION_ERROR_INVENTORY
                    packet << bidError;
                    break;
                case 5:     // AUCTION_ERROR_HIGHER_BID
                    packet << highestBidderGuid;
                    writeBidAmount(highestBid);
                    writeBidAmount(outBid);
                    break;
                default:
                    break;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
