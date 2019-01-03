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
    class CmsgAuctionListOwnerItems : public ManagedPacket
    {
    public:
        WoWGuid guid;

        CmsgAuctionListOwnerItems() : CmsgAuctionListOwnerItems(0)
        {
        }

        CmsgAuctionListOwnerItems(uint64_t guid) :
            ManagedPacket(CMSG_AUCTION_LIST_OWNER_ITEMS, 8),
            guid(guid)
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
            packet >> unpacked_guid;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}}
