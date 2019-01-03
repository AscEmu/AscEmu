/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgAuctionPlaceBid : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t auctionId;
        uint32_t price;

        CmsgAuctionPlaceBid() : CmsgAuctionPlaceBid(0, 0, 0)
        {
        }

        CmsgAuctionPlaceBid(uint64_t guid, uint32_t auctionId, uint32_t price) :
            ManagedPacket(CMSG_AUCTION_PLACE_BID, 0),
            guid(guid),
            auctionId(auctionId),
            price(price)
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
            packet >> unpacked_guid >> auctionId >> price;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}}
