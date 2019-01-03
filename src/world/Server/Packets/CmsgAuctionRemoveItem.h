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
    class CmsgAuctionRemoveItem : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t auctionId;

        CmsgAuctionRemoveItem() : CmsgAuctionRemoveItem(0, 0)
        {
        }

        CmsgAuctionRemoveItem(uint64_t guid, uint32_t auctionId) :
            ManagedPacket(CMSG_AUCTION_REMOVE_ITEM, 12),
            guid(guid),
            auctionId(auctionId)
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
            packet >> unpacked_guid >> auctionId;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}}
