/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgAuctionCommandResult : public ManagedPacket
    {
    public:
        uint32_t auctionId;
        uint32_t command;
        uint32_t error;
        uint32_t misc;

        SmsgAuctionCommandResult() : SmsgAuctionCommandResult(0, 0, 0, 0)
        {
        }

        SmsgAuctionCommandResult(uint32_t auctionId, uint32_t command, uint32_t error) :
            ManagedPacket(SMSG_AUCTION_COMMAND_RESULT, 12),
            auctionId(auctionId),
            command(command),
            error(error),
            misc(0)
        {
        }

        SmsgAuctionCommandResult(uint32_t auctionId, uint32_t command, uint32_t error, uint32_t misc) :
            ManagedPacket(SMSG_AUCTION_COMMAND_RESULT, 12),
            auctionId(auctionId),
            command(command),
            error(error),
            misc(misc)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << auctionId << command << error;
            if (command == 2)   //AUCTION_BID
                packet << misc;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
