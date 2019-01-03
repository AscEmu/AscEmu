/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgSellItem : public ManagedPacket
    {
    public:
        uint64_t vendorGuid;
        uint64_t itemGuid;
        
        SmsgSellItem() : SmsgSellItem(0, 0)
        {
        }

        SmsgSellItem(uint64_t vendorGuid, uint64_t itemGuid) :
            ManagedPacket(SMSG_SELL_ITEM, 1),
            vendorGuid(vendorGuid),
            itemGuid(itemGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return 12; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << vendorGuid << itemGuid << uint8_t(0);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
