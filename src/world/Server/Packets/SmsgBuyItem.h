/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgBuyItem : public ManagedPacket
    {
    public:
        uint64_t sellerGuid;
        // cata specific
        uint32_t time;

        uint32_t itemEntry;
        uint32_t purchasedAmount;
        
        SmsgBuyItem() : SmsgBuyItem(0, 0, 0, 0)
        {
        }

        SmsgBuyItem(uint64_t sellerGuid, uint32_t time, uint32_t itemEntry, uint32_t purchasedAmount) :
            ManagedPacket(SMSG_BUY_ITEM, 1),
            sellerGuid(sellerGuid),
            time(time),
            itemEntry(itemEntry),
            purchasedAmount(purchasedAmount)
        {
        }

    protected:
        size_t expectedSize() const override { return 24; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << sellerGuid;
#if VERSION_STRING > TBC
            packet << time;
#endif
            packet << itemEntry << purchasedAmount;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
