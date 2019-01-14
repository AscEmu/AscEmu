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
    class MsgAuctionHello : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t auctionHouseId;
        uint8_t isAuctionHouseEnabled;

        MsgAuctionHello() : MsgAuctionHello(0, 0, 0)
        {
        }

        MsgAuctionHello(uint64_t guid, uint32_t auctionHouseId, uint8_t isAuctionHouseEnabled) :
            ManagedPacket(MSG_AUCTION_HELLO, 12),
            guid(guid),
            auctionHouseId(auctionHouseId),
            isAuctionHouseEnabled(isAuctionHouseEnabled)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid.GetOldGuid() << auctionHouseId << isAuctionHouseEnabled;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            guid.Init(unpackedGuid);
            return true;
        }
    };
}}
