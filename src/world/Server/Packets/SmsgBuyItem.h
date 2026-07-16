/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
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
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << sellerGuid;

                if (m_protocol.expansion > WoW::Expansion::_TBC)
                    packet << time;

                packet << itemEntry << purchasedAmount;
            }
            else // Mop
            {
                WoWGuid selGuid = sellerGuid;

                packet.writeBit(selGuid[3]);
                packet.writeBit(selGuid[4]);
                packet.writeBit(selGuid[7]);
                packet.writeBit(selGuid[6]);
                packet.writeBit(selGuid[0]);
                packet.writeBit(selGuid[2]);
                packet.writeBit(selGuid[1]);
                packet.writeBit(selGuid[5]);

                packet.writeByteSeq(selGuid[6]);
                packet.writeByteSeq(selGuid[7]);

                packet << purchasedAmount;

                packet.writeByteSeq(selGuid[1]);
                packet.writeByteSeq(selGuid[3]);
                packet.writeByteSeq(selGuid[5]);
                packet.writeByteSeq(selGuid[2]);

                packet << int32_t(-1);

                packet.writeByteSeq(selGuid[0]);
                packet.writeByteSeq(selGuid[4]);

                packet << itemEntry;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
