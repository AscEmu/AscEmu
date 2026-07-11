/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSellItem : public ManagedPacket
    {
    public:
        WoWGuid vendorGuid;
        uint64_t itemGuid;
        int8_t amount;

        CmsgSellItem() : CmsgSellItem(0, 0, 0)
        {
        }

        CmsgSellItem(uint64_t vendorGuid, uint64_t itemGuid, int8_t amount) :
            ManagedPacket(CMSG_SELL_ITEM, 17),
            vendorGuid(vendorGuid),
            itemGuid(itemGuid),
            amount(amount)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t rawGuid;
            packet >> rawGuid >> itemGuid >> amount;
            vendorGuid.init(rawGuid);
            return true;
        }
    };
}
