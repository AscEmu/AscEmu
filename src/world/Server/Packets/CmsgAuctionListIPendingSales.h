/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgAuctionListIPendingSales : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgAuctionListIPendingSales() : CmsgAuctionListIPendingSales(0)
        {
        }

        CmsgAuctionListIPendingSales(uint64_t guid) :
            ManagedPacket(CMSG_AUCTION_LIST_PENDING_SALES, 0),
            guid(guid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid;
            guid.init(unpacked_guid);
            return true;
        }
    };
}
